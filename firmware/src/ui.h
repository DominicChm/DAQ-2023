#include <Arduino.h>

class UI {
    struct command_t {
        std::function<void(UI &)> handler;
        uint32_t hash;
    };

    // Stores commands that can be run.
    command_t *_commands = nullptr;
    size_t _num_commands = 0;

    // Stores current serial input
    char _sbuf[256];
    int _sbuf_idx = 0;

    bool _parse_error = false;
    int _arg_pos = 0;

    Stream &_io;

   public:
    UI(Stream &io) : _io(io) {
    }

    void add_command(const char *command_string, std::function<void(UI &)> handler) {
        // Grow mem at commands pointer to add another command
        _commands = (command_t *)realloc(_commands, (_num_commands + 1) * sizeof(command_t));

        if (_commands == nullptr) {
            printfn("ERROR ALLOCATING COMMANDS");
            return;
        }
        // Init newly allocated memory to 0.
        // NECESSARY - assigning the std::function crashes if this is not done.
        memset(&_commands[_num_commands], 0, sizeof(command_t));

        // Assign the new command
        _commands[_num_commands].hash = hash(command_string);
        _commands[_num_commands].handler = handler;
        _num_commands++;
    }

    void poll() {
        if (!_io.available())
            return;

        char c = _io.read();

        // Ignore all carrige returns ( windows... >:( )
        if (c == '\r')
            return;
        else if (c == '\b' && _sbuf_idx > 0) {
            _sbuf_idx--;
            _io.print("\b \b");
            return;
        } else if (c != '\n') {
            _sbuf[_sbuf_idx++] = c;

            // Echo output
            printf("%c", c);
            return;
        }

        // replace newline with nullterm
        _sbuf[_sbuf_idx] = '\0';
        _sbuf_idx = 0;

        printfn();
        process_stored_command();
        printf("$ ");
    }

    void process_stored_command() {
        char *cmd = strtok(_sbuf, " ");

        if (cmd == nullptr)
            return;

        uint32_t cmd_hash = hash(cmd);

        for (int i = 0; i < _num_commands; i++) {
            if (_commands[i].hash == cmd_hash) {
                run_command(_commands[i]);
                return;
            }
        }

        printfn("Not a command: %s", cmd);
    }

    void printfn() {
        _io.println();
    }

    void printfn(const char *fmt, ...) {
        char buf[1024];
        va_list args;
        va_start(args, fmt);
        vsprintf(buf, fmt, args);
        va_end(args);

        _io.print(buf);
        _io.println();
    }

    void printf(const char *fmt, ...) {
        char buf[1024];
        va_list args;
        va_start(args, fmt);
        vsprintf(buf, fmt, args);
        va_end(args);

        _io.print(buf);
    }

    void run_command(command_t command) {
        _parse_error = false;
        _arg_pos = 0;
        command.handler(*this);
    }

    template <typename T>
    T positional() {
        T dest = {0};
        if (_parse_error) return dest;

        if (_parse_error |= parse_positional(&dest)) {
        }

        _arg_pos++;
        return dest;
    }

    // template <typename T>
    // T positional(T default_value) {

    // }

    bool parse_positional(int *dest) {
        char *tok = strtok(nullptr, " ");
        if (tok == nullptr) return true;

        *dest = atoi(tok);

        return false;
    }

    bool parse_positional(float *dest) {
        char *tok = strtok(nullptr, " ");
        if (tok == nullptr) return true;

        *dest = atoff(tok);

        return false;
    }

    bool parse_positional(double *dest) {
        char *tok = strtok(nullptr, " ");
        if (tok == nullptr) return true;

        *dest = atof(tok);

        return false;
    }

    bool parse_error() {
        if (_parse_error) {
            printfn("Parse Error: Couldn't parse arg at %d", _arg_pos);
        }
        return _parse_error;
    }

    /**
     * Hashes a string into an int at compile time.
     * Can be used in switch statements!
     */
    constexpr unsigned int hash(const char *s, int off = 0) {
        return !s[off] ? 5381 : (hash(s, off + 1) * 33) ^ tolower(s[off]);
    }
};
