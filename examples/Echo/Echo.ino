/*
 * Freenove ES8311: microphone → speaker echo (duplex I2S)
 * Pins below match Freenove ESP32-P4 (FNK0117) on-board ES8311 + PA.
 */
#include <Freenove_ES8311.h>

Freenove_ES8311 audio;

// ---- Pin configuration (Freenove ESP32-P4 on-board audio) ----
const int8_t PIN_I2C_SDA = 7;
const int8_t PIN_I2C_SCL = 8;
const uint8_t ES8311_I2C_ADDR = 0x18;

const int8_t PIN_I2S_MCLK = 9;
const int8_t PIN_I2S_BCLK = 10;
const int8_t PIN_I2S_DIN  = 11;   // microphone
const int8_t PIN_I2S_WS   = 12;
const int8_t PIN_I2S_DOUT = 13;   // to codec / amplifier
const int8_t PIN_PA_EN    = 53;   // speaker amplifier enable (high = on)

static const uint32_t SAMPLE_RATE = 24000;
static const uint16_t MCLK_MULTIPLE = 256;
static const uint8_t VOLUME = 70;
static const uint8_t MIC_GAIN_DB = 40;
static uint8_t echoBuf[2400];

void setup() {
  Serial.begin(115200);
  delay(300);

  audio.setI2CPins(PIN_I2C_SDA, PIN_I2C_SCL, ES8311_I2C_ADDR);
  audio.setI2SPins(PIN_I2S_BCLK, PIN_I2S_WS, PIN_I2S_DOUT, PIN_I2S_DIN, PIN_I2S_MCLK);
  audio.setPAPin(PIN_PA_EN);

  if (!audio.begin(SAMPLE_RATE, MCLK_MULTIPLE, VOLUME, MIC_GAIN_DB, true)) {
    Serial.println("Audio begin failed");
    while (true) {
      delay(1000);
    }
  }
  Serial.println("Echo ready — speak into the microphone");
}

void loop() {
  const size_t n = audio.read(echoBuf, sizeof(echoBuf));
  if (n == 0) {
    delay(10);
    return;
  }
  audio.write(echoBuf, n);
}
