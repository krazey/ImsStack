#ifndef JNI_MTC_CALL_THREAD_H_
#define JNI_MTC_CALL_THREAD_H_

#include "BaseService.h"
#include "BaseServiceThread.h"
#include "call/ParticipantInfo.h"
#include "MtcDef.h"
#include <binder/Parcel.h>
#include "IMSMap.h"

struct JniCallInfo;
struct FailReason;
class MediaInfo;
class SuppService;
class ConfUser;

class JniMtcCallThread final : public BaseServiceThread
{
public:
    JniMtcCallThread();
    virtual ~JniMtcCallThread();

    inline void SetSlotId(IN IMS_SINT32 nSlotId) { m_nSlotId = nSlotId; }

    void OnStarted(IN const JniCallInfo& objCallInfo, IN MediaInfo* pMediaInfo,
            IN const IMSMap<SuppType, SuppService*>& objSuppServices);
    void OnStartFailed(IN const FailReason& objReason);
    void OnProgressing(IN const JniCallInfo& objCallInfo, IN MediaInfo* pMediaInfo,
            IN const IMSMap<SuppType, SuppService*>& objSuppServices,
            IN IMS_BOOL bAlerted = IMS_FALSE);
    void OnHeld(IN const JniCallInfo& objCallInfo, IN MediaInfo* pMediaInfo,
            IN const IMSMap<SuppType, SuppService*>& objSuppServices);
    void OnHoldFailed(IN const FailReason& objReason);
    void OnResumed(IN const JniCallInfo& objCallInfo, IN MediaInfo* pMediaInfo,
            IN const IMSMap<SuppType, SuppService*>& objSuppServices);
    void OnResumeFailed(IN const FailReason& objReason);
    void OnHeldBy(IN const JniCallInfo& objCallInfo, IN MediaInfo* pMediaInfo,
            IN const IMSMap<SuppType, SuppService*>& objSuppServices);
    void OnResumedBy(IN const JniCallInfo& objCallInfo, IN MediaInfo* pMediaInfo,
            IN const IMSMap<SuppType, SuppService*>& objSuppServices);
    void OnTerminated(IN const FailReason& objReason);
    void OnIncomingResume(IN const JniCallInfo& objCallInfo, IN MediaInfo* pMediaInfo,
            IN const IMSMap<SuppType, SuppService*>& objSuppServices);
    void OnIncomingUpdate(IN const JniCallInfo& objCallInfo, IN MediaInfo* pMediaInfo,
            IN const IMSMap<SuppType, SuppService*>& objSuppServices);
    void OnUpdated(IN const JniCallInfo& objCallInfo, IN MediaInfo* pMediaInfo,
            IN const IMSMap<SuppType, SuppService*>& objSuppServices);
    void OnUpdateFailed(IN const FailReason& objReason);
    void OnUpdatedBy(IN const JniCallInfo& objCallInfo, IN MediaInfo* pMediaInfo,
            IN const IMSMap<SuppType, SuppService*>& objSuppServices);

    void OnMerged(IN const JniCallInfo& objCallInfo, IN MediaInfo* pMediaInfo,
            IN const IMSMap<SuppType, SuppService*>& objSuppServices,
            IN const IMSList<ConfUser*>& objUsers);
    void OnMergeFailed(IN const FailReason& objReason);
    void OnConferenceParticipantAdded();
    void OnConferenceParticipantAddFailed(IN const FailReason& objReason);
    void OnConferenceParticipantRemoved();
    void OnConferenceParticipantRemoveFailed(IN const FailReason& objReason);
    void OnConferenceInfoChanged(IN const AString& strDisplayText, IN const AString strSubject,
            IN IMS_UINT32 nUserCount, IN IMS_UINT32 nMaxUserCount, IN const AString& strHost);
    void OnConferenceParticipantsInfoChanged(IN const IMSList<ConfUser*>& objUsers);

    void OnEctCompleted(IN IMS_RESULT nResult, IN const FailReason& objReason);

    void OnIncomingCallReceived(IN IMS_UINTP nCallKey, IN const JniCallInfo& objCallInfo,
            IN MediaInfo* pMediaInfo, IN const IMSMap<SuppType, SuppService*>& objSuppServices,
            IN ParticipantInfo* pParticipantInfo);

private:
    void SetCallDetails(IN_OUT android::Parcel& objParcel, IN const JniCallInfo& objCallInfo,
            IN MediaInfo* pMediaInfo, IN const IMSMap<SuppType, SuppService*>& objSuppServices);

private:
    IMS_SINT32 m_nSlotId;
};

#endif
