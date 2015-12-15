#ifndef __MMA7660_H
#define __MMA7660_H

class MMA7660 {
  public:
    void send(uint8_t addr, uint8_t data);
    void init();
    void standby();
    void readAxis(int8_t* d, uint8_t axis);
    void readXYZ(int8_t* x, int8_t* y, int8_t* z);
    void readXYZ(int16_t* x, int16_t* y, int16_t* z);
};

#endif
