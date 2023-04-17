<script lang="ts">
    import { runSelectModalOpen } from "../appStores";

    let selected = "";
</script>

<div class="navbar shadow-lg bg-neutral text-neutral-content rounded-box m-3 w-auto">
    <div class="flex-none">
        <button class="btn btn-square btn-ghost">
            <svg xmlns="http://www.w3.org/2000/svg" fill="none" viewBox="0 0 24 24" class="inline-block w-6 h-6 stroke-current">
                <path stroke-linecap="round" stroke-linejoin="round" stroke-width="2" d="M4 6h16M4 12h16M4 18h16" />
            </svg>
        </button>
    </div>
    <div class="flex-1 px-2 mx-2">
        <span class="text-lg font-bold"> ESPDAQ 2023 </span>
    </div>
    <div class="flex-none">
        <select class="select select-bordered w-full max-w-xs" placeholder="Dashboard" bind:value={selected}>
            <option>Dash1</option>
            <option>Dash2</option>
            <option>Dash3</option>
        </select>
    </div>
    <button class="btn btn-primary" on:click={() => $runSelectModalOpen = true}>Open</button>
</div>
