#pragma once

#include "vector.h"

class Body
{
    public:
    double m = 1.0;
    Vector pos = Vector();
    Vector vel = Vector();

    Body() {}

    Body(double m, Vector pos, Vector vel)
    {
        this->m = m;
        this->pos = pos;
        this->vel = vel;
    }
};