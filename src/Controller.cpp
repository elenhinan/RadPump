#include "Controller.h"

Controller::Controller(HardwareSerial* stream) :
    stream(stream)
{

}

void Controller::rxPacket()
{
    if(stream->available())
        return;
    
    JsonObject& root = jsonBuffer.parseObject(*stream);
    if(!root.success())
        return;

    if(root.containsKey("injector"))
    {
        uint8_t injector = root["injector"].as<uint8_t>();

        // todo add iterator: for(auto& syr : syringe) { loop }
        if(root.containsKey("isotope"))
        {
            JsonObject& isotope = root["isotope"];
            const char* isotope_name = isotope["name"].asString(); // "F18"
            float isotope_halflife = isotope["halflife"].as<float>(); // "109.7"
            const char* isotope_t0 = isotope["t0"].asString(); // "20180211-151035.23"
            if(isotope.containsKey("slot"))
                int8_t isotope_slot = isotope["slot"].as<int8_t>();

        }
        if(root.containsKey("syringe"))
        {
            JsonObject& syringe = root["syringe"];
            const char* syringe_name = syringe["name"]; // "Omnifix 1ml"
            float syringe_volume = syringe["volume"].as<float>(); // "1.0f"
            float syringe_end = syringe["end"].as<float>(); // "14.5"
            float syringe_start = syringe["start"].as<float>(); // "71.8f"
            if(syringe.containsKey("slot"))
                int8_t syringe_slot = syringe["slot"].as<int8_t>();
        }
        if(root.containsKey("injection"))
        {
            JsonObject& injection = root["injection"];
            const char* injection_time = injection["time"]; // "20180211-152035.32"
            float injection_amount = injection["amount"].as<float>(); // "20" unit
            int8_t unit;
            {
                const char* injection_unit = injection["type"].asString(); // 0:"ml" / 1:"MBq"/ 1:"mm" unit
                //if(stricmp(injection_unit, "ml")) unit = 0;
                //else if(stricmp(injection_unit, "MBq")) unit = 1;
                //else if(stricmp(injection_unit, "mm")) unit = 2;
            }
            float injection_speed = injection["speed"].as<float>(); // "0.01" unit/s
            float injection_duration = injection["duration"].as<float>(); // "150" s
        }
    }



    const char* command_cmd = root["command"]["cmd"]; // "move"
    const char* command_param = root["command"]["param"]; // "20"
}

void Controller::txPacket()
{
    JsonObject& root = jsonBuffer.createObject();
    root["injector"] = "A";

    JsonObject& isotope = root.createNestedObject("isotope");
    isotope["name"] = "F18";
    isotope["halflife"] = "109.7";
    isotope["t0"] = "20180211-151035.23";

    JsonObject& syringe = root.createNestedObject("syringe");
    syringe["name"] = "Omnifix 1ml";
    syringe["volume"] = "1.0f";
    syringe["end"] = "14.5";
    syringe["start"] = "71.8f";

    JsonObject& injection = root.createNestedObject("injection");
    injection["time"] = "20180211-152035.32";
    injection["amount"] = "20";
    injection["type"] = "volume";
    injection["speed"] = "0.01";
    injection["duration"] = "150";

    JsonObject& command = root.createNestedObject("command");
    command["cmd"] = "move";
    command["param"] = "20";

    root.printTo(Serial);
}