#include "PCBattery.h"

namespace battery {

bool PCBattery::begin()
{
    PCBattery::voltage = 3.75;
    PCBattery::percentage = 50.0f;

    return true;
}

void PCBattery::read()
{
    PCBattery::voltage += 0.01;

    if (PCBattery::voltage > 4.20) PCBattery::voltage = 3.40;

    PCBattery::percentage = getBatteryPercentage(PCBattery::voltage);
}

float PCBattery::getPercentage()
{
    return PCBattery::percentage;
}

} // namespace battery