/*
    Author
    <table>
    date      author                    description
    --------  --------------            ----------
    20090622  toastops@                 Created
    </table>

    Description
     The ISessionDescriptor is an interface to the session-level part of incoming/outgoing SDP.

    On the originating endpoint, it is most useful for the application to use the setters
    when the session is in the initiated state. The changes will then be in effect from the start
    of the session. It is not possible to use the getters since there is no incoming SDP
    at the first state. In an established session, the originating endpoint for a session update
    may read and change the SDP before applying the update.

    On the terminating endpoint, the application can read the incoming SDP, but not set
    an outgoing SDP in the ICoreService::SessionInvitationReceived() callback.
    The reason is that all SDP carrying messages have already been exchanged at the time of
    callback. If the terminating side wants to add attributes, it has to do so when
    the session is established, and trigger a session update.

    NOTE:
    The getters read from the last incoming SDP. The setters apply to all outgoing SDPs
    in the current session, beginning with the one to be sent next.
    Note that using the interface setters do not in itself trigger the IMS engine to send the SDP
    in a SIP message. If there is a point in the session lifetime where the modifications should
    cease, the application is responsible to use the setters again, and make sure the SDP is sent.

    RESERVED ATTRIBUTES :
    charset, charset:iso8895-1, group, maxprate, ice-lite, ice-mismatch, ice-options, ice-pwd,
    ice-ufrag, inactive, sendonly, recvonly, sendrecv, csup, creq, acap, tcap
*/

#ifndef _SESSION_DESCRIPTOR_H_
#define _SESSION_DESCRIPTOR_H_

#include "ISessionDescriptor.h"

class ISessionState;

class SessionDescriptor : public ISessionDescriptor
{
public:
    SessionDescriptor(IN ISessionState* piSessionState_);
    virtual ~SessionDescriptor();

private:
    SessionDescriptor(IN CONST SessionDescriptor& objRHS);
    SessionDescriptor& operator=(IN CONST SessionDescriptor& objRHS);

private:
    // ISessionDescriptor interface implementations
    virtual IMS_RESULT AddAttribute(IN CONST AString& strAttribute);
    virtual IMSList<AString> GetAttributes() const;
    virtual AString GetProtocolVersion() const;
    virtual const AString& GetSessionId() const;
    virtual AString GetSessionInfo() const;
    virtual AString GetSessionName() const;
    virtual IMS_RESULT RemoveAttribute(IN CONST AString& strAttribute);
    virtual IMS_RESULT SetSessionInfo(IN CONST AString& strInfo);
    virtual IMS_RESULT SetSessionName(IN CONST AString& strName);

    //// IMS extensions
    virtual IMS_RESULT AddAttribute(IN IMS_SINT32 nType, IN CONST AString& strAttrValue,
            IN CONST AString& strType = AString::ConstNull());
    virtual IMS_RESULT AddAttributeInt(IN IMS_SINT32 nType, IN IMS_SINT32 nAttrValue,
            IN CONST AString& strType = AString::ConstNull());
    virtual IMS_RESULT AddBandwidth(IN IMS_SINT32 nType, IN IMS_SINT32 nBandwidth,
            IN CONST AString& strType = AString::ConstNull());
    virtual const AString& GetAttribute(
            IN IMS_SINT32 nType, IN CONST AString& strType = AString::ConstNull()) const;
    virtual IMS_SINT32 GetAttributeInt(
            IN IMS_SINT32 nType, IN CONST AString& strType = AString::ConstNull()) const;
    virtual IMS_SINT32 GetBandwidth(
            IN IMS_SINT32 nType, IN CONST AString& strType = AString::ConstNull()) const;
    virtual IMS_SINT32 GetDirection() const;
    virtual const AString& GetSessionVersion() const;
    virtual const AString& GetUsername() const;
    virtual IMS_RESULT RemoveAttribute(IN CONST SdpAttribute& objAttribute);
    virtual IMS_RESULT RemoveAttribute(IN IMS_SINT32 nType,
            IN CONST AString& strAttrValue = AString::ConstNull(),
            IN CONST AString& strType = AString::ConstNull());
    virtual IMS_RESULT RemoveAllBandwidths();
    virtual IMS_RESULT SetConnectionAddress(IN CONST AString& strAddress);
    virtual IMS_RESULT SetDirection(IN IMS_SINT32 nDirection);
    virtual IMS_RESULT SetOriginAddress(IN CONST AString& strAddress);
    virtual IPAddress GetLocalAddress() const;
    virtual IPAddress GetRemoteAddress() const;
    virtual const AString& GetRemoteAddressAsString() const;

private:
    enum
    {
        MAX_RESERVED_ATTRIBUTE = 17
    };

    static const IMS_CHAR* RESERVED_ATTRIBUTE[MAX_RESERVED_ATTRIBUTE];

    ISessionState* piSessionState;
};

#endif  // _SESSION_DESCRIPTOR_H_
