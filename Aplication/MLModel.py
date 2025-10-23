import numpy as np
from ultralytics import YOLO, SAM
import supervision as sv
import torch
import gc

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
    def __init__(self, weights='yolo11n.pt', device='cuda' if torch.cuda.is_available() else 'cpu', conf=0.25):
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
        if hasattr(self, 'model'):
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

    def predict(self, processed_image, draw = False):
        """ Perform predictions of detected objects """
        # Check if the image is in cv format
        if not isinstance(processed_image, (np.ndarray, np.generic)):
            print(type(processed_image))
            raise ValueError("Input image must be a valid OpenCV image (numpy array).")
        
        # Verify if the model is loaded
        if not hasattr(self, 'model'):
            self.load_model(
                weights=self.weights,
                task=self.task
            )
            raise RuntimeWarning("The model is not loaded. Loading now, in runtime.")

        # Predict and format detection
        results = self.loaded_model.predict(processed_image, conf=self.conf, device=self.device)
        detections = sv.Detections(
            xyxy=results[0].boxes.xyxy.cpu().numpy(),
            confidence=results[0].boxes.conf.cpu().numpy(),
            class_id=results[0].boxes.cls.cpu().numpy().astype(int),
            data={"xyxyn": results[0].boxes.xyxyn.cpu().numpy()}
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
        else:
            response["item_name"] = self.CLASS_NAMES_DICT[unique_classes[0]]
            response["amount"] = len(detections.class_id)

        return response, annotated_img