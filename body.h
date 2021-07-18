#pragma once

#include "vector.h"

#define KAPPA 6.673e-11
#define EPS 1e-8

struct Body
{
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

    // Force towards body b
    Vector force(const Body &b)
    {
        Vector diff = this->pos - b.pos;
        double dist = diff.length() + EPS;

        return diff * (this->m * b.m / (dist * dist * dist) * -KAPPA);
    }

    // Acceleration towards body b
    Vector acceleration(const Body &b)
    {
        Vector diff = this->pos - b.pos;
        double dist = diff.length() + EPS;

        return diff * (b.m / (dist * dist * dist) * -KAPPA);
    }
};
