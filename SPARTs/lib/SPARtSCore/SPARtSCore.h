#pragma once
#include "Storage.h"
#include <freertos/FreeRTOS.h>
#include "Item.h"
#include "cam_protocol.h"
#include <AsyncTCP.h>
#include <ESPAsyncWebServer.h>
class SPARtSCore {
    const String ssid = "SPARtS";
    const String password = "sparts1234";
public:
    enum class State: uint8_t {
        INITIALIZING,
        AWAITING_SETUP,
        INITIALIZING_CAM,
        IDLE,
        RETRIEVE_ITEM,
        STORE_ITEM,
        AUTO_STORE,
        READ_BUCKET,
        REMAP,
        REORGANIZE,
        PROCESS_IMAGE,
    };
private:
    State current_state = State::INITIALIZING;
    EventGroupHandle_t evg;

    State next_state = State::IDLE;
    String uri;
    controls::rfid_t rfid;
    bool change_id = false;
    uint8_t item_id = 0;
    uint8_t bucket_id = 0;
    uint8_t last_object_sawn = 0;
    storage::Storage::OperationStatus last_storage_status = storage::Storage::OperationStatus::OK;
    AsyncWebServer server {80};

    storage::Storage::OperationStatus auto_store_state();
public:
    storage::Storage storage;
    controls::ConveyorControl conveyor;
    inline State getState() { return current_state; }
    void setState(State newState){  current_state = newState; }
    void run();
    void setupWebServer();


    bool setup(String uri) { 
        if(current_state == State::IDLE || current_state == State::AWAITING_SETUP ||current_state==State::INITIALIZING_CAM)
        {
            this->uri = uri; 
            next_state = State::INITIALIZING_CAM;
            xEventGroupSetBits(evg, 0x01);
            return true;
        } 
        return false;
    }
    String get_json_data()
    {
        String json;
        storage.getJson(json);
        return json;
    }
    String get_json_state()
    {
        String json;
        json = "{ \"state\":";
        if(getState()  == State::IDLE )
        {
            
            json += "\"FINISHED\",";
        } else if(getState()  == State::AWAITING_SETUP)
        {
            json += "\"SETUP\",";
        }
        else {
            json += "\"RUNNING\",";
        }
        json += "\"status\":";
        switch (last_storage_status)
        {
            case storage::Storage::OperationStatus::OK:
                json += "\"OK\"";
                break;
            case storage::Storage::OperationStatus::OK_NEEDS_REORGANIZING:
                json += "\"OK_NEEDS_REORGANIZING\"";
                break;
            case storage::Storage::OperationStatus::ERROR_OUTPUT_EMPTY:
                json += "\"ERROR_OUTPUT_EMPTY\"";
                break;
            case storage::Storage::OperationStatus::ERROR_OUTPUT_NOT_EMPTY:
                json += "\"ERROR_OUTPUT_NOT_EMPTY\"";
                break;
            case storage::Storage::OperationStatus::ERROR_BIN_NOT_FOUND:
                json += "\"ERROR_BIN_NOT_FOUND\"";
                break;
            case storage::Storage::OperationStatus::ERROR_FULL:
                json += "\"ERROR_FULL\"";
                break;
            case storage::Storage::OperationStatus::ERROR_CAM:
                json += "\"ERROR_CAM\"";
                break;
            case storage::Storage::OperationStatus::ERROR_MIXED_ITEM:
                json += "\"ERROR_MIXED_ITEM\"";
            default:
                json += "\"UNKNOWN\"";
                break;
        }
        json += ",";
        json += "\"item_name\":\"";
        json += Item::getName(last_object_sawn);
        json += "\"";
        json += " }";
        return json;
        
    }
    bool process_image()
    {
        if(getState() == State::IDLE)
        {

            this->rfid = rfid;
            next_state = State::PROCESS_IMAGE;
            xEventGroupSetBits(evg, 0x01);
            return true;
        
        }
        return false;
    }
    bool retrieve_item(storage::rfid_t& rfid)
    {
        if(getState() == State::IDLE)
        {
            this->rfid = rfid;
            next_state = State::RETRIEVE_ITEM;
            xEventGroupSetBits(evg, 0x01);
            return true;
        }
        return false;
    }
    bool store_item(bool change_id,uint8_t item_id)
    {
        if(getState() == State::IDLE)
        {
            this->change_id = change_id;
            this->item_id = item_id;
            next_state = State::STORE_ITEM;
            xEventGroupSetBits(evg, 0x01);
            return true;
        }
        return false;
    }
    bool auto_store()
    {
        if(getState() == State::IDLE)
        {
            next_state = State::AUTO_STORE;
            xEventGroupSetBits(evg, 0x01);
            return true;
        }
        return false;
    }
    bool read_bucket(uint8_t bucket_id)
    {
        if(getState() == State::IDLE)
        {
            this->bucket_id = bucket_id;
            next_state = State::READ_BUCKET;
            xEventGroupSetBits(evg, 0x01);
            return true;
        }
        return false;
    }
    bool remap()
    {
        if(getState() == State::IDLE)
        {
            next_state = State::REMAP;
            xEventGroupSetBits(evg, 0x01);
            return true;
        }
        return false;
    }
    bool reorganize()
    {
        if(getState() == State::IDLE)
        {
            next_state = State::REORGANIZE;
            xEventGroupSetBits(evg, 0x01);
            return true;
        }
        return false;
    }
};