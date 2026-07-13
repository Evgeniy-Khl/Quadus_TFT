#ifndef __MAIN_H
#define __MAIN_H

#include <Arduino.h>
#include <ArduinoJson.h>
#include <FS.h>
#include <LittleFS.h>
#define FlashFS LittleFS
#include <TFT_eSPI.h> // Hardware-specific library
#include <SPI.h>
#include <Wire.h>     // Библиотека для I2C связи
#include <RTClib.h>   // Библиотека для работы с RTC DS3231
#include <OneWire.h>
#include <DallasTemperature.h>
#include "SoftwarePWMBit.h" // Подключаем наш новый класс
#include "procedure.h"

#define DEBUG

#ifdef DEBUG
  // Вариативные макросы, принимающие любое количество аргументов
  #define DEBUG_PRINT(...)   Serial.print(__VA_ARGS__)
  #define DEBUG_PRINTLN(...) Serial.println(__VA_ARGS__)
#else
  // "Пустышки" остаются такими же
  #define DEBUG_PRINT(...)
  #define DEBUG_PRINTLN(...)
#endif
// --- Конец блока макросов ---

#define LEDPIN 2
#define ONE_WIRE_BUS_PIN LEDPIN   // используется номер GPIO2
#define MAX_DEVICE 4              // ограничение количества датчиков
#define FONT_SMALL "Arial20"
#define FONT_LARGE "Arial28"

typedef struct {
  int16_t pvT;
  int16_t pvErr;
  int16_t previousValue;
  uint8_t errDevice;
  uint16_t duration;
} Ds;

extern Ds ds[];

// Для предотвращения выравнивания полей компилятором, что может нарушить карту памяти.
// В данном случае все поля одного типа, и проблема маловероятна, но это хорошая практика.
#pragma pack(push, 1)
struct Sp{
    int16_t spT; 	      // Уставка температуры
    int16_t spRH;	      // Уставка относительной влажности (sp[0].spRH->ПОДСТРОЙКА HIH)
    int16_t alarm;      // дельта 5 = 0.5 гр.C
    int16_t coolOn;     // включение охлаждения
    int16_t coolOff;    // выключение охлаждения
    int16_t timer;      // длительность [0]-отключ.состояниe [1]-включ.состояниe
    int16_t aeration;   // [0]-ПАУЗА ПРОВЕТРИВАНИЯ (минут); [1]-ДЛИТЕЛЬНОСТЬ ПРОВЕТРИВАНИЯ (секунд)
    int16_t auxiliary;  // [0]-включение форсированного; [1]-выключение форсированного
    int16_t state;      // [0]-заслонка текущее; [1]-программа текущая
    int16_t flapLimit;  // [0]-закрыта; [1]-открыта
    int16_t pulse;      // [0]-MIN; [1]-Период импульсов
    int16_t mode;       // [0]-релейный 0-НЕТ; 1->по кан.0 2->по кан.1 3->по кан.0&1; 4-импульсное по кан.1; [1]-задержка регулировки по влажному
    int16_t extendMode; // [0]-0-СИРЕНА; 1-АВАРИЙНОЕ ОТКЛЮЧЕНИЕ; [1]-???????????????
    int16_t Kp;         // Пропорциональный
    int16_t Ki;         // Интегральный
};
#pragma pack(pop)

// Определяем union
union SpUnion {
    // Представление 1: Как массив из двух структур
    Sp sp_structs[2];
    // Представление 2: Как линейный массив из 32-х 16-битных значений
    int16_t flat_array[32]; // 16 полей * 2 структуры = 32
};

typedef struct
{
  uint16_t xpos; 
  uint16_t ypos; 
  uint8_t radius; 
  int16_t value; 
  int16_t sp;
} GrafDispl;

struct Bitfield {
    unsigned a0: 1;
    unsigned a1: 1;
    unsigned a2: 1;
    unsigned a3: 1;
    unsigned a4: 1;
    unsigned a5: 1;
    unsigned a6: 1;
    unsigned a7: 1;
};
 
union Byte {
    unsigned char value;
    struct Bitfield bitfield;
};

extern union Byte portOut;
extern union Byte errors;
extern union Byte portFlag;

#define HEATER  portOut.bitfield.a0  // НАГРЕВАТЕЛЬ
#define HUMIDI	portOut.bitfield.a1  // УВЛАЖНИТЕЛЬ
#define TURN		portOut.bitfield.a2  // Поворот лотков
#define EXTRA1	portOut.bitfield.a3  // Заслонка/вентилятор охлаждения
#define EXTRA2	portOut.bitfield.a4  // Вспомогательный нагреватель
#define EXTRA3	portOut.bitfield.a5  // Авария

#define ERROR1  errors.bitfield.a0  //
#define ERROR2	errors.bitfield.a1  //
#define ERROR3	errors.bitfield.a2  //
#define ERROR4	errors.bitfield.a3  //
#define ERROR5	errors.bitfield.a4  //
#define ERROR6	errors.bitfield.a5  //
#define ERROR7	errors.bitfield.a6  //
#define ERROR8	errors.bitfield.a7  // завис датчик.

#define REACHED0  portFlag.bitfield.a0  // pvT[0]-ДОСТИГ spT[0]
#define REACHED1  portFlag.bitfield.a1  // pvT[1]-ДОСТИГ spT[1]
#define VENTIL 	portFlag.bitfield.a2  // Ventilation flag
#define EEPSAVE portFlag.bitfield.a3  // Save in EEPROM flag
#define HIH5030	  portFlag.bitfield.a4  // exist HIH5030 flag
#define AM2301	  portFlag.bitfield.a5  // exist AM2301 flag
#define COOLING   portFlag.bitfield.a6  // охлаждение
#define AERATION  portFlag.bitfield.a7  // проветривание

#define ON 1
#define OFF 0
#define TRIACON 1023
#define DISPLAYOFF 300


extern RTC_DS3231 rtc;
extern char displStr[];
extern bool newDispl;
extern float editValue;
extern uint8_t numberOfDevices, seconds, displNum, displPower, pvTimer, errDevice[];
extern uint16_t xpos, ypos, txt_height, t_x, t_y;
extern uint16_t pvVadcRH, pvRH, heaterValue, humidiValue, pvPulse;
extern SpUnion settings;
extern DallasTemperature sensors;
extern const uint8_t tabRH[];

byte writePCF8574(byte data);
byte readPCF8574();
void testAT24C32();

#endif /* __MAIN_H */