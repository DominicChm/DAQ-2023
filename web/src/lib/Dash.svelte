<script lang="ts">
    import gridHelp from "svelte-grid/build/helper";
    import Grid from "svelte-grid";

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

<Grid gap={[10, 5]} {items} {cols} rowHeight={150} let:item let:dataItem>
    <div class="card card-bordered bg-base-200 h-full">
        <div class="card-body">
          <h2 class="card-title">No Images</h2> 
          <p>Rerum reiciendis beatae tenetur excepturi aut pariatur est eos. Sit sit necessitatibus veritatis sed molestiae voluptates incidunt iure sapiente.</p> 
          <div class="card-actions">
            <button class="btn btn-primary">Get Started</button> 
            <button class="btn btn-ghost">More info</button>
          </div>
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