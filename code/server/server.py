import os
import cv2
import numpy as np
from flask import Flask, request, jsonify
from werkzeug.utils import secure_filename
import requests
from datetime import datetime

app = Flask(__name__)

# Folder to save uploaded images
UPLOAD_FOLDER = 'uploads'
os.makedirs(UPLOAD_FOLDER, exist_ok=True)
app.config['UPLOAD_FOLDER'] = UPLOAD_FOLDER

# Allowed extensions for file uploads
ALLOWED_EXTENSIONS = {'jpg', 'jpeg', 'png'}

def allowed_file(filename):
    return '.' in filename and filename.rsplit('.', 1)[1].lower() in ALLOWED_EXTENSIONS

@app.route('/upload', methods=['POST'])
def upload():
    try:
        if 'file' not in request.files:
            return "No file part", 400
        file = request.files['file']
        if file.filename == '':
            return "No selected file", 400
        
        # Save the file
        if file and allowed_file(file.filename):
            filename = secure_filename(file.filename)
            filepath = os.path.join(app.config['UPLOAD_FOLDER'], filename)
            file.save(filepath)
            
            print(f"Received file: {filename}")
            
            # Read image for further processing
            img = cv2.imread(filepath)
            if img is None:
                return "Error reading image", 400
            
            # Convert to grayscale (optional)
            gray_img = cv2.cvtColor(img, cv2.COLOR_BGR2GRAY)
            gray_filepath = os.path.join(app.config['UPLOAD_FOLDER'], f"gray_{filename}")
            cv2.imwrite(gray_filepath, gray_img)
            
            # Send alert to Telegram
            send_alert(filename, gray_filepath)
            
            return jsonify({"message": "Image uploaded successfully!", "filename": filename}), 200
        else:
            return "Invalid file type", 400
    
    except Exception as e:
        print(f"Error: {str(e)}")
        return "Error processing image", 500

def send_alert(image_filename, file_path):
    try:
        # Telegram credentials
        bot_token = "your bot token"
        chat_id = "your bot id"
        
        # Get current timestamp
        timestamp = datetime.now().strftime("%Y-%m-%d %H:%M:%S")
        caption = f"📸 Motion Detected!\n🕒 {timestamp}"

        # Telegram API endpoint
        url = f"https://api.telegram.org/bot{bot_token}/sendPhoto"
        
        with open(file_path, 'rb') as photo:
            response = requests.post(
                url,
                data={
                    'chat_id': chat_id,
                    'caption': caption
                },
                files={'photo': photo}
            )
        
        if response.status_code == 200:
            print(f"Alert sent to Telegram with image: {image_filename}")
        else:
            print(f"Error sending alert: {response.text}")
    except Exception as e:
        print(f"Error sending alert: {str(e)}")

if __name__ == '__main__':
    app.run(host='0.0.0.0', port=5000, debug=True)
