#include "main.h"
#include "my_settings.h"
#include "display.h"

SystemState sysState;

ESP8266WebServer server(80);

RTC_DS3231 rtc;                             // Create RTC object for DS3231

DHT dht(ONE_WIRE_BUS_PIN, DHT22);
OneWire oneWire(ONE_WIRE_BUS_PIN);          // Create OneWire instance for 1-Wire bus interaction
DallasTemperature sensors(&oneWire);        // Pass oneWire reference to DallasTemperature constructor
DeviceAddress sensorAddresses[MAX_DEVICE];  // Array for unique sensor addresses

TFT_eSPI tft = TFT_eSPI();
byte writePCF8574(byte data);
bool recoverI2C();

InvertedServo incubatorServo;               // Create incubatorServo object for flap control

void setup(){
  #ifdef DEBUG
    Serial.begin(115200, SERIAL_8N1, SERIAL_TX_ONLY);                   // Initialize serial for debugging (free RXD/GPIO3 for servo)
  #endif

  //----------------------------------- MOUNTING FS ----------------------------------------
  MYDEBUG_PRINTLN("mounting FS...");
  bool lFS = LittleFS.begin();
  if(!lFS) {
    MYDEBUG_PRINTLN("failed to mount FS, formatting...");
    LittleFS.format();
    lFS = LittleFS.begin();
  }
  else {
    MYDEBUG_PRINTLN("mounted file system");
    #ifdef DEBUG
      listFilesAndSizes();
    #endif
  }

  // --- Initialize Timezone and Sync System Time from RTC immediately ---
  setenv("TZ", tzInfo, 1);
  tzset();

  time_t init_time = time(nullptr);
  timeinfo = localtime(&init_time);

  if (rtc.begin()) {
    RTCENABLE = true;
    time_t utc_time = rtc.now().unixtime();
    timeinfo = localtime(&utc_time);
    rtcTimeSet = true;
    
    // Set system time from RTC for core time functions
    struct timeval tv = { .tv_sec = utc_time };
    settimeofday(&tv, nullptr);
  } else {
    RTCENABLE = false;
  }

  //--------------------------------- initialize I2C & PCF8574 -----------------------------------
  Wire.begin(); // Initialize I2C (SDA, SCL default for ESP8266 - GPIO4, GPIO5)
  Wire.setClock(100000);                // Снижаем скорость до 100кГц для стабильности
  Wire.setClockStretchLimit(150000);    // 150мс лимит clock stretch (защита от зависания)
  uint8_t pcf = writePCF8574(0xFF);    // Set all pins LOW (if used as outputs)
  //--------------------------------- initialize TFT -----------------------------------
  tft.begin();
  tft.setRotation(3);
  tft.fillScreen(TFT_BLACK);
  touch_calibrate();

  sysLogger.log(String(getMsg(MSG_STARTUP)));
  //------------ Начальный экран --------------------
  xpos = tft.width()/2; ypos = 0;// tft.height()/2-80;
  tft.setTextDatum(TC_DATUM);
  tft.setTextColor(TFT_ORANGE, TFT_BLACK);
  tft.loadFont(FONT_LARGE, LittleFS); // загрузка в память шрифта
  tft.drawString(getMsg(MSG_STARTUP), xpos, ypos);
  tft.loadFont(FONT_MINI, LittleFS); // загрузка в память шрифта
  tft.setTextDatum(TL_DATUM);
  xpos = 0; ypos += 30;
  tft.setTextColor(TFT_RED, TFT_BLACK);
  if (!lFS){
    tft.drawString(getMsg(MSG_FS_OPEN_ERR), xpos, ypos);
    ypos += 20;
  }
  if (!RTCENABLE){
    tft.drawString("Годинника реального часу не знайдено!", xpos, ypos);
    ypos += 20;
  }
  if (pcf){
    tft.drawString("Фатальна помилка мікросхеми керування!", xpos, ypos);
    ypos += 20;
  }

  checkSetpoint();
  
  // sensorType();
  
  if (hasDHT22) {
      sysLogger.log(getMsg(MSG_DHT22_FOUND));
      tft.setTextColor(TFT_GREEN, TFT_BLACK);
      tft.drawString("Датчики температури та вологості знайдені.", xpos, ypos);
      ypos += 20;
  }
  if (numberOfDS18 > 0) {
      sysLogger.log(String(getMsg(MSG_DS18B20_FOUND)) + ": " + String(numberOfDS18));
      sensors.requestTemperatures();
      tft.setTextColor(TFT_GREEN, TFT_BLACK);
      char txt[40];
      snprintf_P(txt, sizeof(txt), PSTR("Датчики температури знайдені: %d шт."), numberOfDS18);
      tft.drawString(txt, xpos, ypos);
      ypos += 20;
  }
  if (!hasDHT22 && numberOfDS18 == 0) {
      sysLogger.log(getMsg(MSG_SENSORS_NONE));
      tft.drawString("Датчики температури та вологості НЕ знайдені!", xpos, ypos);
      ypos += 20;
  }

  //---------------------------- WiFiManager initialization -----------------------------------
  tft.setTextColor(TFT_GREEN, TFT_BLACK);
  if(settings.special & 0x03) initWiFiManag();
  else MYDEBUG_PRINTLN("WiFi connection disabled! Continuing in offline mode.");
  
  if (RTCENABLE) {
    bool hasWifi = (WIFIENABLE && WiFi.status() == WL_CONNECTED);
    if (rtc.lostPower()) {
        MYDEBUG_PRINTLN("RTC lost power, forcing initial time sync.");
        if (hasWifi) syncTime();
        else MYDEBUG_PRINTLN("WiFi not connected. Skipping NTP sync.");
    } else if(settings.special & 0x04){
        settings.special &= 0xFB;
        saveSetPoint();
        if (hasWifi) syncTime();
        else MYDEBUG_PRINTLN("WiFi not connected. Skipping NTP sync.");
    } else {
        MYDEBUG_PRINTLN("RTC has power, time should be valid.");
    }
    testProgs();
  } else {
    MYDEBUG_PRINTLN("Couldn't find RTC!");
  }
  // Initialize servo motor on GPIO3 (PWMOUT_PIN)
  incubatorServo.attach(PWMOUT_PIN);
  pvFlap = settings.curFlap;
  incubatorServo.write(pvFlap);

  sensorCheck();
  displNum = 0;  
  newDispl = true;
  portOut.value = 0xFF;

  if(RTCENABLE){
    logicManager.processIrrigation();
    logicManager.processLighting();
  }

  #ifdef SIMULATION
    ds[0].pvT = 200;
    ds[0].pvErr = 0;
    ds[1].pvT = 160;
    ds[1].pvErr = 0;
  #endif
  grafDispl[0].value = ds[0].pvT;
  grafDispl[0].spOn = settings.spT0on;
  grafDispl[0].spOff = settings.spT0off;
  grafDispl[1].value = ds[1].pvT;
  grafDispl[1].spOn = settings.spT1on;
  grafDispl[1].spOff = settings.spT1off;

  tft.unloadFont();
  delay(3000);
  tft.fillScreen(TFT_BLACK);
  // diagram(grafDispl[0], TFT_WHITE);
  // diagram(grafDispl[1], TFT_WHITE);

  ESP.wdtEnable(5000); // Enable hardware watchdog with 5-second timeout
}

void loop(){
  ESP.wdtFeed();                // Feed the hardware watchdog
  unsigned long now = millis();
  
  if (beepOn && now >= beepOffTime) {
    beepOn = 0;
    BEEP = PCF_OFF;
    writePCF8574(portOut.value);
  }
  server.handleClient();        // Handle incoming requests
  static uint32_t lastTouchTime = 0;
  static uint16_t lastX = 0, lastY = 0;
  static uint8_t touchCount = 0;
  bool pressed = false;

  if (now - lastTouchTime > 30) {
    lastTouchTime = now;
    uint16_t raw_x = 0, raw_y = 0;
    if (tft.getTouch(&raw_x, &raw_y)) {
      // Проверяем стабильность координат
      if (touchCount > 0 && abs((int)raw_x - (int)lastX) < 30 && abs((int)raw_y - (int)lastY) < 30) {
        touchCount++;
      } else {
        touchCount = 1;
      }
      lastX = raw_x;
      lastY = raw_y;
      
      // Требуем стабильного удержания касания в течение 3 циклов (~90 мс)
      if (touchCount >= 3) {
        t_x = raw_x;
        t_y = raw_y;
        pressed = true;
      }
    } else {
      touchCount = 0;
    }
  }

  if(pressed && !newDispl){
    switch (displNum){
        case 0: 
          displNum = 1; newDispl = true;
          menu_1();
          waitForTouchRelease();
          break;
        case 1: checkKeypad(POINTS_1); break;
        case 2: checkKeypad(POINTS_2); break;
        case 3: checkKeypad(POINTS_3); break;
        case 4: checkKeypad(POINTS_4); break;

        case 10: checkKeypad(15); break;
    }
  }
  //============================= NEW HALF-SECOND =================================
  if(now - counter1s > 500){
    counter1s = now;
    halfSecond++; 
    if(resetDisplay){
      if(--resetDisplay == 0) {
        saveSetPoint();
        displNum = 0; newDispl = true;
      }
    }
    if(halfSecond % 2 == 0){//-------- NEW SECOND -----------------------
      countSeconds++; 
      handleWiFi();                 // Handle Wi-Fi connection and services status
      if (tmrTelegramOff > 0) {
        tmrTelegramOff--;
      }
      time_t utc_time = time(nullptr);
      timeinfo = localtime(&utc_time);
      checkAndApplyHourlyProgram();
      #ifndef SIMULATION  
        sensorCheck();                                                  // Опрос датчиков должен быть всегда
      #else
        #define MAXPOINT 5
        // В режиме отладки можно оставить симуляцию, если датчики не подключены
        if(HEATER == PCF_ON){
          if(++ds[0].pvErr > MAXPOINT) ds[0].pvErr = MAXPOINT;
        }  else {
          if(--ds[0].pvErr < -MAXPOINT) ds[0].pvErr = -MAXPOINT;
        }
        if(ds[0].pvErr > 0) ds[0].pvT++; else ds[0].pvT--;
        if(HUMIDI == PCF_ON) {
          if(++ds[1].pvErr > MAXPOINT) ds[1].pvErr = MAXPOINT;
        }  else {
          if(--ds[1].pvErr < -MAXPOINT) ds[1].pvErr = -MAXPOINT;
        }
        if(ds[1].pvErr > 0) ds[1].pvT++; else ds[1].pvT--;
      #endif
      logicManager.processClimate();
      
      // Fast response for auxiliary modes (thermostat/hygrostat)
      if ((settings.modeRelay1 & 0x03) == 0) logicManager.relaySwitch(1);
      if ((settings.modeRelay2 & 0x03) == 0) logicManager.relaySwitch(2);

      writePCF8574(portOut.value);

      logicManager.processAlarms();
      logicManager.updateStatusLeds();

      // Update servo motor position (flap)
      incubatorServo.write(pvFlap);

      if(displNum == 0) mainDispl();
    } //---------------------------------------------------------------
    if(halfSecond > 119){//------ NEW MINUTE ------------------------
      halfSecond = 0; countSeconds = 0; minutes++;
      if(RTCENABLE){
        logicManager.processLighting();
        logicManager.processIrrigation();

        // Запись точек графиков каждые 5 минут
        if (timeinfo && timeinfo->tm_min % 5 == 0) {
            static int lastLoggedMinute = -1;
            if (timeinfo->tm_min != lastLoggedMinute) {
                lastLoggedMinute = timeinfo->tm_min;
                int period_of_day = (timeinfo->tm_hour * 60 + timeinfo->tm_min) / 5;
                int address = DAILY_DATA_START + period_of_day * DAILY_DATA_REC_SIZE;
                eepromWriteInt16(address, ds[0].pvT);
                eepromWriteInt16(address + 2, ds[1].pvT);
                eepromWriteInt16(address + 4, (int16_t)pvRH);
            }
        }

        // Проверка смены суток для сохранения логов вчерашнего дня
        static int lastSavedDay = -1;
        static int lastSavedMonth = -1;
        if (timeinfo) {
            if (lastSavedDay == -1) {
                lastSavedDay = timeinfo->tm_mday;
                lastSavedMonth = timeinfo->tm_mon + 1;
            }
            if (timeinfo->tm_mday != lastSavedDay) {
                saveDailyDataToFile(lastSavedDay, lastSavedMonth);
                clearEEPROM();
                lastSavedDay = timeinfo->tm_mday;
                lastSavedMonth = timeinfo->tm_mon + 1;
            }
        }
      }
      //---------------------------- NEW HOUR ----------------------------------
      if(minutes > 59){
        minutes = 0;
        if(RTCENABLE){
          if(WIFIENABLE && WiFi.status() == WL_CONNECTED){
            // ------------- Daily synchronization logic --------------
            // Sync with RTC at midnight (00:00)
            if (timeinfo->tm_mday != lastSyncDay && timeinfo->tm_hour == 0) { 
              MYDEBUG_PRINTLN("\nMidnight sync: Updating RTC from NTP...");
              configTzTime(tzInfo, ntpServer); // Ensure background sync is active
              
              // Wait a bit for NTP to update system time if needed, 
              // but don't block heavily as it's a background process in ESP8266 core
              time_t now_t = time(nullptr);
              if (now_t > 1000000000) { // If system time is valid
                rtc.adjust(DateTime(now_t));
                lastSyncDay = timeinfo->tm_mday;
                sysLogger.log(getMsg(MSG_RTC_SYNC));
                MYDEBUG_PRINTLN("RTC updated successfully.");
              }
            }
          } else {
            // Offline mode or no WiFi connection: sync system time from RTC to prevent drift
            time_t utc_time = rtc.now().unixtime();
            struct timeval tv = { .tv_sec = utc_time };
            settimeofday(&tv, nullptr);
          }
        }
      } // ------------------------- hour ----------------------------
    } //--------------------------- minute --------------------------
  } //-------------------------- half-second ------------------------
}//============================================== END LOOP =============================================

// Function to recover the I2C bus if SDA or SCL hang
// Returns true if PCF8574 responds on the bus after recovery
bool recoverI2C() {
  static unsigned long lastRecoveryTime = 0;
  if (millis() - lastRecoveryTime < 500) return false; // Limit retry frequency
  lastRecoveryTime = millis();

  MYDEBUG_PRINTLN("Attempting I2C bus recovery...");

  const uint8_t sda = 4; 
  const uint8_t scl = 5;

  // 1. Configure pins to INPUT_PULLUP
  pinMode(sda, INPUT_PULLUP);
  pinMode(scl, INPUT_PULLUP);
  delay(5);

  MYDEBUG_PRINT("Bus state before pulses: SDA=");
  MYDEBUG_PRINT(digitalRead(sda));
  MYDEBUG_PRINT(", SCL=");
  MYDEBUG_PRINTLN(digitalRead(scl));

  // 2. Generate up to 20 SCL clock pulses to clock out stuck slaves
  //    Use a slower speed (~5kHz) for reliability
  pinMode(scl, OUTPUT);
  for (int i = 0; i < 20; i++) {
    digitalWrite(scl, LOW);
    delayMicroseconds(100);
    digitalWrite(scl, HIGH);
    delayMicroseconds(100);
    if (digitalRead(sda) == HIGH && i >= 8) {
       MYDEBUG_PRINT("SDA released after ");
       MYDEBUG_PRINT(i + 1);
       MYDEBUG_PRINTLN(" pulses.");
       break;
    }
  }

  // 3. Generate correct STOP condition
  pinMode(sda, OUTPUT);
  digitalWrite(sda, LOW);
  delayMicroseconds(100);
  digitalWrite(scl, HIGH);
  delayMicroseconds(100);
  digitalWrite(sda, HIGH);   // STOP: SDA LOW to HIGH while SCL is HIGH
  delayMicroseconds(100);

  // 4. Return pins to Wire control
  pinMode(sda, INPUT_PULLUP);
  pinMode(scl, INPUT_PULLUP);
  delay(20); // wait for line stabilization

  Wire.begin(sda, scl);
  Wire.setClock(100000);
  Wire.setClockStretchLimit(150000); // 150ms to prevent lockups during clock stretch
  
  delay(50); // Increased delay to give Wire and devices time to stabilize

  if (digitalRead(sda) == LOW || digitalRead(scl) == LOW) {
    MYDEBUG_PRINT("I2C recovery: FAILED (bus stuck). SDA=");
    MYDEBUG_PRINT(digitalRead(sda));
    MYDEBUG_PRINT(", SCL=");
    MYDEBUG_PRINTLN(digitalRead(scl));
    return false;
  }

  // 5. Verify PCF8574 presence on the bus
  Wire.beginTransmission(PCF8574_ADDRESS);
  uint8_t err = Wire.endTransmission();
  if (err == 0) {
    MYDEBUG_PRINTLN("I2C recovery: SUCCESS (PCF8574 responds)");
    return true;
  } else {
    MYDEBUG_PRINT("I2C recovery: bus OK, but PCF8574 not found. err=");
    MYDEBUG_PRINTLN(err);
    return false;
  }
}

void enterI2cCriticalError() {
  #if defined(LANG_RU)
    sysLogger.log("КРИТИЧЕСКАЯ ОШИБКА I2C: Перезапуск!");
  #elif defined(LANG_UA)
    sysLogger.log("КРИТИЧНА ПОМИЛКА I2C: Перезапуск!");
  #else
    sysLogger.log("CRITICAL I2C ERROR: Restarting!");
  #endif
  MYDEBUG_PRINTLN("\n!!! CRITICAL I2C ERROR: Hardware WDT reset now...");

  // Release the I2C bus via STOP condition before reset
  const uint8_t sda = 4;
  const uint8_t scl = 5;
  pinMode(scl, OUTPUT); digitalWrite(scl, HIGH);
  pinMode(sda, OUTPUT); digitalWrite(sda, LOW);
  delayMicroseconds(100);
  digitalWrite(scl, HIGH);
  delayMicroseconds(100);
  digitalWrite(sda, HIGH); // SDA LOW->HIGH while SCL is HIGH = STOP condition
  delayMicroseconds(100);
  pinMode(sda, INPUT_PULLUP);
  pinMode(scl, INPUT_PULLUP);

  // Hardware reset via Hardware WDT (rst cause:4) - full chip reset.
  // Without yield/delay, WDT triggers in ~8 seconds.
  MYDEBUG_PRINTLN("Waiting for Hardware WDT...");
  ESP.wdtDisable();
  while (true) { }
}

static uint8_t i2c_error_count = 0;        // Counter of consecutive I2C errors
static uint8_t recovery_fail_count = 0;    // Counter of consecutive recoverI2C() failures

static void waitForPCF8574OrReboot(uint8_t maxRetries = 2,
                                   unsigned long pauseMs = 500UL) {
  #if defined(LANG_RU)
    sysLogger.log("ВНИМАНИЕ: PCF8574 недоступен! Попытка восстановления...");
  #elif defined(LANG_UA)
    sysLogger.log("УВАГА: PCF8574 недоступний! Спроба відновлення...");
  #else
    sysLogger.log("WARNING: PCF8574 not responding! Attempting recovery...");
  #endif
  MYDEBUG_PRINTLN("PCF8574: starting recovery (2 attempts, 500ms pause)...");

  for (uint8_t attempt = 1; attempt <= maxRetries; attempt++) {
    ESP.wdtFeed();
    // Pause before attempt - give the bus time to stabilize
    unsigned long waitStart = millis();
    while (millis() - waitStart < pauseMs) {
      ESP.wdtFeed();
      delay(50);
    }
    DEBUG_PRINTF("PCF8574: recovery attempt %d / %d\n", attempt, maxRetries);
    if (recoverI2C()) {
      MYDEBUG_PRINTLN("PCF8574: recovered successfully!");
      #if defined(LANG_RU)
        sysLogger.log("PCF8574 восстановлен. Продолжаем работу.");
      #elif defined(LANG_UA)
        sysLogger.log("PCF8574 відновлено. Продовжуємо роботу.");
      #else
        sysLogger.log("PCF8574 recovered. Continuing work.");
      #endif
      i2c_error_count = 0;
      recovery_fail_count = 0;
      return; // recovered - exit
    }
  }

  // All attempts failed - rebooting
  #if defined(LANG_RU)
    sysLogger.log("PCF8574 не восстановлен. Перезапуск!");
  #elif defined(LANG_UA)
    sysLogger.log("PCF8574 не відновлено. Перезапуск!");
  #else
    sysLogger.log("PCF8574 recovery failed. Restarting!");
  #endif
  enterI2cCriticalError();
}

// Function to write byte to PCF8574 with auto-recovery
byte writePCF8574(byte data) {
  // Wire.beginTransmission(PCF8574_ADDRESS);
  // Wire.write(data);
  // byte error = Wire.endTransmission();
  
  // if (error) {
  //   i2c_error_count++;
  //   MYDEBUG_PRINT("\nError writing to PCF8574. Code: ");
  //   MYDEBUG_PRINT(error);
  //   MYDEBUG_PRINT(" | Fail count: ");
  //   MYDEBUG_PRINTLN(i2c_error_count);

  //   // First attempt of fast recovery
  //   bool recovered = recoverI2C();
  //   if (recovered) {
  //     recovery_fail_count = 0;
  //     delay(50);
  //     Wire.beginTransmission(PCF8574_ADDRESS);
  //     Wire.write(data);
  //     error = Wire.endTransmission();
  //     if (error == 0) {
  //       i2c_error_count = 0;
  //       MYDEBUG_PRINTLN("writePCF8574: retry after recovery OK");
  //     }
  //   }

  //   if (error) {
  //     // Fast recovery failed - launch wait loop with timeout
  //     recovery_fail_count++;
  //     MYDEBUG_PRINT("PCF8574: recovery failed. Consecutive failures: ");
  //     MYDEBUG_PRINTLN(recovery_fail_count);
  //     waitForPCF8574OrReboot(); // 2 attempts -> reboot if unsuccessful

  //     // If we got here - recoverI2C() returned true inside waitForPCF8574OrReboot, repeat write
  //     delay(50);
  //     Wire.beginTransmission(PCF8574_ADDRESS);
  //     Wire.write(data);
  //     error = Wire.endTransmission();
  //   }
  // } else {
  //   i2c_error_count = 0;
  //   recovery_fail_count = 0;
  // }

  // if (error == 0) {
  //   static byte last_relays_state = 0xFF;
  //   byte changed = (last_relays_state ^ data) & 0x3F;
  //   if (changed) {
  //     #if defined(LANG_RU)
  //     const char* const relayNames[] = {
  //         "Освещение",
  //         "Обогреватель",
  //         "Увлажнитель",
  //         "Реле 1",
  //         "Реле 2",
  //         "Реле 3"
  //     };
  //     const char* const relayStates[] = {
  //         "ВЫКЛ",
  //         "ВКЛ"
  //     };
  //     #elif defined(LANG_UA)
  //     const char* const relayNames[] = {
  //         "Освітлення",
  //         "Обігрівач",
  //         "Зволожувач",
  //         "Реле 1",
  //         "Реле 2",
  //         "Реле 3"
  //     };
  //     const char* const relayStates[] = {
  //         "ВИКЛ",
  //         "ВКЛ"
  //     };
  //     #else // LANG_EN
  //     const char* const relayNames[] = {
  //         "Light",
  //         "Heater",
  //         "Humidifier",
  //         "Relay 1",
  //         "Relay 2",
  //         "Relay 3"
  //     };
  //     const char* const relayStates[] = {
  //         "OFF",
  //         "ON"
  //     };
  //     #endif

  //     for (int i = 0; i < 6; i++) {
  //       if (changed & (1 << i)) {
  //         bool shouldLog = false;
  //         if (i == 0) {         // Освещение
  //           shouldLog = true;
  //         } else if (i == 3) {  // Реле 1
  //           shouldLog = (settings.modeRelay1 > 0);
  //         } else if (i == 4) {  // Реле 2
  //           shouldLog = (settings.modeRelay2 > 0);
  //         } else if (i == 5) {  // Реле 3
  //           shouldLog = true;
  //         }

  //         if (shouldLog) {
  //           bool isOn = !(data & (1 << i)); // active low: 0 = ON, 1 = OFF
  //           String msg = String(relayNames[i]) + ": " + (isOn ? relayStates[1] : relayStates[0]);
  //           sysLogger.log(msg);
  //         }
  //       }
  //     }
  //     last_relays_state = (last_relays_state & 0xC0) | (data & 0x3F);
  //   }
  // }

  // return error;
  return 0;
}

// Function to read byte from PCF8574.
// Attempts to recover the bus on error. Returns cached value if unsuccessful.
// Reading does not trigger reboot (writePCF8574 is responsible for that).
byte readPCF8574() {
  // static byte lastKnownValue = 0xFF; // Cache of the last successful read

  // uint8_t count = Wire.requestFrom((uint8_t)PCF8574_ADDRESS, (uint8_t)1);
  // if (count > 0) {
  //   i2c_error_count = 0;
  //   lastKnownValue = Wire.read();
  //   return lastKnownValue;
  // }

  // // --- Read Error ---
  // i2c_error_count++;
  // MYDEBUG_PRINT("\nError reading from PCF8574. Fail count: ");
  // MYDEBUG_PRINTLN(i2c_error_count);

  // recoverI2C();

  // // Retry after recovery
  // delay(50);
  // count = Wire.requestFrom((uint8_t)PCF8574_ADDRESS, (uint8_t)1);
  // if (count > 0) {
  //   i2c_error_count = 0;
  //   lastKnownValue = Wire.read();
  //   MYDEBUG_PRINTLN("readPCF8574: retry after recovery OK");
  //   return lastKnownValue;
  // }

  // // Second retry
  // delay(10);
  // count = Wire.requestFrom((uint8_t)PCF8574_ADDRESS, (uint8_t)1);
  // if (count > 0) {
  //   i2c_error_count = 0;
  //   lastKnownValue = Wire.read();
  //   MYDEBUG_PRINTLN("readPCF8574: 2nd retry OK");
  //   return lastKnownValue;
  // }

  // // Failed to read - return cached value.
  // // Reboot will be initiated via writePCF8574 in the next cycle.
  // MYDEBUG_PRINTLN("readPCF8574: using cached value");
  // return lastKnownValue;
  return 0;
}
