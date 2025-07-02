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
#ifndef INTERFACE_JNI_MTC_SERVICE_THREAD_H_
#define INTERFACE_JNI_MTC_SERVICE_THREAD_H_

#include "IJniEnablerThread.h"
#include "IMtcService.h"
#include "ImsList.h"
#include "ImsMap.h"
#include "ImsTypeDef.h"
#include "IuMtcService.h"
#include "MtcDef.h"

struct CallReasonInfo;
struct JniCallInfo;
struct JniExternalCall;

class IJniMtcServiceThread : public IJniEnablerThread
{
public:
    virtual ~IJniMtcServiceThread() {}

    /**
     * @brief Notifies
     *
     * @param eState
     * @param eReason
     */
    virtual void OnServiceChanged(IN IuMtcService::ServiceState eState, IN IMS_SINT32 eReason) = 0;

    /**
     * @brief Notifies
     *
     * @param eState
     * @param eReason
     * @param eServiceType
     */
    virtual void OnEmergencyServiceChanged(IN IuMtcService::EmergencyServiceState eState,
            IN IuMtcService::EmergencyServiceUnavailableReason eReason,
            IN ServiceType eServiceType) = 0;

    /**
     * @brief Notifies
     *
     * @param nCallKey
     */
    virtual void OnPreIncomingCallReceived(IN IMS_ULONG nCallKey) = 0;

    /**
     * @brief Notifies there is an rejected incoming call for call logs.
     *
     * @param nCallKey
     * @param objCallInfo
     * @param objMediaInfo
     * @param objSuppServices
     * @param eOipType
     * @param strRemoteNumber
     */
    virtual void OnRejectedIncomingCall(IN IMS_ULONG nCallKey, IN const JniCallInfo& objCallInfo,
            IN const MediaInfo& objMediaInfo,
            IN const ImsMap<SuppType, SuppService*>& objSuppServices, IN OipType eOipType,
            IN const AString& strRemoteNumber, IN const CallReasonInfo& objReason) = 0;

    /**
     * @brief Notifies Java that {@code JniExternalCall} state is changed.
     *
     * @param objJniExternalCalls The list of existing {@code JniExternalCall}
     */
    virtual void OnExternalCallsChanged(
            IN ImsList<const JniExternalCall*>& objJniExternalCalls) = 0;
};

#endif
