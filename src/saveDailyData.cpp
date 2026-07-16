#include "saveDailyData.h"

/**
 * @brief Читает 288 пятиминутных записей из EEPROM и сохраняет их в файлы.
 * @param day Номер дня для имени файла.
 * @param month Номер месяца для имени файла.
 */
void saveDailyDataToFile(int day, int month) {
  checkAndManageSpace(); // Проверка и освобождение места

  char buf[32];
  snprintf(buf, sizeof(buf), "%02d_%02d", day, month);
  String dateStr = String(buf);

  DEBUG_PRINTF("Начало сохранения данных за день #%s\n", dateStr.c_str());
  
  String graphFilename = "/day_" + dateStr + "_graph.json";
  File graphFile = LittleFS.open(graphFilename, "w");
  if (!graphFile) {
    MYDEBUG_PRINTLN("Ошибка открытия файла для графика!");
    return;
  }

  // Начинаем JSON массив напрямую в файл
  graphFile.print("[");

  float total_sum_t1 = 0, min_t1 = 200, max_t1 = -200;
  float total_sum_t2 = 0, min_t2 = 200, max_t2 = -200;
  float total_sum_rh = 0, min_rh = 200, max_rh = -200;
  int validReadingsCount = 0;

  // --- Читаем 288 записей и сразу пишем в файл ---
  for (int period = 0; period < DAILY_DATA_MAX_REC; ++period) {
    int currentAddress = DAILY_DATA_START + period * DAILY_DATA_REC_SIZE;
    int16_t raw_t1 = eepromReadInt16(currentAddress);
    int16_t raw_t2 = eepromReadInt16(currentAddress + 2);
    int16_t raw_rh = eepromReadInt16(currentAddress + 4);

    if (raw_t1 == 0 && raw_t2 == 0) continue;

    float t1 = (float)raw_t1 / 10.0;
    float t2 = (float)raw_t2 / 10.0;
    float rh = (float)raw_rh / 10.0;

    if (validReadingsCount > 0) graphFile.print(",");
    
    // Пишем одну точку. printf экономит RAM по сравнению с JsonDocument
    graphFile.printf("{\"p\":%d,\"t1\":%.1f,\"t2\":%.1f,\"rh\":%.1f}", period, t1, t2, rh);

    // Обновляем статистику
    total_sum_t1 += t1; total_sum_t2 += t2; total_sum_rh += rh;
    if (t1 < min_t1) min_t1 = t1; 
    if (t1 > max_t1) max_t1 = t1;
    if (t2 < min_t2) min_t2 = t2; 
    if (t2 > max_t2) max_t2 = t2;
    if (rh < min_rh) min_rh = rh; 
    if (rh > max_rh) max_rh = rh;
    validReadingsCount++;
  }

  graphFile.print("]"); // Закрываем массив
  graphFile.close();
  
  DEBUG_PRINTF("Данные сохранены в %s. Точек: %d\n", graphFilename.c_str(), validReadingsCount);

  if (validReadingsCount > 0) {
    JsonDocument statsDoc;
    statsDoc["avg_t1"] = total_sum_t1 / validReadingsCount;
    statsDoc["min_t1"] = min_t1;
    statsDoc["max_t1"] = max_t1;
    statsDoc["avg_t2"] = total_sum_t2 / validReadingsCount;
    statsDoc["min_t2"] = min_t2;
    statsDoc["max_t2"] = max_t2;
    statsDoc["avg_rh"] = total_sum_rh / validReadingsCount;
    statsDoc["min_rh"] = min_rh;
    statsDoc["max_rh"] = max_rh;

    String statsFilename = "/day_" + dateStr + "_stats.json";
    File statsFile = LittleFS.open(statsFilename, "w");
    if (statsFile) {
      serializeJson(statsDoc, statsFile);
      statsFile.close();
      DEBUG_PRINTF("  Статистика сохранена в %s\n", statsFilename.c_str());
    }
  }
}

/**
 * @brief Очищает область памяти в AT24C32, используемую для хранения суточных данных.
 */
void clearEEPROM() {
  MYDEBUG_PRINTLN("Начало очистки AT24C32 для графиков...");
  for (int period = 0; period < DAILY_DATA_MAX_REC; ++period) {
    int currentAddress = DAILY_DATA_START + period * DAILY_DATA_REC_SIZE;
    eepromWriteInt16(currentAddress, 0);
    eepromWriteInt16(currentAddress + 2, 0);
    eepromWriteInt16(currentAddress + 4, 0);
  }
  sysLogger.log(getMsg(MSG_DAILY_CLEARED));
  MYDEBUG_PRINTLN("Очистка AT24C32 завершена.");
}

/**
 * @brief Удаляет файлы графиков, статистики и логов для конкретного дня.
 */
void deleteFilesForDay(int day, int month) {
  char buf[32];
  snprintf(buf, sizeof(buf), "%02d_%02d", day, month);
  String dateStr = String(buf);

  DEBUG_PRINTF("Удаление файлов для дня #%s...\n", dateStr.c_str());
  String graphFilename = "/day_" + dateStr + "_graph.json";
  String statsFilename = "/day_" + dateStr + "_stats.json";
  String logFilename = "/day_" + dateStr + "_log.txt";

  if (LittleFS.remove(graphFilename)) {
    DEBUG_PRINTF("  - Файл удален: %s\n", graphFilename.c_str());
  }
  if (LittleFS.remove(statsFilename)) {
    DEBUG_PRINTF("  - Файл удален: %s\n", statsFilename.c_str());
  }
  if (LittleFS.remove(logFilename)) {
    DEBUG_PRINTF("  - Файл удален: %s\n", logFilename.c_str());
  }
}

/**
 * @brief Находит хронологически самый старый день в LittleFS, учитывая текущую дату.
 */
ArchiveDay findOldestDay() {
  Dir dir = LittleFS.openDir("/");
  int maxDistance = -1;
  ArchiveDay oldest = {-1, -1};

  int currentDay = 15;
  int currentMonth = 6;
  if (timeinfo && timeinfo->tm_year >= 100) {
      currentDay = timeinfo->tm_mday;
      currentMonth = timeinfo->tm_mon + 1;
  }

  while (dir.next()) {
    String fileName = dir.fileName();
    if (fileName.startsWith("day_") && fileName.endsWith("_stats.json")) {
      // Имя файла: day_DD_MM_stats.json (длина "day_" = 4)
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
          if (distance > maxDistance) {
              maxDistance = distance;
              oldest.day = day;
              oldest.month = month;
          }
      }
    }
  }
  return oldest;
}

/**
 * @brief Проверяет свободное место и удаляет старые файлы при необходимости.
 */
void checkAndManageSpace() {
  const long REQUIRED_SPACE = 25000; // ~25 КБ

  FSInfo fs_info;
  if (LittleFS.info(fs_info)) {
    long freeSpace = fs_info.totalBytes - fs_info.usedBytes;
    // DEBUG_PRINTF("Проверка свободного места: доступно %ld байт. Нужно: %ld байт.\n", freeSpace, REQUIRED_SPACE);

    while (freeSpace < REQUIRED_SPACE) {
      MYDEBUG_PRINTLN("Недостаточно места в FS! Удаление старых файлов графиков...");
      ArchiveDay oldestDay = findOldestDay();
      if (oldestDay.day == -1) {
        MYDEBUG_PRINTLN("Нет файлов для удаления!");
        break;
      }
      deleteFilesForDay(oldestDay.day, oldestDay.month);
      
      LittleFS.info(fs_info);
      freeSpace = fs_info.totalBytes - fs_info.usedBytes;
      DEBUG_PRINTF("Доступно после удаления: %ld байт\n", freeSpace);
    }
  }
}

/**
 * @brief Выводит в Serial Monitor список всех файлов и их размеры.
 * Также показывает общую информацию о занятом и свободном месте.
 */
#ifdef DEBUG
void listFilesAndSizes() {
  MYDEBUG_PRINTLN("\n--- Список файлов в LittleFS ---");
  
  Dir dir = LittleFS.openDir("/");
  long totalSize = 0;
  int fileCount = 0;

  while (dir.next()) {
    // Для каждого элемента получаем объект File
    File entry = dir.openFile("r");
    if (entry) {
      MYDEBUG_PRINT("Файл: ");
      MYDEBUG_PRINT(entry.name());
      MYDEBUG_PRINT("\tРазмер: ");
      MYDEBUG_PRINT(entry.size());
      MYDEBUG_PRINTLN(" Байт");
      totalSize += entry.size();
      fileCount++;
      entry.close(); // Важно закрывать файл после использования
    }
  }

  MYDEBUG_PRINTLN("------------------------------------");
  DEBUG_PRINTF("Всего файлов: %d\n", fileCount);
  DEBUG_PRINTF("Общий размер: %ld Байт\n", totalSize);

  // Дополнительная информация о файловой системе
  FSInfo fs_info;
  LittleFS.info(fs_info);
  DEBUG_PRINTF("Всего места:  %d Байт\n", fs_info.totalBytes);
  DEBUG_PRINTF("Использовано: %d Байт\n", fs_info.usedBytes);
  MYDEBUG_PRINTLN("------------------------------------");
}
#endif
