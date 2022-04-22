/*
    Author
    <table>
    date      author                    description
    --------  --------------            ----------
    20110309  joonhun.shin@             Created
    </table>

    Description

*/

#ifndef _BASE_SERVICE_H_
#define _BASE_SERVICE_H_

#include "EnablerUtils.h"
#include "ImsMessage.h"
#include "IMSProcess.h"
#include <binder/Parcel.h>

#define IMS_STL_USE

typedef int (*CBServiceNoti)(long nNativeObj, const android::Parcel& pParcel);
typedef int (*JniSystem_SendDataToJava)(long nNativeObject,
        const android::Parcel& in, android::Parcel& out, int fileDescriptor);

class BaseService :
        public IMSMSG::IMessageCallback
{
public:
    inline virtual ~BaseService()
    {}
    virtual int SendData(IN const android::Parcel& objParcel) = 0;
    inline virtual int SendData(IN const android::Parcel& /* parcelIn */,
            android::Parcel& /* parcelOut */)
    {
        return 0;
    }

protected:
    inline virtual void MessageCallback_OnMessage(IN IMSMSG& objMsg) override
    {
        android::Parcel* pParcel = reinterpret_cast<android::Parcel*>(objMsg.nLparam);
        pParcel->setDataPosition(0);
        pParcel->readInt32(); // consumes nMsg

        HandleMessage(objMsg.nMSG, *pParcel);
        delete pParcel;
    }

    inline void SendDataUsingEnablerThread(IN const android::Parcel& objParcel,
            IN IMS_UINT32 nSlotId)
    {
        android::Parcel* pParcelOut = new android::Parcel();
        pParcelOut->write(objParcel.data(), objParcel.dataSize());
        pParcelOut->setDataPosition(0);

        IMSMSG objMsg(pParcelOut->readInt32(), 0, reinterpret_cast<IMS_UINTP>(pParcelOut), this);
        IThread* piThread = IMSProcess::GetInstance()->
                GetThread(EnablerUtils::GetEnablerThreadName(nSlotId))->GetThread();
        piThread->PostMessageI(objMsg);
    }

    inline virtual IMS_BOOL IsThreadSwitchingRequired(IN IMS_SINT32 /*nMsg*/) const
    {
        return IMS_TRUE;
    }

    inline virtual void HandleMessage(IN IMS_SINT32 /*nMsg*/,
            IN const android::Parcel& /*objParcel*/)
    {
        // TODO: this will be changed to pure virtual after all services implement this.
    }
};

#endif //_BASE_SERVICE_H_
