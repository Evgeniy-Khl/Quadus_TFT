#include "main.h"

extern TFT_eSPI tft;

#define DEG2RAD 0.0174532925
#define LOOP_DELAY 10 // Loop delay to slow things down
#define MAX_SENSOR 2



extern GrafDispl grafDispl[];

void initMyConfig(void);
void fillArc(int x, int y, int start_angle, int seg_count, int rx, int ry, int w, unsigned int colour);
unsigned int rainbow(byte value);
void diagram(GrafDispl grafDispl, uint16_t color);
void printAddress(DeviceAddress deviceAddress);

