/*
    Author
    <table>
    date      author                    description
    --------  --------------            ----------
    20090326  toastops@                 Created
    </table>

    Description

*/

#ifndef _SIP_CLIENT_TRANSPORT_H_
#define _SIP_CLIENT_TRANSPORT_H_

#include "SIPTransport.h"

class SIPClientTransport : public SIPTransport
{
public:
    explicit SIPClientTransport(IN IMS_SINT32 nSlotId);
    virtual ~SIPClientTransport();

private:
    SIPClientTransport(IN CONST SIPClientTransport& objRHS);

public:
    // MULTI_REG_SIP_PROFILE
    virtual IMS_BOOL FormViaHeader(
            IN_OUT SipMessage*& pstMessage, IN CONST SipProfile* pSIPProfile = IMS_NULL);
    // MULTI_REG_SIP_PROFILE
    virtual IMS_BOOL ReserveResource(IN CONST SipProfile* pSIPProfile = IMS_NULL);
    virtual IMS_BOOL UpdateDestinationInfo(IN SipMessage* pstMessage,
            IN IMS_BOOL bRoutingLR = IMS_TRUE, IN SipAddrSpec* pstImplicitRoute = IMS_NULL);
    virtual IMS_SINT32 ValidateViaHeader(IN SipMessage* pstMessage);

    void SetExtensionTokenForViaBranch(IN CONST AString& strToken);

protected:
    // ISIPSocketListener interface
    virtual void Socket_NotifyError(IN SIPSocket* pSocket, IN IMS_SINT32 nErrorCode);

private:
    static IMS_BOOL IsSameHostAndPort(IN SipAddrSpec* pstAddrSpec1, IN SipAddrSpec* pstAddrSpec2);

private:
    SIPSocket* pServerSocket;

    // Extension for branch parameter in the Via header
    AString strExtensionTokenForViaBranch;
};

#endif  // _SIP_CLIENT_TRANSPORT_H_
