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

<Grid gap={[10, 5]} {items} {cols} rowHeight={150} let:item let:dataItem >
    <div class="card card-bordered bg-base-200 h-full">
        <div class="card-body h-full" >
          <DashChart {cursorSync}></DashChart>
        </div>
      </div>
      
</Grid>

<style>
    :global(.svlt-grid-shadow) {
      /* Back shadow */
      background: hsl(var(--nc)) !important;
      border-radius: 1rem;
    }
  </style>