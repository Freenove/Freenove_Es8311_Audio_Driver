/*
 * Freenove ES8311 Audio Driver Library
 *
 * Combines ESP_I2S (data path) with ES8311 I2C register control (codec path)
 * and an optional PA enable pin. Codec sequences mirror Espressif esp_codec_dev
 * so Arduino and ESP-IDF tracks leave the chip in the same state.
 *
 * Default audio path: I2S master on the SoC, ES8311 slave, 16-bit Philips,
 * mono, 24 kHz, MCLK = 256 x fs. Call begin() after setI2CPins / setI2SPins
 * (or use the Freenove ESP32-P4 board defaults).
 *
 * Music / TX-only: define all pins including DIN, then begin with duplex=false
 * so ESP_I2S does not create an unread RX channel that stalls the peripheral.
 */

#ifndef FREENOVE_ES8311_H
#define FREENOVE_ES8311_H

#include <Arduino.h>
#include <Wire.h>
#include <ESP_I2S.h>

/** 7-bit I2C address for Wire. Espressif ES8311_CODEC_DEFAULT_ADDR 0x30 is 8-bit. */
#ifndef FREENOVE_ES8311_I2C_ADDR
#define FREENOVE_ES8311_I2C_ADDR 0x18
#endif

/** Freenove ESP32-P4 board (FNK0117) on-board audio defaults */
#ifndef FREENOVE_ES8311_PIN_I2C_SDA
#define FREENOVE_ES8311_PIN_I2C_SDA  7
#define FREENOVE_ES8311_PIN_I2C_SCL  8
#define FREENOVE_ES8311_PIN_I2S_MCLK 9
#define FREENOVE_ES8311_PIN_I2S_BCLK 10
#define FREENOVE_ES8311_PIN_I2S_DIN  11
#define FREENOVE_ES8311_PIN_I2S_WS   12
#define FREENOVE_ES8311_PIN_I2S_DOUT 13
#define FREENOVE_ES8311_PIN_PA       53
#endif

class Freenove_ES8311 {
public:
  explicit Freenove_ES8311(TwoWire &wire = Wire,
                           uint8_t i2cAddr = FREENOVE_ES8311_I2C_ADDR);

  // ---- Pin setup (call before begin) ----

  /** I2C bus for codec control. */
  void setI2CPins(int8_t sda, int8_t scl, uint8_t addr = FREENOVE_ES8311_I2C_ADDR);

  /**
   * I2S pins. Always pass the real board pins (including DIN).
   * TX-only vs duplex is chosen later by begin(..., duplex).
   * Order matches ESP_I2S::setPins(bclk, ws, dout, din, mclk).
   */
  void setI2SPins(int8_t bclk, int8_t ws, int8_t dout, int8_t din, int8_t mclk);

  /** Speaker amplifier enable GPIO (active high by default). */
  void setPAPin(int8_t paPin, bool activeHigh = true);

  /** Apply Freenove ESP32-P4 on-board pin map (I2C 7/8, I2S 9–13, PA 53). */
  void setPinsFreenoveESP32P4(bool duplex = true);

  // ---- Bring-up / tear-down ----

  /**
   * Start PA → I2S → codec (open / configSample / start) → volume [/ mic gain].
   * @param duplex true: use DIN for mic RX; false: force DIN=-1 (TX-only).
   * @param micGainDb 0–42 sets mic PGA; negative skips (typical for music).
   */
  bool begin(uint32_t sampleRate = 24000, uint16_t mclkMultiple = 256,
             uint8_t volume = 70, int micGainDb = -1, bool duplex = true);

  /** Stop I2S and mute / release codec readiness flag. */
  void end();

  // ---- PCM I/O (thin wrappers over ESP_I2S) ----

  size_t write(const void *data, size_t len);
  size_t read(void *data, size_t len);
  size_t available();

  // ---- Runtime codec / PA controls ----

  bool setVolume(uint8_t volume);     // 0–100, esp_codec_dev curve
  bool setMicGain(uint8_t gainDb);    // 0–42 dB, 6 dB steps
  bool setMute(bool mute);
  bool setPA(bool enable);

  // ---- Low-level / diagnostics ----

  bool open();                                              // soft reset + open table
  bool configSample(uint32_t sampleRate, uint16_t mclkMultiple);
  bool startCodec();                                        // power-up paths
  uint8_t readRegister(uint8_t reg);
  bool writeRegister(uint8_t reg, uint8_t value);

  bool isReady() const { return _ready; }
  uint32_t sampleRate() const { return _sampleRate; }
  I2SClass &i2s() { return _i2s; }

private:
  void updateRegister(uint8_t reg, uint8_t clearMask, uint8_t setBits);
  static uint8_t volumeToReg(uint8_t volume);
  static uint8_t micGainToReg(uint8_t gainDb);

  TwoWire *_wire;
  I2SClass _i2s;
  uint8_t _addr;
  int8_t _sda, _scl;
  int8_t _bclk, _ws, _dout, _din, _mclk;
  int8_t _paPin;
  bool _paActiveHigh;
  uint32_t _sampleRate;
  uint16_t _mclkMultiple;
  bool _ready;
};

#endif
