/*
    Author
    <table>
    date      author                    description
    --------  --------------            ----------
    20110528  hwangoo.park@             Created
    </table>

    Description

*/

#include "ServiceMemory.h"
#include "SipRtConfig.h"



// LogMask class for SIP run-time (or real-time) configuration
PUBLIC
SIPRTConfig::LogMask::LogMask()
    : Base()
    , nValue(LogMask::I_NONE)
{
}

PUBLIC
SIPRTConfig::LogMask::~LogMask()
{
}



// SocketOption class for SIP run-time (or real-time) configuration
PUBLIC
SIPRTConfig::SocketOption::SocketOption()
    : Base()
    , nValue(0)
    , objIP(IPAddress::NONE)
    , nPort(0)
{
}

PUBLIC
SIPRTConfig::SocketOption::SocketOption(IN CONST SIPRTConfig::SocketOption &objRHS)
    : Base(objRHS)
    , nValue(objRHS.nValue)
    , objIP(objRHS.objIP)
    , nPort(objRHS.nPort)
{
}

PUBLIC VIRTUAL
SIPRTConfig::SocketOption::~SocketOption()
{
}

/*

Remarks

*/
PUBLIC
SIPRTConfig::SocketOption& SIPRTConfig::SocketOption::operator=(
        IN CONST SIPRTConfig::SocketOption &objRHS)
{
    //---------------------------------------------------------------------------------------------

    if (this != &objRHS)
    {
        nValue = objRHS.nValue;
        objIP = objRHS.objIP;
        nPort = objRHS.nPort;
    }

    return (*this);
}

/*

Remarks

*/
PUBLIC VIRTUAL
IMS_BOOL SIPRTConfig::SocketOption::Equals(IN CONST SIPRTConfig::Base &objOther) const
{
    const SIPRTConfig::SocketOption *pSO
            = DYNAMIC_CAST(const SIPRTConfig::SocketOption*, &objOther);

    //---------------------------------------------------------------------------------------------

    return (pSO != IMS_NULL) ? (objIP.Equals(pSO->objIP) && (nPort == pSO->nPort)) : IMS_FALSE;
}

/*

Remarks

*/
PUBLIC
IMS_BOOL SIPRTConfig::SocketOption::IsGlobalOption() const
{
    //---------------------------------------------------------------------------------------------

    return ((objIP.Equals(IPAddress::NONE) || objIP.Equals(IPAddress::IPv6NONE)) && (nPort == 0));
}



// IPQoS class for IP-level QoS configuration
PUBLIC
SIPRTConfig::IPQoS::IPQoS()
    : Base()
    , nValue(0)
    , objIP(IPAddress::NONE)
    , nPort(0)
{
}

PUBLIC
SIPRTConfig::IPQoS::IPQoS(IN CONST SIPRTConfig::IPQoS &objRHS)
    : Base(objRHS)
    , nValue(objRHS.nValue)
    , objIP(objRHS.objIP)
    , nPort(objRHS.nPort)
{
}

PUBLIC VIRTUAL
SIPRTConfig::IPQoS::~IPQoS()
{
}

/*

Remarks

*/
PUBLIC
SIPRTConfig::IPQoS& SIPRTConfig::IPQoS::operator=(IN CONST SIPRTConfig::IPQoS &objRHS)
{
    //---------------------------------------------------------------------------------------------

    if (this != &objRHS)
    {
        nValue = objRHS.nValue;
        objIP = objRHS.objIP;
        nPort = objRHS.nPort;
    }

    return (*this);
}

/*

Remarks

*/
PUBLIC VIRTUAL
IMS_BOOL SIPRTConfig::IPQoS::Equals(IN CONST SIPRTConfig::Base &objOther) const
{
    const SIPRTConfig::IPQoS *pIPQoS = DYNAMIC_CAST(const SIPRTConfig::IPQoS*, &objOther);

    //---------------------------------------------------------------------------------------------

    return (pIPQoS != IMS_NULL) ?
            (objIP.Equals(pIPQoS->objIP) && (nPort == pIPQoS->nPort)) : IMS_FALSE;
}



// Header class for an additional SIP header control
PUBLIC
SIPRTConfig::Header::Header()
    : Base()
    , strName(AString::ConstNull())
    , strParameter(AString::ConstNull())
{
}

PUBLIC
SIPRTConfig::Header::Header(IN CONST SIPRTConfig::Header &objRHS)
    : Base(objRHS)
    , strName(objRHS.strName)
    , strParameter(objRHS.strParameter)
{
}

PUBLIC VIRTUAL
SIPRTConfig::Header::~Header()
{
}

/*

Remarks

*/
PUBLIC
SIPRTConfig::Header& SIPRTConfig::Header::operator=(IN CONST SIPRTConfig::Header &objRHS)
{
    //---------------------------------------------------------------------------------------------

    if (this != &objRHS)
    {
        strName = objRHS.strName;
        strParameter = objRHS.strParameter;
    }

    return (*this);
}

/*

Remarks

*/
PUBLIC VIRTUAL
IMS_BOOL SIPRTConfig::Header::Equals(IN CONST SIPRTConfig::Base &objOther) const
{
    const SIPRTConfig::Header *pHeader = DYNAMIC_CAST(const SIPRTConfig::Header*, &objOther);

    //---------------------------------------------------------------------------------------------

    if (pHeader == IMS_NULL)
    {
        return IMS_FALSE;
    }

    return strName.EqualsIgnoreCase(pHeader->strName);
}

// IPSecSA class to do the SA's validity check
PUBLIC
SIPRTConfig::IPSecSA::IPSecSA()
    : Base()
    , nPortPC(0)
    , nPortPS(0)
    , objIPP(IPAddress::NONE)
    , nPortUC(0)
    , nPortUS(0)
    , objIPU(IPAddress::NONE)
{
}

PUBLIC
SIPRTConfig::IPSecSA::IPSecSA(IN CONST SIPRTConfig::IPSecSA &objRHS)
    : Base(objRHS)
    , nPortPC(objRHS.nPortPC)
    , nPortPS(objRHS.nPortPS)
    , objIPP(objRHS.objIPP)
    , nPortUC(objRHS.nPortUC)
    , nPortUS(objRHS.nPortUS)
    , objIPU(objRHS.objIPU)
{
}

PUBLIC VIRTUAL
SIPRTConfig::IPSecSA::~IPSecSA()
{
}

PUBLIC
SIPRTConfig::IPSecSA& SIPRTConfig::IPSecSA::operator=(IN CONST SIPRTConfig::IPSecSA &objRHS)
{
    //---------------------------------------------------------------------------------------------

    if (this != &objRHS)
    {
        nPortPC = objRHS.nPortPC;
        nPortPS = objRHS.nPortPS;
        objIPP = objRHS.objIPP;
        nPortUC = objRHS.nPortUC;
        nPortUS = objRHS.nPortUS;
        objIPU = objRHS.objIPU;
    }

    return (*this);
}

PUBLIC VIRTUAL
IMS_BOOL SIPRTConfig::IPSecSA::Equals(IN CONST SIPRTConfig::Base &objOther) const
{
    const SIPRTConfig::IPSecSA *pIPSecSA = DYNAMIC_CAST(const SIPRTConfig::IPSecSA*, &objOther);

    //---------------------------------------------------------------------------------------------

    if (pIPSecSA == IMS_NULL)
    {
        return IMS_FALSE;
    }

    return (nPortPC == pIPSecSA->nPortPC)
            && (nPortPS == pIPSecSA->nPortPS)
            && objIPP.Equals(pIPSecSA->objIPP)
            && (nPortUC == pIPSecSA->nPortUC)
            && (nPortUS == pIPSecSA->nPortUS)
            && objIPU.Equals(pIPSecSA->objIPU);
}

PUBLIC
IMS_BOOL SIPRTConfig::IPSecSA::IsEmpty() const
{
    //---------------------------------------------------------------------------------------------

    return (nPortPC == 0) && (nPortPS == 0) && (nPortUC == 0) && (nPortUS == 0);
}

// TCPPortRange class to provision TCP port range
PUBLIC
SIPRTConfig::TCPPortRange::TCPPortRange()
    : Base()
    , nPortStart(0)
    , nPortEnd(0)
{
}

PUBLIC
SIPRTConfig::TCPPortRange::TCPPortRange(IN CONST SIPRTConfig::TCPPortRange &objRHS)
    : Base(objRHS)
    , nPortStart(objRHS.nPortStart)
    , nPortEnd(objRHS.nPortEnd)
{
}

PUBLIC VIRTUAL
SIPRTConfig::TCPPortRange::~TCPPortRange()
{
}

PUBLIC
SIPRTConfig::TCPPortRange& SIPRTConfig::TCPPortRange::operator=(
        IN CONST SIPRTConfig::TCPPortRange &objRHS)
{
    //---------------------------------------------------------------------------------------------

    if (this != &objRHS)
    {
        nPortStart = objRHS.nPortStart;
        nPortEnd = objRHS.nPortEnd;
    }

    return (*this);
}

PUBLIC VIRTUAL
IMS_BOOL SIPRTConfig::TCPPortRange::Equals(IN CONST SIPRTConfig::Base &objOther) const
{
    const SIPRTConfig::TCPPortRange *pPortRange
            = DYNAMIC_CAST(const SIPRTConfig::TCPPortRange*, &objOther);

    //---------------------------------------------------------------------------------------------

    if (pPortRange == IMS_NULL)
    {
        return IMS_FALSE;
    }

    return (nPortStart == pPortRange->nPortStart)
            && (nPortEnd == pPortRange->nPortEnd);
}

// RegContactAddress class to provision RegContact's information
PUBLIC
SIPRTConfig::RegContactAddress::RegContactAddress()
    : Base()
    , strCallId(AString::ConstNull())
    , objUri(SIPAddress::ConstNull())
{
}

PUBLIC
SIPRTConfig::RegContactAddress::RegContactAddress(IN CONST SIPRTConfig::RegContactAddress &objRHS)
    : Base(objRHS)
    , strCallId(objRHS.strCallId)
    , objUri(objRHS.objUri)
{
}

PUBLIC VIRTUAL
SIPRTConfig::RegContactAddress::~RegContactAddress()
{
}

PUBLIC
SIPRTConfig::RegContactAddress& SIPRTConfig::RegContactAddress::operator=(
        IN CONST SIPRTConfig::RegContactAddress &objRHS)
{
    //---------------------------------------------------------------------------------------------

    if (this != &objRHS)
    {
        strCallId = objRHS.strCallId;
        objUri = objRHS.objUri;
    }

    return (*this);
}

PUBLIC VIRTUAL
IMS_BOOL SIPRTConfig::RegContactAddress::Equals(IN CONST SIPRTConfig::Base &objOther) const
{
    const SIPRTConfig::RegContactAddress *pRegContactA
            = DYNAMIC_CAST(const SIPRTConfig::RegContactAddress*, &objOther);

    //---------------------------------------------------------------------------------------------

    if (pRegContactA == IMS_NULL)
    {
        return IMS_FALSE;
    }

    return strCallId.Equals(pRegContactA->strCallId);
}
