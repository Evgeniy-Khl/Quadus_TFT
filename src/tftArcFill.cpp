#include "main.h"
#include <tftArcFill.h>

byte inc = 0;
unsigned int col = 0;

byte red = 31; // Red is the top 5 bits of a 16-bit colour value
byte green = 0;// Green is the middle 6 bits
byte blue = 0; // Blue is the bottom 5 bits
byte state = 0;


void initMyConfig(){
  grafDispl[0].value = ds[0].pvT;
  grafDispl[0].sp = settings.sp_structs[0].spT;
  grafDispl[1].value = ds[1].pvT;
  grafDispl[1].sp = settings.sp_structs[1].spT;
  xpos = tft.width()/2; ypos = tft.height()/2-80;
  tft.setTextDatum(TC_DATUM);
  tft.setTextColor(TFT_ORANGE, TFT_BLACK);
  tft.loadFont(FONT_LARGE, LittleFS); // загрузка в память шрифта
  tft.drawString("КЛІМАТ-5.25", xpos, ypos);
  xpos = 0; ypos += 40;
  tft.loadFont(FONT_SMALL, LittleFS); // загрузка в память шрифта
  tft.setTextDatum(TL_DATUM);
  //--------- Загрузка конфигурации --------------------------------------------
  if(LittleFS.exists("/setpoint.json")){
    if(!loadConfig()){
      tft.setTextColor(TFT_RED, TFT_BLACK);
      tft.drawString("Конфігурація не завантажена!", xpos, ypos);
    }
  }
  else {
    saveConfig();  // Сохраним эти значения в файл
    tft.setTextColor(TFT_GREEN, TFT_BLACK);
    tft.drawString("Конфігурація за замовчуванням!", xpos, ypos);
  }
  xpos = 0; ypos += 25;
  DEBUG_PRINTLN("\n>> Итоговые значения после загрузки из FS:");
  #ifdef DEBUG
    printConfig();
  #endif
  //--------- инициализация PID --------------------------------------------
  PID_Init(&pid[0], settings.sp_structs[0].Kp, settings.sp_structs[0].Ki);
  tft.setTextColor(TFT_YELLOW, TFT_BLACK);
  sprintf(displStr,"Пропорц.= %g  Ітеграл.= %g", pid[0].Kp,pid[0].Ki);
  tft.drawString(displStr, xpos, ypos, 2);
  xpos = 0; ypos += 25;
  //------------------------------------------------------------------------
  /* DEBUG_PRINTLN("\n");
  uint32_t realSize = ESP.getFlashChipRealSize(); // Получаем реальный размер flash
  uint32_t ideSize = ESP.getFlashChipSize();    // Получаем размер, установленный в IDE
  FlashMode_t ideMode = ESP.getFlashChipMode();

  Serial.printf("Flash real id:   %08X\n", ESP.getFlashChipId());
  Serial.printf("Flash real size: %u bytes\n\n", realSize);

  Serial.printf("Flash ide  size: %u bytes\n", ideSize);
  Serial.printf("Flash ide speed: %u Hz\n", ESP.getFlashChipSpeed());
  Serial.printf("Flash ide mode:  %s\n", (ideMode == FM_QIO ? "QIO" : ideMode == FM_QOUT ? "QOUT" : ideMode == FM_DIO ? "DIO" : ideMode == FM_DOUT ? "DOUT" : "UNKNOWN"));

  if (ideSize != realSize) {
    DEBUG_PRINTLN("Внимание! Размер Flash, установленный в IDE, не совпадает с реальным!");
  } else {
    DEBUG_PRINTLN("Размер Flash в IDE совпадает с реальным.");
  }
  DEBUG_PRINTLN(); */

/* 
  //---------- Изменяем яркость светодиода ----------------------------------------
  // Пин, к которому подключен светодиод (GPIO2)
  pinMode(LEDPIN, OUTPUT);    // Устанавливаем пин светодиода как выход
  // Можно установить желаемую частоту ШИМ (опционально)
  // analogWriteFreq(1000);   // По умолчанию и так 1000 Гц
  // Можно установить желаемый диапазон (опционально)
  analogWriteRange(255);      // Если хотите диапазон 0-255
  //===============================================================================
 */

  Wire.begin();               // Инициализация I2C (SDA, SCL по умолчанию для ESP8266 - GPIO4, GPIO5)
  // Wire.begin(D2, D1);      // Если вы хотите использовать другие пины для I2C (например, D2 для SDA, D1 для SCL)
  //--------------------- Инициализация PCF8574 ----------------------------------
  /* Пример: Установить все пины PCF8574 как выходы и выключить их (записать 0)
            Для PCF8574, чтобы использовать пин как "выход", мы просто записываем в него значение.
            Чтобы использовать пин как "вход", мы записываем в него '1' (высокий уровень),
            а затем читаем состояние. Внутренние подтягивающие резисторы слабые. 
  */
  writePCF8574(0x00);         // Установить все пины в LOW (если они используются как выходы)

  //---------- Инициализация DS3231 ----------------------------------------
  if (rtc.begin()) {
    DEBUG_PRINTLN("RTC found!");
    tft.setTextColor(TFT_WHITE, TFT_BLACK);
    tft.drawString("Знайден годинник реального часу!", xpos, ypos, 2);
    xpos = 0; ypos += 25;
  }
  //------------------------------------------------------------------------------
  // testAT24C32();              // тест
  // tft.drawString("AT24C32 test complete.", xpos, ypos, 2);
  // xpos = 0; ypos += 20;
  //==============================================================================

  //------------ Инициализация библиотеки DallasTemperature -----------------------------
  sensors.begin();
  sensors.setWaitForConversion(false);    // false: функция вернет управление немедленно.
  sensors.setCheckForConversion(false);   // Часто используется вместе с waitForConversion = false
  sensors.setAutoSaveScratchPad(false);   // Флаг автоматического сохранения настроек в EEPROM датчика.
  sensors.setResolution(12);// Устанавливаем разрешение для всех датчиков (9, 10, 11, or 12 бит)

  // Поиск устройств на шине 1-Wire
  numberOfDevices = sensors.getDeviceCount();
  if(numberOfDevices > MAX_DEVICE) numberOfDevices = MAX_DEVICE;
  // data[0] = NUMBER_FONT[numberOfDevices]; // отображение числа датчиков на дисплее
  DEBUG_PRINT("Found ");
  DEBUG_PRINT(numberOfDevices, DEC);
  DEBUG_PRINTLN(" devices.");
  tft.setTextColor(TFT_WHITE, TFT_BLACK);
  tft.drawString("Датчиків температури - ", xpos, ypos, 2);
  xpos = 220;
  sprintf(displStr,"%d шт.", numberOfDevices);
  tft.drawString(displStr, xpos, ypos, 2);
  xpos = 0; ypos += 25;

  #ifdef DEBUG
    if (numberOfDevices == 0) {
      DEBUG_PRINTLN("No DS18B20 sensors found! Check wiring and pull-up resistor.");
      // Можно остановить выполнение, если датчики не найдены
      // while(true) delay(100);
    } else {
      sensors.requestTemperatures(); // Отправляем команду на измерение
      DeviceAddress sensorAddress;
      DEBUG_PRINTLN("Sensor addresses:");
      // Выводим адрес каждого найденного устройства
      for (uint8_t i = 0; i < numberOfDevices; i++) {
        if (sensors.getAddress(sensorAddress, i)) {
          DEBUG_PRINT("  Sensor ");
          DEBUG_PRINT(i);
          DEBUG_PRINT(": ");
          printAddress(sensorAddress);
          DEBUG_PRINTLN();
        } else {
          DEBUG_PRINT("Could not get address for sensor ");
          DEBUG_PRINTLN(i);
        }
      }
    }
  #endif
  //==================================================================================
  tft.unloadFont();
  delay(3000);
  tft.fillScreen(TFT_BLACK);
  diagram(grafDispl[0], TFT_WHITE);
  diagram(grafDispl[1], TFT_WHITE);
}

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

  byte seg = 6; // Segments are 3 degrees wide = ypos segments for 360 degrees
  byte inc = 6; // Draw segments every 3 degrees, increase to 6 for segmented ring

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
  
  lightBlue = grafDispl.sp - 8;
  greenValue = grafDispl.sp - 0; 
  yellowValue = grafDispl.sp + 4; 
  redValue = grafDispl.sp + 4 + 8;
  maxtemp = redValue + 10;
  mintemp = lightBlue - 10;
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

  fillArc(grafDispl.xpos, grafDispl.ypos, 0, 40, grafDispl.radius-20, grafDispl.radius-20, seg_w, TFT_BLACK);
  tmpval0 = grafDispl.value;
  if(tmpval0 < mintemp) tmpval0 = mintemp;
  else if(tmpval0 > (maxtemp-5)) tmpval0 = (maxtemp-5);
  tmpval1 = map(tmpval0, mintemp, maxtemp, 0, 240);
  fillArc(grafDispl.xpos, grafDispl.ypos, tmpval1, 1, grafDispl.radius-10, grafDispl.radius-10, seg_w+8, TFT_WHITE);

    tft.setTextSize(1);
    tft.setTextPadding(0);

    // tft.fillRect(xpos-25, ypos-2, 50, 25, TFT_BLACK);
    tft.setTextDatum(MC_DATUM);
    //-----------------------
    if(grafDispl.value<1000) sprintf(tempStr,"%2.1fC",(float)grafDispl.value/10);
    else if(grafDispl.value<1270) sprintf(tempStr,"%5dC", grafDispl.value/10);
    else sprintf(tempStr," ---  ");
    // tft.fillRect(grafDispl.xpos-40, grafDispl.ypos-15, 80, 25, TFT_BLACK);
    // tft.setTextColor(TFT_WHITE);
    tft.setTextColor(TFT_WHITE,TFT_BLUE,true);
    tft.drawString(tempStr, grafDispl.xpos, grafDispl.ypos, 4);
    //-----------------------
    sprintf(tempStr,"%2.1fC",(float)grafDispl.sp/10);
    // tft.fillRect(grafDispl.xpos-40, grafDispl.ypos+10, 80, 25, TFT_WHITE);
    // tft.setTextColor(TFT_BLACK);
    tft.setTextColor(TFT_BLACK,TFT_WHITE,true);
    tft.drawString(tempStr, grafDispl.xpos, grafDispl.ypos+25, 4);
}
#ifdef DEBUG
// Вспомогательная функция для вывода адреса датчика
void printAddress(DeviceAddress deviceAddress) {
  for (uint8_t i = 0; i < 8; i++) {
    if (deviceAddress[i] < 16) DEBUG_PRINT("0");
    DEBUG_PRINT(deviceAddress[i], HEX);
    if (i < 7) DEBUG_PRINT(":");
  }
}
#endif
