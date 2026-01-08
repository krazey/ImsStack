/*
 * Copyright (C) 2022 The Android Open Source Project
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *      http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */
#ifndef REG_STATE_TRACKER_H_
#define REG_STATE_TRACKER_H_

#include "AStringArray.h"
#include "IpAddress.h"
#include "RcObject.h"

#include "SipAddress.h"
#include "SipProfile.h"
#include "SipSecurityHeader.h"

class RegContact;

class RegStateTracker : public RcObject
{
public:
    RegStateTracker();
    RegStateTracker(IN const RegStateTracker& other);
    ~RegStateTracker() override;

    RegStateTracker& operator=(IN const RegStateTracker&) = delete;

public:
    inline const SipAddress& GetAor() const { return m_objAor; }
    inline const AStringArray& GetAssociatedUris() const { return m_objAssociatedUris; }
    const SipAddress& GetAuthorizedAor() const;
    const SipAddress& GetContactAddress() const;
    inline const SipAddress* GetContactAddressForOutgoingMessage() const
    {
        return m_pContactAddressForOutgoingMessage;
    }
    inline const IpAddress& GetIpAddress() const { return m_objIpAddress; }
    inline const AStringArray& GetPathHeaders() const { return m_objPaths; }
    inline IMS_SINT32 GetPortFlowControl() const { return m_nPortFlowControl; }
    inline IMS_SINT32 GetPortUc() const { return m_nPortUc; }
    inline IMS_SINT32 GetPortUs() const { return m_nPortUs; }
    inline const RegContact* GetPreferredContact() const { return m_pPreferredContact; }
    // NAT_REQ_UE_PUBLIC_IP
    inline const IpAddress& GetPublicIpAddress() const { return m_objPublicIpAddress; }
    inline const AStringArray& GetSecurityClients() const { return m_objSecurityClients; }
    inline const AStringArray& GetSecurityVerifys() const { return m_objSecurityVerifys; }
    inline const AStringArray& GetServiceRoutes() const { return m_objServiceRoutes; }
    inline SipProfile* GetSipProfile() const { return m_pSipProfile.Get(); }
    inline const AString& GetSubscriberId() const { return m_strSubsId; }
    inline IMS_SINT32 GetTransportExt() const { return m_nTransportExt; }
    inline IMS_BOOL IsEmergencyRegistration() const { return m_bEmergencyRegistration; }
    IMS_BOOL IsWithinTrustDomain(IN IMS_SINT32 nSlotId) const;

private:
    void AdjustRegistrationDedicatedParameters();
    inline void SetAor(IN const SipAddress& objAor) { m_objAor = objAor; }
    void SetAssociatedUris(IN const AStringArray& objAssociatedUris);
    inline void SetEmergencyRegistration(IN IMS_BOOL bEmergencyRegistration)
    {
        m_bEmergencyRegistration = bEmergencyRegistration;
    }
    inline void SetPathHeaders(IN const AStringArray& objPaths) { m_objPaths = objPaths; }
    inline void SetPortFlowControl(IN IMS_SINT32 nPort) { m_nPortFlowControl = nPort; }
    inline void SetPortUc(IN IMS_SINT32 nPort) { m_nPortUc = nPort; }
    inline void SetPortUs(IN IMS_SINT32 nPort) { m_nPortUs = nPort; }
    void SetPreferredContact(IN RegContact* pContact);
    // NAT_REQ_UE_PUBLIC_IP
    inline void SetPublicIpAddress(IN const IpAddress& objIpAddr)
    {
        m_objPublicIpAddress = objIpAddr;
    }
    void SetSecurityClients(IN const ImsList<SipSecurityHeader>& objClients);
    void SetSecurityVerifys(IN const ImsList<SipSecurityHeader>& objVerifys);
    inline void SetServiceRoutes(IN const AStringArray& objServiceRoutes)
    {
        m_objServiceRoutes = objServiceRoutes;
    }
    inline void SetSipProfile(IN SipProfile* pProfile) { m_pSipProfile = pProfile; }
    inline void SetSubscriberId(IN const AString& strSubsId) { m_strSubsId = strSubsId; }
    inline void SetTransportExt(IN IMS_SINT32 nTransportExt) { m_nTransportExt = nTransportExt; }
    void SetUserInfoForContactHeader(IN const AString& strUserInfo);

private:
    friend class Registration;
    friend class FakeRegistration;

    // MULTI_SUBS : Identifier of the subscriber
    AString m_strSubsId;
    // IMPU : Public User Identity
    SipAddress m_objAor;
    // IMPU : Network authorized Public User Identity (Topmost one in P-Associated-URI)
    SipAddress* m_pAuthorizedAor;
    // Preferred Contact address
    IpAddress m_objIpAddress;
    // NAT_REQ_UE_PUBLIC_IP
    IpAddress m_objPublicIpAddress;
    SipAddress m_objPreferredContactAddress;
    SipAddress* m_pContactAddressForOutgoingMessage;
    RegContact* m_pPreferredContact;
    IMS_SINT32 m_nTransportExt;
    // RFC5626_FLOW_CONTROL
    IMS_SINT32 m_nPortFlowControl;
    // Protected client / server port (uc / us)
    IMS_SINT32 m_nPortUc;
    IMS_SINT32 m_nPortUs;
    // Persistent information which MUST be kept while the registration is active
    AStringArray m_objAssociatedUris;
    AStringArray m_objServiceRoutes;
    AStringArray m_objPaths;
    // Security related headers
    AStringArray m_objSecurityClients;
    AStringArray m_objSecurityVerifys;
    RcPtr<SipProfile> m_pSipProfile;
    // Flag specifying whether this registration is for an emergency registration or not.
    IMS_BOOL m_bEmergencyRegistration;
};

#endif
