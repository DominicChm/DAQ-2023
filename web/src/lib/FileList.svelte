<script lang="ts">
  import { onMount } from "svelte";
  import { apiGetIndex } from "../api";

  let index = [];
  onMount(async () => {
    index = Object.entries(await apiGetIndex());
  });

  let isOpen = true;
</script>

<div class="modal cursor-pointer" class:modal-open={isOpen}>
  <div class="modal-box relative cursor-default">
    <h3 class="text-lg font-bold">Congratulations random Internet user!</h3>
    {#each index as [file_path, i]}
      <p>{file_path}, {i.name}</p>
    {/each}
  </div>
</div>
