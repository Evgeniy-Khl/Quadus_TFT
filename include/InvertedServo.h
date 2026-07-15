#ifndef __INVERTED_SERVO_H
#define __INVERTED_SERVO_H

#include <Arduino.h>

class InvertedServo {
public:
    InvertedServo();
    void attach(int pin);
    void write(int angle);
    void writeMicroseconds(int us);

private:
    int _pin;
};

#endif // __INVERTED_SERVO_H
