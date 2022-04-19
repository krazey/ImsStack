#ifndef INTERFACE_MTC_CALL_H_
#define INTERFACE_MTC_CALL_H_

#include "AString.h"
#include "IMSList.h"
#include "IMSMap.h"
#include "IMSTypeDef.h"
#include "CallInfo.h"
#include "IMtcService.h"

class CallContext;
class ConfUser;
class ISession;
class IDialogEvent;
class JniMediaSessionThread;
class JniMtcCallThread;
class JniMtcServiceThread;
class MediaInfo;
class SuppService;
class IMtcCallContext;
enum class CallType;
enum class UpdateType;
struct FailReason;

using CallKey = IMS_UINTP;

class IMtcCall
{
public:
    enum class State
    {
        IDLE,
        OUTGOING,
        INCOMING,
        ALERTING,
        ESTABLISHED,
        UPDATING,
        TERMINATING,
    };

    virtual ~IMtcCall() {};

    // Sets thread to interact with the Java layer. Nothing happens if the thread is null.
    virtual void Attach(IN JniMtcCallThread* pJniMtcCallThread,
            IN JniMediaSessionThread* pJniMediaThread) = 0;

    // Unsets thread to interact with the Java layer.
    virtual void Detach() = 0;

    // Starts an outgoing call.
    virtual void Start(
            IN CallType eCallType,
            IN const AString& strTarget,
            IN MediaInfo* pMediaInfo,
            IN const IMSMap<IMS_UINT32, SuppService*>& objSuppServices,
            IN JniMediaSessionThread* pJniMediaThread) = 0;

    virtual void StartConference(
            IN CallType eCallType,
            IN const AString& strTarget,
            IN MediaInfo* pMediaInfo,
            IN const IMSMap<IMS_UINT32, SuppService*>& objSuppServices,
            IN const IMSList<ConfUser*> objUsers) = 0;

    virtual void StartConference(
            IN CallType eCallType,
            IN const AString& strTarget,
            IN const IMSList<ConfUser*> objUsers) = 0;

    // TODO: deprecated
    virtual void ExpandToConference(IN CallInfo* pCallInfo, IN IMSList<ConfUser*> lstUsers) = 0;
    // TODO: deprecated
    virtual void MergeToConference(
            IN CallType eCallType, IN CallInfo* pCallInfo, IN IMSList<ConfUser*> lstUsers) = 0;

    // Handles an incoming call.
    virtual void HandleIncoming(
            IN ISession* piSession,
            IN JniMtcServiceThread* pServiceThread) = 0;

    // Notifies that the user alerting for this call is started.
    virtual void HandleUserAlert() = 0;

    // Accepts the incoming call.
    virtual void Accept(IN CallType eCallType, IN MediaInfo* pMediaInfo) = 0;

    // Rejects the incoming call.
    virtual void Reject(IN const FailReason& objReason) = 0;

    // Holds the call.
    virtual void Hold(IN MediaInfo* pMediaInfo) = 0;

    // Resumes the call.
    virtual void Resume(IN MediaInfo* pMediaInfo) = 0;

    // Accepts the resume request from the remote.
    virtual void AcceptResume(IN CallType eCallType, IN MediaInfo* pMediaInfo) = 0;

    // Rejects the resume request from the remote.
    virtual void RejectResume(IN const FailReason& objReason) = 0;

    // Requests call converting to the remote.
    virtual void Convert(IN CallType eCallType, IN MediaInfo* pMediaInfo) = 0;

    // Accepts the call converting request from the remote.
    virtual void AcceptConvert(IN CallType eCallType, IN MediaInfo* pMediaInfo) = 0;

    // Rejects the call converting request from the remote.
    virtual void RejectConvert(IN const FailReason& objReason) = 0;

    // Cancels the ongoing call converting request.
    virtual void CancelConvert(IN const FailReason& objReason) = 0;

    // Terminates the call.
    virtual void Terminate(IN const FailReason& objReason) = 0;

    // Sends DTMF to the remote.
    virtual void SendDtmf(IN const AString& strSignal, IN IMS_SINT32 nDuration) = 0;

    // Sends USSI. Does nothing if the call isn't a USSI call.
    virtual void SendUssi(IN const AString& strUssi) = 0;

    // Notifies that the SRVCC was successful.
    virtual void HandleSrvccSuccess() = 0;

    // Notifies that the SRVCC fails.
    virtual void HandleSrvccFailure(IN UpdateType eUpdateType) = 0;

    // Returns a key to uniquely identify this call.
    virtual CallKey GetKey() const = 0;

    /**
     * Returns the current call state.
     *
     * @return Current state.
     */
    virtual State GetState() const = 0;

    // TODO:
    virtual IMtcCallContext& GetCallContext() const = 0;
};

#endif
