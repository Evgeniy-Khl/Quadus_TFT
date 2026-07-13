#include "main.h"
#include <AT24C32.h>

void testAT24C32() {
  DEBUG_PRINTLN("---------------ESP8266 <-> AT24C32 EEPROM Test---------------");

  uint16_t byteAddr = 0;
  uint16_t intAddr = 10;
  uint16_t floatAddr = 20;
  uint16_t stringAddr = 30;

  // --- Демонстрация записи ---
  DEBUG_PRINT("\nWriting data to EEPROM. EEPROM_I2C_ADDRESS: 0x"); DEBUG_PRINTLN(EEPROM_I2C_ADDRESS, HEX);

  // Запись одного байта
  uint8_t testByte = 0xA5;
  eepromWriteByte(byteAddr, testByte);
  DEBUG_PRINT("Wrote byte 0x"); DEBUG_PRINT(testByte, HEX); DEBUG_PRINT(" to address "); DEBUG_PRINTLN(byteAddr);

  // Запись целого числа (int - 4 байта на ESP8266)
  int testInt = 12345;
  eepromWriteBuffer(intAddr, (uint8_t*)&testInt, sizeof(testInt));
  DEBUG_PRINT("Wrote int "); DEBUG_PRINT(testInt); DEBUG_PRINT(" to address "); DEBUG_PRINTLN(intAddr);
  
  // Запись числа с плавающей точкой
  float testFloat = 3.14159f;
  eepromWriteFloat(floatAddr, testFloat);
  DEBUG_PRINT("Wrote float "); DEBUG_PRINT(testFloat, 5); DEBUG_PRINT(" to address "); DEBUG_PRINTLN(floatAddr);

  // Запись строки
  String testString = "Hello AT24C32!";
  eepromWriteString(stringAddr, testString);
  DEBUG_PRINT("Wrote string '"); DEBUG_PRINT(testString); DEBUG_PRINT("' to address "); DEBUG_PRINTLN(stringAddr);

  DEBUG_PRINTLN("\nWrite operations complete.");
  DEBUG_PRINTLN("------------------------------------");
  delay(1000); // Дадим время на осмысление

  // --- Демонстрация чтения ---
  DEBUG_PRINTLN("Reading data from EEPROM...");
  DEBUG_PRINTLN();
  // Чтение одного байта
  uint8_t readB = eepromReadByte(byteAddr);
  DEBUG_PRINT("Read byte from address "); DEBUG_PRINT(byteAddr); DEBUG_PRINT(": 0x"); DEBUG_PRINTLN(readB, HEX);

  // Чтение целого числа
  int readI;
  eepromReadBuffer(intAddr, (uint8_t*)&readI, sizeof(readI));
  DEBUG_PRINT("Read int from address "); DEBUG_PRINT(intAddr); DEBUG_PRINT(": "); DEBUG_PRINTLN(readI);

  // Чтение числа с плавающей точкой
  float readF = eepromReadFloat(floatAddr);
  DEBUG_PRINT("Read float from address "); DEBUG_PRINT(floatAddr); DEBUG_PRINT(": "); DEBUG_PRINTLN(readF, 5);

  // Чтение строки
  // Укажите максимальную ожидаемую длину строки. Она должна быть не больше, чем было записано.
  // Если строка короче, она будет корректно прочитана благодаря нулевому терминатору.
  String readS = eepromReadString(stringAddr, 30); // 30 - максимальная длина буфера для чтения
  DEBUG_PRINT("Read string from address "); DEBUG_PRINT(stringAddr); DEBUG_PRINT(": '"); DEBUG_PRINT(readS); DEBUG_PRINTLN("'");
  
  DEBUG_PRINTLN("\nRead operations complete.");
}