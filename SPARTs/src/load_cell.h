#ifndef LOADCELL_HPP
#define LOADCELL_HPP

/*=========================================
            HX711/ESP Conection
===========================================
    VCC → 3.3V
    GND → GND
    DT  → GPIO 25
    SCK → GPIO 26
=========================================*/

#include <Arduino.h>

class HX711_ADC;

class LoadCell {
public:
    LoadCell(const byte pinDout, const byte pinSck);

    void setup();

    void tare();//Tare (reset) the scale.

    float getWeight();//Returns the current weight in grams (or the calibrated unit).

private:
    HX711_ADC* hx711; //Pointer to the library object.
    const byte pinDout;
    const byte pinSck;
};

#endif //LOADCELL_HPP