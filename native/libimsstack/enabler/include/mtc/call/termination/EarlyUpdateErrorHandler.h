#ifndef EARLY_UPDATE_ERROR_HANDLER_H_
#define EARLY_UPDATE_ERROR_HANDLER_H_

#include "CallReasonInfo.h"
#include "IMSTypeDef.h"

class IMessage;
class IMtcCallContext;

/*
 * It handles error responses when `ISessionListener::SessionUpdateFailed` occurs before the call
 * establishing.
 */
class EarlyUpdateErrorHandler final
{
public:
    explicit EarlyUpdateErrorHandler();
    ~EarlyUpdateErrorHandler();
    EarlyUpdateErrorHandler(const EarlyUpdateErrorHandler&) = delete;
    EarlyUpdateErrorHandler& operator=(const EarlyUpdateErrorHandler&) = delete;

    /**
     * Returns `CallReasonInfo` for the incoming message.
     *
     * @param piMessage Received error response. Could be null if no response has came.
     * @return See `CallReasonInfo.h` for the possible values.
     */
    CallReasonInfo Handle(IN const IMessage* piMessage) const;

private:
    IMS_BOOL IsTransactionTimeout(IN const IMessage* piMessage) const;
};

#endif
