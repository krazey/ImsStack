#ifndef JNI_MTC_CALL_THREAD_H_
#define JNI_MTC_CALL_THREAD_H_

#include "BaseService.h"
#include "BaseServiceThread.h"
#include "MtcDef.h"
#include <binder/Parcel.h>
#include "IMSMap.h"

struct CallInfo;
struct FailReason;
class MediaInfo;
class SuppService;
class ConfUser;

class JniMtcCallThread final :
        public BaseServiceThread
{
public:
    JniMtcCallThread();
    virtual ~JniMtcCallThread();

    inline void SetSlotId(IN IMS_SINT32 nSlotId) { m_nSlotId = nSlotId; }

    void OnStarted(IN CallInfo* pCallInfo, IN MediaInfo* pMediaInfo,
            IN const IMSMap<IMS_UINT32, SuppService*>& objSuppServices);
    void OnStartFailed(IN const FailReason& objReason);
    void OnProgressing(IN CallInfo* pCallInfo, IN MediaInfo* pMediaInfo,
            IN const IMSMap<IMS_UINT32, SuppService*>& objSuppServices,
            IN IMS_BOOL bAlerted = IMS_FALSE);
    void OnHeld(IN CallInfo* pCallInfo, IN MediaInfo* pMediaInfo,
            IN const IMSMap<IMS_UINT32, SuppService*>& objSuppServices);
    void OnHoldFailed(IN const FailReason& objReason);
    void OnResumed(IN CallInfo* pCallInfo, IN MediaInfo* pMediaInfo,
            IN const IMSMap<IMS_UINT32, SuppService*>& objSuppServices);
    void OnResumeFailed(IN const FailReason& objReason);
    void OnHeldBy(IN CallInfo* pCallInfo, IN MediaInfo* pMediaInfo,
            IN const IMSMap<IMS_UINT32, SuppService*>& objSuppServices);
    void OnResumedBy(IN CallInfo* pCallInfo, IN MediaInfo* pMediaInfo,
            IN const IMSMap<IMS_UINT32, SuppService*>& objSuppServices);
    void OnTerminated(IN const FailReason& objReason);
    void OnIncomingResume(IN CallInfo* pCallInfo, IN MediaInfo* pMediaInfo,
            IN const IMSMap<IMS_UINT32, SuppService*>& objSuppServices);
    void OnIncomingUpdate(IN CallInfo* pCallInfo, IN MediaInfo* pMediaInfo,
            IN const IMSMap<IMS_UINT32, SuppService*>& objSuppServices);
    void OnUpdated(IN CallInfo* pCallInfo, IN MediaInfo* pMediaInfo,
            IN const IMSMap<IMS_UINT32, SuppService*>& objSuppServices);
    void OnUpdateFailed(IN const FailReason& objReason);
    void OnUpdatedBy(IN CallInfo* pCallInfo, IN MediaInfo* pMediaInfo,
            IN const IMSMap<IMS_UINT32, SuppService*>& objSuppServices);

    void OnMerged(IN CallInfo* pCallInfo, IN MediaInfo* pMediaInfo,
            IN const IMSMap<IMS_UINT32, SuppService*>& objSuppServices,
            IN const IMSList<ConfUser*>& objUsers);
    void OnMergeFailed(IN const FailReason& objReason);
    void OnConferenceParticipantAdded();
    void OnConferenceParticipantAddFailed(IN const FailReason& objReason);
    void OnConferenceParticipantRemoved();
    void OnConferenceParticipantRemoveFailed(IN const FailReason& objReason);
    void OnConferenceInfoChanged(IN const AString& strDisplayText, IN const AString strSubject,
            IN IMS_UINT32 nUserCount, IN IMS_UINT32 nMaxUserCount,
            IN const AString& strHost);
    void OnConferenceParticipantsInfoChanged(IN const IMSList<ConfUser*>& objUsers);

private:
    void SetCallDetails(IN_OUT android::Parcel& objParcel, IN CallInfo* pCallInfo,
            IN MediaInfo* pMediaInfo, IN const IMSMap<IMS_UINT32, SuppService*>& objSuppServices);

private:
    IMS_SINT32 m_nSlotId;
};

#endif
