/*
    Author
    <table>
    date      author                    description
    --------  --------------            ----------
    20130227  hwangoo.park@             Created
    </table>

    Description

*/

#ifndef _FAKE_REGISTRATION_H_
#define _FAKE_REGISTRATION_H_

#include "IRegistrationEx.h"
#include "IRegCapabilityChangeListener.h"
#include "EngineActivity.h"
#include "RegContact.h"
#include "RegStateTracker.h"

class IRegistrationListener;
class RegFlow;
class RegParameter;

class FakeRegistration :
        public EngineActivity,
        public IRegistrationEx,
        public IRegCapabilityChangeListener
{
public:
    FakeRegistration();
    virtual ~FakeRegistration();

private:
    FakeRegistration(IN CONST FakeRegistration& objRHS);
    FakeRegistration& operator=(IN CONST FakeRegistration& objRHS);

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

    // IRegCapabilityChangeListener interface
    virtual void RegCapabilityChange_ServiceAdded(
            IN CONST AString& strAppId, IN CONST AString& strServiceId);
    virtual void RegCapabilityChange_ServiceRemoved(
            IN CONST AString& strAppId, IN CONST AString& strServiceId);

    void CallListener(IN IMS_SINT32 nPrevState, IN IMS_SINT32 nPrevSubState, IN IMS_SINT32 nReason);
    void ChoosePreferredContact();
    IMS_SINT32 GetPortUC() const;
    IMS_SINT32 GetPortUS() const;
    IMS_SINT32 GetSubState() const;
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

        // For fake transaction completion
        AMSG_REGISTRATION_RESPONSE_RECEIVED
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
    // Notifier to reorder the network provisioned user identities
    IRegUserIdentityNotifier* piUserIdNotifier;

    // Observers of reg. state
    IMSList<RegObserver*> objObservers;

    // For trust domain checking
    IMS_BOOL bIsWithinTrustDomain;

    // Reference count for ISipConnectionNotifierErrorListener
    IMS_SINT32 nRefCountForSCNEL;
};

#endif  // _FAKE_REGISTRATION_H_
