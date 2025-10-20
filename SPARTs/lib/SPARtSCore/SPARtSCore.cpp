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
        {
            controls::rfid_t rfid;
            if(!storage.readBucket(&storage.interface_bucket)->isEmpty())
            {
                last_storage_status = storage::Storage::OperationStatus::ERROR_OUTPUT_NOT_EMPTY;
                current_state = State::IDLE;
                if(storage.needsReorganizing())
                    last_storage_status = storage::Storage::OK_NEEDS_REORGANIZING;
                else
                    last_storage_status = storage::Storage::OK;
                return;
            }

            res = cam::CamCommunicationMaster::process_image(pdMS_TO_TICKS(15000));

            if(!res.ok())
            {
                current_state = State::INITIALIZING_CAM;
                last_storage_status = storage::Storage::OperationStatus::ERROR_CAM;
                return;
            }
            if(res.item_code == 0)
            {
                if(conveyor.getPos()==controls::ConveyorControl::MAX_BIN-1)
                {
                    current_state = State::IDLE;
                }
                conveyor.next();
                last_storage_status = storage::Storage::OperationStatus::OK;
                current_state = State::AUTO_STORE;
                return;
            }

            if(res.mixed)
            {
                last_storage_status = storage::Storage::OperationStatus::ERROR_MIXED_ITEM;
                current_state = State::IDLE;
                return;
            }
            fits = false;
            while(!fits)
            {
                controls::rfid_t rfid;
                //TODO FIND BIN of type x
                bool found_bin;
                if(!found_bin)
                {
                    //TRY findind bin again (type + quantity now)
                    if(!found_bin)
                    {
                        current_state = State::IDLE;
                        last_storage_status = storage::Storage::ERROR_FULL;
                    }
                }
                opstat = storage.retrieve(rfid);
                using OperationStatus = storage::Storage::OperationStatus;
                if(opstat != OperationStatus::OK && opstat != OperationStatus::OK_NEEDS_REORGANIZING)
                {
                        current_state = State::IDLE;
                        last_storage_status = storage::Storage::ERROR_FULL; 
                        return;
                }
                //AWAIT
                //SEE IF FITS
                if(!fits)
                {
                    opstat = storage.store();
                    if(opstat != OperationStatus::OK && opstat != OperationStatus::OK_NEEDS_REORGANIZING)
                    {
                        current_state = State::IDLE;
                        last_storage_status = storage::Storage::ERROR_FULL; 
                        return;
                    }
                }

                
                //TODO REST OF THIS
            }
            conveyor.next();
            using OperationStatus = storage::Storage::OperationStatus;
            opstat = storage.store();
            if(opstat != OperationStatus::OK && opstat != OperationStatus::OK_NEEDS_REORGANIZING)
            {
                current_state = State::IDLE;
                last_storage_status = storage::Storage::ERROR_FULL; 
                return;
            }
            if(conveyor.getPos()==controls::ConveyorControl::MAX_BIN-1)
                current_state = State::IDLE;
            if(storage.needsReorganizing())
                last_storage_status = storage::Storage::OK_NEEDS_REORGANIZING;
            else
                last_storage_status = storage::Storage::OK;
            break;
            
                
    default:
        current_state = State::IDLE;
        break;
    }}
}