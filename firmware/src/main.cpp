#include <Arduino.h>
#include <RunManager.h>
#include <DataSource.h>
#include <communication/communication.h>
#include <type_name.h>

T_DATA daq_data;

typedef DataSource ds;

uint8_t d0 = 0;
uint8_t d1 = 1;
uint8_t d15 = 15;
uint8_t d2 = 2;
uint8_t d3 = 3;

DataSource data_sources[] = {
    ds(1000, d0, "data0"),
    ds(1000, d1, "data1"),
    ds(1500, d15, "data1.5"),
    ds(2000, d2, "data2"),
    ds(3000, d3, "data3"),
};

RunManager rm(1000, data_sources);

void setup()
{
    daq_data.electrons.voltage_sense36 = 0x01;
    daq_data.sus.bl_pot = 0x02;
    Serial.begin(115200);
    rm.init();
    Serial.print("$ ");
}

void process_serial_command(char *c)
{
    char *cmd = strtok(c, " ");

    if (cmd == nullptr)
        return;

    uint32_t cmd_hash = hash(cmd);

    switch (cmd_hash)
    {
    case hash("start"):
        rm.start_new_run();
        break;

    case hash("stop"):
        rm.finish_current_run();
        break;

    default:
        Serial.printf("Unknown command \"%s\"\n", cmd);
    }
}

void ux()
{
    if (!Serial.available())
        return;

    static char sbuf[256];
    static int idx;

    char c = Serial.read();

    // Ignore all carrige returns ( windows... >:( )
    if (c == '\r')
        return;
    else if (c == '\b' && idx > 0)
    {
        idx--;
        Serial.print("\b \b");
        return;
    }
    else if (c != '\n')
    {
        sbuf[idx++] = c;

        // Echo output
        Serial.print(c);
        return;
    }

    // replace newline with nullterm
    sbuf[idx] = '\0';
    idx = 0;

    Serial.println();
    process_serial_command(sbuf);
    Serial.print("$ ");
}

void loop()
{
    ux();
}