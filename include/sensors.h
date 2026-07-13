#ifndef __SENSORS_H
#define __MA__SENSORS_HIN_H

void temperature_check(void);
int16_t valDcToRH(uint16_t Vadc);
int16_t lowPassF2(int16_t PV);

#endif /* __SENSORS_H */