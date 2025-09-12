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
#ifndef FAKE_REGISTRATION_H_
#define FAKE_REGISTRATION_H_

#include "EngineActivity.h"
#include "IRegCapabilityChangeListener.h"
#include "IRegistrationEx.h"
#include "RegContact.h"
#include "RegParameter.h"
#include "RegStateTracker.h"

class IRegistrationListener;
class RegFlow;

class FakeRegistration :
        public EngineActivity,
        public IRegistrationEx,
        public IRegCapabilityChangeListener
{
public:
    FakeRegistration();
    ~FakeRegistration() override;

    FakeRegistration(IN const FakeRegistration& other) = delete;
    FakeRegistration& operator=(IN const FakeRegistration& other) = delete;

public:
    // IRegistration interface
    IMS_BOOL Equals(IN const IRegistration* piReg) const override;
    inline IMS_BOOL IsBehindNat() const override { return IMS_FALSE; }
    inline IMS_BOOL IsWithinTrustDomain() const override { return m_bIsWithinTrustDomain; }

    // IRegistrationEx interface
    inline const RegInfo* GetRegInfo() const override { return IMS_NULL; }
    inline const RegStateTracker* GetStateTracker() const override { return m_pStateTracker.Get(); }

    IMS_BOOL Create(IN IMS_UINT32 nFlowId, IN const SipAddress& objAor, IN IMS_BOOL bEmergency,
            IN const AString& strSubsId = AString::ConstNull(), IN SipProfile* pProfile = IMS_NULL);
    void Destroy();
    inline const ImsList<RegContact*>& GetAllContactsEx() const { return m_objContacts; }
    IMS_BOOL HasActiveBindings() const;
    IMS_BOOL IsAllBindingsRemoved() const;

private:
    // EngineActivity class
    IMS_BOOL DispatchMessage(IN ImsMessage& objMsg) override;

    // IRegBase interface
    inline ISipMessage* GetNextRequest() override { return IMS_NULL; }
    inline ISipMessage* GetPreviousRequest() const override { return IMS_NULL; }
    inline ISipMessage* GetPreviousResponse() const override { return IMS_NULL; }
    inline void SetSipMessageMediator(IN IMessageMediator* /*piMediator*/) override {}

    // IRegistration interface
    IMS_BOOL CreateBinding(IN const AString& strAppId, IN const AString& strServiceId) override;
    void DestroyBinding(IN const AString& strAppId, IN const AString& strServiceId) override;
    IRegContact* CreateContact(IN const IpAddress& objIpAddr, IN IMS_SINT32 nPort,
            IN IMS_SINT32 nExpiresPolicy = POLICY_EXPIRES_CONFIG,
            IN IMS_UINT32 nExpiresValue = DEFAULT_EXPIRES) override;
    void DestroyContact(IN IRegContact* piContact) override;
    void DestroyContact(IN const IpAddress& objIpAddr, IN IMS_SINT32 nPort) override;
    inline const Credential* GetCredential() const override { return IMS_NULL; }
    inline const SipAddress& GetAor() const override { return m_pStateTracker->GetAor(); }
    inline const AStringArray& GetAssociatedUris() const override
    {
        return m_pStateTracker->GetAssociatedUris();
    }
    inline const SipAddress& GetAuthorizedAor() const override
    {
        return m_pStateTracker->GetAuthorizedAor();
    }
    ImsList<IRegContact*> GetAllContacts() const override;
    IRegContact* GetContact(IN const IpAddress& objIpAddr, IN IMS_SINT32 nPort) const override;
    inline IRegContact* GetPreferredContact() const override
    {
        return const_cast<RegContact*>(m_pStateTracker->GetPreferredContact());
    }
    inline IRegParameter* GetParameter() const override { return m_pRegParam; }
    inline const IpAddress& GetPublicIpAddress() const override { return IpAddress::NONE; }
    inline const AStringArray& GetServiceRoutes() const override
    {
        return m_pStateTracker->GetServiceRoutes();
    }
    inline SipProfile* GetSipProfile() const override
    {
        return (!m_pStateTracker.IsNull()) ? m_pStateTracker->GetSipProfile() : IMS_NULL;
    }
    inline IMS_SINT32 GetState() const override { return m_nState; }
    IMS_BOOL IsBindingsUpdated() const override;
    inline IMS_BOOL IsBindingsUpdating() const override
    {
        return (GetSubState() != SUB_STATE_IDLE);
    }
    // Emergency registration w/o UICC or w/ invalid UICC
    inline IMS_BOOL IsNetworkInterworkingRequired() const override { return IMS_FALSE; }
    IMS_RESULT Register(IN IMS_SINT32 nExpires = (-1)) override;
    IMS_RESULT Deregister() override;
    // REG_RESTORATION_FOR_ACTIVE_BINDING
    void RemoveActiveBindingsForcingly() override;
    void Restore() override;
    // REG_RESTORATION_FOR_ACTIVE_BINDING
    inline IMS_RESULT RestoreActiveBindings() override { return IMS_SUCCESS; }
    inline void SetActiveBindingsRestorationUsage(IN IMS_BOOL /*bEnabled*/) override {}
    void SetAor(IN const SipAddress& objAor,
            IN const AString& strSubsId = AString::ConstNull()) override;
    inline void SetListener(IN IRegistrationListener* piListener) override
    {
        m_piListener = piListener;
    }
    void SetRefreshPolicy(IN IMS_SINT32 nPolicy, IN IMS_SINT32 nCriteriaInterval,
            IN IMS_SINT32 nValueEorLt, IN IMS_SINT32 nValueGt) override;
    void SetSipProfile(IN SipProfile* pProfile) override;
    void SetBindingStateListener(IN IRegBindingStateListener* piListener) override;
    void SetFlagForWithinTrustDomain(IN IMS_BOOL bWithinTrustDomain) override;
    inline void SetUserIdentityNotifier(IN IRegUserIdentityNotifier* piUserIdNotifier) override
    {
        m_piUserIdNotifier = piUserIdNotifier;
    }
    void SetUserInfoForContactHeader(IN const AString& strUserInfo) override;
    inline IRegSubscription* CreateSubscription(IN SipAddress* pResourceUri = IMS_NULL) override
    {
        (void)pResourceUri;
        return IMS_NULL;
    }

    // ISipConnectionNotifierErrorListener class
    void ConnectionNotifierError_NotifyError(IN ISipConnectionNotifier* piScn, IN IMS_SINT32 nCode,
            IN const AString& strMessage) override;

    // IRegistrationEx interface
    void AddObserver(IN RegObserver* pObserver) override;
    void RemoveObserver(IN RegObserver* pObserver) override;
    IMS_SINT32 AddReferenceForScnErrorListener() override;
    IMS_SINT32 RemoveReferenceForScnErrorListener() override;
    inline void NotifyCallerCapabilityChanged() override {}
    inline IMS_BOOL IsActiveBindingsRestorationEnabled() const override { return IMS_FALSE; }
    inline IMS_BOOL IsEmergencyRegistration() const override
    {
        return m_pStateTracker->IsEmergencyRegistration();
    }

    // IRegCapabilityChangeListener interface
    void RegCapabilityChange_ServiceAdded(
            IN const AString& strAppId, IN const AString& strServiceId) override;
    void RegCapabilityChange_ServiceRemoved(
            IN const AString& strAppId, IN const AString& strServiceId) override;

    void CallListener(IN IMS_SINT32 nPrevState, IN IMS_SINT32 nPrevSubState, IN IMS_SINT32 nReason);
    void ChoosePreferredContact();
    void DestroyAllContacts();
    IMS_SINT32 GetPortUc() const;
    IMS_SINT32 GetPortUs() const;
    inline IMS_SINT32 GetSubState() const { return m_nSubState; }
    void NotifyResponse(IN IMS_SINT32 nStatusCode);
    void SetState(IN IMS_SINT32 nState);
    void SetSubState(IN IMS_SINT32 nSubState);
    void StorePersistentHeaders();
    IMS_RESULT UpdateBindings(IN IMS_BOOL bIsContactWildcard = IMS_FALSE);
    void UpdateBindingState(IN IMS_SINT32 nState);
    void UpdateCSeqNumber();

    static const IMS_CHAR* StateToString(IN IMS_SINT32 nState);
    static const IMS_CHAR* SubStateToString(IN IMS_SINT32 nSubState);

private:
    /// MSG for registration
    enum
    {
        AMSG_REGISTRATION_STARTED = AMSG_USER,
        AMSG_REGISTRATION_START_FAILED,
        AMSG_REGISTRATION_UPDATED,
        AMSG_REGISTRATION_UPDATE_FAILED,
        AMSG_REGISTRATION_REMOVED,
        AMSG_REGISTRATION_TERMINATED,

        /// For authentication challenge
        AMSG_REGISTRATION_AKA_RESPONSE_RECEIVED,

        /// For fake transaction completion
        AMSG_REGISTRATION_RESPONSE_RECEIVED
    };

    /// Sub-State of registration
    enum
    {
        /// No transaction
        SUB_STATE_IDLE = 0,
        /// Initial REGISTER or re-REGISTER
        SUB_STATE_REGISTERING,
        /// Implicit re-REGISTER
        SUB_STATE_REFRESHING,
        /// de-REGISTER
        SUB_STATE_DEREGISTERING
    };

    /// Result of binding updates
    enum
    {
        BINDING_UPDATE_OK = 0x00,
        BINDING_UPDATE_NO_EXPIRES = 0x01
    };

    // State of registration binding (from IRegistration)
    IMS_SINT32 m_nState;
    // Sub-State of registration (REGISTERING or IDLE)
    IMS_SINT32 m_nSubState;

    // Reference to the registration flow (Call-ID, Command sequence number, ...)
    // Don't delete this pointer explicitly.
    RegFlow* m_pRegFlow;
    // Parameters for registration
    RegParameter* m_pRegParam;
    // Registration contact list
    ImsList<RegContact*> m_objContacts;
    // Registration State Tracker
    // IMPU & Network authorized IMPU (topmost one in P-Associated-URI)
    // Persistent information (P-Associated-URI, Service-Routes) which MUST be kept
    // while the registration is active
    RcPtr<RegStateTracker> m_pStateTracker;
    // Listener to notify the registration state
    IRegistrationListener* m_piListener;
    // Notifier to reorder the network provisioned user identities
    IRegUserIdentityNotifier* m_piUserIdNotifier;

    // Observers of reg. state
    ImsList<RegObserver*> m_objObservers;

    // For trust domain checking
    IMS_BOOL m_bIsWithinTrustDomain;

    // Reference count for ISipConnectionNotifierErrorListener
    IMS_SINT32 m_nRefCountForScnErrorListener;
};

#endif
