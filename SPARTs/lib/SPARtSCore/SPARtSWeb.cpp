
#include <SPARtSCore.h>

#include <ArduinoJson.h>

static void handleBins(AsyncWebServerRequest *req, SPARtSCore* core)
{
    String json = core->get_json_data();
    req->send(200, "application/json", json);
}

static void handleStatus(AsyncWebServerRequest *req, SPARtSCore* core)
{
    String json = core->get_json_state();
    req->send(200, "application/json", json);
}

static void handleCaptureImage(AsyncWebServerRequest *req, SPARtSCore* core)
{
    if(core->process_image())
    {
        req->send(200);
    } else
    {
        req->send(503);
    }
}

static void handleMap(AsyncWebServerRequest *req, SPARtSCore* core)
{
    if(core->remap())
    {
        req->send(200);
    } else
    {
        req->send(503);
    }
}

static void handleReorganize(AsyncWebServerRequest *req, SPARtSCore* core)
{
    if(core->reorganize())
    {
        req->send(200);
    } else
    {
        req->send(503);
    }
}

static void handleAutoStore(AsyncWebServerRequest *req, SPARtSCore* core)
{
    if(core->auto_store())
    {
        req->send(200);
    } else
    {
        req->send(503);
    }
}

static void handleStore(AsyncWebServerRequest *req, uint8_t *data, size_t len, size_t index, size_t total,SPARtSCore* core)
{
    if(len == 0) {
        if(core->store_item(0, 0)) {
            req->send(200);
        } else {
            req->send(503);
        }
        return;
    }

    JsonDocument doc;
    DeserializationError err = deserializeJson(doc, data, len);
    if(err) {
        // If JSON is invalid, treat it as empty
        if(core->store_item(0, 0)) {
            req->send(200);
        } else {
            req->send(503);
        }
        return;
    }

    // Check if JSON object is empty
    if(doc.is<JsonObject>() && doc.size() == 0) {
        if(core->store_item(0, 0)) {
            req->send(200);
        } else {
            req->send(503);
        }
        return;
    }

    // Process normal JSON with item_name
    String item_name = doc["item_name"] | ""; // default to empty
    if(item_name.length() == 0) {
        // treat missing item_name as empty
        if(core->store_item(0, 0)) {
            req->send(200);
        } else {
            req->send(503);
        }
        return;
    }

    // Store the item
    if(core->store_item(1, Item::getId(item_name))) {
        req->send(200);
    } else {
        req->send(503);
    }
}


static controls::rfid_t hexStringToArray12(const std::string& hexStr) {
    controls::rfid_t result{};
    
    // Ensure we have at least 24 hex characters (12 bytes)
    size_t len = hexStr.size() / 2;
    size_t n = len > 12 ? 12 : len; // clamp to 12 bytes max

    for(size_t i = 0; i < n; ++i) {
        std::string byteStr = hexStr.substr(i * 2, 2);
        result[i] = static_cast<uint8_t>(strtoul(byteStr.c_str(), nullptr, 16));
    }

    return result;
}
static void handleRetrieve(AsyncWebServerRequest *req, uint8_t *data, size_t len, size_t index, size_t total,SPARtSCore* core)
{
    if(len == 0) {
        req->send(400);
        return;
    }

    JsonDocument doc;
    DeserializationError err = deserializeJson(doc, data, len);
    if(err) {
        req->send(400);
        return;
    }

    // Check if JSON object is empty
    if(doc.is<JsonObject>() && doc.size() == 0) {
        req->send(400);
        return;
    }

    // Process normal JSON with item_name
    String rfid = doc["rfid"] | ""; // default to empty
    if(rfid.length() != 24) {
        req->send(400);
        return;
    }
    controls::rfid_t rfid_val = hexStringToArray12(rfid.c_str());


    // Retrieve the item
    if(core->retrieve_item(rfid_val)) {
        req->send(200);
    } else {
        req->send(503);
    }
}

static void handleRead(AsyncWebServerRequest *req, uint8_t *data, size_t len, size_t index, size_t total,SPARtSCore* core)
{
    if(len == 0) {
        req->send(400);
        return;
    }

    JsonDocument doc;
    DeserializationError err = deserializeJson(doc, data, len);
    if(err) {
        req->send(400);
        return;
    }

    // Check if JSON object is empty
    if(doc.is<JsonObject>() && doc.size() == 0) {
        req->send(400);
        return;
    }
    // Process normal JSON with item_name
    
    uint8_t id = doc["id"] | 55; // default to empty
    if(id == 55) {
        req->send(400);
        return;
    }

    // Retrieve the item
    if(core->read_bucket(id)) {
        req->send(200);
    } else {
        req->send(503);
    }
}

static void handleSetup(AsyncWebServerRequest *req, uint8_t *data, size_t len, size_t index, size_t total,SPARtSCore* core)
{
    if(len == 0) {
        req->send(400);
        return;
    }

    JsonDocument doc;
    DeserializationError err = deserializeJson(doc, data, len);
    if(err) {
        req->send(400);
        return;
    }

    // Check if JSON object is empty
    if(doc.is<JsonObject>() && doc.size() == 0) {
        req->send(400);
        return;
    }

    // Process normal JSON with item_name
    String url = doc["image_processing_uri"] | ""; // default to empty
    if(url.length() == 0) {
        req->send(400);
        return;
    }

    // Retrieve the item
    if(core->setup(url)) {
        req->send(200);
    } else {
        req->send(503);
    }
}

static void handleDebugMove(AsyncWebServerRequest *req, uint8_t *data, size_t len, size_t index, size_t total,SPARtSCore* core)
{
    if(len == 0) {
        req->send(400);
        return;
    }

    JsonDocument doc;
    DeserializationError err = deserializeJson(doc, data, len);
    if(err) {
        req->send(400);
        return;
    }

    // Check if JSON object is empty
    if(doc.is<JsonObject>() && doc.size() == 0) {
        req->send(400);
        return;
    }

    // Process normal JSON with item_name
    int x = doc["x"] | 0;
    int y = doc["y"] | 0;

    core->storage.mov_control.xy_table.moveTo({x,y});
    req->send(200);
}







// Wrapper for AsyncWebServer to pass SystemData*
void SPARtSCore::setupWebServer() {
  server.on("/bins", HTTP_GET, [this](AsyncWebServerRequest *req){ handleBins(req, this); });
  server.on("/remap", HTTP_POST, [this](AsyncWebServerRequest *req){ handleMap(req, this); });
  server.on("/reorganize", HTTP_POST, [this](AsyncWebServerRequest *req){ handleReorganize(req, this); });
  server.on("/status", HTTP_GET, [this](AsyncWebServerRequest *req){ handleStatus(req, this); });
  server.on("/capture_image", HTTP_POST, [this](AsyncWebServerRequest *req){ handleCaptureImage(req, this); });
  server.on("/auto_store", HTTP_POST, [this](AsyncWebServerRequest *req){ handleAutoStore(req, this); });


  server.on("/store", HTTP_POST,[](AsyncWebServerRequest *req){ },NULL, [this](AsyncWebServerRequest *req, uint8_t *data, size_t len, size_t index, size_t total)
   { handleStore(req,data,len,index,total, this); });

  server.on("/retrieve", HTTP_POST,[](AsyncWebServerRequest *req){ },NULL, [this](AsyncWebServerRequest *req, uint8_t *data, size_t len, size_t index, size_t total)
   {handleRetrieve(req,data,len,index,total, this); });

  server.on("/read", HTTP_POST,[](AsyncWebServerRequest *req){ },NULL, [this](AsyncWebServerRequest *req, uint8_t *data, size_t len, size_t index, size_t total)
   { handleRead(req,data,len,index,total, this); });

  server.on("/setup", HTTP_POST,[](AsyncWebServerRequest *req){ },NULL, [this](AsyncWebServerRequest *req, uint8_t *data, size_t len, size_t index, size_t total)
   { handleSetup(req,data,len,index,total, this); });

  server.on("/debug/move", HTTP_POST,[](AsyncWebServerRequest *req){ },NULL, [this](AsyncWebServerRequest *req, uint8_t *data, size_t len, size_t index, size_t total)
   {printf("[DEBUG] MOVE\n"); handleDebugMove(req,data,len,index,total, this); });
  server.on("/debug/calibrate/xy", HTTP_POST, [this](AsyncWebServerRequest *req)
  {printf("[DEBUG] CALIBRATE XY\n"); storage.mov_control.xy_table.calibrate();req->send(200); });
  server.on("/debug/calibrate/conveyor", HTTP_POST, [this](AsyncWebServerRequest *req)
  {printf("[DEBUG] CALIBRATE CONVEYOR\n"); conveyor.start();req->send(200); });
  server.on("/debug/calibrate/platform", HTTP_POST, [this](AsyncWebServerRequest *req)
  {printf("[DEBUG] CALIBRATE PLATFORM\n"); storage.mov_control.platform.calibrate();req->send(200); });
  server.on("/debug/platform/extend", HTTP_POST, [this](AsyncWebServerRequest *req)
  {printf("[DEBUG] EXTEND\n"); storage.mov_control.platform.move(controls::PlatformControl::Direction::EXTEND,controls::Speed::FAST);req->send(200); });
  server.on("/debug/platform/retract", HTTP_POST, [this](AsyncWebServerRequest *req)
  {printf("[DEBUG] RETRACT\n"); storage.mov_control.platform.move(controls::PlatformControl::Direction::RETRACT,controls::Speed::FAST);req->send(200); });
    server.on("/debug/conveyor/next", HTTP_POST, [this](AsyncWebServerRequest *req)
  {printf("[DEBUG] CONVEYOR NEXT\n"); conveyor.next();req->send(200); });

  server.begin();
  Serial.println("Web server started...\n");
}