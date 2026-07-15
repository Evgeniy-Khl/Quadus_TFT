#include "LogicManager.h"
#include "main.h"
#include "procedure.h"

LogicManager logicManager;

void LogicManager::processLighting() {
    if (!RTCENABLE) return;
    
    if (dataOut[0] != -1) {
        LIGHT = (dataOut[0] == 1) ? PCF_ON : PCF_OFF;
        return;
    }

    uint8_t currentHour = timeinfo->tm_hour;
    if (checkLightState(currentHour, settings.timerOn, settings.timerOff)) {
        LIGHT = PCF_OFF;
    } else {
        LIGHT = PCF_ON;
    }
}

void LogicManager::processIrrigation() {
    relaySwitch(1);
    relaySwitch(2);
    relaySwitch(3);
}

void LogicManager::processClimate() {
    // Heater processing
    if (dataOut[1] != -1) {
        HEATER = (dataOut[1] == 1) ? PCF_ON : PCF_OFF;
    } else {
        if(!ERROR1 && !ERROR10 && !DHT_ERR)
            HEATER = checkDeviceState(HEATER, ds[0].pvT, settings.spT0on, settings.spT0off, settings.modeHeater);
        else HEATER = PCF_OFF;
    }

    // Humidifier processing
    if (dataOut[2] != -1) {
        HUMIDI = (dataOut[2] == 1) ? PCF_ON : PCF_OFF;
    } else {
        if (hasDHT22) {
            pvRH = ds[1].pvT;
        } else if (numberOfDS18 >= 2) {
            // Calculate RH using dry/wet bulb temperatures
            // ds[0].pvT is dry, ds[1].pvT is wet
            pvRH = tableRH(ds[0].pvT, ds[1].pvT) * 10;
        }
        if(!ERROR2 && !ERROR20 && !DHT_ERR)
            HUMIDI = checkDeviceState(HUMIDI, ds[1].pvT, settings.spT1on, settings.spT1off, settings.modeHumidi);
        else HUMIDI = PCF_OFF;
    }
}

void LogicManager::processAlarms() {
    processAlarm(0);
    processAlarm(1);
}

void LogicManager::updateStatusLeds() {
    for (uint8_t i = 0; i < 6; i++) {
        uint8_t numBit = 1 << i;
        dataLed[i] = (~portOut.value) & numBit;
    }
    dataLed[6] = errorsFlag.value;
}

void LogicManager::relaySwitch(uint8_t cn) {    // README.md
    // stateBit stores current relay state (active low for PCF8574: 0 = ON, 1 = OFF)
    bool stateBit = PCF_OFF, prnBit = false;
    // val: remaining time, spOn: ON duration, spOff: OFF interval, permit: operation mode
    int16_t val = 0, spOn = 0, spOff = 0, permit = 0;
    int8_t manualMode = dataOut[cn + 2]; // dataOut[3, 4, 5] for relays 1, 2, 3

    if (manualMode != -1) {
        stateBit = (manualMode == 1) ? PCF_ON : PCF_OFF;
        switch (cn) {
            case 1: RELAY1 = stateBit; pvTimeR1 = -1; break;
            case 2: RELAY2 = stateBit; pvTimeR2 = -1; break;
            case 3: RELAY3 = stateBit; pvTimeR3 = -1; break;
        }
        return;
    }
    
    // Step 1: Initialize local variables based on the requested channel
    switch (cn) {
        case 1:
            stateBit = RELAY1;
            val = pvTimeR1;
            spOn = settings.water0on;
            spOff = settings.water0off;
            permit = settings.modeRelay1 & 3; // Mask to get the operating mode bits
            break;
        case 2:
            stateBit = RELAY2;
            val = pvTimeR2;
            spOn = settings.water1on;
            spOff = settings.water1off;
            permit = settings.modeRelay2 & 3;
            break;
        case 3:
            stateBit = RELAY3;
            val = pvTimeR3;
            spOn = settings.water2on;
            spOff = settings.water2off;
            permit = settings.modeRelay3 & 3;
            break;
    }

    // Step 2: Check conditional permissions based on light status
    if (cn < 3 && permit == 0) {
        if (cn == 1) {
            int16_t onT, offT;
            uint8_t opMode;
            if (settings.modeHeater == 0) { // Heat -> Emer Cool
                opMode = 1; // Cool
                onT = settings.spT0off + settings.hysteresis0;
                offT = settings.spT0off;
            } else { // Cool -> Emer Heat
                opMode = 0; // Heat
                onT = settings.spT0off - settings.hysteresis0;
                offT = settings.spT0off;
            }
            if(!ERROR1 && !ERROR10 && !DHT_ERR){
                RELAY1 = checkDeviceState(RELAY1, ds[0].pvT, onT, offT, opMode);
                if(RELAY1 == PCF_ON) EXTRA1 = 1; else EXTRA1 = 0;
            }
            else {RELAY1 = PCF_OFF; EXTRA1 = 0;}
        } else if (cn == 2 && (numberOfDS18 > 1 || hasDHT22)) {
            int16_t onH, offH;
            uint8_t opMode;
            if (settings.modeHumidi == 0) { // Humidify -> Emer Dehumidify
                opMode = 1; // Dehumidify
                onH = settings.spT1off + settings.hysteresis1;
                offH = settings.spT1off;
            } else { // Dehumidify -> Emer Humidify
                opMode = 0; // Humidify
                onH = settings.spT1off - settings.hysteresis1;
                offH = settings.spT1off;
            }
            if(!ERROR2 && !ERROR20 && !DHT_ERR){
                RELAY2 = checkDeviceState(RELAY2, ds[1].pvT, onH, offH, opMode);
                if(RELAY2 == PCF_ON) EXTRA2 = 1; else EXTRA2 = 0;
            }
            else {RELAY2 = PCF_OFF; EXTRA2 = 0;}
        }
        return;
    }

    // Operating mode bits: 0 = Always, 1 = Only when LIGHT is OFF, 2 = Only when LIGHT is ON
    if (permit > 0) permit--;

    if (permit) {
        if (LIGHT == PCF_OFF && permit == 2) permit = 0;
        else if (LIGHT == PCF_ON && permit == 1) permit = 0;
    }

    // Step 3: Logic execution
    if (permit == 0) { // If operation is permitted for the current lighting state
        if (cn == 3 && settings.program > 0) {
            if (stateBit == PCF_ON) {
                if (--val <= 0) {
                    RELAY3 = PCF_OFF;
                    pvTimeR3 = -1;
                } else {
                    pvTimeR3 = val;
                }
            } else {
                RELAY3 = PCF_OFF;
                pvTimeR3 = -1;
            }
            return;
        }

        if (spOn == 0) {
            switch (cn) {
                case 1: RELAY1 = PCF_OFF; pvTimeR1 = -1; break;
                case 2: RELAY2 = PCF_OFF; pvTimeR2 = -1; break;
                case 3: RELAY3 = PCF_OFF; pvTimeR3 = -1; break;
            }
            return;
        }
        // Convert the OFF interval index from settings to actual minutes/hours/days
        spOff = transformTimeOff(spOff);
        
        // Decrement the timer. If it reaches zero, toggle the relay state
        if (--val <= 0) {
            prnBit = true;
            if (stateBit == PCF_OFF) { // Current state is OFF -> Switch to ON phase
                val = spOn;            // Set timer to "ON duration" from settings
                stateBit = PCF_ON;     // Physical state becomes ON (0)
                MYDEBUG_PRINT("spOn="); MYDEBUG_PRINT(spOn);
                MYDEBUG_PRINT("; Relay:"); MYDEBUG_PRINT(cn); MYDEBUG_PRINTLN(" state = ON");
            } else { // Current state is ON -> Switch to OFF phase
                val = spOff;           // Set timer to calculated "OFF interval"
                stateBit = PCF_OFF;    // Physical state becomes OFF (1)
                MYDEBUG_PRINT("spOff="); MYDEBUG_PRINT(spOff);
                MYDEBUG_PRINT("; Relay:"); MYDEBUG_PRINT(cn); MYDEBUG_PRINTLN(" state = OFF");
            }
        }
        
        // Step 4: Save updated state and timer back to global system variables
        switch (cn) {
            case 1: RELAY1 = stateBit; pvTimeR1 = val; break;
            case 2: RELAY2 = stateBit; pvTimeR2 = val; break;
            case 3: RELAY3 = stateBit; pvTimeR3 = val; break;
        }
        #ifdef DEBUG
        if (prnBit) printBinary(portOut.value);
        #endif
    } else { 
        // If operation is NOT permitted by lighting mode, force the relay OFF and reset timer
        switch (cn) {
            case 1: RELAY1 = PCF_OFF; pvTimeR1 = -1; break;
            case 2: RELAY2 = PCF_OFF; pvTimeR2 = -1; break;
            case 3: RELAY3 = PCF_OFF; pvTimeR3 = -1; break;
        }
    }
}

bool LogicManager::checkDeviceState(bool previousState, int16_t currentTemp, int16_t onTemp, int16_t offTemp, uint8_t mode) {
    if (onTemp == offTemp) return PCF_OFF;

    if (mode == 0) { // Heating mode / Humidifying mode
        if (currentTemp <= onTemp) return PCF_ON;
        if (currentTemp >= offTemp) return PCF_OFF;
    } else { // Cooling mode / Dehumidifying mode
        if (currentTemp >= onTemp) return PCF_ON;
        if (currentTemp <= offTemp) return PCF_OFF;
    }
    return previousState;
}

bool LogicManager::checkLightState(uint8_t currentHour, uint8_t onHour, uint8_t offHour) {
    if (onHour == offHour) return false;
    if (onHour < offHour) return (currentHour >= onHour && currentHour < offHour);
    else return (currentHour >= onHour || currentHour < offHour);
}

void LogicManager::processAlarm(uint8_t cn) {
    int16_t val, maxVal, minVal, alarmVal, mode;
    bool reached, beep = false;
    val = ds[cn].pvT;
    
    if (cn) {
        maxVal = max(settings.spT1on, settings.spT1off);
        minVal = min(settings.spT1on, settings.spT1off);
        alarmVal = settings.alarm1;
        mode = settings.modeHumidi;
        reached = REACHED1;
    } else {
        maxVal = max(settings.spT0on, settings.spT0off);
        minVal = min(settings.spT0on, settings.spT0off);
        alarmVal = settings.alarm0;
        mode = settings.modeHeater;
        reached = REACHED0;
    }

    if (reached) {
        if (val <= (minVal - alarmVal) || val >= (maxVal + alarmVal)) beep = true;
    } else if (mode == 0) {
        if (val >= minVal) reached = true;
    } else if (mode == 1) {
        if (val <= maxVal) reached = true;
    }

    if (cn == 0){   // только для температуры
        // uint8_t oldFlap = pvFlap;
        if (val > maxVal) pvFlap = settings.maxFlap;
        else if (val < minVal) pvFlap = settings.minFlap;
        else pvFlap = settings.curFlap;

        // if (pvFlap != oldFlap) {
        //     String logMsg;
        //     #if defined(LANG_RU)
        //     logMsg = "Заслонка: " + String(oldFlap) + "% -> " + String(pvFlap) + "%";
        //     #elif defined(LANG_UA)
        //     logMsg = "Заслінка: " + String(oldFlap) + "% -> " + String(pvFlap) + "%";
        //     #else
        //     logMsg = "Flap: " + String(oldFlap) + "% -> " + String(pvFlap) + "%";
        //     #endif
        //     sysLogger.log(logMsg);
        // }
    }
    
    if (cn) {
        if (!REACHED1 && reached) sysLogger.log(getMsg(MSG_CLIMATE_T2_REACHED));
        REACHED1 = reached;
        if (!ERROR8 && beep) sysLogger.log(getMsg(MSG_ALARM_T2_RANGE));
        if (ERROR8 && !beep) sysLogger.log(getMsg(MSG_CLIMATE_T2_REACHED));
        ERROR8 = beep;
    } else {
        if (!REACHED0 && reached) sysLogger.log(getMsg(MSG_CLIMATE_T1_REACHED));
        REACHED0 = reached;
        if (!ERROR4 && beep) sysLogger.log(getMsg(MSG_ALARM_T1_RANGE));
        if (ERROR4 && !beep) sysLogger.log(getMsg(MSG_CLIMATE_T1_REACHED));
        ERROR4 = beep;
    }

    if (errorsFlag.value) {
        uint8_t duration = (errorsFlag.value == 0x03) ? 100 : 50;
        if (disableBeep == 0) beeperOn(duration);
        else disableBeep--;
    } else {
        disableBeep = 0;
    }
}

uint16_t LogicManager::transformTimeOff(uint8_t point) {
    uint16_t val = point;
    switch (point) {
        case 5: val = 6; break;
        case 6: val = 8; break;
        case 7: val = 10; break;
        case 8: val = 12; break;
        case 9: val = 24; break;
        case 10: val = 2; break;
        case 11: val = 3; break;
        case 12: val = 4; break;
        case 13: val = 5; break;
        case 14: val = 6; break;
        case 15: val = 7; break;
    }
    if (point < 10) val *= 60;
    else val *= (60 * 24);
    return val;
}
