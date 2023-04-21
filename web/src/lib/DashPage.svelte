<script lang="ts">
    import type { Readable } from "svelte/store";
    import gridHelp from "svelte-grid/build/helper";
    import Grid from "svelte-grid";
    import type { tLoadedApiData, tLoadedApiDataContainer } from "../api";
    import { createEventDispatcher } from "svelte";
    import DashChart from "./DashChart.svelte";
    import uPlot from "uplot";
    import { X as CloseIcon, CaretDown } from "phosphor-svelte";
    import { fade } from "svelte/transition";
    import AutoComplete from "simple-svelte-autocomplete";

    export let dataStore: Readable<tLoadedApiDataContainer>;

    let dispatch = createEventDispatcher();

    function dispatchExit() {
        dispatch("exit");
    }

    let dashboards = ["test", "lel"];
    let selectedDashboard = null;

    let cursorSync = uPlot.sync("main");

    let items = [
        {
            6: gridHelp.item({
                x: 0,
                y: 0,
                w: 2,
                h: 2,
            }),
            id: 1,
            data: { lel: "test1" },
        },

        {
            6: gridHelp.item({
                x: 2,
                y: 0,
                w: 2,
                h: 2,
            }),
            id: 2,
            data: { lel: "test2" },
        },
    ];

    const cols = [[1200, 6]];
</script>

<!-- Header -->
<div class="navbar shadow-lg bg-base-300 rounded-box m-3 w-auto flex justify-between">
    <!-- <div class="flex-none">
        <button class="btn btn-square btn-ghost">
            <svg xmlns="http://www.w3.org/2000/svg" fill="none" viewBox="0 0 24 24" class="inline-block w-6 h-6 stroke-current">
                <path stroke-linecap="round" stroke-linejoin="round" stroke-width="2" d="M4 6h16M4 12h16M4 18h16" />
            </svg>
        </button>
    </div> -->
    <div class="min-w-0">
        <AutoComplete items={dashboards} bind:selectedItem={selectedDashboard} />
    </div>
    <div class="px-2 mx-2">
        <span class="text-lg font-bold"> ESPDAQ 2023 </span>
    </div>
    <button class="btn btn-square btn-ghost btn-error" on:click={dispatchExit}>
        <CloseIcon size="32" />
    </button>
</div>

<!-- Dashboard -->
<div class="overflow-y-scroll flex-1 ">
    <div class="min-w-0 lg:max-w-full max-w-[95%]">
        <Grid gap={[10, 5]} {items} {cols} rowHeight={100} let:item let:dataItem let:movePointerDown>
            <div class="card card-bordered bg-base-200 h-full overflow-hidden">
                <div class="card-body h-full p-0 overflow-hidden">
                    <DashChart {cursorSync} {dataStore} data={dataItem?.data} />
                </div>
            </div>
        </Grid>
    </div>
</div>

<style>
    :global(.svlt-grid-shadow) {
        background: hsl(var(--nc)) !important;
        border-radius: 1rem;
    }

    :global(.input-container) {
        height: 100%;
        width: 100%;
        padding: 0;
        align-items: center;
    }
</style>
