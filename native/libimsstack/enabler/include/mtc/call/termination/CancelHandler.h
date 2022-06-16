#ifndef CANCEL_HANDLER_H_
#define CANCEL_HANDLER_H_

#include "CallReasonInfo.h"
#include "IMSTypeDef.h"

class IMessage;

class CancelHandler final
{
public:
    explicit CancelHandler();
    ~CancelHandler();
    CancelHandler(const CancelHandler&) = delete;
    CancelHandler& operator=(const CancelHandler&) = delete;

    CallReasonInfo Handle(IN const IMessage& objMessage) const;

private:
    static const AString REASON_TEXT_CALL_BUSY;
    static const AString REASON_TEXT_CALL_COMPLETED;
    static const AString REASON_TEXT_CALL_DECLINED;

    CallReasonInfo GetCallReasonInfoFromReasonHeader(
            IN IMS_SINT32 nCause, IN const AString& strText) const;
};

#endif
