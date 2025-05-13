import cv2
import numpy as np
import pyttsx3
import time
import tensorflow as tf
from PIL import Image

# Voice engine
try:
    engine = pyttsx3.init()
    engine.setProperty('rate', 160)
    voice_enabled = True
except Exception as e:
    print("Voice engine failed:", e)
    voice_enabled = False

# Load TFLite model
interpreter = tf.lite.Interpreter(model_path="ssd_mobilenet_v2.tflite")
interpreter.allocate_tensors()

input_details = interpreter.get_input_details()
output_details = interpreter.get_output_details()

height = input_details[0]['shape'][1]
width = input_details[0]['shape'][2]

# COCO labels (91 classes)
with open("coco_labels.txt", "r") as f:
    labels = [line.strip() for line in f.readlines()]

# Alert dictionary
alert_dict = {
    'person': 'Person ahead!',
    'car': 'Vehicle ahead!',
    'bicycle': 'Bicycle ahead!',
    'motorcycle': 'Motorcycle ahead!',
    'bus': 'Bus approaching!',
    'truck': 'Truck ahead!',
    'train': 'Train ahead!',
    'dog': 'Dog ahead!',
    'cat': 'Cat ahead!',
    'traffic light': 'Traffic light detected!',
    'stop sign': 'Stop sign detected!',
    'chair': 'Chair ahead!',
    'couch': 'Couch ahead!',
    'tv': 'Television ahead!',
    'laptop': 'Laptop detected!',
    'cell phone': 'Cell phone ahead!',
}

# Webcam
cap = cv2.VideoCapture(0)
if not cap.isOpened():
    print("Error: Could not open webcam.")
    exit()

last_alert_time = {}

print("Press Ctrl+C to stop...\n")

try:
    while True:
        ret, frame = cap.read()
        if not ret:
            print("Failed to grab frame.")
            break

        input_frame = cv2.resize(frame, (width, height))
        input_data = np.expand_dims(input_frame, axis=0)
        input_data = (np.float32(input_data) - 127.5) / 127.5  # Normalize

        interpreter.set_tensor(input_details[0]['index'], input_data)
        interpreter.invoke()

        boxes = interpreter.get_tensor(output_details[0]['index'])[0]
        classes = interpreter.get_tensor(output_details[1]['index'])[0].astype(np.int32)
        scores = interpreter.get_tensor(output_details[2]['index'])[0]

        current_time = time.time()
        found_any = False

        for i in range(len(scores)):
            if scores[i] > 0.5:
                class_id = classes[i]
                class_name = labels[class_id] if class_id < len(labels) else 'N/A'
                print(f" - {class_name}: {scores[i]:.2f}")
                found_any = True

                if class_name in alert_dict:
                    if (class_name not in last_alert_time) or (current_time - last_alert_time[class_name] > 3):
                        if voice_enabled:
                            engine.say(alert_dict[class_name])
                            engine.runAndWait()
                        last_alert_time[class_name] = current_time

        if not found_any:
            print(" - No high confidence objects detected.")
        print("-" * 40)

except KeyboardInterrupt:
    print("Stopped by user.")

cap.release()
cv2.destroyAllWindows()
