#include <Arduino.h>

class UI {
    struct command_t {
        std::function<void(UI &)> handler;
        uint32_t hash;
    };

    command_t *commands = (command_t *)malloc(10 * sizeof(command_t));
    size_t num_commands;
    char sbuf[256];
    int idx;

    Stream &_io;

   public:
    UI(Stream &io) : _io(io) {
    }

    void add_command(const char *command_string, std::function<void(UI &)> handler) {
        commands = (command_t *)realloc(commands, (num_commands + 1) * sizeof(command_t));

        if (commands == nullptr) {
            Serial.println("ERROR ALLOCATING COMMANDS");
            return;
        }

        memset(&commands[num_commands], 0, sizeof(command_t));
        commands[num_commands].hash = hash(command_string);
        commands[num_commands].handler = handler;
        num_commands++;
    }

    void poll() {
        if (!Serial.available())
            return;

        char c = Serial.read();

        // Ignore all carrige returns ( windows... >:( )
        if (c == '\r')
            return;
        else if (c == '\b' && idx > 0) {
            idx--;
            Serial.print("\b \b");
            return;
        } else if (c != '\n') {
            sbuf[idx++] = c;

            // Echo output
            printf("%c", c);
            return;
        }

        // replace newline with nullterm
        sbuf[idx] = '\0';
        idx = 0;

        printfn();
        process_stored_command();
        printf("$ ");
    }

    void process_stored_command() {
        char *cmd = strtok(sbuf, " ");

        if (cmd == nullptr)
            return;

        uint32_t cmd_hash = hash(cmd);

        for (int i = 0; i < num_commands; i++) {
            if (commands[i].hash == cmd_hash) {
                run_command(commands[i]);
                return;
            }
        }

        printfn("Not a command: %s", cmd);
    }

    void printfn() {
        Serial.println();
    }

    void printfn(const char *fmt, ...) {
        char buf[1024];
        va_list args;
        va_start(args, fmt);
        vsprintf(buf, fmt, args);
        va_end(args);

        Serial.print(buf);
        Serial.println();
    }

    void printf(const char *fmt, ...) {
        char buf[1024];
        va_list args;
        va_start(args, fmt);
        vsprintf(buf, fmt, args);
        va_end(args);

        Serial.print(buf);
    }

    void run_command(command_t command) {
        command.handler(*this);
    }

    constexpr unsigned int hash(const char *s, int off = 0) {
        return !s[off] ? 5381 : (hash(s, off + 1) * 33) ^ tolower(s[off]);
    }
};
