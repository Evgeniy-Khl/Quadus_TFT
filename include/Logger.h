#ifndef LOGGER_H
#define LOGGER_H

#include <Arduino.h>
#include <LittleFS.h>

#define LOG_FILE "/system.log"
#define MAX_LOG_SIZE 51200 // 50KB limit

/**
 * @brief Class for managing system event logs in LittleFS.
 */
class Logger {
public:
    /**
     * @brief Log a message with a timestamp.
     * @param message The message to log.
     */
    void log(const String& message);

    /**
     * @brief Clear the log file.
     */
    void clear();

    /**
     * @brief Get the content of the log file.
     * @return String containing all log entries.
     */
    String getLogs();

    /**
     * @brief Check and manage log file size.
     */
    void maintain();

    /**
     * @brief Get current log filename.
     */
    String getLogFilename();

private:
    String getTimestamp();
};

extern Logger sysLogger;

#endif // LOGGER_H
