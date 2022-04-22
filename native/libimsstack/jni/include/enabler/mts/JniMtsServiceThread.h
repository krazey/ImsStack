#ifndef JNI_MTS_SERVICE_THREAD_H_
#define JNI_MTS_SERVICE_THREAD_H_

#include "BaseService.h"
#include "IMSAppThread.h"

class JniMtsServiceThread :
        public IMSAppThread
{
private:
    JniMtsServiceThread();

public:
    static IMSAppThread* GetInstance();
    virtual ~JniMtsServiceThread();

    IMS_SINT32 SetCallback(
            IN IMS_SINTP nNativeObj, IN CBServiceNoti pfnNotifier, IN IMS_SINT32 nSlotId = 0);
    IMS_SINT32 GetSimSlot();

protected:
    virtual IMS_BOOL Initialize();
    virtual void Uninitialize();

    virtual IMS_BOOL OnStart(IN IMSMSG& objMSG);
    virtual IMS_BOOL OnTerminate(IN IMSMSG& objMSG);
    virtual IMS_BOOL OnMessage(IN IMSMSG& objMSG);

private:
    IMS_UINTP       m_nNativeObj;
    CBServiceNoti   m_pfnNotifier;
    IMS_SINT32      m_nSlotId;
};

#endif
