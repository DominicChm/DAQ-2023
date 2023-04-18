<script lang="ts">
    import gridHelp from "svelte-grid/build/helper";
    import Grid from "svelte-grid";
    import DashChart from "./DashChart.svelte";
    import uPlot from "uplot";

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

<div class="overflow-y-scroll flex-1">
    <Grid gap={[10, 5]} {items} {cols} rowHeight={150} let:item let:dataItem let:movePointerDown>
        <div class="card card-bordered bg-base-200 h-full overflow-hidden">
            <div class="card-body h-full p-0 overflow-hidden" >
                <DashChart {cursorSync} />
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
