# Calibration & Latency Verification - Prototype V1

Goal: confirm that a reaction time the dashboard reports is actually the
driver's reaction time, and not firmware/network overhead pretending to be
part of it.

## 1. Sources of latency in the pipeline

| Stage                                   | Expected latency        | Notes |
|------------------------------------------|--------------------------|-------|
| Button contact bounce settle             | <5ms (hardware filtered) | RC filter in docs/WIRING.md |
| GPIO ISR to `micros()` timestamp         | ~1-2µs                   | Interrupt latency on ESP32 |
| State machine detects both lanes / timeout | one `loop()` iteration | loop() runs continuously, no `delay()` calls in the hot path |
| JSON serialize + WebSocket broadcast     | <5ms typically           | Measure this explicitly, see below |
| Browser render of updated numbers        | one animation frame (~16ms) | Not part of the *recorded* time, only the *displayed* time |

The number that matters for scoring is captured entirely on-device in
`goTimeUs` / `lane1PressUs` / `lane2PressUs` — network and browser latency
only affect how fast the result *appears* on screen, not the recorded value.
Still worth measuring end-to-end so you know your dashboard isn't lying to
spectators about "live."

## 2. Bench test: solenoid/relay press instead of a human

Human reaction time (150-300ms typical, sub-120ms is suspiciously fast) is
too noisy to validate millisecond accuracy against. Instead:

1. Wire a relay or transistor in parallel with each lane's button contacts.
2. Drive both relays from a second microcontroller (or a signal generator)
   that pulses them at a **known, fixed delay** after watching for the
   lights-out transition (e.g. photodiode on the light tree, or just tap
   `goTimeUs` over serial for the test rig).
3. Run 20-30 trials at a fixed programmed delay (e.g. exactly 200.00ms).
4. Reaction times reported by the dashboard should cluster tightly around
   your programmed delay. Standard deviation across trials tells you your
   system's own jitter floor — this is the number to report as "system
   accuracy," not a spec-sheet claim.

## 3. False start detection test

1. Start a race.
2. Press one lane's button during the light build-up phase (lights still
   turning on) — confirm immediate `FALSE_START` state, correct lane
   flagged, other lane untouched.
3. Repeat during the random hold (all 5 lit, before lights-out) — same
   expected result.
4. Press exactly at/after lights-out — confirm this is treated as a normal
   valid reaction, not a false start (there should be no window where a
   legitimate first-reflex press after GO is misclassified).

## 4. WebSocket round-trip measurement

From the browser console on the dashboard page:

```js
let t0;
const testWs = new WebSocket("ws://" + location.hostname + ":81/");
testWs.onopen = () => { t0 = performance.now(); testWs.send(JSON.stringify({cmd:"reset"})); };
testWs.onmessage = () => { console.log("round trip ms:", performance.now() - t0); testWs.close(); };
```

Run this several times on both Wi-Fi and wired Ethernet clients on the same
LAN and compare — this quantifies the "<10ms real-time updates" target from
the network side of the pipeline, separate from the on-device capture
accuracy measured in step 2.

## 5. Two-driver dry run checklist

- [ ] Both lanes wired, both buttons independently trigger a false start
      when pressed early.
- [ ] Winner badge and reaction times both correctly match the faster of
      two non-DNF, non-false-start lanes.
- [ ] DNF triggers correctly if one lane never presses within
      `REACTION_TIMEOUT_MS`.
- [ ] Race history table accumulates multiple races without a page reload.
- [ ] Reset mid-sequence (before lights-out) returns cleanly to IDLE with
      all lights off.
- [ ] OLED shows current IP and state through a full race cycle.
