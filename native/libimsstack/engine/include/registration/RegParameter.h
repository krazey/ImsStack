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
#ifndef REG_PARAMETER_H_
#define REG_PARAMETER_H_

#include "AStringArray.h"
#include "Credential.h"
#include "ImsSlot.h"

#include "IRegParameter.h"
#include "RegStateTracker.h"
#include "SipAddress.h"
#include "SipSecurityHeader.h"

class ISipMessage;
class ISipClientConnection;
class ExtraHeaders;
class ImsSubscriberInfo;

class RegParameter final : public ImsSlot, public IRegParameter
{
public:
    explicit RegParameter(IN IMS_SINT32 nSlotId);
    ~RegParameter() override;

    RegParameter(IN const RegParameter&) = delete;
    RegParameter& operator=(IN const RegParameter&) = delete;

public:
    // IRegParameter interface
    inline IMS_SINT32 GetPort() const override { return m_nPort; }
    inline const SipAddress& GetTopmostRouteAddress() const override
    {
        return m_objTopmostRouteAddress;
    }
    inline void SetSecurityVerifys(IN const ImsList<SipSecurityHeader>& objSecurityVerifys) override
    {
        m_objSecurityVerifys = objSecurityVerifys;
    }

    IMS_RESULT FormHeaders(
            IN_OUT ISipClientConnection*& piScc, IN const RcPtr<RegStateTracker>& pStateTracker);
    IMS_RESULT FormRouteHeaders(
            IN_OUT ISipClientConnection*& piScc, IN const RcPtr<RegStateTracker>& pStateTracker);
    IMS_RESULT FormSecurityHeaders(IN_OUT ISipClientConnection*& piScc);
    inline Credential& GetCredential() { return m_objCredential; }
    inline IMS_SINT32 GetFlowControlOption() const { return m_nFlowControlOption; }
    inline IMS_SINT32 GetPortFlowControl() const { return m_nPortFlowControl; }
    inline const AStringArray& GetPreloadedRoutes() const { return m_objPreloadedRoutes; }
    IMS_SINT32 GetProtectedPortUc() const;
    IMS_SINT32 GetProtectedPortUs() const;
    inline const ImsList<SipSecurityHeader>& GetSecurityClients() const
    {
        return m_objSecurityClients;
    }
    const ImsList<SipSecurityHeader>& GetSecurityVerifys() const;
    inline const SipTimerValues* GetSipTimerValues() const { return m_pSipTimerValues; }
    inline IMS_SINT32 GetTransportExt() const { return m_nTransportExt; }
    inline IMS_SINT32 GetTransportExtForRegOnly() const { return m_nTransportExtForRegOnly; }
    inline IMS_BOOL IsAuthRealmLenient() const { return m_bIsAuthRealmLenient; }
    inline IMS_BOOL IsSecurityAssociationPresent() const
    {
        return (!m_objSecurityServers.IsEmpty() || !m_objSecurityVerifys.IsEmpty());
    }
    inline IMS_BOOL IsSecurityAssociationRequired() const
    {
        return (!m_objSecurityClients.IsEmpty() || !m_objNewSecurityClients.IsEmpty());
    }
    // IMS_IPSEC_UDP_ENC
    inline IMS_BOOL IsSecurityAssociationRequiredViaUdpEnc() const
    {
        return ((m_nTransportExt & Sip::TRANSPORT_EXT_IPSEC_UDP_ENC) != 0);
    }
    void RemovePreferredSecurityHeaders();
    inline void RemoveSecurityServers() { m_objSecurityServers.Clear(); }
    void Restore();
    void RestoreSecurityHeaders();
    void SetTransportExtForIpSec();
    IMS_BOOL UpdateProfile(
            IN const SipAddress& objAor, IN const AString& strSubsId = AString::ConstNull());
    IMS_BOOL UpdateSecurityHeaders(IN const ISipMessage* piSipMsg);
    void UpdateSipProfile(IN SipProfile* pProfile);

private:
    // IRegParameter interface
    IMS_BOOL AddExtraHeaders(IN const AStringArray& objHeaders) override;
    IMS_BOOL AddMessageBodyPart(IN ISipMessageBodyPart* piBodyPart) override;
    IMS_BOOL AddPreloadedRoute(IN const AString& strRoute) override;
    IMS_BOOL AddPreloadedRoute(IN const AString& strHost, IN IMS_SINT32 nPort,
            IN const AString& strScheme = AString::ConstNull()) override;
    inline IMS_BOOL AddSecurityClient(IN const SipSecurityHeader& objSecurityHeader) override
    {
        return m_objNewSecurityClients.Append(objSecurityHeader);
    }
    inline const SipSecurityHeader* GetPreferredSecurityClient() const override
    {
        return m_pPreferredSecurityClient;
    }
    inline const SipSecurityHeader* GetPreferredSecurityServer() const override
    {
        return m_pPreferredSecurityServer;
    }
    inline const ImsList<SipSecurityHeader>& GetSecurityServers() const override
    {
        return m_objSecurityServers;
    }
    void RemoveAllMessageBodyParts() override;
    inline void RemoveAllPreloadedRoutes() override { m_objPreloadedRoutes.RemoveAllElements(); }
    void RemoveExtraHeaders(IN const AStringArray& objHeaders) override;
    inline void RemoveSecurityClients() override { m_objNewSecurityClients.Clear(); }
    inline void SetAuthenticationCredentials(IN IMS_BOOL bPolicy) override
    {
        m_bPolicyForAuthenticationCredentials = bPolicy;
    }
    inline void SetFlowControlOption(IN IMS_SINT32 nOption) override
    {
        m_nFlowControlOption = nOption;
    }
    inline void SetPort(IN IMS_SINT32 nPort) override { m_nPort = nPort; }
    inline void SetPortFlowControl(IN IMS_SINT32 nPort) override { m_nPortFlowControl = nPort; }
    void SetSipTimerValues(IN const SipTimerValues& objTimerValues) override;
    void SetTransportExt(IN IMS_SINT32 nTransportExt) override;
    inline void SetTransportExtForRegOnly(IN IMS_SINT32 nTransportExt) override
    {
        m_nTransportExtForRegOnly = nTransportExt;
    }

    void ChoosePreferredSecurityClient();
    void ChoosePreferredSecurityServer();

    static const ImsSubscriberInfo* GetImsSubscriberInfo(IN IMS_SINT32 nSlotId,
            IN const SipAddress& objAor, IN const AString& strSubsId = AString::ConstNull());

private:
    // Policy for Authorization header
    IMS_BOOL m_bPolicyForAuthenticationCredentials;
    IMS_SINT32 m_nTransportExt;
    IMS_SINT32 m_nTransportExtForRegOnly;
    // Default SIP port number
    IMS_SINT32 m_nPort;
    // RFC5626_FLOW_CONTROL
    IMS_SINT32 m_nFlowControlOption;
    IMS_SINT32 m_nPortFlowControl;
    // Address of S-CSCF (Registrar)
    AString m_strServingCscf;
    // Preloaded route set (in this moment, only one pre-configured route exists)
    AStringArray m_objPreloadedRoutes;
    // Topmost Route address
    SipAddress m_objTopmostRouteAddress;

    // Security-Client headers
    ImsList<SipSecurityHeader> m_objSecurityClients;
    ImsList<SipSecurityHeader> m_objNewSecurityClients;
    ImsList<SipSecurityHeader> m_objOldSecurityClients;
    // Security-Server header
    ImsList<SipSecurityHeader> m_objSecurityServers;
    ImsList<SipSecurityHeader> m_objOldSecurityServers;
    // Security-Verify header
    ImsList<SipSecurityHeader> m_objSecurityVerifys;
    ImsList<SipSecurityHeader> m_objOldSecurityVerifys;

    // Preferred Security Client/Server
    SipSecurityHeader* m_pPreferredSecurityClient;
    SipSecurityHeader* m_pPreferredSecurityServer;

    // Extra headers from IMS registry
    ExtraHeaders* m_pExtraHeaders;
    // Message body parts which are always added in initial-REG/re-REG/de-REG
    ImsList<ISipMessageBodyPart*> m_objBodyParts;
    // Credentials if present
    Credential m_objCredential;
    IMS_BOOL m_bIsAuthRealmLenient;
    // Timer values of SIP transaction layer for this registration
    SipTimerValues* m_pSipTimerValues;
};

#endif
