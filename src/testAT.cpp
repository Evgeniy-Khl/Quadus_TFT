#include "main.h"
#include <AT24C32.h>

void testAT24C32() {
  MYDEBUG_PRINTLN("---------------ESP8266 <-> AT24C32 EEPROM Test---------------");

  uint16_t byteAddr = 0;
  uint16_t intAddr = 10;
  uint16_t floatAddr = 20;
  uint16_t stringAddr = 30;

  // --- Демонстрация записи ---
  MYDEBUG_PRINT("\nWriting data to EEPROM. EEPROM_I2C_ADDRESS: 0x"); MYDEBUG_PRINTLN(EEPROM_I2C_ADDRESS, HEX);

  // Запись одного байта
  uint8_t testByte = 0xA5;
  eepromWriteByte(byteAddr, testByte);
  MYDEBUG_PRINT("Wrote byte 0x"); MYDEBUG_PRINT(testByte, HEX); MYDEBUG_PRINT(" to address "); MYDEBUG_PRINTLN(byteAddr);

  // Запись целого числа (int - 4 байта на ESP8266)
  int testInt = 12345;
  eepromWriteBuffer(intAddr, (uint8_t*)&testInt, sizeof(testInt));
  MYDEBUG_PRINT("Wrote int "); MYDEBUG_PRINT(testInt); MYDEBUG_PRINT(" to address "); MYDEBUG_PRINTLN(intAddr);
  
  // Запись числа с плавающей точкой
  float testFloat = 3.14159f;
  eepromWriteFloat(floatAddr, testFloat);
  MYDEBUG_PRINT("Wrote float "); MYDEBUG_PRINT(testFloat, 5); MYDEBUG_PRINT(" to address "); MYDEBUG_PRINTLN(floatAddr);

  // Запись строки
  String testString = "Hello AT24C32!";
  eepromWriteString(stringAddr, testString);
  MYDEBUG_PRINT("Wrote string '"); MYDEBUG_PRINT(testString); MYDEBUG_PRINT("' to address "); MYDEBUG_PRINTLN(stringAddr);

  MYDEBUG_PRINTLN("\nWrite operations complete.");
  MYDEBUG_PRINTLN("------------------------------------");
  delay(1000); // Дадим время на осмысление

  // --- Демонстрация чтения ---
  MYDEBUG_PRINTLN("Reading data from EEPROM...");
  MYDEBUG_PRINTLN();
  // Чтение одного байта
  uint8_t readB = eepromReadByte(byteAddr);
  MYDEBUG_PRINT("Read byte from address "); MYDEBUG_PRINT(byteAddr); MYDEBUG_PRINT(": 0x"); MYDEBUG_PRINTLN(readB, HEX);

  // Чтение целого числа
  int readI;
  eepromReadBuffer(intAddr, (uint8_t*)&readI, sizeof(readI));
  MYDEBUG_PRINT("Read int from address "); MYDEBUG_PRINT(intAddr); MYDEBUG_PRINT(": "); MYDEBUG_PRINTLN(readI);

  // Чтение числа с плавающей точкой
  float readF = eepromReadFloat(floatAddr);
  MYDEBUG_PRINT("Read float from address "); MYDEBUG_PRINT(floatAddr); MYDEBUG_PRINT(": "); MYDEBUG_PRINTLN(readF, 5);

  // Чтение строки
  // Укажите максимальную ожидаемую длину строки. Она должна быть не больше, чем было записано.
  // Если строка короче, она будет корректно прочитана благодаря нулевому терминатору.
  String readS = eepromReadString(stringAddr, 30); // 30 - максимальная длина буфера для чтения
  MYDEBUG_PRINT("Read string from address "); MYDEBUG_PRINT(stringAddr); MYDEBUG_PRINT(": '"); MYDEBUG_PRINT(readS); MYDEBUG_PRINTLN("'");
  
  MYDEBUG_PRINTLN("\nRead operations complete.");
}