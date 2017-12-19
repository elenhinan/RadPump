#include "Injector.h"

Injector::Injector(LinearStage* linearstage, Adafruit_GFX* display) :
    state(STATE_INSERT),
    planned_amount(0),
    planned_rate(0.1),
    //planned_start(0xffffffff),
    activity_t0(0),
    activity_c0(0),
    mode(MODE_DISTANCE)    
{
    this->linearstage = linearstage;
    this->display = display;
    set_isotope(0);
    set_syringe(0);

    state = STATE_EMPTY;
    mode = MODE_ACTIVITY;
}

void Injector::update_display()
{
    display->setTextSize(1);
    display->setTextColor(1);

    float vol = get_volume();

    draw_syringe(vol);
    display->setCursor(0,24);
    print_name();
    print_volume(vol);
    print_activity(vol);
    print_plan();

}

void Injector::print_name()
{
    display->print(F("S: \""));
    display->print(syringe.name);
    display->println('"');

}

void Injector::print_volume(float volume)
{
    display->print(F("V:"));
    if(volume<-10)
    {
        display->print(volume, 2);    
    }
    else if (volume<0)
    {
        display->print(F("-0")); // pad
        display->print(-volume, 2);    
    }
    else if (volume<10)
    {
        display->print(F(" 0")); // pad
        display->print(volume, 2);    
    }
    display->println(F(" ml"));
}

void Injector::print_activity(float volume)
{
    display->print(F("A: "));
    display->print(get_activity()*volume, 2);
    display->print(F(" MBq ("));
    display->print(isotope.name);
    display->println(F(")"));
}

void Injector::print_plan()
{
    display->print(F("I: "));
    display->print(planned_amount, 3);
    switch(mode)
    {
        case MODE_ACTIVITY:display->println(F(" MBq")); break;
        case MODE_VOLUME: display->println(F(" ml")); break;
        case MODE_DISTANCE: display->println(F(" mm")); break;
    }
    display->print(F("R: "));
    display->print(planned_rate, 3);
    switch(mode)
    {
        case MODE_ACTIVITY:display->println(F(" ml/s")); break;
        case MODE_VOLUME: display->println(F(" ml/s")); break;
        case MODE_DISTANCE: display->println(F(" mm/s")); break;
    }
}

void Injector::draw_syringe(float volume)
{
    float vol = constrain(volume,0.,1.);

    const int16_t center_y = 8;
    const int16_t pointy_w = 12;
    const int16_t pointy_h = 5;
    const int16_t cyl_x = pointy_w - 1;
    const int16_t cyl_w = 72;
    const int16_t cyl_h = 15;
    const int16_t liquid_x = cyl_x + 3;
    const int16_t liquid_h = cyl_h-6;
    //const int16_t pusher_w = 60;
    const int16_t pusher_h = 5;
    const int16_t pusher_h2 = cyl_h - 4;
    int16_t liquid_w = int16_t((vol/syringe.volume) * float(cyl_w-8));
    int16_t pusher_x = liquid_x + liquid_w + 2;
    int16_t pusher_x2 = (cyl_x + cyl_w + 2) + vol*(127-2-(cyl_x + cyl_w + 2));
    int16_t pusher_w = pusher_x2-pusher_x;

    display->drawRect(0, center_y-pointy_h/2, pointy_w, pointy_h, 1); // pointy end
    display->drawRect(cyl_x, center_y-cyl_h/2, cyl_w, cyl_h, 1); // cylinder
    display->drawFastVLine(cyl_x, center_y-pointy_h/2+1, pointy_h-2, 0); // remove front line to create open end
    display->drawFastVLine(cyl_x+cyl_w-1, center_y-cyl_h/2+1, cyl_h-2, 0); // remove back line to create open cylinder
    display->drawFastHLine(cyl_x+cyl_w-2,center_y+cyl_h/2+1, 2, 1);
    display->drawFastHLine(cyl_x+cyl_w-2,center_y-cyl_h/2-1, 2, 1);
    display->fillRect(liquid_x, center_y-liquid_h/2, liquid_w, liquid_h, 1); // liquid
    display->drawRect(pusher_x, center_y-pusher_h2/2, 2, pusher_h2, 1); // pusher plate
    display->drawRect(pusher_x, center_y-pusher_h/2, pusher_w, pusher_h, 1); // pusher rod
    display->drawRect(pusher_x+pusher_w-1, center_y-cyl_h/2, 2, cyl_h, 1); // pusher plate back    
}
void Injector::set_isotope(uint8_t index)
{
    eeprom_read_block(&isotope, &IsotopesEeprom[index], sizeof(Isotope)); // read from eeprom
    #ifdef DEBUG
    Serial.print(F("Isotope #"));
    Serial.println(index);
    Serial.print(F("  "));
    Serial.println(isotope.name);
    Serial.print(F("  "));
    Serial.println(isotope.halflife);
    #endif
}

// void Injector::set_isotope(char name[8], float halflife)
// {
//     uint8_t i=0;
//     while(name[i] != 0x00)
//         isotope.name[i] = name[i++]; // copy contents of name
//     while(i<sizeof(isotope.name))
//         isotope.name[i++] = 0x00; // zero pad rest
//     isotope.halflife = halflife;
// }

void Injector::set_syringe(uint8_t index)
{
    eeprom_read_block(&syringe, &SyringesEeprom[index], sizeof(Syringe)); // read from eeprom
    #ifdef DEBUG
    Serial.print(F("Syringe #"));
    Serial.println(index);
    Serial.print(F("  "));
    Serial.println(syringe.name);
    Serial.print(F("  empty: "));
    Serial.println(syringe.empty);
    Serial.print(F("  full: "));
    Serial.println(syringe.full);
    Serial.print(F("  vol: "));
    Serial.println(syringe.volume);
    #endif
}

float Injector::get_volume()
{
    float pos = linearstage->get_position_mm();
    float volume = (pos - syringe.empty) / (syringe.full - syringe.empty) * syringe.volume;
    return volume;
}

float Injector::get_activity()
{
    float t = (now() - activity_t0) * 1.0f/60.0f;
    float hl = isotope.halflife;
    float At = activity_c0 * pow(0.5f, t/hl);
    return At;
}

void Injector::set_activity(float a0, time_t t0, float volume)
{
    activity_c0 = a0 / volume;
    activity_t0 = t0;
}

void Injector::inject_volume(float vol, float rate)
{
    planned_amount = vol;
    planned_rate = rate;
    mode = MODE_VOLUME;
}

void Injector::inject_activity(float activity, float rate)
{
    planned_amount = activity;
    planned_rate = rate;
    mode = MODE_ACTIVITY;
}

void Injector::start()
{
    state = STATE_RUNNING;
}

void Injector::delay_start(uint32_t time)
{
    state = STATE_TIMER;
}
