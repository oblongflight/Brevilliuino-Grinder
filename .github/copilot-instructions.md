# Brevilliuino Grinder - AI Coding Instructions

## Project Overview
ESP32-based smart grinder controller for Breville Barista Express that adds stop-on-mass grinding via load cell, power control via relay, LED management, and Google Home integration via SinricPro.

**Target Hardware:** LilyGo T-Display S3 (ESP32-S3 with integrated TFT display)

## Architecture

### Core Components
- **Load Cell + HX711** (pins 1, 2): Measures coffee mass in real-time for auto-stop grinding
- **TLC5947 LED Driver** (pins 17, 18, 21): Controls 11 front panel LEDs (channels 6-16)
- **Relays**: Grinder motor (pin 10), main power (pin 3)
- **Potentiometer** (pin 16): Target mass selection (16g-56g range based on ADC/100 + 16)
- **TFT Display**: Shows target/mass with TFT_eSPI sprites; rotation 3 (landscape, buttons left)

### State Machine
The `menu` flag controls display mode:
- `menu=1`: Target mass selection (orange text) - shown when potentiometer moves
- `menu=0`: Live mass display (green text) - auto-switches after 2 seconds

Grinding stops automatically when `mass >= target - 0.2` (0.2g offset accounts for grounds in transit).

## Build & Deploy

```bash
# Build (uses PlatformIO)
pio run

# Upload to device
pio run -t upload

# Serial monitor
pio device monitor
```

## Critical Setup: credentials.h

You **must** create `src/credentials.h` with SinricPro/WiFi credentials:
```cpp
#define WIFI_SSID "your_ssid"
#define WIFI_PASS "your_password"
#define APP_KEY "sinricpro_app_key"
#define APP_SECRET "sinricpro_app_secret"
#define SWITCH_ID "sinricpro_switch_id"
```

## Key Patterns

### LED PWM Values
- Standard brightness: `3600` (most LEDs)
- Dim brightness: `1080` (channels 9, 10, 15, 16)
- Always call `tlc.write()` after `tlc.setPWM()` changes
- Call `resetLEDs()` after user actions to fix voltage-related LED dropouts

### Button Handlers (EasyButton)
- **Grind/Tare Button** (pin 12): Short press toggles grinder, hold (1s) tares scale
- **Power Button** (pin 11): Toggles machine power with animated LED sequence

### Display Conventions
- Use `TFT_eSprite` for flicker-free updates (`spr` is 300x100)
- Fonts: `NotoSans96` (mass display), `NotoSansBold36` (medium), `NotoSansBold15` (small)
- Colors: `TFT_GREEN` for mass, `TFT_ORANGE` for targets/UI elements

### Load Cell
- Calibration factor: `2237.f` (set in `LC.set_scale()`)
- Use `LC.get_units(1)` for single reading (no averaging for responsiveness)
- Tare on startup and via button hold

## Dependencies (platformio.ini)
- `HX711` - Load cell amplifier
- `TFT_eSPI` - Display driver
- `EasyButton` - Button debouncing with press/hold events
- `Adafruit TLC5947` - LED driver
- `SinricPro` - Google Home integration
