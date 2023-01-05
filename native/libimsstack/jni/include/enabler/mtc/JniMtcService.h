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

#ifndef JNI_MTC_SERVICE_H_
#define JNI_MTC_SERVICE_H_

#include "BaseService.h"
#include "IJniEnabler.h"
#include "ImsTypeDef.h"

class IMtcService;
class IJniEnablerThread;
class JniMtcServiceThread;

class JniMtcService : public BaseService
{
public:
    JniMtcService(IN Jni_SendDataToJava pfnSendDataToJava, IN IMS_SINT32 nSlotId);
    virtual ~JniMtcService();

    virtual int SendData(const android::Parcel& objParcel) override;

    void Initialize(IN Jni_SendDataToJava pfnSendDataToJava);

    void NotifyNativeEnablerSet() override;
    IJniEnablerThread* GetJniThread() const override;

protected:
    void HandleMessage(IN IMS_SINT32 nMsg, IN const android::Parcel& objParcel) override;

private:
    void Attach();
    IMtcService* GetNativeService();
    void NotifySrvccStateChanged(IN const android::Parcel& objParcel);
    void SetTerminalBasedCallWaiting(IN const android::Parcel& objParcel);
    void OpenEmergencyService(IN const android::Parcel& objParcel);
    void ProcessTestCommand(IN const android::Parcel& objParcel);

private:
    JniMtcServiceThread* m_pThread;
};

#endif
