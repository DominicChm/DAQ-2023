#include <Arduino.h>
#include <WiFi.h>

#include "dlf_cfg.h"

class CSCWifiClient
{
    String ssid;
    String password;

public:
    CSCWifiClient(String ssid, String password) : ssid(ssid), password(password)
    {
    }

    void begin()
    {
        WiFi.begin(ssid.c_str(), password.c_str());
        xTaskCreate(task_wifi, "wifi", 1024, this, 5, NULL);
    }

    static void task_wifi(void *arg)
    {
        CSCWifiClient *a = static_cast<CSCWifiClient *>(arg);

        if (WiFi.status() != WL_CONNECTED)
        {
#ifdef DEBUG
            DEBUG.println("Trying to connect to wifi...");
#endif
            WiFi.disconnect();
            WiFi.reconnect();
        }

        // Wait at least 10s between reconnect attempts
        vTaskDelay(pdMS_TO_TICKS(10000));
    }
};