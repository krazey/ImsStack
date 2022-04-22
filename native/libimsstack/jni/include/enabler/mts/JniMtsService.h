#ifndef JNI_MTS_SERVICE_H_
#define JNI_MTS_SERVICE_H_

#include "BaseService.h"

class JniMtsServiceThread;

using namespace android;

class JniMtsService :
        public BaseService
{
    public:
        JniMtsService(IN IMS_SINT32 nSlotId = 0);
        JniMtsService(IN CBServiceNoti pCBServiceNoti, IN IMS_SINT32 nSlotId = 0);
        virtual ~JniMtsService();

        virtual int SendData(IN const Parcel& objParcel) override;

    private:
        void HandleMessage(IN IMS_SINT32 nMsg, IN const Parcel& objParcel) override;

    private:
        JniMtsServiceThread*    m_pJniMtsServiceThread;
        IMS_SINT32              m_nSlotId;
        AString                 m_strTargetActivity;
        AString                 m_strThreadName;
};

#endif
