#include "dlf_logger.h"

run_handle_t CSCLogger::get_available_handle()
{
    for (size_t i = 0; i < MAX_RUNS; i++)
    {
        if (!runs[i])
            return i + 1;
    }

    return 0;
}

run_handle_t CSCLogger::start_run(Encodable meta, std::chrono::microseconds tick_rate)
{
    run_handle_t h = get_available_handle();

    // If 0, out of space.
    if (!h)
        return h;

    Serial.printf("Starting logging with a cycle time-base of %dms\n", tick_rate);

    // Initialize new run
    dlf::Run *run = new dlf::Run(_fs, data_streams, tick_rate, meta);

    if (run == NULL)
        return 0;

    runs[h - 1] = std::unique_ptr<dlf::Run>(run);

    return h;
}

void CSCLogger::stop_run(run_handle_t h)
{
    if (!runs[h - 1])
        return;

    runs[h - 1]->close();
    runs[h - 1].reset();
}