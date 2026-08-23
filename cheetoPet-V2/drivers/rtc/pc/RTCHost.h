#pragma once

#include "../RTC.h"

namespace rtc {

class RTCHost : public RTC
{
public:
    bool begin() override;

    bool isRunning() const override;

    bool read(DateTime& dateTime) override;

    bool write(const DateTime& dateTime) override;
};

} // namespace rtc