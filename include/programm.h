#ifndef PROGRAMM_H
#define PROGRAMM_H
#include "main.h"

uint16_t eepromMemoryAddressForHour(uint8_t prg, uint8_t day);
byte eepromWrBuff(uint16_t memoryAddress, const uint8_t* buffer, uint8_t length);
void eepromRdBuff(uint16_t memoryAddress, uint8_t* buffer, uint8_t length);
void prepareTable(uint8_t prg, int16_t t0on, int16_t t0off, int16_t t1on, int16_t t1off);
void prepareProg(uint8_t prg);
void testProgs();
void checkAndApplyHourlyProgram();

#endif /* PROGRAMM_H */