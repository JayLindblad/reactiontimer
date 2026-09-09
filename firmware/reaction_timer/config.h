// config.h
// Pin map and tunables for the STEM Racing Reaction Timer, Prototype V1.
// Target board: Waveshare ESP32-P4-Nano-ETH.
//
// GPIO assignments below are taken from the board's GPIO header pinout
// table (left/right header pin -> GPIO number), which already excludes the
// SDIO (Wi-Fi co-processor) and RMII (onboard Ethernet PHY) pins since
// those are hardwired PCB-side and never exposed on the header in the
// first place. Every GPIO chosen below is confirmed free of JTAG
// (MTDI/MTDO/MTCK/MTMS on GPIO19-22) and strapping-pin (GPIO34-36)
// functions per that table. The one item NOT covered by the header table,
// and still worth confirming against Waveshare's official ETH.begin() demo
// before flashing, is the ETH_PHY_* block below -- those describe the
// onboard PHY's internal wiring, not header pins.

#pragma once

// ---------- Network ----------
// Set to 1 to bring the link up over the onboard PoE Ethernet PHY.
// Set to 0 to fall back to Wi-Fi via the onboard ESP32-C6 co-processor.
#define USE_ETHERNET 1

#if USE_ETHERNET
  // These five values come from Waveshare's official ETH.begin() demo for
  // the ESP32-P4-Nano-ETH (wiki.waveshare.com) -- DO NOT guess these, copy
  // them from the vendor example that matches your silkscreen revision.
  #define ETH_PHY_TYPE_CFG   ETH_PHY_IP101   // confirm: IP101 vs LAN8720 etc.
  #define ETH_PHY_ADDR_CFG   1
  #define ETH_PHY_MDC_CFG    31
  #define ETH_PHY_MDIO_CFG   52
  #define ETH_PHY_POWER_CFG  51
  #define ETH_CLK_MODE_CFG   ETH_CLOCK_GPIO0_IN
#else
  #define WIFI_SSID "YOUR_SSID"
  #define WIFI_PASS "YOUR_PASSWORD"
#endif

// mDNS name -> reachable at http://reactiontimer.local/
#define MDNS_HOSTNAME "reactiontimer"

// ---------- Trigger buttons (RJ45 handhelds) ----------
// Active LOW: external 1k-4.7k pull-up + 100nF cap to GND per docs/WIRING.md.
// Button shorts the signal line to GND when pressed.
// Header: GPIO14 = left pin 5, GPIO15 = right pin 6.
#define LANE1_BUTTON_PIN   14
#define LANE2_BUTTON_PIN   15

// ---------- F1-style start light tree (5x LED, left to right) ----------
// Set to 0 to shelve the physical LEDs: no GPIOs are driven and the same
// 5-light tree instead renders on the OLED (drawLightTree() in the .ino).
// Flip to 1 once the LEDs are wired per docs/WIRING.md -- no other code
// changes needed, lightsOn[] and the state machine are unaffected either way.
#define USE_PHYSICAL_LEDS 0

// Header: GPIO16=left7, GPIO17=right8, GPIO18=left9, GPIO23=right14, GPIO26=left15.
// (GPIO19-22 skipped: JTAG MTDI/MTDO/MTCK/MTMS. GPIO34-36 skipped: strapping pins.)
#define LED1_PIN 16
#define LED2_PIN 17
#define LED3_PIN 18
#define LED4_PIN 23
#define LED5_PIN 26
#define NUM_LEDS 5
static const uint8_t LED_PINS[NUM_LEDS] = {LED1_PIN, LED2_PIN, LED3_PIN, LED4_PIN, LED5_PIN};

// ---------- OLED status display (I2C SSD1306 128x64) ----------
// Header: GPIO27 = right pin 16, GPIO28 = left pin 17.
#define OLED_SDA_PIN   27
#define OLED_SCL_PIN   28
#define OLED_I2C_ADDR  0x3C
#define OLED_WIDTH     128
#define OLED_HEIGHT    64

// ---------- Timing ----------
#define SEQUENCE_INTERVAL_MS   1000   // one LED lights every 1s during buildup
#define MIN_HOLD_MS             500   // random hold after all 5 lit
#define MAX_HOLD_MS            2500
#define REACTION_TIMEOUT_MS   10000   // DNF if no press within this long after GO
#define DEBOUNCE_US            30000  // software lockout after a valid edge (30ms)

// ---------- History ----------
#define MAX_HISTORY 20
