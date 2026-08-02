# arduino-sketches

**Embedded projects on Arduino, ESP32, and M5Stack. Tested on real hardware.**

Started with blinking an LED, like everyone. Then wanted something useful — a display showing text from an SD card, a temperature sensor controlling a relay, a menu on an LCD. Built a few things. Some work great, some were just for fun.

## Projects

### 1. Smart Home Sensor

Temperature and humidity sensor (DHT22), motion detector (PIR), relay for light control. All displayed on a 16x2 LCD with button-based menu navigation. Auto-turns off the light 30 seconds after no motion.

**Hardware**: Arduino, DHT22, PIR, relay, LCD 16x2, push button

### 2. TFT SD Reader

Reads a text file from an SD card and displays it on a TFT screen. Refreshes every 5 seconds.

**Hardware**: TFT_eSPI-compatible display, SPI, SD card

### 3. M5Stack Cardputer Projects

A collection of apps and tools I wrote for the M5Stack Cardputer:

| Project | Description | Lines |
|---------|------------|-------|
| **cardputer-alarm-clock** | Full alarm clock with RTC, buzzer, LCD menu system. Multiple alarms, time display, settings. | ~5400 C++ |
| **cardputer-gif-player** | Plays GIF animations on the Cardputer screen with SD card support. | ~6900 C++ |
| **cardputer-matrix** | Matrix-style rain effect on the display. | ~1600 C++ |
| **cardputer-notes** | Note-taking app with keyboard input and serial sync. Includes Python companion scripts. | ~7700 C++ |
| **cardputer-pager** | Two-device paging system — sender sends messages, receiver displays them. Uses ESP-NOW. | ~1500 C++ |
| **cardputer-pong** | Pong game for the Cardputer with keyboard controls and display. | ~6700 C++ |
| **cardputer-snake** | Snake game with keyboard input. | ~3600 C++ |

All Cardputer projects use PlatformIO build system and the M5Stack Cardputer board definition.

## Structure

```
├── smart-home-sensor/
│   └── smart-home-sensor.ino
├── tft-sd-reader/
│   └── tft-sd-reader.ino
├── cardputer-alarm-clock/
│   ├── src/            # C++ source
│   ├── boards/         # board config
│   └── platformio.ini
├── cardputer-gif-player/
│   ├── src/
│   └── platformio.ini
├── cardputer-matrix/
│   ├── src/
│   └── platformio.ini
├── cardputer-notes/
│   ├── src/
│   └── platformio.ini
├── cardputer-pager/
│   ├── receiver/       # PlatformIO project
│   └── sender/         # PlatformIO project
├── cardputer-pong/
│   ├── src/
│   └── platformio.ini
└── cardputer-snake/
    ├── src/
    └── platformio.ini
```

## Notes

These sketches are real and tested. Most of the server-side stuff is in my other repos — this is the hardware side of things. I write whatever C++ the project needs — not an embedded systems purist, but I ship working code.

---

MIT License.
