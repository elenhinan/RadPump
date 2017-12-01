#include "Linear.h"

LinearStage *int0_cb_instance;
LinearStage *int1_cb_instance;

void int0_callback() { int0_cb_instance->stall_event(); }
void int1_callback() { int1_cb_instance->stall_event(); }

LinearStage::LinearStage(uint8_t pinEN, uint8_t pinDIR, uint8_t pinSTEP, uint8_t pinCS, uint8_t pinDIAG1)
{
    this->number = LinearStageNumber++; // keep track of number of initalized objects
    this->pinEN = pinEN;
    this->pinDIR = pinDIR;
    this->pinSTEP = pinSTEP;
    this->pinCS = pinCS;
    this->pinSTALL = pinDIAG1;
    this->stepPort = portOutputRegister(digitalPinToPort(pinSTEP));
    this->stepMask = digitalPinToBitMask(pinSTEP);
    this->dirPort = portOutputRegister(digitalPinToPort(pinDIR));
    this->dirMask = digitalPinToBitMask(pinDIR);
}

void LinearStage::setup_timer()
{
    // Set stepper interrupt
    cli();//stop interrupts
    TCCR1A = 0;// set entire TCCR1A register to 0
    TCCR1B = 0;// same for TCCR1B
    TCNT1  = 0;//initialize counter value to 0
    OCR1A = 256;// = (16*10^6) / (1*1024) - 1 (must be <65536) // todo set to board freq
    // turn on CTC mode
    TCCR1B |= (1 << WGM12);
    // Set CS11 bits for 8 prescaler
    TCCR1B |= (1 << CS11);// | (1 << CS10);  
    // enable timer compare interrupt
    TIMSK1 |= (1 << OCIE1A);
    sei();//allow interrupts
}

ISR(TIMER1_COMPA_vect){
  //do stuff
}

void LinearStage::init()
{
#ifdef DEBUG
    Serial.print(F("Stage #"));
    Serial.print(number,DEC);
    Serial.println(F(" initializing.."));
    Serial.print(F("  en: "));
    Serial.println(pinEN,DEC);
    Serial.print(F("  dir: "));
    Serial.println(pinDIR,DEC);
    Serial.print(F("  step: "));
    Serial.println(pinSTEP,DEC);
    Serial.print(F("  cs: "));
    Serial.println(pinCS,DEC);
    Serial.print(F("  stall: "));
    Serial.println(pinSTALL,DEC);
#endif
    pinMode(this->pinSTALL, INPUT);

    stepper_sgt = STALL_VALUE;

    this->stepper = new TMC2130Stepper(pinEN, pinDIR, pinSTEP, pinCS);
    this->setup_driver();

    stalled = false;
    stallguard_enabled = false;
    
    switch(digitalPinToInterrupt(pinSTALL))
    {
        case 0: int0_cb_instance = this; attachInterrupt(0, int0_callback, RISING); break;
        case 1: int1_cb_instance = this; attachInterrupt(1, int1_callback, RISING); break;
        default: Serial.println(F("  No Interrupt")); break;
    }
    Serial.print(F("  steps/mm: "));
    Serial.println(STEPMM,DEC);
    Serial.println(F("  done!"));
}

void LinearStage::stall_event()
{
    stalled = stallguard_enabled;
    // if(stallguard_enabled)
    // {
    //     state = MOVE_STALLED;
    //     #ifdef DEBUG
    //     Serial.print("Stage #");
    //     Serial.print(number);
    //     Serial.println(" Stall!");
    //     #endif
    // }
}

void LinearStage::setup_driver()
{
    //stepper->begin();
    // configure stepper driver
	stepper->rms_current(200); // mA
    stepper->microsteps(MICROSTEPS);
    stepper->interpolate(1);
    stepper->external_ref(0);
    stepper->internal_sense_R(0);
    //stepper->ihold(15);
    //stepper->irun(31);
    stepper->hold_delay(5);
    stepper->power_down_delay(255);
    
    stepper->random_off_time(0);
    stepper->disable_I_comparator(0);
    stepper->hysterisis_start(3);
    stepper->hysterisis_low(3);
    stepper->blank_time(36);
    stepper->off_time(5);
    //stepper->chopper_mode(0);
    stepper->diag1_stall(1);
    stepper->diag1_active_high(1);
    stepper->sg_stall_value(stepper_sgt);
    stepper->sg_filter(0);

    // stealthchop settings
    //stepperA.stealth_freq(1);
    //stepperA.stealth_autoscale(1);
    //stepperA.stealth_gradient(5);
    //stepperA.stealth_amplitude(255);
    //stepperA.sg_stall_value(0);

    // stepper->stealthChop(0);
    // stepper->stealth_max_speed(0xFFFFF);
    // stepper->coolstep_min_speed(0xFFFFF);
    // stepper->fullstep_threshold(0);
    // stepper->high_speed_mode(0);
    // stepper->mode_sw_speed(0);

    // set auto switch from stealthstep
    //stepper->stealth_max_speed(MMS2TSTEP(20)); // disable stealthstep above this velocity
    //stepper->coolstep_min_speed(MMS2TSTEP(0.5)); // disable coolstep and stallguard below this velocity
    //stepper->mode_sw_speed(MMS2TSTEP(40));    // fullstep above this speed
    
    // homing
    //stepperA.coolstep_min_speed(0xfffff);
    //stepperA.stealthChop(0);
    //stepperA.diag1_stall(1);
    
    //stepperA.vhighfs(0);
    //stepperA.vhighchm(0);
    //stepperA.en_pwm_mode(0);

    stealthchop(true);
}

void LinearStage::stallguard(bool enable)
{
    stalled = false;
    stallguard_enabled = enable;
    #ifdef DEBUG
    Serial.print(F("  stallguard:"));
    Serial.println(enable,DEC);
    #endif
}

void LinearStage::stealthchop(bool enable)
{
    stepper->stealthChop(enable);
    stepper->coolstep_min_speed(enable?0:0xfffff);
    #ifdef DEBUG
    Serial.print(F("  stealthchop:"));
    Serial.println(enable,DEC);
    #endif
}

void LinearStage::home(int8_t home_dir)
{
    #ifdef DEBUG
    Serial.print(F("Stage #"));
    Serial.print(number,DEC);
    Serial.println(F(" homing"));
    #endif
    stealthchop(false);
    stallguard(true);
    if(home_dir == DIR_BOTH || home_dir == DIR_POS)
    {
        dir(DIR_POS);
        while(!stalled)
        {
            delayMicroseconds(HOMING_SPEED);
            step();
        }
        #ifdef DEBUG
        Serial.print(F("  endstop found: "));
        Serial.println(position);
        #endif
        stalled = false;
        endstop = position; // find endstop first
        //move(0);
    }
    if(home_dir == DIR_BOTH || home_dir == DIR_NEG)
    {
        dir(DIR_NEG);
        while(!stalled)
        {
            delayMicroseconds(HOMING_SPEED);
            step();
        }
        stalled = false;
        #ifdef DEBUG
        Serial.print(F("  home found: "));
        Serial.println(position);
        #endif
        if(home_dir == DIR_BOTH)
        {
            endstop -= position; // reposition endstop relative to new home
        }
        position = 0;
        //move(endstop);
    }
    #ifdef DEBUG
    Serial.print(F("  range: [0.00 (mm), "));
    Serial.print(float(endstop)/float(STEPMM));
    Serial.println(F(" (mm)]"));
    #endif

    stallguard(false);
    stealthchop(true);
}

void LinearStage::calibrate()
{
    // tune stallguard2
    bool done_tuning = false;
    // setup stepper driver
    //setup_driver();
    stepper->stealthChop(false);
    stepper->sg_filter(1);

    #ifdef DEBUG
    Serial.print(F("Stage #"));
    Serial.print(number,DEC);
    Serial.println(F(" calibrating.."));
    #endif
    int8_t sgt = 0; // start at zero according to datasheet
    stepper->sg_stall_value(sgt);
    uint16_t repetitions = 0;
    uint16_t steps = 0;
    uint16_t sg_value;
    uint16_t sg_value_max = 0;
    bool sgt_increase = true;
    bool sg_zero = true;
    const unsigned long tstep = SG2_TUNE_TSTEP; // us between steps
    unsigned long nexttime = micros();

    while(!done_tuning)
    {
        if(micros() <= nexttime)
            continue;
        nexttime += tstep;
        step();

        if(++steps == MICROSTEPS*5) // sg_filter filters last 4 fullsteps
        {
            steps = 0;
            sg_value = stepper->sg_result();
            sg_value_max = max(sg_value_max, sg_value);
            #ifdef DEBUG
            Serial.print(F("  SGT: "));
            Serial.print(sgt,DEC);
            Serial.print(F(", SG: "));
            Serial.println(sg_value,DEC);
            #endif
            if(sgt_increase)
            {
                if(sgt >= 63 || sg_value >= 1023)
                {
                    sgt_increase = false;
                    repetitions = 0;
                }
                else if(++repetitions == SG2_REP)
                {
                    stepper->sg_stall_value(++sgt);
                    repetitions = 0;
                }
            }
            else
            {
                sg_zero &= (sg_value == 0);
                if(++repetitions == SG2_REP)
                {
                    if(sg_zero)
                    {
                        done_tuning = true;
                    }
                    else
                    {
                        stepper->sg_stall_value(--sgt);
                        if(sgt == -64)
                            done_tuning = true;
                        repetitions = 0;
                        sg_zero = true;
                    }
                }
            }
        }
        
    }
    stepper_sgt = sgt;
    #ifdef DEBUG
    Serial.print(F("  Tuning done, optimal sgt: "));
    Serial.println(stepper_sgt,DEC);
    Serial.print(F("  Max SG value: "));
    Serial.println(sg_value_max,DEC);
    #endif

    stepper->sg_filter(0);
    stepper->stealthChop(true);
}

bool LinearStage::search()
{
    #ifdef DEBUG
    Serial.print(F("Stage #"));
    Serial.print(number,DEC);
    Serial.println(F(" search"));
    #endif

    stealthchop(false);
    stallguard(true);

    dir(DIR_NEG);
    uint32_t sg_val_avg = 0;
    uint32_t sg_val_num = 0;
    for(int i=0;i<2024;i++)
    {
        delayMicroseconds(HOMING_SPEED);
        if(position%MICROSTEPS == 0)
        {
            sg_val_avg += stepper->sg_result();
            sg_val_num++;
        }
        step();
    }
    uint16_t sg_val_thr = sg_val_avg*100/(sg_val_num * 90); // threshold
    #ifdef DEBUG
    Serial.print(F("  threshold: "));
    Serial.println(sg_val_thr,DEC);
    delay(2000);
    #endif
    for(int i=0;i<2024;i++)
    {
        delayMicroseconds(HOMING_SPEED);
        step();
    }

    while(!stalled && position > 0 && position < endstop)
    {
        if(position%MICROSTEPS == 0)
        {
            uint16_t sg = stepper->sg_result();
            Serial.println(sg,DEC);
            if(sg <= sg_val_thr)
            {
                break;
            }
        }
        delayMicroseconds(HOMING_SPEED);
        
        step();
    }
    #ifdef DEBUG
    Serial.print(F("  found: 0 ("));
    Serial.print(position,DEC);
    Serial.println(')');
    #endif
    position = 0;
    stealthchop(true);
    stallguard(false);
    return true;
}

void LinearStage::move_rel(float x, float dx, float ddx)
{
    move(x*STEPMM, dx*STEPMM, ddx*STEPMM, true);
}

void LinearStage::move_abs(float x, float dx, float ddx)
{
    move(x*STEPMM - position, dx*STEPMM, ddx*STEPMM, true);
}


void LinearStage::move(float x, float dx, float ddx, bool limit)
{
    const float ts = 1./1000000;
  
    float speed = dx*ts;
    float accel = 0.5*ddx*ts*ts; // pre-calculate 0.5 in t² * 0.5*accel
    float speed_inv = 1. / speed;
    float accel_inv = 1. / accel;
    target = (int32_t)(x+0.5);
    target>position ? dir(DIR_POS) : dir(DIR_NEG); // set direction of movement

    #ifdef DEBUG
    Serial.print(F("Stage #"));
    Serial.print(number);
    Serial.println(F("  move"));
    Serial.print(F("  pos: "));
    Serial.print(x);
    Serial.print(F(" (steps)  speed: "));
    Serial.print(dx);
    Serial.print(F(" (steps/s)  accel: "));
    Serial.print(ddx);
    Serial.println(F(" (steps/s²)"));
    #endif

    if(limit && (target < 0 || target > endstop)) // check limits
    {
        target = constrain(target, 0, endstop);
        #ifdef DEBUG
        Serial.println(F("  WARNING out of range!"));
        Serial.print(F("  new target: "));
        Serial.print(x);
        Serial.println(F(" (steps)"));
    #endif

    }

    //   Trapezoidal Speed Profile   //
    //            ________           //
    //           /|      |\          //
    //          / |      | \         //
    //       __/  |      |  \__      //
    //         |  |      |  |        //
    //         |tr|--ts--|tr|        //
    //         0  1      2  3        //

    uint32_t d0, d1, d2, d3; // step # for ramp start, slew start, slew end, ramp end
    uint32_t t0, t1, t2, t3; // as above, but time expressed in units of ts

    d0 = 0; // distance to start
    d3 = abs(target);// - position); // distance to end
    d1 = speed * speed / (accel*2); // distance to slew start

    if(2*d1 < d3) // enough space to accel and decel
    {
        d2 = d3 - d1; // distance to slew end
    }
    else if (2*d1 >= d3) // not enough space, skip slewing
    {
        d1 = d3 / 2;
        d2 = d3 - d1;
    }

    // use steps for each block to determine timing for each block
    t0 = 0;
    t1 = (uint32_t)(sqrt(float(d1)/accel))+0.5;
    t2 = (uint32_t)(t1 + float(d2-d1)/speed+0.5);
    t3 = (uint32_t)(t2 + sqrt(float(d3-d2)/accel)+0.5);

    #ifdef DEBUG
    Serial.print(F("  d0: "));
    Serial.print(d0,DEC);
    Serial.print(F("\tt0: "));
    Serial.println(t0,DEC);
    Serial.print(F("  d1: "));
    Serial.print(d1,DEC);
    Serial.print(F("\tt1: "));
    Serial.println(t1,DEC);
    Serial.print(F("  d2: "));
    Serial.print(d2,DEC);
    Serial.print(F("\tt2: "));
    Serial.println(t2,DEC);
    Serial.print(F("  d3: "));
    Serial.print(d3,DEC);
    Serial.print(F("\tt3: "));
    Serial.println(t3,DEC);
    #endif
    
    // calculations for time to next step
    //         t0 = now
    // d<d1  d(t) = t*t*accel
    //       t(d) = sqrt(d/accel)
    //         t1 = t(d1)
    // d<d2  d(t) = d1 + (t-t1)*speed
    //       t(d) = t1 + (d-d1)/speed
    //         t2 = t(d2)
    // d<d3  d(t) = d2 + (t-t2)(speed-(t-t2)*accel)
    //       d(t) = d3 - (t3-t)(t3-t)*accel
    //       t(d) = t3 - sqrt((d3-d)/accel)
    
    uint32_t t_next = 0;
    uint32_t t_start = micros();
    state = MOVE_ACCEL;
    for(uint32_t steps=0;steps<=d3;steps++)
    {
        switch(state)
        {
            case MOVE_ACCEL: t_next = t0 + (uint32_t)(sqrt(float(steps)*accel_inv)); break;
            case MOVE_SLEW:  t_next = t1 + (uint32_t)((float)(steps-d1)*speed_inv); break;
            case MOVE_DECEL: t_next = t3 - (uint32_t)(sqrt((float)(d3-steps)*accel_inv)); break;
        }

        if(steps==d1) state = MOVE_SLEW;
        else if(steps==d2) state = MOVE_DECEL;
        else if(steps==d3) state = MOVE_STOP;
        
        while(true)
        {
            if(micros()>=(t_next+t_start))
            {
                break;
            }
        }
        step();   
    }
    #ifdef DEBUG
    Serial.println(F("  move complete"));
    #endif
}

void LinearStage::wait_move()
{
    while(state != MOVE_STOP && state != MOVE_STALLED)
    {
        delay(1);
    }
}