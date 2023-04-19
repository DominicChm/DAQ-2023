<script lang="ts">
    import { onDestroy, onMount } from "svelte";

    import type { tLoadedApiData, tLoadedApiDataContainer } from "../api";
    import { getVar, mapDotNotations } from "../util";
    import Portal from "svelte-portal";
    import { MagnifyingGlass, PencilSimple } from "phosphor-svelte";
    import { fade } from "svelte/transition";
    import Chart from "./Chart.svelte";
    import type { Readable } from "svelte/store";
    import AutoComplete from "simple-svelte-autocomplete";

    export let dataStore: Readable<tLoadedApiDataContainer>;
    export let cursorSync: uPlot.SyncPubSub;
    //export let data = {};

    let legendContainer;

    let testDataDef = {
        sources: {
            data1: {
                color: "red",
                name: "Custom data1 label",
            },
            data2: {
                color: "orange",
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

    function saveEdits() {
        setEditing(false);
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
            <button class="btn btn-ghost btn-sm loc-hidden" on:pointerdown|stopPropagation on:click={() => setEditing(true)}><PencilSimple size={24}/></button>
            <button class="btn btn-ghost btn-sm loc-hidden" on:pointerdown|stopPropagation on:click={() => setZooming(true)}><MagnifyingGlass size={24}/></button>
        </div>
        <div on:pointerdown|stopPropagation bind:this={legendContainer} />
    </div>
    <Chart {cursorSync} {dataStore} {legendContainer} sources={testDataDef.sources} />
</div>

<!-- Card Edit Modal -->
{#if isEditing}
    <Portal>
        <div class="modal modal-open" transition:fade={{ duration: 100 }}>
            <div class="modal-box w-11/12 max-w-full flex flex-col" style="height: 90%;">
                <h3 class="font-bold text-lg">Edit Chart</h3>
                <div class="flex-1 flex min-h-0">
                    <div class="grid w-full grid-rows-[1fr_1fr] lg:grid-rows-none lg:grid-cols-[1fr_25rem] items-stretch">
                        <Chart {cursorSync} {dataStore} sources={testDataDef.sources} showLegend={false} gridColor={getVar("--b2")}/>
                        <div class="flex flex-col gap-2">
                            <div class="flex-1">
                                <div class="overflow-x-hidden">
                                    <table class="table">
                                        <thead>
                                            <tr>
                                                <th />
                                                <th>Data Path</th>
                                                <th>Job</th>
                                                <th>Favorite Color</th>
                                            </tr>
                                        </thead>
                                        <tbody>
                                            <!-- row 1 -->
                                            <tr>
                                                <th>1</th>
                                                <td><AutoComplete items={[1, 2, 3]} /></td>
                                                <td>Quality Control Specialist</td>
                                                <td>Blue</td>
                                            </tr>
                                        </tbody>
                                    </table>
                                </div>
                            </div>
                            <div class="inline-block self-end">
                                <button class="btn" on:click={cancelEdits}>Cancel</button>
                                <button class="btn btn-primary" on:click={saveEdits}>Save</button>
                            </div>
                        </div>
                    </div>
                </div>
            </div>
        </div>
    </Portal>
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

    table :global(.autocomplete) {
        border-color: hsl(var(--b3)) !important;
    }
</style>
