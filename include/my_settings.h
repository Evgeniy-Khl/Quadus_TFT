#include "main.h"

char botToken[50] = "";     // your Bot Token (Get from Botfather);
char chatID [15]  = "";     // your Chat ID

int8_t dataOut[6] = {-1,-1,-1,-1,-1,-1};

const char* ntpServer = "pool.ntp.org"; // Сервер NTP
const char* tzInfo = "EET-2EEST,M3.5.0/3,M10.5.0/4";

bool rtcTimeSet = false;
struct tm* timeinfo;

bool shouldSaveConfig = false;
bool enabledListen = false;
int8_t  displNum, setupNum;

uint16_t t_x = 0, t_y = 0, xpos = 0, ypos = 0;
bool newDispl = true;
int16_t resetDisplay = 0;
float editValue = 0;
const char* keyLabel[15];
uint16_t keyColor[15];
bool newTxt = false;
GrafDispl grafDispl[2] = {
    { 80, 80, 80, 0, 0, 0 },
    { 240, 80, 80, 0, 0, 0 }
};

uint8_t resetDispl,
        halfSecond,
        beepOn,
        keys,
        keyCount,
        lastKey,
        countSeconds,
        minutes,
        lastSyncDay,
        sources;

int16_t editBuff0, editBuff1;

uint16_t    pvTimer,
            disableBeep,
            waitCheckKeyPad = WAITCHECKKEYPAD;

long counterWait,
        counter10,
        counter1s;

#define PCF8574_ADDRESS 0x27

uint8_t earlyMode = 0, mode = READEEPROM, tmrResetMode = 0, quarter = GET_PROG1, errors, seconds = 0;
int tmrTelegramOff = 30;
long lastSendTime = 0, allTime = 0; 
Interval interval = INTERVAL_1000;

SettingsUnion settings_union = {
    .settings_struct = {
        .spT0on = T0ON,
        .spT0off = T0OFF,
        .spT1on = T1ON,
        .spT1off = T1OFF,
        .water0on = WT0ON,
        .water0off = WT0OFF,
        .water1on = WT1ON,
        .water1off = WT1OFF,
        .water2on = WT2ON,
        .water2off = WT2OFF,
        .curFlap = 0,
        .minFlap = 0,
        .maxFlap = 100,
        .timerOn = TIMERON,
        .timerOff = TIMEROFF,
        .alarm0 = ALARM0,
        .alarm1 = ALARM1,
        .hysteresis0 = HYSTER0,
        .hysteresis1 = HYSTER1,
        .special = 0,
        .deviceNum = 0,
        .program = 0,
        .modeLight = 0,
        .modeHeater = 0,
        .modeHumidi = 0,
        .modeRelay1 = 0x00,
        .modeRelay2 = 0x00,
        .modeRelay3 = 0x00
    }
};

const uint8_t tabRH[420]={
95,90,86,81,77,72,68,64,60,56,52,48,44,40,36,32,29,25,22,18,
95,91,86,82,77,73,69,65,61,57,53,49,45,42,38,34,31,27,24,20,
95,91,87,82,78,74,70,66,62,58,54,50,47,43,40,36,33,29,26,23,
96,91,87,83,79,75,71,67,63,59,55,52,48,45,41,38,34,31,28,25,
96,91,87,83,79,75,71,67,64,60,56,53,49,46,43,39,36,33,30,26,
96,92,88,84,80,76,72,68,65,61,57,54,51,47,44,41,37,34,31,28,
96,92,88,84,80,76,73,69,65,62,58,55,52,48,45,42,39,36,33,30,
96,92,88,84,80,77,73,70,66,63,59,56,53,50,46,43,40,37,34,32,
96,92,88,85,81,77,74,70,67,64,60,57,54,51,48,45,42,39,36,33,
96,92,89,85,81,78,74,71,68,64,61,58,55,52,49,46,43,40,37,35,
96,92,89,85,82,78,75,71,68,65,62,59,56,53,50,47,44,41,39,36,
96,93,89,85,82,79,75,72,69,66,63,60,57,54,51,48,45,42,40,37,
96,93,89,86,82,79,76,73,69,66,63,60,57,55,52,49,46,44,41,38,
96,93,89,86,83,79,76,73,70,67,64,61,58,55,53,50,47,45,42,40,
96,93,90,86,83,80,77,74,71,68,65,62,59,56,54,51,48,46,43,41,
97,93,90,87,83,80,77,74,71,68,65,62,60,57,54,52,49,47,44,42,
97,93,90,87,84,80,77,74,72,69,66,63,60,58,55,53,50,48,45,43,
97,93,90,87,84,81,78,75,72,69,66,64,61,58,56,53,51,48,46,44,
97,93,90,87,84,81,78,75,72,70,67,64,62,59,57,54,52,49,47,45,
97,94,90,87,84,81,79,76,73,70,67,65,62,60,57,55,52,50,48,46,
97,94,91,88,85,82,79,76,73,71,68,65,63,60,58,56,53,51,49,46
};

const uint8_t quadus_[7]         = {0x4B, 0x42, 0x41, 0xE0, 0xA9, 0x43, 0x20};  // КВАДУС_
const uint8_t error_[8]          = {0xA8, 0x4F, 0x4D, 0xA5, 0xA7, 0x4B, 0x41, 0x20};  // ПОМИЛКА_
const uint8_t alarm[8]           = {0x54, 0x70, 0x65, 0xB3, 0x6F, 0xB4, 0x61, 0x21};  // Тревога!
const uint8_t connect[10]        = {0xBE, 0x69, 0xE3, 0xBA, 0xBB, 0xC6, 0xC0, 0x65, 0xBD, 0x6F};  // пiдключено
const uint8_t config[12]         = {0x4B, 0x6F, 0xBD, 0xE4, 0x69, 0xB4, 0x79, 0x70, 0x61, 0xE5, 0x69, 0xC6};// Конфiгурацiю
const uint8_t no_[3]             = {0xBD, 0x65, 0x20};  // не_
const uint8_t saved[10]          = {0xB7, 0xB2, 0x65, 0x70, 0x65, 0xB6, 0x65, 0xBD, 0x6F, 0x21};// збережено!
const uint8_t manual_control[15] = {0x50, 0x79, 0xC0, 0xBD, 0x65, 0x20, 0xBA, 0x65, 0x70, 0x79, 0xB3, 0x61, 0xBD, 0xBD, 0xC7};//Ручне керування
const uint8_t restored[10]       = {0xB3, 0x69, 0xE3, 0xBD, 0x6F, 0xB3, 0xBB, 0x65, 0xBD, 0x61};// вiдновлена
const uint8_t save_time[13]      = {0xA4, 0xB2, 0x65, 0x70, 0x65, 0xB4, 0xBF, 0xB8, 0x20, 0xC0, 0x61, 0x63, 0x3F};// Зберегти час?
const uint8_t time_[4]           = {0xAB, 0x61, 0x63, 0x20};// Час_
const uint8_t no_permissions[13] = {0xBD, 0x65, 0xBC, 0x61, 0x65, 0x20, 0xE3, 0x6F, 0xB7, 0xB3, 0x6F, 0xBB, 0x79};// немае дозволу
const uint8_t sensorsWord[7]     = {0xE0, 0x61, 0xBF, 0xC0, 0xB8, 0xBA, 0xB8};// датчики
const uint8_t settingUp[12]      = {0x48, 0x61, 0xBB, 0x61, 0xC1, 0xBF, 0x79, 0xB3, 0x61, 0xBD, 0xBD, 0xC7};// Налаштування

