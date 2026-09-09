// config.h
// Pin map and tunables for the STEM Racing Reaction Timer, Prototype V1.
// Target board: Waveshare ESP32-P4-Nano-ETH.
//
// *** VERIFY BEFORE WIRING ***
// The ESP32-P4 has no native Wi-Fi/BT (it talks to an onboard ESP32-C6 over
// SDIO) and the onboard RJ45 uses a factory-wired RMII PHY. Both the SDIO
// bus and the RMII bus occupy fixed GPIOs on this board that are NOT
// available on the GPIO header. The pin numbers below are for the FREE
// header pins only, but you must cross-check them against the current
// Waveshare wiki pinout diagram for your board revision before soldering
// anything, because header breakouts have changed between hardware
// revisions of this board. Treat every GPIO number in this file as a
// starting point to confirm, not a guarantee.

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
#define LANE1_BUTTON_PIN   4
#define LANE2_BUTTON_PIN   5

// ---------- F1-style start light tree (5x LED, left to right) ----------
#define LED1_PIN 6
#define LED2_PIN 7
#define LED3_PIN 8
#define LED4_PIN 9
#define LED5_PIN 10
#define NUM_LEDS 5
static const uint8_t LED_PINS[NUM_LEDS] = {LED1_PIN, LED2_PIN, LED3_PIN, LED4_PIN, LED5_PIN};

// ---------- OLED status display (I2C SSD1306 128x64) ----------
#define OLED_SDA_PIN   11
#define OLED_SCL_PIN   12
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
