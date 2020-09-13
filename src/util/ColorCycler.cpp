//
//  ColorCycler.cpp
//  LSCM
//
//  Created by Kyle on 5/5/18.
//  Copyright © 2018 Kyle. All rights reserved.
//

#include "ColorCycler.h"

const Mesh::Color ColorCycler::_Colors[] =
{
    {166,206,227},
    {31,120,180},
    {178,223,138},
    {51,160,44},
    {251,154,153},
    {227,26,28},
    {253,191,111},
    {255,127,0},
    {202,178,214},
    {106,61,154},
    {255,255,153},
    {177,89,40}
};
//{
//    {255, 0, 0},
//    {0, 255, 0},
//    {0, 0, 255},
//    {255, 255, 0},
//    {255, 0, 255},
//    {0, 255, 255},
//    {0, 255, 128},
//    {0, 128, 256},
//    {255, 0, 128},
//    {128, 0, 255},
//    {255, 128, 0},
//    {128, 255, 0},
//};


const size_t ColorCycler::_Num = 12;

ColorCycler ColorCycler::_Shared;

ColorCycler& ColorCycler::Shared()
{
    return _Shared;
}

ColorCycler::ColorCycler()
: _index(0)
{
}

Mesh::Color ColorCycler::color()
{
    const auto& c = _Colors[_index];
    
    _index++;
    if (_index >= _Num)
        _index = 0;
    
    return c;
}
