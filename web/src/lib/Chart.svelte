<script lang="ts">
    import { get, type Readable } from "svelte/store";
    import uPlot from "uplot";
    import { mapDotNotations } from "../util";
    import { onDestroy, onMount } from "svelte";
    import type { tLoadedApiDataContainer } from "../api";
    import { getVar } from "../util";
    
    export let showLegend = true;
    export let dataStore: Readable<tLoadedApiDataContainer>;
    export let cursorSync: uPlot.SyncPubSub;
    export let sources;
    export let legendContainer = null;

    export let gridColor = getVar("--b1");

    let chartContainer;
    let plt: uPlot | null = null;
    let unsubPlt;

    let clientH, clientW;

    $: {
        plt?.setSize({
            height: clientH || 100,
            width: clientW || 100,
        });
    }

    function getSeries(sources) {
        let dataSeries = Object.entries(sources).map(([k, v]: [any, any]) => ({ label: k, stroke: v.color, spanGaps: true }));
        return [{ label: "x" }, ...dataSeries];
    }

    function initPlot(sources, chartContainer, legendContainer, cursorSync) {
        if (sources == null || chartContainer == null) return;

        if (plt) plt.destroy();
        if (unsubPlt) unsubPlt();

        let sourceData = get(dataStore);

        const data = [sourceData.__TIME, ...mapDotNotations(sourceData, Object.keys(sources))];
        //console.log(data, sources, chartContainer, legendContainer, cursorSync);

        const matchSyncKeys = (own, ext) => own == ext;
        plt = new uPlot(
            {
                cursor: {
                    sync: {
                        key: cursorSync.key,
                        setSeries: true,
                        match: [matchSyncKeys, matchSyncKeys],
                    },
                    points: {
                        size: 7,
                    },
                },
                focus: {
                    alpha: 0.3,
                },
                width: clientW,
                height: clientH,
                scales: { x: { time: false } },
                axes: [
                    {
                        stroke: getVar("--bc"),
                        grid: { stroke: gridColor },
                    },
                    {
                        stroke: getVar("--bc"),
                        grid: { stroke: gridColor },
                    },
                ],
                legend: {
                    mount(up, el) {
                        if (legendContainer) legendContainer.appendChild(el);
                    },
                    show: showLegend,
                },
                series: getSeries(sources),
            },
            data,
            chartContainer
        );
        cursorSync.sub(plt);

        // Update the graph when new data comes in.
        // Note: DON'T use the incoming data object. The
        // captured one will be updated because it holds
        // references to the data arrays. Just need to make the
        // plot refresh.
        unsubPlt = dataStore.subscribe(() => {
            plt.setData(data);
        });
    }

    $: initPlot(sources, chartContainer, legendContainer, cursorSync);

    onDestroy(() => {
        plt?.destroy();
        if (unsubPlt) unsubPlt();
    });
</script>

<div class="flex-1 box-border overflow-hidden" bind:this={chartContainer} bind:clientHeight={clientH} bind:clientWidth={clientW} on:pointerdown|stopPropagation />
