#include <stdlib.h>
#include <stdio.h>
#include <math.h>
#include <string>
#include <random>

#define KAPPA 6.673e-11
#define EPS 1e-14

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

class Vector
{
    public:
    double x = 0.0, y = 0.0, z = 0.0;

    Vector() {}

    Vector(double x, double y, double z)
    {
        this->x = x;
        this->y = y;
        this->z = z;
    }

    Vector operator+(Vector &v2)
    {
        return Vector(x + v2.x, y + v2.y, z + v2.z);
    }

    void operator+=(const Vector &v2)
    {
        this->x += v2.x;
        this->y += v2.y;
        this->z += v2.z;
    }

    Vector operator-(Vector &v2)
    {
        return Vector(x - v2.x, y - v2.y, z - v2.z);
    }

    Vector operator*(double s)
    {
        return Vector(s * x, s * y, s * z);
    }

    Vector operator/(double s)
    {
        return Vector(x / s, y / s, z / s);
    }


    double length()
    {
        return sqrt(x * x + y * y + z * z);
    }
};

Vector force(Body b_1, Body b_2)
{
    Vector diff = b_1.pos - b_2.pos;
    double dist = diff.length() + EPS;

    return diff * ((b_1.m * b_2.m) / (dist * dist * dist) * -KAPPA);
}

int main(int argc, char* argv[])
{
    if (argc < 2)
    {
        return 1;
    }
    double N = std::stod(argv[1]);
    Body* bodies = new Body[N];
    

    std::random_device rd;
    std::mt19937 gen(rd());
    std::uniform_real_distribution<double> distrib_mass(100.0, 10000.0);
    std::uniform_real_distribution<double> distrib_pos(-1000.0, 1000.0);
    std::uniform_real_distribution<double> distrib_vel(-10.0, 10.0);

    for (int i = 0; i < N; ++i)
    {
        bodies[i].m = distrib_mass(gen);
        bodies[i].pos = Vector(distrib_pos(gen), distrib_pos(gen), distrib_pos(gen));
        bodies[i].vel = Vector(distrib_vel(gen), distrib_vel(gen), distrib_vel(gen));
    }

    for (int i = 0; i < N; ++i)
    {
        Vector force_sum = Vector();
        for (int j = 0; j < N; ++j)
        {   
            if (i != j)
            {
                force_sum += force(bodies[i], bodies[j]);
            }
        }
    }

}