<script>

import { Radio, Fileupload, Label, Button } from 'flowbite-svelte';

/**
 * @type {import('./ImageData.js').default} cropperData
 */
export let cropperData
let slot = 0

let availableSlot = [
    { index: 0, name: 'A' },
    { index: 1, name: 'B' },
    { index: 2, name: 'C' },
    { index: 3, name: 'D' },
    { index: 4, name: 'E' },
]

async function handleOnSubmit(event) {
    const data = await cropperData.getCroppedImageData()

    if (!data) {
        console.log("Missing image")
    }

    const result = fetch("/upload_image?slot="+slot, {
        method: 'POST',
        body: data
    })
    
    return false
}
</script>

<form class="self-stretch flex flex-col items-stretch gap-3" on:submit|preventDefault={handleOnSubmit}>
    <p class="font-semibold text-gray-900 dark:text-white">Upload to image slot:</p>
    <ul class="items-center w-full rounded-lg border border-gray-200 sm:flex dark:bg-gray-800 dark:border-gray-600 divide-x rtl:divide-x-reverse divide-gray-200 dark:divide-gray-600">
        {#each availableSlot as { index, name }}
        <li class="w-full">
            <Radio class="p-3" id="slot{index}" name="slot" value={index} bind:group={slot}>{name}</Radio>
        </li>
        {/each}
    </ul>
    <Button color="red" type="submit">Upload</Button>
</form>