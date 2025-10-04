#include <Arduino.h>
#include <cam_protocol.h>

cam::CamStatus setup_wifi(const char ssid[30], const char password[30],const char image_process_uri[60])
{
  cam::CamStatus status = cam::CamStatus::PROCESS_OK;


  
  //TODO



  return status;
}
cam::CamResult process_image()
{
  cam::CamResult result = {.status = cam::CamStatus::PROCESS_OK,.item_code = 1,.item_quantity=10,.mixed=false};
  digitalWrite(4,HIGH); // LED ON




  //TODO



  digitalWrite(4,LOW); // LED OFF
  return result;
}




void setup(){
  cam::CamCommunicationSlave::setup_comm();
  pinMode(4, OUTPUT);
}
void loop(){
  static char ssid[30],pwd[30],hook[60];
  switch(cam::CamCommunicationSlave::wait_for_message())
  {
    case cam::CamCommunicationSlave::IMAGE_TO_PROCESS:
        cam::CamCommunicationSlave::send_result({.status=cam::CamStatus::PROCESS_OK},portMAX_DELAY);
    break;
    case cam::CamCommunicationSlave::SETUP_TO_PROCESS:
        strcpy(ssid, cam::CamCommunicationSlave::ssid);
        strcpy(pwd, cam::CamCommunicationSlave::pwd);
        strcpy(hook, cam::CamCommunicationSlave::hook);
        cam::CamCommunicationSlave::send_status(setup_wifi(ssid,pwd,hook),portMAX_DELAY);
        break;
    default:
    break;
  }
  
}
