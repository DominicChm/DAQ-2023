<script lang="ts">
    import { onDestroy, onMount } from "svelte";

    import type { tChartDefinition, tLoadedApiData, tLoadedApiDataContainer } from "../api";
    import { getVar, mapDotNotations } from "../util";
    import Portal from "svelte-portal";
    import { MagnifyingGlass, PencilSimple } from "phosphor-svelte";
    import { fade } from "svelte/transition";
    import Chart from "./Chart.svelte";
    import type { Readable } from "svelte/store";
    import AutoComplete from "simple-svelte-autocomplete";
    import ChartEditor from "./ChartEditor.svelte";

    export let dataStore: Readable<tLoadedApiDataContainer>;
    export let cursorSync: uPlot.SyncPubSub;
    //export let data = {};

    let legendContainer;



    let testDataDef: tChartDefinition = {
        sources: {
            data1: {
                color: "#000",
                name: "Custom data1 label",
            },
            data2: {
                color: "#000",
                name: "Custom data2 label",
            },
        },
        chartName: "lel",
        chartType: "line",
    };

    let isEditing = false;
    let isZooming = false;

    function setZooming(state) {
        isZooming = state;
    }

    function setEditing(state) {
        isEditing = state;
    }

    function saveEdits({detail}) {
        setEditing(false);
        testDataDef = detail;
    }

    function cancelEdits() {
        setEditing(false);
    }
</script>

<!-- Grid Card Contents-->
<div class="w-full h-full flex flex-col overflow-hidden">
    <div class="flex justify-between prose w-auto max-w-none max-h-14">
        <div class="flex items-center gap-2 revealer">
            <h3 class="bg-neutral block px-3 m-0 py-2 rounded-br-2xl cursor-move select-none h-full">{testDataDef.chartName}</h3>
            <button class="btn btn-ghost btn-sm loc-hidden" on:pointerdown|stopPropagation on:click={() => setEditing(true)}><PencilSimple size={24} /></button>
            <button class="btn btn-ghost btn-sm loc-hidden" on:pointerdown|stopPropagation on:click={() => setZooming(true)}><MagnifyingGlass size={24} /></button>
        </div>
        <div on:pointerdown|stopPropagation bind:this={legendContainer} />
    </div>
    <Chart {cursorSync} {dataStore} {legendContainer} sources={testDataDef.sources} />
</div>

<!-- Card Edit Modal -->
{#if isEditing}
    <ChartEditor definition={testDataDef} {cursorSync} {dataStore} on:cancel={cancelEdits} on:commit={saveEdits} />
{/if}

{#if isZooming}
    <Portal>
        <div class="modal modal-open" transition:fade={{ duration: 100 }}>
            <div class="modal-box w-11/12 max-w-full flex flex-col" style="height: 90%;">
                <button class="btn btn-sm btn-circle absolute right-2 top-2" on:click={() => setZooming(false)}>✕</button>
                <h3 class="font-bold text-lg">Zoom</h3>
                <div class="flex-1 flex min-h-0">
                    <Chart {cursorSync} {dataStore} sources={testDataDef.sources} showLegend={true} gridColor={getVar("--b2")} />
                </div>
            </div>
        </div>
    </Portal>
{/if}

<style>
    .revealer:hover .loc-hidden {
        opacity: 1;
    }

    .loc-hidden {
        /* opacity: 0; */
    }


</style>
