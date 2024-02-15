"use strict";
Object.defineProperty(exports, "__esModule", { value: true });
exports.HTTPAdapter = exports.Adapter = exports.LogClient = void 0;
const lightstruct_1 = require("lightstruct");
const dlfTypes_1 = require("./dlfTypes");
/**
 * Creates an adapter to a remote, hosted, DLF Logfile
 */
class LogClient {
    constructor(adapter) {
    }
}
exports.LogClient = LogClient;
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
        const parser = meta_parsers[metaHeader.application];
        if (!parser)
            return null;
        return parser.read(await this.meta_dlf, dlfTypes_1.dlf_meta_header_t[lightstruct_1._minSize]());
    }
    async polled_header() {
        return dlfTypes_1.polled_logfile_header_t.read(await this.polled_dlf);
    }
    async events_header() {
        return dlfTypes_1.event_logfile_header_t.read(await this.events_dlf);
    }
    async events_data() {
        let [header, header_len] = await this.events_header();
        const abuf = await this.events_dlf;
        let blen = abuf.byteLength;
        for (let i = header_len; i < blen;) {
            const [sampleHeader, sampleHeaderLen] = dlfTypes_1.dlf_event_stream_sample_t.read(abuf, i);
            i += sampleHeaderLen;
            const streamHeader = header.streams[sampleHeader.stream];
            const dataParser = this._type_parsers[streamHeader.type_id];
            const [data, dataLen] = dataParser.read(abuf, i);
            i += dataLen;
            console.log(sampleHeader, data);
        }
    }
    async polled_data(downsample = 1n) {
        let [header, headerLen] = (await this.polled_header());
        const abuf = await this.polled_dlf;
        let blen = abuf.byteLength;
        for (let i = headerLen, t = 0n; i < blen; t += downsample) {
            for (const stream of header.streams) {
                if ((t + stream.tick_phase) % stream.tick_interval != 0n)
                    continue;
                let tParser = this._type_parsers[stream.type_id];
                if (tParser) {
                    let [value, size] = tParser.read();
                }
                else {
                    console.warn("Unknown datatype found");
                }
                i += stream.type_size;
            }
        }
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
