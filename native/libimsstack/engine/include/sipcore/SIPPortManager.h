/*
    Author
    <table>
    date      author                    description
    --------  --------------            ----------
    20151229  hwangoo.park@             Created
    </table>

    Description
    This class provides the port information for SIP TCP/TLS transport protocol.
*/

#ifndef _SIP_PORT_MANAGER_H_
#define _SIP_PORT_MANAGER_H_

#include "IPAddress.h"

class SIPPortManager
{
private:
    SIPPortManager();
    ~SIPPortManager();

public:
    void Clear();
    IMS_SINT32 GetPortC(IN CONST IPAddress& objIP) const;
    IMS_BOOL IsPortCProvisioned() const;
    void SetPortC(IN IMS_SINT32 nPortStart, IN IMS_SINT32 nPortEnd);

    static SIPPortManager* GetInstance();

private:
    IMS_SINT32 GetNextPortC() const;
    IMS_BOOL IsPortAvailable(IN CONST IPAddress& objIP, IN IMS_SINT32 nPort) const;
    IMS_SINT32 SelectNextPortC(IN CONST IPAddress& objIP) const;
    void SetNextPortC(IN IMS_SINT32 nPort) const;

private:
    // Port range as a default (not-inclusive).
    enum
    {
        CLIENT_PORT_MIN = 1024,
        CLIENT_PORT_MAX = 65535,

        CLIENT_PORT_START = 40000,
        CLIENT_PORT_END = CLIENT_PORT_MAX
    };

    // Minimum Round-Robin Gap
    // Do not re-use a source port that has been used in any of the previous 32 TCP sockets.
    enum
    {
        MIN_RR_GAP = 32
    };

    // Port range (not-inclusive)
    IMS_SINT32 nPortC_Start;
    IMS_SINT32 nPortC_End;

    // Next client port number
    mutable IMS_SINT32 nNextPortC;
};

#endif  // _SIP_PORT_MANAGER_H_
