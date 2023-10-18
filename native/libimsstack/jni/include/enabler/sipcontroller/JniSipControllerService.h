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

#ifndef JNI_SIP_CONTROLLER_SERVICE_H_
#define JNI_SIP_CONTROLLER_SERVICE_H_

#include "BaseService.h"

class JniSipControllerServiceThread;
class IJniEnablerThread;
class ISipControllerService;
class IUSncSendMessageParam;
class IUSncFeatureTagsParam;
class IUSncNotifyErrorCmdParam;

using namespace android;

class JniSipControllerService : public BaseService
{
public:
    explicit JniSipControllerService(
            Jni_SendDataToJava pfnSendDataToJava, IN IMS_UINT32 nSimSlot = 0);
    virtual ~JniSipControllerService();

    inline void NotifyNativeEnablerSet() override {}
    IJniEnablerThread* GetJniThread() const override;
    int SendData(const Parcel& pParcel) override;

protected:
    void HandleMessage(int nMsg, const Parcel& pParcel) override;

private:
    ISipControllerService* GetNativeService();
    void OpenMessageTracker();
    void SendMessage(IN ISipControllerService* piSipService, IN IUSncSendMessageParam* pParam);
    void NotifyMessageReceiveError(
            IN ISipControllerService* piSipService, IN IUSncNotifyErrorCmdParam* pParam);
    void TriggerDelegateDeregistration(IN ISipControllerService* piSipService);
    void CloseSession(IN ISipControllerService* piSipService, IN const AString& strCallId);
    void UpdateRegistration(
            IN ISipControllerService* piSipService, IN IUSncFeatureTagsParam* pParam);
    IUSncSendMessageParam* makeSendMessageParamFromParcel(IN const android::Parcel& objParcel);
    void ConvertString(IN const android::String16& strSource, OUT AString& strDest);

private:
    JniSipControllerServiceThread* m_pJniSipControllerServiceThread;
    AString m_strThreadName;
};
#endif  // JNI_SIP_CONTROLLER_SERVICE_H_
