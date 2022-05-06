/*
    Author
    <table>
    date      author                    description
    --------  --------------            ----------
    20100506  hwangoo.park@             Created
    </table>

    Description
*/

#ifndef _PAGE_MESSAGE_IMPL_H_
#define _PAGE_MESSAGE_IMPL_H_

#include "IPageMessage.h"
#include "IOnPageMessageListener.h"
#include "PageMessage.h"

class PageMessageImpl : public IPageMessage, public IOnPageMessageListener
{
public:
    explicit PageMessageImpl(IN PageMessage* pPageMessage_);
    virtual ~PageMessageImpl();

private:
    PageMessageImpl(IN CONST PageMessageImpl& objRHS);
    PageMessageImpl& operator=(IN CONST PageMessageImpl& objRHS);

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

    // IPageMessage interface
    virtual const ByteArray& GetContent() const;
    virtual AString GetContentType() const;
    virtual IMS_SINT32 GetState() const;
    virtual IMS_RESULT Send(IN CONST ByteArray& objContent, IN CONST AString& strContentType);
    virtual void SetListener(IN IPageMessageListener* piListener);
    //// IMS extensions
    virtual IMS_RESULT Accept(IN IMS_SINT32 nStatusCode = 200);
    virtual IMS_RESULT Reject(IN IMS_SINT32 nStatusCode, IN IMS_SINT32 nRetryAfter = 0);

    // IOnPageMessageListener interface
    virtual void OnPageMessage_Delivered(IN PageMessage* pPageMessage);
    virtual void OnPageMessage_DeliveryFailed(IN PageMessage* pPageMessage);

private:
    IPageMessageListener* piListener;

    PageMessage* pPageMessage;
};

#endif  // _PAGE_MESSAGE_IMPL_H_
