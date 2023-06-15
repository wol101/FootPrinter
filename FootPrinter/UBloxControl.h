#ifndef UBLOXCONTROL_H
#define UBLOXCONTROL_H

#include <string>

enum class UbloxClass
{
    NAV = 0x01,
    RXM = 0x02,
    INF = 0x04,
    ACK = 0x05,
    CFG = 0x06,
    UPD = 0x09,
    MON = 0x0A,
    AID = 0x0B,
    TIM = 0x0D,
    NMEA = 0xF0,
    PUBX = 0xF1
};

enum class UbloxIDNav
{
    POSECEF = 0x01,
    POSLLH = 0x02,
    POSUTM = 0x08,
    DOP = 0x04,
    STATUS = 0x03,
    SOL = 0x06,
    VELECEF = 0x11,
    VELNED = 0x12,
    TIMEGPS = 0x20,
    TIMEUTC = 0x21,
    CLOCK = 0x22,
    SVINFO = 0x30,
    DGPS = 0x31,
    SBAS = 0x32,
    EKFSTATUS =0x40
};
enum class UBloxIDRXM
{
    RAW = 0x10,
    SVSI = 0x20,
    SFRB = 0x11,
    ALM = 0x30,
    EPH = 0x31,
    POSREQ = 0x40
};
enum class UBloxIDINF
{
    ERROR = 0x00,
    WARNING = 0x01,
    NOTICE = 0x02,
    TEST = 0x03,
    DEBUG = 0x04,
    USER = 0x07
};
enum class UBloxIDACK
{
    ACK = 0x01,
    NAK = 0x00
};
enum class UBloxIDCFG
{
    PRT = 0x00,
    USB = 0x1B,
    MSG = 0x01,
    NMEA = 0x17,
    RATE = 0x08,
    CFG = 0x09,
    TP = 0x07,
    NAV2 = 0x1A,
    DAT = 0x06,
    INF = 0x02,
    RST = 0x04,
    RXM = 0x11,
    ANT = 0x13,
    FXN = 0x0E,
    SBAS = 0x16,
    LIC = 0x80,
    TM = 0x10,
    TM2 = 0x19,
    TMODE = 0x1D,
    EKF = 0x12
};
enum class UBloxIDUPD
{
    DOWNL = 0x01,
    UPLOAD = 0x02,
    EXEC = 0x03,
    MEMCPY = 0x04
};
enum class UBloxIDMON
{
    SCHD = 0x01,
    IO = 0x02,
    MSGPP = 0x06,
    RXBUF = 0x07,
    TXBUF = 0x08,
    HW = 0x09,
    IPC = 0x03,
    USB = 0x0A,
    VER = 0x04,
    EXCEPT = 0x05
};
enum class UBloxIDAID
{
    REQ = 0x00,
    DATA = 0x10,
    INI = 0x01,
    HUI = 0x02,
    ALM = 0x30,
    EPH = 0x31
};
enum class UBloxIDTIM
{
    TM = 0x02,
    TM2 = 0x03,
    TP = 0x01,
    SVIN = 0x04
};
enum class UBloxIDNMEA
{
    GGA = 0x00,
    GLL = 0x01,
    GSA = 0x02,
    GSV = 0x03,
    RMC = 0x04,
    VTG = 0x05,
    GRS = 0x06,
    GST = 0x07,
    ZDA = 0x08,
    GBS = 0x09,
    DTM = 0x0A
};

class UBloxControl
{
public:
    UBloxControl();

    static std::string ConfigureNMEA(enum UBloxIDNMEA messageID, bool enable);

};

#endif // UBLOXCONTROL_H
