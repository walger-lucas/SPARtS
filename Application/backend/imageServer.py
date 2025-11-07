import numpy as np
from ultralytics import YOLO
import supervision as sv
import torch
import cv2
import gc
from enum import Enum
from http.server import HTTPServer, BaseHTTPRequestHandler
import os, json, socket

# Debug macros
DEBUG = False
IMAGE_DIR = "images"
# Server macros
PORT = 9000
# Image macros
IMG_WIDTH = 1600
IMG_HEIGHT = 1200
PERCENT_X_INIT_CROP, PERCENT_X_FINAL_CROP = 0.3, 0.75
PERCENT_Y_INIT_CROP, PERCENT_Y_FINAL_CROP = 0.25, 0.9
# ML Model macros
WEIGHTS = 'mediumv5.pt'
CONF = 0.76

class MLModel():
    """
    A class that processes OpenCV format images using Ultralytics YOLO detection
    and predicts object bounding box in the image.

    ## Parameters:
    - `weights` (string)
    Path of the selected YOLO model. 
    - `device` (str)
    Specifies the device for inference (e.g., cpu, cuda:0 or 0).. 
    - `conf` (float)
    Confidence threshold for detecting a valid bounding box. 
    """
    def __init__(self, weights='yolo11n.pt', device='cpu', conf=0.25):
        self.weights = weights
        self.device = device
        self.conf = conf
        self.load_model(
            weights=self.weights
        )

    def load_model(self, weights):
        """ Loads the YOLO model with the selected parameters"""
        self.loaded_model = YOLO(weights)
        self.loaded_model.fuse()
        self.CLASS_NAMES_DICT = self.loaded_model.model.names

    def unload_model(self):
        """ Unloads the model and stops memory usage """
        if hasattr(self, 'loaded_model'):
            self.loaded_model.to("meta")
            del self.loaded_model
            self.loaded_model = None

        # Run garbage collector
        gc.collect()

        # Empty PyTorch CUDA cache
        if torch.cuda.is_available():
            torch.cuda.empty_cache()
            torch.cuda.ipc_collect()

    def annotate_image(self, processed_image, detections, labels = []):
        """ Annotate detected bounding boxes with the detected objects """
        if len(labels) <= 0:
            labels = [
                    f"{self.CLASS_NAMES_DICT[class_name]} {confidence:.2f}"
                    for class_name, confidence
                    in zip(detections.class_id, detections.confidence)
                ]
        
        box_annotator = sv.BoxAnnotator(color_lookup=sv.ColorLookup.INDEX)
        label_annotator = sv.LabelAnnotator(color_lookup=sv.ColorLookup.INDEX)

        # First, annotate the boxes
        annotated_img = box_annotator.annotate(
            scene=processed_image.copy(), # It's good practice to work on a copy of the image
            detections=detections
        )

        # Then, annotate the labels on the already annotated image
        annotated_img = label_annotator.annotate(
            scene=annotated_img,
            detections=detections,
            labels=labels # Pass your list of labels here
        )

        return annotated_img

    def predict(self, image, draw = False):
        """ Perform predictions of detected objects """
        y_init, y_final = int(IMG_HEIGHT*PERCENT_Y_INIT_CROP), int(IMG_HEIGHT*PERCENT_Y_FINAL_CROP)
        x_init, x_final = int(IMG_WIDTH*PERCENT_X_INIT_CROP), int(IMG_WIDTH*PERCENT_X_FINAL_CROP)
        processed_image = cv2.imread(image)
        processed_image = processed_image[ y_init:y_final, x_init:x_final]

        # Check if the image is in cv format
        if not isinstance(processed_image, (np.ndarray, np.generic)):
            print(type(processed_image))
            raise ValueError("Input image must be a valid OpenCV image (numpy array).")
        
        # Verify if the model is loaded
        if not hasattr(self, 'loaded_model'):
            self.load_model(
                weights=self.weights,
            )
            raise RuntimeWarning("The model is not loaded. Loading now, in runtime.")

        # Predict and format detection
        results = self.loaded_model.predict(processed_image, conf=self.conf, device=self.device)
        detections = sv.Detections(
            xyxy=results[0].boxes.xyxy.cpu().numpy(),
            confidence=results[0].boxes.conf.cpu().numpy(),
            class_id=results[0].boxes.cls.cpu().numpy().astype(int)
        )

        # If draw, annotate the processed_image frame
        if draw:
            annotated_img = self.annotate_image(processed_image, detections)
        else:
            annotated_img = processed_image

        response = {
            "item_name": "None",
            "mixed": False,
            "amount": 0
        }

        # Check if there is more than one unique class_id in detections
        unique_classes = np.unique(detections.class_id)
        if len(unique_classes) > 1:
            response["mixed"] = True
        elif len(unique_classes) > 0:
            response["item_name"] = self.CLASS_NAMES_DICT[unique_classes[0]]
            response["amount"] = len(detections.class_id)

        return response, annotated_img

def next_filename(img_dir):
    files = [f for f in os.listdir(img_dir) if f.lower().endswith(".jpg")]
    count = len(files) + 1
    return os.path.join(img_dir, f"image_{count:04d}.jpg")

class ImageHandler(BaseHTTPRequestHandler):
    ml_model = None  # class attribute

    @classmethod
    def set_model(cls, model):
        cls.ml_model = model

    def do_POST(self):
        print("Image handler post")
        if self.path != "/image":
            self.send_error(404, "Not Found")
            return
        
        content_length = int(self.headers.get('Content-Length', 0))
        if content_length == 0:
            self.send_error(400, "No data received")
            return
        
        image_data = self.rfile.read(content_length)
        if DEBUG:
            os.makedirs(IMAGE_DIR, exist_ok=True)
            filename = next_filename(IMAGE_DIR)
        else:
            filename = next_filename("/tmp/") 
        with open(filename, "wb") as f:
            f.write(image_data)    

        response, annotated_image = self.ml_model.predict(filename, draw=DEBUG)
        
        if DEBUG:
            print("Model Response: " + response)
            annotated_filename = "det" + filename
            det_dir = "det" + IMAGE_DIR
            os.makedirs(det_dir, exist_ok=True)
            cv2.imwrite(annotated_filename, annotated_image)

        resp_bytes = json.dumps(response).encode('utf-8')
        
        self.send_response(200)
        self.send_header("Content-Type", "application/json")
        self.send_header("Content-Length", str(len(resp_bytes)))
        self.end_headers()
        self.wfile.write(resp_bytes)

    def do_GET(self):
        print("Image handler get")
        self.send_response(200)
        self.end_headers()
        self.wfile.write(b"Use POST /image to upload JPEG")

if __name__ == "__main__":
    ml_model = MLModel(weights=WEIGHTS, conf=CONF)
    ImageHandler.set_model(ml_model)
    server = HTTPServer(("", PORT), ImageHandler)
    print(f"  ➜  Image server running on http://localhost:{PORT}")
    server.serve_forever()