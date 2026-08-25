#pragma once

namespace imu {

struct GyroData
{
    float x;
    float y;
    float z;
};

struct AccelData
{
    float x;
    float y;
    float z;
};

class IMU
{
public:
    virtual ~IMU() = default;

    virtual bool begin() = 0;
    virtual void update() = 0;

    virtual GyroData gyro() const = 0;
    virtual AccelData acceleration() const = 0;
};

}