#ifndef MTC_UI_NOTIFIER_H_
#define MTC_UI_NOTIFIER_H_

#include "AString.h"
#include "IMSList.h"
#include "IMSTypeDef.h"
#include "MtcDef.h"
#include "define/MtcStringDef.h"
#include "call/IMtcCall.h"

class ParticipantInfo;
class JniMediaSessionThread;
class JniMtcCallThread;
class JniMtcServiceThread;
class MediaInfo;
class SuppService;
struct CallInfo;
struct FailReason;

class MtcUiNotifier final
{
public:
    MtcUiNotifier();
    ~MtcUiNotifier();
    MtcUiNotifier(IN const MtcUiNotifier&) = delete;
    MtcUiNotifier& operator=(IN const MtcUiNotifier&) = delete;

    // _JNI_MTC_
    inline void SetJniCallThread(IN JniMtcCallThread* pThread) { m_pCallThread = pThread; }
    inline void SetJniServiceThread(IN JniMtcServiceThread* pThread) { m_pServiceThread = pThread; }
    inline void SetJniMediaThread(IN JniMediaSessionThread* pThread) { m_pMediaThread = pThread; }
    inline JniMediaSessionThread* GetJniMediaThread() { return m_pMediaThread; }

    void SendIncomingCallReceived(
            IN CallKey nKey,
            IN CallInfo& objCallInfo,
            IN MediaInfo& objMediaInfo,
            IN const IMSMap<SuppType, SuppService*>& objSuppServices,
            IN ParticipantInfo& objParticipantInfo);
    void SendStarted(IN CallInfo* pCallInfo, IN MediaInfo* pMediaInfo,
            IN const IMSMap<SuppType, SuppService*>& objSuppServices);
    void SendStartFailed(IN const FailReason& objReason);
    void SendProgressing(IN CallInfo* pCallInfo, IN MediaInfo* pMediaInfo,
            IN const IMSMap<SuppType, SuppService*>& objSuppServices,
            IN IMS_BOOL bAlerted = IMS_FALSE);
    void SendHeld(IN CallInfo* pCallInfo, IN MediaInfo* pMediaInfo,
            IN const IMSMap<SuppType, SuppService*>& objSuppServices);
    void SendHoldFailed(IN const FailReason& objReason);
    void SendResumed(IN CallInfo* pCallInfo, IN MediaInfo* pMediaInfo,
            IN const IMSMap<SuppType, SuppService*>& objSuppServices);
    void SendResumeFailed(IN const FailReason& objReason);
    void SendHeldBy(IN CallInfo* pCallInfo, IN MediaInfo* pMediaInfo,
            IN const IMSMap<SuppType, SuppService*>& objSuppServices);
    void SendResumedBy(IN CallInfo* pCallInfo, IN MediaInfo* pMediaInfo,
            IN const IMSMap<SuppType, SuppService*>& objSuppServices);
    void SendTerminated(IN const FailReason& objReason);
    void SendIncomingResume(IN CallInfo* pCallInfo, IN MediaInfo* pMediaInfo,
            IN const IMSMap<SuppType, SuppService*>& objSuppServices);
    void SendIncomingUpdate(IN CallInfo* pCallInfo, IN MediaInfo* pMediaInfo,
            IN const IMSMap<SuppType, SuppService*>& objSuppServices);
    void SendUpdated(IN CallInfo* pCallInfo, IN MediaInfo* pMediaInfo,
            IN const IMSMap<SuppType, SuppService*>& objSuppServices);
    void SendUpdateFailed(IN const FailReason& objReason);
    void SendUpdatedBy(IN CallInfo* pCallInfo, IN MediaInfo* pMediaInfo,
            IN const IMSMap<SuppType, SuppService*>& objSuppServices);
    void SendNotifyInfo(IN IMS_UINT32 eType, IN IMS_UINTP nImsKey,
            IN AString strValue = AString::ConstNull(),
            IN IMS_SINT32 nValue = -1, IN IMS_BOOL bValue = IMS_FALSE);
    void SendExpanded(IN CallInfo* pCallInfo, IN MediaInfo* pMediaInfo,
            IN const IMSMap<SuppType, SuppService*>& objSuppServices);
    void SendExpandFailed(IN const FailReason& objReason);
    void SendExpandedBy(IN CallInfo* pCallInfo, IN MediaInfo* pMediaInfo,
            IN const IMSMap<SuppType, SuppService*>& objSuppServices,
            IN IMS_SINTP nReplaceKey = 0);
    void SendMerged(IN CallInfo* pCallInfo, IN MediaInfo* pMediaInfo,
            IN const IMSMap<SuppType, SuppService*>& objSuppServices,
            IN IMSList<ConfUser*> lstConfUser);
    void SendMergeFailed(IN const FailReason& objReason);
    void SendJoined(IN IMS_BOOL bResult, IN const FailReason& objReason);
    void SendDropped(IN IMS_BOOL bResult, IN const FailReason& objReason);
    void SendNotifyUsersInfo(IN IMSList<ConfUser*> lstConfUser);
    void SendNotifyConfInfo(IN AString strDisplayText, IN AString strSubject,
            IN IMS_SINT32 nMaxUserCount, IN IMS_UINT32 nUserCount, IN AString strHostEntity);
    void SendReplacedBy(IN CallInfo* pCallInfo, IN MediaInfo* pMediaInfo,
            IN const IMSMap<SuppType, SuppService*>& objSuppServices,
            IN IMS_SINTP nReplaceKey = 0, IN IMS_UINTP nType = 0);
    void SendEctCompleted(IN IMS_RESULT nResult, IN const FailReason& objReason);
    void SendCallPushCompleted(IN IMS_BOOL bResult, IN const FailReason& objReason);

private:
    IMS_BOOL IsAvailableToSend();

    JniMtcCallThread* m_pCallThread;
    JniMtcServiceThread* m_pServiceThread;
    JniMediaSessionThread* m_pMediaThread;
};

#endif
