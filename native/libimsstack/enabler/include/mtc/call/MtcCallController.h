#ifndef MTC_CALL_CONTROLLER_H_
#define MTC_CALL_CONTROLLER_H_

#include "IMSTypeDef.h"
#include "IuMtcCall.h"
#include "IuMtcService.h"
#include "IMtcCall.h"
#include "CallInfo.h"
#include "IMtcService.h"

class IMtcCallManager;
class IMtcContext;
class ISession;
class JniMediaSessionThread;
class JniMtcCallThread;
class JniMtcServiceThread;
enum class KeyType;
union Key;

/**
 * Provides operations to manipulate calls. Each operation could be failed or not handled if the
 * current status is not applicable or some other reasons.
 */
class MtcCallController final
{
public:
    MtcCallController(IN IMtcContext& objContext);
    ~MtcCallController();
    MtcCallController(IN const MtcCallController&) = delete;
    MtcCallController& operator=(IN const MtcCallController&) = delete;

    /**
     * Terminates the calls that matches the given key.
     *
     * @param eKeyType All existing calls are affected if `NONE`.
     * @param nKey Key to find the calls.
     * @param objReason The calls can use this information for terminating.
     */
    void TerminateCalls(IN KeyType eKeyType, IN Key nKey, IN const FailReason& objReason);

    /**
     * Removes the calls that matches the given key without terminating behavior.
     *
     * @param eKeyType All existing calls are affected if `NONE`.
     * @param nKey Key to find the calls.
     */
    void RemoveCalls(IN KeyType eKeyType, IN Key nKey);

    /**
     * Creates a new outgoing call.
     *
     * @param eServiceType Service type of the new call.
     * @return The key of the new call.
     */
    CallKey Open(IN ServiceType eServiceType, IN CallInfo& objCallInfo);

    /**
     * Sets an interface to interact with the Java layer.
     *
     * @param nCallKey Key of the call to be manipulated.
     * @param pJniMtcCallThread Interface to send messages to the Java layer.
     */
    void Attach(IN CallKey nCallKey, IN JniMtcCallThread* pJniMtcCallThread,
            IN JniMediaSessionThread* pJniMediaThread);

    /**
     * Disconnects from the interface to interact with the Java layer.
     *
     * @param nCallKey Key of the call to be manipulated.
     */
    void Detach(IN CallKey nCallKey);

    /**
     * Creates a call to handle the incoming call.
     *
     * @param pService Service of the incoming call.
     * @param piSession Session of the incoming call.
     * @param pServiceThread JNI to be notified the incoming call.
     */
    void HandleIncoming(IN IMtcService* pService, IN ISession* piSession,
            IN JniMtcServiceThread* pServiceThread);

    /**
     * Sets the call information to start the outgoing call.
     *
     * @param nCallKey Key of the call to be manipulated.
     * @param eCallType Type of the call.
     * @param strTarget Remote target.
     * @param pMediaInfo Media of the call.
     * @param lstSuppServices Supplementary services.
     * @param pDialog TODO:
     */
    void Start(IN CallKey nCallKey, IN CallType eCallType, IN const AString& strTarget,
            IN MediaInfo* pMediaInfo, IN const IMSMap<IMS_UINT32, SuppService*>& objSuppServices,
            IN IDialogEvent* pDialog);

    /**
     * Notifies the call that the user is alerted by the incoming call.
     *
     * @param nCallKey Key of the call to be manipulated.
     */
    void HandleUserAlert(IN CallKey nCallKey);

    // Accepts an incoming call.
    // - nIMSKey: Key of the call to be manipulated.
    void Accept(IN CallKey nCallKey, IN CallType eCallType, IN MediaInfo* pMediaInfo);

    // Rejects an incoming call.
    // - nIMSKey: Key of the call to be manipulated.
    // - eSessionType: See MtcDef::SESSIONTYPE_*
    // - pMediaInfo: `MediaInfo`
    void Reject(IN CallKey nCallKey, IN const FailReason& objReason);

    // Holds a call.
    // - nIMSKey: Key of the call to be manipulated.
    // - pMediaInfo: `MediaInfo`
    void Hold(IN CallKey nCallKey, IN MediaInfo* pMediaInfo);

    // Resumes a call.
    // - nIMSKey: Key of the call to be manipulated.
    // - pMediaInfo: `MediaInfo`
    void Resume(IN CallKey nCallKey, IN MediaInfo* pMediaInfo);

    // Accepts the resume request from the remote.
    // - nIMSKey: Key of the call to be manipulated.
    // - eSessionType: See MtcDef::SESSIONTYPE_*
    // - pMediaInfo: `MediaInfo`
    void AcceptResume(IN CallKey nCallKey, IN CallType eCallType, IN MediaInfo* pMediaInfo);

    // Rejects the resume request from the remote.
    // - nIMSKey: Key of the call to be manipulated.
    // - failReason: Rejected reason.
    void RejectResume(IN CallKey nCallKey, IN const FailReason& objReason);

    // Terminates a call.
    // - nIMSKey: Key of the call to be manipulated.
    // - failReason: Terminate reason.
    void Terminate(IN CallKey nCallKey, IN const FailReason& objReason);

    // Modifies media parameters of a call.
    // - nIMSKey: Key of the call to be manipulated.
    // - eSessionType: See MtcDef::SESSIONTYPE_*
    // - pMediaInfo: `MediaInfo`
    void Update(IN CallKey nCallKey, IN CallType eCallType, IN MediaInfo* pMediaInfo);

    // Cancels the ongoing modification request.
    // - nIMSKey: Key of the call to be manipulated.
    // - failReason: Canceled reason.
    void CancelUpdate(IN CallKey nCallKey, IN const FailReason& objReason);

    // Accepts the modification request from the remote.
    // - nIMSKey: Key of the call to be manipulated.
    // - eSessionType: See MtcDef::SESSIONTYPE_*
    // - pMediaInfo: `MediaInfo`
    void AcceptUpdate(IN CallKey nCallKey, IN CallType eCallType, IN MediaInfo* pMediaInfo);

    // Rejects the modification request from the remote.
    // - nIMSKey: Key of the call to be manipulated.
    // - failReason: Rejected reason.
    void RejectUpdate(IN CallKey nCallKey, IN const FailReason& objReason);

    // Sends USSI. Nothing happens if the specified call isn't a USSI session.
    // - nIMSKey: Key of the call to be manipulated.
    // - aStrUSSI: USSI string.
    void SendTransaction(IN CallKey nCallKey, IN const AString& strUssi);

    // Handles conference call related IMS messages.
    /*
    void StartGroupCall(IN CallKey nCallKey, IN IMS_UINT32 nCmd, IN IMSList<ConfUser*>& objUsers,
            IN CallInfo& objCallInfo, IN MediaInfo& objMediaInfo,
            IN IMSMap<IMS_UINT32, SuppService*>& objSuppServices);
    */

    void MergeToConference(IN CallKey nCallKey, IN IMSList<ConfUser*>& objUsers);
    void AddToConference(IN CallKey nCallKey, IN IMSList<ConfUser*>& objUsers);
    void RemoveFromConference(IN CallKey nCallKey, IN IMSList<ConfUser*>& objUsers);

    // TODO: Consider ECT, SRVCC

private:
    IMS_BOOL IsUssi(IN ISession* piSession);
    IMS_BOOL IsEct(IN ISession* piSession);

    IMtcContext& m_objContext;
    IMtcCallManager& m_objCallManager;
};

enum class KeyType
{
    NONE,
    CALL_KEY,
    CALL_TYPE,
    SERVICE_TYPE
};

union Key
{
    CallKey nCallKey;
    CallType eCallType;
    ServiceType eServiceType;
};

#endif
