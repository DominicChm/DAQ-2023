<script lang="ts">
    import { writable } from "svelte/store";
    import FileList from "./FileList.svelte";
    import { cancelableLoadingStore, polledRunEntries, polledRuns } from "../apiStores";
    import { fade } from "svelte/transition";
    import DashPage from "./DashPage.svelte";

    let progress = writable(0);
    let urlStore = writable(null);

    let fileDataStore = cancelableLoadingStore(urlStore, progress);

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

{#if $fileDataStore && $urlStore}
    <DashPage dataStore={fileDataStore} on:exit={closeFile} />
{:else}
    <!-- Main Page -->
    <h3 class="text-lg font-bold">Select file to view</h3>
    <FileList on:opened={onOpened} />
{/if}

<!-- Note: Keep this out of above if statement because the transition causes
weird switchover behavior -->
{#if $urlStore && !$fileDataStore}
    <!-- Loading Modal -->
    <div class="modal modal-bottom sm:modal-middle modal-open" transition:fade={{duration: 200}}>
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
{/if}
