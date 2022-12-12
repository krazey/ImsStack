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

#ifndef _IMS_UCE_SERVICE_H_
#define _IMS_UCE_SERVICE_H_

#include "BaseService.h"

class JniUceServiceThread;
class IJniEnablerThread;
class IUceJni;

using namespace android;

class JniUceService : public BaseService
{
public:
    explicit JniUceService(Jni_SendDataToJava pfnSendDataToJava, IN IMS_UINT32 nSimSlot = 0);
    virtual ~JniUceService();

    IJniEnablerThread* GetJniThread() const override;
    virtual int SendData(const Parcel& pParcel) override;

private:
    IUceJni* GetNativeService();
    void HandleMessage(int nMsg, const Parcel& pParcel) override;
    static void SendPublishCmd(IUceJni* pJniUce, const Parcel& pParcel);
    static void SendSingleSubscribeCmd(IUceJni* pJniUce, const Parcel& pParcel);
    static void SendListSubscribeCmd(IUceJni* pJniUce, const Parcel& pParcel);
    static void SendOptionsCmd(IUceJni* pJniUce, const Parcel& pParcel);
    static void SendOptionsRespCmd(IUceJni* pJniUce, const Parcel& pParcel);
    static void ImsRegistrationCheck(IUceJni* pJniUce);

private:
    JniUceServiceThread* m_pJniUceServiceThread;
    AString m_strTarget;
};

#endif  //_IMS_PEOPLE_SERVICE_H_
