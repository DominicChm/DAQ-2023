<script lang="ts">
    import { onMount } from "svelte";
    import uplot from "uplot";
    import { isLoaded, selectedRunLoadedData } from "../apiStores";

    export let cursorSync: uplot.SyncPubSub;

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

    selectedRunLoadedData.subscribe((sources) => {
        if (plt) plt.destroy();

        if (sources == null) return;
        const data = [sources.__TIME, sources.data0, sources.data3, sources["data1.5"]];
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
                        legendContainer.appendChild(el);
                    },
                    show: true,
                },
                series: [
                    { label: "x" },
                    { label: "data0", stroke: "red", spanGaps: true },
                    { label: "data3", stroke: "green", spanGaps: true },
                    { label: "data15", stroke: "orange", spanGaps: true },
                ],
            },
            data,
            chartContainer
        );
        cursorSync.sub(plt);
    });
</script>

<div class="w-full h-full flex flex-col overflow-hidden">
    <div class="flex justify-between prose w-auto max-w-none">
        <h3 class="bg-neutral block px-3 m-0 py-2 rounded-br-xl cursor-move">{"Name, lmao!"}</h3>
        <div on:pointerdown|stopPropagation bind:this={legendContainer} />
    </div>
    <div class="flex-1 box-border overflow-hidden" bind:this={chartContainer} bind:clientHeight={clientH} bind:clientWidth={clientW} on:pointerdown|stopPropagation>
        {#if !$isLoaded}
            <div class="absolute left-0 right-0 w-full h-full flex justify-center items-center">
                <div>No Data</div>
            </div>
        {/if}
    </div>
</div>
