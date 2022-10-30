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
#include "ImsTypeDef.h"

class IJniMtcServiceThread : public IJniEnablerThread
{
public:
    virtual ~IJniMtcServiceThread() {}

    virtual void OnServiceChanged(IN IMS_SINT32 eStatus, IN IMS_SINT32 eReason) = 0;
    virtual void OnEmergencyServiceChanged(
            IN IMS_SINT32 eStatus, IN IMS_SINT32 eReason, IN IMS_SINT32 eServiceType) = 0;
    virtual void OnPreIncomingCallReceived(IN IMS_ULONG nCallKey) = 0;
};

#endif
