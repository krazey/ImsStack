#ifndef START_ERROR_HANDLER_H_
#define START_ERROR_HANDLER_H_

#include "CallReasonInfo.h"
#include "IMSTypeDef.h"

class IMessage;
class IMtcCallContext;
class MtcAosConnector;

/*
 * It handles error responses when `ISessionListener::SessionStartFailed` occurs.
 */
class StartErrorHandler final
{
public:
    explicit StartErrorHandler(IN IMtcCallContext& objContext);
    ~StartErrorHandler();
    StartErrorHandler(const StartErrorHandler&) = delete;
    StartErrorHandler& operator=(const StartErrorHandler&) = delete;

    /**
     * Returns `CallReasonInfo` for the incoming message. Possibly contains some extra logic,
     * but retrying is not handled here.
     *
     * @param piMessage Received error response. Could be null if no response has came.
     * @return See `CallReasonInfo.h` for the possible values.
     */
    CallReasonInfo Handle(IN const IMessage* piMessage) const;

private:
    CallReasonInfo GetCallReasonInfoForTransactionTimeout() const;
    CallReasonInfo HandleResponse(IN const IMessage& objMessage) const;

    CallReasonInfo Handle3xxResponse(IN const IMessage& objMessage) const;
    CallReasonInfo Handle380Response(IN const IMessage& objMessage) const;

    CallReasonInfo Handle4xxResponse(IN const IMessage& objMessage) const;
    CallReasonInfo Handle403Response() const;
    CallReasonInfo Handle404Response() const;
    CallReasonInfo Handle407Response() const;

    CallReasonInfo Handle5xxResponse(IN const IMessage& objMessage) const;
    CallReasonInfo Handle500Response(IN const IMessage& objMessage) const;
    CallReasonInfo Handle503Response(IN const IMessage& objMessage) const;
    CallReasonInfo Handle504Response(IN const IMessage& objMessage) const;

    CallReasonInfo Handle6xxResponse(IN const IMessage& objMessage) const;

    IMS_BOOL IsTransactionTimeout(IN const IMessage* piMessage) const;
    IMS_BOOL IsRetry1xRequiredForNormalCall(IN const IMessage& objMessage) const;
    IMS_BOOL IsNonUeDetectableEmergencyCall(IN const IMessage& objMessage) const;
    IMS_BOOL IsIpcanResourceUnavailable(IN const IMessage& objMessage) const;
    IMS_BOOL HasEmergencyServiceTypeInBody(IN const IMessage& objMessage) const;

    void ControlAos(IMS_UINT32 nCommand) const;
    AString GetPathHeader() const;
    AString GetLastPathHeader() const;
    AString GetServiceRouteHeader() const;
    AString GetSupported() const;
    MtcAosConnector* GetAosConnector() const;

    IMtcCallContext& m_objContext;
};

#endif
