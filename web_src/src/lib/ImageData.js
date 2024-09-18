/**
 * @class
 * @alias module:ImageCropperData
 */
export default class ImageCropperData {
  /**
   * @param {String|ArrayBuffer} rawImageData - Uncropped image data.
   */
  constructor(imageCropper, rawImageData) {
    this.imageCropper = imageCropper
    this.rawImageData = rawImageData
  }

  /**
   * @return {Promise<Image>} Cropped image data.
   */
  async getCroppedImage() {
    let canvas = await this.imageCropper.getCropImage()
    return canvas.toDataURL('image/jpeg')
  }

  /**
   * @return {Promise<ArrayBuffer>} Cropped image data.
   */
  async getCroppedImageData() {
    let canvas = await this.imageCropper.getCropImage()
    const p = new Promise((resolve, reject) => {
      canvas.toBlob((blob) => {
        const reader = new FileReader();
        reader.addEventListener('loadend', () => {
          const arrayBuffer = reader.result;
          resolve(arrayBuffer)
        });
        reader.readAsArrayBuffer(blob);
      }, 'image/jpeg');
    })

    return await p;
  }
 }
