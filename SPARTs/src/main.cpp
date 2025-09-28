#include <MFRC522v2.h>
#include <MFRC522DriverSPI.h>
#include <MFRC522DriverPinSimple.h>
#include <MFRC522Debug.h>

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
}
