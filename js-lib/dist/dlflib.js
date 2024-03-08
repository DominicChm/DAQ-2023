"use strict";
Object.defineProperty(exports, "__esModule", { value: true });
exports.HTTPAdapter = exports.Adapter = exports.LogClient = void 0;
const dlfTypes_1 = require("./dlfTypes");
/**
 * Creates an adapter to a remote, hosted, DLF Logfile
 */
class LogClient {
    constructor(adapter) {
    }
}
exports.LogClient = LogClient;
class EventInterface {
}
class PolledInterface {
}
class Adapter {
    constructor(type_parsers) {
        this._type_parsers = type_parsers;
    }
    /** From metafile **/
    async meta_header() {
        return dlfTypes_1.dlf_meta_header_t.read(await this.meta_dlf);
    }
    async meta(meta_parsers) {
        const [metaHeader, metaHeaderSize] = await this.meta_header();
        // Select user-defined metadata parser based on application string
        const parser = meta_parsers[metaHeader.application];
        if (!parser)
            return null;
        return parser.read(await this.meta_dlf, dlfTypes_1.dlf_meta_header_t.minSize);
    }
    async polled_header() {
        let polledDataFile = await this.polled_dlf;
        return dlfTypes_1.polled_logfile_header_t.read(polledDataFile);
    }
    async events_header() {
        let eventDataFile = await this.events_dlf;
        return dlfTypes_1.event_logfile_header_t.read(eventDataFile);
    }
    async events_data() {
        let [header, header_len] = await this.events_header();
        if (header == null)
            throw new Error(header_len);
        let totalData = Object.fromEntries(header.streams.map(v => [v.id, []]));
        const dataFile = await this.events_dlf;
        let dataFileLen = dataFile.byteLength;
        for (let i = header_len; i < dataFileLen;) {
            const [sampleHeader, sampleHeaderLen] = dlfTypes_1.dlf_event_stream_sample_t.read(dataFile, i);
            if (!sampleHeader)
                throw new Error(sampleHeaderLen);
            i += sampleHeaderLen;
            // Select appropriate stream header based on sample's stream field
            const sampleStreamHeader = header.streams[sampleHeader.stream];
            const dataParser = this._type_parsers[sampleStreamHeader.type_id];
            if (dataParser) {
                const [data, dataLen] = dataParser.read(dataFile, i);
                totalData[sampleStreamHeader.id].push({ tick: sampleHeader.sample_tick, data });
                i += dataLen;
            }
            else {
                i += sampleStreamHeader.type_size;
            }
        }
        return totalData;
    }
    async polled_data(downsample = 1n) {
        let [header, headerLen] = (await this.polled_header());
        let data = Object.fromEntries(header.streams.map(v => [v.id, []]));
        const abuf = await this.polled_dlf;
        let blen = abuf.byteLength;
        for (let i = headerLen, t = 0n; i < blen; t += downsample) {
            for (const stream of header.streams) {
                if ((t + stream.tick_phase) % stream.tick_interval != 0n)
                    continue;
                let tParser = this._type_parsers[stream.type_id];
                if (tParser) {
                    let [value, size] = tParser.read(abuf, i);
                    data[stream.id].push({ tick: t, data: value });
                }
                else {
                    console.warn("Unknown datatype found");
                }
                i += stream.type_size;
            }
        }
        return data;
    }
    async data() {
        return Object.assign({}, await this.polled_data(), await this.events_data());
    }
}
exports.Adapter = Adapter;
/**
 * Todo.
 * Takes a DLF run archive, unzips it, and holds it in memory for reading.
 */
// export class ArchiveAdapter extends Adapter {
// }
/**
 * interacts with an unarchived run at a URL
 */
class HTTPAdapter extends Adapter {
    constructor(type_parsers, url) {
        super(type_parsers);
        this._baseUrl = url;
    }
    get polled_dlf() {
        return fetch(this._baseUrl + "/polled.dlf").then(r => r.arrayBuffer());
    }
    get events_dlf() {
        return fetch(this._baseUrl + "/event.dlf").then(r => r.arrayBuffer());
    }
    get meta_dlf() {
        return fetch(this._baseUrl + "/meta.dlf").then(r => r.arrayBuffer());
    }
}
exports.HTTPAdapter = HTTPAdapter;
