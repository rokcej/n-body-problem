#pragma once

struct Vector
{
    double x = 0.0, y = 0.0, z = 0.0;

    Vector() {}

    Vector(const Vector &v) {
        this->x = v.x;
        this->y = v.y;
        this->z = v.z;
    }

    Vector(double x, double y, double z)
    {
        this->x = x;
        this->y = y;
        this->z = z;
    }

    Vector operator+(const Vector &v2)
    {
        return Vector(x + v2.x, y + v2.y, z + v2.z);
    }

    Vector& operator+=(const Vector &v2)
    {
        this->x += v2.x;
        this->y += v2.y;
        this->z += v2.z;

        return *this;
    }

    Vector operator-(const Vector &v2)
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

    void print_vector()
    {
        printf("x = %lf, y = %lf, z = %lf\n", x, y, z);
    }
};