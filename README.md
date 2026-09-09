# STEM Racing Head-to-Head Reaction Timer — Prototype V1

An ultra-low-latency, millisecond-accurate two-lane reaction timer: an
F1-style 5-LED countdown tree, false-start detection, and a real-time
WebSocket telemetry dashboard.

## Repository layout

```
firmware/reaction_timer/
  reaction_timer.ino   Main sketch: network, WebSocket server, race state machine
  config.h              Every pin assignment and timing constant, in one place
  web_dashboard.h        Single-file dashboard (HTML/CSS/JS) embedded in flash
docs/
  WIRING.md              Full pinout, LED/OLED/button wiring, RJ45 trigger schematic
  CALIBRATION.md          Latency verification and test plan
```

## 1. System architecture

```
                +-------------------------------------------------+
                |         Waveshare ESP32-P4-Nano-ETH              |
                |                                                   |
  Lane 1 -------|--> GPIO ISR --+                                   |
  handheld      |               |                                   |
  (RJ45/Cat5)   |               v                                   |
                |        Race state machine (loop(), micros())     |
  Lane 2 -------|--> GPIO ISR --+        |                          |
  handheld      |                        v                          |
  (RJ45/Cat5)   |          5x GPIO -> LED start-light tree          |
                |          I2C -> OLED (IP / status)                |
                |                        |                          |
                |             WebServer :80  (serves dashboard)     |
                |             WebSocketsServer :81 (telemetry)      |
                +-------------------|-------------------------------+
                                     | PoE (power + wired LAN)
                                     v
                          Ethernet switch / PoE injector
                                     |
                     +---------------+----------------+
                     |                                |
             Laptop leaderboard page          Phone control page
             (same dashboard, wired/Wi-Fi)     (same dashboard, Wi-Fi)
```

One firmware image, one embedded web page, both roles (leaderboard display
and remote control) are the same responsive page — it just lays out
differently on a laptop vs. a phone. Every client connects to the same
`ws://<device-ip>:81/` WebSocket and receives identical state; the "Start"
and "Reset" buttons work from any connected device, matching the "phone
remote control" requirement without a second codebase.

**Goal for V1:** validate the full pipeline end-to-end — real trigger
buttons over RJ45, real LED light tree, real WebSocket telemetry, real
false-start detection — on the bench, before moving to enclosure/finishing
work in V2.

## 2. Pinout

See [`docs/WIRING.md`](docs/WIRING.md) for the full table and the RJ45
noise-filter schematic. GPIO numbers are taken directly from the board's
GPIO header pinout table and avoid the JTAG-muxed pins (GPIO19-22) and the
strapping pins (GPIO34-36); the onboard Ethernet PHY and Wi-Fi co-processor
use separate, non-header GPIOs entirely, so they don't factor into this
table. The one number still worth double-checking before flashing is the
`ETH_PHY_*` block in `config.h` (the PHY's own internal init params, not
header pins) against Waveshare's official `ETH.begin()` demo.

All pin numbers live in `firmware/reaction_timer/config.h` — change them
there once rather than hunting through the sketch.

## 3. Firmware

`firmware/reaction_timer/reaction_timer.ino` (+ `config.h` +
`web_dashboard.h`). Open the `.ino` in Arduino IDE (the other two files
will load alongside it as tabs).

**Install first (Library Manager):**
- ArduinoJson (v7.x)
- WebSockets by Markus Sattler (Links2004/arduinoWebSockets)
- Adafruit SSD1306 + Adafruit GFX Library
- An ESP32 board package with ESP32-P4 support (arduino-esp32 core ≥ 3.x)

**Before flashing:** open `config.h` and set `USE_ETHERNET` (1 for PoE
Ethernet, 0 for Wi-Fi), fill in the `ETH_PHY_*` constants from Waveshare's
official ETH demo for this exact board (do not guess these — wrong RMII
pins just won't link), or your Wi-Fi SSID/password if using the fallback
path.

The sketch serves the dashboard from flash directly (no filesystem upload
step needed for V1) and starts a WebSocket server on port 81 for telemetry.

## 4. Web dashboard

`firmware/reaction_timer/web_dashboard.h` contains the entire single-page
dashboard (F1 light tree, live lane times, winner badge, false-start alert,
Start/Reset controls, race history table) as one embedded HTML/CSS/JS file,
served at `http://<device-ip>/`. It includes a **Test Mode** toggle that
lets you drive the state machine with the F/J keys before any hardware
buttons are wired — the direct successor to the earlier PeerJS/keyboard
simulator, now talking to the real firmware instead of a simulated peer.

## 5. Bring-up order

1. **Breadboard + noise suppression bench test** — wire just the two
   buttons (with their pull-up + 100nF filter) on a breadboard; the 5 LEDs
   can stay shelved (`USE_PHYSICAL_LEDS 0` in `config.h`, the default) since
   the OLED renders the same light tree until you're ready to wire them.
   Flash the firmware, use the dashboard's Test Mode (F/J keys) to drive a
   few full race cycles before touching real trigger hardware. Confirms the
   state machine, light sequencing, and WebSocket telemetry all work in
   isolation.
2. **Flash + network verification** — confirm PoE Ethernet (or Wi-Fi
   fallback) comes up, the OLED shows the correct IP, and
   `http://<device-ip>/` / `http://reactiontimer.local/` loads the
   dashboard from another device on the LAN.
3. **Trigger wiring + RJ45 assembly** — build the two handheld controllers
   and patch cables per `docs/WIRING.md`, swap Test Mode off, and confirm
   real button presses register with the same behavior as the simulated
   ones.
4. **Calibration + latency verification** — work through
   [`docs/CALIBRATION.md`](docs/CALIBRATION.md): a relay-driven bench test
   for on-device timing accuracy, a false-start test matrix, and a
   WebSocket round-trip measurement.

## 6. Race protocol (for reference)

WebSocket messages, JSON, both directions:

**Server → client**
- `{"type":"state","state":"IDLE|SEQUENCE|HOLD|GO|FALSE_START|FINISHED","lights":[0,1,1,0,0]}`
- `{"type":"false_start","lane":1}`
- `{"type":"result","lane1":{"falseStart":false,"dnf":false,"reactionMs":231.4},"lane2":{...},"winner":1}`
- `{"type":"history","races":[ ... ]}`

**Client → server**
- `{"cmd":"start"}`
- `{"cmd":"reset"}`
- `{"cmd":"simulate","lane":1}` — Test Mode only, mimics a physical press

## Known V1 scope limits

- Race history is in RAM only (resets on reboot) — fine for a bench
  prototype, flag for V2 if persistent logging is wanted.
- No authentication on the WebSocket/HTTP endpoints — acceptable on an
  isolated LAN for a competition booth, not for exposing this to the open
  internet.
