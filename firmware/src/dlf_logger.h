#pragma once

#include <Arduino.h>

#include "dlf_run.h"
#include "dlf_logfile.hpp"
#include "dlf_types.h"
#include "datastream/EventStream.hpp"
#include "datastream/PolledStream.hpp"
#include <chrono>

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
    typedef std::chrono::microseconds microseconds;
    typedef std::chrono::milliseconds milliseconds;

    std::unique_ptr<dlf::Run> runs[MAX_RUNS];

    // Todo: Figure out how to do this with unique_ptrs
    dlf::datastream::streams_t data_streams;

    FS &_fs;
    String fs_dir;

public:
    struct
    {
        uint8_t lel;
    } components;

    CSCLogger(FS &fs, String fs_dir = "/") : _fs(fs), fs_dir(fs_dir)
    {
    }

    run_handle_t get_available_handle();

    run_handle_t start_run(Encodable meta, microseconds tick_rate = milliseconds(100));

    void stop_run(run_handle_t h);

    template <typename T>
    CSCLogger &watch(T &value, String id)
    {
        using namespace dlf::datastream;

        AbstractStream *s = new EventStream(value, id);
        data_streams.push_back(s);

        return *this;
    }

    template <typename T>
    CSCLogger &poll(T &value, String id, microseconds sample_interval, microseconds phase = microseconds::zero())
    {
        using namespace dlf::datastream;

        AbstractStream *s = new PolledStream(value, id, sample_interval, phase);
        data_streams.push_back(s);

        return *this;
    }
};