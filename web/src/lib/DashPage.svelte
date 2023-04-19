<script lang="ts">
    import type { Readable } from "svelte/store";
    import gridHelp from "svelte-grid/build/helper";
    import Grid from "svelte-grid";
    import Dash from "./Dash.svelte";
    import type { tLoadedApiData } from "../api";
    import { createEventDispatcher } from "svelte";
    import DashChart from "./DashChart.svelte";

    export let dataStore: Readable<tLoadedApiData>;
        
    let dispatch = createEventDispatcher();

    function dispatchExit() {
        dispatch("exit");
    }

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
        },

        {
            6: gridHelp.item({
                x: 2,
                y: 0,
                w: 2,
                h: 2,
            }),
            id: 2,
        },
    ];

    const cols = [[1200, 6]];
</script>

<!-- Header -->
<div
    class="navbar shadow-lg bg-neutral text-neutral-content rounded-box m-3 w-auto"
>
    <div class="flex-none">
        <button class="btn btn-square btn-ghost">
            <svg
                xmlns="http://www.w3.org/2000/svg"
                fill="none"
                viewBox="0 0 24 24"
                class="inline-block w-6 h-6 stroke-current"
            >
                <path
                    stroke-linecap="round"
                    stroke-linejoin="round"
                    stroke-width="2"
                    d="M4 6h16M4 12h16M4 18h16"
                />
            </svg>
        </button>
    </div>
    <div class="flex-1 px-2 mx-2">
        <span class="text-lg font-bold"> ESPDAQ 2023 </span>
    </div>
    <div class="flex-none">
        <select
            class="select select-bordered w-full max-w-xs"
            placeholder="Dashboard"
        >
            <option>Dash1</option>
            <option>Dash2</option>
            <option>Dash3</option>
        </select>
    </div>
</div>

<!-- Dashboard -->
<div class="overflow-y-scroll flex-1">
    <Grid
        gap={[10, 5]}
        {items}
        {cols}
        rowHeight={150}
        let:item
        let:dataItem
        let:movePointerDown
    >
        <div class="card card-bordered bg-base-200 h-full overflow-hidden">
            <div class="card-body h-full p-0 overflow-hidden">
                <DashChart {cursorSync} {dataStore} />
            </div>
        </div>
    </Grid>
</div>

<style>
    :global(.svlt-grid-shadow) {
        background: hsl(var(--nc)) !important;
        border-radius: 1rem;
    }
</style>
