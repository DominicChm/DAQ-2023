<script lang="ts">
    import { onDestroy, onMount } from "svelte";
    import type { Readable } from "svelte/store";
    import uplot from "uplot";
    import type { tLoadedApiData, tLoadedApiDataContainer } from "../api";
    import { mapDotNotations } from "../util";

    export let dataStore: Readable<tLoadedApiDataContainer>;
    export let cursorSync: uplot.SyncPubSub;
    export let dataItem;

    let testDataDef = {
        sources: {
            data1: {
                color: "red",
            },
            data2: {
                color: "orange",
            },
        },
        chartName: "lel",
        chartType: "line",
    };

    function getSeries(sources) {
        let dataSeries = Object.entries(sources).map(([k, v]: [any, any]) => ({ label: k, stroke: v.color, spanGaps: true }));
        return [{ label: "x" }, ...dataSeries];
    }

    let chartContainer;
    let legendContainer;
    let plt = null;

    let clientH, clientW;

    // Updates graph sizes when their container changes
    $: {
        plt?.setSize({
            height: clientH || 100,
            width: clientW || 100,
        });
    }

    function getVar(v) {
        return "hsl(" + getComputedStyle(document.documentElement).getPropertyValue(v) + ")";
    }

    // https://www.30secondsofcode.org/js/s/lcm/
    const lcm = (...arr) => {
        const gcd = (x, y) => (!y ? x : gcd(y, x % y));
        const _lcm = (x, y) => (x * y) / gcd(x, y);
        return [...arr].reduce((a, b) => _lcm(a, b));
    };

    onDestroy(() => {
        plt?.destroy();
    });

    onMount(() => {
        dataStore.subscribe((sources) => {
            if (plt) plt.destroy();

            if (sources == null) return;

            const data = [sources.__TIME, ...mapDotNotations(sources, Object.keys(testDataDef.sources))];

            const matchSyncKeys = (own, ext) => own == ext;
            plt = new uplot(
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
                    width: 300,
                    height: 300,
                    scales: { x: { time: false } },
                    axes: [
                        {
                            stroke: getVar("--bc"),
                            grid: { stroke: getVar("--b1") },
                        },
                        {
                            stroke: getVar("--bc"),
                            grid: { stroke: getVar("--b1") },
                        },
                    ],
                    legend: {
                        mount(up, el) {
                            if (legendContainer) legendContainer.appendChild(el);
                        },
                        show: true,
                    },
                    series: getSeries(testDataDef.sources),
                },
                data,
                chartContainer
            );
            cursorSync.sub(plt);
        });
    });
</script>

<div class="w-full h-full flex flex-col overflow-hidden">
    <div class="flex justify-between prose w-auto max-w-none max-h-14">
        <h3 class="bg-neutral block px-3 m-0 py-2 rounded-br-2xl cursor-move select-none" >{testDataDef.chartName}</h3>
        <div on:pointerdown|stopPropagation bind:this={legendContainer} />
    </div>
    <div class="flex-1 box-border overflow-hidden" bind:this={chartContainer} bind:clientHeight={clientH} bind:clientWidth={clientW} on:pointerdown|stopPropagation />
</div>
