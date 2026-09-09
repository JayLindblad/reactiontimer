# Wiring Guide - Prototype V1

## ⚠ Before you solder anything

The Waveshare ESP32-P4-Nano-ETH pairs the ESP32-P4 (no native Wi-Fi/BT) with
an onboard **ESP32-C6** co-processor for wireless, talking over a dedicated
**SDIO** bus, plus an onboard **RMII Ethernet PHY** wired to a fixed set of
GPIOs for the PoE port. Both buses are hardwired at the factory and are
**not** available on the general-purpose header, so they don't appear in
the header pinout table at all.

The GPIO numbers below come from the board's GPIO header pinout table
(left/right header pin -> GPIO number) and are confirmed clear of:
- the SDIO/RMII pins (not on the header to begin with),
- the JTAG-muxed pins GPIO19-22 (MTDI/MTDO/MTCK/MTMS), and
- the strapping pins GPIO34-36.

The one thing this header table does *not* cover is the onboard Ethernet
PHY's own init parameters (`ETH_PHY_*` in `config.h`) — those describe how
the PHY is wired internally, not header pins, so pull those five values
from Waveshare's official `ETH.begin()` demo for this board before
flashing.

## GPIO map

| Signal                | GPIO (config.h)         | Header pin | Notes                                   |
|------------------------|--------------------------|------------|------------------------------------------|
| Lane 1 trigger button  | `LANE1_BUTTON_PIN` (14)  | Left 5     | Active LOW, external pull-up + RC filter |
| Lane 2 trigger button  | `LANE2_BUTTON_PIN` (15)  | Right 6    | Active LOW, external pull-up + RC filter |
| Start light 1 (LED)    | `LED1_PIN` (16)          | Left 7     | Leftmost                                 |
| Start light 2 (LED)    | `LED2_PIN` (17)          | Right 8    |                                           |
| Start light 3 (LED)    | `LED3_PIN` (18)          | Left 9     |                                           |
| Start light 4 (LED)    | `LED4_PIN` (23)          | Right 14   |                                           |
| Start light 5 (LED)    | `LED5_PIN` (26)          | Left 15    | Rightmost                                 |
| OLED SDA               | `OLED_SDA_PIN` (27)      | Right 16   | I2C data                                 |
| OLED SCL               | `OLED_SCL_PIN` (28)      | Left 17    | I2C clock                                |
| PoE / Ethernet         | onboard RJ45 port        | —          | No user wiring - PHY hardwired on PCB    |

## LED start lights

**Shelving the physical LEDs for now?** Set `USE_PHYSICAL_LEDS` to `0` in
`config.h` (this is the current default) and skip this section — no GPIOs
get driven, the race state machine and WebSocket telemetry are completely
unaffected, and the same 5-light build-up/hold/GO sequence renders instead
as 5 circles on the OLED (`drawLightTree()` in the sketch). Flip it back to
`1` once the LEDs are wired below; no other code changes are needed.

Each of the 5x 10mm red LEDs gets its own GPIO through a current-limiting
resistor (330Ω-470Ω for a ~10-15mA drive current on a 3.3V logic level; if
you want them brighter, drive each LED through a 2N7000/BC337 transistor
switch instead of straight off the GPIO, and size the resistor for the LED's
actual forward current):

```
GPIO ---[330-470R]---|>|--- GND
                      LED
```

Wire LED1..LED5 physically left-to-right on the light tree to match the
firmware's build-up order (`LED_PINS[]` in config.h).

## OLED (I2C SSD1306, 128x64)

Standard 4-wire I2C hookup:

```
OLED VCC -> 3.3V
OLED GND -> GND
OLED SDA -> OLED_SDA_PIN
OLED SCL -> OLED_SCL_PIN
```

Add 4.7kΩ pull-ups from SDA and SCL to 3.3V if your specific OLED breakout
doesn't already carry them (most SSD1306 breakout boards do — check before
doubling up).

## Trigger buttons over RJ45 / Cat5 patch cable

Each handheld controller is a normally-open arcade pushbutton in a
3D-printed enclosure, wired out through a standard RJ45 jack to a Cat5/Cat5e
patch cable, back to an RJ45 breakout on the main control box.

**Use one twisted pair per lane's signal + return**, not two arbitrary
pins — routing the signal and its ground return as a twisted pair is what
actually gives you the noise rejection over a multi-meter cable run picking
up switching noise from LEDs/PoE:

| RJ45 pin (T568B) | Wire color        | Function              |
|-------------------|--------------------|------------------------|
| 1                  | Orange/White       | Signal (to GPIO)       |
| 2                  | Orange             | Return (to GND)        |
| 3-8                | (remaining pairs)  | Unused, leave unterminated |

Standard **straight-through** patch cables work fine here — we only rely on
pins 1 and 2 carrying continuity end-to-end, which every off-the-shelf
patch cable does.

### Inside the handheld controller

```
RJ45 pin 1 (signal) ---- pushbutton terminal A
RJ45 pin 2 (return) ---- pushbutton terminal B ---- (also tied to GND at the box end)
```

The button simply shorts signal to return when pressed. No electronics are
needed inside the handle itself — all filtering lives at the control-box end
so it's near the microcontroller and easy to service.

### Inside the main control box (per lane)

```
3.3V ----[1kΩ-4.7kΩ pull-up]---- GPIO (LANE_BUTTON_PIN)
                                     |
RJ45 pin 1 (signal) ----------------+---- 100nF ceramic ---- GND
                                     |
                                  (to ESP32 GPIO, short trace)
RJ45 pin 2 (return) --------------------------------------- GND
```

- The pull-up (1kΩ if you want a snappier RC time constant / more noise
  immunity at the cost of a bit more current, 4.7kΩ if you want lower idle
  current — either is fine for this application) holds the line HIGH at
  idle.
- The 100nF ceramic cap across signal-to-GND, placed as close to the GPIO
  pin as possible, forms an RC low-pass filter with the pull-up that
  knocks down fast noise spikes/EMI picked up along the cable run before
  they ever reach the pin.
- Firmware still does a `DEBOUNCE_US` software lockout (config.h, default
  30ms) after the first valid falling edge, since even a filtered mechanical
  switch can chatter on longer cable runs — this belongs on top of the
  hardware filter, not instead of it.
- Configure the GPIO as plain `INPUT` (not `INPUT_PULLUP`) since the
  external resistor is already providing the pull-up; stacking the internal
  weak pull-up on top just changes your RC math slightly and isn't necessary.

## PoE power

The onboard PoE module (802.3af/at, check your board's datasheet for which
class it supports) taps power directly off the same Cat5e/6 cable run to
your network switch/injector — no separate wiring to the ESP32 is required
beyond the network cable itself. Don't power the board simultaneously from
USB while PoE is connected unless your board's datasheet explicitly says
that's safe (most PoE splitter circuits are fine with it since they're
diode-OR'd, but confirm for your revision before assuming).
