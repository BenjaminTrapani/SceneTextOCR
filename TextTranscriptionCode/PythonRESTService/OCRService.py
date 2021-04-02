import web
import base64
import json
import os
from subprocess import Popen, PIPE
from cheroot.server import HTTPServer
from cheroot.ssl.builtin import BuiltinSSLAdapter

SCRIPT_PATH = os.path.dirname(os.path.realpath(__file__))
OCR_EVALUATOR_PATH = os.path.join(SCRIPT_PATH, '../build/SceneTextOCREvaluator')
IMAGE_CACHE = os.path.join(SCRIPT_PATH, 'ImageCache')
MODEL_FILE = os.path.join(SCRIPT_PATH, '../TextTranscriptionWindows/Content/ResNetCRNNNewCNTK32SeqLenOut')
SSL_CERT_PATH = '/etc/letsencrypt/live/gpuandai.com/fullchain.pem'
SSL_CERT_KEY_PATH = '/etc/letsencrypt/live/gpuandai.com/privkey.pem'

if not os.path.exists(IMAGE_CACHE):
    os.makedirs(IMAGE_CACHE)

HTTPServer.ssl_adapter = BuiltinSSLAdapter(
        certificate=SSL_CERT_PATH,
        private_key=SSL_CERT_KEY_PATH)

urls = ('/SceneTextOCR/Api/recognize', 'recognize_text')

app = web.application(urls, globals())
app.reco_process = Popen([OCR_EVALUATOR_PATH, MODEL_FILE], stdin=PIPE, stdout=PIPE, stderr=PIPE)
app.image_id = 0

class recognize_text:
    def POST(self):
        web.header('Access-Control-Allow-Origin',      '*')
        web.header('Access-Control-Allow-Credentials', 'true')
        data = json.loads(web.data())
        base64image = data['base64image']
        image_path = os.path.join(IMAGE_CACHE, str(app.image_id) + '.img');
        with open(image_path, 'w+b') as f:
            f.write(base64.b64decode(base64image));
        app.reco_process.stdin.write(str.encode(image_path + '\n'))
        app.reco_process.stdin.flush()
        result = app.reco_process.stdout.readline().decode('utf-8')

        print(result)
        app.image_id = app.image_id + 1
        return json.dumps(result)
    
    def OPTIONS(self):
        web.header('Access-Control-Allow-Origin',      '*')
        web.header('Access-Control-Allow-Credentials', 'true')
        web.header('Access-Control-Allow-Headers', 'Origin, Content-Type')


if __name__ == "__main__":
    app.run()
