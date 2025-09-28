import os
from flask import Flask, request
from datetime import datetime
import threading
from dotenv import load_dotenv

# Load environment variables from .env file
load_dotenv()

# Variable to control the saving state
save_images = False
lock = threading.Lock()# Ensures that only one thread at a time executes the code

app = Flask(__name__)

# Create the directory to save images if it doesn't exist
if not os.path.exists('saved_images'):
    os.makedirs('saved_images')

@app.route('/upload', methods=['POST'])
def upload_image():
    global save_images
    if request.method == 'POST':
        try:
            image_file = request.files['image']
            
            with lock:
                if save_images:
                    timestamp = datetime.now().strftime('%Y-%m-%d_%H-%M-%S-%f')
                    filename = f'saved_images/photo_{timestamp}.jpg'
                    image_file.save(filename)
                    print(f'Image saved as: {filename}')
                else:
                    print('Image received but not saved (save mode is disabled).')
            
            return 'Image received successfully!', 200
        except Exception as e:
            print(f'Error receiving image: {e}')
            return 'Server error', 500

def keyboard_control():
    #Function to control the saving state via the keyboard.
    global save_images
    print("\n--- Server Control ---")
    print("Press 's' to ENABLE image saving.")
    print("Press 'd' to DISABLE image saving.")
    print("Press 'q' to quit.")
    print("----------------------\n")

    while True:
        try:
            command = input("Enter command: ").lower()
            with lock:
                if command == 's':
                    save_images = True
                    print("\n[STATE] Image saving ENABLED.\n")
                elif command == 'd':
                    save_images = False
                    print("\n[STATE] Image saving DISABLED.\n")
            if command == 'q':
                print("Shutting down the server...")
                os._exit(0)
        except (KeyboardInterrupt, EOFError):
            print("\nShutting down the server...")
            os._exit(0)


if __name__ == '__main__':
    keyboard_thread = threading.Thread(target=keyboard_control)
    keyboard_thread.daemon = True
    keyboard_thread.start()

    # Get server host and port from environment variables, with defaults
    host = os.getenv('SERVER_HOST', '0.0.0.0')
    port = int(os.getenv('SERVER_PORT', 5000))

    print(f"Starting server on {host}:{port}")
    app.run(host=host, port=port)