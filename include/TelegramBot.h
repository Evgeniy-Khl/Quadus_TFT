#ifndef TELEGRAM_BOT_H
#define TELEGRAM_BOT_H

#include <Arduino.h>

/**
 * @brief Send a notification to Telegram.
 * @param message The message text.
 * @param force If true, bypasses the tmrTelegramOff silence timer.
 */
void sendTelegramMessage(const String& message, bool force = false);

#endif // TELEGRAM_BOT_H
