#include "main.h"
#include "TelegramBot.h"

void saveConfigCallback();

static bool serverStarted = false;
static unsigned long lastReconnectAttempt = 0;

void setupWebServerRoutes() {
    server.on("/", HTTP_GET, []() {
      mode = READDEFAULT; interval = INTERVAL_4000; tmrTelegramOff = 300;
      if (!LittleFS.exists("/index.html")) {
        MYDEBUG_PRINTLN("index.html not found");
      } else {
        File file = LittleFS.open("/index.html", "r");
        if (!file) {
            server.send(404, "text/plain", "I can't open the index.html");
            return;
        }
        streamFileChunked(file, "text/html");
        file.close();
      }
    });
    server.on("/setup", HTTP_GET, []() {
      File file = LittleFS.open("/setup.html", "r");
      if (!file) {
          server.send(404, "text/plain", "File Not Found");
          return;
      }
      streamFileChunked(file, "text/html");
      file.close();
    });
    server.on("/table", HTTP_GET, []() {
      File file = LittleFS.open("/table.html", "r");
      if (!file) {
          server.send(404, "text/plain", "File Not Found");
          return;
      }
      streamFileChunked(file, "text/html");
      file.close();
    });
    server.on("/getvalues", HTTP_GET, respondsValues);
    server.on("/geteeprom", HTTP_GET, respondsEeprom);
    server.on("/seteeprom", HTTP_POST, acceptEeprom);
    server.on("/getprogram", HTTP_GET, respondsProgram);
    server.on("/setprogram", HTTP_POST, acceptProgram);
    server.on("/program", HTTP_GET, []() {
      File file = LittleFS.open("/program.html", "r");
      if (!file) {
          server.send(404, "text/plain", "program.html not found");
          return;
      }
      streamFileChunked(file, "text/html");
      file.close();
    });
    
    // Manual control routes
    server.on("/switch", HTTP_GET, []() {
        File file = LittleFS.open("/switch.html", "r");
        if (file) {
            streamFileChunked(file, "text/html");
            file.close();
        } else {
            server.send(404, "text/plain", "switch.html not found");
        }
    });
    server.on("/view_logs", HTTP_GET, []() {
        File file = LittleFS.open("/logs.html", "r");
        if (file) {
            streamFileChunked(file, "text/html");
            file.close();
        } else {
            server.send(404, "text/plain", "logs.html not found");
        }
    });
    server.on("/get_relays", HTTP_GET, handleGetRelayStates);
    server.on("/set_relay", HTTP_POST, handleManualControl);
    server.on("/reset_auto", HTTP_POST, resetAutoControl);
    server.on("/logs", HTTP_GET, handleGetLogs);
    server.on("/clear_logs", HTTP_POST, handleClearLogs);

    // Graph and Archive routes
    server.on("/archive", HTTP_GET, handleArchiveList);
    server.on("/data", HTTP_GET, handleShowData);
    server.on("/current", HTTP_GET, handleCurrentData);
    server.on("/get_graph", HTTP_GET, handleGetGraph);
    server.on("/get_current_graph", HTTP_GET, handleGetCurrentGraph);

    server.onNotFound(notFoundHandler);
}

void setupServices() {
    if (WiFi.status() == WL_CONNECTED) {
        static IPAddress lastIP;
        if (!WIFIENABLE || WiFi.localIP() != lastIP) {
            lastIP = WiFi.localIP();
            MYDEBUG_PRINT("Wi-Fi подключен! IP:");
            MYDEBUG_PRINTLN(lastIP);
            sysLogger.log("Wi-Fi подключен! IP: " + lastIP.toString());

            // Настройка NTP для Киева
            configTzTime(tzInfo, ntpServer);
            sysLogger.log("NTP время настроено.");

            WIFIENABLE = 1;

            // Отправка приветствия в Telegram
            sendTelegramMessage("Устройство запущено! IP: " + lastIP.toString(), true);
        }

        if (!serverStarted) {
            server.begin();
            serverStarted = true;
            MYDEBUG_PRINTLN("HTTP server started");
            
            uint16_t heapSize = ESP.getFreeHeap();
            DEBUG_PRINTF("Free heap size: %d\n", heapSize);
        }
    }
}

void initWiFiManag(void){
    WiFiManager wifiManager;

    wifiManager.setSaveConfigCallback(saveConfigCallback);

    uint8_t tt = (settings.special & 0x03) * 60;
    MYDEBUG_PRINT("Устанавливаем таймаут для портала конфигурации (сек.):");
    MYDEBUG_PRINTLN(tt);
    
    wifiManager.setConfigPortalTimeout(tt);  
    
    setupWebServerRoutes(); // Регистрируем маршруты всегда

    if (!wifiManager.autoConnect("GravitonAP")) {
      MYDEBUG_PRINTLN("He удалось подключиться (истек таймаут). Продолжаем работу в оффлайн-режиме.");
    } else {
        setupServices();
        MYDEBUG_PRINT("Wi-Fi Local ip: ");
        MYDEBUG_PRINTLN(WiFi.localIP());
        delay(2000);
    }
}

void handleWiFi(void) {
    if (WiFi.status() != WL_CONNECTED) {
        if (WIFIENABLE) {
            MYDEBUG_PRINTLN("Wi-Fi связь потеряна!");
            sysLogger.log("Wi-Fi связь потеряна!");
            WIFIENABLE = 0;
            serverStarted = false; // Позволит перезапустить сервер при переподключении
        }
        
        if (millis() - lastReconnectAttempt > 120000 || lastReconnectAttempt == 0) {
            lastReconnectAttempt = millis();
            MYDEBUG_PRINTLN("Попытка переподключения к Wi-Fi...");
            WiFi.begin(); 
        }
    } else {
        setupServices(); // Если подключено, убеждаемся что всё запущено
    }
}

//callback notifying us of the need to save config
void saveConfigCallback() {
    DEBUG_PRINTLN("Should save config");
    shouldSaveConfig = true;
}
