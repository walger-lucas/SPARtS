/*#include <MFRC522v2.h>
#include <MFRC522DriverSPI.h>
#include <MFRC522DriverPinSimple.h>
#include <MFRC522Debug.h>
#include <cam_protocol.h>
// Custom SPI pins (must be compatible with ESP8266)
#define MY_SCK   27  // GPIO14
#define MY_MISO  12  // GPIO12
#define MY_MOSI  14  // GPIO13
#define MY_SS    26  // GPIO2 (or any free GPIO)

// Use default SPI instance

MFRC522DriverPinSimple ss_pin(MY_SS);
SPIClass &spiClass = SPI;
const SPISettings spiSettings = SPISettings(SPI_CLOCK_DIV4, MSBFIRST, SPI_MODE0);
MFRC522DriverSPI driver{ss_pin, spiClass, spiSettings}; // Create SPI driver.

MFRC522 mfrc522{driver}; // Create MFRC522 instance.

void setup() {
  //spiClass.begin(MY_SCK, MY_MISO, MY_MOSI, MY_SS); // Initialize SPI with custom pins.
  Serial.begin(115200); // Initialize serial communications with the PC for debugging.
  while (!Serial);      // Do nothing if no serial port is opened (added for Arduinos based on ATMEGA32U4).
   // Set custom SPI pins.
  Serial.print("hey\n");
  //SPI.begin(MY_SCK, MY_MISO, MY_MOSI, MY_SS); // Initialize SPI with custom pins.
  mfrc522.PCD_Init();   // Init MFRC522 board.
  MFRC522Debug::PCD_DumpVersionToSerial(mfrc522, Serial);	// Show details of PCD - MFRC522 Card Reader details.
	Serial.println(F("Scan PICC to see UID, SAK, type, and data blocks..."));
}

void loop() {
  MFRC522::Uid uid;
  if (mfrc522.PICC_IsNewCardPresent() && mfrc522.PICC_ReadCardSerial()) {
    uid = mfrc522.uid;
    Serial.print("Card UID: ");
    for (byte i = 0; i < uid.size; i++) {
      Serial.printf("%02X ", uid.uidByte[i]);
    }
    Serial.println();
    mfrc522.PICC_HaltA();
  }
}*/
/*
#include "HX711.h"

// HX711 circuit wiring
const int LOADCELL_DOUT_PIN = 32;
const int LOADCELL_SCK_PIN = 33;

HX711 scale;

void setup() {
  Serial.begin(57600);
  scale.begin(LOADCELL_DOUT_PIN, LOADCELL_SCK_PIN);
}

void loop() {

  if (scale.is_ready()) {
    long reading = scale.read();
    Serial.print("HX711 reading: ");
    Serial.println(reading);
  } else {
    Serial.println("HX711 not found.");
  }

  delay(1000);
  
}*/



#include <Arduino.h>

// Task handles (optional)
TaskHandle_t senderTaskHandle;
TaskHandle_t receiverTaskHandle;
/*
// --- Echo sender task ---
void senderTask(void *pvParameters) {
  for (;;) {
    Serial1.println("Hello from sender");  // Send to Serial1
    vTaskDelay(pdMS_TO_TICKS(1000));       // 1s delay
  }
}

// --- Echo receiver task ---
void receiverTask(void *pvParameters) {
  for (;;) {
    while (Serial1.available()) {
      char c = Serial1.read();             // Read byte from Serial1
      Serial.print(c);                     // Echo to Serial monitor
    }
    vTaskDelay(pdMS_TO_TICKS(10));         // Small yield
  }
}

void setup() {
  Serial.begin(115200);      // Debug output to USB
  Serial1.begin(9600, SERIAL_8N1, 16, 17); // TX=17, RX=16

  // Create FreeRTOS tasks
  xTaskCreate(senderTask, "Sender", 2048, nullptr, 1, &senderTaskHandle);
  xTaskCreate(receiverTask, "Receiver", 2048, nullptr, 1, &receiverTaskHandle);
}

void loop() {
  // Nothing here, tasks handle everything
}*/
#include <cam_protocol.h>
#ifdef CAM_MASTER
void setup(){
  Serial.begin(115200); 
  Serial.printf("start\n");
  cam::CamCommunicationMaster::setup_comm();
  cam::CamCommunicationMaster::send_config("","","",pdMS_TO_TICKS(30000));
  
}
void loop(){
  vTaskDelay(pdMS_TO_TICKS(1000));
  cam::CamCommunicationMaster::process_image(pdMS_TO_TICKS(30000));
}
#else

void setup(){
  Serial.begin(115200); 
  Serial.printf("start\n");
  cam::CamCommunicationSlave::setup_comm(GPIO_NUM_17,GPIO_NUM_16);
  pinMode(4, OUTPUT);
}

void loop(){
  switch(cam::CamCommunicationSlave::wait_for_message())
  {
    case cam::CamCommunicationSlave::IMAGE_TO_PROCESS:
      cam::CamCommunicationSlave::send_result({.status=cam::CamStatus::PROCESS_OK},portMAX_DELAY);
    break;
        case cam::CamCommunicationSlave::SETUP_TO_PROCESS:
      cam::CamCommunicationSlave::send_status(cam::CamStatus::PROCESS_OK,portMAX_DELAY);
    break;

  }
  static bool out4 = false;
  digitalWrite(4,out4);
  out4 = !out4;
}
#endif
