#ifndef UPDATE_ERROR_HANDLER_H_
#define UPDATE_ERROR_HANDLER_H_

#include "CallReasonInfo.h"
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
     * Returns `CallReasonInfo` for the incoming message.
     *
     * @param piMessage Received error response. Could be null if no response has came.
     * @return See `CallReasonInfo.h` for the possible values.
     */
    CallReasonInfo Handle(IN const IMessage* piMessage) const;

private:
    CallReasonInfo GetCallReasonInfoForResponse(IN const IMessage& objMessage) const;
    CallReasonInfo GetCallReasonInfoFor3xxResponse(IN const IMessage& objMessage) const;
    CallReasonInfo GetCallReasonInfoFor4xxResponse(IN const IMessage& objMessage) const;
    CallReasonInfo GetCallReasonInfoFor5xxResponse(IN const IMessage& objMessage) const;
    CallReasonInfo GetCallReasonInfoFor6xxResponse(IN const IMessage& objMessage) const;

    IMS_UINT32 GetGlareTimeMillisecond(IN PeerType ePeerType) const;

    IMtcCallContext& m_objContext;
};

#endif
