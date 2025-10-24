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

const std::array<int,13> Item::max_item {0, 5, 32, 50, 30, 30, 20, 11, 12, 45, 14, 30, 45};

const std::array<float,13> Item::weight_per_item {10000.0, 5.54, 2.25, 1.94, 3.52 , 3.55, 4.58, 8.91, 7.89, 2.18, 7.12, 3.33, 2.04};