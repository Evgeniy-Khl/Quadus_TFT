// server.cpp
#include <main.h>
#include "server.h"

extern uint8_t  seconds, mode, quarter;
extern long lastSendTime;
extern int tableData[32][4], tmrTelegramOff;

void notFoundHandler() {
    server.send(404, "text/plain", "Not found");
}

/**
 * @brief Respond with current system values in JSON format.
 */
void respondsValues() {
    char txt[64];
    uint8_t num = settings.deviceNum & 0x0F;
    tmrTelegramOff = 300;
    JsonDocument data;
    
    data["model"] = "Прилад&nbsp;&nbsp;&nbsp;&nbsp;№ " + String(num);
    
    snprintf(txt, sizeof(txt), "%d.%d", ds[0].pvT / 10, abs(ds[0].pvT % 10));
    data["temperature0"] = txt;
    
    snprintf(txt, sizeof(txt), "[%d.%d - %d.%d]", 
             settings.spT0on / 10, abs(settings.spT0on % 10),
             settings.spT0off / 10, abs(settings.spT0off % 10));
    data["settemp0"] = txt;

    if(hasDHT22){
        snprintf(txt, sizeof(txt), "%d.%d", pvRH / 10, abs(pvRH % 10));
        data["humidity"] = txt;
        snprintf(txt, sizeof(txt), "[%d.%d - %d.%d]", 
                 settings.spT1on / 10, abs(settings.spT1on % 10),
                 settings.spT1off / 10, abs(settings.spT1off % 10));
        data["sethum"] = txt;
        data["isTableRH"] = false;
    }
    else {
        if (numberOfDS18 >= 2) {
            snprintf(txt, sizeof(txt), "%d.%d", ds[1].pvT / 10, abs(ds[1].pvT % 10));
            data["temperature1"] = txt;
        }
        
        snprintf(txt, sizeof(txt), "[%d.%d - %d.%d]", 
                 settings.spT1on / 10, abs(settings.spT1on % 10),
                 settings.spT1off / 10, abs(settings.spT1off % 10));
        data["settemp1"] = txt;
        
        if (numberOfDS18 >= 2) {
            snprintf(txt, sizeof(txt), "%d.%d", pvRH / 10, abs(pvRH % 10));
            data["humidity"] = txt;
            data["sethum"] = data["settemp1"];
            data["isTableRH"] = true;
        } else {
            data["isTableRH"] = false;
        }
    }
    
    if (timeinfo) {
        snprintf(txt, sizeof(txt), "%s %02u:%02u [%02u - %02u]", 
                 LIGHT == PCF_OFF ? "↓" : "↑", 
                 timeinfo->tm_hour, timeinfo->tm_min, 
                 settings.timerOn, settings.timerOff);
    } else {
        snprintf(txt, sizeof(txt), "%s 00:00 [%02u - %02u]", 
                 LIGHT == PCF_OFF ? "↓" : "↑", 
                 settings.timerOn, settings.timerOff);
    }
    data["light"] = txt;

    auto formatTimer = [&](int16_t pvTime, bool relayState, int8_t manualMode, const char* label) {
        if (manualMode != -1) return String(label) + (relayState == PCF_OFF ? " Ручне OFF" : " Ручне ON");
        if (pvTime == -1) return String(label) + " немає дозволу";
        if (relayState == PCF_OFF) { // OFF phase
            uint8_t day = pvTime / 1440;
            uint8_t hour = (pvTime % 1440) / 60;
            uint8_t min = pvTime % 60;
            snprintf(txt, sizeof(txt), "↓ OFF %d діб %d год. %d хвл.", day, hour, min);
        } else { // ON phase
            snprintf(txt, sizeof(txt), "↑ ON %d хвл.", pvTime);
        }
        return String(txt);
    };

    data["timer1"] = formatTimer(pvTimeR1, RELAY1, dataOut[3], "T1");
    data["timer2"] = formatTimer(pvTimeR2, RELAY2, dataOut[4], "T2");
    data["timer3"] = formatTimer(pvTimeR3, RELAY3, dataOut[5], "T3");

    data["mR1"] = settings.modeRelay1 & 0x0F;
    data["mR2"] = settings.modeRelay2 & 0x0F;
    data["mR3"] = settings.modeRelay3 & 0x0F;
    
    data["errorsFlag"] = errorsFlag.value;
    data["error1"] = ERROR1;
    data["error2"] = ERROR2;
    data["error4"] = ERROR4;
    data["error8"] = ERROR8;
    
    // Raw values for alarm calculations on client side
    data["pvT0"] = ds[0].pvT;
    data["spT0min"] = min(settings.spT0on, settings.spT0off);
    data["spT0max"] = max(settings.spT0on, settings.spT0off);
    data["alarm0"] = settings.alarm0;
    
    data["pvT1"] = ds[1].pvT;
    data["spT1min"] = min(settings.spT1on, settings.spT1off);
    data["spT1max"] = max(settings.spT1on, settings.spT1off);
    data["alarm1"] = settings.alarm1;

    data["flap"] = String(pvFlap) + "%";
    
    data["program"] = ((settings.program & 0xF) == 0) ? "немає" : "#" + String(settings.program & 0xF);
    
    if (timeinfo) {
        snprintf(txt, sizeof(txt), "%02d.%02d.%04d %02d:%02d:%02d",
                 timeinfo->tm_mday, timeinfo->tm_mon + 1,
                 timeinfo->tm_year + 1900, timeinfo->tm_hour,
                 timeinfo->tm_min, timeinfo->tm_sec);
    } else {
        snprintf(txt, sizeof(txt), "00.00.0000 00:00:00");
    }
    data["currDay"] = txt;
    
    data["led0"] = dataLed[0] ? "ON" : "OFF";
    data["led1"] = dataLed[1] ? "ON" : "OFF";
    data["led2"] = dataLed[2] ? "ON" : "OFF";
    data["led3"] = dataLed[3] ? "ON" : "OFF";
    data["led4"] = dataLed[4] ? "ON" : "OFF";
    data["led5"] = dataLed[5] ? "ON" : "OFF";
    data["led6"] = dataLed[6] ? "ON" : "OFF";
    
    String response;
    serializeJson(data, response);
    server.send(200, "application/json", response);
}

void respondsEeprom() {
    JsonDocument doc;
    doc["spT0on"] = settings.spT0on;
    doc["spT0off"] = settings.spT0off;
    doc["spT1on"] = settings.spT1on;
    doc["spT1off"] = settings.spT1off;
    doc["water0on"] = settings.water0on;
    doc["water0off"] = settings.water0off;
    doc["water1on"] = settings.water1on;
    doc["water1off"] = settings.water1off;
    doc["water2on"] = settings.water2on;
    doc["water2off"] = settings.water2off;
    doc["flpNow"] = settings.curFlap;
    doc["minFlap"] = settings.minFlap;
    doc["maxFlap"] = settings.maxFlap;
    doc["timerOn"] = settings.timerOn;
    doc["timerOff"] = settings.timerOff;
    doc["alarm0"] = settings.alarm0;
    doc["alarm1"] = settings.alarm1;
    doc["hyst0"] = settings.hysteresis0;
    doc["hyst1"] = settings.hysteresis1;
    doc["deviceNum"] = settings.deviceNum;
    doc["program"] = settings.program;
    doc["modeHeater"] = settings.modeHeater & 0x0F;
    doc["modeHumidi"] = settings.modeHumidi & 0x0F;
    doc["modeRelay1"] = settings.modeRelay1 & 0x0F;
    doc["modeRelay2"] = settings.modeRelay2 & 0x0F;
    doc["modeRelay3"] = settings.modeRelay3 & 0x0F;
    doc["botToken"] = botToken;
    doc["chatID"] = chatID;
    doc["status"] = 1;

    String response;
    serializeJson(doc, response);
    server.send(200, "application/json", response);
    
    mode = SAVEEEPROM; 
    interval = INTERVAL_1000;
}

void acceptEeprom() {
    if (server.hasArg("plain")) {
        bool success = false;
        
        // Запоминаем старые значения
        int16_t old_spT0on = settings.spT0on;
        int16_t old_spT0off = settings.spT0off;
        int16_t old_spT1on = settings.spT1on;
        int16_t old_spT1off = settings.spT1off;
        uint8_t old_water0on = settings.water0on;
        uint8_t old_water0off = settings.water0off;
        uint8_t old_water1on = settings.water1on;
        uint8_t old_water1off = settings.water1off;
        uint8_t old_water2on = settings.water2on;
        uint8_t old_water2off = settings.water2off;
        uint8_t old_curFlap = settings.curFlap;
        uint8_t old_minFlap = settings.minFlap;
        uint8_t old_maxFlap = settings.maxFlap;
        uint8_t old_timerOn = settings.timerOn;
        uint8_t old_timerOff = settings.timerOff;
        int16_t old_alarm0 = settings.alarm0;
        int16_t old_alarm1 = settings.alarm1;
        int16_t old_hyst0 = settings.hysteresis0;
        int16_t old_hyst1 = settings.hysteresis1;
        uint8_t old_deviceNum = settings.deviceNum;
        uint8_t old_program = settings.program;
        uint8_t old_modeHeater = settings.modeHeater;
        uint8_t old_modeHumidi = settings.modeHumidi;
        uint8_t old_modeRelay1 = settings.modeRelay1;
        uint8_t old_modeRelay2 = settings.modeRelay2;
        uint8_t old_modeRelay3 = settings.modeRelay3;

        // Scope to free memory immediately after parsing
        {
            JsonDocument doc;
            DeserializationError error = deserializeJson(doc, server.arg("plain"));
            
            if (!error) {
                JsonObject obj = doc.as<JsonObject>();
                
                auto getScaled = [&](const char* key, int16_t current) -> int16_t {
                    if (obj[key].is<float>() || obj[key].is<int>()) return (int16_t)round(obj[key].as<float>() * 10.0);
                    return current;
                };
                auto getUint8 = [&](const char* key, uint8_t current) -> uint8_t {
                    if (obj[key].is<int>()) return obj[key].as<uint8_t>();
                    return current;
                };

                settings.spT0on = getScaled("spT0on", settings.spT0on);
                settings.spT0off = getScaled("spT0off", settings.spT0off);
                settings.spT1on = getScaled("spT1on", settings.spT1on);
                settings.spT1off = getScaled("spT1off", settings.spT1off);
                settings.water0on = getUint8("water0on", settings.water0on);
                settings.water0off = getUint8("water0off", settings.water0off);
                settings.water1on = getUint8("water1on", settings.water1on);
                settings.water1off = getUint8("water1off", settings.water1off);
                settings.water2on = getUint8("water2on", settings.water2on);
                settings.water2off = getUint8("water2off", settings.water2off);
                settings.curFlap = getUint8("flpNow", settings.curFlap);
                settings.minFlap = getUint8("minFlap", settings.minFlap);
                settings.maxFlap = getUint8("maxFlap", settings.maxFlap);
                settings.timerOn = getUint8("timerOn", settings.timerOn);
                settings.timerOff = getUint8("timerOff", settings.timerOff);
                settings.alarm0 = getScaled("alarm0", settings.alarm0);
                settings.alarm1 = getScaled("alarm1", settings.alarm1);
                settings.hysteresis0 = getScaled("hyst0", settings.hysteresis0);
                settings.hysteresis1 = getScaled("hyst1", settings.hysteresis1);
                settings.deviceNum = getUint8("deviceNum", settings.deviceNum);
                settings.program = getUint8("program", settings.program);
                settings.modeHeater = getUint8("modeHeater", settings.modeHeater);
                settings.modeHumidi = getUint8("modeHumidi", settings.modeHumidi);
                settings.modeRelay1 = getUint8("modeRelay1", settings.modeRelay1);
                settings.modeRelay2 = getUint8("modeRelay2", settings.modeRelay2);
                settings.modeRelay3 = getUint8("modeRelay3", settings.modeRelay3);

                if (obj["botToken"].is<const char*>()) {
                    strncpy(botToken, obj["botToken"] | "", sizeof(botToken) - 1);
                    botToken[sizeof(botToken) - 1] = '\0';
                }
                if (obj["chatID"].is<const char*>()) {
                    strncpy(chatID, obj["chatID"] | "", sizeof(chatID) - 1);
                    chatID[sizeof(chatID) - 1] = '\0';
                }

                // Логируем только изменённые параметры
                String logStr = "Налаштування: ";
                bool anyChanged = false;
                char tmp[64];

                #define LOG_CHANGED_F(label, oldVal, newVal, divisor) \
                  if ((oldVal) != (newVal)) { \
                    if (anyChanged) logStr += " | "; \
                    snprintf(tmp, sizeof(tmp), label "%.1f->%.1f", (float)(oldVal)/(divisor), (float)(newVal)/(divisor)); \
                    logStr += tmp; anyChanged = true; \
                  }
                #define LOG_CHANGED_D(label, oldVal, newVal) \
                  if ((oldVal) != (newVal)) { \
                    if (anyChanged) logStr += " | "; \
                    snprintf(tmp, sizeof(tmp), label "%d->%d", (int)(oldVal), (int)(newVal)); \
                    logStr += tmp; anyChanged = true; \
                  }

                LOG_CHANGED_F("T1 увімк.:", old_spT0on, settings.spT0on, 10.0f)
                LOG_CHANGED_F("T1 вимк.:", old_spT0off, settings.spT0off, 10.0f)
                LOG_CHANGED_F("T2 увімк.:", old_spT1on, settings.spT1on, 10.0f)
                LOG_CHANGED_F("T2 вимк.:", old_spT1off, settings.spT1off, 10.0f)
                LOG_CHANGED_D("Вол.0 увімк.:", old_water0on, settings.water0on)
                LOG_CHANGED_D("Вол.0 вимк.:", old_water0off, settings.water0off)
                LOG_CHANGED_D("Вол.1 увімк.:", old_water1on, settings.water1on)
                LOG_CHANGED_D("Вол.1 вимк.:", old_water1off, settings.water1off)
                LOG_CHANGED_D("Вол.2 увімк.:", old_water2on, settings.water2on)
                LOG_CHANGED_D("Вол.2 вимк.:", old_water2off, settings.water2off)
                LOG_CHANGED_D("Заслінка:", old_curFlap, settings.curFlap)
                LOG_CHANGED_D("Заслінка мін.:", old_minFlap, settings.minFlap)
                LOG_CHANGED_D("Заслінка макс.:", old_maxFlap, settings.maxFlap)
                LOG_CHANGED_D("Таймер увімк.:", old_timerOn, settings.timerOn)
                LOG_CHANGED_D("Таймер вимк.:", old_timerOff, settings.timerOff)
                LOG_CHANGED_F("Аварія 1:", old_alarm0, settings.alarm0, 10.0f)
                LOG_CHANGED_F("Аварія 2:", old_alarm1, settings.alarm1, 10.0f)
                LOG_CHANGED_F("Гіст. 1:", old_hyst0, settings.hysteresis0, 10.0f)
                LOG_CHANGED_F("Гіст. 2:", old_hyst1, settings.hysteresis1, 10.0f)
                LOG_CHANGED_D("Пристрій №:", old_deviceNum, settings.deviceNum)
                LOG_CHANGED_D("Програма:", old_program, settings.program)
                LOG_CHANGED_D("Режим нагрів.:", old_modeHeater, settings.modeHeater)
                LOG_CHANGED_D("Режим волог.:", old_modeHumidi, settings.modeHumidi)
                LOG_CHANGED_D("Режим реле 1:", old_modeRelay1, settings.modeRelay1)
                LOG_CHANGED_D("Режим реле 2:", old_modeRelay2, settings.modeRelay2)
                LOG_CHANGED_D("Режим реле 3:", old_modeRelay3, settings.modeRelay3)

                #undef LOG_CHANGED_F
                #undef LOG_CHANGED_D

                if (anyChanged) {
                    sysLogger.log(logStr);
                }

                success = true;
            }
        }
        
        if (success) {
            server.send(200, "application/json", "{\"status\":\"ok\"}");
            // Call saveSetPoint AFTER sending the response
            saveSetPoint();
            return;
        }
    }
    server.send(400, "application/json", "{\"error\":\"Invalid JSON\"}");
}

void respondsProgram() {
    uint8_t prg = 1;
    if (server.hasArg("prg")) {
        prg = server.arg("prg").toInt();
    }
    if (prg < 1 || prg > 4) prg = 1;

    JsonDocument doc;
    JsonArray arr = doc.to<JsonArray>();

    for (uint8_t i = 0; i < 24; i++) {
        uint16_t memoryAddress = eepromMemoryAddressForHour(prg, i);
        eepromRdBuff(memoryAddress, unTable.buffer, sizeof(unTable));
        
        JsonObject hourObj = arr.add<JsonObject>();
        hourObj["hour"] = i;
        hourObj["spT0on"] = unTable.spProg.spT0on / 10.0f;
        hourObj["spT0off"] = unTable.spProg.spT0off / 10.0f;
        hourObj["spT1on"] = unTable.spProg.spT1on / 10.0f;
        hourObj["spT1off"] = unTable.spProg.spT1off / 10.0f;
        hourObj["flapMin"] = unTable.spProg.flapMin;
        hourObj["flapMax"] = unTable.spProg.flapMax;
        hourObj["flapCurr"] = unTable.spProg.flapCurr;
        hourObj["water2run"] = unTable.spProg.water2run;
    }

    String response;
    serializeJson(doc, response);
    server.send(200, "application/json", response);
    tmrTelegramOff = 300;
}

void acceptProgram() {
    uint8_t prg = 1;
    if (server.hasArg("prg")) {
        prg = server.arg("prg").toInt();
    }
    if (prg < 1 || prg > 4) prg = 1;

    if (server.hasArg("plain")) {
        JsonDocument doc;
        DeserializationError error = deserializeJson(doc, server.arg("plain"));
        if (error) {
            server.send(400, "application/json", "{\"error\":\"Invalid JSON\"}");
            return;
        }

        JsonArray arr = doc.as<JsonArray>();
        if (arr.size() != 24) {
            server.send(400, "application/json", "{\"error\":\"Must contain exactly 24 hours\"}");
            return;
        }

        for (uint8_t i = 0; i < 24; i++) {
            JsonObject hourObj = arr[i];
            
            unTable.spProg.spT0on = (int16_t)round(hourObj["spT0on"].as<float>() * 10.0f);
            unTable.spProg.spT0off = (int16_t)round(hourObj["spT0off"].as<float>() * 10.0f);
            unTable.spProg.spT1on = (int16_t)round(hourObj["spT1on"].as<float>() * 10.0f);
            unTable.spProg.spT1off = (int16_t)round(hourObj["spT1off"].as<float>() * 10.0f);
            unTable.spProg.flapMin = hourObj["flapMin"].as<uint8_t>();
            unTable.spProg.flapMax = hourObj["flapMax"].as<uint8_t>();
            unTable.spProg.flapCurr = hourObj["flapCurr"].as<uint8_t>();
            unTable.spProg.water2run = hourObj["water2run"].as<uint8_t>();

            uint16_t memoryAddress = eepromMemoryAddressForHour(prg, i);
            eepromWrBuff(memoryAddress, unTable.buffer, sizeof(unTable));
            yield();
        }

        // Если сохранили текущую активную программу, мгновенно применим ее настройки
        if (settings.program == prg) {
            if (timeinfo) {
                uint8_t currentHour = timeinfo->tm_hour;
                uint16_t memoryAddress = eepromMemoryAddressForHour(prg, currentHour);
                eepromRdBuff(memoryAddress, unTable.buffer, sizeof(unTable));
                
                if (unTable.spProg.spT0on != -1 && (uint16_t)unTable.spProg.spT0on != 0xFFFF) {
                    settings.spT0on = unTable.spProg.spT0on;
                    settings.spT0off = unTable.spProg.spT0off;
                    settings.spT1on = unTable.spProg.spT1on;
                    settings.spT1off = unTable.spProg.spT1off;
                    settings.minFlap = (unTable.spProg.flapMin <= 100) ? unTable.spProg.flapMin : 0;
                    settings.maxFlap = (unTable.spProg.flapMax <= 100) ? unTable.spProg.flapMax : 100;
                    settings.curFlap = (unTable.spProg.flapCurr <= 100) ? unTable.spProg.flapCurr : settings.minFlap;
                    settings.water2on = unTable.spProg.water2run;
                    if (unTable.spProg.water2run > 0) {
                        RELAY3 = PCF_ON;
                        pvTimeR3 = unTable.spProg.water2run;
                    } else {
                        RELAY3 = PCF_OFF;
                        pvTimeR3 = -1;
                    }
                }
            }
        }

        server.send(200, "application/json", "{\"status\":\"ok\"}");
        MYDEBUG_PRINTLN("Hourly program updated");
    } else {
        server.send(400, "application/json", "{\"error\":\"no data\"}");
    }
}

void handleManualControl() {
    if (server.hasArg("plain")) {
        JsonDocument doc;
        DeserializationError error = deserializeJson(doc, server.arg("plain"));
        if (!error) {
            if (doc["rel1"].is<int8_t>()) { dataOut[0] = doc["rel1"].as<int8_t>(); if (dataOut[0] != -1) LIGHT = (dataOut[0] == 1) ? PCF_ON : PCF_OFF; sysLogger.log(String(getMsg(MSG_MANUAL_LIGHT)) + dataOut[0]); }
            if (doc["rel2"].is<int8_t>()) { dataOut[1] = doc["rel2"].as<int8_t>(); if (dataOut[1] != -1) HEATER = (dataOut[1] == 1) ? PCF_ON : PCF_OFF; sysLogger.log(String(getMsg(MSG_MANUAL_HEATER)) + dataOut[1]); }
            if (doc["rel3"].is<int8_t>()) { dataOut[2] = doc["rel3"].as<int8_t>(); if (dataOut[2] != -1) HUMIDI = (dataOut[2] == 1) ? PCF_ON : PCF_OFF; sysLogger.log(String(getMsg(MSG_MANUAL_HUMIDI)) + dataOut[2]); }
            if (doc["rel4"].is<int8_t>()) { dataOut[3] = doc["rel4"].as<int8_t>(); if (dataOut[3] != -1) { RELAY1 = (dataOut[3] == 1) ? PCF_ON : PCF_OFF; pvTimeR1 = -1; } sysLogger.log(String(getMsg(MSG_MANUAL_RELAY1)) + dataOut[3]); }
            if (doc["rel5"].is<int8_t>()) { dataOut[4] = doc["rel5"].as<int8_t>(); if (dataOut[4] != -1) { RELAY2 = (dataOut[4] == 1) ? PCF_ON : PCF_OFF; pvTimeR2 = -1; } sysLogger.log(String(getMsg(MSG_MANUAL_RELAY2)) + dataOut[4]); }
            if (doc["rel6"].is<int8_t>()) { dataOut[5] = doc["rel6"].as<int8_t>(); if (dataOut[5] != -1) { RELAY3 = (dataOut[5] == 1) ? PCF_ON : PCF_OFF; pvTimeR3 = -1; } sysLogger.log(String(getMsg(MSG_MANUAL_RELAY3)) + dataOut[5]); }
            
            server.send(200, "application/json", "{\"status\":\"ok\"}");
            return;
        }
    }
    server.send(400, "application/json", "{\"error\":\"Invalid JSON\"}");
}

void resetAutoControl() {
    for (int i = 0; i < 6; i++) dataOut[i] = -1;
    sysLogger.log(getMsg(MSG_AUTO_RESTORED));
    server.send(200, "application/json", "{\"status\":\"ok\"}");
}

void handleGetRelayStates() {
    JsonDocument doc;
    doc["rel1_m"] = dataOut[0];
    doc["rel2_m"] = dataOut[1];
    doc["rel3_m"] = dataOut[2];
    doc["rel4_m"] = dataOut[3];
    doc["rel5_m"] = dataOut[4];
    doc["rel6_m"] = dataOut[5];
    
    doc["rel1"] = (LIGHT == PCF_ON);
    doc["rel2"] = (HEATER == PCF_ON);
    doc["rel3"] = (HUMIDI == PCF_ON);
    doc["rel4"] = (RELAY1 == PCF_ON);
    doc["rel5"] = (RELAY2 == PCF_ON);
    doc["rel6"] = (RELAY3 == PCF_ON);

    String response;
    serializeJson(doc, response);
    server.send(200, "application/json", response);
}

void handleGetLogs() {
    String filename;
    if (server.hasArg("day") && server.arg("day") != "") {
        filename = "/day_" + server.arg("day") + "_log.txt";
    } else {
        filename = sysLogger.getLogFilename();
    }

    if (LittleFS.exists(filename)) {
        File f = LittleFS.open(filename, "r");
        streamFileChunked(f, "text/plain");
        f.close();
    } else {
        server.send(200, "text/plain", "No logs found for this day.");
    }
}

void handleClearLogs() {
    sysLogger.clear();
    server.send(200, "application/json", "{\"status\":\"ok\"}");
}

void sendPageHeader(String title) {
    server.sendContent(F("<!DOCTYPE html><html><head><meta charset='utf-8'>"));
    server.sendContent(F("<meta name='viewport' content='width=device-width, initial-scale=1.0'>"));
    server.sendContent("<title>" + title + "</title>");
    server.sendContent(F("<script src='https://cdn.jsdelivr.net/npm/chart.js'></script>"));
    server.sendContent(F("<style>"));
    server.sendContent(F("@import url('https://fonts.googleapis.com/css2?family=Inter:wght@300;400;500;600;700&display=swap');"));
    server.sendContent(F("body{font-family:'Inter',sans-serif;background:linear-gradient(135deg,#0f172a 0%,#1e293b 100%);color:#f8fafc;min-height:100vh;margin:0;padding:20px 10px;display:flex;flex-direction:column;align-items:center}"));
    server.sendContent(F("*{box-sizing:border-box}.container{max-width:800px;width:100%;margin:0 auto;padding:20px;background:rgba(255,255,255,0.06);backdrop-filter:blur(12px);-webkit-backdrop-filter:blur(12px);border:1px solid rgba(255,255,255,0.1);border-radius:16px;box-shadow:0 8px 32px 0 rgba(0,0,0,0.2)}"));
    server.sendContent(F("h1{text-align:center;color:#fff;margin-bottom:20px;font-weight:700}"));
    server.sendContent(F(".chart-container{position:relative;margin:20px auto;height:40vh;width:100%;background:rgba(255,255,255,0.03);border:1px solid rgba(255,255,255,0.05);border-radius:12px;padding:10px}"));
    server.sendContent(F("table{border-collapse:collapse;width:100%;margin:20px auto;font-size:0.95rem;border-radius:12px;overflow:hidden;border:1px solid rgba(255,255,255,0.08)}"));
    server.sendContent(F("th,td{border:1px solid rgba(255,255,255,0.05);text-align:center;padding:12px}"));
    server.sendContent(F("th{background-color:rgba(15,23,42,0.5);color:#94a3b8;font-weight:600;text-transform:uppercase;font-size:0.75rem;letter-spacing:0.03em}"));
    server.sendContent(F("tr{transition:background-color .2s}tr:nth-child(even){background-color:rgba(255,255,255,0.01)}tr:hover{background-color:rgba(255,255,255,0.03)}"));
    server.sendContent(F("ul{list-style-type:none;padding:0;display:flex;flex-direction:column;gap:10px}li{margin:0}"));
    server.sendContent(F("a{display:block;padding:14px 20px;background:linear-gradient(135deg,#2563eb 0%,#1d4ed8 100%);color:white;text-align:center;text-decoration:none;border-radius:12px;font-size:1.05rem;font-weight:600;box-shadow:0 4px 12px rgba(37,99,235,0.2);transition:all 0.2s ease}"));
    server.sendContent(F("a:hover{transform:translateY(-2px);box-shadow:0 6px 16px rgba(37,99,235,0.35);background:linear-gradient(135deg,#3b82f6 0%,#2563eb 100%)}"));
    server.sendContent(F("a.back, a.btn{background:rgba(255,255,255,0.08);border:1px solid rgba(255,255,255,0.1);display:inline-block;padding:12px 24px;margin:20px 0;box-shadow:none}"));
    server.sendContent(F("a.back:hover, a.btn:hover{background:rgba(255,255,255,0.12);transform:translateY(-2px)}"));
    server.sendContent(F("a.live{background:linear-gradient(135deg,#10b981 0%,#059669 100%);box-shadow:0 4px 12px rgba(16,185,129,0.2)}a.live:hover{background:linear-gradient(135deg,#34d399 0%,#10b981 100%);box-shadow:0 6px 16px rgba(16,185,129,0.35)}"));
    server.sendContent(F(".summary{background-color:rgba(59,130,246,0.1);font-weight:bold;color:#60a5fa}"));
    server.sendContent(F(".archive-item{display:flex;justify-content:space-between;align-items:center;padding:12px 16px;background:rgba(255,255,255,0.03);border:1px solid rgba(255,255,255,0.05);border-radius:12px;margin-bottom:8px}"));
    server.sendContent(F(".btn-group{display:flex;gap:8px}"));
    server.sendContent(F("a.btn-archive{display:inline-block;padding:8px 16px;font-size:0.85rem;border-radius:8px;box-shadow:none}"));
    server.sendContent(F("a.btn-archive:hover{transform:translateY(-1px)}"));
    server.sendContent(F("a.btn-graph{background:linear-gradient(135deg,#3b82f6 0%,#1d4ed8 100%)}"));
    server.sendContent(F("a.btn-graph:hover{background:linear-gradient(135deg,#60a5fa 0%,#2563eb 100%);box-shadow:0 4px 12px rgba(59,130,246,0.25)}"));
    server.sendContent(F("a.btn-logs{background:linear-gradient(135deg,#10b981 0%,#047857 100%)}"));
    server.sendContent(F("a.btn-logs:hover{background:linear-gradient(135deg,#34d399 0%,#10b981 100%);box-shadow:0 4px 12px rgba(16,185,129,0.25)}"));
    server.sendContent(F("</style></head><body>"));
}

void handleGetGraph() {
    if (!server.hasArg("day") || server.arg("day") == "") {
        server.send(400, "text/plain", "Bad Request: 'day' parameter is missing");
        return;
    }
    String day = server.arg("day");
    String filename = "/day_" + day + "_graph.json";
    
    if (LittleFS.exists(filename)) {
        File file = LittleFS.open(filename, "r");
        streamFileChunked(file, "application/json");
        file.close();
    } else {
        server.send(404, "text/plain", "File Not Found");
    }
}

void handleGetCurrentGraph() {
    int startH = 0, startM = 0;
    int currentPeriod = 287;
    if (timeinfo) {
        currentPeriod = (timeinfo->tm_hour * 60 + timeinfo->tm_min) / 5;
    }

    JsonDocument doc;
    doc["sh"] = startH;
    doc["sm"] = startM;
    JsonArray array = doc["points"].to<JsonArray>();

    for (int period = 0; period <= currentPeriod; period++) {
        int currentAddress = DAILY_DATA_START + period * DAILY_DATA_REC_SIZE;
        int16_t raw_t1 = eepromReadInt16(currentAddress);
        int16_t raw_t2 = eepromReadInt16(currentAddress + 2);
        int16_t raw_rh = eepromReadInt16(currentAddress + 4);

        if (raw_t1 == 0 && raw_t2 == 0 && raw_rh == 0) continue; 

        JsonObject point = array.add<JsonObject>();
        point["p"] = period;
        point["t1"] = (float)raw_t1 / 10.0;
        point["t2"] = (float)raw_t2 / 10.0;
        point["rh"] = (float)raw_rh / 10.0;
    }
    
    server.setContentLength(measureJson(doc));
    server.send(200, "application/json", "");
    serializeJson(doc, server.client());
}

struct ArchiveItem {
    int day;
    int month;
    String dateStr;
    int sortKey;
};

void handleArchiveList() {
    server.setContentLength(CONTENT_LENGTH_UNKNOWN);
    server.send(200, "text/html", "");
    sendPageHeader("Квадус - Архів");

    server.sendContent(F("<div class='container'><h1>Виберіть добу для перегляду</h1>"));
    server.sendContent(F("<a href='/current' class='live' style='margin-bottom:20px;'>Перегляд ПОТОЧНОЇ доби</a>"));
    server.sendContent(F("<ul>"));
    
    std::vector<ArchiveItem> items;
    Dir dir = LittleFS.openDir("/");
    
    int currentDay = 15;
    int currentMonth = 6;
    if (timeinfo && timeinfo->tm_year >= 100) {
        currentDay = timeinfo->tm_mday;
        currentMonth = timeinfo->tm_mon + 1;
    }

    while (dir.next()) {
        String fileName = dir.fileName();
        if (fileName.startsWith("day_") && fileName.endsWith("_graph.json")) {
            // Формат имени: day_DD_MM_graph.json (длина "day_" = 4)
            int day = fileName.substring(4, 6).toInt();
            int month = fileName.substring(7, 9).toInt();
            if (day > 0 && month > 0) {
                int w = month * 32 + day;
                int currentWeight = currentMonth * 32 + currentDay;
                int distance = 0;
                if (w <= currentWeight) {
                    distance = currentWeight - w;
                } else {
                    distance = 384 + currentWeight - w;
                }
                
                char buf[16];
                snprintf(buf, sizeof(buf), "%02d_%02d", day, month);
                ArchiveItem item = {day, month, String(buf), distance};
                items.push_back(item);
            }
        }
    }
    
    // Сортировка: по возрастанию distance (новые файлы имеют меньшую дистанцию)
    std::sort(items.begin(), items.end(), [](const ArchiveItem& a, const ArchiveItem& b) {
        return a.sortKey < b.sortKey;
    });

    for (size_t i = 0; i < items.size(); i++) {
        char link[512];
        snprintf_P(link, sizeof(link), 
                 PSTR("<li class='archive-item'>"
                      "<span style='font-weight:500; font-size:1rem; color:#f8fafc;'>Дата: %02d.%02d</span>"
                      "<span class='btn-group'>"
                        "<a href='/data?day=%s' class='btn-archive btn-graph'>Графік</a>"
                        "<a href='/view_logs?day=%s' class='btn-archive btn-logs'>Логи</a>"
                      "</span>"
                      "</li>"), 
                 items[i].day, items[i].month, items[i].dateStr.c_str(), items[i].dateStr.c_str());
        server.sendContent(link);
        yield();
    }

    server.sendContent(F("</ul><div style='text-align:center;'><a href='/' class='back'>Назад на головну</a></div>"));
    server.sendContent(F("</div></body></html>"));
}

static void formatTimeBuffer(char* buf, size_t size, int period, int sh, int sm) {
    int totalMinutes = (sh * 60 + sm + period * 5) % 1440;
    snprintf(buf, size, "%02d:%02d", totalMinutes / 60, totalMinutes % 60);
}

void handleShowData() {
    if (!server.hasArg("day") || server.arg("day") == "") {
        server.send(400, "text/plain", "Bad Request: 'day' parameter is missing");
        return;
    }
    String day = server.arg("day");

    int startH = 0, startM = 0;

    String statsFilename = "/day_" + day + "_stats.json";
    File statsFile = LittleFS.open(statsFilename, "r");
    JsonDocument statsDoc;
    if (statsFile) {
        deserializeJson(statsDoc, statsFile);
        statsFile.close();
    }

    server.setContentLength(CONTENT_LENGTH_UNKNOWN);
    server.send(200, "text/html", "");
    String displayDay = day;
    displayDay.replace('_', '.');
    sendPageHeader("Квадус - День " + displayDay);

    server.sendContent("<div class='container'><h1 style='text-align:center;'>Дані клімату за " + displayDay + "</h1>");
    server.sendContent(F("<div class='chart-container'><canvas id='tempChart'></canvas></div>"));
    server.sendContent(F("<div style='text-align:center;'><a href='/archive' class='back'>Назад до списку</a></div>"));
    server.sendContent(F("<script>"));
    server.sendContent("const dayNum = \"" + day + "\";");
    server.sendContent("const sh = " + String(startH) + ";");
    server.sendContent("const sm = " + String(startM) + ";");
    server.sendContent(F(R"raw(
    fetch('/get_graph?day=' + dayNum)
      .then(r => r.json())
      .then(data => {
        const labels = data.map(p => {
            let total = (sh * 60 + sm + p.p * 5) % 1440;
            return Math.floor(total / 60).toString().padStart(2, '0') + ':' + (total % 60).toString().padStart(2, '0');
        });
        const t1 = data.map(p => p.t1);
        const t2 = data.map(p => p.t2);
        const rh = data.map(p => p.rh);
        Chart.defaults.color = '#94a3b8';
        Chart.defaults.borderColor = 'rgba(255, 255, 255, 0.08)';
        new Chart(document.getElementById('tempChart'), {
          type: 'line',
          data: {
            labels: labels,
            datasets: [
              { label: 'T1 (°C)', data: t1, borderColor: '#ef4444', backgroundColor: '#ef4444', tension: 0.3, pointRadius: 0, pointHoverRadius: 4, borderWidth: 2 },
              { label: 'T2 (°C)', data: t2, borderColor: '#10b981', backgroundColor: '#10b981', tension: 0.3, pointRadius: 0, pointHoverRadius: 4, borderWidth: 2 },
              { label: 'Вологість (%)', data: rh, borderColor: '#3b82f6', backgroundColor: '#3b82f6', tension: 0.3, pointRadius: 0, pointHoverRadius: 4, borderWidth: 2, yAxisID: 'y1' }
            ]
          },
          options: { 
            responsive: true, 
            maintainAspectRatio: false, 
            scales: { 
              y: { 
                type: 'linear', 
                display: true, 
                position: 'left', 
                title: { display: true, text: 'Темп. (°C)', color: '#94a3b8' }, 
                grid: { color: 'rgba(255, 255, 255, 0.08)' }, 
                ticks: { 
                  color: '#94a3b8',
                  callback: function(value) {
                    return value.toFixed(1);
                  }
                } 
              },
              y1: { type: 'linear', display: true, position: 'right', min: 0, max: 100, grid: { drawOnChartArea: false }, title: { display: true, text: 'Волог. (%)', color: '#94a3b8' }, ticks: { color: '#94a3b8' } },
              x: { grid: { color: 'rgba(255, 255, 255, 0.08)' }, ticks: { color: '#94a3b8' } }
            },
            interaction: { intersect: false, mode: 'index' },
            plugins: {
              legend: { labels: { color: '#f8fafc' } }
            }
          } 
        });
      });
    )raw"));
    server.sendContent(F("</script>"));
    server.sendContent(F("<table><tr><th>Час</th><th>T1 (°C)</th><th>T2 (°C)</th><th>Вологість (%)</th></tr>"));

    if (!statsDoc.isNull()) {
        char summary[256];
        snprintf(summary, sizeof(summary), "<tr><th>Статистика</th><td>Avg: %.1f<br>Min: %.1f<br>Max: %.1f</td><td>Avg: %.1f<br>Min: %.1f<br>Max: %.1f</td><td>Avg: %.1f<br>Min: %.1f<br>Max: %.1f</td></tr>",
                 statsDoc["avg_t1"].as<float>(), statsDoc["min_t1"].as<float>(), statsDoc["max_t1"].as<float>(),
                 statsDoc["avg_t2"].as<float>(), statsDoc["min_t2"].as<float>(), statsDoc["max_t2"].as<float>(),
                 statsDoc["avg_rh"].as<float>(), statsDoc["min_rh"].as<float>(), statsDoc["max_rh"].as<float>());
        server.sendContent(summary);
    }
    
    File graphFile = LittleFS.open("/day_" + day + "_graph.json", "r");
    if (graphFile) {
        JsonDocument tempDoc;
        if (!deserializeJson(tempDoc, graphFile)) {
           JsonArray array = tempDoc.as<JsonArray>();
           for (int i = array.size() - 1; i >= 0; i--) {
                JsonObject point = array[i];
                char row[128];
                char fmtTime[32];
                formatTimeBuffer(fmtTime, sizeof(fmtTime), point["p"].as<int>(), startH, startM);
                snprintf(row, sizeof(row), "<tr><td>%s</td><td>%.1f</td><td>%.1f</td><td>%.1f</td></tr>", 
                         fmtTime, point["t1"].as<float>(), point["t2"].as<float>(), point["rh"].as<float>());
                server.sendContent(row);
                if (i % 20 == 0) yield();
           }
        }
        graphFile.close();
    }
    server.sendContent(F("</table><div style='text-align:center;'><a href='/archive' class='back'>Назад</a></div></div></body></html>"));
}

void handleCurrentData() {
    int startH = 0, startM = 0;

    int currentPeriod = 287;
    if (timeinfo) {
        currentPeriod = (timeinfo->tm_hour * 60 + timeinfo->tm_min) / 5;
    }
    server.setContentLength(CONTENT_LENGTH_UNKNOWN);
    server.send(200, "text/html", "");
    sendPageHeader("Квадус - Поточна доба");
    server.sendContent(F("<div class='container'><h1>Дані за поточну добу</h1>"));
    server.sendContent(F("<div class='chart-container'><canvas id='tempChart'></canvas></div>"));
    server.sendContent(F("<div style='text-align:center;'><a href='/archive' class='back'>Назад до списку</a></div>"));
    
    server.sendContent(F("<script>"));
    server.sendContent("const sh = " + String(startH) + ";");
    server.sendContent("const sm = " + String(startM) + ";");
    server.sendContent(F(R"raw(
    fetch('/get_current_graph')
      .then(r=>r.json())
      .then(json=>{
        const data = json.points;
        const labels = data.map(p => {
            let total = (sh * 60 + sm + p.p * 5) % 1440;
            return Math.floor(total / 60).toString().padStart(2, '0') + ':' + (total % 60).toString().padStart(2, '0');
        });
        const t1 = data.map(p => p.t1);
        const t2 = data.map(p => p.t2);
        const rh = data.map(p => p.rh);
        Chart.defaults.color = '#94a3b8';
        Chart.defaults.borderColor = 'rgba(255, 255, 255, 0.08)';
        new Chart(document.getElementById('tempChart'), {
          type: 'line',
          data: {
            labels: labels,
            datasets: [
              { label: 'T1 (°C)', data: t1, borderColor: '#ef4444', backgroundColor: '#ef4444', tension: 0.3, pointRadius: 0, pointHoverRadius: 4, borderWidth: 2 },
              { label: 'T2 (°C)', data: t2, borderColor: '#10b981', backgroundColor: '#10b981', tension: 0.3, pointRadius: 0, pointHoverRadius: 4, borderWidth: 2 },
              { label: 'Вологість (%)', data: rh, borderColor: '#3b82f6', backgroundColor: '#3b82f6', tension: 0.3, pointRadius: 0, pointHoverRadius: 4, borderWidth: 2, yAxisID: 'y1' }
            ]
          },
          options: { 
            responsive: true, 
            maintainAspectRatio: false, 
            scales: { 
              y: { 
                type: 'linear', 
                display: true, 
                position: 'left', 
                title: { display: true, text: 'Темп. (°C)', color: '#94a3b8' }, 
                grid: { color: 'rgba(255, 255, 255, 0.08)' }, 
                ticks: { 
                  color: '#94a3b8',
                  callback: function(value) {
                    return value.toFixed(1);
                  }
                } 
              },
              y1: { type: 'linear', display: true, position: 'right', min: 0, max: 100, grid: { drawOnChartArea: false }, title: { display: true, text: 'Волог. (%)', color: '#94a3b8' }, ticks: { color: '#94a3b8' } },
              x: { grid: { color: 'rgba(255, 255, 255, 0.08)' }, ticks: { color: '#94a3b8' } }
            },
            interaction: { intersect: false, mode: 'index' },
            plugins: {
              legend: { labels: { color: '#f8fafc' } }
            }
          } 
        });
      });
    )raw"));
    server.sendContent(F("</script>"));
    server.sendContent(F("<table><tr><th>Час</th><th>T1 (°C)</th><th>T2 (°C)</th><th>Вологість (%)</th></tr>"));

    for (int i = currentPeriod; i >= 0; i--) {
        int currentAddress = DAILY_DATA_START + i * DAILY_DATA_REC_SIZE;
        int16_t raw_t1 = eepromReadInt16(currentAddress);
        int16_t raw_t2 = eepromReadInt16(currentAddress + 2);
        int16_t raw_rh = eepromReadInt16(currentAddress + 4);

        if (raw_t1 == 0 && raw_t2 == 0 && raw_rh == 0) continue;

        char row[128];
        char fmtTime[32];
        formatTimeBuffer(fmtTime, sizeof(fmtTime), i, startH, startM);
        snprintf(row, sizeof(row), "<tr><td>%s</td><td>%.1f</td><td>%.1f</td><td>%.1f</td></tr>", 
                 fmtTime, (float)raw_t1/10.0, (float)raw_t2/10.0, (float)raw_rh);
        server.sendContent(row);
        if (i % 20 == 0) yield();
    }
    server.sendContent(F("</table><div style='text-align:center;'><a href='/archive' class='back'>Назад</a></div></div></body></html>"));
}

void streamFileChunked(File& file, const String& contentType) {
    server.sendHeader("Connection", "close");
    server.setContentLength(CONTENT_LENGTH_UNKNOWN);
    server.send(200, contentType, "");
    
    auto client = server.client();
    if (!client) return;

    uint8_t buffer[1024];
    while (file.available()) {
        if (!client.connected()) break; // Прекращаем чтение, если клиент отключился
        size_t len = file.read(buffer, sizeof(buffer));
        if (len > 0) {
            char hexBuf[16];
            snprintf(hexBuf, sizeof(hexBuf), "%X\r\n", (unsigned int)len);
            client.write((const uint8_t*)hexBuf, strlen(hexBuf));
            client.write(buffer, len);
            client.write((const uint8_t*)"\r\n", 2);
        }
        yield();
    }
    if (client.connected()) {
        client.write((const uint8_t*)"0\r\n\r\n", 5);
    }
}

