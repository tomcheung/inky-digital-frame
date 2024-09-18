<script>
  import ImageCropper from "./lib/ImageCropper.svelte";
  import "./app.css";
  import { Heading, Dropzone, Button, Modal } from "flowbite-svelte";
  import svelteLogo from "./assets/svelte.svg";
  import ImageUpload from "./lib/ImageUpload.svelte";
  import ImageCropperData from "./lib/ImageData";

 
  let imageCropperData;
  let imageCropper;
  let previewImage;

  $: {
    if (imageCropperData) { 
      imageCropperData.imageCropper = imageCropper
    }
  }

  $: showPreview = previewImage != null

  /**
   * @param {File} file
   */
  function readFile(file) {
    const reader = new FileReader();
    reader.onload = () => {
      imageCropperData = new ImageCropperData(imageCropper, reader.result);
    };

    if (file) {
      reader.readAsDataURL(file);
      console.log("Loading image");
    } else {
      console.log("File empty");
    }
  }

  function handleChange(event) {
    const files = event.target.files;
    if (files.length > 0) {
      readFile(files[0]);
    }
  }

  function clearImage() {
    imageCropperData = null
  }

  async function preview() {
    previewImage = await imageCropperData.getCroppedImage()
  }

</script>

<main>
  <div>
    <a href="https://svelte.dev" target="_blank" rel="noreferrer">
      <img src={svelteLogo} class="logo svelte" alt="Svelte Logo" />
    </a>
  </div>
  <div class="flex flex-col space-y-4 items-center">
    <Heading tag="h2">Inky Frame Photo Manager</Heading>
    {#if !imageCropperData}
      <Dropzone on:change={handleChange}>
        <p class="mb-2 font-semibold text-sm text-gray-500 dark:text-gray-400">
          Upload an image to start
        </p>
        <p class="text-xs text-gray-500 dark:text-gray-400">
          <span class="font-semibold ">Click or drag and drop to load an image</span>
          <br/>
          Support PNG or JPG
        </p>
      </Dropzone>
    {:else}
      <ImageCropper bind:this={imageCropper} image={imageCropperData.rawImageData} />
      <div class="flex flex-row gap-3 self-stretch">
        <Button class='flex-1' color=blue on:click={preview}>Preview</Button>
        <Button class='self-end' color='red' on:click={clearImage}>Clear</Button>
      </div>
      <ImageUpload cropperData={imageCropperData}/>
    {/if}
  </div>
  <Modal title='Preview' bind:open={showPreview} on:close={() => previewImage = null} autoclose>
    <img src={previewImage} alt=Preview/>
  </Modal>
 
</main>

<style>
  .logo {
    height: 6em;
    padding: 1.5em;
    will-change: filter;
    transition: filter 300ms;
  }
  .logo:hover {
    filter: drop-shadow(0 0 2em #646cffaa);
  }
  .logo.svelte:hover {
    filter: drop-shadow(0 0 2em #ff3e00aa);
  }
</style>
