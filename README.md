# Reaction Trainer

An F1-style head-to-head reaction timer for 1–4 lanes, running on a
**WEMOS D1 mini (ESP8266)**. The ESP owns the clock: it schedules the start
sequence, times every press by interrupt, decides who won, and serves a web
dashboard at **http://reaction.local**.

- `reactiontimer.ino` — the sketch: WiFi, WebSocket server, round timing, wiring health
- `page.h` — the dashboard (HTML/CSS/JS), served from flash

## How timing works

1. Each round the ESP picks the whole start sequence (5 flashes → solid → out)
   in its own microsecond clock and announces it ahead of time.
2. Every open page keeps its clock synced to the ESP and draws the lights on
   that schedule.
3. The page marked **This device is the start screen** (Settings) reports how
   far its lights-out frame landed from the ESP's GO, plus its screen lag.
4. The ESP corrects each press by that, judges it (valid / anticipated / jump),
   picks the winner, and broadcasts the result, so every screen agrees.

Presses that the ESP never sees held down are flagged as possible electrical
noise and kept out of the history. A lane held low (stuck button, shorted
cable) blocks the round from starting.

## Setup (Arduino IDE)

1. Boards Manager: install **esp8266** by ESP8266 Community
2. Library Manager: install **WebSockets** by Markus Sattler
3. Board: **LOLIN(WEMOS) D1 R2 & mini**
4. Put your WiFi name/password at the top of `reactiontimer.ino`, upload,
   and open the Serial Monitor at 115200.

If it can't join your WiFi within 15 s it starts its own hotspot
(`ReactionTrainer` / `racestart`, then open http://192.168.4.1).

Command line: copy both files into a folder named `reactiontimer/`, then
`arduino-cli compile --fqbn esp8266:esp8266:d1_mini reactiontimer`.

## Wiring

Each button: one leg to the pin, the other to GND (internal pull-ups are used).

| Input        | GPIO | D1 mini pin |
|--------------|------|-------------|
| Lane 1       | 5    | D1          |
| Lane 2       | 4    | D2          |
| Lane 3       | 12   | D6          |
| Lane 4       | 13   | D7          |
| Start button | 14   | D5          |
| Onboard LED  | 2    | D4 (nothing to wire) |

## Checking the hardware

Open **TEST** on the dashboard:

- **Button tiles** — press each button; shows bounce, noise and stuck pins.
- **Slot check** — walks through each slot to catch swapped or dead cables.
- **Board health** — WiFi, clock sync, memory, loop stalls, last reset reason.
- **Timing self-test** — 10 rounds where the ESP presses lane 1 itself, to
  check press timing and how closely the screen's lights-out matches the ESP.
