#pragma once

#include <cstdint>

namespace rtc {

struct DateTime
{
    uint16_t year;

    uint8_t month;
    uint8_t day;

    uint8_t hour;
    uint8_t minute;
    uint8_t second;

    uint8_t dayOfWeek;
};

class RTC
{
public:
    virtual ~RTC() = default;

    virtual bool begin() = 0;

    virtual bool isRunning() const = 0;

    virtual bool read(DateTime& dateTime) = 0;

    virtual bool write(const DateTime& dateTime) = 0;
};

} // namespace rtc