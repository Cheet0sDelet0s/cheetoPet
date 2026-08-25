#pragma once

#include "../Battery.h"

namespace battery {

class PCBattery : public Battery
{
public:
    bool begin() override;
    void read() override;

    bool getChargingState() override;

    float getVoltage() override;
    float getPercentage() override;
};
} // namespace battery