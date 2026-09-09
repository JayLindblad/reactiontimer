// reaction_timer.ino
// STEM Racing Head-to-Head Reaction Timer - Prototype V1
// Target: Waveshare ESP32-P4-Nano-ETH
//
// Libraries required (Arduino Library Manager):
//   - ArduinoJson        (v7.x)          by Benoit Blanchon
//   - WebSockets         (v2.4+)         by Markus Sattler (Links2004/arduinoWebSockets)
//   - Adafruit SSD1306 + Adafruit GFX Library
//   - ESP32 board package with ESP32-P4 support (arduino-esp32 core >= 3.x)
//
// See docs/WIRING.md for the full pinout and RJ45 trigger wiring, and
// config.h for every pin/timing constant used below.
//
// Architecture:
//   - WebServer (port 80) serves the single-file dashboard from flash.
//   - WebSocketsServer (port 81) pushes state/telemetry to all clients and
//     accepts {"cmd":"start"|"reset"|"simulate"} control messages.
//   - A non-blocking state machine in loop() drives the F1 light sequence
//     and reaction capture using micros() for sub-millisecond accuracy.
//   - Button edges are captured in ISRs (timestamped with micros()) and
//     processed in loop(), keeping ISRs minimal.

#include <Arduino.h>
#include <Wire.h>
#include <WebServer.h>
#include <WebSocketsServer.h>
#include <ArduinoJson.h>
#include <ESPmDNS.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>

#if USE_ETHERNET
  #include <ETH.h>
#else
  #include <WiFi.h>
#endif

#include "config.h"
#include "web_dashboard.h"

// ---------------------------------------------------------------------
// Globals
// ---------------------------------------------------------------------

WebServer server(80);
WebSocketsServer webSocket(81);
Adafruit_SSD1306 oled(OLED_WIDTH, OLED_HEIGHT, &Wire, -1);

enum RaceState : uint8_t { ST_IDLE, ST_SEQUENCE, ST_HOLD, ST_GO, ST_FALSE_START, ST_FINISHED };
RaceState raceState = ST_IDLE;

bool lightsOn[NUM_LEDS] = {false, false, false, false, false};
uint8_t nextLedIndex = 0;
uint32_t sequenceStartMs = 0;
uint32_t holdDurationMs = 0;
uint32_t holdStartMs = 0;
uint64_t goTimeUs = 0;
uint8_t falseStartLane = 0;

volatile bool lane1Pressed = false;
volatile bool lane2Pressed = false;
volatile uint32_t lane1PressUs = 0;
volatile uint32_t lane2PressUs = 0;
volatile uint32_t lastLane1IsrUs = 0;
volatile uint32_t lastLane2IsrUs = 0;

struct RaceResult {
  bool lane1FalseStart, lane2FalseStart;
  bool lane1Dnf, lane2Dnf;
  float lane1Ms, lane2Ms;
  uint8_t winner; // 0 = none, 1 or 2
};
RaceResult history[MAX_HISTORY];
uint8_t historyCount = 0;

bool ethConnected = false;
String ipAddressStr = "----";

// ---------------------------------------------------------------------
// ISRs - keep these as short as possible
// ---------------------------------------------------------------------

void IRAM_ATTR lane1ISR() {
  uint32_t now = micros();
  if (now - lastLane1IsrUs < DEBOUNCE_US) return;
  lastLane1IsrUs = now;
  if (!lane1Pressed) {
    lane1Pressed = true;
    lane1PressUs = now;
  }
}

void IRAM_ATTR lane2ISR() {
  uint32_t now = micros();
  if (now - lastLane2IsrUs < DEBOUNCE_US) return;
  lastLane2IsrUs = now;
  if (!lane2Pressed) {
    lane2Pressed = true;
    lane2PressUs = now;
  }
}

// ---------------------------------------------------------------------
// LED helpers
// ---------------------------------------------------------------------

void setLight(uint8_t i, bool on) {
  lightsOn[i] = on;
#if USE_PHYSICAL_LEDS
  digitalWrite(LED_PINS[i], on ? HIGH : LOW);
#endif
  // lightsOn[] is updated either way, so the OLED light tree (drawLightTree())
  // and the WebSocket "lights" telemetry stay accurate even with LEDs shelved.
}

void allLightsOff() {
  for (uint8_t i = 0; i < NUM_LEDS; i++) setLight(i, false);
}

void allLightsOn() {
  for (uint8_t i = 0; i < NUM_LEDS; i++) setLight(i, true);
}

// ---------------------------------------------------------------------
// WebSocket broadcast helpers
// ---------------------------------------------------------------------

const char *stateName(RaceState s) {
  switch (s) {
    case ST_IDLE: return "IDLE";
    case ST_SEQUENCE: return "SEQUENCE";
    case ST_HOLD: return "HOLD";
    case ST_GO: return "GO";
    case ST_FALSE_START: return "FALSE_START";
    case ST_FINISHED: return "FINISHED";
  }
  return "UNKNOWN";
}

void broadcastState() {
  JsonDocument doc;
  doc["type"] = "state";
  doc["state"] = stateName(raceState);
  JsonArray lights = doc["lights"].to<JsonArray>();
  for (uint8_t i = 0; i < NUM_LEDS; i++) lights.add(lightsOn[i] ? 1 : 0);
  String out;
  serializeJson(doc, out);
  webSocket.broadcastTXT(out);
}

void broadcastFalseStart(uint8_t lane) {
  JsonDocument doc;
  doc["type"] = "false_start";
  doc["lane"] = lane;
  String out;
  serializeJson(doc, out);
  webSocket.broadcastTXT(out);
}

void broadcastResult() {
  if (historyCount == 0) return;
  RaceResult &r = history[historyCount - 1];
  JsonDocument doc;
  doc["type"] = "result";
  doc["winner"] = r.winner;

  JsonObject l1 = doc["lane1"].to<JsonObject>();
  l1["falseStart"] = r.lane1FalseStart;
  l1["dnf"] = r.lane1Dnf;
  l1["reactionMs"] = r.lane1Ms;

  JsonObject l2 = doc["lane2"].to<JsonObject>();
  l2["falseStart"] = r.lane2FalseStart;
  l2["dnf"] = r.lane2Dnf;
  l2["reactionMs"] = r.lane2Ms;

  String out;
  serializeJson(doc, out);
  webSocket.broadcastTXT(out);
}

void broadcastHistory() {
  JsonDocument doc;
  doc["type"] = "history";
  JsonArray races = doc["races"].to<JsonArray>();
  for (uint8_t i = 0; i < historyCount; i++) {
    RaceResult &r = history[i];
    JsonObject race = races.add<JsonObject>();
    race["winner"] = r.winner;
    JsonObject l1 = race["lane1"].to<JsonObject>();
    l1["falseStart"] = r.lane1FalseStart;
    l1["dnf"] = r.lane1Dnf;
    l1["reactionMs"] = r.lane1Ms;
    JsonObject l2 = race["lane2"].to<JsonObject>();
    l2["falseStart"] = r.lane2FalseStart;
    l2["dnf"] = r.lane2Dnf;
    l2["reactionMs"] = r.lane2Ms;
  }
  String out;
  serializeJson(doc, out);
  webSocket.broadcastTXT(out);
}

// ---------------------------------------------------------------------
// Race state machine
// ---------------------------------------------------------------------

void clearLaneFlags() {
  noInterrupts();
  lane1Pressed = false;
  lane2Pressed = false;
  interrupts();
}

void startSequence() {
  if (raceState != ST_IDLE && raceState != ST_FINISHED && raceState != ST_FALSE_START) return;
  clearLaneFlags();
  allLightsOff();
  nextLedIndex = 0;
  sequenceStartMs = millis();
  raceState = ST_SEQUENCE;
  broadcastState();
}

void resetRace() {
  clearLaneFlags();
  allLightsOff();
  nextLedIndex = 0;
  raceState = ST_IDLE;
  broadcastState();
}

void pushResult(const RaceResult &r) {
  if (historyCount < MAX_HISTORY) {
    history[historyCount++] = r;
  } else {
    // drop oldest, shift left
    for (uint8_t i = 1; i < MAX_HISTORY; i++) history[i - 1] = history[i];
    history[MAX_HISTORY - 1] = r;
  }
}

void triggerFalseStart(uint8_t lane) {
  falseStartLane = lane;
  allLightsOn(); // solid red = false start indicator
  raceState = ST_FALSE_START;
  broadcastState();
  broadcastFalseStart(lane);

  RaceResult r = {};
  r.lane1FalseStart = (lane == 1);
  r.lane2FalseStart = (lane == 2);
  r.lane1Dnf = (lane != 1);
  r.lane2Dnf = (lane != 2);
  r.lane1Ms = 0;
  r.lane2Ms = 0;
  r.winner = 0;
  pushResult(r);
  broadcastHistory();

  clearLaneFlags();
}

void finishRace() {
  RaceResult r = {};
  r.lane1FalseStart = false;
  r.lane2FalseStart = false;

  if (lane1Pressed) {
    r.lane1Dnf = false;
    r.lane1Ms = (lane1PressUs - goTimeUs) / 1000.0f;
  } else {
    r.lane1Dnf = true;
    r.lane1Ms = 0;
  }

  if (lane2Pressed) {
    r.lane2Dnf = false;
    r.lane2Ms = (lane2PressUs - goTimeUs) / 1000.0f;
  } else {
    r.lane2Dnf = true;
    r.lane2Ms = 0;
  }

  if (!r.lane1Dnf && !r.lane2Dnf) {
    r.winner = (r.lane1Ms <= r.lane2Ms) ? 1 : 2;
  } else if (!r.lane1Dnf) {
    r.winner = 1;
  } else if (!r.lane2Dnf) {
    r.winner = 2;
  } else {
    r.winner = 0;
  }

  pushResult(r);
  raceState = ST_FINISHED;
  allLightsOff();
  broadcastState();
  broadcastResult();
  broadcastHistory();
}

void updateSequence() {
  uint32_t elapsed = millis() - sequenceStartMs;
  uint32_t targetIndex = elapsed / SEQUENCE_INTERVAL_MS;
  bool changed = false;
  while (nextLedIndex < NUM_LEDS && nextLedIndex <= targetIndex) {
    setLight(nextLedIndex, true);
    nextLedIndex++;
    changed = true;
  }
  if (changed) broadcastState();

  if (nextLedIndex >= NUM_LEDS) {
    holdDurationMs = MIN_HOLD_MS + (esp_random() % (MAX_HOLD_MS - MIN_HOLD_MS + 1));
    holdStartMs = millis();
    raceState = ST_HOLD;
  }
}

void updateHold() {
  if (millis() - holdStartMs >= holdDurationMs) {
    allLightsOff();
    goTimeUs = micros();
    raceState = ST_GO;
    broadcastState();
  }
}

void updateGo() {
  bool bothIn = lane1Pressed && lane2Pressed;
  bool timedOut = (micros() - goTimeUs) > (uint64_t)REACTION_TIMEOUT_MS * 1000ULL;
  if (bothIn || timedOut) {
    finishRace();
  }
}

// ---------------------------------------------------------------------
// WebSocket command handling
// ---------------------------------------------------------------------

void onWsEvent(uint8_t num, WStype_t type, uint8_t *payload, size_t length) {
  if (type != WStype_TEXT) {
    if (type == WStype_CONNECTED) {
      broadcastState();
      broadcastHistory();
    }
    return;
  }

  JsonDocument doc;
  DeserializationError err = deserializeJson(doc, payload, length);
  if (err) return;

  const char *cmd = doc["cmd"] | "";
  if (strcmp(cmd, "start") == 0) {
    startSequence();
  } else if (strcmp(cmd, "reset") == 0) {
    resetRace();
  } else if (strcmp(cmd, "simulate") == 0) {
    int lane = doc["lane"] | 0;
    uint32_t now = micros();
    if (lane == 1 && !lane1Pressed) { lane1Pressed = true; lane1PressUs = now; }
    if (lane == 2 && !lane2Pressed) { lane2Pressed = true; lane2PressUs = now; }
  }
}

// ---------------------------------------------------------------------
// HTTP handlers
// ---------------------------------------------------------------------

void handleRoot() {
  server.send_P(200, "text/html", INDEX_HTML);
}

// ---------------------------------------------------------------------
// OLED status
// ---------------------------------------------------------------------

// Renders the same 5-light build-up/hold/GO tree the LEDs would show,
// straight from lightsOn[] -- used in place of the physical LEDs while
// USE_PHYSICAL_LEDS is 0. Filled circle = lit, outline = off.
void drawLightTree(int16_t y) {
  const int16_t d = 14, gap = 6, r = d / 2;
  const int16_t totalW = NUM_LEDS * d + (NUM_LEDS - 1) * gap;
  int16_t x = (OLED_WIDTH - totalW) / 2;
  for (uint8_t i = 0; i < NUM_LEDS; i++) {
    int16_t cx = x + i * (d + gap) + r;
    int16_t cy = y + r;
    if (lightsOn[i]) oled.fillCircle(cx, cy, r, SSD1306_WHITE);
    else oled.drawCircle(cx, cy, r, SSD1306_WHITE);
  }
}

void updateOled() {
  oled.clearDisplay();
  drawLightTree(2); // occupies roughly y:2-16

  oled.setTextSize(1);
  oled.setTextColor(SSD1306_WHITE);

  oled.setCursor(0, 20);
  oled.print("St:");
  oled.println(stateName(raceState));

  oled.setCursor(0, 30);
  oled.print("IP:");
  oled.println(ipAddressStr);

  if (raceState == ST_FALSE_START) {
    oled.setCursor(0, 42);
    oled.print("FALSE START - L");
    oled.println(falseStartLane);
  } else if (historyCount > 0) {
    RaceResult &r = history[historyCount - 1];
    oled.setCursor(0, 42);
    oled.print("Win: ");
    oled.println(r.winner == 0 ? "-" : (r.winner == 1 ? "Lane 1" : "Lane 2"));

    if (raceState == ST_FINISHED) {
      char l1[12], l2[12];
      if (!r.lane1Dnf && !r.lane1FalseStart) snprintf(l1, sizeof(l1), "%.0fms", r.lane1Ms);
      else snprintf(l1, sizeof(l1), "--");
      if (!r.lane2Dnf && !r.lane2FalseStart) snprintf(l2, sizeof(l2), "%.0fms", r.lane2Ms);
      else snprintf(l2, sizeof(l2), "--");
      oled.setCursor(0, 52);
      oled.print("L1:");
      oled.print(l1);
      oled.print("  L2:");
      oled.println(l2);
    }
  }

  oled.display();
}

// ---------------------------------------------------------------------
// Network bring-up
// ---------------------------------------------------------------------

#if USE_ETHERNET
void onEthEvent(WiFiEvent_t event) {
  switch (event) {
    case ARDUINO_EVENT_ETH_GOT_IP:
      ipAddressStr = ETH.localIP().toString();
      ethConnected = true;
      break;
    case ARDUINO_EVENT_ETH_DISCONNECTED:
      ethConnected = false;
      break;
    default: break;
  }
}
#endif

void setupNetwork() {
#if USE_ETHERNET
  WiFi.onEvent(onEthEvent);
  ETH.begin(ETH_PHY_TYPE_CFG, ETH_PHY_ADDR_CFG, ETH_PHY_MDC_CFG, ETH_PHY_MDIO_CFG,
            ETH_PHY_POWER_CFG, ETH_CLK_MODE_CFG);
  Serial.println("Waiting for Ethernet link...");
  uint32_t start = millis();
  while (!ethConnected && millis() - start < 10000) {
    delay(100);
  }
#else
  WiFi.mode(WIFI_STA);
  WiFi.begin(WIFI_SSID, WIFI_PASS);
  Serial.println("Connecting to Wi-Fi...");
  uint32_t start = millis();
  while (WiFi.status() != WL_CONNECTED && millis() - start < 15000) {
    delay(250);
    Serial.print(".");
  }
  Serial.println();
  ipAddressStr = WiFi.localIP().toString();
#endif
  Serial.print("IP address: ");
  Serial.println(ipAddressStr);

  if (MDNS.begin(MDNS_HOSTNAME)) {
    Serial.print("mDNS started: http://");
    Serial.print(MDNS_HOSTNAME);
    Serial.println(".local/");
  }
}

// ---------------------------------------------------------------------
// Setup / loop
// ---------------------------------------------------------------------

void setup() {
  Serial.begin(115200);

  pinMode(LANE1_BUTTON_PIN, INPUT); // external pull-up per docs/WIRING.md
  pinMode(LANE2_BUTTON_PIN, INPUT);
  attachInterrupt(digitalPinToInterrupt(LANE1_BUTTON_PIN), lane1ISR, FALLING);
  attachInterrupt(digitalPinToInterrupt(LANE2_BUTTON_PIN), lane2ISR, FALLING);

#if USE_PHYSICAL_LEDS
  for (uint8_t i = 0; i < NUM_LEDS; i++) pinMode(LED_PINS[i], OUTPUT);
#endif
  allLightsOff();

  Wire.begin(OLED_SDA_PIN, OLED_SCL_PIN);
  if (!oled.begin(SSD1306_SWITCHCAPVCC, OLED_I2C_ADDR)) {
    Serial.println("SSD1306 OLED not found - continuing without display");
  } else {
    oled.clearDisplay();
    oled.display();
  }

  setupNetwork();

  server.on("/", handleRoot);
  server.begin();

  webSocket.begin();
  webSocket.onEvent(onWsEvent);

  Serial.println("Reaction Timer ready.");
}

void loop() {
  server.handleClient();
  webSocket.loop();

  if (raceState == ST_SEQUENCE || raceState == ST_HOLD) {
    if (lane1Pressed) {
      triggerFalseStart(1);
    } else if (lane2Pressed) {
      triggerFalseStart(2);
    }
  }

  switch (raceState) {
    case ST_SEQUENCE: updateSequence(); break;
    case ST_HOLD: updateHold(); break;
    case ST_GO: updateGo(); break;
    default: break;
  }

  // With USE_PHYSICAL_LEDS off, the OLED *is* the light tree, so refresh it
  // fast enough that the lights-out GO signal isn't laggy on-screen. 250ms
  // is fine once real LEDs take over that job and the OLED is secondary.
  static uint32_t lastOledMs = 0;
  const uint32_t oledIntervalMs = USE_PHYSICAL_LEDS ? 250 : 30;
  if (millis() - lastOledMs > oledIntervalMs) {
    lastOledMs = millis();
    updateOled();
  }
}
