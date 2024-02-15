import { jsType } from "lightstruct";
/************ polled.dlf ************/
export declare const polled_stream_header_t: import("lightstruct").cType<{
    type_id: string;
    id: string;
    notes: string;
    type_size: number;
    tick_interval: bigint;
    tick_phase: bigint;
}>;
export type polled_stream_header_jst = jsType<typeof polled_stream_header_t>;
export declare const polled_logfile_header_t: import("lightstruct").cType<{
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
}>;
export type polled_logfile_header_jst = jsType<typeof polled_logfile_header_t>;
/************ events.dlf ************/
export declare const event_stream_header_t: import("lightstruct").cType<{
    type_id: string;
    id: string;
    notes: string;
    type_size: number;
}>;
export type event_stream_header_jst = jsType<typeof event_stream_header_t>;
export declare const event_logfile_header_t: import("lightstruct").cType<{
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
}>;
export type event_logfile_header_jst = jsType<typeof event_logfile_header_t>;
export declare const dlf_event_stream_sample_t: import("lightstruct").cType<{
    stream: number;
    sample_tick: bigint;
}>;
/********* meta.dlf *********/
export declare const dlf_meta_header_t: import("lightstruct").cType<{
    magic: number;
    tick_base_us: number;
    application: string;
    meta_size: number;
}>;
export type dlf_meta_header_jst = jsType<typeof dlf_meta_header_t>;
