#include "Injector.h"

Syringe::Syringe()
{
    index = 0;
    //name = "1ml braun     ";
    cal_factor = 1.0;
    volume = 1.0;
    empty = 14.5;
    full = 71.8;
    recalc();
}

void Syringe::load()
{
    int addr = get_addr();

}

void Syringe::save()
{
    int addr = get_addr();
}

void Syringe::recalc()
{
    mm_per_ml = (full-empty)/(volume*cal_factor);
    ml_per_mm = 1. / mm_per_ml;
}

Injector::Injector(LinearStage* linearstage, Adafruit_GFX* display)
{
    this->linearstage = linearstage;
    this->display = display;

    state = STATE_EMPTY;
    planned_amount = 0;
    planned_time = 0;
    start_amount = 0;
    start_time = micros();
    mode = MODE_ACTIVITY;
}

//float Injector::activity_a()
//{
//    float n_hl = EventTimer::Now()
//    return pow(activity_a0,)
//}

void Injector::update_display()
{
    display->setTextSize(1);
    display->setTextColor(1);

    float pos = linearstage->get_position_mm();
    float vol = (pos - syringe.empty) * syringe.ml_per_mm;

    draw_syringe(vol);

    display->setCursor(0,32);
    print_volume(vol);

    if(mode == MODE_ACTIVITY)
    {

    }
}

void Injector::print_volume(float volume)
{
    display->print(F("V: "));
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
    display->print(F(" / "));
    if(volume<10)display->print('0'); // pad
    display->print(syringe.volume, 2);
    display->print(F(" ml"));
}

//void Injector::print_activity(float volume)
//{
//
//}

void Injector::draw_syringe(float volume)
{
    float vol = constrain(volume,0.,1.);

    const int16_t center_y = 8;
    const int16_t pointy_w = 12;
    const int16_t pointy_h = 5;
    const int16_t cyl_x = pointy_w - 1;
    const int16_t cyl_w = 64;
    const int16_t cyl_h = 15;
    const int16_t liquid_x = cyl_x + 3;
    const int16_t liquid_h = cyl_h-6;
    const int16_t pusher_w = 60;
    const int16_t pusher_h = 5;
    const int16_t pusher_h2 = cyl_h - 4;
    int16_t liquid_w = int16_t((vol/syringe.volume) * float(cyl_w-4));
    int16_t pusher_x = liquid_x + liquid_w + 2;

    display->drawRect(0, center_y-pointy_h/2, pointy_w, pointy_h, 1); // pointy end
    display->drawRect(cyl_x, center_y-cyl_h/2, cyl_w, cyl_h, 1); // cylinder
    display->drawFastVLine(cyl_x, center_y-pointy_h/2+1, pointy_h-2, 0); // remove front line to create open end
    display->drawFastVLine(cyl_x+cyl_w-1, center_y-cyl_h/2+1, cyl_h-2, 0); // remove back line to create open cylinder
    display->fillRect(liquid_x, center_y-liquid_h/2, liquid_w, liquid_h, 1); // liquid
    display->drawRect(pusher_x, center_y-pusher_h2/2, 2, pusher_h2, 1); // pusher plate
    display->drawRect(pusher_x, center_y-pusher_h/2, pusher_w, pusher_h, 1); // pusher rod
    display->drawRect(pusher_x+pusher_w-1, center_y-cyl_h/2, 2, cyl_h, 1); // pusher plate back    
}
