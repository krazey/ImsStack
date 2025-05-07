/**
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

#ifndef JNI_MEDIA_SESSION_H
#define JNI_MEDIA_SESSION_H

#include "BaseService.h"
#include "ImsTypeDef.h"
#include "IJniEnabler.h"
#include "IJniMediaManager.h"
#include "IJniMedia.h"

using namespace android;

class IJniEnablerThread;
class JniMediaSessionThread;

class JniMediaSession : public BaseService
{
public:
    JniMediaSession(IN Jni_SendDataToJava pfnSendDataToJava, IN IMS_SINT32 nSlotId,
            IN IMS_SINTP nCallKey, IN IMS_SINTP nNativeObject);
    virtual ~JniMediaSession();

    virtual int SendData(const Parcel& objParcel) override;
    void Initialize(IN Jni_SendDataToJava pfnSendDataToJava, IN IMS_SINTP nNativeObject);
    void SetMtcCallId(IN IMS_SINTP nCallKey);
    void NotifyNativeEnablerSet() override;
    IJniEnablerThread* GetJniThread() const override;
    static IMS_BOOL IsMediaMessage(IN IMS_SINT32 nMsg);

protected:
    virtual IMS_BOOL IsThreadSwitchingRequired(IN IMS_SINT32 nMsg) const override;
    void HandleMessage(IN IMS_SINT32 nMsg, IN const Parcel& objParcel) override;

private:
    void SetJniMediaSessionThread();
    IJniMediaManager* GetMediaManager();
    MEDIA_CONTENT_TYPE ConvertToMediaType(IN SessionType eSessiontype);
    void OnResponses(IN IMS_SINT32 nMsg, IN IMS_BOOL bNeedConfig, IN const Parcel& objParcel);
    void OnNotifyMediaInactivity(IN IMS_SINT32 nMsg, IN const Parcel& objParcel);
    void OnNotifyPacketLosses(IN IMS_SINT32 nMsg, IN const Parcel& objParcel);
    void OnNotifyCallQualityChange(IN IMS_SINT32 nMsg, IN const Parcel& objParcel);
    void OnNotifyHeaderExtension(IN IMS_SINT32 nMsg, IN const Parcel& objParcel);
    void OnNotifyQosInfo(IN IMS_SINT32 nMsg, IN const Parcel& objParcel);
    void OnNotifyMediaDetach(IN IMS_SINT32 nMsg);
    void OnSendDtmf(IN IMS_SINT32 nMsg, IN const Parcel& objParcel);
    void OnNotifyAnbrReceived(IN IMS_SINT32 nMsg, IN const Parcel& objParcel);
    void OnVideoMessage(IN IMS_SINT32 nMsg, IN const Parcel& objParcel);
    void ConvertString(IN const String16& strSource, OUT AString& strDest);

    JniMediaSessionThread* m_pThread;
    AString m_strThreadName;
    IMS_SINTP m_nCallKey;
};

#endif
