import web
import base64
import json
import os
from subprocess import Popen, PIPE

urls = ('/SceneTextOCR/Api/recognize', 'recognize_text')

OCR_EVALUATOR_PATH = '../build/SceneTextOCREvaluator'
IMAGE_CACHE = 'ImageCache'
MODEL_FILE = '../TextTranscriptionWindows/Content/ResNetCRNNNewCNTK32SeqLenOut'

app = web.application(urls, globals())
app.reco_process = Popen([OCR_EVALUATOR_PATH, MODEL_FILE], stdin=PIPE, stdout=PIPE, stderr=PIPE)
app.image_id = 0

class recognize_text:
    def POST(self):
        data = json.loads(web.data())
        base64image = data['base64image']
        image_path = os.path.join(IMAGE_CACHE, str(app.image_id) + '.img');
        with open(image_path, 'w+b') as f:
            f.write(base64.b64decode(base64image));
        app.reco_process.stdin.write(image_path + '\n')
        app.reco_process.stdin.flush()
        result = app.reco_process.stdout.readline()
            
        print(result)
        app.image_id = app.image_id + 1
        return result
        
if __name__ == "__main__":
    app.run()
