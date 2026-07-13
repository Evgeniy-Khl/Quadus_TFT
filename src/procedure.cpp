#include "main.h"
#include "procedure.h"

#define UNALTERED   2 // неизменный

void PID_Init(PIDController *pid, uint16_t Kp, uint16_t Ki) {
    pid->Kp = (float)Kp/10;
    pid->Ki = (float)Ki/1000;
}

uint8_t UpdatePID(uint8_t cn){
 int16_t error;
 float output;
  // Вычисление ошибки
  error = settings.sp_structs[cn].spT - ds[cn].pvT;
  ds[cn].pvErr = error;         // error > 0 -> холодно
  // Пропорциональная составляющая
  pid[cn].pPart = (float)error * pid[cn].Kp;
  // Интегральная составляющая
  pid[cn].iPart += (float)error * pid[cn].Ki;// * dt;
  // Суммарное управляющее воздействие
  output = pid[cn].pPart + pid[cn].iPart;
  // Ограничение выходного значения и антивиндовинг
  if (output > 100) output = 110;
  else if (output < 0) output = 0;
  if (pid[cn].pPart >= 100) pid[cn].iPart = 0; // Сброс интеграла
  else if (pid[cn].pPart <= -50) pid[cn].iPart = 0; // Сброс интеграла

  error = output;
  return (uint8_t)error;
}
//------------- симистричный таймер -------------------
void rotate_trays(void){ 
  if(TURN){
    if(--pvTimer == 0){pvTimer = settings.sp_structs[0].timer; TURN = OFF;}
  } else {
    if(--pvTimer == 0){
        if(settings.sp_structs[1].timer){pvTimer = settings.sp_structs[1].timer; TURN = ON;}
        else {pvTimer = settings.sp_structs[0].timer; TURN = ON;}
    }
  }
}

//------------- индикация 66,0 - завис датчик. --------------
bool check_freeze(uint8_t i){
 if(ds[i].pvT == ds[i].previousValue){
    if(++ds[i].duration > 600){ds[i].duration = 600; return true;}
 } else {ds[i].duration = 0; ds[i].previousValue = ds[i].pvT;}
 return false;
}

int16_t checkPV(uint8_t cn){
  int16_t err;
  if(cn==1 && HIH5030){
     if(pvVadcRH < 80) {errors.value |= (cn+1); err = 0;}
     else err = settings.sp_structs[1].spRH - pvRH;
     ds[1].pvErr = err;         // err > 0 -> холодно
  } else {
     if(ds[cn].pvT >= 850) {errors.value |= (cn+1); err = 0;}
     else err = settings.sp_structs[cn].spT - ds[cn].pvT;
     ds[cn].pvErr = err;        // err > 0 -> холодно
  };
  return err;
}

uint8_t RelayPos(unsigned char cn, unsigned char hysteresis){	// [n] канал № 1 или 2
  uint8_t x=UNALTERED;
  int16_t err = checkPV(cn);        // err > 0 -> холодно
  if(err >= hysteresis) x = ON;    // включить
  if(err <= 0) x = OFF;            // отключить
  return x;
}

uint8_t RelayNeg(uint8_t cn, uint8_t on, uint8_t off){	// [n] канал № 1 или 2
  uint8_t x=UNALTERED;
  int16_t err = checkPV(cn);        // err > 0 -> холодно
  if ((err+on) <= 0) x = ON;        // включить
  if ((err+off) >= 0) x = OFF;      // отключить
  return x;
}

void OutPulse(void){
  int16_t err = checkPV(1);                     // err > 0 -> холодно
  if(err == 0){pvPulse = 0; return;};
  if(ds[0].pvErr >= settings.sp_structs[0].alarm){pvPulse = 0; return;};          // отключение впрыска по 2 каналу если идет разогрев
  pvPulse = UpdatePID(1);                       // определение длительности ВКЛ. состояния
  if(pvPulse < settings.sp_structs[0].pulse) pvPulse = settings.sp_structs[0].pulse;
  else if(pvPulse > settings.sp_structs[1].pulse) pvPulse = settings.sp_structs[1].pulse;      // длит. впрыска не должна превыщать длит.переода
  if(ds[1].pvErr < 0) pvPulse = 0;                  // отключение впрыска по 2 каналу если перелив
}

uint8_t tableRH(int16_t maxT, int16_t minT){
  int16_t dT;
   if (maxT>199 && maxT<410){ // maxT> 19.9 и maxT< 41.0
     dT = (maxT-minT)*16/10;    //?????????????????????????????????????
     if (dT<0) dT = 240;        // задаем число при котором dT >>=3; выполняется -> dT>20
     maxT /=10;
     dT >>=3;
     if (dT>20) dT = 255;
     else if (dT==0) dT = 100;
     else {maxT -= 20; maxT *= 20; maxT += (dT-1); dT = tabRH[maxT];};
   } else dT = 255;
   return dT;
 }

//-------- Функция для печати текущих значений структуры в Serial порт --------
#ifdef DEBUG
void printConfig() {
    DEBUG_PRINTLN("--------------------");
    for (int i = 0; i < 2; i++) {
        Serial.printf("Элемент settings.sp_structs[%d]:\n", i);
        Serial.printf("  spT: %d\n", settings.sp_structs[i].spT);
        Serial.printf("  spRH: %d\n", settings.sp_structs[i].spRH);
        Serial.printf("  alarm: %d\n", settings.sp_structs[i].alarm);
        Serial.printf("  coolOn: %d\n", settings.sp_structs[i].coolOn);
        Serial.printf("  coolOff: %d\n", settings.sp_structs[i].coolOff);
        Serial.printf("  timer: %d\n", settings.sp_structs[i].timer);
        Serial.printf("  aeration: %d\n", settings.sp_structs[i].aeration);
        Serial.printf("  auxiliary: %d\n", settings.sp_structs[i].auxiliary);
        Serial.printf("  flapLimit: %d\n", settings.sp_structs[i].flapLimit);
        Serial.printf("  state: %d\n", settings.sp_structs[i].state);
        Serial.printf("  pulse: %d\n", settings.sp_structs[i].pulse);
        Serial.printf("  mode: %d\n", settings.sp_structs[i].mode);
        Serial.printf("  extendMode: %d\n", settings.sp_structs[i].extendMode);
        Serial.printf("  Kp: %d\n", settings.sp_structs[i].Kp);
        Serial.printf("  Ki: %d\n", settings.sp_structs[i].Ki);
    }
    DEBUG_PRINTLN("--------------------");
}
#endif

//----------- Функция сохранения конфигурации в JSON файл ----------------
void saveConfig() {
    DEBUG_PRINTLN("Сохранение конфигурации...");

    // Создаем JSON документ. Размер 512 байт более чем достаточен.
    StaticJsonDocument<1024> doc;

    // Создаем корневой JSON массив
    JsonArray jsonArray = doc.to<JsonArray>();

    // Проходим по массиву структур и добавляем данные в JSON
    for (int i = 0; i < 2; i++) {
        JsonObject obj = jsonArray.createNestedObject();
        obj["spT"] = settings.sp_structs[i].spT;
        obj["spRH"] = settings.sp_structs[i].spRH;
        obj["alarm"] = settings.sp_structs[i].alarm;
        obj["coolOn"] = settings.sp_structs[i].coolOn;
        obj["coolOff"] = settings.sp_structs[i].coolOff;
        obj["timer"] = settings.sp_structs[i].timer;
        obj["aeration"] = settings.sp_structs[i].aeration;
        obj["auxiliary"] = settings.sp_structs[i].auxiliary;
        obj["flapLimit"] = settings.sp_structs[i].flapLimit;
        obj["state"] = settings.sp_structs[i].state;
        obj["pulse"] = settings.sp_structs[i].pulse;
        obj["mode"] = settings.sp_structs[i].mode;
        obj["extendMode"] = settings.sp_structs[i].extendMode;
        obj["Kp"] = settings.sp_structs[i].Kp;
        obj["Ki"] = settings.sp_structs[i].Ki;
    }

    // Открываем файл для записи
    File configFile = LittleFS.open("/setpoint.json", "w");
    if (!configFile) {
        DEBUG_PRINTLN("Не удалось открыть файл для записи");
        return;
    }

    // Сериализуем JSON в файл
    if (serializeJson(doc, configFile) == 0) {
        DEBUG_PRINTLN("Ошибка записи в файл");
    } else {
        DEBUG_PRINTLN("Конфигурация успешно сохранена.");
    }
    
    configFile.close();
}

//------------ Функция загрузки конфигурации из JSON файла -------------
bool loadConfig() {
    DEBUG_PRINTLN("Загрузка конфигурации...");

    // Открываем файл для чтения
    File configFile = LittleFS.open("/setpoint.json", "r");
    if (!configFile) {
        DEBUG_PRINTLN("Не удалось открыть файл для чтения. Используются значения по умолчанию.");
        return false;
    }

    // Создаем JSON документ для десериализации
    StaticJsonDocument<1024> doc;

    // Десериализуем JSON из файла
    DeserializationError error = deserializeJson(doc, configFile);
    if (error) {
        DEBUG_PRINT("Ошибка десериализации JSON: ");
        DEBUG_PRINTLN(error.c_str());
        configFile.close();
        return false;
    }
// Закрываем файл после чтения
    configFile.close();

    // Получаем корневой JSON массив
    JsonArray jsonArray = doc.as<JsonArray>();

// spT spRH timer alarm coolOn coolOff aeration flapLimit state service pulse mode extendMode Kp Ki Kd
    // Проходим по JSON массиву и заполняем структуру
    int i = 0;
    for (JsonObject obj : jsonArray) {
        if (i < 2) {
            settings.sp_structs[i].spT = obj["spT"];
            settings.sp_structs[i].spRH = obj["spRH"];
            settings.sp_structs[i].alarm = obj["alarm"];
            settings.sp_structs[i].coolOn = obj["coolOn"];
            settings.sp_structs[i].coolOff = obj["coolOff"];
            settings.sp_structs[i].timer = obj["timer"];
            settings.sp_structs[i].aeration = obj["aeration"];
            settings.sp_structs[i].auxiliary = obj["auxiliary"];
            settings.sp_structs[i].flapLimit = obj["flapLimit"];
            settings.sp_structs[i].state = obj["state"];
            settings.sp_structs[i].pulse = obj["pulse"];
            settings.sp_structs[i].mode = obj["mode"];
            settings.sp_structs[i].extendMode = obj["extendMode"];
            settings.sp_structs[i].Kp = obj["Kp"];
            settings.sp_structs[i].Ki = obj["Ki"];
            i++;
        }
    }
    DEBUG_PRINTLN("Конфигурация успешно загружена.");
    return true;
}

// // Вспомогательная функция для печати
// void printBinary(unsigned char byte) {
//   for (int i = 7; i >= 0; i--) {
//     DEBUG_PRINTLN(bitRead(byte, i));
//   }
// }