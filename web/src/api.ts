import { cStruct } from "c-type-util";
import { url } from "./url";

export async function apiGetIndex() {
    console.log(`FETCH: ${url("/__INDEX")}`);
    const res = await fetch(url("/__INDEX"));
    const text = (await res.text()).replace(/,\s*$/g, "");
    return JSON.parse("{" + text + "}");
}

export function parse_header(buf: ArrayBuffer) {
    cStruct({
        
    })
}