#pragma once

#include <Arduino.h>
#include <FS.h>

#include "dlf_base_component.h"

class CSCDBSynchronizer : public BaseComponent {
    String server_ip;
    FS &_fs;
    String dir;
    uint16_t port;
    size_t max_retries;
    size_t upload_chunk_size;

    EventGroupHandle_t state;

   public:
    bool is_offloading;

    CSCDBSynchronizer(FS &fs, String fs_dir = "/", size_t max_retries = 10, size_t upload_size = 1024);

    void syncTo(String server_ip, uint16_t port);

    bool begin();

    // https://arduino.stackexchange.com/questions/93818/arduinohttpclient-post-multipart-form-data-from-sd-card-returning-400-bad-reques
    bool upload_run(File run_dir, String path);

    static void task_sync(void *arg);
};