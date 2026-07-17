#include "main.h"
#include "display.h"
#include "tftArcFill.h"

TFT_eSPI_Button key[15];

// Набор подписей Датчик температури Т1
const char* labelsMenu1[POINTS_1] = {
  "увімкн. грд.Ц",    "вимкн. грд.Ц", 
  "гістерезис грд.Ц", "аварійне грд.Ц", 
  "заслінка полож.",  "задана програма",
  "повернуться", "наступне"
};

uint16_t colorsMenu1[POINTS_1] = {
  TFT_WHITE, TFT_WHITE, TFT_WHITE, TFT_WHITE,
  TFT_WHITE, TFT_WHITE,
  TFT_YELLOW, TFT_GREEN
};

// Набор подписей Датчик вологості Т2
const char* labelsMenu2[POINTS_2] = {
  "увімкн. Ц / %",    "вимкн. Ц / %", 
  "гістерезис Ц / %", "аварійне Ц / %",
  "повернуться", "наступне"
};

uint16_t colorsMenu2[POINTS_2] = {
  TFT_WHITE, TFT_WHITE, TFT_WHITE, TFT_WHITE,
  TFT_YELLOW, TFT_GREEN
};

// Набор подписей Таймери
const char* labelsMenu3[POINTS_3] = {
  "освітлен. увімк.", "освітлен. вимкн.",
  "робота РЕЛЕ-1",    "період РЕЛЕ-1",
  "робота РЕЛЕ-2",    "період РЕЛЕ-2",
  "робота РЕЛЕ-3",    "період РЕЛЕ-3",
  "повернуться", "наступне"
};

uint16_t colorsMenu3[POINTS_3] = {
  TFT_WHITE, TFT_WHITE, TFT_WHITE, TFT_WHITE,
  TFT_WHITE, TFT_WHITE, TFT_WHITE, TFT_WHITE,
  TFT_YELLOW, TFT_GREEN
};

// Набор подписей Системні
const char* labelsMenu4[POINTS_4] = {
  "охолодж./нагрів", "осушен./зволож.",
  "функція РЕЛЕ-1",  "функція РЕЛЕ-2",
  "функція РЕЛЕ-3",  "функція освітлен",
  "заслінка закр.",  "заслінка відкр.",
  "підключ.мережі",  "номер приладу",
  "повернуться", "наступне"
};

uint16_t colorsMenu4[POINTS_4] = {
  TFT_WHITE, TFT_WHITE, TFT_WHITE, TFT_WHITE,
  TFT_WHITE, TFT_WHITE, TFT_WHITE, TFT_WHITE,
  TFT_WHITE, TFT_WHITE,
  TFT_YELLOW, TFT_GREEN
};

// Набор подписей
const char* labelsCalculator[15] = {
  "+1","+5","+10","+50","+100","-1","-5","-10","-50","-100","","","","X","Ok"
};
uint16_t colorCalculator[15] = {
  TFT_CYAN, TFT_CYAN, TFT_CYAN, TFT_CYAN,
  TFT_CYAN, TFT_CYAN, TFT_CYAN, TFT_CYAN, 
  TFT_CYAN, TFT_CYAN, TFT_BLACK, TFT_BLACK,
  TFT_BLACK, TFT_RED, TFT_GREEN

};

//**************** ОСНОВНОЙ ЭКРАН *******************
void mainDispl(void){
  uint16_t h;
  char displStr[32];
  if(newDispl){
    tft.fillScreen(TFT_BLACK);
  }
//-----------
  if(grafDispl[0].value != ds[0].pvT || newDispl) {
    grafDispl[0].value = ds[0].pvT;
    diagram(grafDispl[0], TFT_WHITE);
  }
  if(grafDispl[1].value != ds[1].pvT || newDispl) {
    grafDispl[1].value = ds[1].pvT;
    diagram(grafDispl[1], TFT_WHITE);
  }
  newDispl = false;
//-----------
  h = lampUpdate(15, 125);
//-----------
  #define posErr 160
  tft.setTextPadding(310);
  xpos = 0; ypos = h+8-5;
  // tft.drawRect(xpos-5, ypos-5, 319, 85, TFT_WHITE);
  tft.loadFont(FONT_SMALL, LittleFS); // загрузка в память шрифта
  tft.setTextDatum(TL_DATUM);
  h = tft.fontHeight();
  tft.setTextColor(TFT_YELLOW, TFT_BLACK);
  tft.drawString("ОСВІТЛЕННЯ:", xpos, ypos);
  if (ERROR1 | ERROR4 | ERROR10 | DHT_ERR){
    tft.setTextColor(TFT_RED, TFT_BLACK);
    tft.drawString("Датчик темпер.!", posErr, ypos);
  }
  
  ypos += (h+3);
  // DateTime time = rtc.now();
  // sprintf(displStr,"%2d:%02d Ув.%d Вим.%d", time.hour(), time.minute(), settings.timerOn, settings.water0off);
  sprintf(displStr,"Вим.%02d Уві.%02d", settings.timerOn, settings.timerOff);
  tft.setTextColor(TFT_WHITE, TFT_BLACK);
  tft.drawString(displStr, xpos, ypos);
  if (ERROR2 | ERROR8 | ERROR20 | DHT_ERR){
    tft.setTextColor(TFT_RED, TFT_BLACK);
    tft.drawString("Датчик вологи!", posErr, ypos);
  }

  ypos += (h+3);
  tft.setTextColor(TFT_CYAN, TFT_BLACK);
  sprintf(displStr,"ЗАСЛІНКА:%3d%%",pvFlap);
  tft.drawString(displStr, xpos, ypos);
  for (size_t i = 0; i < 6; i++){
    if (dataOut[i] != -1){
      tft.setTextColor(TFT_ORANGE, TFT_BLACK);
      tft.drawString("Ручне управління!", posErr, ypos);
      break;
    }
  }
  
  ypos += (h+3);
  tft.setTextColor(TFT_GREEN, TFT_BLACK);
  sprintf(displStr,"ПРОГРАМА:%d",settings.program);
  tft.drawString(displStr, xpos, ypos);
  if (RESERVE) tft.drawString("Інші помилки!", posErr, ypos);
  tft.unloadFont(); // выгрузка шрифта из памяти
}

void menu_1(){
  // if(newDispl){
  for (int i = 0; i < POINTS_1; i++) {
    keyLabel[i] = labelsMenu1[i];
    keyColor[i] = colorsMenu1[i];
  }
    tft.fillScreen(TFT_BLACK);
    tft.loadFont(FONT_SMALL, LittleFS); // загрузка в память шрифта
    MYDEBUG_PRINTLN("menu_1():Arial20");
    tft.setTextColor(TFT_ORANGE, TFT_BLACK);
    tft.setTextDatum(TC_DATUM);
    tft.drawString("Датчик температури Т1", 160, 5);
    drawKeypad_longName_12(keyLabel, keyColor, POINTS_1 / 2, 2);
    // MYDEBUG_PRINTLN(keyLabel[POINTS_1-1]);
    newDispl = false;
    // tft.unloadFont(); // выгрузка шрифта из памяти
  // }
}

void menu_2(){
  // if(newDispl){
  for (int i = 0; i < POINTS_2; i++) {
    keyLabel[i] = labelsMenu2[i];
    keyColor[i] = colorsMenu2[i];
  }
    tft.fillScreen(TFT_BLACK);
    tft.loadFont(FONT_SMALL, LittleFS); // загрузка в память шрифта
    MYDEBUG_PRINTLN("menu_2():Arial20");
    tft.setTextColor(TFT_ORANGE, TFT_BLACK);
    tft.setTextDatum(TC_DATUM);
    tft.drawString("Датчик вологості Т2", 160, 5);
    drawKeypad_longName_12(keyLabel, keyColor, POINTS_2 / 2, 2);
    // MYDEBUG_PRINTLN(keyLabel[POINTS_1-1]);
    newDispl = false;
    // tft.unloadFont(); // выгрузка шрифта из памяти
  // }
}

void menu_3(){
  // if(newDispl){
  for (int i = 0; i < POINTS_3; i++) {
    keyLabel[i] = labelsMenu3[i];
    keyColor[i] = colorsMenu3[i];
  }
    tft.fillScreen(TFT_BLACK);
    tft.loadFont(FONT_SMALL, LittleFS); // загрузка в память шрифта
    MYDEBUG_PRINTLN("menu_3():Arial20");
    tft.setTextColor(TFT_ORANGE, TFT_BLACK);
    tft.setTextDatum(TC_DATUM);
    tft.drawString("Таймери", 160, 5);
    drawKeypad_longName_12(keyLabel, keyColor, POINTS_3 / 2, 2);
    // MYDEBUG_PRINTLN(keyLabel[POINTS_2-1]);
    newDispl = false;
    // tft.unloadFont(); // выгрузка шрифта из памяти
  // }
}

void menu_4(){
  // if(newDispl){
  for (int i = 0; i < POINTS_4; i++) {
    keyLabel[i] = labelsMenu4[i];
    keyColor[i] = colorsMenu4[i];
  }
  keyLabel[0] = settings.modeHeater ? "режим: ОХОЛОДЖ." : "режим: НАГРІВ";
  keyLabel[1] = settings.modeHumidi ? "режим: ОСУШЕН." : "режим: ЗВОЛОЖ.";
    tft.fillScreen(TFT_BLACK);
    tft.loadFont(FONT_SMALL, LittleFS); // загрузка в память шрифта
    MYDEBUG_PRINTLN("menu_4():Arial20");
    tft.setTextColor(TFT_ORANGE, TFT_BLACK);
    tft.setTextDatum(TC_DATUM);
    tft.drawString("Системні", 160, 5);
    drawKeypad_longName_12(keyLabel, keyColor, POINTS_4 / 2, 2);
    // MYDEBUG_PRINTLN(keyLabel[POINTS_2-1]);
    newDispl = false;
    // tft.unloadFont(); // выгрузка шрифта из памяти
  // }
}

void calcDisplay(const char* txt){
  // Create 15 keys for the keypad
  // if(newDispl){
    for (int i = 0; i < 15; i++) {
      keyLabel[i] = labelsCalculator[i];
      keyColor[i] = colorCalculator[i];
    }  
    // Draw keypad background
    tft.fillScreen(TFT_DARKGREY);
    // Draw number display area and frame
    tft.fillRect(DISP_X, DISP_Y, DISP_W, DISP_H, TFT_BLACK);
    tft.drawRect(DISP_X, DISP_Y, DISP_W, DISP_H, TFT_WHITE);

    tft.loadFont(FONT_LARGE, LittleFS); // загрузка в память шрифта
    MYDEBUG_PRINTLN("calcDisplay():Arial28");
    tft.setTextColor(TFT_ORANGE, TFT_BLACK);
    tft.setTextDatum(TC_DATUM);
    tft.drawString(txt, DISP_W/2, DISP_Y + 5);
    tft.unloadFont();
    MYDEBUG_PRINTLN("calcDisplay():unloadFont");
    drawKeypad(keyLabel, keyColor);
    newDispl = false;
  // }
}

void drawKeypad_longName_7(const char* keyLabel[], uint16_t keyColor[], uint8_t amt_row, uint8_t amt_col){
  const char* nul = "";
  uint16_t key_x = 160, key_y = 50, key_w = 300, key_h = 25;
  // char dd[50];
  for (uint8_t row = 0; row < amt_row; row++) {
    if(row < amt_row-1){
      for (uint8_t col = 0; col < amt_col; col++) {
        uint8_t b = col + row * amt_col;
        key[b].initButton(&tft, key_x + col * (key_w + 5),
                          key_y + row * (key_h + 5), // x, y, w, h, outline, fill, text
                          key_w, key_h, TFT_WHITE, keyColor[b], TFT_BLACK,
                          (char*)nul, KEY_TEXTSIZE);
        String string = String(keyLabel[b]);
        key[b].drawButton(false, string);
        MYDEBUG_PRINT("b=="); MYDEBUG_PRINT(b); MYDEBUG_PRINT("  str:"); MYDEBUG_PRINTLN(string);
        // sprintf(dd,"X=%i Xw=%i Y=%i Yh=%i",160 + col * (300 + 5),300,50 + row * (25 + 5),25);
        // MYDEBUG_PRINTLN(dd);
      }
    }else{
      for (uint8_t col = 0; col < 2; col++) {
        uint8_t b = col + row;
        key[b].initButton(&tft, key_x/2 + 2 + col * (key_w/2+4),
                          key_y + row * (key_h + 5), // x, y, w, h, outline, fill, text
                          key_w/2 - 5, key_h, TFT_WHITE, keyColor[b], TFT_BLACK,
                          (char*)nul, KEY_TEXTSIZE);
        String string = String(keyLabel[b]);
        key[b].drawButton(false, string);
        MYDEBUG_PRINT("b=="); MYDEBUG_PRINT(b); MYDEBUG_PRINT("  str:"); MYDEBUG_PRINTLN(string);
        // sprintf(dd,"X=%i Xw=%i Y=%i Yh=%i",80 + col * (145 + 5),145,50 + row * (25 + 5),25);
        // MYDEBUG_PRINTLN(dd);
      }
    }
  }
  MYDEBUG_PRINTLN("=======================================================");
}

void drawKeypad_longName_12(const char* keyLabel[], uint16_t keyColor[], uint8_t amt_row, uint8_t amt_col){
  const char* nul = "";
  uint16_t key_x = 160/amt_col, key_y = 50, key_w = 300/amt_col, key_h = 25;
  for (uint8_t row = 0; row < amt_row; row++) {
    for (uint8_t col = 0; col < amt_col; col++) {
      uint8_t b = col + row * amt_col;
// TFT_eSPI *gfx, int16_t x, int16_t y, uint16_t w, uint16_t h, uint16_t outline, uint16_t fill, uint16_t textcolor, char *label, uint8_t textsize
      key[b].initButton(&tft, key_x + col * (key_w + 5),
                        key_y + row * (key_h + 5), // x, y, w, h, outline, fill, text
                        key_w, key_h, TFT_WHITE, keyColor[b], TFT_BLACK,
                        (char*)nul, KEY_TEXTSIZE);
      String string = String(keyLabel[b]);
      key[b].drawButton(false, string);
      MYDEBUG_PRINT("b=="); MYDEBUG_PRINT(b); MYDEBUG_PRINT("  str:"); MYDEBUG_PRINTLN(string);
    }
  }
  MYDEBUG_PRINTLN("=======================================================");
}

void drawKeypad(const char* keyLabel[], uint16_t keyColor[]){
  tft.setFreeFont(LABEL2_FONT);
  for (uint8_t row = 0; row < 3; row++) {
    for (uint8_t col = 0; col < 5; col++) {
      uint8_t b = col + row * 5;
// TFT_eSPI *gfx, int16_t x, int16_t y, uint16_t w, uint16_t h, uint16_t outline, uint16_t fill, uint16_t textcolor, char *label, uint8_t textsize
      key[b].initButton(&tft, KEY_X + col * (KEY_W + KEY_SPACING_X),
                        KEY_Y + row * (KEY_H + KEY_SPACING_Y), // x, y, w, h, outline, fill, text
                        KEY_W, KEY_H, TFT_WHITE, keyColor[b], TFT_BLACK,
                        (char*)keyLabel[b], KEY_TEXTSIZE);
      // key[b].drawButton();
      String string = String(keyLabel[b]);
      key[b].drawButton(false, string);
    }
  }
}

uint16_t lampUpdate(uint16_t xpos, uint16_t ypos){
    uint16_t txt_width, h;
    bool on = false;
    tft.loadFont("Calibri14", LittleFS); // загрузка в память шрифта
    h = tft.fontHeight()+4;
    // tft.fillRect(xpos-10, ypos-4, 310, h+4, TFT_DARKGREY);
    // tft.drawRect(xpos-10, ypos-4, 310, h+4, TFT_MAGENTA);
    //----------
    tft.setCursor(xpos-5, ypos);
    on = LIGHT ? false : true;
    if(on) tft.setTextColor(TFT_BLACK, TFT_GREEN, true);
    else tft.setTextColor(TFT_BLACK, TFT_BLACK, true);
    tft.print(" ОСВІТЛ ");
    txt_width = tft.textWidth(" ОСВІТЛ ");
    xpos += txt_width+5;
    //---------
    tft.setCursor(xpos, ypos);
    on = HEATER ? false : true;
    if(on) tft.setTextColor(TFT_BLACK, TFT_ORANGE, true);
    else tft.setTextColor(TFT_BLACK, TFT_BLACK, true);
    tft.print(" НАГРІВ ");
    txt_width = tft.textWidth(" НАГРІВ ");
    xpos += txt_width+5;
    //----------
    tft.setCursor(xpos, ypos);
    on = HUMIDI ? false : true;
    if(on) tft.setTextColor(TFT_BLACK, TFT_CYAN, true);
    else tft.setTextColor(TFT_BLACK, TFT_BLACK, true);
    tft.print(" ЗВОЛОЖ ");
    txt_width = tft.textWidth(" ЗВОЛОЖ ");
    xpos += txt_width+5;
    //----------
    tft.setCursor(xpos, ypos);
    on = RELAY1 ? false : true;
    if(on) tft.setTextColor(TFT_BLACK, TFT_GREENYELLOW, true);
    else tft.setTextColor(TFT_BLACK, TFT_BLACK, true);
    tft.print(" РЕЛ1 ");
    txt_width = tft.textWidth(" РЕЛ1 ");
    xpos += txt_width+5;
    //----------
    tft.setCursor(xpos, ypos);
    on = RELAY2 ? false : true;
    if(on) tft.setTextColor(TFT_BLACK, TFT_GREENYELLOW, true);
    else tft.setTextColor(TFT_BLACK, TFT_BLACK, true);
    tft.print(" РЕЛ2 ");
    txt_width = tft.textWidth(" РЕЛ2 ");
    xpos += txt_width+5;
    //----------
    tft.setCursor(xpos, ypos);
    on = RELAY3 ? false : true;
    if(on) tft.setTextColor(TFT_BLACK, TFT_GREENYELLOW, true);
    else tft.setTextColor(TFT_BLACK, TFT_BLACK, true);
    tft.print(" РЕЛ3 ");
    txt_width = tft.textWidth(" РЕЛ3 ");
    xpos += txt_width+5;
    ypos += h;
    tft.unloadFont(); // выгрузка шрифта из памяти
    return ypos;
}

void waitForTouchRelease() {
  uint16_t tx, ty;
  while (tft.getTouch(&tx, &ty)) {
    delay(10);
    ESP.wdtFeed();
  }
}
