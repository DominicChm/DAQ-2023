<script lang="ts">
    import { onMount } from "svelte";
    import uplot from "uplot";

    export let cursorSync: uplot.SyncPubSub;

    let chartContainer;
    let u;

    let clientH, clientW;
    // $: console.log(ch);

    // Updates graph sizes when their container changes
    $: {
        u?.setSize({
            height: clientH || 100,
            width: clientW || 100,
        });
    }

    function getVar(v) {
        return "hsl(" + getComputedStyle(document.documentElement).getPropertyValue(v) + ")";
    }
    console.log(getVar("hsl(--bc)"))

    onMount(() => {
        let data = [
            [1, 2, 3, 4, 5],
            [1, 3, 2, 5, 4],
        ];
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
                        size: 7
                    },
                },
                focus: {
                    alpha: 0.3,
                },
                width: 300,
                height: 300,
                scales: { x: { time: false } },
                axes: [
                    { stroke: getVar("--bc"), grid: { stroke:getVar("--b1") } },
                    { stroke: getVar("--bc"), grid: { stroke: getVar("--b1") } },
                ],
                series: [
                    { label: "x" }, 
                    { label: "y", stroke: "red" }
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

<div class="w-full h-full text-inherit" bind:this={chartContainer} bind:clientHeight={clientH} bind:clientWidth={clientW} on:pointerdown|stopPropagation />
