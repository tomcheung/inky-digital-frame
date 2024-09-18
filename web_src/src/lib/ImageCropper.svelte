<script>
  import Cropper from "cropperjs";
  import { Button } from "flowbite-svelte";
  const previewSize = {width: 400, height: 240}
  let cropperCanvas
  let cropperImage
  let cropperSelection
  let isMatchSize = false
  export let image;

  $: {
    if (cropperImage) {
      cropperImage.$ready((img) => {
        const diff = img.width / img.height - 80 / 48
        let width = 0
        let height = 0
        isMatchSize = diff == 0
        if (diff > 0) {
          height = img.height / img.width * previewSize.width
          width = 80 / 48 * height
        } else {
          width = img.width / img.height * previewSize.height
          height = previewSize.height
        }

        cropperSelection.$change(
          Math.floor((previewSize.width - width) / 2), 
          Math.floor((previewSize.height - height) / 2), 
          Math.floor(width), 
          Math.floor(height)
        )
        shade.$change((previewSize.width - width) / 2, (previewSize.height - height) / 2, width, height);
      })
    }
  }

  /**
   * @typedef {Object} Selection - Crop selection
   * @property {number} x
   * @property {number} y
   * @property {number} width
   * @property {number} height
   */

  let shade;

  function handleSelection(event) {
    /** @type {Selection} */
    const selection = event.detail;

    if (isMatchSize) {
      shade.$change(selection.x, selection.y, selection.width, selection.height);
      return
    }

    const canvasRect = cropperCanvas.getBoundingClientRect()
    const imageRect = cropperImage.getBoundingClientRect()

    let imageInset = {x: imageRect.x - canvasRect.x, y: imageRect.y - canvasRect.y}
    
    const cropperRectInImage = {x: selection.x - imageInset.x, y: selection.y - imageInset.y, width: selection.width, height: selection.height}
    shade.$change(selection.x, selection.y, selection.width, selection.height);

    if (Math.ceil(cropperRectInImage.x) < 0 || Math.ceil(cropperRectInImage.y) < 0 
      || Math.floor(cropperRectInImage.x + cropperRectInImage.width) > imageRect.width
      || Math.floor(cropperRectInImage.y + cropperRectInImage.height) > imageRect.height
    ) {
      event.preventDefault()
    }
  }

  export function getCropImage() {
    return cropperSelection.$toCanvas({width: 800, height: 480})
  }
</script>
<cropper-canvas background scale-step={0} id="cropper" bind:this={cropperCanvas} disabled={isMatchSize}>
  <cropper-image
    bind:this={cropperImage}
    src={image}
    alt="Picture"
    scalable
    translatable
  ></cropper-image>
  <cropper-shade bind:this={shade}></cropper-shade>
  <cropper-handle action="select" plain></cropper-handle>
  <cropper-selection
    movable
    resizable
    aspect-ratio={80 / 48}
    bind:this={cropperSelection}
    on:change={handleSelection}
  >
    <cropper-crosshair centered></cropper-crosshair>
    <cropper-handle action="move" theme-color="transparent"></cropper-handle>
    <cropper-handle action="n-resize"></cropper-handle>
    <cropper-handle action="e-resize"></cropper-handle>
    <cropper-handle action="s-resize"></cropper-handle>
    <cropper-handle action="w-resize"></cropper-handle>
    <cropper-handle action="ne-resize"></cropper-handle>
    <cropper-handle action="nw-resize"></cropper-handle>
    <cropper-handle action="se-resize"></cropper-handle>
    <cropper-handle action="sw-resize"></cropper-handle>
  </cropper-selection>
</cropper-canvas>
<style>
  #cropper {
    width: 400px;
    height: 240px;
  }
</style>
