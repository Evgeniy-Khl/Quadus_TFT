#include "main.h"
#include "tftProcessing.h"

void initMyFont(void){
    uint16_t h;
    // tft.init();
    // tft.setRotation(0);
    // tft.fillScreen(TFT_BLACK);

    // инициализация LittleFS
    if (!LittleFS.begin()) {
        DEBUG_PRINTLN("ERROR file system!");
    }
    xpos = 0; ypos = 150;
    /* tft.loadFont("Arial14"); // загрузка в память шрифта
    tft.setCursor(x_pos, y_pos);
    tft.setTextColor(TFT_YELLOW);
    tft.println("АаБбВвГгДдЖжЗзИиКкЛлМмНнОоПпРрСсТтУуФфХхЧчШшЩщІЇіїЄє");
    h = (tft.fontHeight()+5);
    y_pos += 2*h;
    tft.unloadFont(); // выгрузка шрифта из памяти */
    //---------------------------------------------------------
    /* tft.loadFont("Arial18"); // загрузка в память шрифта
    tft.setCursor(x_pos, y_pos);
    tft.setTextColor(TFT_YELLOW);
    tft.println("АаБбВвГгДдЖжЗзИиКкЛлМмНнОоПпРрСсТтУуФфХхЧчШшЩщІЇіїЄє");
    h = (tft.fontHeight()+5);
    y_pos += 3*h;
    tft.unloadFont(); // выгрузка шрифта из памяти */
    //---------------------------------------------------------
    tft.loadFont(FONT_SMALL, LittleFS); // загрузка в память шрифта
    tft.setCursor(xpos, ypos);
    tft.setTextColor(TFT_YELLOW);
    tft.println("АаБбВвГгДдЖжІЇіїЄє");  // ЗзИиКкЛлМмНнОоПпРрСсТтУуФфХхЧчШшЩщ
    h = (tft.fontHeight()+5);
    ypos += 1*h;
    tft.unloadFont(); // выгрузка шрифта из памяти
    //---------------------------------------------------------
    tft.loadFont("Arial24"); // загрузка в память шрифта
    tft.setCursor(xpos, ypos);
    tft.setTextColor(TFT_YELLOW);
    tft.println("АаБбВвГгДдЖжІЇіїЄє");  // ЗзИиКкЛлМмНнОоПпРрСсТтУуФфХхЧчШшЩщ
    h = (tft.fontHeight()+5);
    ypos += 2*h;
    tft.unloadFont(); // выгрузка шрифта из памяти
    //---------------------------------------------------------
    // tft.loadFont(FONT_LARGE, LittleFS); // загрузка в память шрифта
    // tft.setCursor(x_pos, y_pos);
    // tft.setTextColor(TFT_YELLOW);
    // tft.println("АаБбВвГгДдЖжЗзИиКкЛлМмНнОоПпРрСсТтУуФфХхЧчШшЩщІЇіїЄє28%");
    // y_pos += tft.fontHeight();
    // tft.unloadFont(); // выгрузка шрифта из памяти
    //---------------------------------------------------------
    // x_pos = tft.width() / 2; // Half the screen width
    // tft.loadFont("Calibri78"); // загрузка в память шрифта
    // tft.setCursor(x_pos, y_pos);
    // tft.setTextColor(TFT_ORANGE);
    // tft.println("67%");
    // y_pos += tft.fontHeight();
    // tft.setCursor(x_pos, y_pos);
    // tft.println("999999");
    // tft.unloadFont(); // выгрузка шрифта из памяти
    //==============================================
}