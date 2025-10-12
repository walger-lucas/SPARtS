#pragma once
#include <Arduino.h>
#include <freertos/FreeRTOS.h>

class TransportProt {
private:
    static constexpr TickType_t RESEND_TIMEOUT = 800;
    static constexpr size_t MAX_RESEND = 10000;
    bool ack_num: 1;
    bool seq_num: 1;
    bool acknowledged;
    
    bool first_call = true;

    uint16_t (*tx_space)();
    void (*tx_byte)(uint8_t);
    void (*process_message)(uint8_t,const uint8_t*, size_t);
    uint8_t message[256];
    size_t message_size = 0;
    bool message_to_send = false;
    uint8_t id;

    static size_t translate_to_mac(uint8_t* mac,const uint8_t* message,const size_t message_size);
    static size_t translate_from_mac(uint8_t* message,const uint8_t* mac,const size_t mac_size);
    static uint8_t checksum(uint8_t* buffer, size_t len);
    void send_final_message(uint8_t id,uint8_t* message,size_t message_size,bool seq, bool ack = false, bool force= false);
    void internal_process_message(uint8_t id,uint8_t* message,size_t message_size,bool seq, bool ack, bool force);
public:

    TransportProt(uint16_t (*tx_space)(),
        void (*tx_byte)(uint8_t),
        void (*process_message)(uint8_t,const uint8_t*, size_t));

    void poll(uint8_t* buffer,size_t buffer_len);
    bool send_message(uint8_t id,uint8_t* buffer,size_t buffer_len);

};