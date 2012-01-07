/*
Joseph Dombrowski - 1073257
Rohit Nirmal - 0848815

Computer Graphics
*/
#include <iostream>
class glVector3
{
private:
    float v[3];
public:
    glVector3();
    glVector3(float x, float y, float z);

    float &operator[](int index);
    float operator[](int index) const;

    glVector3 operator*(float scale) const;
    glVector3 operator/(float scale) const;
    glVector3 operator+(const glVector3 &other) const;
    glVector3 operator-(const glVector3 &other) const;
    glVector3 operator-() const;

    const glVector3 &operator*=(float scale);
    const glVector3 &operator/=(float scale);
    const glVector3 &operator+=(const glVector3 &other);
    const glVector3 &operator-=(const glVector3 &other);

    float magnitude() const;
    float magnitudeSquared() const;
    glVector3 normalize() const;
    float dot(const glVector3 &other) const;
    glVector3 cross(const glVector3 &other) const;
};

glVector3 operator*(float scale, const glVector3 &v);
std::ostream &operator<<(std::ostream &output, const glVector3 &v);