<script lang="ts">
  import { isLoaded, polledRunEntries, selectedRun, selectedRunData, selectedRunHeader, selectedRunLoadedData } from "../apiStores";
  import { runSelectModalOpen } from "../appStores";

  import FileList from "./FileList.svelte";
  $: if ($isLoaded) {
    $runSelectModalOpen = false;
  }

  function setRun(u) {
    $selectedRun = u;
  }
</script>

<div class="modal cursor-pointer" class:modal-open={$runSelectModalOpen}>
  <div class="modal-box relative cursor-default">
    <h3 class="text-lg font-bold">Congratulations random Internet user!</h3>
    <FileList />
  </div>
</div>
