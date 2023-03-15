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

#ifndef JNI_MTC_SERVICE_THREAD_H_
#define JNI_MTC_SERVICE_THREAD_H_

#include "BaseServiceThread.h"
#include "IJniMtcServiceThread.h"
#include "IMtcService.h"
#include "ImsMap.h"
#include "IuMtcService.h"
#include "MtcDef.h"

class JniMtcServiceThread final : public BaseServiceThread, public IJniMtcServiceThread
{
public:
    JniMtcServiceThread();
    virtual ~JniMtcServiceThread();

    void OnServiceChanged(IN IuMtcService::ServiceState eState, IN IMS_SINT32 eReason) override;
    void OnEmergencyServiceChanged(IN IuMtcService::EmergencyServiceState eState,
            IN IMS_SINT32 eReason, IN ServiceType eServiceType) override;
    void OnPreIncomingCallReceived(IN IMS_ULONG nCallKey) override;

    void OnJniReady();
    void OnExternalCallsChanged(IN ImsList<const JniExternalCall*>& objJniExternalCalls) override;
};

#endif
