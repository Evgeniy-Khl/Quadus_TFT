#include "main.h"
#include <tftArcFill.h>

// #########################################################################
// Draw a circular or elliptical arc with a defined thickness
// #########################################################################

// x,y == coords of centre of arc
// start_angle = 0 - 359
// seg_count = number of 6 degree segments to draw (60 => 360 degree arc)
// rx = x axis outer radius
// ry = y axis outer radius
// w  = width (thickness) of arc in pixels
// colour = 16-bit colour value
// Note if rx and ry are the same then an arc of a circle is drawn

void fillArc(int x, int y, int start_angle, int seg_count, int rx, int ry, int w, unsigned int colour)
{

  uint8_t seg = 6; // Segments are 3 degrees wide = ypos segments for 360 degrees
  uint8_t inc = 6; // Draw segments every 3 degrees, increase to 6 for segmented ring

  // Calculate first pair of coordinates for seg_w start
  float sx = cos((start_angle - 210) * DEG2RAD);
  float sy = sin((start_angle - 210) * DEG2RAD);
  uint16_t x0 = sx * (rx - w) + x;
  uint16_t y0 = sy * (ry - w) + y;
  uint16_t x1 = sx * rx + x;
  uint16_t y1 = sy * ry + y;

  // Draw colour blocks every inc degrees
  for (int i = start_angle; i < start_angle + seg * seg_count; i += inc) {

    // Calculate pair of coordinates for seg_w end
    float sx2 = cos((i + seg - 210) * DEG2RAD);
    float sy2 = sin((i + seg - 210) * DEG2RAD);
    int x2 = sx2 * (rx - w) + x;
    int y2 = sy2 * (ry - w) + y;
    int x3 = sx2 * rx + x;
    int y3 = sy2 * ry + y;

    tft.fillTriangle(x0, y0, x1, y1, x2, y2, colour);
    tft.fillTriangle(x1, y1, x2, y2, x3, y3, colour);

    // Copy seg_w end to seg_w start for next seg_w
    x0 = x2;
    y0 = y2;
    x1 = x3;
    y1 = y3;
  }
}

//#########################################################################
void diagram(GrafDispl grafDispl, uint16_t color){
  char tempStr[10]; // Буфер для строки температуры
  uint8_t seg_w = 20;
  uint16_t tmpval0,tmpval1, maxtemp, mintemp, lightBlue, greenValue, yellowValue, redValue;
  if(grafDispl.xpos - grafDispl.radius < 0) grafDispl.xpos = grafDispl.radius;
  if(grafDispl.xpos + grafDispl.radius > tft.width()) grafDispl.xpos = tft.width() - grafDispl.radius;
  if(grafDispl.ypos - grafDispl.radius < 0) grafDispl.ypos  = grafDispl.radius;
  if(grafDispl.ypos + grafDispl.radius > tft.height()) grafDispl.ypos = tft.height() - grafDispl.radius;
  if(grafDispl.radius < 60) grafDispl.radius = 60;
  
  lightBlue   = grafDispl.spOn -  50;  // 170 (grafDispl.spOn = 220)
  greenValue  = grafDispl.spOn +   5;  // 225
  yellowValue = grafDispl.spOff + 10;  // 250 (grafDispl.spOn = 240)
  redValue    = grafDispl.spOff + 10 + 50; // 300
  maxtemp = redValue  + 50; // 350
  mintemp = lightBlue - 50; // 70

  tmpval1 = map(lightBlue, mintemp, maxtemp, 0, 240);
  fillArc(grafDispl.xpos, grafDispl.ypos, 0, tmpval1/6, grafDispl.radius, grafDispl.radius, seg_w, TFT_BLUE);
  tmpval0 = tmpval1-5;
  tmpval1 = map(greenValue, mintemp, maxtemp, 0, 240);
  fillArc(grafDispl.xpos, grafDispl.ypos, tmpval0, (tmpval1-tmpval0)/6, grafDispl.radius, grafDispl.radius, seg_w, TFT_CYAN);
  tmpval0 = tmpval1-5;
  tmpval1 = map(yellowValue, mintemp, maxtemp, 0, 240);
  fillArc(grafDispl.xpos, grafDispl.ypos, tmpval0, (tmpval1-tmpval0)/6, grafDispl.radius, grafDispl.radius, seg_w, TFT_DARKGREEN);
  tmpval0 = tmpval1-5;
  tmpval1 = map(redValue, mintemp, maxtemp, 0, 240);
  fillArc(grafDispl.xpos, grafDispl.ypos, tmpval0, (tmpval1-tmpval0)/6, grafDispl.radius, grafDispl.radius, seg_w, TFT_ORANGE);
  tmpval0 = tmpval1-5;
  tmpval1 = map(maxtemp, mintemp, maxtemp, 0, 240);
  fillArc(grafDispl.xpos, grafDispl.ypos, tmpval0, (tmpval1-tmpval0)/6, grafDispl.radius, grafDispl.radius, seg_w, TFT_RED);

  fillArc(grafDispl.xpos, grafDispl.ypos, 0, 40, grafDispl.radius-20, grafDispl.radius-20, seg_w-8, TFT_BLACK);
  tmpval0 = grafDispl.value;
  if(tmpval0 < mintemp) tmpval0 = mintemp;                // 450
  else if(tmpval0 > (maxtemp-30)) tmpval0 = (maxtemp-30); // 930
  tmpval1 = map(tmpval0, mintemp, maxtemp, 0, 240);
  fillArc(grafDispl.xpos, grafDispl.ypos, tmpval1, 1, grafDispl.radius-7, grafDispl.radius-7, seg_w+3, TFT_WHITE);
    //-----------------------
    // tft.setTextSize(1);
    tft.setTextPadding(0);
    tft.setTextDatum(MC_DATUM);
    //-----------------------
    if(grafDispl.value<1000) sprintf(tempStr,"%2.1f°C",(float)grafDispl.value/10);
    else if(grafDispl.value<1270) sprintf(tempStr,"%5d°C", grafDispl.value/10);
    else sprintf(tempStr," ---  ");
    // tft.fillRect(grafDispl.xpos-40, grafDispl.ypos-15, 80, 25, TFT_BLACK);
    // tft.setTextColor(TFT_WHITE);
    tft.loadFont(FONT_LARGE, LittleFS); // загрузка в память шрифта
    tft.setTextColor(TFT_WHITE,TFT_BLACK,true);
    tft.drawString(tempStr, grafDispl.xpos, grafDispl.ypos-5, 4);
    //-----------------------
    sprintf(tempStr," %2.1f°C ",(float)grafDispl.spOff/10);
    // tft.fillRect(grafDispl.xpos-40, grafDispl.ypos+10, 80, 25, TFT_WHITE);
    // tft.setTextColor(TFT_BLACK);
    tft.loadFont(FONT_MINI, LittleFS); // загрузка в память шрифта
    tft.setTextColor(TFT_BLACK,TFT_ORANGE,true);
    tft.drawString(tempStr, grafDispl.xpos, grafDispl.ypos+15, 4);
    //-----------------------
    sprintf(tempStr," %2.1f°C ",(float)grafDispl.spOn/10);
    // tft.fillRect(grafDispl.xpos-40, grafDispl.ypos+10, 80, 25, TFT_WHITE);
    // tft.setTextColor(TFT_BLACK);
    tft.setTextColor(TFT_BLACK,TFT_CYAN,true);
    tft.drawString(tempStr, grafDispl.xpos, grafDispl.ypos+30, 4);
}

