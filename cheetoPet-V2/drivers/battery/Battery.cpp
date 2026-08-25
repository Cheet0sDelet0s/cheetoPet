#include "Battery.h"

namespace battery {

const VoltageMapping batteryTable[] = {
    {4.20f, 100.0f},
    {4.05f,  90.0f},
    {3.95f,  80.0f},
    {3.85f,  70.0f},
    {3.80f,  60.0f},
    {3.75f,  50.0f},
    {3.70f,  40.0f},
    {3.65f,  30.0f},
    {3.55f,  20.0f},
    {3.40f,  10.0f},
    {3.00f,   0.0f}
};

const size_t TABLE_SIZE = sizeof(batteryTable) / sizeof(batteryTable[0]);

float Battery::getBatteryPercentage(float measuredVoltage) {
    // 1. Handle upper and lower boundaries immediately
    if (measuredVoltage >= batteryTable[0].voltage) return 100.0f;
    if (measuredVoltage <= batteryTable[TABLE_SIZE - 1].voltage) return 0.0f;

    // 2. Find the two boundary points our measured voltage falls between
    for (size_t i = 0; i < TABLE_SIZE - 1; ++i) {
        const VoltageMapping& upper = batteryTable[i];
        const VoltageMapping& lower = batteryTable[i + 1];

        if (measuredVoltage <= upper.voltage && measuredVoltage > lower.voltage) {
            // 3. Linear interpolation formula
            float voltageRange = upper.voltage - lower.voltage;
            float percentRange = upper.percentage - lower.percentage;
            
            float percentage = lower.percentage + ((measuredVoltage - lower.voltage) * percentRange) / voltageRange;
            return percentage;
        }
    }
    
    return 0.0f; // Fallback safety
}

} // namespace battery