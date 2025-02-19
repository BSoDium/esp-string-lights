# ESP32 String Lights Controller

MQTT-enabled controller for string lights with multiple effects, designed for ESP32.

## Features

- MQTT control integration
- Physical button control
- Multiple lighting effects
- Persistent state across power cycles
- HomeAssistant compatible

## Hardware Setup

- ESP32 development board
- Relay module (connected to pin 16)
- Push button (connected to pin 2)
- Transistor for effect control (base connected to pin 4)
- LED string lights (connected through relay)

### Wiring Diagram

```
ESP32           Components
Pin 16 -------- Relay Input
Pin 2  -------- Push Button (other end to GND)
Pin 4  -------- Transistor Base (through resistor)
GND   -------- Common Ground
```

## Software Configuration

1. Copy `src/config.h.template` to `src/config.h`
2. Edit `src/config.h` with your credentials:
   ```cpp
   const char *ssid = "YOUR_WIFI_SSID";
   const char *password = "YOUR_WIFI_PASSWORD";
   const char *mqtt_server = "YOUR_MQTT_SERVER";
   ```

## MQTT Topics

- `home/livingroom/string-light` - Power control (ON/OFF)
- `home/livingroom/string-light/effect` - Effect control

### Available Effects

- IN_WAVES
- SEQUENTIAL
- SLO_GLO
- CHASING
- FADE
- TWINKLE
- STATIC
- COMBINATION

## Operation

### Physical Control
- Short press button: Cycle through effects
- Light indicates effect change through blink patterns

### MQTT Control
- Send "ON"/"OFF" to power topic
- Send effect name to effect topic

## Development

Built using PlatformIO. Main components:
- WiFi connection management
- MQTT client for remote control
- Effect cycling logic
- Hardware interrupt handling

### Building

1. Install PlatformIO
2. Configure credentials in `src/config.h`
3. Build and upload:
   ```bash
   pio run -t upload
   ```

## Troubleshooting

1. **No MQTT Connection**
   - Check MQTT server address
   - Verify WiFi credentials
   - ESP32 will attempt 10 reconnections

2. **Effect Not Changing**
   - Verify transistor connections
   - Check effect name spelling in MQTT messages
   - Monitor serial output for debugging

## Security Note

`config.h` is gitignored to prevent credential leaks. Always use the template file as reference and keep your credentials secure.
