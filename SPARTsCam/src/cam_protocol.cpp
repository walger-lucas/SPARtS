#include "cam_protocol.h"
#include "Wire.h"
#include "string.h"
using namespace cam;
static uint16_t tx_space()
{
  uint16_t n = Serial1.availableForWrite();
  return n;
}

static void tx_byte(uint8_t byte)
{
  Serial1.write(&byte, 1U); 
}

#ifdef CAM_MASTER
#define CamCommunication CamCommunicationMaster
#else
#define CamCommunication CamCommunicationSlave
#endif

void CamCommunication::setup_comm(gpio_num_t rx_pin, gpio_num_t tx_pin)
{
    Serial1.begin(115200, SERIAL_8N1, rx_pin, tx_pin);
    mutex_uart_min = xSemaphoreCreateMutex();
    xSemaphoreGive(mutex_uart_min);
    min_events = xEventGroupCreate();
    xEventGroupClearBits(min_events,0xFFFF);
    xTaskCreate(uart_thread,"min_thread",configMINIMAL_STACK_SIZE*4,nullptr,configMAX_PRIORITIES-1,nullptr);
}

void CamCommunication::uart_thread(void* param)
{
    char buf[256];
    size_t buf_len;

    for(;;)
    {
        if (Serial1.available() > 0) 
        {
            buf_len = Serial1.readBytes(buf, min(256,Serial1.available()));

        } else {
            buf_len = 0;
        }
        xSemaphoreTake(mutex_uart_min,portMAX_DELAY);
        transport.poll((uint8_t *)buf, buf_len);
        xSemaphoreGive(mutex_uart_min);
        vTaskDelay(pdMS_TO_TICKS(10));
    }
}
SemaphoreHandle_t CamCommunication::mutex_uart_min {};
EventGroupHandle_t CamCommunication::min_events {};
TransportProt CamCommunication::transport = TransportProt(tx_space,tx_byte,CamCommunication::application_handler);


#ifdef CAM_MASTER
CamResult CamCommunicationMaster::result {};

void CamCommunicationMaster::application_handler(uint8_t min_id, uint8_t const *min_payload, size_t len_payload)
{
    //Serial.printf(" received %d (%d)\n",min_id,len_payload);
    if(len_payload==0)
        return;
    result.status = (CamStatus) min_payload[0];
    if(min_id == 0x1)
    {
        xEventGroupSetBits(min_events,SETUP_PROCESSED);
    }
    if(min_id == 0x2)
    {
        if(len_payload == 5)
        {
            result.item_code = min_payload[1];
            result.item_quantity = *((uint16_t*)(min_payload+2));
            result.mixed = min_payload[4];
        }
        xEventGroupSetBits(min_events,IMAGE_PROCESSED);
    }
}

CamStatus CamCommunicationMaster::send_config(const char* ssid,const char* pwd,const char* image_processing_hook, TickType_t timeout)
{

    char buffer[120];
    if(!xSemaphoreTake(CamCommunicationMaster::mutex_uart_min,timeout))
        return CamStatus::TIMEOUT;
    strncpy(buffer,ssid,30);
    strncpy(buffer+30,pwd,30);
    strncpy(buffer+60,image_processing_hook,60);

    xEventGroupClearBits(CamCommunicationMaster::min_events,CamCommunicationMaster::SETUP_PROCESSED);

    if(!transport.send_message( 0x1, (uint8_t *)buffer, 120))
    {
        xSemaphoreGive(CamCommunicationMaster::mutex_uart_min);
        return CamStatus::QUEUE_FULL;
    }

    xSemaphoreGive(CamCommunicationMaster::mutex_uart_min);

    auto events = xEventGroupWaitBits(min_events,CamCommunicationMaster::SETUP_PROCESSED,pdTRUE,pdTRUE,timeout);
    if(events & SETUP_PROCESSED)
    {
        if(!xSemaphoreTake(CamCommunicationMaster::mutex_uart_min,timeout))
            return CamStatus::TIMEOUT;
        CamStatus status = result.status;
        xSemaphoreGive(CamCommunicationMaster::mutex_uart_min);
        return status;
    }
    return CamStatus::TIMEOUT;
}

CamResult CamCommunicationMaster::process_image(TickType_t timeout)
{
    char buffer[1];
    if(!xSemaphoreTake(CamCommunicationMaster::mutex_uart_min,timeout))
        return {.status = TIMEOUT};
    buffer[0] = 0;
    xEventGroupClearBits(CamCommunicationMaster::min_events,CamCommunicationMaster::IMAGE_PROCESSED);
    if(!transport.send_message( 0x2, (uint8_t *)buffer, 1))
    {
        xSemaphoreGive(CamCommunicationMaster::mutex_uart_min);
        return {.status = QUEUE_FULL};
    }

    xSemaphoreGive(CamCommunicationMaster::mutex_uart_min);

    auto events = xEventGroupWaitBits(min_events,CamCommunicationMaster::IMAGE_PROCESSED,pdTRUE,pdTRUE,timeout);

    if(events & CamCommunicationMaster::IMAGE_PROCESSED)
    {

        if(!xSemaphoreTake(CamCommunicationMaster::mutex_uart_min,timeout))
            return {.status = TIMEOUT};
        CamResult res = result;
        xSemaphoreGive(CamCommunicationMaster::mutex_uart_min);
        return res;
    }
    return {.status = TIMEOUT};
}
#else
char CamCommunicationSlave::ssid[30]="", CamCommunicationSlave::pwd[30]="", CamCommunicationSlave::hook[60]="";

void CamCommunicationSlave::application_handler(uint8_t min_id, uint8_t const *min_payload, size_t len_payload)
{
    if(min_id == 0x1)
    {
        strncpy(ssid,(const char*) min_payload,30);
        strncpy(pwd,(const char*) (min_payload+30),30);
        strncpy(hook,(const char*) (min_payload+60),60);
        xEventGroupSetBits(min_events,SETUP_TO_PROCESS);
    }
    if(min_id == 0x2)
    {
        xEventGroupSetBits(min_events,IMAGE_TO_PROCESS);
    }
}

bool CamCommunicationSlave::send_result(CamResult result, TickType_t timeout)
{
    uint8_t buffer[5];

    if(!xSemaphoreTake(mutex_uart_min,timeout))
        return false;

    buffer[0] = result.status;
    buffer[1] = result.item_code;
    *((uint16_t*)(buffer+2)) = result.item_quantity;
    buffer[4] = result.mixed;

    if(!transport.send_message( 0x2, (uint8_t *)buffer, 5))
    {
        xSemaphoreGive(mutex_uart_min);
        return false;
    }
    xSemaphoreGive(mutex_uart_min);
    return true;
}

bool CamCommunicationSlave::send_status(CamStatus status, TickType_t timeout)
{
    uint8_t buffer[1];

    if(!xSemaphoreTake(mutex_uart_min,timeout))
        return false;

    buffer[0] = status;


    if(!transport.send_message( 0x1, (uint8_t *)buffer, 1))
    {
        xSemaphoreGive(mutex_uart_min);
        return false;
    }
    xSemaphoreGive(mutex_uart_min);
    return true;
}
CamCommunicationSlave::MinEventBits CamCommunicationSlave::wait_for_message()
{

    auto bit = xEventGroupWaitBits(min_events,IMAGE_TO_PROCESS|SETUP_TO_PROCESS,true,false,portMAX_DELAY);
    if(bit & SETUP_TO_PROCESS)
        return SETUP_TO_PROCESS;
    else if(bit & IMAGE_TO_PROCESS)
        return IMAGE_TO_PROCESS;
}

#endif