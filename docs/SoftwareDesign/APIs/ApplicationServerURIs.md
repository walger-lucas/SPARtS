URIs Application:
- Uri to request the application to process the image given.

POST /process/image

body is the image.

returns:
{
    "type":"NAME_OF_OBJECT", <-- THE ITEM THAT APPEARS THE MOST
    "amount":QUANTITY,  <-- THE TOTAL QUANTITY OF ALL THE ITEMS
    "mixed": IF_THERE_ARE_MIXED_ITEMS (TRUE/FALSE)
}


- Application webhook to receive events from embedded software. Application must treat this carefully as every event goes through this, so it must be able to map the correct event to the current function that needs this event.
If one hook is lost, the uri "/operation" of the embedded software webserver gets the same thing.

POST /operation/hook

body:
{
    "state": CURRENT STATE OF EMBEDDED SERVER
    "status": OK AND ERROR CODES
}

returns:
200 OK