#include "load_cell.h"
#include <HX711_ADC.h>

// --- FATOR DE CALIBRAÇÃO ---
// Este valor é CRUCIAL. Você precisará encontrar o seu.
// 1. Deixe este valor como 1.0 inicialmente.
// 2. Faça o upload do código e abra o Monitor Serial.
// 3. Coloque um objeto de peso conhecido (ex: uma lata de 350g) na balança.
// 4. Anote o valor que aparece no monitor (ex: -123456.0).
// 5. Calcule o novo fator: new_factor = (valor_lido / peso_conhecido) -> (-123456 / 350) = -352.7
// 6. Atualize a constante abaixo com o seu novo fator (negativo se necessário).
const float CALIBRATION_FACTOR = -350.0; // <-- MUDE ESTE VALOR

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