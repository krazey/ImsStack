#ifndef JNI_MTC_CALL_H_
#define JNI_MTC_CALL_H_

#include "BaseService.h"
#include "JniMediaSession.h"
#include "JniMtcCallThread.h"
#include "IuMtcCall.h"
#include "call/IMtcCall.h"
#include "IMSMap.h"

class MtcCallController;

class JniMtcCall final :
        public BaseService
{
public:
    JniMtcCall(IN CBServiceNoti pfnNotifier, IN IMS_SINTP nCallKey = -1, IN IMS_SINT32 nSlotId = 0);
    virtual ~JniMtcCall();

    virtual IMS_SINT32 SendData(IN const android::Parcel& objParcel) override;
    void Initialize();

    JniMtcCallThread* GetThread();

protected:
    virtual IMS_BOOL IsThreadSwitchingRequired(IN IMS_SINT32 nMsg) const override;
    void HandleMessage(IN IMS_SINT32 nMsg, IN const android::Parcel& objParcel) override;

private:
    void Attach();
    void Open(IN const android::Parcel& objParcel);
    void Start(IN const android::Parcel& objParcel);
    void OnUserAlert(IN const android::Parcel& objParcel); //naming...
    void Accept(IN const android::Parcel& objParcel);
    void Reject(IN const android::Parcel& objParcel);
    void Hold(IN const android::Parcel& objParcel);
    void Resume(IN const android::Parcel& objParcel);
    void Terminate(IN const android::Parcel& objParcel);
    void Update(IN const android::Parcel& objParcel);
    void AcceptUpdate(IN const android::Parcel& objParcel);
    void RejectUpdate(IN const android::Parcel& objParcel);
    void CancelUpdate(IN const android::Parcel& objParcel);
    void AcceptResume(IN const android::Parcel& objParcel);
    void RejectResume(IN const android::Parcel& objParcel);

    void StartGroupCall(IN const android::Parcel& objParcel);
    void MergeToConference(IN const android::Parcel& objParcel);
    void ExpandToConference(IN const android::Parcel& objParcel);
    void AddToConference(IN const android::Parcel& objParcel);
    void RemoveFromConference(IN const android::Parcel& objParcel);

    void Transfer();
    void TransferWithNumber(IN const android::Parcel& objParcel);

private:
    JniMtcCallThread* m_pThread;
    CBServiceNoti m_pfnNotifier;
    AString m_strThreadName;
    IMS_SINT32 m_nSlotId;
    MtcCallController& m_objCallController;
    CallKey m_nCallKey;
    JniMediaSession* m_pJniMediaSession;
};
#endif
