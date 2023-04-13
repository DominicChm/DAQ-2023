#include <run_format.h>
#include <Run.h>
#include <SD_MMC.h>
#define INDEX_FILE_PATH "/__INDEX"
template <size_t NUM_SOURCES>
class RunManager
{
    const DataSource (&data_sources)[NUM_SOURCES];
    const uint32_t base_cycle_interval_ms;

public:
    RunManager(uint32_t base_cycle_interval_ms, const DataSource (&data_sources)[NUM_SOURCES]) : data_sources(data_sources),
                                                                                                 base_cycle_interval_ms(base_cycle_interval_ms) {}
    bool init()
    {
        if (!SD_MMC.begin())
        {
            return false;
        }

        Serial.printf("SD Initialized! Size: %lumb\n", SD_MMC.cardSize() / (1llu << 20));
        return true;
    }

    void new_run()
    {
        run_header_t<NUM_SOURCES> head{};
        String file_name = get_random_file_name();
        // SD_MMC.open(get_random_id(), FILE_WRITE);
        Serial.printf("Beginning new run in %s\n", file_name.c_str());
        index_add_new(file_name, head);
        // Create run file

        // Instantiate a run on that file
    }

    void loop()
    {
    }

    String get_random_file_name()
    {
        uint8_t rng_buf[16];
        esp_fill_random(rng_buf, sizeof(rng_buf));

        char id_buf[sizeof(rng_buf) * 2 + 2] = {'/'};

        for (size_t i = 0; i < sizeof(rng_buf); i++)
        {
            sprintf(&(id_buf[i * 2 + 1]), "%02X", rng_buf[i]);
        }

        return String(id_buf);
    }

    void index_add_new(String file_name, run_header_t<NUM_SOURCES> &header)
    {
        File f = SD_MMC.open(INDEX_FILE_PATH, FILE_APPEND);
        if (!f)
        {
            Serial.println("!! Error opening index file!");
            return;
        }
        f.printf(R"("%s": {"name":"%s"})", file_name.c_str(), header.name);
        f.println();
        f.close();

        Serial.println("Instantiated index entry!");
    }

    void index_delete()
    {
    }
};