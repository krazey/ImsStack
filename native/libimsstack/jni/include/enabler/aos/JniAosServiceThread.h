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
#include "IJniAosServiceThread.h"

class JniAosServiceThread final : public BaseServiceThread, public IJniAosServiceThread
{
public:
    JniAosServiceThread();
    ~JniAosServiceThread() override;

    IMS_BOOL NotifyRegistered(IN IMS_SINT32 nRegType, IN IMS_SINT32 nNetworkType,
            IN IMS_UINT32 nFeatureTagBits, IN const ImsList<AString>& objFeatureTags) override;

    IMS_BOOL NotifyRegistering(IN IMS_SINT32 nRegType, IN IMS_SINT32 nNetworkType,
            IN IMS_UINT32 nFeatureTagBits, IN const ImsList<AString>& objFeatureTags) override;

    IMS_BOOL NotifyDeregistered(IN IMS_SINT32 nRegType, IN IMS_SINT32 nNetworkType,
            IN IMS_SINT32 nReason, IN IMS_SINT32 nDataFailureReason) override;

    IMS_BOOL NotifyDeregistering(IN IMS_SINT32 nRegType) override;

    IMS_BOOL NotifyTechnologyChangeFailed(
            IN IMS_SINT32 nRegType, IN IMS_SINT32 nNetworkType, IN IMS_SINT32 nReason) override;

    IMS_BOOL NotifyAssociatedUriChanged(IN const ImsList<AString>& objUris) override;

    IMS_BOOL NotifyCapabilitiesUpdateFailed(IN IMS_UINT32 nCapabilities, IN IMS_SINT32 nNetworkType,
            IN IMS_SINT32 nReason) override;

    IMS_BOOL NotifyAosIsimState(IN IMS_UINT32 nState) override;

    IMS_BOOL NotifyRegEventState(
            IN IMS_UINT32 nStatusCode, IN const ImsList<AString>& objImpus) override;

    IMS_BOOL NotifyImsFeatureChanged(IN IMS_SINT32 nRegType, IN IMS_SINT32 nNetworkType,
            IN IMS_UINT32 nFeatureTagBits) override;

    IMS_BOOL NotifyTrace(IN IMS_SINT32 nRegType, IN const AString& strLog) override;

    IMS_BOOL RequestPhoneNumberRetry(IN IMS_UINT32 nCommand) override;

    IMS_BOOL RequestWifiService(IN IMS_BOOL bIsOn) override;
};

#endif  // JNI_AOS_SERVICE_THREAD_H_
