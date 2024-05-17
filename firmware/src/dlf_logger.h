#pragma once

#include <Arduino.h>

#include <chrono>
#include <vector>

#include "components/dlf_sync.h"
#include "components/dlf_wifi.h"
#include "datastream/EventStream.hpp"
#include "datastream/PolledStream.hpp"
#include "dlf_logfile.hpp"
#include "dlf_run.h"
#include "dlf_types.h"

#define INDEX_FILE_PATH "/__INDEX"
#define BLOCK_SIZE 512
#define NUM_BLOCKS 10
#define BLOCK_OVERHEAD 512
#define MAX_RUNS 1

// 0 is error, > 0 is valid handle
typedef int run_handle_t;

class CSCLogger : public BaseComponent {
    typedef std::chrono::microseconds microseconds;
    typedef std::chrono::milliseconds milliseconds;

    std::unique_ptr<dlf::Run> runs[MAX_RUNS];

    // Todo: Figure out how to do this with unique_ptrs
    dlf::datastream::streams_t data_streams;

    FS &_fs;
    String fs_dir;

    std::vector<BaseComponent *> components;

   public:
    enum LoggerEvents : uint32_t {
        NEW_RUN = 1
    };

    EventGroupHandle_t ev;

    CSCLogger(FS &fs, String fs_dir = "/");

    bool begin();

    run_handle_t get_available_handle();

    run_handle_t start_run(Encodable meta, microseconds tick_rate = milliseconds(100));

    void stop_run(run_handle_t h);

    bool run_is_active(const char* uuid);

    CSCLogger &watch(Encodable value, String id);

    CSCLogger &poll(Encodable value, String id, microseconds sample_interval, microseconds phase = microseconds::zero());

    CSCLogger &syncTo(String server_ip, uint16_t port);

    CSCLogger &wifi(String ssid, String password);
};