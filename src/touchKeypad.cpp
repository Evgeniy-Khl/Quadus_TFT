#include "main.h"
#include "TFT_eSPI.h"
#include "display.h"
#include "tftArcFill.h"
#include "touchKeypad.h"

char numberBuffer[NUM_LEN + 1] = "";
uint8_t numberIndex, txtIndex, earlyDispl;

void checkKeypad(uint8_t amt){
  const char* txt;
  // / Check if any key coordinate boxes contain the touch coordinates
  for (uint8_t b = 0; b < amt; b++) {
    if (key[b].contains(t_x, t_y)) {
      key[b].press(true);  // tell the button it is pressed
    } else {
      key[b].press(false);  // tell the button it is NOT pressed
    }
  }

  for (uint8_t b = 0; b < amt; b++) {
      if (key[b].isPressed()) {
        if(amt==15){      
          int8_t v = butCalculator(b);
          DEBUG_PRINT("checkKeypad/amt==15: v="); DEBUG_PRINTLN(v);
          drawValue(v, true);
        } else {
          switch (displNum){
          case 1:
            newDispl = true;
            if (b == MENU_1-1){
              DEBUG_PRINTLN("checkKeypad(): b == MENU_1-1: case 1: displNum = 2 unloadFont()");
              displNum = 2;
              menu_2();
            }
            else if (b == MENU_1-2) {
              tft.unloadFont(); // выгрузка шрифта из памяти
              DEBUG_PRINTLN("checkKeypad(): b == MENU_1-2: case 1: displNum = 0 unloadFont()");
              displNum = 0; newDispl = true;
            }else {
              earlyDispl = displNum;
              txtIndex = b;
              numberIndex = b;
              editValue = settings.flat_array[numberIndex];
              tft.setTextColor(TFT_WHITE, TFT_BLACK);
              txt = labelsMenu1[txtIndex];
              // tft.drawString(labelsMenu1[txtIndex], DISP_W/2, DISP_Y + 5);
              DEBUG_PRINT("checkKeypad(): b < MENU_1-2: case 1: displNum = 10 txt:"); DEBUG_PRINTLN(txt);
              displNum = 10;
              calcDisplay(txt);
              drawValue(0, true);
            }
          break;
          case 2: 
            newDispl = true;
            if (b == MENU_1-1){
              DEBUG_PRINTLN("checkKeypad(): b == MENU_1-1: case 2: displNum = 3 unloadFont()");
              displNum = 3;
              menu_3();
            }
            else if (b == MENU_1-2) {
              DEBUG_PRINTLN("checkKeypad(): b == MENU_1-2: case 2: displNum = 1 unloadFont()");
              displNum = 1;
              menu_1();
            }else {
              earlyDispl = displNum;
              txtIndex = b;
              numberIndex = b+15;
              editValue = settings.flat_array[numberIndex];
              tft.setTextColor(TFT_WHITE, TFT_BLACK);
              txt = labelsMenu1[txtIndex];
              // tft.drawString(labelsMenu1[txtIndex], DISP_W/2, DISP_Y + 5);
              DEBUG_PRINT("checkKeypad(): b < MENU_1-2: case 2: displNum = 10"); DEBUG_PRINTLN(txt);
              displNum = 10;
              calcDisplay(txt);
              drawValue(0, true);
            }
          break;
          case 3: 
            newDispl = true;
            if (b == MENU_2-1){
              DEBUG_PRINTLN("checkKeypad(): b == MENU_2-1: case 3: displNum = 4 unloadFont()");
              displNum = 4;
              menu_4();
            }
            else if (b == MENU_2-2) {
              DEBUG_PRINTLN("checkKeypad(): b == MENU_2-2: case 3: displNum = 2 unloadFont()");
              displNum = 2;
              menu_2();
            }else {
              earlyDispl = displNum;
              txtIndex = b;
              numberIndex = b/2+5;
              if(b%2) numberIndex += 15;
              editValue = settings.flat_array[numberIndex];
              tft.setTextColor(TFT_WHITE, TFT_BLACK);
              txt = labelsMenu2[txtIndex];
              // tft.drawString(labelsMenu1[txtIndex], DISP_W/2, DISP_Y + 5);
              DEBUG_PRINT("checkKeypad(): b < MENU_2-2: case 3: displNum = 10"); DEBUG_PRINTLN(txt);
              displNum = 10;
              calcDisplay(txt);
              drawValue(0, false);
            }
          break;
          case 4: 
            newDispl = true;
            if (b == MENU_3-1){
              tft.unloadFont(); // выгрузка шрифта из памяти
              DEBUG_PRINTLN("checkKeypad(): b == MENU_3-1: case 4: displNum = 0 unloadFont()");
              displNum = 0; newDispl = true;
            }
            else if (b == MENU_3-2) {
              // tft.unloadFont(); // выгрузка шрифта из памяти
              DEBUG_PRINTLN("checkKeypad(): b == MENU_3-2: case 4: displNum = 3 unloadFont()");
              displNum = 3;
              menu_3();
            }else {
              earlyDispl = displNum;
              txtIndex = b;
              numberIndex = b/2+11;
              if(b%2) numberIndex += 15;
              editValue = settings.flat_array[numberIndex];
              tft.setTextColor(TFT_WHITE, TFT_BLACK);
              txt = labelsMenu3[txtIndex];
              // tft.drawString(labelsMenu1[txtIndex], DISP_W/2, DISP_Y + 5);
              DEBUG_PRINT("checkKeypad(): b < MENU_3-2: case 4: displNum = 10"); DEBUG_PRINTLN(txt);
              displNum = 10;
              calcDisplay(txt);
              drawValue(0, false);
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
      DEBUG_PRINTLN("Пустая строка.");
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
      DEBUG_PRINTLN("Найдена команда 'Отмена' (X).");
    }
    if (strcmp(current_label, "Ok") == 0){
      settings.flat_array[numberIndex] = editValue;
      saveConfig();  // Сохраним эти значения в файл
      if(numberIndex == 0 || numberIndex == 15){
        grafDispl[0].sp = settings.sp_structs[0].spT;
        grafDispl[1].sp = settings.sp_structs[1].spT;
      }
      displNum = earlyDispl;
      switch (displNum){
        case 0: newDispl = true; break;
        case 1: menu_1();  break;
        case 2: menu_2();  break;
        case 3: menu_3();  break;
        case 4: menu_4();  break;
      }
      DEBUG_PRINTLN("Найдена команда 'Подтвердить' (Ok).");
    }
    // 3. Если это не команда и не пустая строка, пытаемся преобразовать в число
    char* end; // Указатель на символ, где остановился парсинг
    value = strtol(current_label, &end, 10); // 10 - десятичная система
    if (end != current_label){
      DEBUG_PRINT("Преобразовано в число: ");
      DEBUG_PRINTLN(value);
    }
    return value;
}

void drawValue(int8_t val, bool divide){
  // uint8_t dividerValue = 1;
  if(displNum == 10){
    // if(divide) dividerValue = 10;
    editValue += val;
    newTxt = true;
    // sprintf(displStr,"%5.1f  Д=%d  К=%i",editValue/dividerValue, dividerValue, val);
    tft.loadFont(FONT_LARGE, LittleFS); // загрузка в память шрифта
    DEBUG_PRINTLN("drawValue():Arial28");
    tft.setTextColor(TFT_WHITE, TFT_BLACK);
    if(divide) sprintf(displStr,"%5.1f",editValue/10);
    else sprintf(displStr,"%5.0f",editValue);
    tft.drawString(displStr, DISP_W/2, DISP_Y + 5 + 28);
    sprintf(displStr,"індекс=%2d",numberIndex);
    tft.drawString(displStr, DISP_W/2, DISP_Y + 10 + 56);
    tft.unloadFont(); // выгрузка шрифта из памяти
    DEBUG_PRINTLN("drawValue():unloadFont");
  }
}

void touch_calibrate()
{
  uint16_t calData[5];
  uint8_t calDataOK = 0;

  // check file system exists
  if (!LittleFS.begin()) {
    DEBUG_PRINTLN("formatting file system");
    LittleFS.format();
    LittleFS.begin();
  }

  // check if calibration file exists and size is correct
  if (LittleFS.exists(CALIBRATION_FILE)) {
    if (REPEAT_CAL)
    {
      // Delete if we want to re-calibrate
      LittleFS.remove(CALIBRATION_FILE);
    }
    else
    {
      File f = LittleFS.open(CALIBRATION_FILE, "r");
      if (f) {
        if (f.readBytes((char *)calData, 14) == 14)
          calDataOK = 1;
        f.close();
      }
    }
  }

  if (calDataOK && !REPEAT_CAL) {
    // calibration data valid
    tft.setTouch(calData);
  } else {
    // data not valid so recalibrate
    tft.fillScreen(TFT_BLACK);
    tft.setCursor(20, 0);
    tft.setTextFont(2);
    tft.setTextSize(1);
    tft.setTextColor(TFT_WHITE, TFT_BLACK);

    tft.println("Touch corners as indicated");

    tft.setTextFont(1);
    tft.println();

    if (REPEAT_CAL) {
      tft.setTextColor(TFT_RED, TFT_BLACK);
      tft.println("Set REPEAT_CAL to false to stop this running again!");
    }

    tft.calibrateTouch(calData, TFT_MAGENTA, TFT_BLACK, 15);

    tft.setTextColor(TFT_GREEN, TFT_BLACK);
    tft.println("Calibration complete!");

    // store data
    File f = LittleFS.open(CALIBRATION_FILE, "w");
    if (f) {
      f.write((const unsigned char *)calData, 14);
      f.close();
    }
  }
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