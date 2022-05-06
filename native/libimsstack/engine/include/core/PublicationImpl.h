/*
    Author
    <table>
    date      author                    description
    --------  --------------            ----------
    20100423  hwangoo.park@             Created
    </table>

    Description

*/

#ifndef _PUBLICATION_IMPL_H_
#define _PUBLICATION_IMPL_H_

#include "IPublication.h"
#include "IOnPublicationListener.h"
#include "Publication.h"

class PublicationImpl : public IPublication, public IOnPublicationListener
{
public:
    explicit PublicationImpl(IN Publication* pPublication_);
    virtual ~PublicationImpl();

private:
    PublicationImpl(IN CONST PublicationImpl& objRHS);
    PublicationImpl& operator=(IN CONST PublicationImpl& objRHS);

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

    // IPublication interface
    virtual const AString& GetEvent() const;
    virtual IMS_SINT32 GetState() const;
    virtual IMS_RESULT Publish(IN CONST ByteArray& objState, IN CONST AString& strContentType);
    virtual void SetListener(IN IPublicationListener* piListener);
    virtual IMS_RESULT Unpublish();

    //// IMS extensions
    virtual void SetRefreshListener(IN IRefreshListener* piListener);
    virtual void SetRefreshPolicy(IN IMS_SINT32 nPolicy, IN IMS_SINT32 nCriteriaInterval,
            IN IMS_SINT32 nValueEorLT, IN IMS_SINT32 nValueGT);

    // IOnPublicationListener interface
    virtual void OnPublication_Delivered(IN Publication* pPublication);
    virtual void OnPublication_DeliveryFailed(IN Publication* pPublication);
    virtual void OnPublication_Terminated(IN Publication* pPublication);
    //[2012/11/5]hyunho.shin : add publication refresh api
    virtual void OnPublication_RefreshStarted(IN Publication* pPublication);
    virtual void OnPublication_RefreshCompleted(IN Publication* pPublication);
    //[2012/11/5]hyunho.shin : end

private:
    IPublicationListener* piListener;

    Publication* pPublication;
};

#endif  // _PUBLICATION_IMPL_H_
