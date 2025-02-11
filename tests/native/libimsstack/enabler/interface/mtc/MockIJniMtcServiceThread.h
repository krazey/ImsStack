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

#ifndef MOCK_I_JNI_MTC_SERVICE_THREAD_H_
#define MOCK_I_JNI_MTC_SERVICE_THREAD_H_

#include <gmock/gmock.h>

#include "CallReasonInfo.h"
#include "IJniMtcServiceThread.h"

class MockIJniMtcServiceThread : public IJniMtcServiceThread
{
public:
    inline virtual ~MockIJniMtcServiceThread() {}

    MOCK_METHOD(void, OnServiceChanged, (IN IuMtcService::ServiceState, IN IMS_SINT32), (override));
    MOCK_METHOD(void, OnEmergencyServiceChanged,
            (IN IuMtcService::EmergencyServiceState, IN IMS_SINT32, IN ServiceType), (override));
    MOCK_METHOD(void, OnPreIncomingCallReceived, (IN IMS_ULONG), (override));
    MOCK_METHOD(void, OnRejectedIncomingCall,
            (IN IMS_ULONG, IN const JniCallInfo&, IN const MediaInfo&,
                    (IN const ImsMap<SuppType, SuppService*>&), IN OipType, IN const AString&,
                    IN const CallReasonInfo&),
            (override));
    MOCK_METHOD(void, OnExternalCallsChanged, (IN ImsList<const JniExternalCall*>&), (override));
};

#endif
