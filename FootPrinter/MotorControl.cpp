#include "MotorControl.h"

#include <chrono>
#include <thread>
#include <algorithm>
#include <utility>
#include <iostream>

MotorControl::MotorControl()
{
    m_motor1Enable.off();
    m_motor1Direction.off();
    m_motor1Step.off();
    m_motor2Enable.off();
    m_motor2Direction.off();
    m_motor2Step.off();

    m_loadData.reserve(size_t(std::abs(m_stepsLimit) * 2));
}

MotorControl::~MotorControl()
{
    StopMotor(); // this is safe even if the motor is already stopped
    m_motor1Enable.off();
    m_motor1Direction.off();
    m_motor1Step.off();
    m_motor2Enable.off();
    m_motor2Direction.off();
    m_motor2Step.off();
}

void MotorControl::SetPositionToZero()
{
    m_motorThread = std::make_unique<std::future<void>>(std::async(std::launch::async, &MotorControl::SetPositionToZeroPrivate, this));
}


void MotorControl::PlungeStroke()
{
    m_motorThread = std::make_unique<std::future<void>>(std::async(std::launch::async, &MotorControl::PlungeStrokePrivate, this));
}

void MotorControl::StopMotor()
{
    // first check to see whether the motor thread is running
    if (!m_motorThread.get()) return;
    // now set the interrupt flag
    m_motorStopRequested = true;
    // and wait until the thread is completed
    m_motorThread->wait();
    m_motorThread.release();
    m_motorStopRequested = false;
}

bool MotorControl::MotorStopped()
{
    // first check to see whether the motor thread is running
    if (!m_motorThread.get()) return true;
    // then check to see whether the motor thread has finished
    if (m_motorThread->wait_for(std::chrono::seconds(0)) != std::future_status::ready) return false;
    // need to delete the thread because it has finished
    m_motorThread.release();
    m_motorStopRequested = false;
    return true;
}

void MotorControl::SetPositionToZeroPrivate()
{
    int64_t currentTotal = 0;
    int64_t currentCount = 0;
    m_motor2Step.off();
    m_motor2Direction.off(); // off is up (which is negative)
    m_motor2Enable.on();
    while (m_motorStopRequested == false)
    {
        if (m_stopSensor.read()) break;
        m_motor2Step.on();
        std::this_thread::sleep_for(std::chrono::microseconds(m_stepOnDuration));
        m_motor2Step.off();
        std::this_thread::sleep_for(std::chrono::microseconds(m_stepOffDuration));
        m_stepsOffset--;
        currentTotal += m_nau7802->threadedValue();
        currentCount++;
    }
    m_stepsOffset = 0;
    m_motor2Direction.on(); // on is down (which is positive)
    while (m_stepsOffset < m_startingOffset && m_motorStopRequested == false)
    {
        m_motor2Step.on();
        std::this_thread::sleep_for(std::chrono::microseconds(m_stepOnDuration));
        m_motor2Step.off();
        std::this_thread::sleep_for(std::chrono::microseconds(m_stepOffDuration));
        m_stepsOffset++;
        currentTotal += m_nau7802->threadedValue();
        currentCount++;
    }
    m_motor2Enable.off();
    if (currentCount == 0) m_zeroOffset = 0;
    else m_zeroOffset = int32_t(currentTotal / currentCount);
    std::cerr<< "m_zeroOffset = " << m_zeroOffset << "\n";
    m_stepsOffsetValid = true;
}

void MotorControl::PlungeStrokePrivate()
{
    if (!m_stepsOffsetValid) SetPositionToZeroPrivate();
    m_motor2Step.off();
    m_motor2Direction.on(); // on is down (which is positive)
    m_motor2Enable.on();
    int overloadCount = 0;
    while (m_motorStopRequested == false)
    {
        if (m_stopSensor.read()) break;
        if (m_stepsOffset >= m_stepsLimit)
        {
            std::cerr << "m_stepsOffset = " << m_stepsOffset << "\n";
            break;
        }
        m_motor2Step.on();
        std::this_thread::sleep_for(std::chrono::microseconds(m_stepOnDuration));
        m_motor2Step.off();
        std::this_thread::sleep_for(std::chrono::microseconds(m_stepOffDuration));
        auto t0 = std::chrono::high_resolution_clock::now();
        auto duration = t0.time_since_epoch();
        m_time = std::chrono::duration_cast<std::chrono::nanoseconds>(duration).count();
        int32_t currentReading = m_nau7802->threadedValue();
        m_loadData.push_back({m_time, m_stepsOffset, currentReading});
        if (std::abs(currentReading - m_zeroOffset) > m_readingLimit)
        {
            overloadCount++;
            if (overloadCount >= m_overloadCountLimit)
            {
                std::cerr << "currentReading - m_zeroOffset = " << currentReading - m_zeroOffset << "\n";
                break;
            }
        }
        else
        {
            overloadCount = 0;
        }
        m_stepsOffset++;
    }
    m_motor2Enable.off();
    SetPositionToZeroPrivate();
}

int32_t MotorControl::overloadCountLimit() const
{
    return m_overloadCountLimit;
}

void MotorControl::setOverloadCountLimit(const int32_t &overloadCountLimit)
{
    m_overloadCountLimit = overloadCountLimit;
}

int32_t MotorControl::zeroOffset() const
{
    return m_zeroOffset;
}

int32_t MotorControl::readingLimit() const
{
    return m_readingLimit;
}

void MotorControl::setReadingLimit(const int32_t &readingLimit)
{
    m_readingLimit = readingLimit;
}

int32_t MotorControl::startingOffset() const
{
    return m_startingOffset;
}

void MotorControl::setStartingOffset(const int32_t &startingOffset)
{
    m_startingOffset = startingOffset;
}

int32_t MotorControl::stepOffDuration() const
{
    return m_stepOffDuration;
}

void MotorControl::setStepOffDuration(const int32_t &stepOffDuration)
{
    m_stepOffDuration = stepOffDuration;
}

int32_t MotorControl::stepOnDuration() const
{
    return m_stepOnDuration;
}

void MotorControl::setStepOnDuration(const int32_t &stepOnDuration)
{
    m_stepOnDuration = stepOnDuration;
}

int32_t MotorControl::stepsLimit() const
{
    return m_stepsLimit;
}

void MotorControl::setStepsLimit(const int32_t &stepsLimit)
{
    m_stepsLimit = stepsLimit;
}

//int32_t MotorControl::currentReading() const
//{
//    return m_currentReading;
//}

void MotorControl::SetNau7802(NAU7802 *nau7802)
{
    m_nau7802 = nau7802;
}

int32_t MotorControl::position() const
{
    return m_stepsOffset;
}

std::vector<MotorControl::LoadData> *MotorControl::GetLoadData()
{
    if (MotorStopped()) return &m_loadData;
    return nullptr;
}

