#pragma once

#include "../IMU.h"

#include <SDL2/SDL.h>

namespace imu {

class IMUHost : public IMU
{
public:
    IMUHost();

    bool begin() override;
    void update() override;

    GyroData gyro() const override;
    AccelData acceleration() const override;

    void setMouseControlEnabled(bool enabled);
    bool isMouseControlEnabled() const;

    void setMouseSensitivity(float sensitivity);

    void resetOrientation();

    float pitch() const;
    float roll() const;
    float yaw() const;

private:
    float pitch_;
    float roll_;
    float yaw_;

    float previousPitch_;
    float previousRoll_;
    float previousYaw_;

    GyroData gyro_;
    AccelData acceleration_;

    float mouseSensitivity_;

    bool mouseControlEnabled_;

    Uint64 lastUpdateTime_;

    float velocityX_;
    float velocityY_;
    float velocityZ_;

    float driveAcceleration_;
};

} // namespace imu