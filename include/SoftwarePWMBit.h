#ifndef SOFTWAREPWMBIT_H
#define SOFTWAREPWMBIT_H

#include <Arduino.h>

// =================================================================
// Декларация класса SoftwarePWMBit
// =================================================================
class SoftwarePWMBit {
private:
    // Поля класса (переменные)
    unsigned char* targetByte;      
    unsigned char bitMask;          
    int dutyCycle;                  
    
    unsigned long periodMicros;     
    unsigned long lastCycleStartMicros; 

public:
    // Декларации методов (функций) класса
    SoftwarePWMBit(unsigned char* byte, uint8_t bitPosition);
    void setFrequency(int freqHz);
    void write(int value);
    bool update();
};

#endif // SOFTWAREPWMBIT_H