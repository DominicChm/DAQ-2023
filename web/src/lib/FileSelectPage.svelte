<script lang="ts">
    import { writable } from "svelte/store";
    import FileList from "./FileList.svelte";
    import { polledRunEntries, polledRuns, runDataStore } from "../apiStores";
    import { fade } from "svelte/transition";
    import DashPage from "./DashPage.svelte";

    let progress = writable(0);
    let urlStore = writable(null);

    let ac = new AbortController();

    $: runDataLoader = runDataStore(urlStore, progress, ac, $urlStore);

    function onOpened({ detail }) {
        urlStore.set(detail);
    }

    function closeFile() {
        $urlStore = null;
    }

    // Set store null -> value to force a re-try.
    function retryDownload() {
        let u = $urlStore;
        urlStore.set(null);
        urlStore.set(u);
    }
</script>

{#await runDataLoader}
    <!-- Loading Modal -->
    <div class="modal modal-bottom sm:modal-middle modal-open" transition:fade={{ duration: 200 }}>
        <div class="modal-box prose">
            <h3 class="font-bold text-lg">Loading "{$polledRuns?.[$urlStore]?.name ?? ""}"</h3>
            <div class="flex flex-col justify-center items-center pt-8">
                <progress class="progress progress-primary w-full" value={$progress * 100} max="100" />
                <span>{Math.floor($progress * 100)}%</span>
            </div>
            {#if $progress != null}
                <p>This might take a little, depending on the size of the run file...</p>
            {:else}
                <p>There was a problem with the download. Blame Andrew.</p>
            {/if}
            <div class="modal-action">
                {#if $progress == null}
                    <!-- svelte-ignore a11y-click-events-have-key-events -->
                    <label for="my-modal-6" class="btn btn-error" on:click={retryDownload}>Retry</label>
                {/if}
                <!-- svelte-ignore a11y-click-events-have-key-events -->
                <label for="my-modal-6" class="btn" on:click={closeFile}>Cancel</label>
            </div>
        </div>
    </div>
{:then runDataStore}
    {#if runDataStore}
        <DashPage dataStore={runDataStore} on:exit={closeFile} />
    {:else}
        <h3 class="text-lg font-bold">Select file to view</h3>
        <button class="btn" on:click={() => onOpened({ detail: "/__LIVE" })}>Live</button>
        <FileList on:opened={onOpened} />
    {/if}
{:catch e}
    {console.error(e)}
    <p>ERR {e}</p>
{/await}
<!-- Main Page -->

<!-- Note: Keep this out of above if statement because the transition causes
weird switchover behavior -->
