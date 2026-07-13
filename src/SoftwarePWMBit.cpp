#include "SoftwarePWMBit.h" // Подключаем наш заголовочный файл

// =================================================================
// Реализация методов класса SoftwarePWMBit
// =================================================================

// Конструктор
SoftwarePWMBit::SoftwarePWMBit(unsigned char* byte, uint8_t bitPosition) {
    targetByte = byte;
    bitMask = 1 << bitPosition;
    dutyCycle = 0;
    setFrequency(100);
    lastCycleStartMicros = micros();
}

// Метод для установки частоты
void SoftwarePWMBit::setFrequency(int freqHz) {
    if (freqHz > 0) {
        periodMicros = 1000000 / freqHz;
    }
}

// Метод для установки скважности
void SoftwarePWMBit::write(int value) {
    dutyCycle = constrain(value, 0, 255);
}

// Главный метод обновления
bool SoftwarePWMBit::update() {
    unsigned long now = micros();
    bool stateChanged = false;

    if (now - lastCycleStartMicros >= periodMicros) {
        lastCycleStartMicros += periodMicros;
    }

    unsigned long onTime = (periodMicros * dutyCycle) / 255;
    
    bool shouldBeOn = (now - lastCycleStartMicros < onTime);
    bool isCurrentlyOn = (*targetByte & bitMask) != 0;

    if (shouldBeOn != isCurrentlyOn) {
        stateChanged = true;
        if (shouldBeOn) {
            *targetByte |= bitMask;
        } else {
            *targetByte &= ~bitMask;
        }
    }
    
    return stateChanged;
}