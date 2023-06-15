#include "UBloxControl.h"

UBloxControl::UBloxControl()
{

}

std::string UBloxControl::ConfigureNMEA(enum UBloxIDNMEA messageID, bool enable)
{
    std::string message;
    message.reserve(64);
    // header
    message.push_back('\xB5'); // header 1
    message.push_back('\x62'); // header 2
    message.push_back(uint8_t(UbloxClass::CFG));
    message.push_back(uint8_t(UBloxIDCFG::MSG));
    message.push_back(3); // LSB little endian payload length
    message.push_back(0); // MSB little endian payload length

    // payload (3 bytes) for enable/disable a periodic message
    message.push_back(uint8_t(UbloxClass::NMEA));
    message.push_back(uint8_t(messageID));
    if (enable) message.push_back(1);
    else message.push_back(0);

    // checksum
    uint8_t CK_A = 0;
    uint8_t CK_B = 0;
    for (size_t i = 2; i < message.size(); i++)
    {
        CK_A = CK_A + message[i];
        CK_B = CK_B + CK_A;
    }
    message.push_back(CK_A);
    message.push_back(CK_B);
    return message;
}


