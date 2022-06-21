/*
 * Copyright (C) 2022 The Android Open Source Project
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *      http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */
#ifndef BASE_SERVICE_H_
#define BASE_SERVICE_H_

#include <binder/Parcel.h>

#include "EnablerUtils.h"
#include "ImsMessage.h"
#include "ImsProcess.h"

typedef int (*Jni_SendDataToJava)(long nNativeObject, const android::Parcel& pParcel);
typedef int (*JniSystem_SendDataToJava)(
        long nNativeObject, const android::Parcel& in, android::Parcel& out, int fileDescriptor);

class BaseService : public ImsMessage::IMessageCallback
{
public:
    inline virtual ~BaseService() {}
    virtual int SendData(IN const android::Parcel& objParcel) = 0;
    inline virtual int SendData(IN const android::Parcel& /*in*/, android::Parcel& /*out*/)
    {
        return 0;
    }

protected:
    inline void MessageCallback_OnMessage(IN ImsMessage& objMsg) override
    {
        android::Parcel* pParcel = reinterpret_cast<android::Parcel*>(objMsg.nLparam);
        pParcel->setDataPosition(0);
        pParcel->readInt32();  // consumes nMsg

        HandleMessage(objMsg.nMSG, *pParcel);
        delete pParcel;
    }

    inline void SendDataUsingEnablerThread(
            IN const android::Parcel& objParcel, IN IMS_UINT32 nSlotId)
    {
        android::Parcel* pParcelOut = new android::Parcel();
        pParcelOut->write(objParcel.data(), objParcel.dataSize());
        pParcelOut->setDataPosition(0);

        ImsMessage objMsg(
                pParcelOut->readInt32(), 0, reinterpret_cast<IMS_UINTP>(pParcelOut), this);
        IThread* piThread = ImsProcess::GetInstance()
                                    ->GetThread(EnablerUtils::GetEnablerThreadName(nSlotId))
                                    ->GetThread();
        piThread->PostMessageI(objMsg);
    }

    inline virtual IMS_BOOL IsThreadSwitchingRequired(IN IMS_SINT32 /*nMsg*/) const
    {
        return IMS_TRUE;
    }

    inline virtual void HandleMessage(
            IN IMS_SINT32 /*nMsg*/, IN const android::Parcel& /*objParcel*/)
    {
        // TODO: this will be changed to pure virtual after all services implement this.
    }
};

#endif
