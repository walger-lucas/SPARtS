#include "Item.h"

const std::unordered_map<std::string,uint8_t> Item::forward ={
        {"None",0},
        {"cantoneira-maior",1},
        {"cantoneira-menor",2},
        {"paraf-m4-dourado-15mm",3},
        {"paraf-m5-aco-15mm",4},
        {"paraf-m5-inox-15mm",5},
        {"paraf-m6-zincado-10mm",6},
        {"paraf-m8-aco-20mm",7},
        {"paraf-m8-inox-15mm",8},
        {"porc-m5-vslot-canal8",9},
        {"porc-m6-martelo-base30",10},
        {"porc-m6-martelo-canal8",11},
        {"porc-m6-sextavada",12}
    };

const std::unordered_map<uint8_t,std::string> Item::backward ={
        {0,"None"},
        {1,"cantoneira-maior"},
        {2,"cantoneira-menor"},
        {3,"paraf-m4-dourado-15mm"},
        {4,"paraf-m5-aco-15mm"},
        {5,"paraf-m5-inox-15mm"},
        {6,"paraf-m6-zincado-10mm"},
        {7,"paraf-m8-aco-20mm"},
        {8,"paraf-m8-inox-15mm"},
        {9,"porc-m5-vslot-canal8"},
        {10,"porc-m6-martelo-base30"},
        {11,"porc-m6-martelo-canal8"},
        {12,"porc-m6-sextavada"}
    };

const std::array<int,13> Item::max_item {0,25,25,25,25,25,25,25,25,25,25,25,25};

const std::array<float,13> Item::weight_per_item {0.1,3.0,5.0,5.0,3.0,2.0,15.0,10.0,9.0,1.0,5.0,3.0,100.0};