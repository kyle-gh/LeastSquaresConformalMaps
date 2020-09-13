
#pragma once

#include "../util/MeshDef.h"

class ColorCycler
{
private:
    const static Mesh::Color _Colors[];
    const static size_t _Num;
    static ColorCycler _Shared;

    int _index;
    
public:
    static ColorCycler& Shared();

    ColorCycler();

    Mesh::Color color();
    
    Mesh::Color next() { return color(); }
};
