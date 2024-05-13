#include <Arduino.h>
#include <WiFi.h>

#include "dlf_cfg.h"

class CSCWifiClient {
    String ssid;
    String password;

   public:
    CSCWifiClient(String ssid, String password) : ssid(ssid), password(password) {
    }

    void begin() {
        WiFi.begin(ssid.c_str(), password.c_str());
        xTaskCreate(task_wifi, "wifi", 1024, this, 5, NULL);
    }

    static void task_wifi(void* arg) {
        CSCWifiClient* a = static_cast<CSCWifiClient*>(arg);

        while (true) {
#ifdef DEBUG
            DEBUG.println("Trying to connect to wifi...");
#endif
            while (WiFi.status() != WL_CONNECTED) {
                vTaskDelay(pdMS_TO_TICKS(200));
            }
#ifdef DEBUG
            DEBUG.println(WiFi.localIP());
#endif
            while (WiFi.status() == WL_CONNECTED) {
                vTaskDelay(pdMS_TO_TICKS(1000));
            }
#ifdef DEBUG
            DEBUG.println("Wifi disconnected");
#endif
        }
    }
};