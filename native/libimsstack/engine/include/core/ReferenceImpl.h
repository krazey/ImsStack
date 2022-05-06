/*
    Author
    <table>
    date      author                    description
    --------  --------------            ----------
    20100413  hwangoo.park@             Created
    </table>

    Description

*/

#ifndef _REFERENCE_IMPL_H_
#define _REFERENCE_IMPL_H_

#include "IReference.h"
#include "IOnNotificationListener.h"
#include "IOnReferenceListener.h"
#include "Reference.h"

class ReferenceImpl : public IReference, public IOnReferenceListener, public IOnNotificationListener
{
public:
    explicit ReferenceImpl(IN Reference* pReference_);
    virtual ~ReferenceImpl();

private:
    ReferenceImpl(IN CONST ReferenceImpl& objRHS);
    ReferenceImpl& operator=(IN CONST ReferenceImpl& objRHS);

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

    // IReference interface
    virtual IMS_RESULT Accept();
    virtual IMS_RESULT ConnectReferMethod(IN IServiceMethod* piServiceMethod);
    virtual const AString& GetReferMethod() const;
    virtual const AString& GetReferToUserId() const;
    virtual const AString& GetReplaces() const;
    virtual IMS_SINT32 GetState() const;
    virtual IMS_RESULT Refer(IN IMS_BOOL bImplicitSubscription);
    virtual IMS_RESULT Reject();
    virtual void SetListener(IN IReferenceListener* piListener);
    virtual IMS_RESULT SetReplaces(IN CONST AString& strSessionId);
    //// IMS extensions
    virtual IMS_RESULT AcceptEx(IN IMS_SINT32 nStatusCode = 202, IN IMS_BOOL b100Trying = IMS_TRUE);
    virtual IMS_RESULT ReferEx(IN IMS_BOOL bImplicitSubscription,
            IN CONST AString& strHeadersForReferTo = AString::ConstNull());
    virtual IMS_RESULT RejectEx(IN IMS_SINT32 nStatusCode);
    virtual IMS_RESULT SendNotification(IN IMS_SINT32 nSubState, IN CONST ByteArray& objContent,
            IN IMS_SINT32 nReason = ISubscriptionState::REASON_NONE, IN IMS_SINT32 nExpires = (-1));
    virtual void SetNotificationListener(IN INotificationListener* piListener);
    virtual void SetImplicitRoutingRequired(IN IMS_BOOL bFlag);

    // IOnReferenceListener interface
    virtual void OnReference_Delivered(IN Reference* pReference);
    virtual void OnReference_DeliveryFailed(IN Reference* pReference);
    virtual void OnReference_NotifyReceived(IN Reference* pReference, IN Message* pNotify);
    virtual void OnReference_Terminated(IN Reference* pReference);

    // IOnNotificationListener interface
    virtual void OnNotification_Delivered(IN ServiceMethod* pMethod);
    virtual void OnNotification_DeliveryFailed(
            IN ServiceMethod* pMethod, IN IMS_SINT32 nStatusCode);

private:
    IReferenceListener* piListener;
    INotificationListener* piNotificationListener;

    Reference* pReference;
};

#endif  // _REFERENCE_IMPL_H_
