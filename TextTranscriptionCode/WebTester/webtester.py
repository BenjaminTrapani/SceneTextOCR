import requests
import base64
import json
import sys

if (len(sys.argv) != 2):
    print("Usage: ./webtester.py [path to image to recognize]")

image_file_handle = open(sys.argv[1], "rb")
image_file_data = image_file_handle.read()
image_file_handle.close()

base64image = base64.b64encode(image_file_data)

base64imagestring = base64image.decode('utf-8')
payload = {'base64image': base64imagestring}
headers = {'Content-Type': 'application/json'}

r = requests.post("https://skytopsoftware.ddns.net:63264/SceneTextOCR/Api/recognize", headers=headers, data=json.dumps(payload))
print("Recognized text: " + r.text)
