#include "InvertedServo.h"

InvertedServo::InvertedServo() {
  _pin = -1;
}

void InvertedServo::attach(int pin) {
  _pin = pin;
  pinMode(_pin, OUTPUT);
  
  // Настраиваем аппаратный ШИМ ESP8266 на частоту 50 Гц (период 20 мс)
  analogWriteFreq(50);
  analogWriteRange(1023);
  
  // По умолчанию устанавливаем 0% открытия (импульс 544 мкс)
  // Для инвертированного сигнала: 1023 - (544 * 1023 / 20000) = 995
  analogWrite(_pin, 995);
}

void InvertedServo::writeMicroseconds(int us) {
  if (_pin < 0) return;
  if (us < 544) us = 544;
  if (us > 2400) us = 2400;
  
  // Переводим микросекунды в скважность 0-1023 для периода 20000 мкс (20 мс)
  // Так как сигнал инвертирован транзистором, вычитаем полученную скважность из 1023
  int duty = 1023 - (us * 1023 / 20000);
  analogWrite(_pin, duty);
}

void InvertedServo::write(int percent) {
  if (percent < 0) percent = 0;
  if (percent > 100) percent = 100;
  
  // 100% открытия заслонки соответствует углу в 90 градусов (максимальный ход заслонки)
  int us = map(percent, 0, 100, 544, 1472);
  writeMicroseconds(us);
}
