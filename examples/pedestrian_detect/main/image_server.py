from flask import Flask, send_file, request, abort
import os

app = Flask(__name__)

# CHANGE THIS to the actual folder where your images are
IMAGE_FOLDER = "/Users/mahikamaini/Downloads/seattleish-camera-traps"

@app.route('/image')
def serve_image():
    filename = request.args.get('name')
    if not filename:
        abort(400, "Missing image filename parameter")

    # Prevent going outside the folder
    safe_path = os.path.abspath(os.path.join(IMAGE_FOLDER, filename))
    if not safe_path.startswith(os.path.abspath(IMAGE_FOLDER)):
        abort(403, "Forbidden")

    if not os.path.isfile(safe_path):
        abort(404, "Image not found")

    return send_file(safe_path, mimetype='image/jpeg')

if __name__ == '__main__':
    app.run(host='0.0.0.0', port=8000)
