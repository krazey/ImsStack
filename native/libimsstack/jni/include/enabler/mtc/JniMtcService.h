#ifndef JNI_MTC_SERVICE_H_
#define JNI_MTC_SERVICE_H_

#include "BaseService.h"
#include "IMSTypeDef.h"
#include "JniMtcServiceThread.h"

class IMtcService;

class JniMtcService : public BaseService
{
public:
    JniMtcService(IN CBServiceNoti pfnNotifier, IN IMS_SINT32 nSlotId);
    virtual ~JniMtcService();

    virtual int SendData(const android::Parcel& objParcel) override;

    void Initialize(IN CBServiceNoti pfnNotifier);

    void SetMtcService(IN IMtcService* piMtcService);

    JniMtcServiceThread* GetThread();

protected:
    void HandleMessage(IN IMS_SINT32 nMsg, IN const android::Parcel& objParcel) override;

private:
    void Attach();
    void NotifySrvccStateChanged(IN const android::Parcel& objParcel);
    void SetTerminalBasedCallWaiting(IN const android::Parcel& objParcel);
    void OpenEmergencyService(IN const android::Parcel& objParcel);

private:
    JniMtcServiceThread* m_pThread;
    AString m_strThreadName;
    IMS_SINT32 m_nSlotId;
    IMtcService* m_piMtcService;
};

#endif
