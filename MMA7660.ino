#include "MMA7660.h"

void MMA7660::send(uint8_t addr, uint8_t data) {
  Wire.beginTransmission(MMA7660_ADDRESS);
  Wire.write(addr);
  Wire.write(data);
  Wire.endTransmission();
}

void MMA7660::init() {
  Wire.begin();
  send(0x07, 0x00);
  send(0x08, 0x00); // 120/sec every 16 2/3 frames, 8 1/3 ms
  /* send(0x08, 0x01); // 64/sec every 31.25 frames or every 15.625ms */
  send(0x07, 0x01);
}
void MMA7660::standby() {
  Wire.begin();
  send(0x07, 0x10);
}

void MMA7660::readAxis(int8_t* d, uint8_t axis) {
  uint8_t v = 64;
  Wire.beginTransmission(MMA7660_ADDRESS);
  Wire.write(axis);
  Wire.endTransmission();
  Wire.requestFrom(MMA7660_ADDRESS, 1);
  if (Wire.available()) {
    v = Wire.read();
  }
  *d = ((int8_t)(v << 2)) / 4;
}

void MMA7660::readXYZ(int8_t* x, int8_t* y, int8_t* z) {
  Wire.beginTransmission(MMA7660_ADDRESS);
  Wire.write(0x00);
  Wire.endTransmission();
  Wire.requestFrom(MMA7660_ADDRESS, 3);

  if (Wire.available()) {
    *x = Wire.read();
    *y = Wire.read();
    *z = Wire.read();
  }
}

void MMA7660::readXYZ(int16_t* x, int16_t* y, int16_t* z) {
  Wire.beginTransmission(MMA7660_ADDRESS);
  Wire.write(0x00);
  Wire.endTransmission();
  Wire.requestFrom(MMA7660_ADDRESS, 3);

  if (Wire.available()) {
    *x = Wire.read();
    *y = Wire.read();
    *z = Wire.read();
  }
}
