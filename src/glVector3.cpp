/*
Joseph Dombrowski - 1073257
Rohit Nirmal - 0848815

Computer Graphics
*/
#include <math.h>
#include "glVector3.h"

using namespace std;

glVector3::glVector3()
{
}

glVector3::glVector3(float x, float y, float z)
{
    v[0] = x;
    v[1] = y;
    v[2] = z;
}

float &glVector3::operator[](int index)
{
    return v[index];
}

float glVector3::operator[](int index) const
{
    return v[index];
}

glVector3 glVector3::operator*(float scale) const
{
    return glVector3(v[0] * scale, v[1] * scale, v[2] * scale);
}

glVector3 glVector3::operator/(float scale) const
{
    return glVector3(v[0] / scale, v[1] / scale, v[2] / scale);
}

glVector3 glVector3::operator+(const glVector3 &other) const
{
    return glVector3(v[0] + other.v[0], v[1] + other.v[1], v[2] + other.v[2]);
}

glVector3 glVector3::operator-(const glVector3 &other) const
{
    return glVector3(v[0] - other.v[0], v[1] - other.v[1], v[2] - other.v[2]);
}

glVector3 glVector3::operator-() const
{
    return glVector3(-v[0], -v[1], -v[2]);
}

const glVector3 &glVector3::operator*=(float scale)
{
    v[0] *= scale;
    v[1] *= scale;
    v[2] *= scale;
    return *this;
}

const glVector3 &glVector3::operator/=(float scale)
{
    v[0] /= scale;
    v[1] /= scale;
    v[2] /= scale;
    return *this;
}

const glVector3 &glVector3::operator+=(const glVector3 &other)
{
    v[0] += other.v[0];
    v[1] += other.v[1];
    v[2] += other.v[2];
    return *this;
}

const glVector3 &glVector3::operator-=(const glVector3 &other)
{
    v[0] -= other.v[0];
    v[1] -= other.v[1];
    v[2] -= other.v[2];
    return *this;
}

float glVector3::magnitude() const
{
    return sqrt(v[0] * v[0] + v[1] * v[1] + v[2] * v[2]);
}

float glVector3::magnitudeSquared() const
{
    return v[0] * v[0] + v[1] * v[1] + v[2] * v[2];
}

glVector3 glVector3::normalize() const
{
    float m = sqrt(v[0] * v[0] + v[1] * v[1] + v[2] * v[2]);
    return glVector3(v[0] / m, v[1] / m, v[2] / m);
}

float glVector3::dot(const glVector3 &other) const
{
    return v[0] * other.v[0] + v[1] * other.v[1] + v[2] * other.v[2];
}

glVector3 glVector3::cross(const glVector3 &other) const
{
    return glVector3(v[1] * other.v[2] - v[2] * other.v[1],
                     v[2] * other.v[0] - v[0] * other.v[2],
                     v[0] * other.v[1] - v[1] * other.v[0]);
}

glVector3 operator*(float scale, const glVector3 &v)
{
    return v * scale;
}

ostream &operator<<(ostream &output, const glVector3 &v)
{
    cout << '(' << v[0] << ", " << v[1] << ", " << v[2] << ')';
    return output;
}









