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

void LinearStage::init()
{
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
    pinMode(this->pinSTALL, INPUT);

    stepper_sgt = STALL_VALUE;

    this->stepper = new TMC2130Stepper(pinEN, pinDIR, pinSTEP, pinCS);
    this->setup_driver();

    stalled = false;
    stallguard_enabled = false;
    
    switch(digitalPinToInterrupt(pinSTALL))
    {
        case 0: Serial.println(F("  INT0 attached")); int0_cb_instance = this; attachInterrupt(0, int0_callback, RISING); break;
        case 1: Serial.println(F("  INT1 attached")); int1_cb_instance = this; attachInterrupt(1, int1_callback, RISING); break;
        default: Serial.println(F("  No Interrupt")); break;
    }
    Serial.println(F("  done!"));
}

void LinearStage::stall_event()
{
    stalled = stallguard_enabled;
    //Serial.print("Stage #");
    //Serial.print(number,DEC);
    //Serial.println("Stall!");
    //Serial.println(number,DEC);
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
}

void LinearStage::stealthchop(bool enable)
{
    stepper->stealthChop(enable);
    stepper->coolstep_min_speed(enable?0:0xfffff);
}

void LinearStage::home(uint8_t home_dir)
{
    Serial.print(F("Stage #"));
    Serial.print(number,DEC);
    Serial.println(F(" homing"));

    stealthchop(false);
    stallguard(true);
    if(home_dir == DIR_BOTH || home_dir == DIR_NEG)
    {
        dir(DIR_NEG);
        while(!stalled)
        {
            delayMicroseconds(HOMING_SPEED);
            step();
        }
        Serial.print(F("  home: 0 ("));
        Serial.print(position,DEC);
        Serial.println(')');
        stalled = false;
        position = 0;
        //move(0);
    }
    if(home_dir == DIR_BOTH || home_dir == DIR_POS)
    {
        dir(DIR_POS);
        while(!stalled)
        {
            delayMicroseconds(HOMING_SPEED);
            step();
        }
        stalled = false;
        endstop = position;
        Serial.print(F("  endstop:"));
        Serial.print(position,DEC);
        //move(endstop);
    }

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
    
    Serial.print(F("Stage #"));
    Serial.print(number,DEC);
    Serial.println(F(" calibrating.."));
    int8_t sgt = 0; // start at zero according to datasheet
    stepper->sg_stall_value(sgt);
    uint16_t repetitions = 0;
    uint16_t steps = 0;
    uint16_t sg_value;
    uint16_t sg_value_max = 0;
    bool sgt_increase = true;
    bool sg_zero = true;
    const unsigned long tstep = (60ul * 1000ul * 1000ul * STEP_DEG / (SG2_TUNE_RPM * MICROSTEPS * 360ul)); // us between steps
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
            Serial.print(F("  SGT: "));
            Serial.print(sgt,DEC);
            Serial.print(F(", SG: "));
            Serial.println(sg_value,DEC);

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
    Serial.print(F("  Tuning done, optimal sgt: "));
    Serial.println(stepper_sgt,DEC);
    Serial.print(F("  Max SG value: "));
    Serial.println(sg_value_max,DEC);

    stepper->sg_filter(0);
    //stepper->stealthChop(true);
}