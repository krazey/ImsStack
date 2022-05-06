/*
    Author
    <table>
    date      author                    description
    --------  --------------            ----------
    20201023  hwangoo.park@             Created
    </table>

    Description

*/

#ifndef _VIRTUAL_SESSION_IMPL_H_
#define _VIRTUAL_SESSION_IMPL_H_

#include "ISession.h"
#include "Session.h"
#include "VirtualSession.h"

class ISessionDescriptor;
class IMedia;
class MediaImpl;

class VirtualSessionImpl : public ISession
{
public:
    explicit VirtualSessionImpl(IN ISession* piOwnerSession_, IN VirtualSession* pSession_);
    virtual ~VirtualSessionImpl();

private:
    VirtualSessionImpl(IN const VirtualSessionImpl& objRHS);
    VirtualSessionImpl& operator=(IN const VirtualSessionImpl& objRHS);

public:
    inline VirtualSession* GetSession() const { return pSession.Get(); }
    inline void UpdateSession(IN VirtualSession* pSession_) { pSession = pSession_; }

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
    inline virtual ISession* GetOwnerSession() const { return piOwnerSession; }
    inline virtual ISession* GetVirtualSession() const { return IMS_NULL; }
    // }

private:
    ISession* piOwnerSession;
    RCPtr<VirtualSession> pSession;
    IMSList<MediaImpl*> objMediaImpls;
};

#endif  // _VIRTUAL_SESSION_IMPL_H_
