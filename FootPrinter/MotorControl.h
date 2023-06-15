#ifndef MOTORCONTROL_H
#define MOTORCONTROL_H

#include "cppgpio.hpp"
#include "NAU7802.h"

#include <future>

class MotorControl
{
public:
    MotorControl();
    ~MotorControl();

    void SetPositionToZero();
    void PlungeStroke();

    void StopMotor();
    bool MotorStopped();

    void SetNau7802(NAU7802 *nau7802);

    struct LoadData
    {
        int64_t time;
        int32_t position;
        int32_t reading;
    };
    std::vector<LoadData> *GetLoadData();

    int32_t position() const;

    int32_t stepsLimit() const;
    void setStepsLimit(const int32_t &stepsLimit);

    int32_t stepOnDuration() const;
    void setStepOnDuration(const int32_t &stepOnDuration);

    int32_t stepOffDuration() const;
    void setStepOffDuration(const int32_t &stepOffDuration);

    int32_t startingOffset() const;
    void setStartingOffset(const int32_t &startingOffset);

    int32_t readingLimit() const;
    void setReadingLimit(const int32_t &readingLimit);

    int32_t zeroOffset() const;

    int32_t overloadCountLimit() const;
    void setOverloadCountLimit(const int32_t &overloadCountLimit);

private:

    void SetPositionToZeroPrivate();
    void PlungeStrokePrivate();

    bool m_stepsOffsetValid = false;
    std::atomic_int32_t m_stepsOffset = {0}; // offset from the zero point
    int32_t m_stepsLimit = 250 * 200; // 200 steps per mm
    int32_t m_stepOnDuration = 500; // 500 microseconds works, 100 does not
    int32_t m_stepOffDuration = 500; // 500 microseconds works, 100 does not
    int32_t m_startingOffset = 200; // steps to move downwards after zeroing

    // note that pin 4 is pull up by default and this causes motor 2 to be on at startup
	// fix using "gpio=4,12,13,18,19,24=pd" in /boot/config.txt on RPi
    GPIO::DigitalIn m_stopSensor = {23};
    GPIO::DigitalOut m_motor1Enable = {12};
    GPIO::DigitalOut m_motor1Direction = {13};
    GPIO::DigitalOut m_motor1Step = {19};
    GPIO::DigitalOut m_motor2Enable = {4};
    GPIO::DigitalOut m_motor2Direction = {24};
    GPIO::DigitalOut m_motor2Step = {18};

//    std::mutex m_mutex;
    std::unique_ptr<std::future<void>> m_motorThread;
    std::atomic_bool m_motorStopRequested = {false};

    NAU7802 *m_nau7802 = nullptr;
    std::vector<LoadData> m_loadData;
//    std::atomic_int32_t m_currentReading = {0}; // current load cell reading
    std::atomic_int32_t m_zeroOffset = {0};
    int32_t m_readingLimit = 1070000; // this is approx 1 kg
    int32_t m_overloadCountLimit = 10;

    int64_t m_time = 0;
};

#endif // MOTORCONTROL_H
