#include <common/maths.hpp>

// Quaternions
Quaternion::Quaternion() {}

Quaternion::Quaternion(const float w, const float x, const float y, const float z)
{
    this->w = w;
    this->x = x;
    this->y = y;
    this->z = z;
}

Quaternion::Quaternion(const float pitch, const float yaw)
{
    float cosPitch = cos(0.5f * pitch);
    float sinPitch = sin(0.5f * pitch);
    float cosYaw = cos(0.5f * yaw);
    float sinYaw = sin(0.5f * yaw);

    this->w = cosPitch * cosYaw;
    this->x = sinPitch * cosYaw;
    this->y = cosPitch * sinYaw;
    this->z = sinPitch * sinYaw;
}

glm::mat4 Quaternion::matrix()
{
    float s = 2.0f / (w * w + x * x + y * y + z * z);
    float xs = x * s, ys = y * s, zs = z * s;
    float xx = x * xs, xy = x * ys, xz = x * zs;
    float yy = y * ys, yz = y * zs, zz = z * zs;
    float xw = w * xs, yw = w * ys, zw = w * zs;

    glm::mat4 rotate;
    rotate[0][0] = 1.0f - (yy + zz);
    rotate[0][1] = xy + zw;
    rotate[0][2] = xz - yw;
    rotate[1][0] = xy - zw;
    rotate[1][1] = 1.0f - (xx + zz);
    rotate[1][2] = yz + xw;
    rotate[2][0] = xz + yw;
    rotate[2][1] = yz - xw;
    rotate[2][2] = 1.0f - (xx + yy);

    return rotate;
}


// SLERP
Quaternion Maths::SLERP(Quaternion q1, Quaternion q2, const float t)
{
    // Calculate cos(theta)
    float cosTheta = q1.w * q2.w + q1.x * q2.x + q1.y * q2.y + q1.z * q2.z;

    // If q1 and q2 are close together return q2 to avoid divide by zero errors
    if (cosTheta > 0.9999f)
        return q2;

    // Avoid taking the long path around the sphere by reversing sign of q2
    if (cosTheta < 0)
    {
        q2 = Quaternion(-q2.w, -q2.x, -q2.y, -q2.z);
        cosTheta = -cosTheta;
    }

    // Calculate SLERP
    Quaternion q;
    float theta = acos(cosTheta);
    float a = sin((1.0f - t) * theta) / sin(theta);
    float b = sin(t * theta) / sin(theta);
    q.w = a * q1.w + b * q2.w;
    q.x = a * q1.x + b * q2.x;
    q.y = a * q1.y + b * q2.y;
    q.z = a * q1.z + b * q2.z;

    return q;
}

glm::mat4 Maths::translate(const glm::vec3& v)
{
    glm::mat4 translate(1.0f);
    translate[3][0] = v.x, translate[3][1] = v.y, translate[3][2] = v.z;
    return translate;
}

glm::mat4 Maths::scale(const glm::vec3& v)
{
    glm::mat4 scale(1.0f);
    scale[0][0] = v.x; scale[1][1] = v.y; scale[2][2] = v.z;
    return scale;
}

glm::mat4 Maths::transpose(const glm::mat4& m)
{
    glm::mat4 transposedMatrix;
    transposedMatrix[0][0] = m[0][0];
    transposedMatrix[0][1] = m[1][0];
    transposedMatrix[0][2] = m[2][0];
    transposedMatrix[0][3] = m[3][0];

    transposedMatrix[1][0] = m[0][1];
    transposedMatrix[1][1] = m[1][1];
    transposedMatrix[1][2] = m[2][1];
    transposedMatrix[1][3] = m[3][1];

    transposedMatrix[2][0] = m[0][2];
    transposedMatrix[2][1] = m[1][2];
    transposedMatrix[2][2] = m[2][2];
    transposedMatrix[2][3] = m[3][2];

    transposedMatrix[3][0] = m[0][3];
    transposedMatrix[3][1] = m[1][3];
    transposedMatrix[3][2] = m[2][3];
    transposedMatrix[3][3] = m[3][3];
    return transposedMatrix;
}

float Maths::radians(float angle)
{
    return angle * 3.1416f / 180.0;
}

float Maths::length(const glm::vec3& v)
{  
    return sqrtf(powf(v.x, 2) + powf(v.y, 2) + powf(v.z, 2));
}

glm::vec3 Maths::normalize(const glm::vec3& v)
{
    return v / Maths::length(v);
}

float Maths::dot(const glm::vec3& v1, const glm::vec3& v2)
{
    return v1.x*v2.x + v1.y*v2.y + v1.z*v2.z;
}

glm::vec3 Maths::cross(const glm::vec3& v1, const glm::vec3& v2)
{
    return glm::vec3(v1.y*v2.z - v1.z*v2.y, v1.z*v2.x - v1.x*v2.z, v1.x*v2.y - v1.y*v2.x);
}

glm::mat4 Maths::rotate(const float& angle, glm::vec3& v)
{
    v = Maths::normalize(v);
    float c = cos(0.5f * angle);
    float s = sin(0.5f * angle);
    Quaternion q(c, s * v.x, s * v.y, s * v.z);

    return q.matrix();

    //Non Quaternion Rotation
    /*v = glm::normalize(v);
    float c = cos(angle);
    float s = sin(angle);

    float x2 = v.x * v.x, y2 = v.y * v.y, z2 = v.z * v.z;
    float xy = v.x * v.y, xz = v.x * v.z, yz = v.y * v.z;
    float xs = v.x * s, ys = v.y * s, zs = v.z * s;

    glm::mat4 rotate;
    rotate[0][0] = (1 - c) * x2 + c;
    rotate[0][1] = (1 - c) * xy + zs;
    rotate[0][2] = (1 - c) * xz - ys;
    rotate[1][0] = (1 - c) * xy - zs;
    rotate[1][1] = (1 - c) * y2 + c;
    rotate[1][2] = (1 - c) * yz + xs;
    rotate[2][0] = (1 - c) * xz + ys;
    rotate[2][1] = (1 - c) * yz - xs;
    rotate[2][2] = (1 - c) * z2 + c;

    return rotate; */
}

float Maths::clamp(float number, float min, float max)
{
    if (number < min) {
        number = min;
    }
    if (number > max) {
        number = max;
    }
    return number;
}
