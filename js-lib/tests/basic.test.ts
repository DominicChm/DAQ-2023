import { expect, test } from "vitest";
import { Adapter } from "../src/dlflib";
import { readFile } from "node:fs/promises"
import { resolve } from "node:path"
import { Parser } from "binary-parser"
import { FSAdapter } from "../src/fsadapter"
export function readRes<T>(value: T, size: number) {
    return [value, size];
}

export function writeRes<T>(buffer: T, size: number) {
    return [buffer, size];
}



const a = new FSAdapter("tests/resources/basic");

test("Basic meta", async () => {
    expect(await a.meta_header()).toMatchObject({
        magic: 33812,
        tick_base_us: 100000,
        meta_size: 16,
        meta_id: "constexpr const char* t() [with T = setup()::lele]",
        meta_structure: "meta_struct;time:uint32_t:0;another:uint64_t:8",
    });

    expect(await a.meta()).toMatchObject({
        another: 5n,
        time: 0
    });
});

test("polled headers", async () => {
    expect(await a.polled_header()).toMatchObject({
        "magic": 33812,
        "num_streams": 1,
        "stream_type": 0,
        "streams": [
            {
                "id": "led_poll",
                "notes": "Notes...",
                "stream_info": {
                    "tick_interval": 10,
                    "tick_phase": 0,
                },
                "type_id": "constexpr const char* t() [with T = bool]",
                "type_size": 1,
                "type_structure": "bool",
            },
        ],
        "tick_span": 67n,
    });
});


test("event headers", async () => {
    expect(await a.events_header()).toMatchObject({
        "magic": 33812,
        "num_streams": 3,
        "stream_type": 1,
        "streams": [
            {
                "id": "led_event",
                "notes": "Notes...",
                "stream_info": {},
                "type_id": "constexpr const char* t() [with T = bool]",
                "type_size": 1,
                "type_structure": "bool",
            },
            {
                "id": "start_time",
                "notes": "Notes...",
                "stream_info": {},
                "type_id": "constexpr const char* t() [with T = long int]",
                "type_size": 4,
                "type_structure": "uint32_t",
            },
            {
                "id": "Test struct",
                "notes": "Notes...",
                "stream_info": {},
                "type_id": "constexpr const char* t() [with T = a_struct]",
                "type_size": 2,
                "type_structure": "a_struct;a1:uint8_t:0;a2:uint8_t:1",
            },
        ],
        "tick_span": 67n,
    });
});

test("Events data", async () => {
    console.log(await a.events_data())
})