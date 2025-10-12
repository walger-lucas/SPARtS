#include "Controls.h"

// Definição dos pinos (motor esquerdo)
#define STEP_LEFT 25
#define DIR_LEFT 26

// Definição dos pinos (motor direito)
#define STEP_RIGHT 27
#define DIR_RIGHT 14

controls::ConveyorControl conveyor;

void setup() {
  Serial.begin(115200);
  
  // Configuração dos motores
  delay(1000);
  conveyor.start();
  delay(1000);
  conveyor.next();
  /*
  stepMotor.goToBin(2,2);
  delay(2000);
  stepMotor.goToBin(0,0);
  delay(2000);
  stepMotor.goToBin(3,3);
  delay(2000);
  stepMotor.goToBin(0,0);
  delay(2000);
  stepMotor.goToBin(5,5);*/
}

void loop() {
  // StepMotor stepMotor;
  //stepMotor.goToBin(2,2);
  delay(5000);
  conveyor.next();
  
  // stepMotor.goToBin(3,3);
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
