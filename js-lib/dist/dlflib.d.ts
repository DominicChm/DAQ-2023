import { cType, readResult } from "lightstruct";
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
    meta_header(): Promise<readResult<dlf_meta_header_jst>>;
    meta<T>(meta_parsers: {
        [key: string]: cType<any>;
    }): Promise<readResult<T> | null>;
    polled_header(): Promise<readResult<{
        streams: import("lightstruct").OmitReferences<{
            tick_interval: any;
            tick_phase: any;
        } & {
            type_id: any;
            id: any;
            notes: any;
            type_size: any;
        }>[];
    } & {
        magic: number;
        stream_type: number;
        tick_span: bigint;
        num_streams: import("lightstruct").cTypeLengthReference;
    }>>;
    events_header(): Promise<readResult<{
        streams: import("lightstruct").OmitReferences<{
            type_id: any;
            id: any;
            notes: any;
            type_size: any;
        }>[];
    } & {
        magic: number;
        stream_type: number;
        tick_span: bigint;
        num_streams: import("lightstruct").cTypeLengthReference;
    }>>;
    events_data(): Promise<{
        [k: string]: any[];
    }>;
    polled_data(downsample?: bigint): Promise<{
        [k: string]: any[];
    }>;
    data(): Promise<{
        [k: string]: any[];
    }>;
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
