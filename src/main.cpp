/*
RAM:   [====      ]  43.4% (used 35520 bytes from 81920 bytes)
Flash: [===       ]  34.9% (used 364343 bytes from 1044464 bytes)
*/

#include "main.h"
#include "tftArcFill.h"
#include "display.h"
#include "touchKeypad.h"
#include "procedure.h"
#include "sensors.h"
#include "AT24C32.h"
#include "my_settings.h"

PIDController pid[2];
SoftwarePWMBit heaterPwm(&portOut.value, 0); 
SoftwarePWMBit humidiPwm(&portOut.value, 1);

RTC_DS3231 rtc;                     // Создаем объект RTC для DS3231

OneWire oneWire(ONE_WIRE_BUS_PIN);  // Создаем экземпляр объекта OneWire для взаимодействия с шиной 1-Wire
DallasTemperature sensors(&oneWire);// Передаем ссылку на объект oneWire в конструктор DallasTemperature


// Создаем объекты TFT
TFT_eSPI tft = TFT_eSPI();    // Создаем экземпляр библиотеки

void setup() {
  #ifdef DEBUG
    Serial.begin(115200);       // Инициализация последовательного порта для отладки
  #endif
  tft.begin();
  tft.setRotation(3);
  tft.fillScreen(TFT_BLACK);
  // Calibrate the touch screen and retrieve the scaling factors
  touch_calibrate();
  //--------- инициализация FS -----------------------------------------
  if (!LittleFS.begin()) {
    DEBUG_PRINTLN("Flash FS initialisation failed!");
    tft.setTextDatum(TC_DATUM);
    tft.setTextColor(TFT_RED, TFT_YELLOW);
    tft.drawString("ERROR file system!", tft.width()/2, tft.height()/2-20, 4);
    delay(10000);
  }
  Serial.println("\nFlash FS available!");
  bool font_missing = false;
  if (LittleFS.exists("/Arial20.vlw") == false) font_missing = true;
  if (LittleFS.exists("/Arial28.vlw") == false) font_missing = true;
  if (font_missing){
    DEBUG_PRINTLN("\nFont missing in Flash FS, did you upload it?");
  } else DEBUG_PRINTLN("\nFonts found OK.");
  //--------- инициализация Конфигурации --------------------------------------------
  initMyConfig();

  pvTimer = settings.sp_structs[0].timer;                  // инициализация времени выключенного состояния таймера
  pvWait = settings.sp_structs[0].aeration;                // инициализация ПАУЗы ПРОВЕТРИВАНИЯ (минут)
  portOut.value = 0;
  heaterPwm.write(heaterValue);
  humidiPwm.write(humidiValue);
}

void loop() {

  //--------------------------- УПРАВЛЕНИЕ СИМИСТОРОМ ---------------------------------
  bool hasChanged = false;
  hasChanged |= heaterPwm.update();
  hasChanged |= humidiPwm.update();
  if (hasChanged)  writePCF8574(portOut.value);

  // Pressed will be set true is there is a valid touch on the screen
  bool pressed = tft.getTouch(&t_x, &t_y);
  if(pressed && !newDispl){
    switch (displNum){
        case 0: 
          displNum = 1; newDispl = true;
          menu_1();
          break;
        case 1: checkKeypad(MENU_1); break;
        case 2: checkKeypad(MENU_1); break;
        case 3: checkKeypad(MENU_2); break;
        case 4: checkKeypad(MENU_3); break;

        case 10: checkKeypad(15); break;
    }
  } 
  //=================== НОВАЯ СЕКУНДА =================================
  long now = millis();
  if (now - lastMsg > 1000){
    seconds++; lastMsg = now;
    errors.value = 0;

    if(resetDisplay) --resetDisplay; 
    else if(displNum){displNum = 0; newDispl = true; displOff=DISPLAYOFF;}  // возврат к главному дисплею
    else if(displOff) --displOff;
    // else HAL_GPIO_WritePin(LED_GPIO_Port, LED_Pin, GPIO_PIN_RESET);       // отключение дисплея через 5 минут
  //------------------------ ЗНАЧЕНИЯ ТЕМПЕРАТУРЫ --------------------------
  #ifndef DEBUG  
    temperature_check();
  
    if (HIH5030){
      uint16_t adc=1024;
      pvVadcRH = adc;//lowPassF2(adc);           // относительная влажность в Vadc ??????????????????????????????????????????
      if (pvVadcRH>80) pvRH = valDcToRH(pvVadcRH); // относительная влажность в %
      else pvRH = 1990;
    } else {
      uint8_t valTable = tableRH(ds[0].pvT, ds[1].pvT);               // если отсутствует HIH4000 то ...
      if(valTable>100) pvRH = 999; else pvRH = valTable;
    }
  #else
    //-----температура воздуха------
    dpv0 = pid[0].pPart/500 + pid[0].iPart/10;
    ds[0].pvT += dpv0;
    dpv1 = pid[1].pPart/500 + pid[1].iPart/10;
    ds[1].pvT += dpv1;
    //------
  #endif
    if(!COOLING){  //-------------- нормальная работа -------------------------
      switch (settings.sp_structs[0].mode) {
          uint8_t val;
          case 0:
            heaterValue = UpdatePID(0);            // ПИД нагреватель
            humidiValue = UpdatePID(1);            // ПИД увлажнитель
            break;
          case 1:
            val = RelayPos(0,2);
            switch (val){
                case ON: heaterValue = TRIACON; break;
                case OFF: heaterValue = OFF;    break;
            }
            humidiValue = UpdatePID(1);            // ПИД увлажнитель
            break;
          case 2:
            heaterValue = UpdatePID(0);            // ПИД нагреватель
            val = RelayPos(1,3);
            switch (val){
                case ON: humidiValue = TRIACON; break;
                case OFF: humidiValue = OFF;    break;
            }
            break;
          case 3:
            val = RelayPos(0,2);
            switch (val){
                case ON: heaterValue = TRIACON; break;
                case OFF: heaterValue = OFF;    break;
            }
            val = RelayPos(1,3);
            switch (val){
                case ON: humidiValue = TRIACON; break;
                case OFF: humidiValue = OFF;    break;
            }
            break;
          case 4:
            heaterValue = UpdatePID(0);           // ПИД нагреватель
            OutPulse();                           // импульсное управление увлажнителем
            break;
      }
    } else {heaterValue = 0; displPower = 0;}      //-- идет ОХЛАЖДЕНИЕ!--
    if(settings.sp_structs[1].mode == 1 && !REACHED0) humidiValue=OFF; // задержка регулирования по 2 каналу до прогрева инкубатора
    heaterPwm.write(heaterValue);
    humidiPwm.write(humidiValue);

    if(!COOLING){  //-------------- нормальная работа -------------------------
      //------ КАНАЛ ВСПОМОГАТЕЛЬНОГО НАГРЕВАТЕЛЯ -------------------------------------------------
      if(ERROR1 == 0){
        if (ds[0].pvErr >= settings.sp_structs[0].auxiliary) EXTRA2 = ON;       // включить вспомогательны нагреватель
        else if (ds[0].pvErr <= settings.sp_structs[1].auxiliary) EXTRA2 = OFF; // отключить вспомогательны нагреватель
      } else EXTRA2 = OFF;                                                      // отключить вспомогательны нагреватель
      //------ ОХЛАЖДЕНИЕ  ОСУШЕНИЕ ---------------------------------------------------------------
      uint8_t val = RelayNeg(0,settings.sp_structs[0].coolOn,settings.sp_structs[0].coolOff);
      if(val == OFF && (ds[0].pvErr <= settings.sp_structs[0].alarm)) 
              val = RelayNeg(1,settings.sp_structs[1].coolOn,settings.sp_structs[1].coolOff); // если холодно то не открываем заслонку.
      if(val == ON){EXTRA1 = ON; pvFlap = 100;} else if(val == OFF){EXTRA1 = OFF; pvFlap = settings.sp_structs[0].state;}
      //------ АВАРИЙНОЕ ВЫКЛЮЧЕНИЕ ---------------------------------------------------------------
      if(settings.sp_structs[0].extendMode&1){    // [0]-0-СИРЕНА; 1-АВАРИЙНОЕ ОТКЛЮЧЕНИЕ;
        uint8_t val = RelayNeg(0,settings.sp_structs[0].alarm,settings.sp_structs[0].spT); // канал 5 АВАРИЙНОЕ ВЫКЛЮЧЕНИЕ.
        if(val == ON) EXTRA3 = ON;                // включить канал 5
        else if(val == OFF) EXTRA3 = OFF;         // отключить канал 5
      }
    //------- ПРОВЕТРИВАНИЕ ----------------------------------------------------------------------    
      if(AERATION){     // Идет ПРОВЕТРИВАНИЕ !
        EXTRA1 = ON; pvFlap = 100; beepOn = 10;
        if(--pvVenting == 0){pvWait = settings.sp_structs[0].aeration; AERATION =0;}
      }
      // if(setup==0) alarm();
    }
    // setflap();                            // задание положения заслонки 
    // if((setup+setprgday)==0) display(displmode);// вывод на дисплей

    //-------------------------
    
    // DateTime now = rtc.now();
    if(displNum == 0) mainDispl();
    //-----------------------------------------------------------------------------

    // -- Пример 1: Управление выходами PCF8574 (как светодиодами) ---
    // writePCF8574(now.second()%10);
    writePCF8574(seconds % 10);
    /* -- Пример 2: Чтение входов PCF8574 ---
          Чтобы читать пины как входы, сначала запишите в них 0xFF (все единицы),
          чтобы перевести их в режим "квази-входа" с высоким импедансом.
          Если к пину ничего не подключено или подключено к VCC, вы прочитаете '1'.
          Если пин замкнут на GND, вы прочитаете '0'. 
    writePCF8574(0x80); // Устанавливаем  пин в режим "квази-входа"
    delay(100); // Небольшая задержка для стабилизации
    byte inputData = readPCF8574();
    // Пример проверки состояния конкретного пина (например, P8)
    if (!(inputData & 0x80)) { // Если P8 равен 0
      DEBUG_PRINTLN("Pin P8 is LOW");
    } else {
      DEBUG_PRINTLN("Pin P8 is HIGH");
    }
    */
    //================================= НОВАЯ МИНУТА ==============================
    if(seconds > 59){
        seconds = 0;
      //---------------------------- ПОВОРОТ ЛОТКОВ ----------------------------
        if(settings.sp_structs[0].timer) rotate_trays();
      //---------------------------- ПРОВЕТРИВАНИЕ !! --------------------------
        if(!AERATION && !COOLING && settings.sp_structs[1].aeration){
          if(--pvWait == 0){
            pvVenting = settings.sp_structs[1].aeration; AERATION = 1; EXTRA1 = ON;
          //  if((relayMode & 4) && checkDry==0) {pwTriac1=maxRun; CN2 = CN2ON;}// принудительный впрыск воды!!!
          }
        } else if(COOLING){
          EXTRA1 = ON; pvFlap = 100; beepOn = 50;
          if(--pvVenting == 0){pvWait = settings.sp_structs[0].aeration; COOLING = 0;}
          // if(extendMode&1) BREAK=ON; 
        }
    }
  //==================================================================================
  }
    //-----------------------------------------------------------------------------
}

// Функция для записи байта на PCF8574
byte writePCF8574(byte data) {
  Wire.beginTransmission(PCF8574_ADDRESS);
  Wire.write(data);
  byte error = Wire.endTransmission();
  if (error == 0) {
    //DEBUG_PRINT("Data written: 0b");
    //printBinary(data);
    //DEBUG_PRINTLN();
  } else {
    DEBUG_PRINT("Error writing to PCF8574. Error code: ");
    DEBUG_PRINTLN(error);
  }
  return error;
}

// Функция для чтения байта с PCF8574
byte readPCF8574() {
  Wire.requestFrom(PCF8574_ADDRESS, 1); // Запросить 1 байт данных
  if (Wire.available()) {
    return Wire.read();
  } else {
    DEBUG_PRINTLN("Error reading from PCF8574: No data available.");
    return 0xFF; // Возвращаем 0xFF в случае ошибки (можно выбрать другое значение)
  }
}

// Вспомогательная функция для печати байта в двоичном формате
void printBinary(byte inByte) {
  for (int b = 7; b >= 0; b--) {
    DEBUG_PRINT(bitRead(inByte, b));
  }
}

