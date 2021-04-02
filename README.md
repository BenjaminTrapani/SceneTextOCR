# SceneTextOCR

## Demo
http://gpuandai.com/

## Description
This is a modified implementation of the CRNN architecture. It transcribes cropped images of a single word into text. It is built to handle all kinds of text (cursive, some handwriting, printed etc.) It uses an 18 layer ResNet instead of the VGG stack in the original model, and gets about 97% transcription accuracy. It is trained on the mjsynth dataset. It is trained to map uppercase and lowercase letters to their uppercase version. It does not support punctuation or numbers yet. The maximum word length supported is 16 characters, although I plan to make this size arbitrary soon.

## Building
### Manually (preferred if doing development)
On a machine with an nvidia GPU:
1. sudo docker run --rm --gpus all -i -t docker.pkg.github.com/benjamintrapani/scenetextocr/cntk_nvidia_gpu_with_mods:1.0 bash
2. apt-get install git-lfs
3. git lfs install
4. git clone https://github.com/BenjaminTrapani/SceneTextOCR.git
5. cd SceneTextOCR/TextTranscriptionCode/
6. mkdir build
7. cd build/
8. cmake -DCNTK_DIR=/home/CNTK/ -DBOOST_ROOT=/usr/local/boost-1.60.0/ -DBoost_NO_SYSTEM_PATHS=TRUE -B. -S../
9. ./SceneTextOCREvaluator  ../TextTranscriptionWindows/Content/ResNetCRNNNewCNTK32SeqLenOut

The evaluator waits for a path to an image, and evaluates the decoded image in the trained model. For example, pasting
    /home/SceneTextOCR/TextTranscriptionCode/WebTester/water.jpg
will print out the decoded text WATER with the separator character '\_' included.

### Pre-build (easier to test out the model)
1. sudo docker run --rm --gpus all -i -t docker.pkg.github.com/benjamintrapani/scenetextocr/scene_text_ocr_built:1.0 bash
2. cd /home/SceneTextOCR/TextTranscriptionCode/build
3. Step 9 and below match the manual steps

### Training your own model
1. Download the mjsynth data set:
    wget https://thor.robots.ox.ac.uk/~vgg/data/text/mjsynth.tar.gz
    tar -xf mjsynth.tar.gz

2. Generate mapping files to be decoded by CNTK during training:
    ./MapFileMaker ./mnt/ image_map_train.txt text_map_train.txt image_map_test.txt text_map_test.txt 32 6000000 9000000

Replace ./mnt/ with the path to the root of the extracted mjsynth data. 32 is the maximum sequence length. 6000000 and 9000000 represent the number of training images and the total number of images respectively.

3. Run the trainer:
./SceneTextOCRTrainer MapFileMaker/image_map_train.txt MapFileMaker/text_map_train.txt MapFileMaker/image_map_test.txt MapFileMaker/text_map_test.txt /home/SceneTextOCR/TextTranscriptionCode/build/my_new_model

