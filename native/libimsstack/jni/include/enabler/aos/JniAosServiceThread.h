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
#ifndef JNI_AOS_SERVICE_THREAD_H_
#define JNI_AOS_SERVICE_THREAD_H_

#include "BaseServiceThread.h"

class JniAosServiceThread final
    : public BaseServiceThread
{
public:
    JniAosServiceThread();
    virtual ~JniAosServiceThread();

    inline void SetSlotId(IN IMS_SINT32 nSlotId) { m_nSlotId = nSlotId; }

    void NotifyRegistered(IN IMS_SINT32 nNetworkType, IN IMS_UINT32 nFeatureTagBits,
            IN const IMSList<AString>& objFeatureTags);

    void NotifyRegistering(IN IMS_SINT32 nNetworkType, IN IMS_UINT32 nFeatureTagBits,
            IN const IMSList<AString>& objFeatureTags);

    void NotifyDeregistered(IN IMS_SINT32 nReason);
    void NotifyTechnologyChangeFailed(IN IMS_SINT32 nNetworkType, IN IMS_SINT32 nCauseCode);
    void NotifyAssociatedUriChanged(IN const IMSList<AString>& objUris);
    void NotifyCapabilitiesUpdateFailed(IN IMS_UINT32 nCapabilities, IN IMS_SINT32 nNetworkType,
            IN IMS_SINT32 nReason);

    void NotifyAosIsimState(IN IMS_UINT32 nState);
    void NotifyRegEventState(IN IMS_UINT32 nState);
    void RequestPhoneNumberRetry(IN IMS_UINT32 nCommand);
    void RequestWifiService(IN IMS_BOOL bIsOn);

private:
    IMS_SINT32 m_nSlotId;
};

#endif // JNI_AOS_SERVICE_THREAD_H_