#include "main.h"
#include "TFT_eSPI.h"
#include "display.h"
#include "tftArcFill.h"
#include "touchKeypad.h"

char numberBuffer[NUM_LEN + 1] = "";
uint8_t numberIndex, txtIndex, earlyDispl;

enum EditType {
    EDIT_CALC,   // Открыть калькулятор
    EDIT_TOGGLE, // Переключить (0 <-> 1)
};

struct MenuItem {
    uint8_t index;     // Индекс во flat_array
    EditType type;     // Способ редактирования
    bool divide;       // Делить на 10 (для EDIT_CALC)
    int16_t minValue;  // MIN значение
    int16_t maxValue;  // MAX значение
};

MenuItem curItem;

const MenuItem mapMenu1[] = {
    {0, EDIT_CALC, true, 0, 990},   // кнопка 0 (spT0on)
    {1, EDIT_CALC, true, 0, 990},   // кнопка 1 (spT0off)
    {17, EDIT_CALC, true, 1, 100},  // кнопка 2 (hysteresis0)
    {15, EDIT_CALC, true, 0, 990},  // кнопка 3 (alarm0)
    {10, EDIT_CALC, false, 0, 100}, // кнопка 4 (curFlap)
    {21, EDIT_CALC, false, 0, 4}    // кнопка 5 (program)
};

const MenuItem mapMenu2[] = {
    {2, EDIT_CALC, true, 0, 990},   // кнопка 0 (spT1on)
    {3, EDIT_CALC, true, 0, 990},   // кнопка 1 (spT1off)
    {18, EDIT_CALC, true, 1, 100},  // кнопка 2 (hysteresis1)
    {16, EDIT_CALC, true, 0, 990}   // кнопка 3 (alarm1)
};

const MenuItem mapMenu3[] = {
    {13, EDIT_CALC, false, 0, 23},  // кнопка 0 (timerOn)
    {14, EDIT_CALC, false, 0, 23},  // кнопка 1 (timerOff)
    { 4, EDIT_CALC, false, 1, 99},  // кнопка 2 (water0on)
    { 5, EDIT_CALC, false, 0, 11},  // кнопка 3 (water0off)
    { 6, EDIT_CALC, false, 1, 99},  // кнопка 4 (water1on)
    { 7, EDIT_CALC, false, 0, 11},  // кнопка 5 (water1off)
    { 8, EDIT_CALC, false, 1, 99},  // кнопка 6 (water2on)
    { 9, EDIT_CALC, false, 0, 11}   // кнопка 7 (water2off)
};

const MenuItem mapMenu4[] = {
    {23, EDIT_TOGGLE, false, 0, 1}, // кнопка 0 (modeHeater, переключатель 0 <-> 1)
    {24, EDIT_TOGGLE, false, 0, 1}, // кнопка 1 (modeHumidi, переключатель 0 <-> 1)
    {25, EDIT_CALC, false, 0, 3},   // кнопка 2 (modeRelay1)
    {26, EDIT_CALC, false, 0, 3},   // кнопка 3 (modeRelay2)
    {27, EDIT_CALC, false, 0, 3},   // кнопка 4 (modeRelay3)
    {22, EDIT_CALC, false, 0, 1},   // кнопка 5 (modeLight)
    {11, EDIT_CALC, false, 0, 100}, // кнопка 6 (minFlap)
    {12, EDIT_CALC, false, 0, 100}, // кнопка 7 (maxFlap)
    {19, EDIT_CALC, false, 0, 99},  // кнопка 8 (special)
    {20, EDIT_CALC, false, 1, 99}   // кнопка 9 (deviceNum)
};

void checkKeypad(uint8_t amt){
  const char* txt;
  #ifdef DEBUG
    char displStr[128];
  #endif
  // Проверьте, содержатся ли координаты касания в каких-либо полях ключевых координат.
  for (uint8_t b = 0; b < amt; b++) {
    if (key[b].contains(t_x, t_y)) {
      key[b].press(true);  // сообщите кнопке, что она нажата
    } else {
      key[b].press(false);  // сообщите кнопке, что она НЕ нажата.
    }
  }

  for (uint8_t b = 0; b < amt; b++) {
      if (key[b].isPressed()) {
        if(amt==15){      
          int8_t v = butCalculator(b);
          MYDEBUG_PRINT("checkKeypad/amt==15: v="); MYDEBUG_PRINTLN(v);
          drawValue(v, curItem.divide);
        } else {
          switch (displNum){
          case 1:
            newDispl = true;
            if (b == POINTS_1-1){ // кнопка "наступне"
              MYDEBUG_PRINTLN("checkKeypad(); кнопка 'наступне'; case 1:");
              displNum = 2;
              menu_2();
              waitForTouchRelease();
            }
            else if (b == POINTS_1-2){ // кнопка "повернуться"
              tft.unloadFont(); // выгрузка шрифта из памяти
              MYDEBUG_PRINTLN("checkKeypad(); кнопка 'повернуться'; case 2:");
              displNum = 0; newDispl = true;
              waitForTouchRelease();
            }else {
              curItem = mapMenu1[b];
              numberIndex = curItem.index;
              if (curItem.type == EDIT_TOGGLE) {
                settings_union.flat_array[numberIndex] = !settings_union.flat_array[numberIndex];
                saveSetPoint();
                menu_1();
                waitForTouchRelease();
              } else {
                earlyDispl = displNum;
                txtIndex = b;
                editValue = settings_union.flat_array[numberIndex];
                tft.setTextColor(TFT_WHITE, TFT_BLACK);
                txt = labelsMenu1[txtIndex];
                #ifdef DEBUG
                  sprintf(displStr,"case %d: b=%d val=%d txt:%s", displNum, b, (int)editValue, txt);
                  MYDEBUG_PRINTLN(displStr);
                #endif
                displNum = 10;
                calcDisplay(txt);
                drawValue(0, curItem.divide);
                waitForTouchRelease();
              }
            }
          break;
          case 2: 
            newDispl = true;
            if (b == POINTS_2-1){ // кнопка "наступне"
              MYDEBUG_PRINTLN("checkKeypad(); кнопка 'наступне'; case 2:");
              displNum = 3;
              menu_3();
              waitForTouchRelease();
            }
            else if (b == POINTS_2-2){ // кнопка "повернуться"
              MYDEBUG_PRINTLN("checkKeypad(); кнопка 'повернуться'; case 2:");
              displNum = 1;
              menu_1();
              waitForTouchRelease();
            }else {
              curItem = mapMenu2[b];
              numberIndex = curItem.index;
              if (curItem.type == EDIT_TOGGLE) {
                settings_union.flat_array[numberIndex] = !settings_union.flat_array[numberIndex];
                saveSetPoint();
                menu_2();
                waitForTouchRelease();
              } else {
                earlyDispl = displNum;
                txtIndex = b;
                editValue = settings_union.flat_array[numberIndex];
                tft.setTextColor(TFT_WHITE, TFT_BLACK);
                txt = labelsMenu2[txtIndex];
                #ifdef DEBUG
                  sprintf(displStr,"case %d: b=%d val=%d txt:%s", displNum, b, (int)editValue, txt);
                  MYDEBUG_PRINTLN(displStr);
                #endif
                displNum = 10;
                calcDisplay(txt);
                drawValue(0, curItem.divide);
                waitForTouchRelease();
              }
            }
          break;
          case 3: 
            newDispl = true;
            if (b == POINTS_3-1){ // кнопка "наступне"
              MYDEBUG_PRINTLN("checkKeypad(); кнопка 'наступне'; case 3:");
              displNum = 4;
              menu_4();
              waitForTouchRelease();
            }
            else if (b == POINTS_3-2){ // кнопка "повернуться"
              MYDEBUG_PRINTLN("checkKeypad(); кнопка 'повернуться'; case 3:");
              displNum = 2;
              menu_2();
              waitForTouchRelease();
            }else {
              curItem = mapMenu3[b];
              numberIndex = curItem.index;
              if (curItem.type == EDIT_TOGGLE) {
                settings_union.flat_array[numberIndex] = !settings_union.flat_array[numberIndex];
                saveSetPoint();
                menu_3();
                waitForTouchRelease();
              } else {
                earlyDispl = displNum;
                txtIndex = b;
                editValue = settings_union.flat_array[numberIndex];
                tft.setTextColor(TFT_WHITE, TFT_BLACK);
                txt = labelsMenu3[txtIndex];
                #ifdef DEBUG
                  sprintf(displStr,"case %d: b=%d val=%d txt:%s", displNum, b, (int)editValue, txt);
                  MYDEBUG_PRINTLN(displStr);
                #endif
                displNum = 10;
                calcDisplay(txt);
                drawValue(0, curItem.divide);
                waitForTouchRelease();
              }
            }
          break;
          case 4: 
            newDispl = true;
            if (b == POINTS_4-1){ // кнопка "наступне"
              tft.unloadFont(); // выгрузка шрифта из памяти
              MYDEBUG_PRINTLN("checkKeypad(); кнопка 'наступне'; case 4:");
              displNum = 0; newDispl = true;
              waitForTouchRelease();
            }
            else if (b == POINTS_4-2){ // кнопка "повернуться"
              // tft.unloadFont(); // выгрузка шрифта из памяти
              MYDEBUG_PRINTLN("checkKeypad(); кнопка 'повернуться'; case 4:");
              displNum = 3;
              menu_3();
              waitForTouchRelease();
            }else {
              curItem = mapMenu4[b];
              numberIndex = curItem.index;
              if (curItem.type == EDIT_TOGGLE) {
                settings_union.flat_array[numberIndex] = !settings_union.flat_array[numberIndex];
                saveSetPoint();
                menu_4();
                waitForTouchRelease();
              } else {
                earlyDispl = displNum;
                txtIndex = b;
                editValue = settings_union.flat_array[numberIndex];
                tft.setTextColor(TFT_WHITE, TFT_BLACK);
                txt = labelsMenu4[txtIndex];
                #ifdef DEBUG
                  sprintf(displStr,"case %d: b=%d val=%d txt:%s", displNum, b, (int)editValue, txt);
                  MYDEBUG_PRINTLN(displStr);
                #endif
                displNum = 10;
                calcDisplay(txt);
                drawValue(0, curItem.divide);
                waitForTouchRelease();
              }
            }
          break;
          }
        }
      }
  }
}
  
int8_t butCalculator(uint8_t butt){
  const char* current_label = labelsCalculator[butt];
  long value = 0;
  // 1. Проверяем на пустую строку
    if (strlen(current_label) == 0) {
      MYDEBUG_PRINTLN("Пустая строка.");
    }

    // 2. Проверяем на известные команды
    if (strcmp(current_label, "X") == 0){
      displNum = earlyDispl;
      switch (displNum){
        case 0: newDispl = true; break;
        case 1: menu_1();  break;
        case 2: menu_2();  break;
        case 3: menu_3();  break;
        case 4: menu_4();  break;
      }
      waitForTouchRelease();
      MYDEBUG_PRINTLN("Найдена команда 'Отмена' (X).");
    }
    if (strcmp(current_label, "Ok") == 0){
      settings_union.flat_array[numberIndex] = editValue;
      if(numberIndex == 0 || numberIndex == 2){
        grafDispl[0].spOn = settings_union.settings_struct.spT0on;
        grafDispl[0].spOff = settings_union.settings_struct.spT0off;
        grafDispl[1].spOn = settings_union.settings_struct.spT1on;
        grafDispl[1].spOff = settings_union.settings_struct.spT1off;
      }
      displNum = earlyDispl;
      switch (displNum){
        case 0: newDispl = true; break;
        case 1: menu_1();  break;
        case 2: menu_2();  break;
        case 3: menu_3();  break;
        case 4: menu_4();  break;
      }
      waitForTouchRelease();
      MYDEBUG_PRINTLN("Найдена команда 'Подтвердить' (Ok).");
    }
    // 3. Если это не команда и не пустая строка, пытаемся преобразовать в число
    char* end; // Указатель на символ, где остановился парсинг
    value = strtol(current_label, &end, 10); // 10 - десятичная система
    if (end != current_label){
      MYDEBUG_PRINT("Преобразовано в число: ");
      MYDEBUG_PRINTLN(value);
    }
    return value;
}

void drawValue(int8_t val, bool divide){
  char displStr[64];
  if(displNum == 10){
    float tempVal = editValue + val;
    if (tempVal < curItem.minValue) tempVal = curItem.minValue;
    if (tempVal > curItem.maxValue) tempVal = curItem.maxValue;
    editValue = tempVal;

    newTxt = true;
    tft.loadFont(FONT_LARGE, LittleFS); // загрузка в память шрифта
    MYDEBUG_PRINTLN("drawValue():Arial28");
    tft.setTextColor(TFT_WHITE, TFT_BLACK);
    if(divide) sprintf(displStr,"%5.1f",editValue/10);
    else sprintf(displStr,"%5.0f",editValue);
    tft.drawString(displStr, DISP_W/2, DISP_Y + 5 + 28);
    sprintf(displStr,"індекс=%2d",numberIndex);
    tft.drawString(displStr, DISP_W/2, DISP_Y + 10 + 56);
    tft.unloadFont(); // выгрузка шрифта из памяти
    MYDEBUG_PRINTLN("drawValue():unloadFont");
  }
}

void touch_calibrate()
{
  uint16_t calData[5] = { 275, 3620, 264, 3532, 1 }; // Default calibration data

  // If calibration file does not exist, save default data to avoid manual blocking calibration
  if (!LittleFS.exists(CALIBRATION_FILE)) {
    File f = LittleFS.open(CALIBRATION_FILE, "w");
    if (f) {
      f.write((const unsigned char *)calData, 14);
      f.close();
    }
  } else {
    // If it exists, read user's custom calibration
    File f = LittleFS.open(CALIBRATION_FILE, "r");
    if (f) {
      if (f.readBytes((char *)calData, 14) == 14) {
        // Data loaded
      }
      f.close();
    }
  }

  tft.setTouch(calData);
}

/* // Print something in the mini status bar
void status(const char *msg) {
  tft.loadFont(FONT_LARGE, LittleFS); // загрузка в память шрифта
  tft.setTextPadding(320);
  //tft.setCursor(STATUS_X, STATUS_Y);
  tft.setTextColor(TFT_WHITE, TFT_BLACK);
//   tft.setTextFont(0);
  tft.setTextDatum(TC_DATUM);
//   tft.setTextSize(1);
  tft.drawString(msg, STATUS_X, STATUS_Y);
  tft.unloadFont(); // выгрузка шрифта из памяти
} */