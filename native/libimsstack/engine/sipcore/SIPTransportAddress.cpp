/*
    Author
    <table>
    date      author                    description
    --------  --------------            ----------
    20100322  hwangoo.park@             Change the class name from SIPTransportTuple
    </table>

    Description

*/

#include "ServiceMemory.h"
#include "SIPPrivate.h"
#include "SIPTransportAddress.h"

// TODO:: selects a default transport protocol
PUBLIC
SIPTransportAddress::SIPTransportAddress() :
        nProtocol(PROTOCOL_UDP),
        nPort(Sip::PORT_UNSPECIFIED),
        objIPAddress(IPAddress::NONE)
{
}

PUBLIC
SIPTransportAddress::SIPTransportAddress(IN CONST SIPTransportAddress& objRHS) :
        nProtocol(objRHS.nProtocol),
        nPort(objRHS.nPort),
        objIPAddress(objRHS.objIPAddress)
{
}

PUBLIC
SIPTransportAddress::SIPTransportAddress(
        IN IMS_SINT32 nProtocol_, IN IMS_SINT32 nPort_, IN CONST AString& strAddress_) :
        nProtocol(nProtocol_),
        nPort(nPort_)
{
    objIPAddress.Parse(strAddress_);
}

PUBLIC
SIPTransportAddress::~SIPTransportAddress() {}

/*

Remarks

*/
PUBLIC
SIPTransportAddress& SIPTransportAddress::operator=(IN CONST SIPTransportAddress& objRHS)
{
    //---------------------------------------------------------------------------------------------

    if (this != &objRHS)
    {
        nProtocol = objRHS.nProtocol;
        nPort = objRHS.nPort;
        objIPAddress = objRHS.objIPAddress;
    }

    return (*this);
}

/*

Remarks

*/
PUBLIC
IMS_BOOL SIPTransportAddress::Equals(IN CONST SIPTransportAddress& objTA) const
{
    //---------------------------------------------------------------------------------------------

    if (nProtocol != objTA.nProtocol)
        return IMS_FALSE;

    if (nPort != objTA.nPort)
        return IMS_FALSE;

    if (!objIPAddress.Equals(objTA.objIPAddress))
        return IMS_FALSE;

    return IMS_TRUE;
}
