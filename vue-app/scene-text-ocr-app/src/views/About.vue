<template>
  <div class="about">
    <h1>This is an about page</h1>
    <clipper-upload v-model="imgURL">upload image</clipper-upload>
    <clipper-basic class="my-clipper" ref="clipper" :src="imgURL">
      <div class="placeholder" slot="placeholder">No image</div>
    </clipper-basic>
    <h2>Recognized text: {{recognizedText}}</h2>
    <button v-on:click="recognizeClip">Recognize text in clip</button>
  </div>
</template>
<script lang="ts">
import { Component, Vue } from 'vue-property-decorator';
import HelloWorld from '@/components/HelloWorld.vue'; // @ is an alias to /src

@Component({
  components: {
    HelloWorld,
  },
})
export default class Home extends Vue {
  private imgURL: string = '';
  private recognizedText: string = '';

  private recognizeClip(): void {
    const clipper: any = this.$refs.clipper;
    const canvas = clipper.clip();
    const pngUrl = canvas.toDataURL('image/png', 1);
    const encodingToken = 'base64,';
    const indexOfEncoding = pngUrl.indexOf(encodingToken);
    if (indexOfEncoding < 0) {
      throw new Error('base64 encoding not present in png url string, ummmmmm');
    }
    const pngAsBase64 = pngUrl.substring(indexOfEncoding + encodingToken.length);
    console.log(pngUrl);
    console.log(pngAsBase64);
    const recoPayload = { base64image: pngAsBase64 };
    const thisRef = this;
    fetch("http://64.187.164.30:63264/SceneTextOCR/Api/recognize", {
      method: "POST",
      body: JSON.stringify(recoPayload),
      headers: {
        "Content-Type": "application/json",
      },
    })
    .then((response) => {
      response.json().then((parsedResponse) => {
        thisRef.recognizedText = parsedResponse;
        console.log("Recognized text: " + thisRef.recognizedText);
      });
    });
  }
}
</script>

<style scoped lang="scss">
.my-clipper {
  width: 100%;
  max-width: 700px;
}
</style>