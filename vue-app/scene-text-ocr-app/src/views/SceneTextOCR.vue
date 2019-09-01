<template>
  <div class="about">
    <clipper-upload v-model="imgURL">
      <button>
        Upload an image
      </button>
    </clipper-upload>
    <p v-show="imgURL.length > 0">
      Select a single word to recognize in the image below.
    </p>
    <clipper-basic class="my-clipper" ref="clipper" :src="imgURL">
      <div class="placeholder" slot="placeholder"></div>
    </clipper-basic>
    <button v-on:click="recognizeClip">Recognize text in clip</button>
    <h2 v-show="isRecoInProgress">Recognizing...</h2>
    <h2 v-show="recognizedText.length > 0">
      Recognized text: {{recognizedText}}
    </h2>
    <h3 v-show="trimmedRecognizedText.length > 0">
      Final text: {{trimmedRecognizedText}}
    </h3>
  </div>
</template>
<script lang="ts">
import { Component, Vue } from 'vue-property-decorator';

@Component({
  components: {
  },
})
export default class SceneTextOCR extends Vue {
  private imgURL: string = '';
  private recognizedText: string = '';
  private trimmedRecognizedText: string = '';
  private isRecoInProgress: boolean = false;

  private recognizeClip(): void {
    this.recognizedText = '';
    this.trimmedRecognizedText = '';
    this.isRecoInProgress = true;
    const clipper: any = this.$refs.clipper;
    const canvas = clipper.clip();
    const pngUrl = canvas.toDataURL('image/png', 1);
    const encodingToken = 'base64,';
    const indexOfEncoding = pngUrl.indexOf(encodingToken);
    if (indexOfEncoding < 0) {
      throw new Error('base64 encoding not present in png url string, ummmmmm');
    }
    const pngAsBase64 = pngUrl.substring(indexOfEncoding + encodingToken.length);
    const recoPayload = { base64image: pngAsBase64 };
    const thisRef = this;
    fetch('http://skytopsoftware.ddns.net:63264/SceneTextOCR/Api/recognize', {
      method: 'POST',
      body: JSON.stringify(recoPayload),
      headers: {
        'Content-Type': 'application/json',
      },
    })
    .then((response) => {
      response.json().then((parsedResponse) => {
        thisRef.isRecoInProgress = false;
        thisRef.recognizedText = parsedResponse;
        thisRef.trimmedRecognizedText = thisRef.recognizedText.replace(/_/g, '');
      });
    });
  }
}
</script>

<style scoped lang="scss">
.my-clipper {
  width: 100%;
  max-width: 700px;
  display: block;
  margin-left: auto;
  margin-right: auto;
}
</style>