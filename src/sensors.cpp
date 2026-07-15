#include "main.h"

#define TUNING	170

/**
 * @brief Detect all connected sensors (DS18B20 OR DHT22).
 * Priority: DS18B20 first. If found, DHT22 is ignored.
 */
void sensorType(){
  MYDEBUG_PRINTLN("Detecting sensors...");
  
  // 1. Scan for DS18B20 sensors
  sensors.begin(); // Initialize 1-Wire bus
  numberOfDS18 = sensors.getDS18Count();
  if(numberOfDS18 > 0) {
      if(numberOfDS18 > MAX_DEVICE) numberOfDS18 = MAX_DEVICE;
      MYDEBUG_PRINT("DS18B20 detected: "); MYDEBUG_PRINT(numberOfDS18, DEC); MYDEBUG_PRINTLN(" pcs.");
      sensors.setWaitForConversion(false);
      sensors.setCheckForConversion(false);
      sensors.setAutoSaveScratchPad(false);
      sensors.setResolution(12);
      sensors.requestTemperatures();
      
      for (int i = 0; i < numberOfDS18; i++){
        if(sensors.getAddress(sensorAddresses[i], i)){
          DEBUG_PRINTF("  Sensor %d: ", i);
          printAddress(sensorAddresses[i]);
          MYDEBUG_PRINTLN();
        }
      }
      hasDHT22 = false; // Disable DHT22 if DS18B20 found
   } else {
      MYDEBUG_PRINTLN("No DS18B20 sensors found. Trying DHT22...");
      // 2. Try to detect DHT22 only if NO DS18B20 present
      delay(1000); 
      dht.begin();
      float testT = dht.readTemperature();
      if (!isnan(testT)) {
        hasDHT22 = true;
        MYDEBUG_PRINTLN("DHT22 sensor detected.");
      } else {
        hasDHT22 = false;
        MYDEBUG_PRINTLN("DHT22 not found.");
      }
   }
}

/**
 * @brief Main sensor update routine. Handles either DHT22 or DS18B20.
 */
void sensorCheck(){
  // Read DHT22 if present (implies no DS18B20)
  if (hasDHT22) {
    float h = dht.readHumidity();
    float t = dht.readTemperature();

    if (isnan(h) || isnan(t)) {
      MYDEBUG_PRINTLN("DHT22 read error!");
      if(++ds[0].errDevice > 5){ 
        ds[0].errDevice = 5;
        if (!DHT_ERR) {sysLogger.log(getMsg(MSG_DHT_ERR)); DHT_ERR = 1;}
      }
    } else {
      ds[0].errDevice = 0;
      ds[0].pvT = round(t * 10.0);
      ds[1].pvT = round(h * 10.0);
      if (DHT_ERR) {sysLogger.log(getMsg(MSG_DHT_OK)); DHT_ERR = 0;}
      MYDEBUG_PRINT("DHT Air t="); MYDEBUG_PRINT(t); MYDEBUG_PRINT(" RH="); MYDEBUG_PRINTLN(h);
    }
  }
  // Read DS18B20 sensors if present
  else if (numberOfDS18 > 0) {
    checkDs18b20();
  }
  // If NO sensors are found at all
  else {
    MYDEBUG_PRINTLN("ALARM: No sensors connected!");
  }
}

/**
 * @brief Check if sensor data has frozen (not changing).
 */
bool check_freeze(uint8_t i, float val){
 if(val == ds[i].previousValue){
    if(++ds[i].froze > 600){
      ds[i].froze = 600; 
      return true;
    }
 } else {ds[i].froze = 0; ds[i].previousValue = val;}
 return false;
}

/**
 * @brief Process DS18B20 sensor data. 
 */
void checkDs18b20(void){
  for (uint8_t i = 0; i < numberOfDS18; i++){
    if (i >= MAX_DEVICE) break;

    float tempC = sensors.getTempC(sensorAddresses[i]);
    
    if(tempC == DEVICE_DISCONNECTED_C) {
      if(++ds[i].errDevice > 5){
        ds[i].errDevice = 5;
        switch (i){
        case 0: if (!ERROR1) sysLogger.log(getMsg(MSG_HEATER_ERR)); ERROR1 = 1; break;
        case 1: if (!ERROR2) sysLogger.log(getMsg(MSG_HUMIDITY_ERR)); ERROR2 = 1; break;
        }
      }
    }
    else {
      ds[i].pvT = round(tempC * 10.0);
      ds[i].errDevice = 0;
      switch (i){
      case 0: if (ERROR1) sysLogger.log(getMsg(MSG_CLIMATE_T1_REACHED)); ERROR1 = 0; break;
      case 1: if (ERROR2) sysLogger.log(getMsg(MSG_CLIMATE_T2_REACHED)); ERROR2 = 0; break;
      }
    }

    if(check_freeze(i, tempC)){
      switch (i){
      case 0: if (!ERROR10) sysLogger.log(getMsg(MSG_CLIMATE_T1_FROZE)); ERROR10 = 1; break;
      case 1: if (!ERROR20) sysLogger.log(getMsg(MSG_CLIMATE_T2_FROZE)); ERROR20 = 1; break;
      }
    }
    else {
      switch (i){
      case 0: if (ERROR10) sysLogger.log(getMsg(MSG_CLIMATE_T1_OK)); ERROR10 = 0; break;
      case 1: if (ERROR20) sysLogger.log(getMsg(MSG_CLIMATE_T2_OK)); ERROR20 = 0; break;
      }
    }
  }
  sensors.requestTemperatures(); 
}
