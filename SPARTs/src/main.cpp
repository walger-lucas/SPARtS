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

