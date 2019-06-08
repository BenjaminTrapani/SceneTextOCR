# SceneTextOCR

## Demo
http://ai.skytopsoftware.com/

This is a modified implementation of the CRNN architecture. It transcribes cropped images of a single word into text. It is built to handle all kinds of text (cursive, some handwriting, printed etc.) It uses an 18 layer ResNet instead of the VGG stack in the original model, and gets about 97% transcription accuracy. It is trained on the mjsynth dataset. It is trained to map uppercase and lowercase letters to their uppercase version. It does not support punctuation or numbers yet. The maximum word length supported is 16 characters, although I plan to make this size arbitrary soon.

Instructions for building are also coming soon. For now, a trained model can be tested out and used from your apps by using the script "TextTranscriptionCode/WebTester/webtester.py" This script accepts a path to an input image as an argument and hits a public server running the model. Some sample images are provided in the WebTester directory for use. Feel free to adapt that script and use it in your apps, in addition to the server.
