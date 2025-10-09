#include "load_cell.h"
#include <HX711_ADC.h>

/*=========================================
            CALIBRATION FACTOR
===========================================
    1. Leave this value as 1.0 initially.
    2. Upload the code and open the Serial Monitor.
    3. Place a known weight (e.g., a 350g can) on the scale.
    4. Note the value shown on the monitor (e.g., -123456.0).
    5. Calculate the new factor: new_factor = (measured_value / known_weight) -> (-123456 / 350) = -352.7
    6. Update the constant below with your new factor (negative if necessary).
=========================================*/
const float CALIBRATION_FACTOR = 1671.625; // <-- CHANGE THIS VALUE

//Class constructor.
LoadCell::LoadCell(const byte pinDout, const byte pinSck)
    : pinDout(pinDout), pinSck(pinSck) {
    
    this->hx711 = new HX711_ADC(this->pinDout, this->pinSck);//Creates the library object instance.
}

//Initialization method.
void LoadCell::setup() {
    Serial.println("Initializing HX711...");
    this->hx711->begin();
    this->hx711->start(2000, true); //Starts and performs tare at startup.

    if (this->hx711->getTareTimeoutFlag()) {
        Serial.println("Tare timeout, check wiring.");
    } else {
        Serial.println("Tare complete.");
    }
    this->hx711->setCalFactor(CALIBRATION_FACTOR);
    Serial.println("HX711 ready.");
}

//Method for zeroing the scale.
void LoadCell::tare() {
    this->hx711->tare();
    Serial.println("Tare command sent.");
}

//Method for obtaining weight.
float LoadCell::getWeight() {
    this->hx711->update();
    return this->hx711->getData();//Returns the data already converted by the calibration factor.
}