#include "IMUHost.h"

#include <cmath>
#include <algorithm>

namespace imu {

namespace {

constexpr float GRAVITY = 9.80665f;

// Maximum simulated angular velocity in radians/sec.
constexpr float MAX_ANGULAR_VELOCITY = 6.0f;

// Mouse movement is converted into angular velocity.
constexpr float MOUSE_SENSITIVITY_DEFAULT = 0.005f;

// Maximum acceleration produced by W/S.
constexpr float MAX_DRIVE_ACCELERATION = 5.0f;

// How quickly W/S acceleration changes.
constexpr float DRIVE_ACCELERATION_RATE = 10.0f;

// Maximum simulated velocity.
constexpr float MAX_VELOCITY = 5.0f;

float clamp(
    float value,
    float minimum,
    float maximum)
{
    return std::max(
        minimum,
        std::min(maximum, value)
    );
}

} // namespace


IMUHost::IMUHost()
    : pitch_(0.0f),
      roll_(0.0f),
      yaw_(0.0f),

      previousPitch_(0.0f),
      previousRoll_(0.0f),
      previousYaw_(0.0f),

      gyro_{},
      acceleration_{},

      mouseSensitivity_(MOUSE_SENSITIVITY_DEFAULT),

      mouseControlEnabled_(false),

      lastUpdateTime_(0),

      velocityX_(0.0f),
      velocityY_(0.0f),
      velocityZ_(0.0f),

      driveAcceleration_(0.0f)
{
}


bool IMUHost::begin()
{
    if (!(SDL_WasInit(SDL_INIT_EVENTS) & SDL_INIT_EVENTS))
    {
        if (SDL_InitSubSystem(SDL_INIT_EVENTS) != 0)
            return false;
    }

    lastUpdateTime_ = SDL_GetPerformanceCounter();

    resetOrientation();

    return true;
}


void IMUHost::update()
{
    const Uint64 currentTime =
        SDL_GetPerformanceCounter();

    const double frequency =
        static_cast<double>(
            SDL_GetPerformanceFrequency()
        );

    float deltaTime =
        static_cast<float>(
            static_cast<double>(
                currentTime - lastUpdateTime_
            ) / frequency
        );

    lastUpdateTime_ = currentTime;

    // Prevent a large jump if the debugger pauses
    // or the application stalls.
    deltaTime = clamp(
        deltaTime,
        0.0001f,
        0.1f
    );


    /*
     * Save the previous orientation so that we can
     * calculate angular velocity later.
     */
    previousPitch_ = pitch_;
    previousRoll_  = roll_;
    previousYaw_   = yaw_;


    /*
     * Mouse control
     */
    if (mouseControlEnabled_)
    {
        int mouseX = 0;
        int mouseY = 0;

        SDL_GetRelativeMouseState(
            &mouseX,
            &mouseY
        );

        yaw_ +=
            static_cast<float>(mouseX) *
            mouseSensitivity_;

        pitch_ +=
            static_cast<float>(mouseY) *
            mouseSensitivity_;
    }


    /*
     * Keyboard control
     */
    const Uint8* keyboard =
        SDL_GetKeyboardState(nullptr);


    /*
     * Q/E controls roll.
     */
    float rollInput = 0.0f;

    if (keyboard[SDL_SCANCODE_Q])
        rollInput -= 1.0f;

    if (keyboard[SDL_SCANCODE_E])
        rollInput += 1.0f;

    roll_ +=
        rollInput *
        MAX_ANGULAR_VELOCITY *
        deltaTime;


    /*
     * Keep pitch and roll within sensible ranges.
     */
    pitch_ = clamp(
        pitch_,
        -1.5708f,
        1.5708f
    );

    roll_ = clamp(
        roll_,
        -1.5708f,
        1.5708f
    );


    /*
     * Calculate angular velocity.
     *
     * This is deliberately based on the change in
     * orientation rather than simply returning the
     * mouse movement.
     */
    gyro_.x =
        (pitch_ - previousPitch_) /
        deltaTime;

    gyro_.y =
        (roll_ - previousRoll_) /
        deltaTime;

    gyro_.z =
        (yaw_ - previousYaw_) /
        deltaTime;


    gyro_.x = clamp(
        gyro_.x,
        -MAX_ANGULAR_VELOCITY,
        MAX_ANGULAR_VELOCITY
    );

    gyro_.y = clamp(
        gyro_.y,
        -MAX_ANGULAR_VELOCITY,
        MAX_ANGULAR_VELOCITY
    );

    gyro_.z = clamp(
        gyro_.z,
        -MAX_ANGULAR_VELOCITY,
        MAX_ANGULAR_VELOCITY
    );


    /*
    * W/S controls simulated forward acceleration.
    */
    float driveInput = 0.0f;

    if (keyboard[SDL_SCANCODE_W])
        driveInput += 1.0f;

    if (keyboard[SDL_SCANCODE_S])
        driveInput -= 1.0f;

    const float targetAcceleration =
        driveInput * MAX_DRIVE_ACCELERATION;


    /*
    * Smoothly approach the requested acceleration.
    */
    const float accelerationDifference =
        targetAcceleration - driveAcceleration_;

    driveAcceleration_ +=
        accelerationDifference *
        clamp(
            DRIVE_ACCELERATION_RATE * deltaTime,
            0.0f,
            1.0f
        );


    /*
    * Integrate acceleration into velocity.
    */
    velocityZ_ +=
        driveAcceleration_ * deltaTime;

    velocityZ_ = clamp(
        velocityZ_,
        -MAX_VELOCITY,
        MAX_VELOCITY
    );


    /*
     * The accelerometer measures acceleration INCLUDING
     * gravity.
     *
     * Start with gravity pointing down in world space,
     * then rotate it according to the virtual device
     * orientation.
     */

    const float sinPitch = std::sin(pitch_);
    const float cosPitch = std::cos(pitch_);

    const float sinRoll = std::sin(roll_);
    const float cosRoll = std::cos(roll_);


    /*
     * Gravity vector in device coordinates.
     *
     * Device is assumed to be flat at:
     *
     * X = 0
     * Y = 0
     * Z = +9.81
     */
    float gravityX =
        -GRAVITY * sinRoll;

    float gravityY =
        GRAVITY *
        sinPitch *
        cosRoll;

    float gravityZ =
        GRAVITY *
        cosPitch *
        cosRoll;


    /*
     * Linear acceleration from W/S.
     *
     * This is acceleration along the device's forward
     * axis.
     */
    float linearX = 0.0f;
    float linearY = 0.0f;
    float linearZ = 0.0f;

    /*
     * Rotate the forward acceleration according to
     * the virtual orientation.
     */
    const float forwardX =
        std::sin(yaw_) * cosPitch;

    const float forwardY =
        -sinPitch;

    const float forwardZ =
        std::cos(yaw_) * cosPitch;


    linearX +=
        forwardX *
        driveAcceleration_;

    linearY +=
        forwardY *
        driveAcceleration_;

    linearZ +=
        forwardZ *
        driveAcceleration_;


    acceleration_.x =
        gravityX + linearX;

    acceleration_.y =
        gravityY + linearY;

    acceleration_.z =
        gravityZ + linearZ;
}


GyroData IMUHost::gyro() const
{
    return gyro_;
}


AccelData IMUHost::acceleration() const
{
    return acceleration_;
}


void IMUHost::setMouseControlEnabled(
    bool enabled)
{
    mouseControlEnabled_ = enabled;

    SDL_SetRelativeMouseMode(
        enabled ? SDL_TRUE : SDL_FALSE
    );
}


bool IMUHost::isMouseControlEnabled() const
{
    return mouseControlEnabled_;
}


void IMUHost::setMouseSensitivity(
    float sensitivity)
{
    mouseSensitivity_ = sensitivity;
}


void IMUHost::resetOrientation()
{
    pitch_ = 0.0f;
    roll_ = 0.0f;
    yaw_ = 0.0f;

    previousPitch_ = 0.0f;
    previousRoll_ = 0.0f;
    previousYaw_ = 0.0f;

    gyro_ = {};
    acceleration_ = {};

    velocityX_ = 0.0f;
    velocityY_ = 0.0f;
    velocityZ_ = 0.0f;

    driveAcceleration_ = 0.0f;
}


float IMUHost::pitch() const
{
    return pitch_;
}


float IMUHost::roll() const
{
    return roll_;
}


float IMUHost::yaw() const
{
    return yaw_;
}

} // namespace imu