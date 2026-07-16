#ifndef __MAIN_H
#define __MAIN_H

#define LED_DISPLAY

#define DEBUG
#define SIMULATION

#include <Arduino.h>
#include <WiFiManager.h>    // https://github.com/tzapu/WiFiManager
#include <ArduinoJson.h>
#include <FS.h>
#include <LittleFS.h>
#define FlashFS LittleFS

#define READDEFAULT 0
#define READEEPROM  1
#define SAVEEEPROM  2
#define READPROG    3
#define SAVEPROG    4
#define GET_PROG1   0

#include <SPI.h>
#include <Wire.h>     // Библиотека для I2C связи
#include <TFT_eSPI.h>
#include <RTClib.h>   // Библиотека для работы с RTC DS3231
#include <OneWire.h>
#include <DallasTemperature.h>
#include <DHT.h>
#include "AT24C32.h"
#include "server.h"
#include "programm.h"
#include "procedure.h"
#include "sensors.h"
#include "LogicManager.h"
#include "Logger.h"
#include "saveDailyData.h"
#include "InvertedServo.h"
typedef struct
{
  uint16_t xpos; 
  uint16_t ypos; 
  uint8_t radius; 
  int16_t value; 
  int16_t sp;
} GrafDispl;

extern GrafDispl grafDispl[];

#include "display.h"
#include "tftArcFill.h"
#include "touchKeypad.h"
#include "tftProcessing.h"

// --- AT24C32 Memory Map for Graphs ---
#define DAILY_DATA_START      0x700  // Начало данных графиков в EEPROM (после программ, которые идут с 0x000 по 0x600)
#define DAILY_DATA_REC_SIZE   6      // Размер записи (t1, t2, rh по 2 байта)
#define DAILY_DATA_MAX_REC    288    // Записей в сутки




#ifdef DEBUG
  #define DEBUG_SPRINTF(...)  sprintf(__VA_ARGS__)
  #define MYDEBUG_PRINT(...)   Serial.print(__VA_ARGS__)
  #define MYDEBUG_PRINTLN(...) Serial.println(__VA_ARGS__)
  #define DEBUG_PRINTF(...) Serial.printf(__VA_ARGS__)
#else
  #define DEBUG_SPRINTF(...)
  #define MYDEBUG_PRINT(...)
  #define MYDEBUG_PRINTLN(...)
  #define DEBUG_PRINTF(...)
#endif

#define FONT_SMALL "Arial20"
#define FONT_LARGE "Arial28"
#define FONT_MINI  "Calibri14"
#define heaterValue (HEATER == PCF_ON ? 1 : 0)

#define LEDPIN            2
#define ONE_WIRE_BUS_PIN  LEDPIN
#define MAX_DEVICE        2
#define START_MARKER      0xDD

typedef struct {
  int16_t  pvT;              // current value (multiplied by 10, e.g. 225 = 22.5)
  int16_t  pvErr;            // current error (multiplied by 10)
  float   previousValue;    // previous raw value
  uint8_t errDevice;        // sensor no response counter
  uint8_t deviation;        // deviation from setpoint
  uint16_t froze;           // freeze duration counter
} Ds;

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

/**
 * @brief Structure for current system state.
 * Member names have _m suffix to avoid macro collisions.
 */
struct SystemState {
    Ds ds_m[MAX_DEVICE] = {{0,0,0,0,0},{0,0,0,0,0}};
    union Byte portOut_m;
    union Byte errorsFlag_m;
    union Byte portFlag_m;
    uint8_t dataLed_m[7];
    int16_t pvTimeR1_m = 0;
    int16_t pvTimeR2_m = 0;
    int16_t pvTimeR3_m = 0;
    uint8_t pvFlap_m = 0;
    uint8_t numberOfDS18_m = 0;
    int16_t pvRH_m = 0;
    bool hasDHT22_m = false;
};

extern SystemState sysState;

// Language Selection: Uncomment only one
//#define LANG_EN
//#define LANG_RU
#define LANG_UA

enum LogMsgId {
    MSG_HEATER_ERR,
    MSG_HUMIDITY_ERR,
    MSG_CLIMATE_T2_REACHED,
    MSG_ALARM_T2_RANGE,
    MSG_CLIMATE_T1_REACHED,
    MSG_ALARM_T1_RANGE,
    MSG_STARTUP,
    MSG_DHT22_FOUND,
    MSG_DS18B20_FOUND,
    MSG_SENSORS_NONE,
    MSG_RTC_SYNC,
    MSG_FS_OPEN_ERR,
    MSG_JSON_ERR,
    MSG_CONFIG_SAVED,
    MSG_MANUAL_ON,
    MSG_MANUAL_LIGHT,
    MSG_MANUAL_HEATER,
    MSG_MANUAL_HUMIDI,
    MSG_MANUAL_RELAY1,
    MSG_MANUAL_RELAY2,
    MSG_MANUAL_RELAY3,
    MSG_AUTO_RESTORED,
    MSG_LOG_ROTATED,
    MSG_DHT_ERR,
    MSG_DHT_OK,
    MSG_CLIMATE_T1_FROZE,
    MSG_CLIMATE_T2_FROZE,
    MSG_CLIMATE_T1_OK,
    MSG_CLIMATE_T2_OK,
    MSG_DAILY_CLEARED
};

const char* getMsg(LogMsgId id);

enum HeaterMode {
    HEATER_MODE_HEAT = 0,
    HEATER_MODE_COOL = 1
};

enum HumidiMode {
    HUMIDI_MODE_HUMIDIFY = 0,
    HUMIDI_MODE_DEHUMIDIFY = 1
};

#pragma pack(push, 1)
struct Settings {
    int16_t spT0on; 	  // Setpoint T0 ON (value * 10)
    int16_t spT0off; 	  // Setpoint T0 OFF (value * 10)
    int16_t spT1on; 	  // Setpoint T1/Humidity ON (value * 10)
    int16_t spT1off; 	  // Setpoint T1/Humidity OFF (value * 10)
    int16_t water0on;   // Irrigation 1 ON duration (min)
    int16_t water0off;  // Irrigation 1 OFF interval (preset point)
    int16_t water1on;   // Irrigation 2 ON duration (min)
    int16_t water1off;  // Irrigation 2 OFF interval (preset point)
    int16_t water2on;   // Irrigation 3 ON duration (min)
    int16_t water2off;  // Irrigation 3 OFF interval (preset point)
    int16_t curFlap;    // Flap current position (0-100%)
    int16_t minFlap;    // Flap min open (%)
    int16_t maxFlap;    // Flap max open (%)
    int16_t timerOn;    // Lighting ON hour (0-24)
    int16_t timerOff;   // Lighting OFF hour (0-24)
    int16_t alarm0;     // Alarm deviation t0
    int16_t alarm1;     // Alarm deviation t1
    int16_t hysteresis0; // Hysteresis for t0
    int16_t hysteresis1; // Hysteresis for t1
    int16_t special;     // Flags for WiFi/Time/???Reset???
    int16_t deviceNum;   // Device ID
    int16_t program;     // Active program number
    int16_t modeLight;    // Lighting relay mode
    int16_t modeHeater;   // Heater relay mode: 0=Heat, 1=Cool
    int16_t modeHumidi;   // Humidifier relay mode: 0=Humidify, 1=Dehumidify
    int16_t modeRelay1;   // Relay 1 mode and source
    int16_t modeRelay2;   // Relay 2 mode and source
    int16_t modeRelay3;   // Relay 3 mode and source
};
#pragma pack(pop)

union SettingsUnion {
    Settings settings_struct;
    int16_t flat_array[30]; // 30 fields total
};

extern SettingsUnion settings_union;
#define settings settings_union.settings_struct

struct TableForOneHour {
    int16_t spT0on;   // Scaled by 10
    int16_t spT0off;  // Scaled by 10
    int16_t spT1on;   // Scaled by 10
    int16_t spT1off;  // Scaled by 10
    uint8_t water2run;
    uint8_t flapMin;
    uint8_t flapMax;
    uint8_t flapCurr;
};

union TableBuff {
    uint8_t buffer[12];
    struct TableForOneHour spProg;
};

extern TableBuff unTable;

// Legacy macros for compatibility - using _m members to avoid recursion
#define LIGHT		sysState.portOut_m.bitfield.a0
#define HEATER      sysState.portOut_m.bitfield.a1
#define HUMIDI	    sysState.portOut_m.bitfield.a2
#define RELAY1	    sysState.portOut_m.bitfield.a3
#define RELAY2	    sysState.portOut_m.bitfield.a4
#define RELAY3 	    sysState.portOut_m.bitfield.a5

#define ds              sysState.ds_m
#define dataLed         sysState.dataLed_m
#define pvTimeR1        sysState.pvTimeR1_m
#define pvTimeR2        sysState.pvTimeR2_m
#define pvTimeR3        sysState.pvTimeR3_m
#define pvFlap          sysState.pvFlap_m
#define numberOfDS18    sysState.numberOfDS18_m
#define pvRH            sysState.pvRH_m
#define hasDHT22        sysState.hasDHT22_m
#define portOut         sysState.portOut_m
#define errorsFlag      sysState.errorsFlag_m
#define portFlag        sysState.portFlag_m

#define ERROR1    sysState.errorsFlag_m.bitfield.a0 // DEVICE_DISCONNECTED
#define ERROR2	  sysState.errorsFlag_m.bitfield.a1 // DEVICE_DISCONNECTED
#define ERROR4	  sysState.errorsFlag_m.bitfield.a2 // MSG_ALARM_T1_RANGE
#define ERROR8	  sysState.errorsFlag_m.bitfield.a3 // MSG_ALARM_T2_RANGE
#define ERROR10	  sysState.errorsFlag_m.bitfield.a4 // FROZE_T1
#define ERROR20	  sysState.errorsFlag_m.bitfield.a5 // FROZE_T2
#define DHT_ERR   sysState.errorsFlag_m.bitfield.a6 // DHT22 ERROR
#define RESERVE   sysState.errorsFlag_m.bitfield.a7

#define REACHED0    sysState.portFlag_m.bitfield.a0 // MSG_CLIMATE_T1_REACHED
#define REACHED1    sysState.portFlag_m.bitfield.a1 // MSG_CLIMATE_T2_REACHED
#define NEWSCREEN   sysState.portFlag_m.bitfield.a2 // NEW SCREEN
#define RTCENABLE   sysState.portFlag_m.bitfield.a3 // Couldn't find RTC!
#define WIFIENABLE	sysState.portFlag_m.bitfield.a4 // Wi-Fi Local ip:
#define EXTRA1      sysState.portFlag_m.bitfield.a5 // Допоміжний 1
#define EXTRA2      sysState.portFlag_m.bitfield.a6 // Допоміжний 2
#define SAVING      sysState.portFlag_m.bitfield.a7

#define PCF_ON      0
#define PCF_OFF     1

#define BEEP_PIN        0
#define PWMOUT_PIN      15
#define RESETDISPLAY    40
#define MINWAIT         100
#define WAITCHECKKEYPAD 1000

extern uint16_t t_x, t_y, xpos, ypos;
extern bool newDispl;
extern int16_t resetDisplay;
extern float editValue;
extern const char* keyLabel[15];
extern uint16_t keyColor[15];
extern bool newTxt;

#define T0ON      220
#define T0OFF     240
#define T1ON      180
#define T1OFF     200
#define WT0ON     10
#define WT0OFF    1
#define WT1ON     20
#define WT1OFF    7
#define WT2ON     30
#define WT2OFF    10
#define TIMERON   5
#define TIMEROFF  22
#define ALARM0    10
#define ALARM1    10
#define HYSTER0    2
#define HYSTER1    2

extern const char* version;
extern char displStr[18];
extern char botToken[50];
extern char chatID [15];
extern bool shouldSaveConfig;

extern uint8_t earlyMode, mode, tmrResetMode, quarter, errors, seconds;
extern int tmrTelegramOff;
extern long lastSendTime, allTime; 
extern Interval interval;

extern RTC_DS3231 rtc;
extern struct tm* timeinfo;
extern bool rtcTimeSet;
extern InvertedServo incubatorServo;

extern const char* ntpServer;
extern const char* tzInfo;
extern DHT dht;
extern DallasTemperature sensors;
extern DeviceAddress sensorAddresses[MAX_DEVICE];

extern TFT_eSPI tft;
extern int8_t dataOut[6];

extern bool newDispl;
extern long counterWait, counter10, counter1s;
extern int8_t displNum, setupNum;

extern uint8_t resetDispl, halfSecond, beepOn, keys, keyCount, lastKey, countSeconds, minutes, lastSyncDay, sources;
extern int16_t editBuff0, editBuff1;
extern uint16_t pvTimer, disableBeep, waitCheckKeyPad;

extern const uint8_t tabRH[];

byte writePCF8574(byte data);
byte readPCF8574();
void initWiFiManag(void);
void handleWiFi(void);
void displSwitch();
void sensorCheck();
void sensorType();
void checkDs18b20();
void checkkey(uint8_t keys);
void setupSwitch();
void myPrint(const uint8_t* str, uint8_t size);
void testProgs();

extern const uint8_t quadus_[7], error_[8], alarm[8], connect[10], config[12], no_[3], saved[10], timeout_[9], manual_control[15], 
                        restored[10], save_time[13], time_[4], no_permissions[13], sensorsWord[7], settingUp[12];

#endif /* __MAIN_H */
