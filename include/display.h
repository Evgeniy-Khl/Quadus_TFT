#ifndef __DISPLAY_H
#define __DISPLAY_H

#include <main.h>
#include "tftArcFill.h"

#define MENU_1 7
#define MENU_2 12
#define MENU_3 12

// Keypad start position, key sizes and spacing
#define KEY_X 40 // Centre of key
#define KEY_Y 130
#define KEY_W 55 // Width and height
#define KEY_H 35
#define KEY_SPACING_X 5 // X and Y gap
#define KEY_SPACING_Y 5
#define KEY_TEXTSIZE 1   // Font size multiplier

// Numeric display box size and location
#define DISP_X 1
#define DISP_Y 10
#define DISP_W 318
#define DISP_H 100
#define DISP_TSIZE 3
#define DISP_TCOLOR TFT_CYAN

// Using two fonts since numbers are nice when bold
#define LABEL1_FONT &FreeSansOblique12pt7b // Key label font 1
#define LABEL2_FONT &FreeSansBold12pt7b    // Key label font 2

extern const char* labelsMenu1[];
extern const char* labelsMenu2[];
extern const char* labelsMenu3[];
extern const char* labelsCalculator[];
extern char numberBuffer[];
extern uint8_t numberIndex, earlyDispl;
extern bool newTxt;

extern TFT_eSPI_Button key[];
extern const char* keyLabel[15];
extern uint16_t keyColor[15];

void mainDispl(void);
void menu_1(void);
void menu_2(void);
void menu_3(void);
void menu_4(void);
void calcDisplay(const char* txt);
void drawKeypad(const char* keyLabel[], uint16_t keyColor[]);
void drawKeypad_longName_7(const char* keyLabel[], uint16_t keyColor[], uint8_t amt_row, uint8_t amt_col);
void drawKeypad_longName_12(const char* keyLabel[], uint16_t keyColor[], uint8_t amt_row, uint8_t amt_col);
uint16_t lampUpdate(uint16_t xpos, uint16_t ypos);

#endif /* __DISPLAY_H */