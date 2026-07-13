#include "main.h"
#include <AT24C32.h>

// ----- Функции для работы с EEPROM -----

/**
 * @brief Записывает один байт в EEPROM.
 * @param memoryAddress 16-битный адрес ячейки памяти (0-4095).
 * @param data Байт для записи.
 */
void eepromWriteByte(uint16_t memoryAddress, uint8_t data) {
  Wire.beginTransmission(EEPROM_I2C_ADDRESS);
  Wire.write((uint8_t)(memoryAddress >> 8));
  Wire.write((uint8_t)(memoryAddress & 0xFF));
  Wire.write(data);
  uint8_t status = Wire.endTransmission();
  if (status != 0) {
    DEBUG_PRINT("I2C Write Error for byte at addr "); DEBUG_PRINT(memoryAddress);
    DEBUG_PRINT(". Status: "); DEBUG_PRINTLN(status);
    // Коды ошибок: 0:success, 1:data too long, 2:NACK on address, 3:NACK on data, 4:other
  }
  delay(EEPROM_WRITE_DELAY);
}

/**
 * @brief Читает один байт из EEPROM.
 * @param memoryAddress 16-битный адрес ячейки памяти (0-4095).
 * @return Прочитанный байт или 0 если произошла ошибка.
 */
uint8_t eepromReadByte(uint16_t memoryAddress) {
  uint8_t readData = 0;
  Wire.beginTransmission(EEPROM_I2C_ADDRESS);
  Wire.write((uint8_t)(memoryAddress >> 8));   // Старший байт адреса
  Wire.write((uint8_t)(memoryAddress & 0xFF)); // Младший байт адреса
  Wire.endTransmission(); // Отправка адреса без STOP для "dummy write"

  Wire.requestFrom(EEPROM_I2C_ADDRESS, 1); // Запросить 1 байт
  if (Wire.available()) {
    readData = Wire.read();
  }
  return readData;
}

/**
 * @brief Записывает массив байт (буфер) в EEPROM.
 * @param memoryAddress Начальный 16-битный адрес ячейки памяти.
 * @param buffer Указатель на массив байт для записи.
 * @param length Количество байт для записи.
 * Примечание: Эта функция записывает байты последовательно.
 * AT24C32 имеет страницы по 32 байта. Для оптимальной записи больших блоков
 * следует учитывать границы страниц, эта функция делает явно.
 * Однако, последовательная запись через Wire.write() для AT24C32 обычно работает корректно
 * в пределах страницы. Для записи через границы страниц может потребоваться несколько транзакций.
 * AT24C32 адреса страниц: 0-31, 32-63, ...
 */
void eepromWriteBuffer(uint16_t memoryAddress, const uint8_t* buffer, uint16_t length) {
  uint16_t currentBufferIndex = 0;
  while (currentBufferIndex < length) {
    // Рассчитываем, сколько байт можно записать до конца текущей физической страницы EEPROM
    uint8_t bytesToWriteInThisPageSegment = 32 - (memoryAddress % 32);

    // Не записываем больше, чем осталось в буфере
    if (bytesToWriteInThisPageSegment > (length - currentBufferIndex)) {
      bytesToWriteInThisPageSegment = length - currentBufferIndex;
    }

    Wire.beginTransmission(EEPROM_I2C_ADDRESS);
    Wire.write((uint8_t)(memoryAddress >> 8));   // Старший байт адреса
    Wire.write((uint8_t)(memoryAddress & 0xFF)); // Младший байт адреса

    for (uint8_t i = 0; i < bytesToWriteInThisPageSegment; ++i) {
      Wire.write(buffer[currentBufferIndex + i]);
    }

    byte status = Wire.endTransmission();
    if (status != 0) {
      DEBUG_PRINT("I2C Write Error in buffer (addr "); DEBUG_PRINT(memoryAddress);
      DEBUG_PRINT("). Status: "); DEBUG_PRINTLN(status);
      // Прервать дальнейшую запись этого буфера, если есть ошибка
      return; 
    }
    delay(EEPROM_WRITE_DELAY); // Ожидание завершения цикла записи страницы

    memoryAddress += bytesToWriteInThisPageSegment;      // Сдвигаем адрес в EEPROM
    currentBufferIndex += bytesToWriteInThisPageSegment; // Сдвигаем указатель в буфере данных
  }
}

/**
 * @brief Читает массив байт (буфер) из EEPROM.
 * @param memoryAddress Начальный 16-битный адрес ячейки памяти.
 * @param buffer Указатель на буфер для сохранения прочитанных данных.
 * @param length Количество байт для чтения.
 */
void eepromReadBuffer(uint16_t memoryAddress, uint8_t* buffer, uint16_t length) {
  Wire.beginTransmission(EEPROM_I2C_ADDRESS);
  Wire.write((uint8_t)(memoryAddress >> 8));
  Wire.write((uint8_t)(memoryAddress & 0xFF));
  Wire.endTransmission();

  Wire.requestFrom(EEPROM_I2C_ADDRESS, (int)length); // Запросить 'length' байт
  for (uint16_t i = 0; i < length; i++) {
    if (Wire.available()) {
      buffer[i] = Wire.read();
    } else {
      buffer[i] = 0; // В случае ошибки заполнить нулем
    }
  }
}

// ----- Функции для работы с разными типами данных -----

void eepromWriteFloat(uint16_t address, float value) {
  uint8_t buffer[sizeof(float)];
  memcpy(buffer, &value, sizeof(float));
  eepromWriteBuffer(address, buffer, sizeof(float));
}

float eepromReadFloat(uint16_t address) {
  float value = 0.0f;
  uint8_t buffer[sizeof(float)];
  eepromReadBuffer(address, buffer, sizeof(float));
  memcpy(&value, buffer, sizeof(float));
  return value;
}

void eepromWriteString(uint16_t address, const String& str) {
  // Записываем длину строки первым байтом, затем саму строку (макс. длина 254 + 1 байт для null)
  // или просто строку до \0, если не хотим хранить длину явно.
  // Для простоты запишем строку как есть, включая нулевой терминатор.
  uint16_t len = str.length() + 1; // +1 для '\0'
  char charBuf[len];
  str.toCharArray(charBuf, len);
  eepromWriteBuffer(address, (uint8_t*)charBuf, len);
}

String eepromReadString(uint16_t address, uint16_t maxLength) {
  char charBuf[maxLength + 1]; // Буфер для строки + '\0'
  eepromReadBuffer(address, (uint8_t*)charBuf, maxLength);
  charBuf[maxLength] = '\0'; // Гарантируем нулевой терминатор

  // Найдем фактический конец строки, если он раньше maxLength
  for(uint16_t i=0; i<maxLength; ++i) {
    if (charBuf[i] == '\0') break;
  }
  return String(charBuf);
}
