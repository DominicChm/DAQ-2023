<script lang="ts">
    import { onMount } from "svelte";
    import TimeChart from "timechart";

    let chartContainer;
    let ch;
    $: console.log(ch);
    let resizeWatch = () => {};
    onMount(() => {
        let t = new TimeChart(chartContainer, {
            series: [
                {
                    name: "test",
                    data: [
                        { x: 1, y: 1 },
                        { x: 2, y: 2 },
                    ],
                },
            ],
        });

        resizeWatch = t.onResize; 
    });
</script>

<div class="w-full h-full relative" bind:this={chartContainer} bind:clientHeight={ch} on:pointerdown|stopPropagation/>
