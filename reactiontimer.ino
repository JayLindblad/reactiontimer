
/*
  REACTION TRAINER — ESP8266 edition (NodeMCU / Wemos D1 mini)
  -----------------------------------------------------------
  The ESP8266 owns the clock. Each round it schedules the whole start
  sequence in its own microsecond time and announces that plan ahead of
  time. The web page keeps its clock synced to the ESP's (NTP-style pings)
  and draws every light on that schedule, so the lights-out people see
  matches the ESP's "GO" exactly. Presses are timed by interrupts against
  the scheduled lights-out; finished results go over WiFi to the page.

  Sequence:  5 flashes  →  solid (random hold)  →  OUT = GO
  Open:          http://reaction.local   (or the IP printed in Serial Monitor)

  Arduino IDE setup
    1. Boards Manager: install "esp8266" by ESP8266 Community
    2. Library Manager: install "WebSockets" by Markus Sattler
    3. Board: "NodeMCU 1.0 (ESP-12E Module)" or "LOLIN(WEMOS) D1 R2 & mini"
    4. Fill in your WiFi below, upload, open Serial Monitor at 115200

   Wiring (each button: one leg to the pin, other leg to GND)
    Lane 1 trigger  → GPIO5  (D1)
    Lane 2 trigger  → GPIO4  (D2)
    Lane 3 trigger  → GPIO12 (D6)
    Lane 4 trigger  → GPIO13 (D7)
    Start button    → GPIO14 (D5)
    Onboard LED     = GPIO2  (D4), nothing to wire

*/ 

#include <ESP8266WiFi.h>
#include <ESP8266WebServer.h>
#include <ESP8266mDNS.h>
#include <WebSocketsServer.h>

// ---------- WiFi ----------
// If it can't join your WiFi within 15 s, it starts its own hotspot instead
// (join "ReactionTrainer" and open http://192.168.4.1).
const char* WIFI_SSID = "WIFI";
const char* WIFI_PASS = "PASS";
const char* AP_SSID   = "ReactionTrainer";
const char* AP_PASS   = "racestart";        // at least 8 characters
const char* HOSTNAME  = "reaction";         // → http://reaction.local

// ---------- Pins ----------
const uint8_t LED_PIN     = 2;              // onboard blue LED, active LOW
const uint8_t LANE_PIN[4] = {5, 4, 12, 13}; // D1, D2, D6, D7
const uint8_t START_PIN   = 14;             // D5
#define LED_ON  LOW
#define LED_OFF HIGH

// ---------- Timing ----------
const uint16_t FLASH_ON_MS  = 300;
const uint16_t FLASH_OFF_MS = 300;
const uint8_t  FLASHES      = 5;
const uint16_t TIMEOUT_MS   = 3000;         // no press this long after GO = "no press"
const uint16_t LEAD_MS      = 250;          // plan is announced this far ahead so the page has it in time
const uint16_t CORR_WAIT_MS = 250;          // longest a press waits after GO for the start screen's correction
uint8_t  cfgLanes   = 2;                    // updated by the web page
uint16_t cfgMinHold = 1000;                 // solid-LED hold range, ms
uint16_t cfgMaxHold = 3000;
uint16_t cfgAntiMs  = 100;                  // faster than this = anticipated, can't win. 0 = off

ESP8266WebServer server(80);
WebSocketsServer ws(81);
#include "page.h"   // the web page lives in its own tab (see the bottom of this file)

// ---------- Round state (shared with interrupts) ----------
enum Phase : uint8_t { IDLE, SEQ, GO };
volatile Phase    phase       = IDLE;
volatile uint8_t  activeLanes = 2;
volatile uint32_t goUs32      = 0;          // scheduled lights-out, low 32 bits of micros64() = micros()
volatile uint32_t pressUs[4];
volatile bool     pressed[4], jumped[4];
volatile bool     startReq = false;

bool     reported[4];
int32_t  rtUs[4];                                // each lane's corrected time, for picking the winner
bool     okLane[4];
// ---------- Start screen correction ----------
// The screen drivers watch reports how far its lights-out frame landed from
// the scheduled GO (plus its display lag). Every time is corrected by that.
uint16_t roundId   = 0;
bool     corrKnown = false;
int32_t  corrUs    = 0;
// ---------- Button test mode ----------
bool     debugMode   = false;
uint32_t dbgLastMs   = 0;
uint32_t dbgStatusMs = 0;
volatile uint16_t edgeCount[5];                  // lanes 1-4, start — only ever counts up
uint8_t  dbgLevel[5] = {1, 1, 1, 1, 1};
const uint8_t DBG_PIN[5] = {5, 4, 12, 13, 14};
// ---------- Wiring health (always running, not just in test mode) ----------
// loop() polls every pin. A real press holds the contact for tens of ms, so the
// poll sees it LOW. An interrupt edge the poll never sees as LOW is a glitch:
// noise on the cable, which in a round would count as a jump start.
const uint8_t  DEBOUNCE_MS = 20;                 // ignore level flips this soon after a change
const uint8_t  SETTLE_MS   = 5;                  // stray edges this old with the pin still HIGH = glitch
const uint8_t  XT_MS       = 20;                 // glitch this close to another pin's change = crosstalk
const uint16_t STUCK_MS    = 2000;               // LOW this long while idle = held down / shorted
uint16_t lastEdges[5];                           // edgeCount at the previous poll
uint16_t bounceEdges[5], suspect[5];             // edges around the current press / not yet judged
uint32_t changedMs[5], suspectMs[5];
bool     bounceSent[5] = {true, true, true, true, true};
bool     lowSeen[4];                             // poll saw the lane LOW since the round started
uint16_t glitches[5];
uint8_t  stuckMask = 0;
uint32_t loopPrevUs = 0, loopMaxUs = 0;          // worst gap between loop() runs
uint8_t  step, nFlashes;
uint32_t lastStartMs, holdMs;
uint64_t seqStartUs, goUs;                       // the round's plan, in micros64() time
// ---------- Timing self-test ----------
bool     testRound = false, injected = false, injReleased = false;
uint64_t injAtUs;
uint32_t injUs;

// ---------- Interrupts: timestamp the very first contact ----------
// Judged against the scheduled lights-out, not when loop() got round to it.
void IRAM_ATTR laneHit(uint8_t i) {
  uint32_t now = micros();
  edgeCount[i]++;
  if (phase == IDLE || i >= activeLanes || pressed[i] || jumped[i]) return;
  pressUs[i] = now;
  if ((int32_t)(now - goUs32) < 0) jumped[i] = true;
  else pressed[i] = true;
}
void IRAM_ATTR isrLane0() { laneHit(0); }
void IRAM_ATTR isrLane1() { laneHit(1); }
void IRAM_ATTR isrLane2() { laneHit(2); }
void IRAM_ATTR isrLane3() { laneHit(3); }
void IRAM_ATTR isrStart() { edgeCount[4]++; startReq = true; }

// ---------- Helpers ----------
void emit(const char* fmt, ...) {
  char buf[200];
  va_list a; va_start(a, fmt); vsnprintf(buf, sizeof buf, fmt, a); va_end(a);
  ws.broadcastTXT(buf);
}

// micros64() time as milliseconds with microsecond decimals, e.g. "123456.789"
void fmtMs(char* b, uint64_t us) {
  snprintf(b, 20, "%lu.%03lu", (unsigned long)(us / 1000), (unsigned long)(us % 1000));
}

// When step s of the sequence happens: 2 steps per flash (on, off), then solid, then GO
uint64_t stepAtUs(uint8_t s) {
  uint32_t cyc = FLASH_ON_MS + FLASH_OFF_MS;
  if (s < 2 * nFlashes) return seqStartUs + ((uint64_t)(s / 2) * cyc + (s % 2 ? FLASH_ON_MS : 0)) * 1000;
  if (s == 2 * nFlashes) return seqStartUs + (uint64_t)nFlashes * cyc * 1000;
  return goUs;
}

// Test rounds: no flashes, short hold, one lane that the ESP presses itself
void startRound(bool test = false) {
  if (phase != IDLE) return;
  // A lane that's already LOW would never fire its press interrupt — refuse rather than run a dead lane
  char held[12] = "";
  for (uint8_t i = 0; i < (test ? 1 : cfgLanes); i++)
    if (digitalRead(LANE_PIN[i]) == LOW)
      snprintf(held + strlen(held), sizeof held - strlen(held), "%s%u", held[0] ? "," : "", i);
  if (held[0]) { emit("{\"ev\":\"refused\",\"stuck\":[%s],\"test\":%u}", held, test); return; }
  testRound = test;
  activeLanes = test ? 1 : cfgLanes;
  for (uint8_t i = 0; i < 4; i++) { pressed[i] = jumped[i] = false; reported[i] = okLane[i] = lowSeen[i] = false; }
  roundId++;
  corrKnown = false; corrUs = 0;
  uint16_t lo = test ? 500 : min(cfgMinHold, cfgMaxHold), hi = test ? 1000 : max(cfgMinHold, cfgMaxHold);
  holdMs = lo + (hi > lo ? RANDOM_REG32 % (hi - lo + 1) : 0);   // hardware random number
  nFlashes = test ? 0 : FLASHES;
  uint64_t now = micros64();
  seqStartUs = now + LEAD_MS * 1000ULL;
  goUs = seqStartUs + ((uint64_t)nFlashes * (FLASH_ON_MS + FLASH_OFF_MS) + holdMs) * 1000;
  goUs32 = (uint32_t)goUs;                      // set before phase so the ISR never sees a stale GO
  if (test) { injAtUs = goUs + (150 + RANDOM_REG32 % 251) * 1000ULL; injected = injReleased = false; }
  step = 0;
  lastStartMs = millis();
  phase = SEQ;
  char tn[20], ts[20];
  fmtMs(tn, now); fmtMs(ts, seqStartUs);
  emit("{\"ev\":\"start\",\"id\":%u,\"lanes\":%u,\"now\":%s,\"t\":%s,\"n\":%u,\"on\":%u,\"off\":%u,\"hold\":%lu,\"test\":%u}",
       roundId, activeLanes, tn, ts, nFlashes, FLASH_ON_MS, FLASH_OFF_MS, (unsigned long)holdMs, test);
}

void releaseInject() {
  if (injected && !injReleased) { pinMode(LANE_PIN[0], INPUT_PULLUP); injReleased = true; }
}

// Report a lane's press once it's confirmed and corrected.
// Confirmed: the poll saw the pin LOW (a real press), or 10 ms went by without
// that — an electrical glitch, flagged "noise". Corrected: the start screen has
// said how far its lights-out frame landed from GO, or CORR_WAIT_MS passed.
// Then it's judged here (ok / anticipated / jump) so every screen agrees.
void reportLane(uint8_t i, bool force) {
  if (reported[i] || (!jumped[i] && !pressed[i])) return;
  bool noise = !lowSeen[i];
  if (noise && !force && micros() - pressUs[i] < 10000) return;
  if (jumped[i]) {
    reported[i] = true;
    emit("{\"ev\":\"result\",\"lane\":%u,\"status\":\"jump\",\"noise\":%u}", i, noise);
    return;
  }
  unsigned long raw = pressUs[i] - goUs32;
  if (testRound) {   // self-test checks raw interrupt timing, so no correction
    reported[i] = true;
    emit("{\"ev\":\"result\",\"lane\":%u,\"status\":\"ok\",\"us\":%lu,\"inj\":%lu}", i, raw,
         (unsigned long)(injUs - goUs32));
    return;
  }
  if (!force && !corrKnown && micros64() < goUs + CORR_WAIT_MS * 1000ULL) return;
  reported[i] = true;
  int32_t rt = (int32_t)raw - corrUs;
  const char* st;
  if (rt < 0) st = "jump";                                       // pressed before the screen went dark
  else if (cfgAntiMs && rt < (int32_t)cfgAntiMs * 1000) st = "anti";
  else { st = "ok"; if (!noise) { okLane[i] = true; rtUs[i] = rt; } }   // noise can't win
  emit("{\"ev\":\"result\",\"lane\":%u,\"status\":\"%s\",\"us\":%ld,\"cor\":%u,\"noise\":%u}",
       i, st, (long)rt, corrKnown, noise);
}

// ---------- Wiring health: poll every pin once per loop() ----------
void pollPins() {
  uint32_t nowMs = millis();
  uint8_t mask = 0;
  for (uint8_t i = 0; i < 5; i++) {
    // Edges first, then the level: an edge landing in between is judged on the next pass
    noInterrupts(); uint16_t e = edgeCount[i]; interrupts();
    uint16_t de = e - lastEdges[i]; lastEdges[i] = e;
    uint8_t v = digitalRead(DBG_PIN[i]);
    if (v == LOW && i < 4) lowSeen[i] = true;
    bool settling = nowMs - changedMs[i] < DEBOUNCE_MS;

    if (v != dbgLevel[i] && !settling) {
      dbgLevel[i] = v; changedMs[i] = nowMs;
      if (v == LOW) { bounceEdges[i] = de + suspect[i]; bounceSent[i] = false; }  // those edges were this press starting
      suspect[i] = 0;
      if (debugMode) emit("{\"ev\":\"pin\",\"i\":%u,\"v\":%u,\"e\":%u}", i, v, v == LOW ? bounceEdges[i] : 0);
    } else if (settling || v == LOW) {
      if (dbgLevel[i] == LOW) bounceEdges[i] += de;  // bounce while pressed; release bounce is dropped
    } else if (de) {
      if (!suspect[i]) suspectMs[i] = nowMs;
      suspect[i] += de;
    }

    // Edges while the pin stayed HIGH: glitch. If another pin changed at the same moment, it's crosstalk.
    if (suspect[i] && nowMs - suspectMs[i] >= SETTLE_MS) {
      glitches[i] += suspect[i];
      if (debugMode)
        for (uint8_t k = 0; k < 5; k++)
          if (k != i && (uint32_t)abs((int32_t)(suspectMs[i] - changedMs[k])) <= XT_MS)
            emit("{\"ev\":\"xt\",\"from\":%u,\"to\":%u}", k, i);
      suspect[i] = 0;
    }
    if (!bounceSent[i] && !settling) {
      bounceSent[i] = true;
      if (debugMode) emit("{\"ev\":\"bnc\",\"i\":%u,\"e\":%u}", i, bounceEdges[i]);
    }
    if (dbgLevel[i] == LOW && nowMs - changedMs[i] >= STUCK_MS) mask |= 1 << i;
  }
  // Only judged between rounds: holding a button after pressing it mid-round is normal
  if (phase == IDLE && mask != stuckMask) {
    stuckMask = mask;
    emit("{\"ev\":\"stuck\",\"m\":%u}", mask);
  }
}

void finishRound() {
  releaseInject();
  for (uint8_t i = 0; i < activeLanes; i++) {
    reportLane(i, true);
    if (!reported[i]) { reported[i] = true; emit("{\"ev\":\"result\",\"lane\":%u,\"status\":\"none\"}", i); }
  }
  // Winner: fastest valid time (ties are a dead heat). Needs 2+ lanes.
  uint8_t win = 0;
  if (!testRound && activeLanes > 1) {
    int32_t best = INT32_MAX;
    for (uint8_t i = 0; i < activeLanes; i++) if (okLane[i] && rtUs[i] < best) best = rtUs[i];
    for (uint8_t i = 0; i < activeLanes; i++) if (okLane[i] && rtUs[i] == best) win |= 1 << i;
  }
  phase = IDLE;
  digitalWrite(LED_PIN, LED_OFF);
  emit("{\"ev\":\"done\",\"win\":%u}", win);
}

void abortRound() {
  releaseInject();
  phase = IDLE;
  digitalWrite(LED_PIN, LED_OFF);
  emit("{\"ev\":\"aborted\"}");
}

// ---------- WebSocket commands from the page ----------
//   "sync <n>" | "start" | "selftest" | "abort" | "shown <roundId> <offsetUs>"
//   "cfg <lanes> <minHoldMs> <maxHoldMs> [antiMs]"
//   test mode: "debug 0|1" (heartbeat) | "led 0|1" | "info" | "dreset"
void onWs(uint8_t num, WStype_t type, uint8_t* payload, size_t len) {
  if (type == WStype_CONNECTED) {
    char b[64];
    snprintf(b, sizeof b, "{\"ev\":\"hello\",\"busy\":%s,\"stuck\":%u}", phase == IDLE ? "false" : "true", stuckMask);
    ws.sendTXT(num, b);
  } else if (type == WStype_TEXT) {
    char m[64];
    size_t n = min(len, sizeof m - 1);
    memcpy(m, payload, n); m[n] = 0;
    if (!strncmp(m, "sync", 4)) {      // clock sync: answer at once with our time
      char t[20], b[64];
      fmtMs(t, micros64());
      snprintf(b, sizeof b, "{\"ev\":\"sync\",\"n\":%lu,\"t\":%s}", strtoul(m + 4, nullptr, 10), t);
      ws.sendTXT(num, b);
    }
    else if (!strncmp(m, "start", 5))    startRound();
    else if (!strncmp(m, "selftest", 8)) startRound(true);
    else if (!strncmp(m, "abort", 5)) abortRound();
    else if (!strncmp(m, "shown", 5)) {   // start screen: its lights-out frame vs the scheduled GO
      char* e;
      unsigned long id = strtoul(m + 5, &e, 10);
      long off = strtol(e, nullptr, 10);
      if (phase != IDLE && !testRound && !corrKnown && id == roundId) {
        corrUs = constrain(off, -100000L, 300000L);
        corrKnown = true;
      }
    }
        else if (!strncmp(m, "debug", 5)) {
      bool on = (m[6] == '1');
      dbgLastMs = millis();
      if (on && !debugMode) loopMaxUs = 0;
      if (!on && debugMode && phase == IDLE) digitalWrite(LED_PIN, LED_OFF);
      debugMode = on;
      if (on) emit("{\"ev\":\"pins\",\"v\":[%u,%u,%u,%u,%u]}",
                   dbgLevel[0], dbgLevel[1], dbgLevel[2], dbgLevel[3], dbgLevel[4]);
    }
    else if (!strncmp(m, "dreset", 6)) {
      for (uint8_t i = 0; i < 5; i++) glitches[i] = 0;
      loopMaxUs = 0;
    }
    else if (!strncmp(m, "info", 4)) {   // board health, once per test-panel open
      bool ap = WiFi.getMode() == WIFI_AP;
      char b[220];
      snprintf(b, sizeof b,
               "{\"ev\":\"info\",\"rst\":\"%s\",\"rr\":%u,\"build\":\"" __DATE__ " " __TIME__ "\",\"ip\":\"%s\","
               "\"ap\":%u,\"chip\":\"%06X\",\"flash\":%u,\"core\":\"%s\"}",
               ESP.getResetReason().c_str(), (unsigned)ESP.getResetInfoPtr()->reason,
               (ap ? WiFi.softAPIP() : WiFi.localIP()).toString().c_str(), ap,
               ESP.getChipId(), ESP.getFlashChipRealSize() / 1024, ESP.getCoreVersion().c_str());
      ws.sendTXT(num, b);
    }
    else if (!strncmp(m, "led", 3)) {
      if (phase == IDLE) digitalWrite(LED_PIN, m[4] == '1' ? LED_ON : LED_OFF);
    }
    else if (!strncmp(m, "cfg", 3)) {
      unsigned a, b, c, d;
      int got = sscanf(m + 3, "%u %u %u %u", &a, &b, &c, &d);
      if (got >= 3) {
        cfgLanes   = constrain(a, 1u, 4u);
        cfgMinHold = constrain(b, 300u, 10000u);
        cfgMaxHold = constrain(c, 300u, 10000u);
      }
      if (got == 4) cfgAntiMs = min(d, 1000u);
    }
  }
}

void setup() {
  Serial.begin(115200);
  pinMode(LED_PIN, OUTPUT);
  digitalWrite(LED_PIN, LED_OFF);
  for (uint8_t i = 0; i < 4; i++) pinMode(LANE_PIN[i], INPUT_PULLUP);
  pinMode(START_PIN, INPUT_PULLUP);
  attachInterrupt(digitalPinToInterrupt(LANE_PIN[0]), isrLane0, FALLING);
  attachInterrupt(digitalPinToInterrupt(LANE_PIN[1]), isrLane1, FALLING);
  attachInterrupt(digitalPinToInterrupt(LANE_PIN[2]), isrLane2, FALLING);
  attachInterrupt(digitalPinToInterrupt(LANE_PIN[3]), isrLane3, FALLING);
  attachInterrupt(digitalPinToInterrupt(START_PIN),   isrStart, FALLING);

  // Join WiFi (LED blinks fast while connecting)
  WiFi.mode(WIFI_STA);
  WiFi.hostname(HOSTNAME);
  WiFi.begin(WIFI_SSID, WIFI_PASS);
  uint32_t t = millis();
  while (WiFi.status() != WL_CONNECTED && millis() - t < 15000) {
    digitalWrite(LED_PIN, !digitalRead(LED_PIN));
    delay(100);
  }
  digitalWrite(LED_PIN, LED_OFF);

  Serial.println();
  if (WiFi.status() == WL_CONNECTED) {
    Serial.print("Connected. Open http://"); Serial.print(HOSTNAME);
    Serial.print(".local  or  http://"); Serial.println(WiFi.localIP());
  } else {
    WiFi.mode(WIFI_AP);
    WiFi.softAP(AP_SSID, AP_PASS);
    Serial.print("Hotspot mode. Join \""); Serial.print(AP_SSID);
    Serial.println("\" and open http://192.168.4.1");
  }
  WiFi.setSleepMode(WIFI_NONE_SLEEP);   // keeps WiFi responsive

  MDNS.begin(HOSTNAME);
  MDNS.addService("http", "tcp", 80);
  server.on("/", []() { server.send_P(200, "text/html", PAGE); });
  server.begin();
  ws.begin();
  ws.onEvent(onWs);
}

void loop() {
  uint32_t nowUs = micros();                      // worst gap between runs: how long WiFi etc. can stall us
  if (loopPrevUs && nowUs - loopPrevUs > loopMaxUs) loopMaxUs = nowUs - loopPrevUs;
  loopPrevUs = nowUs;

  ws.loop();
  server.handleClient();
  MDNS.update();
  pollPins();

  // Physical start button (1 s lockout also absorbs switch bounce)
  if (startReq) {
    startReq = false;
    if (!debugMode && phase == IDLE && millis() - lastStartMs > 1000) startRound();
  }

  // LED sequence: steps 0-9 = five on/off flashes, 10 = solid, 11 = lights out
  // The onboard LED follows the same plan the page draws. It's only a mirror:
  // times are measured from the scheduled goUs, not from when this runs.
  if (phase == SEQ && micros64() >= stepAtUs(step)) {
    if (step < 2 * nFlashes) {
      digitalWrite(LED_PIN, step % 2 == 0 ? LED_ON : LED_OFF);
      step++;
    } else if (step == 2 * nFlashes) {
      digitalWrite(LED_PIN, LED_ON);
      step++;
    } else {
      digitalWrite(LED_PIN, LED_OFF);
      phase = GO;
    }
  }

  // Self-test: pull lane 1's input low ourselves, noting exactly when
  if (testRound && phase == GO) {
    if (!injected && micros64() >= injAtUs) {
      injUs = micros();
      digitalWrite(LANE_PIN[0], LOW);   // latch low first so the pin never drives high into a pressed button
      pinMode(LANE_PIN[0], OUTPUT);     // keeps the attached interrupt
      injected = true;
    } else if (injected && micros64() >= injAtUs + 20000) releaseInject();
  }

  // Report results as they happen
  if (phase != IDLE) {
    for (uint8_t i = 0; i < activeLanes; i++) reportLane(i, false);
    bool all = true;
    for (uint8_t i = 0; i < activeLanes; i++) if (!reported[i]) all = false;
    if (all || (phase == GO && micros64() >= goUs + TIMEOUT_MS * 1000ULL)) finishRound();
  }
    // ---------- Button test mode ----------
  if (debugMode) {
    if (millis() - dbgLastMs > 10000) {           // page closed without saying so
      debugMode = false;
      if (phase == IDLE) digitalWrite(LED_PIN, LED_OFF);
    } else if (millis() - dbgStatusMs >= 1000) {   // pin changes are sent by pollPins()
      dbgStatusMs = millis();
      emit("{\"ev\":\"stat\",\"rssi\":%d,\"up\":%lu,\"heap\":%u,\"frag\":%u,\"ap\":%u,\"loop\":%lu,\"cl\":%u,"
           "\"g\":[%u,%u,%u,%u,%u]}",
           WiFi.RSSI(), millis() / 1000, ESP.getFreeHeap(), ESP.getHeapFragmentation(), WiFi.getMode() == WIFI_AP,
           (unsigned long)loopMaxUs, ws.connectedClients(),
           glitches[0], glitches[1], glitches[2], glitches[3], glitches[4]);
      loopMaxUs = 0;
    }
  }
}
