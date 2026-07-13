#include "Arduino.h"
#include <Wire.h>

// I2C адрес AT24C32.
// Если A0, A1, A2 подключены к GND, адрес 0x50.
// Адрес может быть от 0x50 до 0x57 в зависимости от состояния пинов A0-A2.
#define EEPROM_I2C_ADDRESS 0x57

// Задержка после операции записи в мс (время записи страницы для AT24C32 до 5мс)
#define EEPROM_WRITE_DELAY 5

/**
 * @brief Записывает один байт в EEPROM.
 * @param memoryAddress 16-битный адрес ячейки памяти (0-4095).
 * @param data Байт для записи.
 */
void eepromWriteByte(uint16_t memoryAddress, uint8_t data);

/**
 * @brief Читает один байт из EEPROM.
 * @param memoryAddress 16-битный адрес ячейки памяти (0-4095).
 * @return Прочитанный байт или 0 если произошла ошибка.
 */
uint8_t eepromReadByte(uint16_t memoryAddress);

/**
 * @brief Записывает массив байт (буфер) в EEPROM.
 * @param memoryAddress Начальный 16-битный адрес ячейки памяти.
 * @param buffer Указатель на массив байт для записи.
 * @param length Количество байт для записи.
 * Примечание: Эта функция записывает байты последовательно.
 * AT24C32 имеет страницы по 32 байта. Для оптимальной записи больших блоков
 * следует учитывать границы страниц, но эта функция для простоты этого не делает явно.
 * Однако, последовательная запись через Wire.write() для AT24C32 обычно работает корректно
 * в пределах страницы. Для записи через границы страниц может потребоваться несколько транзакций.
 */
void eepromWriteBuffer(uint16_t memoryAddress, const uint8_t* buffer, uint16_t length);

/**
 * @brief Читает массив байт (буфер) из EEPROM.
 * @param memoryAddress Начальный 16-битный адрес ячейки памяти.
 * @param buffer Указатель на буфер для сохранения прочитанных данных.
 * @param length Количество байт для чтения.
 */
void eepromReadBuffer(uint16_t memoryAddress, uint8_t* buffer, uint16_t length);

// ----- Функции для работы с разными типами данных -----

void eepromWriteFloat(uint16_t address, float value);
float eepromReadFloat(uint16_t address);
void eepromWriteString(uint16_t address, const String& str);
String eepromReadString(uint16_t address, uint16_t maxLength);
