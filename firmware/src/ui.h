#include <Arduino.h>

class UI
{
    struct command_t
    {
        std::function<void(UI &)> handler;
        uint32_t hash;
    };

    command_t *commands;
    size_t num_commands;
    char sbuf[256];
    int idx;

    Stream &_io;

public:
    UI(Stream &io) : _io(io)
    {
    }

    void add_command(const char *command_string, std::function<void(UI &)> handler)
    {
        commands = (command_t *)realloc(commands, ++num_commands * sizeof(command_t));
        if (commands == nullptr)
        {
            Serial.println("ERROR ALLOCATING COMMANDS");
            return;
        }
        Serial.println((uint32_t)commands);
        // commands[num_commands - 1].hash = hash(command_string);

        commands[num_commands - 1].handler = handler;

        Serial.println("Added 1 command");
    }

    void poll()
    {
        if (!Serial.available())
            return;

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
        process_stored_command();
        Serial.print("$ ");
    }

    void process_stored_command()
    {
        char *cmd = strtok(sbuf, " ");

        if (cmd == nullptr)
            return;

        uint32_t cmd_hash = hash(cmd);

        for (int i = 0; i < num_commands; i++)
        {
            if (commands[i].hash == cmd_hash)
            {
                run_command(commands[i]);
                return;
            }
        }

        Serial.printf("Not a command: %s\n", cmd);
    }

    void printfn()
    {
    }

    void run_command(command_t command)
    {
        command.handler(*this);
    }

    constexpr unsigned int hash(const char *s, int off = 0)
    {
        return !s[off] ? 5381 : (hash(s, off + 1) * 33) ^ tolower(s[off]);
    }
};