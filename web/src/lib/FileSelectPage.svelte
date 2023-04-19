<script lang="ts">
    import { writable } from "svelte/store";
    import FileList from "./FileList.svelte";
    import { cancelableLoadingStore } from "../apiStores";
    import { fade } from "svelte/transition";
    import DashPage from "./DashPage.svelte";

    let progress = writable(0);
    let urlStore = writable(null);

    let fileDataStore = cancelableLoadingStore(urlStore, progress);
    $: console.log($fileDataStore);
    function onOpened({ detail }) {
        urlStore.set(detail);
    }
</script>

{#if $fileDataStore}
    <DashPage dataStore={fileDataStore} />
{:else}
    <!-- Main Page -->
    <h3 class="text-lg font-bold">Select file to view</h3>
    <FileList on:opened={onOpened} />

    {#if $urlStore}
        <!-- Loading Modal -->
        <div
            class="modal modal-bottom sm:modal-middle modal-open"
            transition:fade
        >
            <div class="modal-box">
                <h3 class="font-bold text-lg">Loading...</h3>
                <div>
                    <div
                        class="radial-progress"
                        style={`--value:${$progress * 100};`}
                    >
                        {Math.floor($progress * 100)}%
                    </div>
                </div>
                <div class="modal-action">
                    <label for="my-modal-6" class="btn">Cancel</label>
                </div>
            </div>
        </div>
    {/if}
{/if}
