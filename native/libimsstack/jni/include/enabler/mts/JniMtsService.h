#ifndef JNI_MTS_SERVICE_H_
#define JNI_MTS_SERVICE_H_

#include "BaseService.h"
#include "JniMtsServiceThread.h"

class IMtsService;

class JniMtsService : public BaseService
{
public:
    JniMtsService(IN CBServiceNoti pCbServiceNoti, IN IMS_SINT32 nSlotId);
    virtual ~JniMtsService();
    virtual int SendData(const android::Parcel& objParcel) override;
    void SetMtsService(IN IMtsService* piMtsService);
    JniMtsServiceThread* GetThread() const;

protected:
    void HandleMessage(IN IMS_SINT32 nMsg, IN const android::Parcel& objParcel) override;

private:
    IMS_BOOL Attach();
    void Initialize(IN CBServiceNoti pCbServiceNoti);
    void TriggerSendMoSms(IN const android::Parcel& objParcel);
    void NotifyMtResult(IN const android::Parcel& objParcel);

private:
    IMS_SINT32 m_nSlotId;
    AString m_strThreadName;
    IMtsService* m_piMtsService;
    JniMtsServiceThread* m_pJniMtsServiceThread;
};

#endif
