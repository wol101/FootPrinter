#include "GPSAccess.h"
#include "pystring.h"
#include "UBloxControl.h"

#include <poll.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <string.h>
#include <chrono>
#include <thread>
#include <iostream>

using namespace std::string_literals;

GPSAccess::GPSAccess()
{
}

GPSAccess::~GPSAccess()
{
    if (m_gpsFD >= 0)
    {
        ::close(m_gpsFD);
    }
}

int GPSAccess::OpenDevice(const std::string &device)
{
    int timeout = 0;
    const int timeoutLimit = 60;
    while (true)
    {
        m_gpsFD = ::open(device.c_str(), O_RDWR);
        if (m_gpsFD >= 0) break;
        timeout++;
        std::this_thread::sleep_for(std::chrono::seconds(1));
        if (timeout > timeoutLimit)
        {
            std::cerr << "GPSAccess::OpenDevice error: " << errno << " " << strerror(errno) << "\n";
            return __LINE__;
        }
    }
    std::cerr << "GPSAccess::OpenDevice connected\n";
    return 0;
}

bool GPSAccess::DataReady()
{
    pollfd pfd;
    pfd.fd = m_gpsFD;
    pfd.events = POLLIN;
    if (poll(&pfd, 1, m_dataReadyTimeout))
    {
        if (pfd.revents & POLLIN) return true;
    }
    return false;
}

size_t GPSAccess::ReadData()
{
    m_readDataBuffer.resize(m_readDataBufferSize);
    ssize_t bytesRead = read(m_gpsFD, &m_readDataBuffer[0], m_readDataBuffer.size());
    m_readDataBuffer.resize(size_t(bytesRead));
    return m_readDataBuffer.size();
}

void GPSAccess::GetLines(std::vector<std::string> *lines)
{
    pystring::splitlines(m_readDataBuffer, *lines);
}

std::map<std::string, std::vector<std::string> > *GPSAccess::ParseNMEA()
{
    m_mapNMEA.clear();
    std::vector<std::string> lines;
    pystring::splitlines(m_readDataBuffer, lines);
    for (auto &&line : lines)
    {
        if (line.size() < 9 || line.at(0) != '$' || line.at(line.size() - 3) != '*') continue;
        std::string hex(&line[line.size() - 2], 2);
        line.resize(line.size() - 3);
        int checksum = 0;
        for (size_t i = 1; i < line.size(); i++)
        {
            checksum ^= line.at(i);
        }
        if (checksum != std::stoi(hex, nullptr, 16)) continue;
        std::vector<std::string> tokens;
        pystring::split(line, tokens, ","s);
        m_mapNMEA[tokens[0]] = std::move(tokens);
    }
    return &m_mapNMEA;
}

std::string GPSAccess::ParseTimeString()
{
    std::string result;
    auto it = m_mapNMEA.find("$GPZDA"s);
    if (it == m_mapNMEA.end()) return result;
    if (it->second.size() != 7) return result;
    if (it->second.at(1).size() != 9) return result;
    if (it->second.at(2).size() != 2) return result;
    if (it->second.at(3).size() != 2) return result;
    if (it->second.at(4).size() != 4) return result;
    result.append(it->second.at(1), 0, 2);
    result.append("-"s);
    result.append(it->second.at(1), 2, 2);
    result.append("-"s);
    result.append(it->second.at(1), 4, 5);
    result.append("_"s);
    result.append(it->second.at(2));
    result.append("-"s);
    result.append(it->second.at(3));
    result.append("-"s);
    result.append(it->second.at(4));
    m_lastValidTime = result;
    return result;
}

std::string GPSAccess::ParsePositionString()
{
    std::string result;
    auto it = m_mapNMEA.find("$GPGGA"s);
    if (it == m_mapNMEA.end()) return result;
    if (it->second.size() != 15) return result;
    result.append(it->second.at(2));
    result.append(it->second.at(3));
//    result.append("_"s);
    result.append(it->second.at(4));
    result.append(it->second.at(5));
    if (result.size() == 0 || result.find_first_not_of(" \t\n\v\f\r") == std::string::npos) return result;
    m_lastValidPosition = result;
    return result;
}

int GPSAccess::SetMessages()
{
    std::string message;
    // start by disabing all the NMEA messages
    // note that this should probably check the ACK/NACK responses and report errors
    // but currently it just assumes that the comms are reliable
    for (int i = static_cast<int>(UBloxIDNMEA::GGA); i <= static_cast<int>(UBloxIDNMEA::DTM); i++)
    {
        message = UBloxControl::ConfigureNMEA(static_cast<UBloxIDNMEA>(i), false);
        write(m_gpsFD, message.c_str(), message.size());
    }

    // then enable the two I want
    // again no error checking
    message = UBloxControl::ConfigureNMEA(UBloxIDNMEA::GGA, true); // $GPGGA (position)
    write(m_gpsFD, message.c_str(), message.size());
    message = UBloxControl::ConfigureNMEA(UBloxIDNMEA::ZDA, true); // $GPZDA (date and time)
    write(m_gpsFD, message.c_str(), message.size());

    return 0;
}

std::string GPSAccess::GetLastValidTime() const
{
    return m_lastValidTime;
}

std::string GPSAccess::GetLastValidPosition() const
{
    return m_lastValidPosition;
}


