/*
    Author
    <table>
    date      author                    description
    --------  --------------            ----------
    20090719  toastops@                 Created
    </table>

    Description

*/

#ifndef _SESSION_IMPL_H_
#define _SESSION_IMPL_H_

#include "ISession.h"
#include "IOnSessionListener.h"
#include "IOnSessionExListener.h"
#include "SessionEx.h"
#include "VirtualSessionImpl.h"

class ICapabilities;
class IReference;
class ISubscription;
class ISessionDescriptor;
class IMedia;
class MediaImpl;
class VirtualSession;

class SessionImpl : public ISession, public IOnSessionListener, public IOnSessionExListener
{
public:
    explicit SessionImpl(IN SessionEx* piSession_);
    virtual ~SessionImpl();

private:
    SessionImpl(IN CONST SessionImpl& objRHS);
    SessionImpl& operator=(IN CONST SessionImpl& objRHS);

public:
    inline SessionEx* GetSession() const { return pSession; }

private:
    // IMethod interface
    virtual void Destroy();
    // SIP_MESSAGE_MEDIATOR
    virtual void SetMessageMediator(IN IMessageMediator* piMediator);

    // IServiceMethod interface
    virtual IMessage* GetNextRequest();
    virtual IMessage* GetNextResponse();
    virtual IMessage* GetPreviousRequest(IN IMS_SINT32 nServiceMethod) const;
    virtual IMessage* GetPreviousResponse(IN IMS_SINT32 nServiceMethod) const;
    virtual IMSList<IMessage*> GetPreviousResponses(IN IMS_SINT32 nServiceMethod) const;
    virtual IMSList<AString> GetRemoteUserId() const;

    // ISession interface
    virtual IMS_RESULT Accept();
    virtual ICapabilities* CreateCapabilities();
    virtual IMedia* CreateMedia(IN CONST AString& strType, IN IMS_SINT32 nDirection,
            IN IMS_SINT32 nCountOfDescriptor = 0, IN IMS_BOOL bIMSExtension = IMS_TRUE);
    virtual IReference* CreateReference(
            IN CONST AString& strReferTo, IN CONST AString& strReferMethod);
    virtual IMSList<IMedia*> GetMedia();
    virtual ISessionDescriptor* GetSessionDescriptor();
    virtual IMS_SINT32 GetState() const;
    virtual IMS_BOOL HasPendingUpdate() const;
    virtual IMS_RESULT Reject();
    virtual IMS_RESULT Reject(IN IMS_SINT32 nStatusCode);
    virtual IMS_RESULT RejectWithDiversion(IN CONST AString& strAlternativeUserAddress);
    virtual IMS_RESULT RemoveMedia(IN IMedia* piMedia);
    virtual IMS_RESULT Restore();
    virtual void SetListener(IN ISessionListener* piListener);
    virtual IMS_RESULT Start();
    virtual IMS_RESULT Terminate();
    virtual IMS_RESULT Update();
    //// IMS extensions
    virtual ISubscription* CreateSubscription(IN CONST AString& strEvent);
    virtual ISipClientConnection* CreateTransaction(IN CONST SipMethod& objMethod);
    virtual IMS_SINT32 GetConfiguration() const;
    virtual const ISipHeader* GetContactHeader() const;
    virtual const Replaces* GetReplaces() const;
    virtual const AString& GetSessionId() const;
    virtual IMS_SINT32 GetTerminationReason() const;
    virtual IMS_BOOL IsFinalResponseReceivedForInitialInviteRequest() const;
    virtual IMS_BOOL IsReliableProvResponseSupported() const;
    virtual IMS_BOOL IsSDPNegotiationAllowedForNonRPR() const;
    virtual IMS_RESULT RejectEx(
            IN IMS_SINT32 nStatusCode, IN CONST AString& strReasonPhrase = AString::ConstNull());
    virtual IMS_RESULT RespondToEarlyUpdate(
            IN IMS_SINT32 nStatusCode, IN CONST AString& strReason = AString::ConstNull());
    virtual IMS_RESULT RespondToPRAck(
            IN IMS_SINT32 nStatusCode, IN CONST AString& strReason = AString::ConstNull());
    virtual IMS_RESULT SendAck();
    virtual IMS_RESULT SendPRAck();
    virtual IMS_RESULT SendProvisionalResponse(IN IMS_SINT32 nStatusCode,
            IN CONST AString& strReason = AString::ConstNull(), IN IMS_SINT32 nFlags = 0);
    virtual IMS_RESULT SendRPR(IN IMS_SINT32 nStatusCode,
            IN CONST AString& strReason = AString::ConstNull(), IN IMS_BOOL bSDP = IMS_TRUE,
            IN IMS_SINT32 nFlags = 0);
    virtual IMS_RESULT SetCallerPreference(IN CONST IMSList<AString>& objCallerPreference);
    virtual void SetConfiguration(IN IMS_SINT32 nConfigValue);
    // CONTACT_HEADER_PARAMETER_CONTROL_FOR_MID_DIALOG_REQUEST
    virtual IMS_RESULT SetContactParameter(
            IN CONST AString& strParameter, IN IMS_SINT32 nOperation = 0 /* (0: ADD, 1: REMOVE) */);
    virtual void SetImplicitRoutingRequired(IN IMS_BOOL bFlag);
    virtual void SetReasonForCallTermination(IN IMS_SINT32 nReason);
    virtual void SetRefreshListener(IN IRefreshListener* piListener);
    virtual void SetRefreshPolicy(IN IMS_SINT32 nPolicy, IN IMS_SINT32 nCriteriaInterval,
            IN IMS_SINT32 nValueEorLT, IN IMS_SINT32 nValueGT);
    virtual IMS_RESULT TerminateEx(IN IMS_BOOL bTerminateMethodBYE = IMS_FALSE);
    virtual IMS_RESULT UpdateEarlyMedia();
    virtual IMS_RESULT UpdateEx(
            IN IMS_SINT32 nMethod = SipMethod::INVALID, IN IMS_BOOL bSessionRefresh = IMS_FALSE);

    // REFUSE_SDP_OFFER_ANSWER_EXCHANGE {
    virtual IMS_RESULT CreateFailureSdp();
    virtual void DestroyFailureSdp();
    virtual ISessionParameter* GetFailureSdp() const;
    // }
    // EARLY_SESSION_MODEL {
    inline virtual ISession* GetOwnerSession() const { return IMS_NULL; }
    inline virtual ISession* GetVirtualSession() const { return pVirtualSessionImpl; }
    // }

    // IOnSessionListener interface
    virtual void OnSession_Alerting(IN Session* pSession);
    virtual void OnSession_ReferenceReceived(IN Session* pSession, IN Reference* pReference);
    virtual void OnSession_Started(IN Session* pSession);
    virtual void OnSession_StartFailed(IN Session* pSession);
    virtual void OnSession_Terminated(IN Session* pSession);
    virtual void OnSession_Updated(IN Session* pSession);
    virtual void OnSession_UpdateFailed(IN Session* pSession);
    virtual void OnSession_UpdateReceived(IN Session* pSession);
    virtual void OnSession_CancelDelivered(IN Session* pSession);
    virtual void OnSession_CancelDeliveryFailed(IN Session* pSession);
    virtual IMS_BOOL OnSession_ForkedResponseReceived(
            IN Session* pSession, IN Session* pForkedSession);
    virtual void OnSession_ProvisionalResponseReceived(
            IN Session* pSession, IN IMS_UINT32 nIndex = 0xFFFFFFFF);
    virtual IMS_BOOL OnSession_TransactionReceived(
            IN Session* pSession, IN ISipServerConnection* piSSC);

    // IOnSessionExListener interface
    virtual void OnSessionEx_EarlyMediaUpdated(IN SessionEx* pSessionEx);
    virtual void OnSessionEx_EarlyMediaUpdateFailed(IN SessionEx* pSessionEx);
    virtual void OnSessionEx_EarlyMediaUpdateReceived(IN SessionEx* pSessionEx);
    virtual void OnSessionEx_PRAckDelivered(IN SessionEx* pSessionEx);
    virtual void OnSessionEx_PRAckDeliveryFailed(IN SessionEx* pSessionEx);
    virtual void OnSessionEx_PRAckReceived(IN SessionEx* pSessionEx);
    virtual void OnSessionEx_RPRDeliveryFailed(IN SessionEx* pSessionEx);
    virtual void OnSessionEx_RPRReceived(IN SessionEx* pSessionEx,
            IN VirtualSession* pVirtualSession, IN IMS_UINT32 nIndex = 0xFFFFFFFF);

    void UpdateVirtualSession(IN VirtualSession* pVirtualSession);

private:
    ISessionListener* piListener;
    SessionEx* pSession;
    IMSList<MediaImpl*> objMediaImpls;
    VirtualSessionImpl* pVirtualSessionImpl;
};

#endif  // _SESSION_IMPL_H_
