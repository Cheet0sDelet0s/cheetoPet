#pragma once

#if defined(PLATFORM_PC)

    #include "display/pc/DisplaySDL2.h"
    #include "input/pc/InputSDL2.h"
    #include "rtc/pc/RTCHost.h"
    #include "battery/pc/PCBattery.h"
    #include "imu/pc/IMUHost.h"
    #include "audio/pc/AudioSDL2.h"

    namespace platform {
        using Display = display::DisplaySDL2;
        using Input = input::InputSDL2;
        using RTC = rtc::RTCHost;
        using Battery = battery::PCBattery;
        using IMU = imu::IMUHost;
        using Audio = audio::AudioSDL2;
    }

#elif defined(PLATFORM_ESP32)

    #include "display/esp32/DisplayST7789.h"
    #include "input/esp32/InputESP32.h"
    #include "rtc/esp32/RTC___.h" // name not decided

    namespace platform {
        using Display = display::DisplayST7789;
        using Input = input::InputESP32;
        using RTC = rtc::RTC___; // name not decided
    }

#else

    #error "No platform selected. Define PLATFORM_PC or PLATFORM_ESP32."

#endif