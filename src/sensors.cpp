#include "main.h"
#include "sensors.h"
#include "procedure.h"

#define TUNING	170

void temperature_check(void){
  char buff[100];
  DeviceAddress sensorAddress;        // Переменная для хранения адреса датчика
  for (uint8_t i = 0; i < numberOfDevices; i++){
    float tempC = sensors.getTempCByIndex(i);
    sprintf(buff, "TempCByIndex(%i): %5.1f °C",i,tempC);
    Serial.println(buff);
    if(tempC == DEVICE_DISCONNECTED_C) {
      ds[i].errDevice++;
      if(ds[i].errDevice > 5) {ds[i].pvT = 1990; ds[i].errDevice = 5;}
    }
    else {
      ds[i].pvT = tempC * 10;
      ds[i].errDevice = 0;
    }
    //----- Коректировка датчика DS18B20 ---------
    sensors.getAddress(sensorAddress, i);
    uint8_t alarmH = sensors.getHighAlarmTemp(sensorAddress);
    sprintf(buff, "HighAlarmTemp(%i): %3i",i,alarmH);
    Serial.println(buff);
    if(alarmH==TUNING){
      uint8_t alarmL = sensors.getLowAlarmTemp(sensorAddress);
      ds[i].pvT += alarmL;
    }
    if(check_freeze(i)){
      ds[i].pvT = 660;    // индикация 66,0 - завис датчик.
      ERROR8 = 1;
    }
  }
  sensors.requestTemperatures();
}

#define V_REF   	5 //?????????????????????????????????????????????????????
#define ADC_RESOLUTION	512 //?????????????????????????????????????????????????????????????????????
// для HIH-5030
// Vadc бинарное значение ADC -> в десятичное значение относительной влажности (%)
int16_t valDcToRH(uint16_t Vadc){
 float tmpRH, tmpK;
  tmpRH = (float)Vadc/ADC_RESOLUTION; //????????????????????????????????????????????????????????????????
  tmpRH -= 0.1515; tmpRH /= 0.00636;
  if(ds[0].pvT<850){tmpK = 0.00216 * ds[0].pvT/10; tmpK = 1.0546 - tmpK;}// корекция по температуре
  else tmpK=1;      
  tmpRH /= tmpK;
  tmpRH *= 10;
  tmpRH += settings.sp_structs[0].spRH;             //sp[0].spRH->ПОДСТРОЙКА HIH-5030!!
  if (tmpRH>1000) tmpRH=1000; else if (tmpRH<0) tmpRH=0;
  return tmpRH;
}

/* int16_t lowPassF2(int16_t PV)
{
float val;
  // val = A1*PVold1-A2*PVold2+A3*PV;
  // PVold2 = PVold1;
  // PVold1 = val;
  return val;
}; */