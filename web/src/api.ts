import { cString, cStruct, float, uint32 } from "c-type-util";
import { url } from "./url";
import { getSourceType } from "./dataSourceTypes";

export interface I__INDEX {
    [key: string]: {
        name: string,
        description: string,
        time_base: number,
        start_time: number,
        location: {
            longitude: number,
            latitude: number,
        }
    }
}

export async function apiGetIndex(): Promise<I__INDEX> {
    console.log(`FETCH: ${url("/__INDEX")}`);
    const res = await fetch(url("/__INDEX"));
    const text = (await res.text()).replace(/,\s*$/g, "");
    return JSON.parse("{" + text + "}");
}

export function apiParseHeader(buf: ArrayBuffer) {
    const header_t = cStruct({
        header_version: uint32,
        cycle_time_base_ms: uint32,
        start_time_epoch: uint32,
        run_location: cStruct({
            longitude: float,
            latitude: float,
        }),
        name: cString(1024),
        description: cString(4096),
        checksum_intermediate: uint32,
        num_entries: uint32,
    });

    const data_source_t = cStruct({
        cycle_interval: uint32,
        name: cString(128),
        type_name: cString(128),
    })

    let header = header_t.readLE(buf);

    let entries = [];
    for (let i = 0; i < header.num_entries; i++) {
        entries[i] = data_source_t.readLE(buf, header_t.size + data_source_t.size * i);
    }

    let size = header_t.size + data_source_t.size * header.num_entries;

    return { ...header, entries, size } as typeof header & { entries: typeof entries, size: number };
}

export function apiLoadData(buf: ArrayBuffer, head) {
    buf = buf.slice(head.size);

    // Parse Entries
    let pe = head.entries.map(e => ({
        ...e,
        cycle: 0,
        source_type: getSourceType(e.type_name),
        data: []
    }));

    let idx = 0, c = 0;

    while (idx < buf.byteLength) {
        // Try to parse each entry, if its interval is up
        for (let i = 0; i < pe.length; i++) {
            const e = pe[i];

            if (--e.cycle > 0) continue;
            e.cycle = e.cycle_interval;
            e.data.push({
                x: c * head.cycle_time_base_ms,
                y: e.source_type.readLE(buf, idx)
            });

            idx += e.source_type.size;
        }
        c++;
    }

    return pe.map(p => ({
        name: p.name,
        data: p.data
    }));
}

function hash(buf: ArrayBuffer,len): number {
    buf = new Uint8Array(buf);
    let h = 5381;
    for(let i = len - 1; i >= 0; i--) {
        h = (h * 33 ^ buf[i]) & 0xFFFFFFFF;
    }
    return h;
}
