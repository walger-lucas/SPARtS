#pragma once
#include "Arduino.h"
#include "freertos/FreeRTOS.h"
#include "uart_transport_prot.h"
//#define CAM_MASTER
namespace cam
{
    enum CamStatus: uint8_t 
    {
        WIFI_CONNECTED = 0x1,
        SERVER_CONNECTED = 0x2,
        PROCESS_OK = 0x4,
        TIMEOUT = 0x8,
        QUEUE_FULL = 0x16
    };

    struct CamResult 
    {
        CamStatus status;
        uint8_t item_code;
        uint16_t item_quantity;
        bool mixed;
        inline const bool ok() {return status & PROCESS_OK;}
    };

    #ifdef CAM_MASTER
    class CamCommunicationMaster 
    {
    private:
    enum MinEventBits {
            IMAGE_PROCESSED = 0x1,
            SETUP_PROCESSED = 0x2,
    };
    static SemaphoreHandle_t mutex_uart_min;
    static EventGroupHandle_t min_events;
    static CamResult result;
    static TransportProt transport;
    
    CamCommunicationMaster() = delete;
    public:
        static void setup_comm(gpio_num_t rx_pin = gpio_num_t::GPIO_NUM_17, gpio_num_t tx_pin = gpio_num_t::GPIO_NUM_16);
        static void uart_thread(void* param);
        static CamStatus send_config(const char* ssid,const char* pwd,const char* image_processing_hook, TickType_t timeout = portMAX_DELAY);
        static void application_handler(uint8_t min_id, uint8_t const *min_payload, size_t len_payload);
        static CamResult process_image(TickType_t timeout = portMAX_DELAY);
    };
    #else
    class CamCommunicationSlave
    {
    private:

    static SemaphoreHandle_t mutex_uart_min;
    static EventGroupHandle_t min_events;
    static TransportProt transport;
    
    CamCommunicationSlave() = delete;
    public:
        static char ssid[30], pwd[30], hook[60];
        enum MinEventBits {
            IMAGE_TO_PROCESS = 0x1,
            SETUP_TO_PROCESS = 0x2,
        };
        static void setup_comm(gpio_num_t rx_pin = gpio_num_t::GPIO_NUM_3, gpio_num_t tx_pin = gpio_num_t::GPIO_NUM_1);
        static void uart_thread(void* param);
        static bool send_status(CamStatus status,TickType_t timeout);
        static void application_handler(uint8_t min_id, uint8_t const *min_payload, size_t len_payload);
        static bool send_result(CamResult result, TickType_t timeout);
        static MinEventBits wait_for_message();
        
    };
    #endif
}