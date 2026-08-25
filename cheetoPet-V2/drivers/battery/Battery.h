#pragma once

#include <cstddef>

namespace battery {

struct VoltageMapping {
    float voltage;
    float percentage;
};

class Battery
{
public:
    float voltage;
    float percentage;

    virtual bool begin() = 0;
    virtual void read() = 0;
    
    virtual float getPercentage() = 0;
protected:
    float getBatteryPercentage(float measuredVoltage);
};

} // namespace battery