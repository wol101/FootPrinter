#ifndef GPSACCESS_H
#define GPSACCESS_H

#include <string>
#include <vector>
#include <map>

class GPSAccess
{
public:
    GPSAccess();
    ~GPSAccess();

    int OpenDevice(const std::string &device);
    bool DataReady();
    size_t ReadData();
    void GetLines(std::vector<std::string> *lines);
    std::map<std::string, std::vector<std::string>> *ParseNMEA();
    std::string ParseTimeString();
    std::string ParsePositionString();
    int SetMessages();

    std::string GetLastValidTime() const;
    std::string GetLastValidPosition() const;

private:
    int m_gpsFD = -1;
    int m_dataReadyTimeout = 1; // wait for data to be ready (ms)
    std::string m_readDataBuffer;
    size_t m_readDataBufferSize = 10000;
    std::map<std::string, std::vector<std::string>> m_mapNMEA;
    std::string m_lastValidTime;
    std::string m_lastValidPosition;
};

#endif // GPSACCESS_H
