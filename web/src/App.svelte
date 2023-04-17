<script lang="ts">
  import svelteLogo from "./assets/svelte.svg";
  import viteLogo from "/vite.svg";
  import Counter from "./lib/Counter.svelte";
  import Navbar from "./lib/Navbar.svelte";
    import Dash from "./lib/Dash.svelte";
    import FileList from "./lib/FileList.svelte";
</script>

<main>
  <FileList />
  <Navbar />
  <Dash></Dash>
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
