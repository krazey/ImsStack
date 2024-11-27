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

#ifndef JNI_MTC_CALL_H_
#define JNI_MTC_CALL_H_

#include "BaseService.h"
#include "IJniEnabler.h"
#include "IMtcCallController.h"
#include "ImsMap.h"
#include "IuMtcCall.h"
#include "call/IMtcCall.h"

class IJniEnablerThread;
class JniMediaSession;
class JniMtcCallThread;

class JniMtcCall : public BaseService
{
public:
    explicit JniMtcCall(IN Jni_SendDataToJava pfnSendDataToJava, IN IMS_SINT32 nSlotId = 0);
    virtual ~JniMtcCall();
    void Destroy() override;

    virtual IMS_SINT32 SendData(IN const android::Parcel& objParcel) override;
    void Initialize();

    IJniEnablerThread* GetJniThread() const override;
    inline void NotifyNativeEnablerSet() override {}

protected:
    void HandleMessage(IN IMS_SINT32 nMsg, IN const android::Parcel& objParcel) override;
    IMS_BOOL IsThreadSwitchingRequired(IN IMS_SINT32 nMsg) const override;

private:
    void Attach(IN IMtcCallController& objCallController);
    void Attach(IN IMtcCallController& objCallController, IN const android::Parcel& objParcel);
    void Open(IN IMtcCallController& objCallController, IN const android::Parcel& objParcel);
    void Start(IN IMtcCallController& objCallController, IN const android::Parcel& objParcel);
    void OnUserAlert(IN IMtcCallController& objCallController, IN const android::Parcel& objParcel);
    void Accept(IN IMtcCallController& objCallController, IN const android::Parcel& objParcel);
    void Reject(IN IMtcCallController& objCallController, IN const android::Parcel& objParcel);
    void Hold(IN IMtcCallController& objCallController, IN const android::Parcel& objParcel);
    void Resume(IN IMtcCallController& objCallController, IN const android::Parcel& objParcel);
    void Terminate(IN IMtcCallController& objCallController, IN const android::Parcel& objParcel);
    void Update(IN IMtcCallController& objCallController, IN const android::Parcel& objParcel);
    void AcceptUpdate(
            IN IMtcCallController& objCallController, IN const android::Parcel& objParcel);
    void RejectUpdate(
            IN IMtcCallController& objCallController, IN const android::Parcel& objParcel);
    void CancelUpdate(
            IN IMtcCallController& objCallController, IN const android::Parcel& objParcel);
    void AcceptResume(
            IN IMtcCallController& objCallController, IN const android::Parcel& objParcel);
    void RejectResume(
            IN IMtcCallController& objCallController, IN const android::Parcel& objParcel);
    void SendUssd(IN IMtcCallController& objCallController, IN const android::Parcel& objParcel);

    static void StartGroupCall(
            IN IMtcCallController& objCallController, IN const android::Parcel& objParcel);
    void MergeToConference(
            IN IMtcCallController& objCallController, IN const android::Parcel& objParcel);
    static void ExpandToConference(
            IN IMtcCallController& objCallController, IN const android::Parcel& objParcel);
    void AddToConference(
            IN IMtcCallController& objCallController, IN const android::Parcel& objParcel);
    void RemoveFromConference(
            IN IMtcCallController& objCallController, IN const android::Parcel& objParcel);

    void Transfer(IN IMtcCallController& objCallController);
    void TransferWithNumber(
            IN IMtcCallController& objCallController, IN const android::Parcel& objParcel);

    IMtcCallController* GetCallController() const;

private:
    JniMtcCallThread* m_pThread;
    Jni_SendDataToJava m_pfnSendDataToJava;
    CallKey m_nCallKey;
    JniMediaSession* m_pJniMediaSession;
};

#endif
