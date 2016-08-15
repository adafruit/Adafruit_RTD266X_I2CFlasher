#include <Wire.h>
#include <Arduino.h>

#define RTD_I2CADDR  0x4A       // shouldn't change, i2c addr o

// uncomment for long/detailed i2c data xfer
//#define I2C_DEBUG_OUTPUT

bool WriteReg(uint8_t a, uint8_t d) {
    Wire.beginTransmission(RTD_I2CADDR);
    Wire.write(a);
    Wire.write(d);
    Wire.endTransmission();
#ifdef I2C_DEBUG_OUTPUT
    Serial.print("$"); Serial.print(a, HEX); Serial.print(F(" <- 0x")); Serial.println(d, HEX);
#endif
    return true;
}

bool WriteBytesToAddr(uint8_t reg, uint8_t* values, uint8_t len) {
    Wire.beginTransmission(RTD_I2CADDR);
    Wire.write(reg);
#ifdef I2C_DEBUG_OUTPUT
    Serial.print("$"); Serial.print(reg, HEX); Serial.print(" <-");
#endif
for(uint8_t i=0; i< len; i++) {
      Wire.write(values[i]);
#ifdef I2C_DEBUG_OUTPUT
      Serial.print(" 0x"); Serial.print(values[i], HEX);
#endif
}
    Wire.endTransmission();
#ifdef I2C_DEBUG_OUTPUT
    Serial.println();
#endif
return true;
}

uint32_t ReadReg(uint8_t a) {
    Wire.beginTransmission(RTD_I2CADDR);
    Wire.write(a);
    Wire.endTransmission();
    Wire.requestFrom((uint8_t)RTD_I2CADDR, (uint8_t)1);
    uint8_t d = Wire.read();
#ifdef I2C_DEBUG_OUTPUT
    Serial.print("$"); Serial.print(a, HEX); Serial.print(F(" -> 0x")); Serial.println(d, HEX);
#endif
    return d;    
}

bool ReadBytesFromAddr(uint8_t reg, uint8_t* dest, uint8_t len) {
  Wire.beginTransmission(RTD_I2CADDR);
  Wire.write(reg);
  Wire.endTransmission();

#ifdef I2C_DEBUG_OUTPUT
  Serial.print("$"); Serial.print(reg, HEX); Serial.print(" ->");
#endif

  Wire.requestFrom((uint8_t)RTD_I2CADDR, len);
  for (uint8_t i=0; i<len; i++) {
    dest[i] = Wire.read();
#ifdef I2C_DEBUG_OUTPUT
    Serial.print(" 0x"); Serial.print(dest[i], HEX);
#endif
  }
#ifdef I2C_DEBUG_OUTPUT
  Serial.println();
#endif
  return true;
}

