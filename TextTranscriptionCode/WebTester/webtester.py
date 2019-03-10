import requests
import base64
import json

image_file_handle = open("recurring.jpg", "rb")
image_file_data = image_file_handle.read()
image_file_handle.close()

base64image = base64.b64encode(image_file_data)
print(base64image)

base64imagestring = base64image.decode('utf-8')
payload = {'base64image': base64imagestring}
headers = {'Content-Type': 'application/json'}

r = requests.post("http://localhost:63264/Api/recognize", headers=headers, data=json.dumps(payload))
#r = requests.post("http://localhost:63264/Api/values", params={'value': 5})
print(r.text)
