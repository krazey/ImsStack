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
#include "IJniEnabler.h"

class IJniEnablerThread;

typedef int (*Jni_SendDataToJava)(long nNativeObject, const android::Parcel& pParcel);
typedef int (*JniSystem_SendDataToJava)(
        long nNativeObject, const android::Parcel& in, android::Parcel& out, int fileDescriptor);

class BaseService : public IJniEnabler
{
public:
    BaseService(IN IMS_SINT32 nSlotId);
    virtual ~BaseService();
    virtual void Destroy();
    virtual int SendData(IN const android::Parcel& objParcel) = 0;
    inline virtual int SendData(IN const android::Parcel& /*in*/, android::Parcel& /*out*/)
    {
        return 0;
    }

    inline void NotifyNativeEnablerSet() override {}
    inline IJniEnablerThread* GetJniThread() const override { return IMS_NULL; }
    inline IMS_SINT32 GetSlotId() const { return m_nSlotId; }

protected:
    void MessageCallback_OnMessage(IN ImsMessage& objMsg) override;
    IMS_UINT32 RemovePendingMessages();
    void SendDataUsingEnablerThread(IN const android::Parcel& objParcel);

    inline virtual IMS_BOOL IsThreadSwitchingRequired(IN IMS_SINT32 /*nMsg*/) const
    {
        return IMS_TRUE;
    }

    inline virtual void HandleMessage(
            IN IMS_SINT32 /*nMsg*/, IN const android::Parcel& /*objParcel*/)
    {
        // TODO: this will be changed to pure virtual after all services implement this.
    }

    static const IMS_SINT32 MSG_DESTROY = -1;
    IMS_SINT32 m_nSlotId;
};

#endif
