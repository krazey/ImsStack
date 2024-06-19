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
#ifndef INTERFACE_JNI_AOS_SERVICE_THREAD_H_
#define INTERFACE_JNI_AOS_SERVICE_THREAD_H_

#include "ImsTypeDef.h"
#include "ImsList.h"
#include "AString.h"
#include "IJniEnablerThread.h"

class IJniAosServiceThread : public IJniEnablerThread
{
public:
    virtual ~IJniAosServiceThread() {}

    virtual IMS_BOOL NotifyRegistered(IN IMS_SINT32 nRegType, IN IMS_SINT32 nNetworkType,
            IN IMS_UINT32 nFeatureTagBits, IN const ImsList<AString>& objFeatureTags) = 0;
    virtual IMS_BOOL NotifyRegistering(IN IMS_SINT32 nRegType, IN IMS_SINT32 nNetworkType,
            IN IMS_UINT32 nFeatureTagBits, IN const ImsList<AString>& objFeatureTags) = 0;
    virtual IMS_BOOL NotifyDeregistered(IN IMS_SINT32 nNetworkType, IN IMS_SINT32 nReason) = 0;
    virtual IMS_BOOL NotifyTechnologyChangeFailed(
            IN IMS_SINT32 nRegType, IN IMS_SINT32 nNetworkType, IN IMS_SINT32 nCauseCode) = 0;
    virtual IMS_BOOL NotifyAssociatedUriChanged(IN const ImsList<AString>& objUris) = 0;
    virtual IMS_BOOL NotifyCapabilitiesUpdateFailed(
            IN IMS_UINT32 nCapabilities, IN IMS_SINT32 nNetworkType, IN IMS_SINT32 nReason) = 0;
    virtual IMS_BOOL NotifyAosIsimState(IN IMS_UINT32 nState) = 0;
    virtual IMS_BOOL NotifyRegEventState(
            IN IMS_UINT32 nStatusCode, IN const ImsList<AString>& objImpus) = 0;
    virtual IMS_BOOL RequestPhoneNumberRetry(IN IMS_UINT32 nCommand) = 0;
    virtual IMS_BOOL RequestWifiService(IN IMS_BOOL bIsOn) = 0;
};

#endif
