#include "main.h"
#include "display.h"
#include "tftArcFill.h"

TFT_eSPI_Button key[15];

// Набор подписей
const char* labelsMenu1[MENU_1] = {
  "завдання у грд.Цесія", 
  "завдання у відсотках", 
  "аварійне відхилення", 
  "охолодж./осушування увімк.", 
  "охолодж./осушування вимкн.",
  "повернуться", "наступне"
};

uint16_t colorsMenu1[MENU_1] = {
  TFT_WHITE, TFT_WHITE, TFT_WHITE, TFT_WHITE, TFT_WHITE, TFT_YELLOW, TFT_GREEN
};

// Набор подписей
const char* labelsMenu2[MENU_2] = {
  "лотки увім.", "лотки вимкн.", 
  "провітр. пауза", "провітр. робота",
  "допоміж. увім.", "допоміж. вимкн.",
  "заслінка полож.", "задана програма",
  "заслінка закр.", "заслінка відкр.", 
  "повернуться", "наступне"
};

uint16_t colorsMenu2[MENU_2] = {
  TFT_WHITE, TFT_WHITE, TFT_WHITE, TFT_WHITE, TFT_WHITE,
  TFT_WHITE, TFT_WHITE, TFT_WHITE, TFT_WHITE, TFT_WHITE,
  TFT_YELLOW, TFT_GREEN
};

// Набор подписей 
const char* labelsMenu3[MENU_3] = {
  "імпульс мінім.", "імпульс максім.",
  "період імпульс.", "аварійн. режим", 
  "режим реле", "затрим. зволож.",
  "пропорц. 1", "пропорц. 2",
  "ітеграл. 1", "ітеграл. 2", 
  "повернуться", "наступне"
};

uint16_t colorsMenu3[MENU_3] = {
  TFT_WHITE, TFT_WHITE, TFT_WHITE, TFT_WHITE, TFT_WHITE,
  TFT_WHITE, TFT_WHITE, TFT_WHITE, TFT_WHITE, TFT_WHITE,
  TFT_YELLOW, TFT_GREEN
};

// Набор подписей display_3
const char* labelsCalculator[15] = {
  "+1","+5","+10","+50","+100","-1","-5","-10","-50","-100","","","","X","Ok"
};
uint16_t color_for_display_3[15] = {
  TFT_CYAN, TFT_CYAN, TFT_CYAN, TFT_CYAN,
  TFT_CYAN, TFT_CYAN, TFT_CYAN, TFT_CYAN, 
  TFT_CYAN, TFT_CYAN, TFT_BLACK, TFT_BLACK,
  TFT_BLACK, TFT_RED, TFT_GREEN

};
//--------- ОСНОВНОЙ ЭКРАН ----------------------
void mainDispl(void){
  uint16_t h;
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
  h = lampUpdate(15, 130);
//-----------
  tft.setTextPadding(310);
  xpos = 5; ypos = h+8;
  tft.drawRect(xpos-5, ypos-4, 319, 70, TFT_WHITE);
  tft.loadFont(FONT_SMALL, LittleFS); // загрузка в память шрифта
  tft.setTextDatum(TL_DATUM);
  h = tft.fontHeight();
  tft.setTextColor(TFT_WHITE, TFT_BLACK);
  sprintf(displStr,"РЕЖИМ: Р=%g  І=%g Н=%g", pid[0].pPart/500, pid[0].iPart/10, heaterValue);
  // w = tft.textWidth("РЕЖИМ:");
  // tft.fillRect(xpos+w, ypos, tft.width()-(xpos+w), h, TFT_BLACK);
  tft.drawString(displStr, xpos, ypos);
  // tft.fillRect(xpos + w, ypos, tft.width() - w, h, TFT_BLACK);

  ypos += (h+3);
  tft.setTextColor(TFT_CYAN, TFT_BLACK);
  sprintf(displStr,"до повороту лотків: %3d сек.",seconds);
  // w = tft.textWidth("ПОВОРОТ:");
  // tft.fillRect(xpos+w, ypos, tft.width()-(xpos+w), h, TFT_BLACK);
  tft.drawString(displStr, xpos, ypos);
  // tft.fillRect(xpos + w, ypos, tft.width() - w, h, TFT_BLACK);

  ypos += (h+3);
  tft.setTextColor(TFT_YELLOW, TFT_BLACK);
  sprintf(displStr,"ІНКУБАЦІЯ: програма:0 доба:%3d",seconds);
  // w = tft.textWidth("ІНКУБАЦІЯ:");
  // tft.fillRect(xpos+w, ypos, tft.width()-(xpos+w), h, TFT_BLACK);
  tft.drawString(displStr, xpos, ypos);
  // tft.fillRect(xpos + w, ypos, tft.width() - w, h, TFT_BLACK);
  tft.unloadFont(); // выгрузка шрифта из памяти
}

void menu_1(){
  // if(newDispl){
    for (int i = 0; i < MENU_1; i++) {
      keyLabel[i] = labelsMenu1[i];
      keyColor[i] = colorsMenu1[i];
    }
    tft.fillScreen(TFT_BLACK);
    tft.loadFont(FONT_SMALL, LittleFS); // загрузка в память шрифта
    DEBUG_PRINTLN("menu_1():Arial20");
    tft.setTextColor(TFT_ORANGE, TFT_BLACK);
    tft.setTextDatum(TC_DATUM);
    tft.drawString("Сухий датчик канал 1", 160, 5);
    drawKeypad_longName_7(keyLabel, keyColor, MENU_1-1, 1);
    // DEBUG_PRINTLN(keyLabel[MENU_1-1]);
    newDispl = false;
    // tft.unloadFont(); // выгрузка шрифта из памяти
  // }
}

void menu_2(){
  // if(newDispl){
    for (int i = 0; i < MENU_1; i++) {
      keyLabel[i] = labelsMenu1[i];
      keyColor[i] = colorsMenu1[i];
    }
    tft.fillScreen(TFT_BLACK);
    tft.loadFont(FONT_SMALL, LittleFS); // загрузка в память шрифта
    DEBUG_PRINTLN("menu_2():Arial20");
    tft.setTextColor(TFT_ORANGE, TFT_BLACK);
    tft.setTextDatum(TC_DATUM);
    tft.drawString("Вологий датчик канал 2", 160, 5);
    drawKeypad_longName_7(keyLabel, keyColor, MENU_1-1, 1);
    // DEBUG_PRINTLN(keyLabel[MENU_1-1]);
    newDispl = false;
    // tft.unloadFont(); // выгрузка шрифта из памяти
  // }
}

void menu_3(){
  // if(newDispl){
    for (int i = 0; i < MENU_2; i++) {
      keyLabel[i] = labelsMenu2[i];
      keyColor[i] = colorsMenu2[i];
    }
    tft.fillScreen(TFT_BLACK);
    tft.loadFont(FONT_SMALL, LittleFS); // загрузка в память шрифта
    DEBUG_PRINTLN("menu_3():Arial20");
    tft.setTextColor(TFT_ORANGE, TFT_BLACK);
    tft.setTextDatum(TC_DATUM);
    tft.drawString("Загальні параметри", 160, 5);
    drawKeypad_longName_12(keyLabel, keyColor, MENU_2/2, 2);
    // DEBUG_PRINTLN(keyLabel[MENU_2-1]);
    newDispl = false;
    // tft.unloadFont(); // выгрузка шрифта из памяти
  // }
}

void menu_4(){
  // if(newDispl){
    for (int i = 0; i < MENU_3; i++) {
      keyLabel[i] = labelsMenu3[i];
      keyColor[i] = colorsMenu3[i];
    }
    tft.fillScreen(TFT_BLACK);
    tft.loadFont(FONT_SMALL, LittleFS); // загрузка в память шрифта
    DEBUG_PRINTLN("menu_4():Arial20");
    tft.setTextColor(TFT_ORANGE, TFT_BLACK);
    tft.setTextDatum(TC_DATUM);
    tft.drawString("Технічні параметри", 160, 5);
    drawKeypad_longName_12(keyLabel, keyColor, MENU_3/2, 2);
    // DEBUG_PRINTLN(keyLabel[MENU_2-1]);
    newDispl = false;
    // tft.unloadFont(); // выгрузка шрифта из памяти
  // }
}

void calcDisplay(const char* txt){
  // Create 15 keys for the keypad
  // if(newDispl){
    for (int i = 0; i < 15; i++) {
      keyLabel[i] = labelsCalculator[i];
      keyColor[i] = color_for_display_3[i];
    }  
    // Draw keypad background
    tft.fillScreen(TFT_DARKGREY);
    // Draw number display area and frame
    tft.fillRect(DISP_X, DISP_Y, DISP_W, DISP_H, TFT_BLACK);
    tft.drawRect(DISP_X, DISP_Y, DISP_W, DISP_H, TFT_WHITE);

    tft.loadFont(FONT_LARGE, LittleFS); // загрузка в память шрифта
    DEBUG_PRINTLN("calcDisplay():Arial28");
    tft.setTextColor(TFT_ORANGE, TFT_BLACK);
    tft.setTextDatum(TC_DATUM);
    tft.drawString(txt, DISP_W/2, DISP_Y + 5);
    tft.unloadFont();
    DEBUG_PRINTLN("calcDisplay():unloadFont");
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
        DEBUG_PRINT("b=="); DEBUG_PRINT(b); DEBUG_PRINT("  str:"); DEBUG_PRINTLN(string);
        // sprintf(dd,"X=%i Xw=%i Y=%i Yh=%i",160 + col * (300 + 5),300,50 + row * (25 + 5),25);
        // DEBUG_PRINTLN(dd);
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
        DEBUG_PRINT("b=="); DEBUG_PRINT(b); DEBUG_PRINT("  str:"); DEBUG_PRINTLN(string);
        // sprintf(dd,"X=%i Xw=%i Y=%i Yh=%i",80 + col * (145 + 5),145,50 + row * (25 + 5),25);
        // DEBUG_PRINTLN(dd);
      }
    }
  }
  DEBUG_PRINTLN("=======================================================");
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
      DEBUG_PRINT("b=="); DEBUG_PRINT(b); DEBUG_PRINT("  str:"); DEBUG_PRINTLN(string);
    }
  }
  DEBUG_PRINTLN("=======================================================");
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
    tft.loadFont("Calibri14"); // загрузка в память шрифта
    h = tft.fontHeight()+4;
    tft.fillRect(xpos-10, ypos-4, 310, h+4, TFT_DARKGREY);
    tft.drawRect(xpos-10, ypos-4, 310, h+4, TFT_MAGENTA);
    tft.setCursor(xpos, ypos);
    on = seconds&1 ? true : false;
    if(on) tft.setTextColor(TFT_BLACK, TFT_ORANGE, true);
    else tft.setTextColor(TFT_BLACK, TFT_BLACK, true);
    tft.print(" НАГРІВ ");
    txt_width = tft.textWidth(" НАГРІВ ");
    xpos += txt_width+5;
    //----------
    tft.setCursor(xpos, ypos);
    on = seconds&2 ? true : false;
    if(on) tft.setTextColor(TFT_BLACK, TFT_CYAN, true);
    else tft.setTextColor(TFT_BLACK, TFT_BLACK, true);
    tft.print(" ЗВОЛОЖ ");
    txt_width = tft.textWidth(" ЗВОЛОЖ ");
    xpos += txt_width+5;
    //----------
    tft.setCursor(xpos, ypos);
    on = seconds&4 ? true : false;
    if(on) tft.setTextColor(TFT_BLACK, TFT_GREEN, true);
    else tft.setTextColor(TFT_BLACK, TFT_BLACK, true);
    tft.print(" ПОВОРОТ ");
    txt_width = tft.textWidth(" ПОВОРОТ ");
    xpos += txt_width+5;
    //----------
    tft.setCursor(xpos, ypos);
    on = seconds&8 ? true : false;
    if(on) tft.setTextColor(TFT_BLACK, TFT_GREENYELLOW, true);
    else tft.setTextColor(TFT_BLACK, TFT_BLACK, true);
    tft.print(" ОХЛ ");
    txt_width = tft.textWidth(" ОХЛ ");
    xpos += txt_width+5;
    //----------
    tft.setCursor(xpos, ypos);
    on = seconds&0x10 ? true : false;
    if(on) tft.setTextColor(TFT_BLACK, TFT_GREENYELLOW, true);
    else tft.setTextColor(TFT_BLACK, TFT_BLACK, true);
    tft.print(" ДОП ");
    txt_width = tft.textWidth(" ДОП ");
    xpos += txt_width+5;
    //----------
    tft.setCursor(xpos, ypos);
    on = seconds&0x20 ? true : false;
    if(on) tft.setTextColor(TFT_YELLOW, TFT_RED, true);
    else tft.setTextColor(TFT_BLACK, TFT_BLACK, true);
    tft.print(" АВР ");
    txt_width = tft.textWidth(" АВР ");
    xpos += txt_width+5;
    ypos += h;
    tft.unloadFont(); // выгрузка шрифта из памяти
    return ypos;
}
