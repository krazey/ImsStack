#ifndef JNI_MTC_SERVICE_THREAD_H_
#define JNI_MTC_SERVICE_THREAD_H_

#include "BaseServiceThread.h"
#include "MtcDef.h"
#include "IMSMap.h"

class ParticipantInfo;

class JniMtcServiceThread final : public BaseServiceThread
{
public:
    JniMtcServiceThread();
    virtual ~JniMtcServiceThread();

    inline void SetSlotId(IN IMS_SINT32 nSlotId) { m_nSlotId = nSlotId; }
    // void SetCallback(IN IMS_SINTP nNativeObj, IN Jni_SendDataToJava pfnSendDataToJava);

    void OnServiceChanged(IN IMS_SINT32 eStatus, IN IMS_SINT32 eReason);  // enum class
    void OnEmergencyServiceChanged(IN IMS_SINT32 eStatus, IN IMS_SINT32 eReason); // enum class
    void OnPreIncomingCallReceived(IN IMS_ULONG nCallKey);

private:
    IMS_SINT32 m_nSlotId;
};

#endif
