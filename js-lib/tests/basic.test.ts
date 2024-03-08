import { _minSize, debugType, int32, struct, uint8 } from "lightstruct";
import { expect, test } from "vitest";
import { polled_logfile_header_t, polled_stream_header_t } from "../src/dlfTypes";
import { Adapter } from "../src/dlflib";
import { readFile } from "node:fs/promises"
import { resolve } from "node:path"

export function readRes<T>(value: T, size: number) {
    return [value, size];
}

export function writeRes<T>(buffer: T, size: number) {
    return [buffer, size];
}

/**
 * Reads an unpacked run from disk. Useful for testing.
 */
class FSAdapter extends Adapter {

    _rootDir: string;

    constructor(type_parsers, rootDir: string) {
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

test("Basicist meta", async () => {
    const a = new FSAdapter(typeParsers, "tests/resources/basicist_run");

    expect(await a.meta_header()).toMatchObject([
        {
            magic: 33812,
            tick_base_us: 10000,
            application: 'TESTAPP',
            meta_size: 4
        },
        42
    ]);

    expect(await a.meta(metaParsers)).toMatchObject([
        {
            val: 1
        },
        4,
    ]);
});

test("data headers", async () => {
    const a = new FSAdapter(typeParsers, "tests/resources/basicist_run");

    console.log(await a.polled_header())

    expect(await a.polled_header()).toMatchObject([
        {
            magic: 33812,
            stream_type: 0,
            tick_span: 1048n,
            num_streams: 2,
            streams: [
                {
                    type_id: 'constexpr const char* t() [with T = polled_data1]',
                    id: 'Polled 1',
                    notes: 'NOTES....',
                    tick_interval: 100n,
                    tick_phase: 0n,
                    type_size: 5,
                },
                {
                    type_id: 'constexpr const char* t() [with T = polled_data2]',
                    id: 'Polled 2',
                    notes: 'NOTES....',
                    tick_interval: 100n,
                    tick_phase: 0n,
                    type_size: 6,
                }
            ]
        },
        629]);

    expect(await a.events_header()).toMatchObject([
        {
            magic: 33812,
            stream_type: 1,
            tick_span: 1048n,
            num_streams: 2,
            streams: [
                {
                    type_id: 'constexpr const char* t() [with T = unsigned char]',
                    id: 'Event 2',
                    notes: 'NOTES....',
                    type_size: 1,
                },
                {
                    type_id: 'constexpr const char* t() [with T = unsigned char]',
                    id: 'Event 3',
                    notes: 'NOTES....',
                    type_size: 1,
                }
            ]
        },
        597,
    ]);


    console.log(await a.events_data())
    console.log(await a.data())

});