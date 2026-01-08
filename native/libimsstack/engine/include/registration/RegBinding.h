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
#ifndef REG_BINDING_H_
#define REG_BINDING_H_

#include "IRegBinding.h"
#include "RegObserver.h"

class CallerCapability;
class IRegContact;
class IRegistrationEx;
class ISipConnectionNotifier;

class RegBinding : public RegObserver, public IRegBinding
{
public:
    RegBinding();

protected:
    ~RegBinding() override;

    RegBinding(IN const RegBinding&) = delete;
    RegBinding& operator=(IN const RegBinding&) = delete;

public:
    IMS_BOOL Create(IN IRegistrationEx* piRegEx);
    void Destroy();
    inline IMS_BOOL IsSameContact(IN const IRegContact* piContact) const
    {
        return (m_piContact != piContact) ? IMS_FALSE : IMS_TRUE;
    }
    inline IMS_BOOL IsSameRegistration(IN const IRegistrationEx* piRegEx) const
    {
        return (m_piRegEx != piRegEx) ? IMS_FALSE : IMS_TRUE;
    }
    void QueryCapability(OUT CallerCapability*& pCapability) const;
    void QueryRegistrationHeaders(OUT AStringArray& objHeaders) const;
    void UpdateContact(IN IRegContact* piContact);

protected:
    // RegObserver class
    void Update(IN IMS_SINT32 nWhat) override;

    // IRegBinding class
    const AStringArray& GetAssociatedUris() const override;
    const SipAddress& GetAuthorizedAor() const override;
    const SipAddress& GetContactAddress() const override;
    const SipAddress* GetContactAddressForOutgoingMessage() const override;
    const IpAddress& GetIpAddress() const override;
    const AStringArray& GetPathHeaders() const override;
    IMS_SINT32 GetPortFlowControl() const override;
    IMS_SINT32 GetPortUc() const override;
    IMS_SINT32 GetPortUs() const override;
    const IRegInfo* GetRegInfo() const override;
    const AStringArray& GetSecurityClients() const override;
    const AStringArray& GetSecurityVerifys() const override;
    const AStringArray& GetServiceRoutes() const override;
    SipProfile* GetSipProfile() const override;
    inline IMS_SINT32 GetState() const override { return m_nState; }
    const AString& GetSubscriberId() const override;
    IMS_SINT32 GetTransportExt() const override;
    const SipParameter* GetInstanceParameter() const override;

    const SipAddress* GetPublicGruu() const override;
    const SipAddress* GetTemporaryGruu() const override;
    const ImsList<SipAddress*>& GetTemporaryGruus() const override;

    IMS_BOOL IsBehindNat() const override;
    IMS_BOOL IsWithinTrustDomain() const override;
    IMS_BOOL IsEmergencyRegistration() const override;
    void NotifyCallerCapabilityChanged() override;
    void SetListener(IN IRegBindingListener* piListener) override;

private:
    void CreateSipConnectionNotifier();
    void DestroySipConnectionNotifier();
    IMS_BOOL IsBindingActive() const;
    // REG_RESTORATION_FOR_ACTIVE_BINDING
    void RestoreTransportResourceForClientInitiatedConnection();
    void RestoreTransportResourceForServerConnection(IN IMS_BOOL bNotifyError = IMS_TRUE);
    void SetState(IN IMS_SINT32 nState);

    static const IMS_CHAR* StateToString(IN IMS_SINT32 nState);

private:
    IRegistrationEx* m_piRegEx;
    IRegContact* m_piContact;

    IMS_SINT32 m_nState;
    ISipConnectionNotifier* m_piScn;

    IRegBindingListener* m_piListener;

    // Flag to indicate that the reg-state is transited from ACTIVE to TERMINATED
    // since the registration procedure is failed by the txn timeout or failure response.
    IMS_BOOL m_bDeregistrationFailed;
};

#endif
