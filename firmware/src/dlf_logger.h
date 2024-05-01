#pragma once

#include <Arduino.h>

#include "dlf_run.h"
#include "dlf_logfile.hpp"
#include "dlf_types.h"

#define INDEX_FILE_PATH "/__INDEX"
#define BLOCK_SIZE 512
#define NUM_BLOCKS 10
#define BLOCK_OVERHEAD 512
#define MAX_RUNS 1

// #define DONT_WRITE_HEADER

// 0 is error, > 0 is valid handle
typedef int run_handle_t;

class CSCLogger
{
    std::unique_ptr<dlf::Run> runs[MAX_RUNS];

    // Todo: Figure out how to do this with unique_ptrs
    dlf::datastream::streams_t data_streams;

    FS &_fs;
    String fs_dir;

public:
    CSCLogger(FS &fs, String fs_dir = "/") : _fs(fs), fs_dir(fs_dir)
    {
    }

    run_handle_t get_available_handle()
    {
        for (size_t i = 0; i < MAX_RUNS; i++)
        {
            if (!runs[i])
                return i + 1;
        }

        return 0;
    }

    template <typename meta_t>
    run_handle_t start_run(meta_t meta, std::chrono::microseconds tick_rate = std::chrono::milliseconds(100))
    {
        run_handle_t h = get_available_handle();

        // If 0, out of space.
        if (!h)
            return h;

        Serial.printf("Starting logging with a cycle time-base of %dms\n", tick_rate);

        // Initialize new run
        dlf::Run *run = new dlf::Run(_fs, data_streams, tick_rate, meta);

        if(run == NULL)
            return 0;

        runs[h - 1] = std::unique_ptr<dlf::Run>(run);

        return h;
    }

    void stop_run(run_handle_t h)
    {
        if (!runs[h - 1])
            return;

        runs[h - 1]->close();
        runs[h - 1].reset();
    }

    template <typename T>
    CSCLogger &watch(T& value, String id)
    {
        using namespace dlf::datastream;

        AbstractStream *s = new EventStream(value, id);
        data_streams.push_back(s);
        
        return *this;
    }

    template <typename T>
    CSCLogger &poll(T& value, String id, microseconds sample_interval, microseconds phase = std::chrono::microseconds::zero())
    {
        using namespace dlf::datastream;

        AbstractStream *s = new PolledStream(value, id, sample_interval, phase);
        data_streams.push_back(s);

        return *this;
    }
};