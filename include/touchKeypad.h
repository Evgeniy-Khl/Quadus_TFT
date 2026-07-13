#include "main.h"
#include "display.h"

// Numeric display box size and location
#define DISP_X 1
#define DISP_Y 10
#define DISP_W 318
#define DISP_H 100
#define DISP_TSIZE 3
#define DISP_TCOLOR TFT_CYAN

// This is the file name used to store the calibration data
// You can change this to create new calibration files.
// The LittleFS file name must start with "/".
#define CALIBRATION_FILE "/TouchCalData1"

// Set REPEAT_CAL to true instead of false to run calibration
// again, otherwise it will only be done once.
// Repeat calibration if you change the screen rotation.
#define REPEAT_CAL false

// Number length, buffer for storing it and character index
#define NUM_LEN 12

// We have a status line for messages
#define STATUS_X 120 // Centred on this
#define STATUS_Y 65

void checkKeypad(uint8_t amt);
void touch_calibrate();
void status(const char *msg);
int8_t butCalculator(uint8_t butt);
void drawValue(int8_t val, bool divide);
