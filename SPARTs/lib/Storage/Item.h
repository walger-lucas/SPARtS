#pragma once
#include <unordered_map>
#include <Arduino.h>
#include <string>
class Item {
    static const std::unordered_map<std::string,uint8_t> forward;
    static const std::unordered_map<uint8_t,std::string> backward;
    static const std::array<int,13> max_item; 
    static const std::array<float,13> weight_per_item; 

    public:

    static uint8_t getId(String& name)
    {
        auto it = forward.find(name.c_str());
        if(it != forward.end())
            return it->second;
        return 0;
    }

    static uint8_t getMaxAmount(uint8_t id)
    {
        return (id>=0 && id<max_item.size()) ? max_item[id] : max_item[0];
    }

    static uint8_t getWeight(uint8_t id)
    {
        return (id>=0 && id<max_item.size()) ? weight_per_item[id] : weight_per_item[0];
    }

    

    static String getName(uint8_t id)
    {
        auto it = backward.find(id);
        if(it != backward.end())
            return String(it->second.c_str());
        return "None";
    }

};