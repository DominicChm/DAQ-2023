<script lang="ts">
  import Navbar from "./lib/Navbar.svelte";
  import Dash from "./lib/Dash.svelte";
  import FileListModal from "./lib/FileListModal.svelte";
</script>

<main>
  <FileListModal />
  <Navbar />
  <Dash />
</main>

<style>
  :global(body) {
    height: 100%;
    width: 100%;
    overflow: hidden;
  }

  :global(html) {
    height: 100%;
    width: 100%;
  }
</style>
