<script lang="ts">
    import { onMount } from "svelte";
    import uplot from "uplot";
    import { selectedRunLoadedData } from "../apiStores";

    export let cursorSync: uplot.SyncPubSub;

    let chartContainer;
    let u;

    let clientH, clientW;
    // $: console.log(ch);
    $: console.log($selectedRunLoadedData);
    // Updates graph sizes when their container changes
    $: {
        u?.setSize({
            height: clientH || 100,
            width: clientW || 100,
        });
    }

    function getVar(v) {
        return (
            "hsl(" +
            getComputedStyle(document.documentElement).getPropertyValue(v) +
            ")"
        );
    }

    // https://www.30secondsofcode.org/js/s/lcm/
    const lcm = (...arr) => {
        const gcd = (x, y) => (!y ? x : gcd(y, x % y));
        const _lcm = (x, y) => (x * y) / gcd(x, y);
        return [...arr].reduce((a, b) => _lcm(a, b));
    };

    onMount(() => {
            
        
        const matchSyncKeys = (own, ext) => own == ext;

        u = new uplot(
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
                series: [
                    { label: "x" },
                    { label: "data0", stroke: "red", spanGaps: true},
                    { label: "data3", stroke: "green", spanGaps: true},
                ],
            },
            data,
            chartContainer
        );
        cursorSync.sub(u);

        // let to = null;
        // resizeEvent.addEventListener("resize", () => {
        //     if (to != null) clearTimeout(to);
        //     to = setTimeout(() => t.onResize(), 100);
        // });
    });
</script>

<div
    class="w-full h-full text-inherit"
    bind:this={chartContainer}
    bind:clientHeight={clientH}
    bind:clientWidth={clientW}
    on:pointerdown|stopPropagation
/>
