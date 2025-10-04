#include <uart_transport_prot.h>

TransportProt::TransportProt(uint16_t (*tx_space)(),
        void (*tx_byte)(uint8_t),
        void (*process_message)(uint8_t,const uint8_t*, size_t))
: seq_num(false), ack_num(false), acknowledged(true),
tx_space(tx_space),tx_byte(tx_byte),process_message(process_message), message()
{
}


#define INIT_CODE 0xAF
#define INIT_CODE_REPLACE 0xA0
#define END_CODE 0xEF
#define END_CODE_REPLACE 0xE0
#define SPECIAL_CODE 0xBF
#define SPECIAL_CODE_REPLACE 0xB0
size_t TransportProt::translate_to_mac(uint8_t* mac,const uint8_t* message,const size_t message_size)
{
    size_t mac_size = 0;
    for(size_t i = 0;i<message_size;i++)
    {
        switch(message[i])
        {
            case INIT_CODE:
                mac[mac_size] = SPECIAL_CODE;
                    mac_size++;
                mac[mac_size] = INIT_CODE_REPLACE;
            break;
            case END_CODE:
                mac[mac_size] = SPECIAL_CODE;
                    mac_size++;
                mac[mac_size] = END_CODE_REPLACE;
            break;
            case SPECIAL_CODE:
                mac[mac_size] = SPECIAL_CODE;
                mac_size++;
                mac[mac_size] = SPECIAL_CODE_REPLACE;
            break;
            default:
                mac[mac_size] = message[i];
        }
        mac_size++;
    }
    return mac_size;
}

size_t TransportProt::translate_from_mac(uint8_t* message,const uint8_t* mac,const size_t mac_size)
{
    size_t message_size = 0;
    for(size_t i = 0;i<mac_size;i++)
    {
        if(mac[i]==SPECIAL_CODE)
        {
            i++;
            switch(mac[i])
            {
                case INIT_CODE_REPLACE:
                    message[message_size] = INIT_CODE;
                break;
                case END_CODE_REPLACE:
                    message[message_size] = END_CODE;
                break;
                case SPECIAL_CODE_REPLACE:
                    message[message_size] = SPECIAL_CODE;
                break;
                default:
                break;
            }
            
        } else
        {
            message[message_size] = mac[i];
        } 
        message_size++;
    }
    return mac_size;
}

bool TransportProt::send_message(uint8_t id,uint8_t* buffer,size_t buffer_len)
{
    if(buffer_len+3>256 || !acknowledged || message_to_send)
        return false;

    this->id = id;
    memcpy(message,buffer,buffer_len);
    message_size = buffer_len;
    message_to_send = true;
    return true;

}


void TransportProt::poll(uint8_t* buffer,size_t buffer_len)
{
    static uint8_t mac_message[512];
    static size_t mac_size = 0;
    static unsigned long last_sent = millis();
    static uint32_t cur_resend=0;
    static bool receiving = false;
    static uint8_t message[256];
    static size_t message_size;
    for(int i =0; i<buffer_len;i++)
    {
        if(receiving)
        {

            if(buffer[i] == END_CODE || mac_size >= sizeof(mac_message))
            {
                receiving = false;
                message_size = translate_from_mac(message,mac_message,mac_size);
                if(checksum(message,message_size-1) == message[message_size-1])
                {
                    internal_process_message(message[1],message+2,message_size-3,message[0]&0b1, message[0]&0b10, message[0]&0b100);
                }
                mac_size = 0;
            } else
            {
                mac_message[mac_size++] = buffer[i];
            }
        } else if(buffer[i] == INIT_CODE)
        {
            receiving = true;
            mac_size = 0;
        }
    }
    //if has message to send
    if(acknowledged)
    {
        
        if(message_to_send)
        {
            acknowledged = false;
            send_final_message(this->id,this->message,this->message_size,seq_num,false,first_call);
            first_call = false;
            message_to_send = false;
        }


    //if awaiting a an ACK that has not come on the timeout
    } else if(RESEND_TIMEOUT < millis()-last_sent)
    {

        //resend current message
        send_final_message(this->id,this->message,this->message_size,seq_num,false,false);
        cur_resend++;
        if(cur_resend>= MAX_RESEND)
        {
            cur_resend = 0;
            acknowledged = true;
            first_call = true;
        }
        last_sent = millis();
    }
    //RECEIVE BUFFER, SEE IF THERE IS A MESSAGE IN IT
}

 void TransportProt::send_final_message(uint8_t id,uint8_t* message,size_t message_size,bool seq, bool ack, bool force)
 {
    uint8_t message_buffer[256];
    uint8_t mac_buffer[512];
    if(message_size+3 >= 256)
        return;
    Serial.printf("sent id: %d, seq %d, ack %d, force %d\n cur_seq %d, cur_ack %d\n",id,seq,ack,force,seq_num,ack_num);
    message_buffer[0] = (force? 0b100:00) | (ack? 0b10:0b00) | (seq ? 0b1 : 0b0);
    message_buffer[1] = id;
    memcpy(message_buffer+2,message,message_size);
    message_buffer[2+message_size] = checksum(message_buffer,message_size+2);
    size_t mac_size = translate_to_mac(mac_buffer,message_buffer,message_size+3);
    
    tx_byte(INIT_CODE);
    for(int i=0;i<mac_size;i++)
    {
        tx_byte(mac_buffer[i]);
    }
    tx_byte(END_CODE);
    
 }

 uint8_t TransportProt::checksum(uint8_t* buffer, size_t len)
 {
    uint8_t checksum = 0;
    for(int i =0 ;i<len;i++)
    {
        checksum ^= buffer[i];
    }
    return checksum;

 }

 void TransportProt::internal_process_message(uint8_t id,uint8_t* message,size_t message_size, bool seq,bool ack, bool force)
 {
    Serial.printf("received id: %d, seq %d, ack %d, force %d\n cur_seq %d, cur_ack %d\n",id,seq,ack,force,seq_num,ack_num);
    if(ack)
    {
        if(seq == seq_num)
        {
            acknowledged = true;
            seq_num = !seq_num;
        }
        return;
    }

    if(force)
    {
        ack_num = seq;
    }

    send_final_message(id,nullptr,0,seq,true);
    if(seq == ack_num)
    {
        ack_num = !ack_num;
        process_message(id,message,message_size);
        
    }
 }