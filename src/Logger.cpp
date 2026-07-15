#include "Logger.h"
#include "main.h"
#include "TelegramBot.h"

Logger sysLogger;

static bool isCriticalOrRecoveryMessage(const String& msg) {
    String upper = msg;
    upper.toUpperCase();

    // Critical keywords (RU, UA, EN)
    if (upper.indexOf("ТРЕВОГА") != -1 ||
        upper.indexOf("ТРИВОГА") != -1 ||
        upper.indexOf("ALARM") != -1 ||
        upper.indexOf("ОШИБКА") != -1 ||
        upper.indexOf("ПОМИЛКА") != -1 ||
        upper.indexOf("ERROR") != -1 ||
        upper.indexOf("СБОЙ") != -1 ||
        upper.indexOf("ЗБІЙ") != -1 ||
        upper.indexOf("FAILED") != -1 ||
        upper.indexOf("TIMEOUT") != -1 ||
        upper.indexOf("ЗАВИС") != -1 ||
        upper.indexOf("FROZEN") != -1) {
        return true;
    }

    // Recovery keywords (RU, UA, EN)
    if (upper.indexOf("ДОСТИГ") != -1 ||
        upper.indexOf("ДОСЯГ") != -1 ||
        upper.indexOf("REACHED") != -1 ||
        upper.indexOf("ВОССТАНОВ") != -1 ||
        upper.indexOf("ВІДНОВ") != -1 ||
        upper.indexOf("RESTORED") != -1 ||
        upper.indexOf("UPDATED") != -1 ||
        upper.indexOf("ИЗМЕНИЛ") != -1 ||
        upper.indexOf("ЗМІНИЛ") != -1) {
        return true;
    }

    return false;
}

void Logger::log(const String& message) {
    maintain();
    String filename = getLogFilename();
    File f = LittleFS.open(filename, "a");
    if (f) {
        String entry = getTimestamp() + " " + message + "\n";
        f.print(entry);
        f.close();
        MYDEBUG_PRINT("LOG: "); MYDEBUG_PRINT(entry);
    }

    if (isCriticalOrRecoveryMessage(message)) {
        sendTelegramMessage(message);
    }
}

void Logger::clear() {
    String filename = getLogFilename();
    LittleFS.remove(filename);
    MYDEBUG_PRINTLN("Log cleared.");
}

String Logger::getLogs() {
    String filename = getLogFilename();
    if (!LittleFS.exists(filename)) return "No logs found.";
    File f = LittleFS.open(filename, "r");
    if (!f) return "Error opening log file.";
    String content = f.readString();
    f.close();
    return content;
}

void Logger::maintain() {
    String filename = getLogFilename();
    if (!LittleFS.exists(filename)) return;
    File f = LittleFS.open(filename, "r");
    if (f) {
        size_t size = f.size();
        f.close();
        if (size > MAX_LOG_SIZE) {
            LittleFS.remove(filename);
            log(getMsg(MSG_LOG_ROTATED));
        }
    }
}

String Logger::getLogFilename() {
    if (timeinfo && timeinfo->tm_year >= 100) {
        char buf[32];
        snprintf(buf, sizeof(buf), "/day_%02d_%02d_log.txt", timeinfo->tm_mday, timeinfo->tm_mon + 1);
        return String(buf);
    }
    return "/day_00_00_log.txt";
}

String Logger::getTimestamp() {
    if (timeinfo && timeinfo->tm_year >= 100) {
        char buf[25];
        snprintf(buf, sizeof(buf), "[%02d.%02d %02d:%02d:%02d]", 
                 timeinfo->tm_mday, timeinfo->tm_mon + 1,
                 timeinfo->tm_hour, timeinfo->tm_min, timeinfo->tm_sec);
        return String(buf);
    }
    return "[00.00 00:00:00]";
}
