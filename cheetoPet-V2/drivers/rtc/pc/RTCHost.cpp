#include "RTCHost.h"

#include <chrono>
#include <ctime>

namespace rtc {

bool RTCHost::begin()
{
    return true;
}

bool RTCHost::isRunning() const
{
    return true;
}

bool RTCHost::read(DateTime& dateTime)
{
    const auto now =
        std::chrono::system_clock::now();

    const std::time_t time =
        std::chrono::system_clock::to_time_t(now);

    std::tm localTime{};

    if (!localtime_r(&time, &localTime))
        return false;

    dateTime.year =
        static_cast<uint16_t>(
            localTime.tm_year + 1900
        );

    dateTime.month =
        static_cast<uint8_t>(
            localTime.tm_mon + 1
        );

    dateTime.day =
        static_cast<uint8_t>(
            localTime.tm_mday
        );

    dateTime.hour =
        static_cast<uint8_t>(
            localTime.tm_hour
        );

    dateTime.minute =
        static_cast<uint8_t>(
            localTime.tm_min
        );

    dateTime.second =
        static_cast<uint8_t>(
            localTime.tm_sec
        );

    // tm_wday: Sunday = 0
    dateTime.dayOfWeek =
        static_cast<uint8_t>(
            localTime.tm_wday
        );

    return true;
}

bool RTCHost::write(
    const DateTime&)
{
    /*
     * We deliberately don't change the PC's system clock.
     */
    return false;
}

} // namespace rtc