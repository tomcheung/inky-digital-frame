<script lang="ts">
let image: FileList
let slot = 0

async function handleOnSubmit(event: SubmitEvent) {
    const formData = new FormData()

    const fileInput = (document.getElementById('image') as HTMLInputElement)
    const imageFile = fileInput.files![0]

    const reader = new FileReader()
    reader.readAsArrayBuffer(imageFile)
    
    reader.onload = function () {
        const result = fetch("/upload_image?slot="+slot, {
            method: 'POST',
            body: reader.result
        }).then(function (result) {
            console.log(result);            
        })
    }

    return false
}
</script>

<form on:submit|preventDefault={handleOnSubmit}>
    <div>Selected slot: {slot}</div>
    <div>
        {#each [0,1,2,3] as number}
        <input type="radio" id="slot{number}" name="slot" value={number} bind:group={slot}/>
        <label for="slot{number}">{number}</label>
        {/each}
    </div>
    <div class="group">
        <label for="image">Upload a picture: </label>
        <input accept="image/png, image/jpeg" id="image" name="image" type="file" />
    </div>
    <button type="submit">Submit</button>
</form>