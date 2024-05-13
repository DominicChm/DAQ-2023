import { Parser } from "binary-parser"

/**
 * Creates an adapter to a remote, hosted, DLF Logfile 
 */
export class LogClient {
    constructor(adapter: Adapter) {
    }
}

let binary_parsers_primitives = {
    "uint8_t": "uint8",
    "bool": "uint8",
    "uint16_t": "uint16le",
    "uint32_t": "uint32le",
    "uint64_t": "uint64le",
    "int8_t": "int8",
    "int16_t": "int16le",
    "int32_t": "int32le",
    "int64_t": "int64le",
    "float": "floatle",
    "double": "doublele",
}

const meta_header_t = new Parser()
    .endianness("little")
    .uint16("magic")
    .uint32("tick_base_us")
    .string("meta_id", { zeroTerminated: true })
    .string("meta_structure", { zeroTerminated: true })
    .uint32("meta_size")
    .buffer("meta", { readUntil: "eof" });


type Tlogfile_header_t = {
    magic: number,
    stream_type: number,
    tick_span: BigInt,
    num_streams: number,
    streams: {
        type_id: string,
        type_structure: string,
        id: string,
        notes: string,
        type_size: number,
        stream_info: {
            tick_interval: number,
            tick_phase: number,
        } | {}
    }[],
    data: Uint8Array
}

const logfile_header_t = new Parser()
    // @ts-ignore
    .useContextVars()
    .endianness("little")
    .uint16("magic")
    .uint8("stream_type")
    .uint64("tick_span")
    .uint16("num_streams")
    .array("streams", {
        length: "num_streams",
        type: new Parser()
            .string("type_id", { zeroTerminated: true })
            .string("type_structure", { zeroTerminated: true })
            .string("id", { zeroTerminated: true })
            .string("notes", { zeroTerminated: true })
            .uint32le("type_size")
            .choice("stream_info", {
                // @ts-ignore
                tag: function () { return this.$root.stream_type },
                choices: {
                    0: new Parser()
                        .uint32le("tick_interval")
                        .uint32le("tick_phase"), // polled
                    1: new Parser(), // event
                }
            })
    })
    .buffer("data", { readUntil: "eof" })


export abstract class Adapter {
    abstract get polled_dlf(): Promise<Uint8Array>;
    abstract get events_dlf(): Promise<Uint8Array>;
    abstract get meta_dlf(): Promise<Uint8Array>;

    create_parser(structure: string, structure_size?: number): Parser {
        // No contained structure
        if (structure.startsWith("!"))
            return null

        if (binary_parsers_primitives[structure]) {
            return binary_parsers_primitives[structure]
        }

        // Create parser
        // name;member_1:primitive_type:offset;...
        const [name, ...members] = structure.split(";");


        let member_parser = new Parser()
            .endianness("little")
            .saveOffset("_____off")

        for (const m of members) {
            const [name, type_name, offset] = m.split(":");

            const relOff = parseInt(offset)
            const bin_parse_type = binary_parsers_primitives[type_name];

            console.log("Member parser", name, bin_parse_type, relOff);

            member_parser = member_parser
                .pointer(name, {
                    type: bin_parse_type, offset: function () {
                        return this.off + relOff
                    }
                });


            // member_parser = member_parser.pointer(name, { type: bin_parse_type, offset: off });
        }

        if (structure_size != null) {
            member_parser = member_parser.seek(structure_size)
        }

        return member_parser
    }

    /** From metafile **/
    async meta_header() {
        return meta_header_t.parse(await this.meta_dlf)
    }

    async meta() {
        const mh = await this.meta_header();
        const meta_structure = mh.meta_structure;
        const metadata = mh.meta;

        const parser = this.create_parser(meta_structure, mh.meta_size, null);

        if (!parser) return null;

        return parser.parse(metadata)
    }

    async polled_header(): Promise<Tlogfile_header_t> {
        let polledDataFile = await this.polled_dlf
        return logfile_header_t.parse(polledDataFile)
    }

    async events_header(): Promise<Tlogfile_header_t> {
        let eventDataFile = await this.events_dlf;
        return logfile_header_t.parse(eventDataFile)
    }

    async events_data() {
        let header = await this.events_header()

        // Create choices
        const choices = {};
        for (const [i, stream] of header.streams.entries()) {
            choices[i] = this.create_parser(stream.type_structure, stream.type_size, 10);
        }


        const file_parser = new Parser()
            .array("data", {
                readUntil: "eof",
                type: new Parser()
                    .uint16le("stream_idx")
                    .uint64le("sample_tick")
                    .choice("data", {
                        tag: "stream_idx",
                        choices
                    })
            });

        const { data } = file_parser.parse(header.data);

        const merged_data = data.map(({ stream_idx, sample_tick, data }) => ({
            stream: header.streams[stream_idx],
            stream_idx,
            tick: sample_tick,
            data,
        }));

        return merged_data
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