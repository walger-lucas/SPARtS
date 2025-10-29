#include "SPARtSCore.h"
#include <WiFi.h>

void WiFiEvent(WiFiEvent_t event) {
  switch (event) {
    case SYSTEM_EVENT_AP_STACONNECTED: {
      wifi_sta_list_t stationList;
      tcpip_adapter_get_sta_list(&stationList, nullptr);

      Serial.println("New station connected!");
      Serial.printf("Connected Stations: %d\n", stationList.num);
      for (int i = 0; i < stationList.num; i++) {
        wifi_sta_info_t station = stationList.sta[i];
        Serial.printf("MAC: %02X:%02X:%02X:%02X:%02X:%02X\n",
                      station.mac[0], station.mac[1], station.mac[2],
                      station.mac[3], station.mac[4], station.mac[5]);
      }
      break;
    }

    case SYSTEM_EVENT_AP_STADISCONNECTED: {
      Serial.println("A station disconnected!");
      break;
    }

    default:
      break;
  }
}
void SPARtSCore::run()
{
    cam::CamResult res;
    cam::CamStatus stat;
    storage::Storage::OperationStatus opstat;
    bool fits = false;
    switch (current_state)
    {
    case State::INITIALIZING:
        printf("Initializing SPARtSCore...\n");
        evg = xEventGroupCreate();
        cam::CamCommunicationMaster::setup_comm();
        conveyor.start();
        storage.init();

        WiFi.softAP(ssid.c_str(),password.c_str());
        WiFi.onEvent(WiFiEvent);
        setupWebServer();
        setState(State::AWAITING_SETUP);
        
        break;
    case State::AWAITING_SETUP:
        printf("Awaiting setup...\n");

        next_state = State::INITIALIZING;
        while(next_state != State::INITIALIZING_CAM)
        {
            xEventGroupClearBits(evg,0xff);
            xEventGroupWaitBits(evg,0x01,true,true,portMAX_DELAY);
        }
        setState(State::INITIALIZING_CAM);
        break;
    case State::INITIALIZING_CAM:
        printf("Initializing CAM...\n");
        stat = cam::CamCommunicationMaster::send_config(ssid.c_str(),password.c_str(),uri.c_str(),pdMS_TO_TICKS(15000));
        if(stat == cam::CamStatus::PROCESS_OK)
        {
            printf("CAM initialized successfully.\n");
            setState(State::IDLE);
            next_state = State::IDLE;
        } else {
            printf("CAM initialization failed.\n");

        }
        break;

    case State::IDLE:
        printf("Awaiting Command..\n");
        while(next_state == State::IDLE)
        {
            xEventGroupClearBits(evg,0xff);
            xEventGroupWaitBits(evg,0x01,true,true,portMAX_DELAY);
        }

        setState(next_state);
        next_state = State::IDLE;
        break;

    case State::RETRIEVE_ITEM:
        printf("Retrieving item...\n");
        last_storage_status = storage.retrieve(rfid);
        setState(State::IDLE);
        break;
    case State::STORE_ITEM:
        printf("Storing item...\n");
        last_storage_status = storage.store(change_id,item_id);
        setState(State::IDLE);
        break;
    case State::READ_BUCKET:
        printf("Read Bucket...\n");
        storage.readBucketFromId(bucket_id);
        if(storage.needsReorganizing())
            last_storage_status = storage::Storage::OK_NEEDS_REORGANIZING;
        else
            last_storage_status = storage::Storage::OK;
        setState(State::IDLE);
        break;
    case State::REMAP:
        printf("Remapping...\n");
        last_storage_status = storage.map();
        setState(State::IDLE);
        break;
    case State::REORGANIZE:
        printf("Reorganizing...\n");
        last_storage_status = storage.reorganize();
        setState(State::IDLE);
        break;

    case State::AUTO_STORE:
        printf("Auto storing item...\n");
        auto_store_state();
        break;
            
    case State::PROCESS_IMAGE:
            res = cam::CamCommunicationMaster::process_image(pdMS_TO_TICKS(20000));
            last_storage_status = storage::Storage::OperationStatus::OK;
            if(!res.ok())
            {
                setState(State::INITIALIZING_CAM);
                last_storage_status = storage::Storage::OperationStatus::ERROR_CAM;
                return;
            }
            last_object_sawn = res.item_code;
            setState(State::IDLE);
            break;
            
    default:
        current_state = State::IDLE;
        break;
    }
}


storage::Storage::OperationStatus SPARtSCore::auto_store_state()
{

    auto res = cam::CamCommunicationMaster::process_image(pdMS_TO_TICKS(15000));

    if(!res.ok())
    {
        current_state = State::INITIALIZING_CAM;
        last_storage_status = storage::Storage::OperationStatus::ERROR_CAM;
        return storage::Storage::OperationStatus::ERROR_CAM;
    }
    if(res.item_code == 0)
    {
        if(conveyor.getPos()==controls::ConveyorControl::MAX_BIN-1)
        {
            current_state = State::IDLE;
        } else 
        {
            current_state = State::AUTO_STORE;
        }
        conveyor.next();
        if(storage.needsReorganizing())
        {
            last_storage_status = storage::Storage::OK_NEEDS_REORGANIZING;
            return last_storage_status;
        }
        else
        {
            last_storage_status = storage::Storage::OK;
            return last_storage_status;
        }
    }

    if(res.mixed)
    {
        last_storage_status = storage::Storage::OperationStatus::ERROR_MIXED_ITEM;
        current_state = State::IDLE;
        return storage::Storage::OperationStatus::ERROR_MIXED_ITEM;
    }
    bool fits = false;
    while(!fits)
    {
        storage::Bucket* found_bin = storage.FindFittingBucket(res.item_quantity,res.item_code);
        if(!found_bin)
        {
            storage.map();
            found_bin = storage.FindFittingBucket(res.item_quantity,res.item_code);
            if(!found_bin)
            {
                current_state = State::IDLE;
                last_storage_status = storage::Storage::ERROR_FULL;
                return storage::Storage::ERROR_FULL;
            }
        }
        controls::rfid_t rfid = found_bin->getBin()->getRFID();
        auto opstat = storage.retrieve(rfid);
        using OperationStatus = storage::Storage::OperationStatus;
        if(opstat != OperationStatus::OK && opstat != OperationStatus::OK_NEEDS_REORGANIZING)
        {
                current_state = State::IDLE;
                last_storage_status = opstat; 
                return opstat;
        }
        storage.interface_bucket.updateWeight(res.item_code);
        fits = storage.interface_bucket.getBin()->getAmount()+res.item_quantity <= Item::getMaxAmount(res.item_code);
        if(!fits)
        {
            
            opstat = storage.store();
            if(opstat != OperationStatus::OK && opstat != OperationStatus::OK_NEEDS_REORGANIZING)
            {
                current_state = State::IDLE;
                last_storage_status = opstat; 
                return opstat;
            }
        }
        storage.interface_bucket.getBin()->setItemId(res.item_code);
    }
    
    conveyor.next();
    using OperationStatus = storage::Storage::OperationStatus;
    delay(1000);
    auto opstat = storage.store();
    if(opstat != OperationStatus::OK && opstat != OperationStatus::OK_NEEDS_REORGANIZING)
    {
        current_state = State::IDLE;
        last_storage_status = opstat; 
        return opstat;
    }
    if(conveyor.getPos()==controls::ConveyorControl::MAX_BIN-1)
    {
        current_state = State::IDLE;
    } else 
    {
        current_state = State::AUTO_STORE;
    }
    if(storage.needsReorganizing())
    {
        last_storage_status = storage::Storage::OK_NEEDS_REORGANIZING;
        return OperationStatus::OK_NEEDS_REORGANIZING;
    }
    else
    {
        last_storage_status = storage::Storage::OK;
        return OperationStatus::OK;
    }

}