#include "Freenove_ES8311.h"
#include <math.h>

#define ES8311_REG_RESET   0x00
#define ES8311_REG_CLK_SRC 0x01
#define ES8311_REG_CLK_DIV 0x02
#define ES8311_REG_ADC_FS  0x03
#define ES8311_REG_DAC_FS  0x04
#define ES8311_REG_AD_DAC  0x05
#define ES8311_REG_BCLK    0x06
#define ES8311_REG_LRCK_H  0x07
#define ES8311_REG_LRCK_L  0x08
#define ES8311_REG_SDPIN   0x09
#define ES8311_REG_SDPOUT  0x0A
#define ES8311_REG_SYS0B   0x0B
#define ES8311_REG_SYS0C   0x0C
#define ES8311_REG_SYS0D   0x0D
#define ES8311_REG_SYS0E   0x0E
#define ES8311_REG_SYS10   0x10
#define ES8311_REG_SYS11   0x11
#define ES8311_REG_SYS12   0x12
#define ES8311_REG_SYS13   0x13
#define ES8311_REG_SYS14   0x14
#define ES8311_REG_ADC15   0x15
#define ES8311_REG_ADC16   0x16
#define ES8311_REG_ADC17   0x17
#define ES8311_REG_ADC1B   0x1B
#define ES8311_REG_ADC1C   0x1C
#define ES8311_REG_DAC31   0x31
#define ES8311_REG_DAC32   0x32
#define ES8311_REG_DAC37   0x37
#define ES8311_REG_GPIO44  0x44
#define ES8311_REG_GP45    0x45

Freenove_ES8311::Freenove_ES8311(TwoWire &wire, uint8_t i2cAddr)
  : _wire(&wire),
    _addr(i2cAddr),
    _sda(FREENOVE_ES8311_PIN_I2C_SDA),
    _scl(FREENOVE_ES8311_PIN_I2C_SCL),
    _bclk(FREENOVE_ES8311_PIN_I2S_BCLK),
    _ws(FREENOVE_ES8311_PIN_I2S_WS),
    _dout(FREENOVE_ES8311_PIN_I2S_DOUT),
    _din(FREENOVE_ES8311_PIN_I2S_DIN),
    _mclk(FREENOVE_ES8311_PIN_I2S_MCLK),
    _paPin(FREENOVE_ES8311_PIN_PA),
    _paActiveHigh(true),
    _sampleRate(24000),
    _mclkMultiple(256),
    _ready(false) {}

void Freenove_ES8311::setI2CPins(int8_t sda, int8_t scl, uint8_t addr) {
  _sda = sda;
  _scl = scl;
  _addr = addr;
}

void Freenove_ES8311::setI2SPins(int8_t bclk, int8_t ws, int8_t dout, int8_t din, int8_t mclk) {
  _bclk = bclk;
  _ws = ws;
  _dout = dout;
  _din = din;
  _mclk = mclk;
}

void Freenove_ES8311::setPAPin(int8_t paPin, bool activeHigh) {
  _paPin = paPin;
  _paActiveHigh = activeHigh;
}

void Freenove_ES8311::setPinsFreenoveESP32P4(bool duplex) {
  setI2CPins(FREENOVE_ES8311_PIN_I2C_SDA, FREENOVE_ES8311_PIN_I2C_SCL, FREENOVE_ES8311_I2C_ADDR);
  setI2SPins(FREENOVE_ES8311_PIN_I2S_BCLK,
             FREENOVE_ES8311_PIN_I2S_WS,
             FREENOVE_ES8311_PIN_I2S_DOUT,
             duplex ? (int8_t)FREENOVE_ES8311_PIN_I2S_DIN : (int8_t)-1,
             FREENOVE_ES8311_PIN_I2S_MCLK);
  setPAPin(FREENOVE_ES8311_PIN_PA, true);
}

bool Freenove_ES8311::writeRegister(uint8_t reg, uint8_t value) {
  _wire->beginTransmission(_addr);
  _wire->write(reg);
  _wire->write(value);
  return _wire->endTransmission() == 0;
}

uint8_t Freenove_ES8311::readRegister(uint8_t reg) {
  _wire->beginTransmission(_addr);
  _wire->write(reg);
  if (_wire->endTransmission(false) != 0) {
    return 0;
  }
  if (_wire->requestFrom((int)_addr, 1) != 1) {
    return 0;
  }
  return (uint8_t)_wire->read();
}

void Freenove_ES8311::updateRegister(uint8_t reg, uint8_t clearMask, uint8_t setBits) {
  const uint8_t value = (uint8_t)((readRegister(reg) & ~clearMask) | setBits);
  writeRegister(reg, value);
}

bool Freenove_ES8311::open() {
  if (!writeRegister(ES8311_REG_RESET, 0x1F)) {
    return false;
  }
  delay(5);

  if (readRegister(ES8311_REG_SYS0D) != 0xFA) {
    writeRegister(ES8311_REG_SYS0D, 0xFA);
  }
  writeRegister(ES8311_REG_GPIO44, 0x08);
  writeRegister(ES8311_REG_GPIO44, 0x08);
  writeRegister(ES8311_REG_CLK_SRC, 0x30);
  writeRegister(ES8311_REG_CLK_DIV, 0x00);
  writeRegister(ES8311_REG_ADC_FS, 0x10);
  writeRegister(ES8311_REG_ADC16, 0x24);
  writeRegister(ES8311_REG_DAC_FS, 0x10);
  writeRegister(ES8311_REG_AD_DAC, 0x00);
  writeRegister(ES8311_REG_SYS0B, 0x00);
  writeRegister(ES8311_REG_SYS0C, 0x00);
  writeRegister(ES8311_REG_SYS10, 0x1F);
  writeRegister(ES8311_REG_SYS11, 0x7F);
  writeRegister(ES8311_REG_RESET, 0x80);

  updateRegister(ES8311_REG_RESET, 0x40, 0x00);
  writeRegister(ES8311_REG_CLK_SRC, 0x3F);
  updateRegister(ES8311_REG_BCLK, 0x20, 0x00);
  writeRegister(ES8311_REG_SYS13, 0x10);
  writeRegister(ES8311_REG_ADC1B, 0x0A);
  writeRegister(ES8311_REG_ADC1C, 0x6A);
  writeRegister(ES8311_REG_GPIO44, 0x58);
  return true;
}

bool Freenove_ES8311::configSample(uint32_t sampleRate, uint16_t mclkMultiple) {
  if (sampleRate != 24000 || mclkMultiple != 256) {
    return false;
  }

  updateRegister(ES8311_REG_SDPIN, 0x00, 0x0C);
  updateRegister(ES8311_REG_SDPOUT, 0x00, 0x0C);
  updateRegister(ES8311_REG_SDPIN, 0x03, 0x00);
  updateRegister(ES8311_REG_SDPOUT, 0x03, 0x00);

  updateRegister(ES8311_REG_CLK_DIV, 0xF8, 0x00);
  writeRegister(ES8311_REG_AD_DAC, 0x00);
  updateRegister(ES8311_REG_ADC_FS, 0x7F, 0x10);
  updateRegister(ES8311_REG_DAC_FS, 0x7F, 0x10);
  updateRegister(ES8311_REG_LRCK_H, 0x3F, 0x00);
  writeRegister(ES8311_REG_LRCK_L, 0xFF);
  updateRegister(ES8311_REG_BCLK, 0x1F, 0x03);
  return true;
}

bool Freenove_ES8311::startCodec() {
  writeRegister(ES8311_REG_RESET, 0x80);
  writeRegister(ES8311_REG_CLK_SRC, 0x3F);

  updateRegister(ES8311_REG_SDPIN, 0x40, 0x00);
  updateRegister(ES8311_REG_SDPOUT, 0x40, 0x00);

  writeRegister(ES8311_REG_ADC17, 0xBF);
  writeRegister(ES8311_REG_SYS0E, 0x02);
  writeRegister(ES8311_REG_SYS12, 0x00);
  writeRegister(ES8311_REG_SYS14, 0x1A);
  writeRegister(ES8311_REG_SYS0D, 0x01);
  writeRegister(ES8311_REG_ADC15, 0x40);
  writeRegister(ES8311_REG_DAC37, 0x08);
  writeRegister(ES8311_REG_GP45, 0x00);
  return true;
}

uint8_t Freenove_ES8311::volumeToReg(uint8_t volume) {
  if (volume == 0) {
    return 0x00;
  }
  if (volume > 100) {
    volume = 100;
  }
  const float hwGainDb = 20.0f * log10f(3.3f / 5.0f);
  const float db = (0.0f - hwGainDb) - (100 - volume) * 0.5f;
  const float reg = (db - (-95.5f)) * (255.0f / (32.0f - (-95.5f)));
  return (uint8_t)constrain((int)(reg + 0.5f), 0, 255);
}

uint8_t Freenove_ES8311::micGainToReg(uint8_t gainDb) {
  return (uint8_t)constrain(gainDb / 6, 0, 7);
}

bool Freenove_ES8311::setVolume(uint8_t volume) {
  updateRegister(ES8311_REG_DAC31, 0x60, 0x00);
  return writeRegister(ES8311_REG_DAC32, volumeToReg(volume));
}

bool Freenove_ES8311::setMicGain(uint8_t gainDb) {
  return writeRegister(ES8311_REG_ADC16, micGainToReg(gainDb));
}

bool Freenove_ES8311::setMute(bool mute) {
  if (mute) {
    updateRegister(ES8311_REG_DAC31, 0x00, 0x60);
  } else {
    updateRegister(ES8311_REG_DAC31, 0x60, 0x00);
  }
  return true;
}

bool Freenove_ES8311::setPA(bool enable) {
  if (_paPin < 0) {
    return false;
  }
  pinMode((uint8_t)_paPin, OUTPUT);
  const bool level = _paActiveHigh ? enable : !enable;
  digitalWrite((uint8_t)_paPin, level ? HIGH : LOW);
  return true;
}

bool Freenove_ES8311::begin(uint32_t sampleRate, uint16_t mclkMultiple,
                            uint8_t volume, int micGainDb, bool duplex) {
  _ready = false;
  _sampleRate = sampleRate;
  _mclkMultiple = mclkMultiple;

  const int8_t din = duplex ? _din : (int8_t)-1;

  setPA(true);

  _i2s.setPins(_bclk, _ws, _dout, din, _mclk);
  if (!_i2s.begin(I2S_MODE_STD, sampleRate, I2S_DATA_BIT_WIDTH_16BIT, I2S_SLOT_MODE_MONO)) {
    return false;
  }

  // Codec is an I2S slave: clocks must already be running
  _wire->begin(_sda, _scl);
  if (!open() || !configSample(sampleRate, mclkMultiple) || !startCodec()) {
    _i2s.end();
    return false;
  }
  if (!setVolume(volume)) {
    _i2s.end();
    return false;
  }
  if (micGainDb >= 0 && !setMicGain((uint8_t)micGainDb)) {
    _i2s.end();
    return false;
  }

  _ready = true;
  return true;
}

void Freenove_ES8311::end() {
  if (_ready) {
    setMute(true);
  }
  setPA(false);
  _i2s.end();
  _ready = false;
}

size_t Freenove_ES8311::write(const void *data, size_t len) {
  return _i2s.write((const uint8_t *)data, len);
}

size_t Freenove_ES8311::read(void *data, size_t len) {
  return _i2s.readBytes((char *)data, len);
}

size_t Freenove_ES8311::available() {
  return _i2s.available();
}
