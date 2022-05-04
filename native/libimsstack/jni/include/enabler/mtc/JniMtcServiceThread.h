#ifndef JNI_MTC_SERVICE_THREAD_H_
#define JNI_MTC_SERVICE_THREAD_H_

#include "BaseServiceThread.h"
#include "MtcDef.h"
#include "IMSMap.h"

class ParticipantInfo;
struct CallInfo;

class JniMtcServiceThread final : public BaseServiceThread
{
public:
    JniMtcServiceThread();
    virtual ~JniMtcServiceThread();

    inline void SetSlotId(IN IMS_SINT32 nSlotId) { m_nSlotId = nSlotId; }
    // void SetCallback(IN IMS_SINTP nNativeObj, IN CBServiceNoti pfnNotifier);

    void OnServiceChanged(IN IMS_SINT32 eStatus, IN IMS_SINT32 eReason);  // enum class
    void OnIncomingCallReceived(IN IMS_UINTP nCallKey, IN CallInfo* pCallInfo,
            IN MediaInfo* pMediaInfo, IN const IMSMap<SuppType, SuppService*>& objSuppServices,
            IN ParticipantInfo* pParticipantInfo);

private:
    IMS_SINT32 m_nSlotId;
};

#endif
