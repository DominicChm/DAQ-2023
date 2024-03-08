const express = require("express")
const { struct, uint8 } = require("lightstruct");

const metaParsers = {
    "TESTAPP": struct({
        val: int32
    })
}

const typeParsers = {
    "constexpr const char* t() [with T = polled_data1]": struct({
        i1: uint8,
        i2: uint8,
        i3: uint8,
        i4: uint8,
        i5: uint8
    }),
    "constexpr const char* t() [with T = polled_data2]": struct({
        i1: uint8,
        i2: uint8,
        i3: uint8,
        i4: uint8,
        i5: uint8,
        i6: uint8,
    }),
    'constexpr const char* t() [with T = unsigned char]': uint8,
}

/**
 * Reads an unpacked run from disk. Useful for testing.
 */
class FSAdapter extends Adapter {
    constructor(type_parsers, rootDir) {
        super(type_parsers);
        this._rootDir = rootDir;
    }

    get polled_dlf() {
        return readFile(resolve(this._rootDir, "polled.dlf")).then(v => v.buffer)
    }

    get events_dlf() {
        return readFile(resolve(this._rootDir, "event.dlf")).then(v => v.buffer)
    }

    get meta_dlf() {
        return readFile(resolve(this._rootDir, "meta.dlf")).then(v => v.buffer)
    }
}


