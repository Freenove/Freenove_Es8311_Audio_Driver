# Freenove_Es8311_Audio_Driver

Arduino library for **ES8311 + I2S** audio on Freenove ESP32-P4 boards.

- **I2S**: PCM play / capture via `ESP_I2S` (arduino-esp32)
- **Codec**: ES8311 over `Wire`, register sequence aligned with Espressif `esp_codec_dev`
- **PA**: optional amplifier enable GPIO

## Install

Copy this folder into the Arduino `libraries` directory, or use **Sketch → Include Library → Add .ZIP Library…**.

Requires **arduino-esp32** (provides `ESP_I2S` and `Wire`).

## Quick start (Freenove ESP32-P4 defaults)

```cpp
#include <Freenove_ES8311.h>

Freenove_ES8311 audio;

const int8_t PIN_I2C_SDA = 7, PIN_I2C_SCL = 8;
const int8_t PIN_I2S_MCLK = 9, PIN_I2S_BCLK = 10, PIN_I2S_DIN = 11;
const int8_t PIN_I2S_WS = 12, PIN_I2S_DOUT = 13, PIN_PA_EN = 53;

void setup() {
  Serial.begin(115200);
  audio.setI2CPins(PIN_I2C_SDA, PIN_I2C_SCL, 0x18);
  audio.setI2SPins(PIN_I2S_BCLK, PIN_I2S_WS, PIN_I2S_DOUT, PIN_I2S_DIN, PIN_I2S_MCLK);
  audio.setPAPin(PIN_PA_EN);
  if (!audio.begin(24000, 256, 70, 40, true)) {
    Serial.println("audio begin failed");
    while (true) delay(1000);
  }
}

void loop() {
  uint8_t buf[2400];
  size_t n = audio.read(buf, sizeof(buf));
  if (n) audio.write(buf, n);
}
```

Music / TX-only: keep all pins defined (including DIN=11), call `begin(..., duplex=false)` so RX stays closed.

## API overview

| Group | Functions |
|-------|-----------|
| Pins | `setI2CPins`, `setI2SPins`, `setPAPin`, `setPinsFreenoveESP32P4` |
| Life cycle | `begin`, `end` |
| PCM | `write`, `read`, `available` |
| Controls | `setVolume`, `setMicGain`, `setMute`, `setPA` |
| Low level | `open`, `configSample`, `startCodec`, `readRegister`, `writeRegister` |

First release locks the codec clock path to **24 kHz / MCLK × 256**.
