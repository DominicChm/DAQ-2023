import { cType, readResult } from "lightstruct"
import { dlf_event_stream_sample_t, dlf_meta_header_jst, dlf_meta_header_t, event_logfile_header_t, polled_logfile_header_t } from "./dlfTypes"

/**
 * Creates an adapter to a remote, hosted, DLF Logfile 
 */
export class LogClient {
    constructor(adapter: Adapter) {

    }
}

class EventInterface {

}

class PolledInterface {

}

export abstract class Adapter {
    _type_parsers;

    abstract get polled_dlf(): Promise<ArrayBuffer>;
    abstract get events_dlf(): Promise<ArrayBuffer>;
    abstract get meta_dlf(): Promise<ArrayBuffer>;

    constructor(type_parsers) {
        this._type_parsers = type_parsers;
    }

    /** From metafile **/
    async meta_header(): Promise<readResult<dlf_meta_header_jst>> {
        return dlf_meta_header_t.read(await this.meta_dlf)
    }

    async meta<T>(meta_parsers: { [key: string]: cType<any> }): Promise<readResult<T> | null> {
        const [metaHeader, metaHeaderSize] = await this.meta_header()

        // Select user-defined metadata parser based on application string
        const parser = meta_parsers[metaHeader.application];

        if (!parser) return null;

        return parser.read(await this.meta_dlf, dlf_meta_header_t.minSize)
    }

    async polled_header() {
        let polledDataFile = await this.polled_dlf
        return polled_logfile_header_t.read(polledDataFile)
    }

    async events_header() {
        let eventDataFile = await this.events_dlf;
        return event_logfile_header_t.read(eventDataFile)
    }

    async events_data() {
        let [header, header_len] = await this.events_header()

        if (header == null)
            throw new Error(header_len as string);

        let totalData = Object.fromEntries(header.streams.map(v => [v.id, []]));

        const dataFile = await this.events_dlf;
        let dataFileLen = dataFile.byteLength;

        for (let i = header_len as number; i < dataFileLen;) {
            const [sampleHeader, sampleHeaderLen] = dlf_event_stream_sample_t.read(dataFile, i);

            if (!sampleHeader)
                throw new Error(sampleHeaderLen as string);

            i += sampleHeaderLen as number;

            // Select appropriate stream header based on sample's stream field
            const sampleStreamHeader = header.streams[sampleHeader.stream];

            const dataParser = this._type_parsers[sampleStreamHeader.type_id];
            if (dataParser) {
                const [data, dataLen] = dataParser.read(dataFile, i);
                totalData[sampleStreamHeader.id].push({ tick: sampleHeader.sample_tick, data });
                i += dataLen;
            } else {
                i += sampleStreamHeader.type_size;
            }
        }

        return totalData
    }

    async polled_data(downsample = 1n) {
        let [header, headerLen] = (await this.polled_header())

        let data = Object.fromEntries(header.streams.map(v => [v.id, []]));

        const abuf = await this.polled_dlf;
        let blen = abuf.byteLength as number;

        for (let i = headerLen as number, t = 0n; i < blen; t += downsample) {
            for (const stream of header.streams) {
                if ((t + stream.tick_phase) % stream.tick_interval != 0n)
                    continue;

                let tParser = this._type_parsers[stream.type_id];
                if (tParser) {
                    let [value, size] = tParser.read(abuf, i);
                    data[stream.id].push({ tick: t, data: value });

                } else {
                    console.warn("Unknown datatype found")
                }
                i += stream.type_size;
            }
        }
        return data
    }

    async data() {
        return Object.assign(
            {},
            await this.polled_data(),
            await this.events_data(),
        )
    }
}

/**
 * Todo.
 * Takes a DLF run archive, unzips it, and holds it in memory for reading.
 */
// export class ArchiveAdapter extends Adapter {

// }

/**
 * interacts with an unarchived run at a URL
 */
export class HTTPAdapter extends Adapter {
    _baseUrl: string;
    constructor(type_parsers, url: string) {
        super(type_parsers);
        this._baseUrl = url;
    }

    get polled_dlf() {
        return fetch(this._baseUrl + "/polled.dlf").then(r => r.arrayBuffer())
    }

    get events_dlf() {
        return fetch(this._baseUrl + "/event.dlf").then(r => r.arrayBuffer())
    }

    get meta_dlf() {
        return fetch(this._baseUrl + "/meta.dlf").then(r => r.arrayBuffer())
    }
}