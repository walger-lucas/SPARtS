#!/usr/bin/env python3
from http.server import BaseHTTPRequestHandler, HTTPServer
import json, os, socket
import requests
import time
from enum import Enum
import tkinter as tk
from tkinter import ttk, messagebox, scrolledtext
import threading
from http.server import ThreadingHTTPServer
from backend.imageServer import MLModel, ImageHandler, WEIGHTS, CONF

PORT = 9000

def get_local_ip():
    """Get the local IP address of the machine."""
    s = socket.socket(socket.AF_INET, socket.SOCK_DGRAM)
    try:
        # Doesn't need to connect for real — just to get the local IP used
        s.connect(("8.8.8.8", 80))
        ip = s.getsockname()[0]
    except Exception:
        ip = "127.0.0.1"
    finally:
        s.close()
    return ip

class SpartsOpCode(Enum):
    OK = 0
    OK_NEEDS_REORGANIZING = 1
    ERROR_OUTPUT_EMPTY = 2
    ERROR_OUTPUT_NOT_EMPTY = 3
    ERROR_BIN_NOT_FOUND = 4
    ERROR_FULL = 5
    ERROR_CAM = 6
    ERROR_MIXED_ITEM = 7

class ApiSparts:
    def __init__(self,image_url):
        self.url = "http://192.168.4.1"
        self.image_url = image_url
        self.status = SpartsOpCode.OK
        self.image_item_name = "None"
        self.bins = []
        pass

    def await_finish(self):
        time.sleep(2)
        resp = requests.get(self.url+"/status")
        data = resp.json()
        finished = resp.status_code == 200 and data["state"] == "FINISHED"
        while not finished:
            time.sleep(2)
            resp = requests.get(self.url+"/status")
            data = resp.json()
            print(data)
            finished = resp.status_code == 200 and data["state"] == "FINISHED"
        self.status = SpartsOpCode[data["status"]]
        self.image_item_name = data["item_name"]

    def setup(self):
        resp =requests.post(self.url+"/setup",json={"image_processing_uri":self.image_url})
        if resp.status_code!=200 :
            return False
        self.await_finish()
        return True
    
    def map(self):
        resp =requests.post(self.url+"/remap")
        if resp.status_code!=200 :
            return False
        self.await_finish()
        return True
    
    def organize(self):
        resp =requests.post(self.url+"/reorganize")
        if resp.status_code!=200 :
            return False
        self.await_finish()
        return True
    
    def auto_store(self):
        resp =requests.post(self.url+"/auto_store")
        if resp.status_code!=200 :
            return False
        self.await_finish()
        return True
    
    def image(self):
        resp =requests.post(self.url+"/capture_image")
        if resp.status_code!=200 :
            return False
        self.await_finish()
        return True
    
    def get_bins(self):
        resp =requests.get(self.url+"/bins")
        if resp.status_code!=200 :
            return False
        print(resp.text)
        data = resp.json()
        self.bins = data["bins"]
        return True
    
    def store(self,change_type = False, item_name_to_change = "None"):
        if change_type:
            resp =requests.post(self.url+"/store",json={"item_name":item_name_to_change})
        else:
            resp =requests.post(self.url+"/store",json={})
        if resp.status_code!=200 :
            return False
        self.await_finish()
        return True
    
    def retrieve(self, rfid_text = "000000000000000000000000"):
        resp =requests.post(self.url+"/retrieve",json={"rfid":rfid_text})
        if resp.status_code!=200 :
            return False
        self.await_finish()
        return True
    
    def read(self, id = 255):
        resp =requests.post(self.url+"/read",json={"id":id})
        if resp.status_code!=200 :
            return False
        self.await_finish()
        return True
    
    def debug_move(self, x = 0,y=0):
        resp =requests.post(self.url+"/debug/move",json={"x":x,"y":y})
        if resp.status_code!=200 :
            return False
        return True
    
    def debug_xy_calibrate(self):
        resp =requests.post(self.url+"/debug/calibrate/xy")
        if resp.status_code!=200 :
            return False
        return True
    
    def debug_conveyor_calibrate(self):
        resp =requests.post(self.url+"/debug/calibrate/conveyor")
        if resp.status_code!=200 :
            return False
        return True
    def debug_conveyor_next(self):
        resp =requests.post(self.url+"/debug/conveyor/next")
        if resp.status_code!=200 :
            return False
        return True
    
    def debug_platform_calibrate(self):
        resp =requests.post(self.url+"/debug/calibrate/platform")
        if resp.status_code!=200 :
            return False
        return True
    
    def debug_platform_extend(self):
        resp =requests.post(self.url+"/debug/platform/extend")
        if resp.status_code!=200 :
            return False
        return True
    
    def debug_platform_retract(self):
        resp =requests.post(self.url+"/debug/platform/retract")
        if resp.status_code!=200 :
            return False
        return True

class SpartsGUI:
    def __init__(self, root,api):
        self.root = root
        self.root.title("SPARTS API Interface")
        self.root.geometry("700x600")

        self.rfid_var = tk.StringVar(value="000000000000000000000000")
        self.id_var = tk.StringVar(value="0")
        self.item_name_var = tk.StringVar(value="None")
        self.x_var = tk.StringVar(value="0")
        self.y_var = tk.StringVar(value="0")

        self.api = api

        # UI Layout
        self.create_widgets()

    def create_widgets(self):
        frm = ttk.Frame(self.root, padding=10)
        frm.pack(fill="both", expand=True)

        # Action buttons
        actions = [
            ("Setup", self.run_threaded(self.call_setup)),
            ("Map", self.run_threaded(self.call_map)),
            ("Organize", self.run_threaded(self.call_organize)),
            ("Auto Store", self.run_threaded(self.call_auto_store)),
            ("Capture Image", self.run_threaded(self.call_image)),
            ("Store", self.run_threaded(self.call_store)),
            ("Retrieve", self.run_threaded(self.call_retrieve)),
            ("Bins", self.run_threaded(self.call_bins)),
            ("Read", self.run_threaded(self.call_read))
        ]
        for i, (label, cmd) in enumerate(actions):
            ttk.Button(frm, text=label, command=cmd, width=15).grid(row=1 + i // 3, column=i % 3, pady=5)

        # Debug section
        ttk.Label(frm, text="Debug Commands", font=("Arial", 10, "bold")).grid(row=5, column=0, pady=(15, 5), sticky="w")

        debug_actions = [
            ("Move XY", self.run_threaded(self.call_move)),
            ("XY Calibrate", self.run_threaded(self.call_debug_xy_cal)),
            ("Conveyor Calibrate", self.run_threaded(self.call_debug_conv_cal)),
            ("Platform Calibrate", self.run_threaded(self.call_debug_platform_cal)),
            ("Extend Platform", self.run_threaded(self.call_debug_platform_ext)),
            ("Retract Platform", self.run_threaded(self.call_debug_platform_ret)),
            ("Conveyor Next", self.run_threaded(self.call_debug_conv_next))
        ]
        for i, (label, cmd) in enumerate(debug_actions):
            ttk.Button(frm, text=label, command=cmd, width=20).grid(row=6 + i // 3, column=i % 3, pady=5)

        # Parameters for debug move and store
        ttk.Label(frm, text="RFID:").grid(row=9, column=0, sticky="e")
        ttk.Entry(frm, textvariable=self.rfid_var, width=30).grid(row=9, column=1, sticky="w")
        ttk.Label(frm, text="ID:").grid(row=10, column=0, sticky="e")
        ttk.Entry(frm, textvariable=self.id_var, width=30).grid(row=10, column=1, sticky="w")

        ttk.Label(frm, text="Item Name:").grid(row=11, column=0, sticky="e")
        ttk.Entry(frm, textvariable=self.item_name_var, width=30).grid(row=11, column=1, sticky="w")

        ttk.Label(frm, text="X:").grid(row=12, column=0, sticky="e")
        ttk.Entry(frm, textvariable=self.x_var, width=10).grid(row=12, column=1, sticky="w")

        ttk.Label(frm, text="Y:").grid(row=13, column=0, sticky="e")
        ttk.Entry(frm, textvariable=self.y_var, width=10).grid(row=13, column=1, sticky="w")

        # Console log
        ttk.Label(frm, text="Console Output:").grid(row=14, column=0, columnspan=3, pady=(10, 0), sticky="w")
        self.console = scrolledtext.ScrolledText(frm, width=80, height=10, state="disabled")
        self.console.grid(row=15, column=0, columnspan=3, pady=5)

    # --- Helper functions ---
    def log(self, text):
        self.console.config(state="normal")
        self.console.insert("end", text + "\n")
        self.console.see("end")
        self.console.config(state="disabled")

    def run_threaded(self, func):
        def runner():
            return lambda: threading.Thread(target=func, daemon=True).start()
        return runner()

    def call_setup(self):
        if not self.api: return
        self.log("Running setup()...")
        if self.api.setup():
            self.log("Setup completed successfully.")
        else:
            self.log("Setup failed.")
        self.log("Status: "+ self.api.status.name)

    def call_map(self):
        if not self.api: return
        self.log("Running map()...")
        self.log("Map success." if self.api.map() else "Map failed.")
        self.log("Status: "+self.api.status.name)

    def call_organize(self):
        if not self.api: return
        self.log("Running organize()...")
        self.log("Organize success." if self.api.organize() else "Organize failed.")
        self.log("Status: "+self.api.status.name)

    def call_auto_store(self):
        if not self.api: return
        self.log("Running auto_store()...")
        self.log("Auto store success." if self.api.auto_store() else "Auto store failed.")
        self.log("Status: "+self.api.status.name)

    def call_image(self):
        if not self.api: return
        self.log("Capturing image...")
        self.log(f"Image success. {self.api.image_item_name}" if self.api.image() else "Image failed.")
        self.log("Status: "+self.api.status.name)

    def call_store(self):
        if not self.api: return
        name = self.item_name_var.get()
        self.log(f"Storing item: {name}")
        if(name != ""):
            self.log("Store success." if self.api.store(True, name) else "Store failed.")
        else:
            self.log("Store success." if self.api.store(False, name) else "Store failed.")
        self.log("Status: "+self.api.status.name)

    def call_retrieve(self):
        if not self.api: return
        rfid = self.rfid_var.get()
        self.log(f"Retrieving item with RFID: {rfid}")
        self.log("Retrieve success." if self.api.retrieve(rfid) else "Retrieve failed.")
        self.log("Status: "+self.api.status.name)

    def call_read(self):
        if not self.api: return
        id = int(self.id_var.get())
        self.log(f"Reading Slot: {id}")
        self.log("Read success." if self.api.read(id) else "Read failed.")
        self.log("Status: "+self.api.status.name)

    def call_move(self):
        if not self.api: return
        x, y = int(self.x_var.get()), int(self.y_var.get())
        self.log(f"Debug move X={x}, Y={y}")
        self.log("Move success." if self.api.debug_move(x, y) else "Move failed.")

    def call_bins(self):
        if not self.api: return
        self.log("Success." if self.api.get_bins() else "Move failed.")
        self.log(str(self.api.bins))

    def call_debug_xy_cal(self):
        if not self.api: return
        self.log("XY Calibration...")
        self.log("Success." if self.api.debug_xy_calibrate() else "Failed.")

    def call_debug_conv_cal(self):
        if not self.api: return
        self.log("Conveyor Calibration...")
        self.log("Success." if self.api.debug_conveyor_calibrate() else "Failed.")
    
    def call_debug_conv_next(self):
        if not self.api: return
        self.log("Conveyor Next...")
        self.log("Success." if self.api.debug_conveyor_next() else "Failed.")

    def call_debug_platform_cal(self):
        if not self.api: return
        self.log("Platform Calibration...")
        self.log("Success." if self.api.debug_platform_calibrate() else "Failed.")

    def call_debug_platform_ext(self):
        if not self.api: return
        self.log("Platform Extend...")
        self.log("Success." if self.api.debug_platform_extend() else "Failed.")

    def call_debug_platform_ret(self):
        if not self.api: return
        self.log("Platform Retract...")
        self.log("Success." if self.api.debug_platform_retract() else "Failed.")

if __name__ == "__main__":
    ip = get_local_ip()
    url = f"http://{ip}:{PORT}/image"
    print(f"✅ Server running on {ip}:{PORT}")
    print(f"🔗 Arduino HTTPClient URL:\n    {url}\n")
    root = tk.Tk()
    api = ApiSparts(url)
    gui = SpartsGUI(root,api)
    # start HTTP server in a background thread and run the Tk mainloop
    weights_path = './backend/' + WEIGHTS
    ml_model = MLModel(weights=weights_path, conf=CONF)
    ImageHandler.set_model(ml_model)
    server = HTTPServer(("", PORT), ImageHandler)
    threading.Thread(target=server.serve_forever, daemon=True).start()
    print(f"✅ HTTP server thread listening on port {PORT}")

    def on_close():
        print("Shutting down HTTP server...")
        server.shutdown()
        server.server_close()
        root.destroy()

    root.protocol("WM_DELETE_WINDOW", on_close)
    root.mainloop()



