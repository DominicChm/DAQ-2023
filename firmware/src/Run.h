#include <run_format.h>
#include <DataSource.h>
#include <FS.h>

#define MAX_ID 10

template<size_t NUM_SOURCES>
class Run
{
    run_header_t<NUM_SOURCES> header;

    Run(File f, DataSource sources[NUM_SOURCES])
    {

    }

    ~Run()
    {
    }
    
    void write_header()
    {
    }

    void add_format_entry()
    {
    }
};