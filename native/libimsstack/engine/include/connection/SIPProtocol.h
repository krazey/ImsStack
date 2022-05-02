/*
    Author
    <table>
    date      author                    description
    --------  --------------            ----------
    20091126  toastops@                 Created
    </table>

    Description

*/

#ifndef _SIP_PROTOCOL_H_
#define _SIP_PROTOCOL_H_

#include "Protocol.h"
#include "Sip.h"



class SIPProtocol
    : public Protocol
{
private:
    SIPProtocol();
    SIPProtocol(IN CONST SIPProtocol &objRHS);
public:
    virtual ~SIPProtocol();

public:
    static SIPProtocol* GetInstance();

private:
    // Protocol class
    virtual IConnection* OpenPrim(IN CONST AString &strName);
    virtual IConnection* OpenPrim(IN CONST AString &strScheme, IN CONST AString &strTarget,
            IN CONST AString &strParams);

    IConnection* CreateConnectionNotifier(IN IMS_SINT32 nScheme, IN IMS_SINT32 nPort,
            IN CONST AString &strParams, IN IMS_BOOL bSharedMode = IMS_FALSE);
};

#endif // _SIP_PROTOCOL_H_
