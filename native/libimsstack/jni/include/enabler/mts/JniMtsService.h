#ifndef JNI_MTS_SERVICE_H_
#define JNI_MTS_SERVICE_H_

#include "BaseService.h"

class IJniEnablerThread;
class IMtsService;
class JniMtsServiceThread;

class JniMtsService : public BaseService
{
public:
    JniMtsService(IN Jni_SendDataToJava pfnSendDataToJava, IN IMS_SINT32 nSlotId);
    virtual ~JniMtsService();
    virtual int SendData(const android::Parcel& objParcel) override;

    inline void NotifyNativeEnablerSet() override {}
    IJniEnablerThread* GetJniThread() const override;

protected:
    void HandleMessage(IN IMS_SINT32 nMsg, IN const android::Parcel& objParcel) override;

private:
    void Attach();
    IMtsService* GetNativeService();
    void Initialize(IN Jni_SendDataToJava pfnSendDataToJava);
    void TriggerSendMoSms(IN const android::Parcel& objParcel);
    void NotifyMtResult(IN const android::Parcel& objParcel);
    void NotifyScbmState(IN const android::Parcel& objParcel);

private:
    IMS_SINT32 m_nSlotId;
    JniMtsServiceThread* m_pJniMtsServiceThread;
};

#endif
