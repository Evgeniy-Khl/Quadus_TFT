#include "TelegramBot.h"
#include "main.h"
#include <WiFiClientSecure.h>
#include <ESP8266HTTPClient.h>

void sendTelegramMessage(const String& message, bool force) {
    if (botToken[0] == '\0' || chatID[0] == '\0') {
        MYDEBUG_PRINTLN("Telegram: Bot Token or Chat ID not configured.");
        return;
    }

    if (WiFi.status() != WL_CONNECTED) {
        MYDEBUG_PRINTLN("Telegram: WiFi not connected.");
        return;
    }

    if (!force && tmrTelegramOff > 0) {
        DEBUG_PRINTF("Telegram: Notification suppressed. Cooldown active (tmrTelegramOff = %d)\n", tmrTelegramOff);
        return;
    }

    MYDEBUG_PRINTLN("Telegram: Sending message...");
    
    // Feed the watchdog to prevent resets during potential network delays
    ESP.wdtFeed();

    WiFiClientSecure client;
    client.setInsecure(); // Skip TLS certificate verification to prevent issues when Telegram rotates certificates

    HTTPClient http;
    String url = "https://api.telegram.org/bot" + String(botToken) + "/sendMessage";

    // Setup JSON payload
    JsonDocument doc;
    doc["chat_id"] = chatID;
    
    // Format message with device prefix
    String fullMessage = "[Quadus #" + String(settings.deviceNum & 0x0F) + "] " + message;
    doc["text"] = fullMessage;

    String payload;
    serializeJson(doc, payload);

    if (http.begin(client, url)) {
        http.addHeader("Content-Type", "application/json");
        int httpCode = http.POST(payload);

        if (httpCode > 0) {
            DEBUG_PRINTF("Telegram: Response code %d\n", httpCode);
            #ifdef DEBUG
            String response = http.getString();
            MYDEBUG_PRINTLN("Telegram: Response: " + response);
            #endif
        } else {
            DEBUG_PRINTF("Telegram: Send failed, error: %s\n", http.errorToString(httpCode).c_str());
        }
        http.end();
    } else {
        MYDEBUG_PRINTLN("Telegram: Connection failed.");
    }

    ESP.wdtFeed();
}
