URIs Embedded Software:

URIs:

- Gets status as if the webhook had fired.
GET /operation
return
{
    "state": CURRENT STATE OF EMBEDDED SERVER
    "status": OK OR ERROR CODES
}
- GETS all bins that have already been read by the embedded software
GET /bins
return
{
    "bins" = [
        {"rfid":rfid_code,"type":ITEM_TYPE,"amount":quantity,"position":NONE/POSTION_ID},
        {"rfid":rfid_code,"type":ITEM_TYPE,"amount":quantity,"position":NONE/POSTION_ID},
        {"rfid":rfid_code,"type":ITEM_TYPE,"amount":quantity,"position":NONE/POSTION_ID},
        ...
    ]
}

- Receives confirmation of reading of status, via the /operation or the webhook.
POST /operation/ok
body empty
return 200 OK

- Receives information of the current state of the EspCam

GET /image
body empty
return
{
    "type":"NAME_OF_OBJECT", <-- THE ITEM THAT APPEARS THE MOST
    "amount":QUANTITY,  <-- THE TOTAL QUANTITY OF ALL THE ITEMS
    "mixed": IF_THERE_ARE_MIXED_ITEMS (TRUE/FALSE)
}

- Configure the embedded software
POST /configure
{
        "image_processing_uri" : "x.x.x.x:xxxx/image",
        "state_web_hook" : "x.x.x.x:xxxx/hook"
}
return 200 OK
## for all the uris below, their return does not indicate if the task has been done successfully, only that the task has started or not. They can only start when the embedded software is on the AWAIT state, else, they will not work. IF AN ERROR OCCURS, THE TASK WILL BE ABORTED, TO CORRECT THE ERROR THE APPLICATION MUST CALL OTHER OPERATIONS DEPENDING ON THE ERROR.

- Starts routine to read all the bin positions and remap their rfid location

POST /operation/remap
body empty
return 200 OK if in AWAIT STATE else 503 Service Unavailable

- Starts operation to reorganize the bins by their location and use. If there is a bin on the output, it will fail, if it happens, use /operation/store to store the bin.

POST /operation/reorganize
body empty
return 200 OK if in AWAIT STATE else 503 Service Unavailable

- Retrieves the bin with rfid xxxx and stores it on the output area. If output area used, it will fail. Use /operation/store to store the bin on the output area before it.

POST /operation/retrieve?id=xxxx
body empty
return 200 OK if in AWAIT STATE else 503 Service Unavailable

- Stores pieces by vision on the buffer bins. if move is true, then it will move the bin to get the next move, else, it will take the picture of the current bin. Use /image to get if there is an object to choose if it is needed to move to the next bin or not. (may be used in the case that it fails after the photo, but then the error is treated and you may need to use this operation again, but without moving the bin to the next)

POST /operation/store_piece?move=xxxx
body empty
return 200 OK if in AWAIT STATE else 503 Service Unavailable


- Stores the bin on the output area back to an available position.
POST /operation/store
body empty
return 200 OK if in AWAIT STATE else 503 Service Unavailable

- Reads the rfid and updates it on the map of the given pos_id of 
POST /operation/read_pos?pos_id=xxxx
body empty
return 200 OK if in AWAIT STATE else 503 Service Unavailable

