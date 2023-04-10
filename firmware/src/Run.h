#include <run_format.h>
#include <FS.h>

#define MAX_ID 10

class Run
{
    void *format_data[MAX_ID];

    Run(File f)
    {
    }

    ~Run()
    {
    }

    template <class fmt>
    void add_fmt(uint32_t interval)
    {
        uint8_t* f = new fmt();
        // static_assert(f->id < MAX_ID, "ID Error!");
    }

    void write_header()
    {
    }

    void add_format_entry()
    {
    }
};