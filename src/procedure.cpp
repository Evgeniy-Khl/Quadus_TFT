#include "main.h"

void beeperOn(uint8_t val){
  beepOn = 1;
  beepOffTime = millis() + val;
  BEEP = PCF_ON;
  writePCF8574(portOut.value);
}

/**
 * @brief Load setpoints from LittleFS or set defaults.
 */
void checkSetpoint(void){
  //--------- Load configuration --------------------------------------------
  if(LittleFS.exists("/setpoint.json")){
      if(!loadSetPoint()){
        MYDEBUG_PRINTLN("Configuration not loaded!");
        tft.drawString("Конфігурації приладу не знайдено!", xpos, ypos);
        ypos += 20;
        saveSetPoint();  // use defaults
      }
  } else {
      saveSetPoint();  // use defaults
      MYDEBUG_PRINTLN("Default configuration applied!");
      tft.drawString("Застосовано конфігурацію за замовчуванням!", xpos, ypos);
      ypos += 20;
  }
  
  sources = settings.modeRelay1 >> 4;         // mask 0x0F - relay 3 permissions; mask 0xF0 - source
  MYDEBUG_PRINT("Source relay 1:"); MYDEBUG_PRINT(sources);
  sources |= settings.modeRelay2 & 0xF0;      // mask 0x0F - relay 4 permissions; mask 0xF0 - source
  MYDEBUG_PRINT("; Source relay 2:"); MYDEBUG_PRINTLN(sources >> 4);
  MYDEBUG_PRINTLN("\n>> Values after FS load:");
  #ifdef DEBUG
    printSetPoint();
  #endif
}

#ifdef DEBUG
/**
 * @brief Print current settings to Serial.
 */
void printSetPoint() {
    MYDEBUG_PRINTLN("--------------------");
    DEBUG_PRINTF("  spT0on: %d\n", settings.spT0on);
    DEBUG_PRINTF("  spT0off: %d\n", settings.spT0off);
    DEBUG_PRINTF("  spT1on: %d\n", settings.spT1on);
    DEBUG_PRINTF("  spT1off: %d\n", settings.spT1off);
    DEBUG_PRINTF("  water0on: %d\n", settings.water0on);
    DEBUG_PRINTF("  water0off: %d\n", settings.water0off);
    DEBUG_PRINTF("  water1on: %d\n", settings.water1on);
    DEBUG_PRINTF("  water1off: %d\n", settings.water1off);
    DEBUG_PRINTF("  water2on: %d\n", settings.water2on);
    DEBUG_PRINTF("  water2off: %d\n", settings.water2off);
    DEBUG_PRINTF("  curFlap: %d\n", settings.curFlap);
    DEBUG_PRINTF("  minFlap: %d\n", settings.minFlap);
    DEBUG_PRINTF("  maxFlap: %d\n", settings.maxFlap);
    DEBUG_PRINTF("  timerOn: %d\n", settings.timerOn);
    DEBUG_PRINTF("  timerOff: %d\n", settings.timerOff);
    DEBUG_PRINTF("  alarm0: %d\n", settings.alarm0);
    DEBUG_PRINTF("  alarm1: %d\n", settings.alarm1);
    DEBUG_PRINTF("  hyst0: %d\n", settings.hysteresis0);
    DEBUG_PRINTF("  hyst1: %d\n", settings.hysteresis1);
    DEBUG_PRINTF("  special: %d\n", settings.special);
    DEBUG_PRINTF("  deviceNum: %d\n", settings.deviceNum);
    DEBUG_PRINTF("  program: %d\n", settings.program);
    DEBUG_PRINTF("  modeLight: %d\n", settings.modeLight);
    DEBUG_PRINTF("  modeHeater: %d\n", settings.modeHeater);
    DEBUG_PRINTF("  modeHumidi: %d\n", settings.modeHumidi);
    DEBUG_PRINTF("  modeRelay1: %d\n", settings.modeRelay1);
    DEBUG_PRINTF("  modeRelay2: %d\n", settings.modeRelay2);
    DEBUG_PRINTF("  modeRelay3: %d\n", settings.modeRelay3);
    MYDEBUG_PRINTLN("--------------------");
}
#endif

/**
 * @brief Save current settings to LittleFS as JSON.
 */
void saveSetPoint() {
    MYDEBUG_PRINTLN("Saving configuration...");
    waitCheckKeyPad = WAITCHECKKEYPAD * 5;  // 5 sec. lockout
    JsonDocument doc;
    JsonObject obj = doc.to<JsonObject>();

    obj["spT0on"] = settings.spT0on;
    obj["spT0off"] = settings.spT0off;
    obj["spT1on"] = settings.spT1on;
    obj["spT1off"] = settings.spT1off;
    obj["water0on"] = settings.water0on;
    obj["water0off"] = settings.water0off;
    obj["water1on"] = settings.water1on;
    obj["water1off"] = settings.water1off;
    obj["water2on"] = settings.water2on;
    obj["water2off"] = settings.water2off;
    obj["curFlap"] = settings.curFlap;
    obj["minFlap"] = settings.minFlap;
    obj["maxFlap"] = settings.maxFlap;
    obj["timerOn"] = settings.timerOn;
    obj["timerOff"] = settings.timerOff;
    obj["alarm0"] = settings.alarm0;
    obj["alarm1"] = settings.alarm1;
    obj["hyst0"] = settings.hysteresis0;
    obj["hyst1"] = settings.hysteresis1;
    obj["special"] = settings.special;
    obj["deviceNum"] = settings.deviceNum;
    obj["program"] = settings.program;
    obj["modeLight"] = settings.modeLight;
    obj["modeHeater"] = settings.modeHeater;
    obj["modeHumidi"] = settings.modeHumidi;
    obj["modeRelay1"] = settings.modeRelay1;
    obj["modeRelay2"] = settings.modeRelay2;
    obj["modeRelay3"] = settings.modeRelay3;
    obj["botToken"] = botToken;
    obj["chatID"] = chatID;

    File configFile = LittleFS.open("/setpoint.json", "w");
    if (!configFile) {
        MYDEBUG_PRINTLN("Failed to open file for writing");
        sysLogger.log(getMsg(MSG_FS_OPEN_ERR));
        return;
    }
    if (serializeJson(doc, configFile) == 0) {
        MYDEBUG_PRINTLN("Error writing to file");
        sysLogger.log(getMsg(MSG_JSON_ERR));
    } else {
        MYDEBUG_PRINTLN("Configuration saved successfully.");
        sysLogger.log(getMsg(MSG_CONFIG_SAVED));
    }
    configFile.close();
    logicManager.relaySwitch(1);
    logicManager.relaySwitch(2);
    logicManager.relaySwitch(3);
}

/**
 * @brief Load configuration from LittleFS.
 */
bool loadSetPoint() {
    MYDEBUG_PRINTLN("Loading configuration...");

    File configFile = LittleFS.open("/setpoint.json", "r");
    if (!configFile) {
        MYDEBUG_PRINTLN("Failed to open file for reading. Using defaults.");
        return false;
    }

    JsonDocument doc;
    DeserializationError error = deserializeJson(doc, configFile);
    if (error) {
        MYDEBUG_PRINT("JSON deserialization error: ");
        MYDEBUG_PRINTLN(error.c_str());
        configFile.close();
        return false;
    }
    configFile.close();

    JsonObject obj = doc.as<JsonObject>();

    settings.spT0on = obj["spT0on"];
    settings.spT0off = obj["spT0off"];
    settings.spT1on = obj["spT1on"];
    settings.spT1off = obj["spT1off"];
    settings.water0on = obj["water0on"];
    settings.water0off = obj["water0off"];
    settings.water1on = obj["water1on"];
    settings.water1off = obj["water1off"];
    settings.water2on = obj["water2on"];
    settings.water2off = obj["water2off"];
    settings.curFlap = obj["curFlap"] | obj["flap"] | 0;
    settings.minFlap = obj["minFlap"] | 0;
    settings.maxFlap = obj["maxFlap"] | 100;
    settings.timerOn = obj["timerOn"];
    settings.timerOff = obj["timerOff"];
    settings.alarm0 = obj["alarm0"];
    settings.alarm1 = obj["alarm1"];
    settings.hysteresis0 = obj["hyst0"] | 5; // Default 0.5 if not found
    settings.hysteresis1 = obj["hyst1"] | 5;
    settings.special = obj["special"];
    settings.deviceNum = obj["deviceNum"];
    settings.program = obj["program"];
    settings.modeLight = obj["modeLight"];
    settings.modeHeater = obj["modeHeater"];
    settings.modeHumidi = obj["modeHumidi"];
    settings.modeRelay1 = obj["modeRelay1"];
    settings.modeRelay2 = obj["modeRelay2"];
    settings.modeRelay3 = obj["modeRelay3"];

    if (obj["botToken"].is<const char*>()) {
        strncpy(botToken, obj["botToken"] | "", sizeof(botToken) - 1);
        botToken[sizeof(botToken) - 1] = '\0';
    }
    if (obj["chatID"].is<const char*>()) {
        strncpy(chatID, obj["chatID"] | "", sizeof(chatID) - 1);
        chatID[sizeof(chatID) - 1] = '\0';
    }
  
    MYDEBUG_PRINTLN("Configuration loaded successfully.");
    return true;
}

#ifdef DEBUG
/**
 * @brief Helper to print sensor address.
 */
void printAddress(DeviceAddress deviceAddress) {
  for (uint8_t i = 0; i < 8; i++) {
    if (deviceAddress[i] < 16) MYDEBUG_PRINT("0");
    MYDEBUG_PRINT(deviceAddress[i], HEX);
    if (i < 7) MYDEBUG_PRINT(":");
  }
}
#endif

/**
 * @brief Psychrometric table calculation for humidity.
 */
uint8_t tableRH(int16_t maxT, int16_t minT){
  int16_t dT = 255;
  if (maxT > 99 && maxT < 450){
     dT = (maxT - minT) * 16 / 10;
     if (dT < 0) dT = 240;
     maxT /= 10;
     if (maxT < 20) maxT = 20;
     if (maxT > 40) maxT = 40;
     dT >>= 3;
     if (dT > 20) dT = 255;
     else if (dT == 0) dT = 100;
     else {maxT -= 20; maxT *= 20; maxT += (dT - 1); dT = tabRH[maxT];};
  }
  return dT;
}

/**
 * @brief Helper to print binary representation of a byte.
 */
void printBinary(unsigned char byte) {
  for (int i = 7; i >= 0; i--) {
    if(i > 5) {MYDEBUG_PRINT("x");}
    else {MYDEBUG_PRINT(bitRead(byte, i));}
  }
    MYDEBUG_PRINTLN("\n-----------------");
}

/**
 * @brief Reset all settings to hardcoded defaults.
 */
void reset(void){
    settings.spT0on = T0ON * 10;
    settings.spT0off = T0OFF * 10;
    settings.spT1on = T1ON * 10;
    settings.spT1off = T1OFF * 10;
    settings.water0on = WT0ON;
    settings.water0off = WT0OFF;
    settings.water1on = WT1ON;
    settings.water1off = WT1OFF;
    settings.water2on = WT2ON;
    settings.water2off = WT2OFF;
    settings.curFlap = 0;
    settings.minFlap = 0;
    settings.maxFlap = 100;
    settings.timerOn = TIMERON;
    settings.timerOff = TIMEROFF;
    settings.alarm0 = ALARM0;
    settings.alarm1 = ALARM1;
    settings.hysteresis0 = HYSTER0;
    settings.hysteresis1 = HYSTER1;
    settings.special = 0;
    settings.deviceNum = 0;
    settings.program = 0;
    settings.modeLight = 0;
    settings.modeHeater = 0;
    settings.modeHumidi = 0;
    settings.modeRelay1 = 0;
    settings.modeRelay2 = 0;
    settings.modeRelay3 = 0;

    beeperOn(50);
    delay(500);
    beeperOn(50);
    delay(500);
    saveSetPoint();
    beeperOn(100);
    delay(3000);
}

/**
 * @brief Performs a safe factory reset by deleting only user data files and keeping system HTMLs.
 */
void safeFactoryReset(void) {
    MYDEBUG_PRINTLN("FACTORY RESET...");
    delay(500);

    // 1. Очищаем суточные графики в EEPROM
    clearEEPROM();
    
    // 2. Удаляем файлы графиков, статистики и логов из LittleFS, оставляя HTML-страницы веб-интерфейса
    Dir dir = LittleFS.openDir("/");
    while (dir.next()) {
        String fileName = dir.fileName();
        if (fileName.startsWith("day_") || fileName.equals("system.log")) {
            LittleFS.remove(fileName);
            MYDEBUG_PRINT("Removed user file: ");
            MYDEBUG_PRINTLN(fileName);
        }
    }

    // 3. Сбрасываем настройки в дефолтные и сохраняем
    reset(); 
}



/**
 * @brief Synchronize system time with NTP and update RTC.
 */
bool syncTime() {
  MYDEBUG_PRINTLN("\nStarting time synchronization...");
  if (WiFi.status() != WL_CONNECTED) {
    MYDEBUG_PRINTLN("WiFi not connected. Skipping NTP sync.");
    return true;
  }
  configTzTime(tzInfo, ntpServer);
  MYDEBUG_PRINT("Waiting for NTP response");
  unsigned long startAttempt = millis();
  while (time(nullptr) < 1000000000) {
    if (millis() - startAttempt > 10000) {
      MYDEBUG_PRINTLN("\nFailed to obtain time (timeout).");
      return true;
    }
    MYDEBUG_PRINT(".");
    delay(1000);
  }
  MYDEBUG_PRINTLN("\nTime successfully synchronized.");
  rtc.adjust(DateTime(time(nullptr)));
  MYDEBUG_PRINTLN("RTC time has been updated.");
  lastSyncDay = rtc.now().day();
  return false;
}
