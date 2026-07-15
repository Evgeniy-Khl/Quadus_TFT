#ifndef _SAVEDAILYDATA_H
#define _SAVEDAILYDATA_H

#include "main.h"

struct ArchiveDay {
    int day;
    int month;
};

void saveDailyDataToFile(int day, int month);
void clearEEPROM();
void checkAndManageSpace();
ArchiveDay findOldestDay();
void deleteFilesForDay(int day, int month);
void listFilesAndSizes();

#endif /* _SAVEDAILYDATA_H */
