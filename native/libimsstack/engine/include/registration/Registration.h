/*
    Author
    <table>
    date      author                    description
    --------  --------------            ----------
    20090908  toastops@                 Created
    20100321  hwangoo.park@             Change Register(...) for expires value
    </table>

    Description

*/

#ifndef _REGISTRATION_H_
#define _REGISTRATION_H_

#include "ITimer.h"
#include "IDigestAkaListener.h"
#include "ISipClientConnectionListener.h"
#include "ISipErrorListener.h"
// SIP_MESSAGE_MEDIATOR
#include "base/IMessageMediator.h"
#include "IRegistrationEx.h"
#include "util/IRefreshable.h"
#include "IRegCapabilityChangeListener.h"
#include "IRegInfoListener.h"
#include "EngineActivity.h"
#include "Credential.h"
#include "RegContact.h"
#include "RegStateTracker.h"

class IDigestAka;
class ISipGenericChallenge;  // IMS_AUTH_NONCE_REUSE
class SubscriberConfig;
class IRegistrationListener;
class RegFlow;
class RegInfo;
class RegParameter;
class RegRefreshHelper;
class RegSubscription;

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
    virtual ~Registration();

private:
    Registration(IN CONST Registration& objRHS);
    Registration& operator=(IN CONST Registration& objRHS);

public:
    // IRegistration interface
    virtual IMS_BOOL Equals(IN CONST IRegistration* piReg) const;
    virtual IMS_BOOL IsBehindNAT() const;
    virtual IMS_BOOL IsWithinTrustDomain() const;

    // IRegistrationEx interface
    virtual const RegInfo* GetRegInfo() const;
    virtual const RegStateTracker* GetStateTracker() const;

    // MULTI_SUBS
    // MULTI_REG_SIP_PROFILE
    IMS_BOOL Create(IN IMS_UINT32 nFlowId, IN CONST SipAddress& objAOR,
            IN CONST AString& strSubsId = AString::ConstNull(),
            IN SipProfile* pSIPProfile = IMS_NULL);
    void Destroy();
    const IMSList<RegContact*>& GetAllContactsEx() const;
    IMS_BOOL HasActiveBindings() const;
    IMS_BOOL IsAllBindingsRemoved() const;

private:
    // EngineActivity class
    virtual IMS_BOOL DispatchMessage(IN IMSMSG& objMSG);

    // ISipClientConnectionListener interface
    virtual void ClientConnection_NotifyResponse(
            IN ISipClientConnection* piSCC, IN ISipClientConnection* piForkedSCC = IMS_NULL);

    // ISipErrorListener interface
    virtual void Error_NotifyError(
            IN ISipConnection* piSC, IN IMS_SINT32 nCode, IN CONST AString& strMessage);

    // IRegBase interface
    virtual ISipMessage* GetNextRequest();
    virtual ISipMessage* GetPreviousRequest() const;
    virtual ISipMessage* GetPreviousResponse() const;
    // SIP_MESSAGE_MEDIATOR
    virtual void SetSipMessageMediator(IN IMessageMediator* piMediator);

    // IRegistration interface
    virtual IMS_BOOL CreateBinding(IN CONST AString& strAppId, IN CONST AString& strServiceId);
    virtual void DestroyBinding(IN CONST AString& strAppId, IN CONST AString& strServiceId);
    virtual IRegContact* CreateContact(IN CONST IPAddress& objIPA, IN IMS_SINT32 nPort,
            IN IMS_SINT32 nExpiresPolicy = POLICY_EXPIRES_CONFIG,
            IN IMS_UINT32 nExpiresValue = DEFAULT_EXPIRES);
    virtual void DestroyAllContacts();
    virtual void DestroyContact(IN IRegContact* piContact);
    virtual void DestroyContact(IN CONST IPAddress& objIPA, IN IMS_SINT32 nPort);
    virtual const Credential* GetCredential() const;
    virtual const SipAddress& GetAOR() const;
    virtual const AStringArray& GetAssociatedURIs() const;
    virtual const SipAddress& GetAuthorizedAOR() const;
    virtual IMSList<IRegContact*> GetAllContacts() const;
    virtual IRegContact* GetContact(IN CONST IPAddress& objIPA, IN IMS_SINT32 nPort) const;
    virtual IRegContact* GetPreferredContact() const;
    virtual IRegParameter* GetParameter() const;
    virtual const IPAddress& GetPublicIPAddress() const;
    virtual const AStringArray& GetServiceRoutes() const;
    // MULTI_REG_SIP_PROFILE
    virtual SipProfile* GetSIPProfile() const;
    virtual IMS_SINT32 GetState() const;
    virtual IMS_BOOL IsBindingsUpdated() const;
    virtual IMS_BOOL IsBindingsUpdating() const;
    virtual IMS_BOOL IsNetworkInterworkingRequired() const;
    virtual IMS_RESULT Register(IN IMS_SINT32 nExpires = (-1));
    virtual IMS_RESULT Deregister();
    // REG_RESTORATION_FOR_ACTIVE_BINDING
    virtual void RemoveActiveBindingsForcingly();
    virtual void Restore();
    // REG_RESTORATION_FOR_ACTIVE_BINDING
    virtual IMS_RESULT RestoreActiveBindings();
    virtual void SetActiveBindingsRestorationUsage(IN IMS_BOOL bEnabled);
    // MULTI_SUBS
    virtual void SetAOR(
            IN CONST SipAddress& objAOR, IN CONST AString& strSubsId = AString::ConstNull());
    virtual void SetListener(IN IRegistrationListener* piListener);
    virtual void SetRefreshPolicy(IN IMS_SINT32 nPolicy, IN IMS_SINT32 nCriteriaInterval,
            IN IMS_SINT32 nValueEorLT, IN IMS_SINT32 nValueGT);
    // MULTI_REG_SIP_PROFILE
    virtual void SetSIPProfile(IN SipProfile* pProfile);
    virtual void SetBindingStateListener(IN IRegBindingStateListener* piListener);
    virtual void SetFlagForWithinTrustDomain(IN IMS_BOOL bWithinTrustDomain);
    virtual void SetUserIdentityNotifier(IN IRegUserIdentityNotifier* piUserIdNotifier);
    virtual void SetUserInfoForContactHeader(IN CONST AString& strUserInfo);
    virtual IRegSubscription* CreateSubscription(IN SipAddress* pResourceUri = IMS_NULL);

    // ISipConnectionNotifierErrorListener class
    virtual void ConnectionNotifierError_NotifyError(
            IN ISipConnectionNotifier* piSCN, IN IMS_SINT32 nCode, IN CONST AString& strMessage);

    // IRegistrationEx interface
    virtual void AddObserver(IN RegObserver* pObserver);
    virtual void RemoveObserver(IN RegObserver* pObserver);
    virtual IMS_SINT32 AddReferenceForSCNEL();
    virtual IMS_SINT32 RemoveReferenceForSCNEL();
    virtual void NotifyCallerCapabilityChanged();

    // IRefreshable interface
    virtual void Refreshable_RefreshCompleted(
            IN ISipClientConnection* piSCC, IN IMS_SINT32 nCode = 0);
    virtual IMS_BOOL Refreshable_RefreshStarted();
    virtual void Refreshable_RefreshTerminated();

    // IRegCapabilityChangeListener interface
    virtual void RegCapabilityChange_ServiceAdded(
            IN CONST AString& strAppId, IN CONST AString& strServiceId);
    virtual void RegCapabilityChange_ServiceRemoved(
            IN CONST AString& strAppId, IN CONST AString& strServiceId);

    // IRegInfoListener interface
    virtual void RegInfo_Updated(IN IMS_BOOL bStale = IMS_FALSE);
    virtual void RegInfo_UpdateFailed();

    // IDigestAkaListener interface
    virtual void DigestAka_OnResponse(IN const ByteArray& objRES,
            IN const ByteArray& objIK = ByteArray::ConstNull(),
            IN const ByteArray& objCK = ByteArray::ConstNull());
    virtual void DigestAka_OnAutsFailed(IN const ByteArray& objAUTS);
    virtual void DigestAka_OnMacFailed();

    // SIP_MESSAGE_MEDIATOR
    IMS_RESULT AdjustMessage(IN_OUT ISipMessage* piSIPMsg,
            IN IMS_SINT32 nMessage = IMessageMediator::MESSAGE_NORMAL);
    void CallListener(IN IMS_SINT32 nPrevState, IN IMS_SINT32 nPrevSubState, IN IMS_SINT32 nReason);
    void CheckUALocation(IN ISipMessage* piSIPMsg);
    void ChoosePreferredContact();
    void ClearNextRequest();
    IDigestAka* CreateDigestAKA(IN CONST SubscriberConfig* pSubsConfig);
    IMS_BOOL CreateSA(IN CONST Credential& objCredential, IN CONST IMS_SA_KEY& objSAKey);
    IMS_SINT32 GetPortUC() const;
    IMS_SINT32 GetPortUS() const;
    const SubscriberConfig* GetSubsConfig() const;
    IMS_SINT32 GetSubState() const;
    IMS_BOOL IsAkaSupported(IN CONST SubscriberConfig* pSubsConfig) const;
    IMS_BOOL IsFlowControlRequired() const;
    IMS_RESULT ReformContactHeader(IN_OUT ISipMessage* piSIPMsg);
    IMS_BOOL RegisterOnImplicitRefresh();
    IMS_BOOL RespondToChallenge(IN ISipClientConnection* piSCC);
    IMS_BOOL RespondToPendingChallenge(IN CONST Credential& objCredential);
    void RestoreSecurityHeaders();
    // IMS_AUTH_NONCE_REUSE {
    void SetAuthenticationChallenge(IN ISipGenericChallenge* piChallenge);
    // }
    IMS_RESULT SetContactNExpiresHeader(
            IN_OUT ISipMessage* piSIPMsg, IN IMS_SINT32 nExpires = (-1));
    IMS_RESULT SetExpiresHeader(IN_OUT ISipMessage* piSIPMsg, IN IMS_SINT32 nExpires = (-1));
    IMS_RESULT SetHeaders(IN ISipClientConnection* piSCC);
    // IMS_AUTH_NONCE_REUSE {
    void SetNextAuthenticationInfo(IN_OUT ISipClientConnection*& piSCC);
    // SIP_DIGEST_AUTH_NONCE_REUSE
    void SetNextNonce(IN ISipMessage* piSIPMsg);
    // }
    void SetOngoingConnection(IN ISipClientConnection* piSCC);
    void SetPreviousRequest(IN ISipMessage* piSIPMsg);
    void SetPreviousResponse(IN ISipMessage* piSIPMsg);
    void SetState(IN IMS_SINT32 nState);
    void SetSubState(IN IMS_SINT32 nSubState);
    void StorePersistentHeaders(IN CONST ISipMessage* piSIPMsg);
    IMS_RESULT UpdateBindings(IN CONST ISipMessage* piSIPMsg);
    void UpdateBindingState(IN IMS_SINT32 nState);
    // IMS_IPSEC_UDP_ENC
    void UpdateHostInfoInAllContacts();
    void UpdateCSeqNumber(IN CONST ISipMessage* piSIPMsg);
    void UpdateProtectedServerPortForContact(IN_OUT ISipMessage* piSIPMsg);
    void UpdateRefreshTimer();

    // Utility methods
    static ISipClientConnection* CreateConnection(IN Registration* pReg);
    static void DestroyAllHeaders(IN_OUT IMSList<ISipHeader*>& objHeaders);

    static const IMS_CHAR* StateToString(IN IMS_SINT32 nState);
    static const IMS_CHAR* SubStateToString(IN IMS_SINT32 nSubState);

private:
    // MSG for registration
    enum
    {
        AMSG_REGISTRATION_STARTED = AMSG_USER,
        AMSG_REGISTRATION_START_FAILED,
        AMSG_REGISTRATION_UPDATED,
        AMSG_REGISTRATION_UPDATE_FAILED,
        AMSG_REGISTRATION_REMOVED,
        AMSG_REGISTRATION_TERMINATED,

        // For authentication challenge
        AMSG_REGISTRATION_AKA_RESPONSE_RECEIVED,

        // For binding state (ex. caller capability) change notification
        AMSG_REGISTRATION_BINDING_STATE_CHANGED
    };

    // Sub-State of registration
    enum
    {
        // No transaction
        SUB_STATE_IDLE = 0,
        // Initial REGISTER or re-REGISTER
        SUB_STATE_REGISTERING,
        // Implicit re-REGISTER
        SUB_STATE_REFRESHING,
        // de-REGISTER
        SUB_STATE_DEREGISTERING
    };

    // Result of binding updates
    enum
    {
        BINDING_UPDATE_OK = 0x00,
        BINDING_UPDATE_NO_EXPIRES = 0x01
    };

    // Type of binding state changes
    enum
    {
        BINDING_STATE_CALLER_CAPABILITY
    };

    // Mark the flag if Registration object is destroyed
    IMS_BOOL bDestroyed;

    // State of registration binding (from IRegistration)
    IMS_SINT32 nState;
    // Sub-State of registration (REGISTERING or IDLE)
    IMS_SINT32 nSubState;

    // Reference to the registration flow (Call-ID, Command sequence number, ...)
    // Don't delete this pointer explicitly.
    RegFlow* pRegFlow;
    // Parameters for registration
    RegParameter* pRegParam;
    // Registration contact list
    IMSList<RegContact*> objContacts;
    // Registration State Tracker
    // IMPU & Network authorized IMPU (topmost one in P-Associated-URI)
    // Persistent information (P-Associated-URI, Service-Routes) which MUST be kept
    // while the registration is active
    RCPtr<RegStateTracker> pStateTracker;
    // Listener to notify the registration state
    IRegistrationListener* piListener;
    // Listener to monitor the changes of the service's state
    IRegBindingStateListener* piBindingStateListener;
    // Notifier to reorder the network provisioned user identities
    IRegUserIdentityNotifier* piUserIdNotifier;

    // Refresh helper
    RegRefreshHelper* pRefreshHelper;

    // Digest AKA interface
    IDigestAka* piDigestAKA;
    IMS_SA_KEY objActiveSAKey;
    Credential objActiveCredential;
    // IMS_AUTH_NONCE_REUSE {
    ISipGenericChallenge* piGenericChallenge;
    // }

    // Current SIP connection for abnormal cases
    ISipClientConnection* piOngoingSCC;
    // Message for the next REGISTER request
    ISipMessage* piNextRequest;
    // Message for the previous REGISTER request
    ISipMessage* piPreviousRequest;
    // Message for the previous REGISTER response
    ISipMessage* piPreviousResponse;
    // SIP_MESSAGE_MEDIATOR
    IMessageMediator* piMessageMediator;

    // Observers of reg. state
    IMSList<RegObserver*> objObservers;

    // For NAT or firewall checking
    IMS_BOOL bIsBehindNAT;
    // For trust domain checking
    IMS_BOOL bIsWithinTrustDomain;
    // REG_RESTORATION_FOR_ACTIVE_BINDING
    IMS_BOOL bActiveBindingsRestorationEnabled;

    // Reference count for ISipConnectionNotifierErrorListener
    IMS_SINT32 nRefCountForSCNEL;
};

#endif  // _REGISTRATION_H_
