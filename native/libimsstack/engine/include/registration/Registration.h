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
#ifndef REGISTRATION_H_
#define REGISTRATION_H_

#include "IDigestAkaListener.h"
#include "ITimer.h"

#include "Credential.h"
#include "EngineActivity.h"
#include "IRegCapabilityChangeListener.h"
#include "IRegInfoListener.h"
#include "IRegistrationEx.h"
#include "ISipClientConnectionListener.h"
#include "ISipErrorListener.h"
#include "RegContact.h"
#include "RegParameter.h"
#include "RegStateTracker.h"
#include "base/IMessageMediator.h"
#include "util/IRefreshable.h"

class IDigestAka;
class IRegistrationListener;
class ISipGenericChallenge;
class RegFlow;
class RegInfo;
class RegRefreshHelper;
class SubscriberConfig;

class Registration :
        public EngineActivity,
        public ISipClientConnectionListener,
        public ISipErrorListener,
        public IRegistrationEx,
        public IRefreshable,
        public IRegCapabilityChangeListener,
        public IRegInfoListener,
        public IDigestAkaListener
{
public:
    Registration();
    ~Registration() override;

    Registration(IN const Registration&) = delete;
    Registration& operator=(IN const Registration&) = delete;

public:
    // IRegistration interface
    IMS_BOOL Equals(IN const IRegistration* piReg) const override;
    inline IMS_BOOL IsBehindNat() const override { return m_bIsBehindNat; }
    inline IMS_BOOL IsWithinTrustDomain() const override
    {
        return m_bIsWithinTrustDomain && m_pStateTracker->IsWithinTrustDomain(GetSlotId());
    }

    // IRegistrationEx interface
    const RegInfo* GetRegInfo() const override;
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

    // ISipClientConnectionListener interface
    void ClientConnection_NotifyResponse(IN ISipClientConnection* piScc,
            IN ISipClientConnection* piForkedScc = IMS_NULL) override;

    // ISipErrorListener interface
    void Error_NotifyError(
            IN ISipConnection* piSc, IN IMS_SINT32 nCode, IN const AString& strMessage) override;

    // IRegBase interface
    ISipMessage* GetNextRequest() override;
    inline ISipMessage* GetPreviousRequest() const override { return m_piPreviousRequest; }
    inline ISipMessage* GetPreviousResponse() const override { return m_piPreviousResponse; }
    void SetSipMessageMediator(IN IMessageMediator* piMediator) override;

    // IRegistration interface
    IMS_BOOL CreateBinding(IN const AString& strAppId, IN const AString& strServiceId) override;
    void DestroyBinding(IN const AString& strAppId, IN const AString& strServiceId) override;
    IRegContact* CreateContact(IN const IpAddress& objIpAddr, IN IMS_SINT32 nPort,
            IN IMS_SINT32 nExpiresPolicy = POLICY_EXPIRES_CONFIG,
            IN IMS_UINT32 nExpiresValue = DEFAULT_EXPIRES) override;
    void DestroyContact(IN IRegContact* piContact) override;
    void DestroyContact(IN const IpAddress& objIpAddr, IN IMS_SINT32 nPort) override;
    inline const Credential* GetCredential() const override { return &m_objActiveCredential; }
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
    inline const IpAddress& GetPublicIpAddress() const override
    {
        return m_pStateTracker->GetPublicIpAddress();
    }
    inline const AStringArray& GetServiceRoutes() const override
    {
        return m_pStateTracker->GetServiceRoutes();
    }
    inline SipProfile* GetSipProfile() const override { return m_pStateTracker->GetSipProfile(); }
    inline IMS_SINT32 GetState() const override { return m_nState; }
    IMS_BOOL IsBindingsUpdated() const override;
    inline IMS_BOOL IsBindingsUpdating() const override
    {
        return (GetSubState() != SUB_STATE_IDLE);
    }
    // Emergency / normal registration
    inline IMS_BOOL IsNetworkInterworkingRequired() const override { return IMS_TRUE; }
    IMS_RESULT Register(IN IMS_SINT32 nExpires = (-1)) override;
    IMS_RESULT Deregister() override;
    // REG_RESTORATION_FOR_ACTIVE_BINDING
    void RemoveActiveBindingsForcingly() override;
    void Restore() override;
    // REG_RESTORATION_FOR_ACTIVE_BINDING
    IMS_RESULT RestoreActiveBindings() override;
    inline void SetActiveBindingsRestorationUsage(IN IMS_BOOL bEnabled) override
    {
        m_bActiveBindingsRestorationEnabled = bEnabled;
    }
    void SetAor(IN const SipAddress& objAor,
            IN const AString& strSubsId = AString::ConstNull()) override;
    inline void SetListener(IN IRegistrationListener* piListener) override
    {
        m_piListener = piListener;
    }
    void SetRefreshPolicy(IN IMS_SINT32 nPolicy, IN IMS_SINT32 nCriteriaInterval,
            IN IMS_SINT32 nValueEorLt, IN IMS_SINT32 nValueGt) override;
    void SetSipProfile(IN SipProfile* pProfile) override;
    inline void SetBindingStateListener(IN IRegBindingStateListener* piListener) override
    {
        m_piBindingStateListener = piListener;
    }
    void SetFlagForWithinTrustDomain(IN IMS_BOOL bWithinTrustDomain) override;
    inline void SetUserIdentityNotifier(IN IRegUserIdentityNotifier* piUserIdNotifier) override
    {
        m_piUserIdNotifier = piUserIdNotifier;
    }
    void SetUserInfoForContactHeader(IN const AString& strUserInfo) override;
    IRegSubscription* CreateSubscription(IN SipAddress* pResourceUri = IMS_NULL) override;

    // ISipConnectionNotifierErrorListener class
    void ConnectionNotifierError_NotifyError(IN ISipConnectionNotifier* piScn, IN IMS_SINT32 nCode,
            IN const AString& strMessage) override;

    // IRegistrationEx interface
    void AddObserver(IN RegObserver* pObserver) override;
    void RemoveObserver(IN RegObserver* pObserver) override;
    IMS_SINT32 AddReferenceForScnErrorListener() override;
    IMS_SINT32 RemoveReferenceForScnErrorListener() override;
    void NotifyCallerCapabilityChanged() override;
    inline IMS_BOOL IsActiveBindingsRestorationEnabled() const override
    {
        return m_bActiveBindingsRestorationEnabled;
    }
    inline IMS_BOOL IsEmergencyRegistration() const override
    {
        return m_pStateTracker->IsEmergencyRegistration();
    }

    // IRefreshable interface
    void Refreshable_RefreshCompleted(
            IN ISipClientConnection* piScc, IN IMS_SINT32 nCode = 0) override;
    IMS_BOOL Refreshable_RefreshStarted() override;
    void Refreshable_RefreshTerminated() override;

    // IRegCapabilityChangeListener interface
    void RegCapabilityChange_ServiceAdded(
            IN const AString& strAppId, IN const AString& strServiceId) override;
    void RegCapabilityChange_ServiceRemoved(
            IN const AString& strAppId, IN const AString& strServiceId) override;

    // IRegInfoListener interface
    void RegInfo_Updated(IN IMS_BOOL bStale = IMS_FALSE) override;
    inline void RegInfo_UpdateFailed() override {}

    // IDigestAkaListener interface
    void DigestAka_OnResponse(IN const ByteArray& objRes,
            IN const ByteArray& objIk = ByteArray::ConstNull(),
            IN const ByteArray& objCk = ByteArray::ConstNull()) override;
    void DigestAka_OnAutsFailed(IN const ByteArray& objAuts) override;
    void DigestAka_OnMacFailed() override;

    IMS_RESULT AdjustMessage(IN_OUT ISipMessage* piSipMsg,
            IN IMS_SINT32 nMessage = IMessageMediator::MESSAGE_NORMAL);
    void CallListener(IN IMS_SINT32 nPrevState, IN IMS_SINT32 nPrevSubState, IN IMS_SINT32 nReason);
    void CheckUaLocation(IN ISipMessage* piSipMsg);
    void ChoosePreferredContact();
    void ClearNextRequest();
    IDigestAka* CreateDigestAka(IN const SubscriberConfig* pSubsConfig);
    IMS_BOOL CreateSa(IN const Credential& objCredential, IN const ImsSaKey& objSaKey);
    void DestroyAllContacts();
    IMS_SINT32 GetPortUc() const;
    IMS_SINT32 GetPortUs() const;
    const SubscriberConfig* GetSubsConfig() const;
    inline IMS_SINT32 GetSubState() const { return m_nSubState; }
    IMS_BOOL IsAkaSupported(IN const SubscriberConfig* pSubsConfig) const;
    IMS_BOOL IsFlowControlRequired() const;
    IMS_RESULT ReformContactHeader(IN_OUT ISipMessage* piSipMsg);
    IMS_BOOL RegisterOnImplicitRefresh();
    IMS_BOOL RespondToChallenge(IN ISipClientConnection* piScc);
    IMS_BOOL RespondToPendingChallenge(IN const Credential& objCredential);
    void RestoreSecurityHeaders();
    // IMS_AUTH_NONCE_REUSE {
    void SetAuthenticationChallenge(IN ISipGenericChallenge* piChallenge);
    // }
    IMS_RESULT SetContactNExpiresHeader(
            IN_OUT ISipMessage* piSipMsg, IN IMS_SINT32 nExpires = (-1));
    IMS_RESULT SetExpiresHeader(IN_OUT ISipMessage* piSipMsg, IN IMS_SINT32 nExpires = (-1));
    IMS_RESULT SetHeaders(IN ISipClientConnection* piScc);
    // IMS_AUTH_NONCE_REUSE {
    void SetNextAuthenticationInfo(IN_OUT ISipClientConnection*& piScc);
    // SIP_DIGEST_AUTH_NONCE_REUSE
    void SetNextNonce(IN ISipMessage* piSipMsg);
    // }
    void SetOngoingConnection(IN ISipClientConnection* piScc);
    void SetPreviousRequest(IN ISipMessage* piSipMsg);
    void SetPreviousResponse(IN ISipMessage* piSipMsg);
    void SetState(IN IMS_SINT32 nState);
    void SetSubState(IN IMS_SINT32 nSubState);
    void StorePersistentHeaders(IN const ISipMessage* piSipMsg);
    IMS_RESULT UpdateBindings(IN const ISipMessage* piSipMsg);
    void UpdateBindingState(IN IMS_SINT32 nState);
    // IMS_IPSEC_UDP_ENC
    void UpdateHostInfoInAllContacts();
    void UpdateCSeqNumber(IN const ISipMessage* piSipMsg);
    void UpdateProtectedServerPortForContact(IN_OUT ISipMessage* piSipMsg);
    void UpdateRefreshTimer();

    // Utility methods
    static ISipClientConnection* CreateConnection(IN Registration* pReg);
    static void DestroyAllHeaders(IN_OUT ImsList<ISipHeader*>& objHeaders);

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

        /// For binding state (ex. caller capability) change notification
        AMSG_REGISTRATION_BINDING_STATE_CHANGED
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

    /// Type of binding state changes
    enum
    {
        BINDING_STATE_CALLER_CAPABILITY
    };

    // Mark the flag if Registration object is destroyed
    IMS_BOOL m_bDestroyed;

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
    // Listener to monitor the changes of the service's state
    IRegBindingStateListener* m_piBindingStateListener;
    // Notifier to reorder the network provisioned user identities
    IRegUserIdentityNotifier* m_piUserIdNotifier;

    // Refresh helper
    RegRefreshHelper* m_pRefreshHelper;

    // Digest AKA interface
    IDigestAka* m_piDigestAka;
    ImsSaKey m_objActiveSaKey;
    Credential m_objActiveCredential;
    // IMS_AUTH_NONCE_REUSE {
    ISipGenericChallenge* m_piGenericChallenge;
    // }

    // Current SIP connection for abnormal cases
    ISipClientConnection* m_piOngoingScc;
    // Message for the next REGISTER request
    ISipMessage* m_piNextRequest;
    // Message for the previous REGISTER request
    ISipMessage* m_piPreviousRequest;
    // Message for the previous REGISTER response
    ISipMessage* m_piPreviousResponse;
    IMessageMediator* m_piMessageMediator;

    // Observers of reg. state
    ImsList<RegObserver*> m_objObservers;

    // For NAT or firewall checking
    IMS_BOOL m_bIsBehindNat;
    // For trust domain checking
    IMS_BOOL m_bIsWithinTrustDomain;
    // REG_RESTORATION_FOR_ACTIVE_BINDING
    IMS_BOOL m_bActiveBindingsRestorationEnabled;
    // Reference count for ISipConnectionNotifierErrorListener
    IMS_SINT32 m_nRefCountForScnErrorListener;
    // Lenient to check Contact URI field of "reginfo" package subscription
    IMS_BOOL m_bLenientToCheckContactUriOfRegInfo;
};

#endif
