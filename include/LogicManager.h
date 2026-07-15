#ifndef LOGIC_MANAGER_H
#define LOGIC_MANAGER_H

#include <Arduino.h>

/**
 * @brief Класс для управления основной логикой теплицы (термостат, полив, освещение).
 */
class LogicManager {
public:
    /**
     * @brief Обработка логики освещения.
     */
    void processLighting();

    /**
     * @brief Обработка логики полива (таймеры реле).
     */
    void processIrrigation();

    /**
     * @brief Обработка логики климата (обогрев и увлажнение).
     */
    void processClimate();

    /**
     * @brief Обработка аварийных ситуаций.
     */
    void processAlarms();

    /**
     * @brief Обновление состояний светодиодов статуса.
     */
    void updateStatusLeds();

    // Вспомогательные методы управления
    void relaySwitch(uint8_t channel);
    bool checkDeviceState(bool previousState, int16_t currentVal, int16_t onVal, int16_t offVal, uint8_t mode);
    bool checkLightState(uint8_t currentHour, uint8_t onHour, uint8_t offHour);
    void processAlarm(uint8_t channel);

private:
    uint16_t transformTimeOff(uint8_t point);
};

extern LogicManager logicManager;

#endif // LOGIC_MANAGER_H
