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
#ifndef REG_CONTACT_H_
#define REG_CONTACT_H_

#include "AStringBuffer.h"
#include "ImsSlot.h"

#include "IRegContact.h"
#include "util/CallerCapability.h"

class IRegCapabilityChangeListener;
class SipProfile;

class RegContact final : public ImsSlot, public IRegContact
{
public:
    RegContact(IN IMS_SINT32 nSlotId, IN const IpAddress& objIpAddr, IN IMS_SINT32 nPort,
            IN IMS_SINT32 nUserInfoPart, IN IRegCapabilityChangeListener* piListener,
            IN IMS_SINT32 nRegId = (-1));
    ~RegContact() override;

    RegContact(IN const RegContact&) = delete;
    RegContact& operator=(IN const RegContact&) = delete;

public:
    // IRegContact interface
    inline const SipAddress& GetContactAddress() const override { return m_objContactAddress; }
    IMS_UINT32 GetExpires() const override;
    inline const IpAddress& GetIpAddress() const override { return m_objIpAddr; }
    inline IMS_SINT32 GetPort() const override { return m_objContactAddress.GetPort(); }
    inline const ImsList<SipParameter*>& GetHeaderParameters() const override
    {
        return m_objHeaderParams;
    }
    inline const SipParameter* GetInstanceParameter() const override
    {
        return m_pInstanceParameter;
    }
    inline const SipParameter* GetRegIdParameter() const override { return m_pRegIdParameter; }

    inline const SipAddress* GetPublicGruu() const override { return m_pPubGruu; }
    inline const SipAddress* GetTemporaryGruu() const override { return m_pTempGruu; }
    inline const ImsList<SipAddress*>& GetTemporaryGruus() const override { return m_objTempGruus; }

    IMS_BOOL IsActiveBinding() const override;
    inline IMS_BOOL IsEmpty() const override { return m_objCallerCapabilities.IsEmpty(); }

    void DestroyGruu();

    inline IMS_UINT32 GetInitialExpires() const { return m_nInitialExpires; }
    const AString& GetPreference() const;
    inline IMS_SINT32 GetState() const { return m_nState; }
    inline IMS_BOOL IsBindingsUpdated() const { return m_bBindingsUpdateTracker; }
    inline IMS_BOOL IsExpirationValueSpecified() const
    {
        return (m_nInitialExpires != EXPIRES_NOT_SPECIFIED);
    }
    void Restore();
    void SetAor(IN const SipAddress& objAor);
    inline void SetExpires(IN IMS_UINT32 nExpires) { m_nInitialExpires = nExpires; }
    // IMS_IPSEC_UDP_ENC
    inline void SetHostInfo(IN const IpAddress& objIpAddr)
    {
        m_objContactAddress.SetHost(objIpAddr.ToString());
    }
    void SetTerminated();
    AString ToString() const;
    AString ToStringWithExpires() const;
    // For fake registration
    IMS_SINT32 UpdateParameter(IN IMS_SINT32 nExpiresValue);
    IMS_SINT32 UpdateParameter(
            IN const ImsList<ISipHeader*>& objContactHeaders, IN IMS_SINT32 nExpiresValue);

private:
    // IRegContact interface
    IMS_BOOL AddHeaderParameter(
            IN const AString& strName, IN const AString& strValue = AString::ConstNull()) override;
    IMS_BOOL AddUriParameter(
            IN const AString& strName, IN const AString& strValue = AString::ConstNull()) override;
    void RemoveAllHeaderParameters() override;
    void RemoveHeaderParameter(
            IN const AString& strName, IN const AString& strValue = AString::ConstNull()) override;
    void RemoveUriParameter(
            IN const AString& strName, IN const AString& strValue = AString::ConstNull()) override;
    inline void SetDisplayName(IN const AString& strDisplayName) override
    {
        m_objContactAddress.SetDisplayName(strDisplayName);
    }
    inline void SetListener(IN IRegContactListener* /*piListener*/) override {}
    inline void SetPolicyForCallerCapability(IN IMS_BOOL bCapsByApp) override
    {
        m_bAllCapabilitiesByConfig = (bCapsByApp) ? IMS_FALSE : IMS_TRUE;
    }
    inline void SetPort(IN IMS_SINT32 nPort) override { m_objContactAddress.SetPort(nPort); }
    void SetUserInfo(IN IMS_SINT32 nUserInfoPart) override;
    IMS_BOOL AddExtraCapability(IN const AString& strName, IN const AString& strValue) override;
    void RemoveExtraCapability(IN const AString& strName, IN const AString& strValue) override;
    IMS_BOOL AddService(IN const AString& strAppId, IN const AString& strServiceId) override;
    void RemoveService(IN const AString& strAppId, IN const AString& strServiceId) override;
    IMS_BOOL IsServiceRegistered(
            IN const AString& strAppId, IN const AString& strServiceId) const override;
    IMS_BOOL IsFeatureRegistered(IN const AString& strFtName,
            IN const AString& strFtValue = AString::ConstNull()) const override;
    void RecalculateCallerCapabilities() override;

    void FormContact(IN IMS_BOOL bExpiresRequired, OUT AStringBuffer& objSb) const;
    IMS_BOOL AddCallerCapability(IN const CallerCapability* pCc);
    IMS_BOOL RemoveCallerCapability(IN const CallerCapability* pCc);
    IMS_BOOL RegisterServiceCapability(IN const CallerCapability* pCc);
    void UnregisterServiceCapability(IN const CallerCapability* pCc);
    void SetState(IN IMS_SINT32 nState);
    void UpdateGruu(IN const ISipHeader* piHeader);
    void UpdateRegisteredCapabilities(IN const ISipHeader* piHeader);
    void SetUserInfoPart();

    static const IMS_CHAR* StateToString(IN IMS_SINT32 nState);

public:
    /// State of the contact
    enum
    {
        STATE_CREATED,
        STATE_ACTIVE,
        STATE_TERMINATED
    };

    /// Result of parameter updates
    enum
    {
        UPDATE_OK = 0x00,
        UPDATE_NO_EXPIRES = 0x01
    };

    enum
    {
        EXPIRES_NOT_SPECIFIED = (0xFFFFFFFF)
    };

private:
    // State of the contact
    IMS_SINT32 m_nState;
    SipAddress* m_pAor;
    // URI for Contact header
    IpAddress m_objIpAddr;
    IMS_SINT32 m_nUserInfoPart;
    SipAddress m_objContactAddress;

    // Header parameter: +sip.instance
    SipParameter* m_pInstanceParameter;
    // Header parameter: reg-id
    SipParameter* m_pRegIdParameter;

    SipAddress* m_pPubGruu;
    SipAddress* m_pTempGruu;
    ImsList<SipAddress*> m_objTempGruus;

    // All the caller capabilities for this contact
    IMS_BOOL m_bBindingsUpdateTracker;
    // Capability for Contact header
    IMS_BOOL m_bAllCapabilitiesByConfig;
    CallerCapability* m_pAllCapabilities;
    CallerCapability* m_pExtraCapabilities;
    // Caller capability for each service
    ImsList<CallerCapability*> m_objCallerCapabilities;
    IRegCapabilityChangeListener* m_piCapabilityChangeListener;
    // Registered capabilities: composed from 200OK-REGISTER
    CallerCapability* m_pRegisteredCapabilities;

    // Expiration time for this contact
    // 4 origin expires & network provisioned expires
    IMS_UINT32 m_nInitialExpires;
    IMS_UINT32 m_nNetworkProvisionedExpires;

    // Header parameters
    ImsList<SipParameter*> m_objHeaderParams;
};

#endif
