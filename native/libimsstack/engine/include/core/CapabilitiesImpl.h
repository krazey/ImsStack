/*
    Author
    <table>
    date      author                    description
    --------  --------------            ----------
    20091201  toastops@                 Created
    </table>

    Description
*/

#ifndef _CAPABILITIES_IMPL_H_
#define _CAPABILITIES_IMPL_H_

#include "ICapabilities.h"
#include "IOnCapabilitiesListener.h"
#include "Capabilities.h"

class CapabilitiesImpl : public ICapabilities, public IOnCapabilitiesListener
{
public:
    explicit CapabilitiesImpl(IN Capabilities* pCapabilities_);
    virtual ~CapabilitiesImpl();

private:
    CapabilitiesImpl(IN CONST CapabilitiesImpl& objRHS);
    CapabilitiesImpl& operator=(IN CONST CapabilitiesImpl& objRHS);

private:
    // IMethod
    virtual void Destroy();
    // SIP_MESSAGE_MEDIATOR
    virtual void SetMessageMediator(IN IMessageMediator* piMediator);

    // IServiceMethod
    virtual IMessage* GetNextRequest();
    virtual IMessage* GetNextResponse();
    virtual IMessage* GetPreviousRequest(IN IMS_SINT32 nServiceMethod) const;
    virtual IMessage* GetPreviousResponse(IN IMS_SINT32 nServiceMethod) const;
    virtual IMSList<IMessage*> GetPreviousResponses(IN IMS_SINT32 nServiceMethod) const;
    virtual IMSList<AString> GetRemoteUserId() const;

    // ICapabilities
    virtual IMSList<AString> GetRemoteUserIdentities() const;
    virtual IMS_SINT32 GetState() const;
    virtual IMS_BOOL HasCapabilities(IN CONST AString& strConnection) const;
    virtual IMS_RESULT QueryCapabilities(IN IMS_BOOL bSDPInRequest,
            IN IMS_BOOL bContactInRequest = IMS_TRUE, IN IMS_BOOL bCheckSupport = IMS_TRUE);
    virtual IMS_RESULT QueryCapabilitiesEx();
    virtual void SetListener(IN ICapabilitiesListener* piListener);

    //// IMS extensions
    virtual IMS_RESULT Accept(
            IN IMS_BOOL bFeatureInContact = IMS_TRUE, IN IMS_BOOL bCheckSupport = IMS_TRUE);
    virtual IMS_RESULT AcceptEx();
    virtual IMS_RESULT Reject(IN IMS_SINT32 nStatusCode, IN IMS_SINT32 nRetryAfter = 0);

    // IOnCapabilitiesListener interface
    virtual void OnCapabilities_QueryDelivered(IN Capabilities* pCapabilities);
    virtual void OnCapabilities_QueryDeliveryFailed(IN Capabilities* pCapabilities);

private:
    ICapabilitiesListener* piListener;

    Capabilities* pCapabilities;
};

#endif  // _CAPABILITIES_IMPL_H_
