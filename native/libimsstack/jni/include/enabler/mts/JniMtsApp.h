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

#ifndef JNI_MTS_APP_H_
#define JNI_MTS_APP_H_

#include "BaseService.h"

class IJniEnablerThread;
class IMtsJni;
class JniMtsAppThread;

class JniMtsApp : public BaseService
{
public:
    JniMtsApp(IN Jni_SendDataToJava pfnSendDataToJava, IN IMS_SINT32 nSlotId);
    virtual ~JniMtsApp() override;
    virtual int SendData(const android::Parcel& objParcel) override;

    IJniEnablerThread* GetJniThread() const override;

protected:
    void HandleMessage(IN IMS_SINT32 nMsg, IN const android::Parcel& objParcel) override;

private:
    void Attach();
    IMtsJni* GetNativeApp();
    void Initialize(IN Jni_SendDataToJava pfnSendDataToJava);
    void TriggerSendMoSms(IN const android::Parcel& objParcel);
    void TriggerNotifyMoSmsTimedOut();

private:
    JniMtsAppThread* m_pJniMtsAppThread;
};

#endif
