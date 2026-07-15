#ifndef __SENSORS_H
#define __MA__SENSORS_HIN_H
#include "main.h"

void sensorType();
void sensorCheck();
bool check_freeze(uint8_t i, float val);
void checkDs18b20(void);
int16_t lowPassF2(int16_t PV);

#endif /* __SENSORS_H */