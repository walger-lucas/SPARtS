#pragma once
#include <unordered_map>
#include <Arduino.h>
#include <string>
class Item {
    static const std::unordered_map<std::string,uint8_t> forward;
    static const std::unordered_map<uint8_t,std::string> backward;

    public:

    static uint8_t getId(String& name)
    {
        auto it = forward.find(name.c_str());
        if(it != forward.end())
            return it->second;
        return 0;
    }

    static String getName(uint8_t id)
    {
        auto it = backward.find(id);
        if(it != backward.end())
            return String(it->second.c_str());
        return "None";
    }

};