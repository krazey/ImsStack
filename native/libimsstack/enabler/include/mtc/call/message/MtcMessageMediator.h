#ifndef MTC_MESSAGE_MEDIATOR_H_
#define MTC_MESSAGE_MEDIATOR_H_

#include "IMSTypeDef.h"
#include "base/IMessageMediator.h"

class IMtcCallContext;
class ISipMessage;

class MtcMessageMediator final : public IMessageMediator
{
public:
    explicit MtcMessageMediator(IN IMtcCallContext& objContext);
    ~MtcMessageMediator();
    MtcMessageMediator(IN const MtcMessageMediator&) = delete;
    MtcMessageMediator& operator=(IN const MtcMessageMediator&) = delete;

    IMS_RESULT MessageMediator_AdjustMessage(
            IN_OUT ISipMessage* piSipMessage, IN IMS_SINT32 nMessage) override;

private:
    AString GetContactHeaderWithoutFeatureTag(IN const AString& strFeatureTag);
    CallType GetCallTypeOfCurrentMessage();

    IMtcCallContext& m_objContext;
    AString m_strOriginalContactHeader;
};

#endif
