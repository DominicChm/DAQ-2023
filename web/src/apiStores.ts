import { derived, readable, writable, type Writable } from "svelte/store";
import { apiGetIndex, apiLoadData, apiParseHeader, type tLoadedApiDataContainer } from "./api";
import { url as envUrl } from "./url";
import dot from "dot-object"

// https://javascript.info/fetch-progress
// https://javascript.info/fetch-abort#:~:text=To%20be%20able%20to%20cancel,how%20to%20work%20with%20AbortController%20.

const POLL_INTERVAL = 1000;
function polledReadable<T>(fn: () => Promise<T>) {
    return readable<null | T>(null, set => {
        const update = async () => set(await fn());

        update(); // Perform an immediate update.

        // Poll for updates.
        const interval = setInterval(update, POLL_INTERVAL)
        return () => clearInterval(interval)
    });
};

export function cancelableLoadingStore(urlStore: Writable<null | string>, progressStore: Writable<number>): Writable<tLoadedApiDataContainer> {
    const { subscribe, set, update } = writable<null | tLoadedApiDataContainer>(null);

    let controller: AbortController | null = null;
    let contentLen = 0;
    let rxLen = 0;

    async function load(url: string) {
        cancel();

        if (url == null) return;

        try {
            controller = new AbortController();

            const res = await fetch(envUrl(url), { signal: controller.signal, method: "get" });
            const reader = res.body.getReader();

            contentLen = +res.headers.get("Content-Length");

            const buf = new Uint8Array(contentLen);

            while (true) {
                const { done, value } = await reader.read();

                if (done) {
                    break;
                }

                buf.set(value, rxLen);
                rxLen += value.length;

                progressStore.set(rxLen / contentLen);
            }

            // https://stackoverflow.com/questions/37228285/uint8array-to-arraybuffer
            let dataBuf = buf.buffer.slice(buf.byteOffset, buf.byteLength + buf.byteOffset) as ArrayBuffer;

            let header = apiParseHeader(dataBuf);
            let data = apiLoadData(dataBuf, header);
            dot.keepArray = true;
            let dotNotationData = dot.dot(data);
            set(dotNotationData);
        } catch (e) {
            cancel();
            progressStore.set(null);
        }
    }

    function cancel() {
        if (controller)
            controller.abort();

        reset();
    }

    function reset() {
        set(null);
        progressStore.set(0);
        controller = null;
        rxLen = 0;
        contentLen = 0;
    }

    urlStore.subscribe(url => {
        load(url)
    });

    return {
        subscribe,
        set,
        update
    };
}

export const polledRuns = polledReadable(apiGetIndex);
export const polledRunEntries = derived([polledRuns], ([$polledRuns]) => $polledRuns != null ? Object.entries($polledRuns) : [], []);
