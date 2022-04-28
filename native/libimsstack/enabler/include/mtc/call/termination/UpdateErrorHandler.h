#ifndef UPDATE_ERROR_HANDLER_H_
#define UPDATE_ERROR_HANDLER_H_

#include "FailReason.h"
#include "IMSTypeDef.h"

class IMessage;
class IMtcCallContext;

/*
 * It handles error responses when `ISessionListener::SessionUpdateFailed` occurs after the call
 * establishing.
 */
class UpdateErrorHandler final
{
public:
    explicit UpdateErrorHandler(IN IMtcCallContext& objContext);
    ~UpdateErrorHandler();
    UpdateErrorHandler(const UpdateErrorHandler&) = delete;
    UpdateErrorHandler& operator=(const UpdateErrorHandler&) = delete;

    /**
     * Returns `FailReason` for the incoming message.
     *
     * @param piMessage Received error response. Could be null if no response has came.
     * @return See `FailReason.h` for the possible values.
     */
    FailReason Handle(IN const IMessage* piMessage) const;

private:
    FailReason GetFailReasonForResponse(IN const IMessage& objMessage) const;
    FailReason GetFailReasonFor3xxResponse(IN const IMessage& objMessage) const;
    FailReason GetFailReasonFor4xxResponse(IN const IMessage& objMessage) const;
    FailReason GetFailReasonFor5xxResponse(IN const IMessage& objMessage) const;
    FailReason GetFailReasonFor6xxResponse(IN const IMessage& objMessage) const;

    IMS_UINT32 GetGlareTimeMillisecond(IN PeerType ePeerType) const;

    IMtcCallContext& m_objContext;
};

#endif
