
//#define IMAGE_SERVER
#ifndef IMAGE_SERVER
#include <Arduino.h>
#include <ArduinoJson.h>
#include <cam_protocol.h>
#include <WiFi.h>
#include "esp_camera.h"
#include <HTTPClient.h>
#include "Item.h"

#define PWDN_GPIO_NUM     32
#define RESET_GPIO_NUM    -1
#define XCLK_GPIO_NUM      0
#define SIOD_GPIO_NUM     26
#define SIOC_GPIO_NUM     27
#define Y9_GPIO_NUM       35
#define Y8_GPIO_NUM       34
#define Y7_GPIO_NUM       39
#define Y6_GPIO_NUM       36
#define Y5_GPIO_NUM       21
#define Y4_GPIO_NUM       19
#define Y3_GPIO_NUM       18
#define Y2_GPIO_NUM        5
#define VSYNC_GPIO_NUM    25
#define HREF_GPIO_NUM     23
#define PCLK_GPIO_NUM     22
String serverUrl= "";

cam::CamStatus setup_wifi(const char ssid[30], const char password[30],const char image_process_uri[60])
{
  cam::CamStatus status = cam::CamStatus::PROCESS_OK;
  WiFi.begin(ssid, password);
  uint32_t timeout_ms = 5000;
  uint32_t start = millis();
  while (WiFi.status() != WL_CONNECTED && millis() - start < timeout_ms) {
    delay(10);
  }

  if(WiFi.status()!= WL_CONNECTED)
    cam::CamStatus status = cam::CamStatus::TIMEOUT;

  serverUrl = image_process_uri;

  return status;
}
cam::CamResult process_image()
{
  cam::CamResult result = {.status = cam::CamStatus::TIMEOUT,.item_code = 1,.item_quantity=10,.mixed=false};
  //digitalWrite(4,HIGH); // LED ON
  
   
   for(int i =0; i<5;i++){
      camera_fb_t * fb = esp_camera_fb_get();
      if (fb) esp_camera_fb_return(fb);
      delay(50);
   }
   camera_fb_t * fb = esp_camera_fb_get();
    if (!fb) {
      digitalWrite(4,LOW);
      return result;
    }
    if (WiFi.status() != WL_CONNECTED) {
      digitalWrite(4,LOW);
      esp_camera_fb_return(fb);
      return result;
  }
  delay(20);
  digitalWrite(4,LOW);
  HTTPClient http;
  http.begin(serverUrl);
  http.addHeader("Content-Type", "image/jpeg");

  // Send binary buffer directly
  int httpResponseCode = http.POST(fb->buf, fb->len);
  esp_camera_fb_return(fb);
  if (httpResponseCode <= 0 || httpResponseCode != HTTP_CODE_OK ) {
    http.end();
    return result;
  }
  
  String payload = http.getString();
  http.end();

  StaticJsonDocument<512> doc;
  //if error deserializing
  if(deserializeJson(doc, payload))
  {
      return result;
  }

  if(doc.containsKey("item_name") && doc.containsKey("amount") && doc.containsKey("mixed"))
  {
    String name = doc["item_name"] | "None";
    result.item_code = Item::getId(name);
    result.item_quantity = doc["amount"].as<uint16_t>();
    result.mixed = doc["mixed"].as<bool>();
    result.status = cam::CamStatus::PROCESS_OK;
  }
  
  return result;
}




void setup(){
  camera_config_t config;
  config.ledc_channel = LEDC_CHANNEL_0;
  config.ledc_timer = LEDC_TIMER_0;
  config.pin_d0 = Y2_GPIO_NUM;
  config.pin_d1 = Y3_GPIO_NUM;
  config.pin_d2 = Y4_GPIO_NUM;
  config.pin_d3 = Y5_GPIO_NUM;
  config.pin_d4 = Y6_GPIO_NUM;
  config.pin_d5 = Y7_GPIO_NUM;
  config.pin_d6 = Y8_GPIO_NUM;
  config.pin_d7 = Y9_GPIO_NUM;
  config.pin_xclk = XCLK_GPIO_NUM;
  config.pin_pclk = PCLK_GPIO_NUM;
  config.pin_vsync = VSYNC_GPIO_NUM;
  config.pin_href = HREF_GPIO_NUM;
  config.pin_sscb_sda = SIOD_GPIO_NUM;
  config.pin_sscb_scl = SIOC_GPIO_NUM;
  config.pin_pwdn = PWDN_GPIO_NUM;
  config.pin_reset = RESET_GPIO_NUM;
  config.xclk_freq_hz = 20000000;
  config.pixel_format = PIXFORMAT_JPEG;
  config.frame_size = FRAMESIZE_UXGA; // (1600x1200)
  config.jpeg_quality = 10;
  config.fb_count = 2;
  esp_camera_init(&config);
  cam::CamCommunicationSlave::setup_comm();
  pinMode(4, OUTPUT);
}
void loop(){
  static char ssid[30],pwd[30],hook[60];
  switch(cam::CamCommunicationSlave::wait_for_message())
  {
    case cam::CamCommunicationSlave::IMAGE_TO_PROCESS:
        cam::CamCommunicationSlave::send_result(process_image(),pdMS_TO_TICKS(15000));
    break;
    case cam::CamCommunicationSlave::SETUP_TO_PROCESS:
        strcpy(ssid, cam::CamCommunicationSlave::ssid);
        strcpy(pwd, cam::CamCommunicationSlave::pwd);
        strcpy(hook, cam::CamCommunicationSlave::hook);
        cam::CamCommunicationSlave::send_status(setup_wifi(ssid,pwd,hook),pdMS_TO_TICKS(15000));
        break;
    default:
    break;
  }
  
}

#else 
#include <Arduino.h>
#include "esp_camera.h"
#include <WiFi.h>
#include <HTTPClient.h>
#include "credentials.hpp"

// Pin definition for AI-Thinker ESP32-CAM
#define PWDN_GPIO_NUM     32
#define RESET_GPIO_NUM    -1
#define XCLK_GPIO_NUM      0
#define SIOD_GPIO_NUM     26
#define SIOC_GPIO_NUM     27
#define Y9_GPIO_NUM       35
#define Y8_GPIO_NUM       34
#define Y7_GPIO_NUM       39
#define Y6_GPIO_NUM       36
#define Y5_GPIO_NUM       21
#define Y4_GPIO_NUM       19
#define Y3_GPIO_NUM       18
#define Y2_GPIO_NUM        5
#define VSYNC_GPIO_NUM    25
#define HREF_GPIO_NUM     23
#define PCLK_GPIO_NUM     22

#define TIME_TO_TAKE_PHOTO 3000 // Time interval between photos in milliseconds

// The server URL is constructed using the server IP from credentials.h
String serverUrl = "http://" + String(SERVER_IP) + ":" + String(PORT) + "/upload";

void setup() {
  Serial.begin(115200);
  Serial.println("Starting up...");

  camera_config_t config;
  config.ledc_channel = LEDC_CHANNEL_0;
  config.ledc_timer = LEDC_TIMER_0;
  config.pin_d0 = Y2_GPIO_NUM;
  config.pin_d1 = Y3_GPIO_NUM;
  config.pin_d2 = Y4_GPIO_NUM;
  config.pin_d3 = Y5_GPIO_NUM;
  config.pin_d4 = Y6_GPIO_NUM;
  config.pin_d5 = Y7_GPIO_NUM;
  config.pin_d6 = Y8_GPIO_NUM;
  config.pin_d7 = Y9_GPIO_NUM;
  config.pin_xclk = XCLK_GPIO_NUM;
  config.pin_pclk = PCLK_GPIO_NUM;
  config.pin_vsync = VSYNC_GPIO_NUM;
  config.pin_href = HREF_GPIO_NUM;
  config.pin_sscb_sda = SIOD_GPIO_NUM;
  config.pin_sscb_scl = SIOC_GPIO_NUM;
  config.pin_pwdn = PWDN_GPIO_NUM;
  config.pin_reset = RESET_GPIO_NUM;
  config.xclk_freq_hz = 20000000;
  config.pixel_format = PIXFORMAT_JPEG;
  config.frame_size = FRAMESIZE_UXGA; // (1600x1200)
  config.jpeg_quality = 10;
  config.fb_count = 2;

  // Initialize the camera
  esp_err_t err = esp_camera_init(&config);
  if (err != ESP_OK) {
    Serial.printf("Camera init failed with error 0x%x", err);
    return;
  }

  // Connect to Wi-Fi using credentials from credentials.h
  WiFi.begin(WIFI_SSID, WIFI_PASSWORD);
  Serial.print("Connecting to WiFi...");
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println("\nWiFi connected");
  Serial.print("ESP32-CAM IP Address: ");
  Serial.println(WiFi.localIP());
}

void loop() {
  camera_fb_t * fb = esp_camera_fb_get();
  if (!fb) {
    Serial.println("Camera capture failed");
    return;
  }

  if (WiFi.status() == WL_CONNECTED) {
    WiFiClient client;
    //Attempts to connect to the server on port 5000.
    if (!client.connect(SERVER_IP, 5000)) {
      Serial.println("Connection to server failed!");
      esp_camera_fb_return(fb);
      delay(TIME_TO_TAKE_PHOTO);
      return;
    }

    Serial.println("Connected to server, preparing to send image...");

    String boundary = "----WebKitFormBoundary7MA4YWxkTrZu0gW";
    String head = "--" + boundary + "\r\n" +
                  "Content-Disposition: form-data; name=\"image\"; filename=\"capture.jpg\"\r\n" +
                  "Content-Type: image/jpeg\r\n\r\n";
    String tail = "\r\n--" + boundary + "--\r\n";

    //Calculates the total size of the request content.
    uint32_t contentLength = head.length() + fb->len + tail.length();

    //Sends HTTP headers.
    client.println("POST /upload HTTP/1.1");
    client.println("Host: " + String(SERVER_IP));
    client.println("Content-Length: " + String(contentLength));
    client.println("Content-Type: multipart/form-data; boundary=" + boundary);
    client.println(); //Final blank line to separate headers from body

    //Send the body of the request in parts.
    client.print(head);
    client.write(fb->buf, fb->len);
    client.print(tail);

    //Serial.println("Image sent. Waiting for server response...");
    
    //Wait for the server's response for a while.
    /*long startTime = millis();
    while (client.connected() && !client.available() && millis() - startTime < 3000) {
      delay(100);
    }*/

    //Reads and prints the server response.
    /*if (client.available()) {
      Serial.println("--- Server Response ---");
      while (client.available()) {
        char c = client.read();
        Serial.write(c);
      }
      Serial.println("-----------------------");
    } else {
      Serial.println("No response from server.");
    }*/

    client.stop();
  } else {
    Serial.println("WiFi disconnected. Please check connection.");
  }

  esp_camera_fb_return(fb);
  delay(TIME_TO_TAKE_PHOTO);
}
#endif
