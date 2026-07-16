#ifndef _PROCEDURE_H
#define _PROCEDURE_H
#include "main.h"

// Перечисление для состояний нашего меню
enum SetState {
  SET_YEAR,
  SET_MONTH,
  SET_DAY,
  SET_HOUR,
  SET_MINUTE,
  CONFIRM_SAVE
};

extern uint8_t seconds;

void beeperOn(uint8_t val);
uint16_t lampUpdate(uint16_t xpos, uint16_t ypos);
void rotate_trays(void);
void checkSetpoint(void);
uint8_t checkConfig(void);
void printSetPoint();
void saveSetPoint();
bool loadSetPoint();
void printAddress(DeviceAddress deviceAddress);
void printBinary(unsigned char byte);
uint8_t tableRH(int16_t maxT, int16_t minT);
void reset(void);
void safeFactoryReset(void);
bool syncTime();
void displTimeSetting(SetState state, const DateTime& dt);
void manualTimeSet();
void keyTimeSetting(SetState& currentState, uint8_t key, DateTime& tempTime);
void setSystemTimeFromRTC();

#endif /* _PROCEDURE_H */
