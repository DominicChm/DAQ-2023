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

export type tChartDefinition = {
    chartName: string,
    chartType: string,
    sources: {
        [key: string]: {
            color: string,
            name: string
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

function setupData(reference) {
    if (typeof reference == "object") {
        let dat = {};
        for (const k in reference) {
            dat[k] = setupData(reference[k]);
        }
        return dat;
    } else {
        return [];
    }
}

function mergeData(dat, newReading) {
    if (dat == null) {
        dat = setupData(newReading);
    }

    if (typeof newReading == "object") {
        for (const k in newReading) {
            dat[k] = mergeData(dat[k], newReading[k]);
        }
    } else {
        dat.push(newReading);
    }

    return dat;
}


export type tSparseArray = (number | null)[];
export type tLoadedApiData = tSparseArray | { [key: string]: tLoadedApiData };
export type tLoadedApiDataContainer = {
    __TIME: number[],
    [key: string]: tSparseArray,
}
export function apiLoadData(buf: ArrayBuffer, head): tLoadedApiDataContainer {
    buf = buf.slice(head.size);

    // Parse Entries
    let pe = head.entries.map(e => ({
        ...e,
        cycle: 0,
        source_type: getSourceType(e.type_name),
        data: null
    }));

    let idx = 0, c = 0;

    while (idx < buf.byteLength) {
        // Try to parse each entry, if its interval is up
        for (let i = 0; i < pe.length; i++) {
            const e = pe[i];

            if (--e.cycle > 0) {
                e.data.push(null)
                continue;
            }

            const nextDat = e.source_type.readLE(buf, idx);

            e.data = mergeData(e.data, nextDat);
            e.cycle = e.cycle_interval;
            idx += e.source_type.size;
        }
        c++;
    }

    const totalData = Object.fromEntries(pe.map(p => [p.name, p.data]));
    return {
        __TIME: new Array(c).fill(0).map((_, i) => i * head.cycle_time_base_ms),
        ...totalData,
    }
}

function hash(buf: ArrayBuffer, len): number {
    buf = new Uint8Array(buf);
    let h = 5381;
    for (let i = len - 1; i >= 0; i--) {
        h = (h * 33 ^ buf[i]) & 0xFFFFFFFF;
    }
    return h;
}