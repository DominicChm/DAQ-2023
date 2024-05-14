#include <Arduino.h>
#include <FS.h>
#include <HTTPClient.h>
#include <WiFi.h>

#include "dlf_cfg.h"

class CSCDBSynchronizer
{
    String server_ip;
    FS &_fs;
    String dir;

public:
    CSCDBSynchronizer(String server_ip, FS &fs, String fs_dir = "/") : server_ip(server_ip), _fs(fs), dir(fs_dir)
    {
    }

    void begin()
    {
        xTaskCreate(task_sync, "sync", 4096, this, 5, NULL);
    }

    // void run_needs_upload(String *run_name)
    // {
    //     HTTPClient http;
    //     http.begin(serverPath.c_str());

    //     int httpResponseCode = http.GET();
    // }

    // https://arduino.stackexchange.com/questions/93818/arduinohttpclient-post-multipart-form-data-from-sd-card-returning-400-bad-reques
    void upload_file(File f, String abs_path)
    {
        WiFiClient client;

        if (!f)
        {
            Serial.println("No file.");
            vTaskDelete(NULL);
            return;
        }

        Serial.println("Uploading File...");
        if (!client.connect(server_ip.c_str(), 8080))
        {
            Serial.println("Failed connecting to Server");
            vTaskDelete(NULL);
            return;
        }

        String content = "--boundary1\r\n";
        content += "Content-Disposition: form-data; name=\"file\"; filename=" + String(f.name()) + "\r\n"; // the fileToUpload is the form parameter
        content += "Content-Type: application/x-binary\r\n\r\n";

        // after this, post the file data.
        String closingContent = "\r\n--boundary1--\r\n";

        // Serial.println(content);
        client.println("POST " + abs_path + " HTTP/1.1"); // or wherever you need to call/print to.
        client.println("Host: " + server_ip);
        client.println("Content-Length: " + String(content.length() + f.size() + closingContent.length()));
        client.print("Content-Type: multipart/form-data; boundary=boundary1\r\n\r\n");
        client.print(content);

        String data = "";
        const size_t chunk_size = 128;
        uint8_t buf[chunk_size];

        while (f.available())
        {
            size_t num_read = f.read(buf, chunk_size);
            client.write(buf, num_read);
        }
        f.close();
        client.print(closingContent);

        // Receive content
        unsigned long timeout = millis();
        while (client.available() == 0)
        {
            if (millis() - timeout > 5000)
            {
                Serial.println(">>> Client Timeout !");
                client.stop();
                return;
            }
        }

        // Read all the lines of the reply from server and print them to Serial
        while (client.available())
        {
            String line = client.readStringUntil('\r');
            Serial.print(line);
        }
        client.stop();
    }

    static void task_sync(void *arg)
    {
        CSCDBSynchronizer *a = static_cast<CSCDBSynchronizer *>(arg);

        while (true)
        {
            // TODO: Replace this with a common API/semaphore/mutex
            while (WiFi.status() != WL_CONNECTED)
            {
                vTaskDelay(pdMS_TO_TICKS(1000));
            }
#ifdef DEBUG
            DEBUG.println("Connected - Beginning sync");
#endif

            File root = a->_fs.open(a->dir);

            if (!root)
            {
                Serial.println("No storage");
                vTaskDelay(pdMS_TO_TICKS(1000));
                continue;
            }

            if (!root.isDirectory())
            {
                Serial.println("Root is not dir - exiting sync");
                return;
            }

            File run_dir;
            while (WiFi.status() == WL_CONNECTED && (run_dir = root.openNextFile()))
            {
                if (!strcmp(run_dir.name(), "System Volume Information"))
                    continue;

                Serial.printf("SYNC: %s\n", run_dir.name());

                if (!run_dir.isDirectory())
                    continue;

                File f;
                while (f = run_dir.openNextFile())
                {
                    // Upload.
                    String path = String("/upload/") + run_dir.name();
                    a->upload_file(f, path);
                }

                vTaskDelay(pdMS_TO_TICKS(100));
            }

            Serial.println("Done syncing");

            // Wait for disconnect to restart sync process
            // TODO: Make sync rerun whenever run is finalized.
            while (WiFi.status() == WL_CONNECTED)
            {
                vTaskDelay(pdMS_TO_TICKS(1000));
            }
        }
    }
};