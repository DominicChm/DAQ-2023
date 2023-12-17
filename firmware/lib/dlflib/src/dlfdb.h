#include <FS.h>

/**
 * Manages append-only JSONL files for the various JSON endpoints
 *
 * DLFLib JSON files are append-only at runtime to keep performance high.
 * They are structured as one JSON object per file, to take advantage of JS's built-in
 * key deduplication.
 *
 * This allows modification, deletion, and addition operations to be created using appends
 * without custom code JS-side.
 *
 * IE to delete "test" from {test: 1}
 * a null assignment will be appended, so the object loaded in JS would look like
 * {test: 1, test: null}. JS only keeps the last value so this effectively deletes test.
 *
 * DLFDBs are stored without opening and closing parentheses. These must be appended when loading
 * JS-side.
 */
class DLFDb {
    FS &fs;
    DLFDb(const char *file, FS &fs);

    bool open();
    bool close();
    bool reset();
    bool optimize();
};