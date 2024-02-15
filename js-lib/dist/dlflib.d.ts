import { cType, cTypeReadResult } from "lightstruct";
import { dlf_meta_header_jst } from "./dlfTypes";
/**
 * Creates an adapter to a remote, hosted, DLF Logfile
 */
export declare class LogClient {
    constructor(adapter: Adapter);
}
export declare abstract class Adapter {
    _type_parsers: any;
    abstract get polled_dlf(): Promise<ArrayBuffer>;
    abstract get events_dlf(): Promise<ArrayBuffer>;
    abstract get meta_dlf(): Promise<ArrayBuffer>;
    constructor(type_parsers: any);
    /** From metafile **/
    meta_header(): Promise<cTypeReadResult<dlf_meta_header_jst>>;
    meta<T>(meta_parsers: {
        [key: string]: cType<any>;
    }): Promise<cTypeReadResult<T> | null>;
    polled_header(): Promise<cTypeReadResult<{
        magic: number;
        stream_type: number;
        tick_span: bigint;
        num_streams: number;
        streams: {
            type_id: any;
            id: any;
            notes: any;
            type_size: any;
            tick_interval: any;
            tick_phase: any;
        }[];
    }>>;
    events_header(): Promise<cTypeReadResult<{
        magic: number;
        stream_type: number;
        tick_span: bigint;
        num_streams: number;
        streams: {
            type_id: any;
            id: any;
            notes: any;
            type_size: any;
        }[];
    }>>;
    events_data(): Promise<void>;
    polled_data(downsample?: bigint): Promise<void>;
}
/**
 * Todo.
 * Takes a DLF run archive, unzips it, and holds it in memory for reading.
 */
/**
 * interacts with an unarchived run at a URL
 */
export declare class HTTPAdapter extends Adapter {
    _baseUrl: string;
    constructor(type_parsers: any, url: string);
    get polled_dlf(): Promise<ArrayBuffer>;
    get events_dlf(): Promise<ArrayBuffer>;
    get meta_dlf(): Promise<ArrayBuffer>;
}
