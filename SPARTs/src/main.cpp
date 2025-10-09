#include <cam_protocol.h>
include "load_cell.h"

/*GPIO ======================================= */
const byte DOUT_PIN = 25; //Data pin (DT).
const byte SCK_PIN = 26;  //Clock pin (SCK).
/*============================================ */

LoadCell myLoadCell(DOUT_PIN, SCK_PIN);//Create LoadCell object.

#ifdef CAM_MASTER
void setup(){
  Serial.begin(115200); 
  Serial.printf("start\n");
  cam::CamCommunicationMaster::setup_comm();
  cam::CamCommunicationMaster::send_config("","","",pdMS_TO_TICKS(30000));
  
  delay(1000); //To stabilize the serial.
  Serial.println("--- Load Cell Reader ---");

  myLoadCell.setup();//Initializes the load cell.

  Serial.println("Setup complete. Readings will start shortly.");
  Serial.println("Send 't' via serial to tare the scale.");
}
void loop(){
  /*UART test========*/
  //vTaskDelay(pdMS_TO_TICKS(1000));
  //cam::CamCommunicationMaster::process_image(pdMS_TO_TICKS(30000));
  /*=================*/

  if (Serial.available() > 0) {
    char command = Serial.read();
    if (command == 't' || command == 'T') {
      myLoadCell.tare(); //Reset the scale.
    }
  }

  float weight = myLoadCell.getWeight();
  Serial.printf("Weight: %.2f g\n", weight); //Print the weight with 2 decimal places.

  delay(100); //Wait 0.1 second between readings.
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

