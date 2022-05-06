/*
    Author
    <table>
    date      author                    description
    --------  --------------            ----------
    20101207  hwangoo.park@             Created
    </table>

    Description

*/

#ifndef _REG_STATE_TRACKER_H_
#define _REG_STATE_TRACKER_H_

#include "RCObject.h"
#include "AStringArray.h"
#include "IPAddress.h"
#include "SipAddress.h"
#include "SipSecurityHeader.h"
#include "SipProfile.h"

class RegContact;

class RegStateTracker : public RCObject
{
public:
    RegStateTracker();
    RegStateTracker(IN const RegStateTracker& objRHS);
    virtual ~RegStateTracker();

private:
    RegStateTracker& operator=(IN const RegStateTracker& objRHS);

public:
    const SipAddress& GetAOR() const;
    const AStringArray& GetAssociatedURIs() const;
    const SipAddress& GetAuthorizedAOR() const;
    const SipAddress& GetContactAddress() const;
    const SipAddress* GetContactAddressForOutgoingMessage() const;
    const IPAddress& GetIPAddress() const;
    const AStringArray& GetPathHeaders() const;
    IMS_SINT32 GetPortFlowControl() const;
    IMS_SINT32 GetPortUC() const;
    IMS_SINT32 GetPortUS() const;
    const RegContact* GetPreferredContact() const;
    // NAT_REQ_UE_PUBLIC_IP
    const IPAddress& GetPublicIPAddress() const;
    const AStringArray& GetSecurityClients() const;
    const AStringArray& GetSecurityVerifys() const;
    const AStringArray& GetServiceRoutes() const;
    // MULTI_REG_SIP_PROFILE
    SipProfile* GetSIPProfile() const;
    // MULTI_SUBS
    const AString& GetSubscriberId() const;
    // MULTI_REG_TRANSPORT
    IMS_SINT32 GetTransportExt() const;

    IMS_BOOL IsWithinTrustDomain(IN IMS_SINT32 nSlotId) const;

private:
    void SetAOR(IN CONST SipAddress& objAOR);
    void SetAssociatedURIs(IN CONST AStringArray& objAssociatedURIs);
    void SetPathHeaders(IN CONST AStringArray& objPaths);
    void SetPortFlowControl(IN IMS_SINT32 nPort);
    void SetPortUC(IN IMS_SINT32 nPort);
    void SetPortUS(IN IMS_SINT32 nPort);
    void SetPreferredContact(IN RegContact* pContact);
    // NAT_REQ_UE_PUBLIC_IP
    void SetPublicIPAddress(IN CONST IPAddress& objIP);
    void SetSecurityClients(IN CONST IMSList<SipSecurityHeader>& objClients);
    void SetSecurityVerifys(IN CONST IMSList<SipSecurityHeader>& objVerifys);
    void SetServiceRoutes(IN CONST AStringArray& objServiceRoutes);
    // MULTI_REG_SIP_PROFILE
    void SetSIPProfile(IN SipProfile* pProfile);
    // MULTI_SUBS
    void SetSubscriberId(IN CONST AString& strSubsId);
    // MULTI_REG_TRANSPORT
    void SetTransportExt(IN IMS_SINT32 nTransportExt);
    void SetUserInfoForContactHeader(IN CONST AString& strUserInfo);

private:
    friend class Registration;
    friend class FakeRegistration;

    // MULTI_SUBS : Identifier of the subscriber
    AString strSubsId;
    // IMPU : Public User Identity
    SipAddress objAOR;
    // IMPU : Network authorized Public User Identity (Topmost one in P-Associated-URI)
    SipAddress* pAuthorizedAOR;

    // Preferred Contact address
    IPAddress objIPAddress;
    // NAT_REQ_UE_PUBLIC_IP
    IPAddress objPublicIPAddress;
    SipAddress objPreferredContactAddress;
    SipAddress* pContactAddressForOutgoingMessage;
    RegContact* pPreferredContact;

    // MULTI_REG_TRANSPORT
    IMS_SINT32 nTransportExt;
    // RFC5626_FLOW_CONTROL
    IMS_SINT32 nPortFlowControl;
    // Protected client / server port (uc / us)
    IMS_SINT32 nPortUC;
    IMS_SINT32 nPortUS;

    // Persistent information which MUST be kept while the registration is active
    AStringArray objAssociatedURIs;
    AStringArray objServiceRoutes;
    AStringArray objPaths;

    // Security related headers
    AStringArray objSecurityClients;
    AStringArray objSecurityVerifys;

    // MULTI_REG_SIP_PROFILE
    RCPtr<SipProfile> pSIPProfile;
};

#endif  // _REG_STATE_TRACKER_H_
