import { derived, readable, writable } from "svelte/store";
import { apiGetIndex, apiLoadData, apiParseHeader } from "./api";
import { url as envUrl } from "./url";

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

function cancelableLoadingStore(urlStore) {
    const { subscribe, set, update } = writable<null | number | ArrayBuffer>(null);

    let controller: AbortController | null = null;
    let contentLen = 0;
    let rxLen = 0;

    async function load(url: string) {
        cancel();

        if (url == null) return;

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

            set(rxLen / contentLen);
        }

        // https://stackoverflow.com/questions/37228285/uint8array-to-arraybuffer
        set(buf.buffer.slice(buf.byteOffset, buf.byteLength + buf.byteOffset) as ArrayBuffer);
    }

    function cancel() {
        if (controller)
            controller.abort();

        reset();
    }

    function reset() {
        set(null);
        controller = null;
        rxLen = 0;
        contentLen = 0;
    }

    urlStore.subscribe(url => {
        load(url)
    });

    return {
        subscribe,
    };
}

export const polledRuns = polledReadable(apiGetIndex);
export const polledRunEntries = derived([polledRuns], ([$polledRuns]) => $polledRuns != null ? Object.entries($polledRuns) : [], []);

export const selectedRun = writable<null | string>(null);
export const selectedRunData = cancelableLoadingStore(selectedRun);

export const selectedRunHeader = derived(selectedRunData, ($selectedRunData) => {
    if (!($selectedRunData instanceof ArrayBuffer)) return null;
    return apiParseHeader($selectedRunData);
});

export const selectedRunLoadedData = derived([selectedRunData, selectedRunHeader], ([$selectedRunData, $selectedRunHeader]) => {
    if (!$selectedRunHeader) return null;
    if (!($selectedRunData instanceof ArrayBuffer)) return null;

    try {
        console.log("Loading");
        return apiLoadData($selectedRunData as ArrayBuffer, $selectedRunHeader);
    } catch (e) {
        console.warn("ERROR LOADING DATA!");
        console.error(e);
        return null;
    }
});

export const isLoaded = derived(selectedRunHeader, $s => $s != null);