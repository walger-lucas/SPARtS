#include "SPARtSCore.h"
#if 1
// Definição dos pinos (motor esquerdo)
#define STEP_LEFT 25
#define DIR_LEFT 26

// Definição dos pinos (motor direito)
#define STEP_RIGHT 27
#define DIR_RIGHT 14

//list
/*
* RETEST CONVEYOR
* TEST RFID IN
* TEST READ
* TEST FETCH
* TEST STORE
* TEST AND FINDOUT THE PROBLEM WITH THE LOAD CELL
* MARK BINS POSITIONS AND TEST RELIABILITY

//SEGUNDA (TESTAR TUDO ISSO)
//TERÇA (FAZER OS DIAGRAMAS DE CASO DE USO)
//QUARTA (DIAGRAMAS E + TESTES)
//QUINTA (DIAGRAMAS)

// - (604,0) (907,0) (1208,0))

*/
SPARtSCore core;
void setup() {
  Serial.begin(115200);
  
  
  

}

void loop() {
  //conveyor
  core.run();

  
  // stepMotor.goToBin(3,3);
  
}
#else
#include <MFRC522v2.h>
#include <MFRC522DriverSPI.h>
#include <MFRC522DriverPinSimple.h>
#include <MFRC522Debug.h>

// Custom SPI pins (must be compatible with ESP8266)
#define MY_SCK   27  // GPIO14
#define MY_MISO  12  // GPIO12
#define MY_MOSI  14  // GPIO13
//#define MY_SS    26  // GPIO2 (or any free GPIO)

// Use default SPI instance

MFRC522DriverPinSimple ss_pin(15);
SPIClass &spiClass = SPI;
const SPISettings spiSettings = SPISettings(SPI_CLOCK_DIV4, MSBFIRST, SPI_MODE0);
MFRC522DriverSPI driver{ss_pin, spiClass, spiSettings}; // Create SPI driver.
controls::ConveyorControl conveyor;
controls::XYControl xy;
controls::PlatformControl platform;
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
}
/*
#include <cam_protocol.h>

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

*/
#endif
