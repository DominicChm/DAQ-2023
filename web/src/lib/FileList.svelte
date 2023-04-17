<script lang="ts">
  import { isLoaded, polledRunEntries, selectedRun, selectedRunData, selectedRunHeader, selectedRunLoadedData } from "../apiStores";
  import { runSelectModalOpen } from "../appStores";

  $: if ($isLoaded) {
    $runSelectModalOpen = false;
  }

  function setRun(u) {
    $selectedRun = u;
  }
</script>

{#each $polledRunEntries as [file_path, i]}
  <p>{file_path}, {i.name} <button class="btn" on:click={() => setRun(file_path)}>Set</button></p>
{/each}
<button class="btn" on:click={() => setRun(null)}>Cancel Load/Unload</button>
{#if typeof $selectedRunData == "number"}
  <p>{$selectedRunData * 100}% loaded</p>
{:else if $selectedRunData instanceof ArrayBuffer}
  <p>Loaded!</p>
  <!-- <pre>{JSON.stringify($selectedRunHeader, null, 2)}</pre> -->
  <!-- <pre>{JSON.stringify($selectedRunLoadedData, null, 2)}</pre> -->
{:else}
  <p>Data Unloaded</p>
{/if}
