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
#include "IMSTypeDef.h"
#include "JniMediaSessionThread.h"
#include "IMediaManager.h"

class JniMediaSession : public BaseService
{
public:
    JniMediaSession(IN CBServiceNoti pfnNotifier, IN IMS_SINT32 nSlotId, IN IMS_SINTP nCallKey,
            IN IMS_SINTP nNativeObject);
    virtual ~JniMediaSession();

    virtual int SendData(const android::Parcel& objParcel) override;
    void Initialize(IN CBServiceNoti pfnNotifier, IN IMS_SINTP nNativeObject);
    void SetMtcCallId(IN IMS_SINTP nCallKey);
    JniMediaSessionThread* GetThread();
    static IMS_BOOL IsMediaMessage(IN IMS_SINT32 nMsg);

protected:
    virtual IMS_BOOL IsThreadSwitchingRequired(IN IMS_SINT32 nMsg) const override;
    void HandleMessage(IN IMS_SINT32 nMsg, IN const android::Parcel& objParcel) override;

private:
    void SetJniMediaSessionThread();
    MEDIA_CONTENT_TYPE ConvertToMediaType(IN SessionType eSessiontype);
    void OnResponses(
            IN IMS_SINT32 nMsg, IN IMS_BOOL bNeedConfig, IN const android::Parcel& objParcel);
    void OnNofityMediaInactitivy(IN IMS_SINT32 nMsg, IN const android::Parcel& objParcel);
    void OnNofityPacketLosses(IN IMS_SINT32 nMsg, IN const android::Parcel& objParcel);
    void OnNofityMediaQualityChange(IN IMS_SINT32 nMsg, IN const android::Parcel& objParcel);
    void OnResponseSessionChanged(IN IMS_SINT32 nMsg, IN const android::Parcel& objParcel);
    void OnNofityHeaderExtension(IN IMS_SINT32 nMsg, IN const android::Parcel& objParcel);
    void OnNotifyQosInfo(IN IMS_SINT32 nMsg, IN const android::Parcel& objParcel);
    void OnNotifyMediaDetach(IN IMS_SINT32 nMsg);
    void OnCmdSetSurface(IN IMS_SINT32 nMsg, IN const android::Parcel& objParcel);
    void OnCmdSelectCamera(IN IMS_SINT32 nMsg, IN const android::Parcel& objParcel);
    void OnCmdChangeCameraZoom(IN IMS_SINT32 nMsg, IN const android::Parcel& objParcel);
    void OnCmdOrientationChanged(IN IMS_SINT32 nMsg, IN const android::Parcel& objParcel);
    void ConvertString(IN const android::String16& strSource, OUT AString& strDest);

private:
    JniMediaSessionThread* m_pThread;
    AString m_strThreadName;
    IMS_SINT32 m_nSlotId;
    IMediaManager* m_piMediaManager;
    IMS_SINTP m_nCallKey;
};

#endif
