#include "Injector.h"

Syringe::Syringe()
{

}

Syringe::Syringe(uint8_t index)
{

}

Syringe::Syringe(String name, float ml_per_mm, float empty, float full, float max_speed)
{

}

void Syringe::save(int8_t index)
{
    eeprom.write()
}


void Syringe::load(int8_t index)
{

}
Injector::Injector(LinearStage* linearstage, Adafruit_GFX* display)
{
    this->linearstage = linearstage;
    this->display = display;
    syringe = Syringe("test")
    planned_amount = 0;
    injected_amount = 0;
    planned_time = 0;
}