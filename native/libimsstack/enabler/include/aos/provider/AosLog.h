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
#ifndef AOS_LOG_H_
#define AOS_LOG_H_

#include "interface/AosInternalMsgDef.h"

class AosLog
{
public:
    AosLog();
    virtual ~AosLog();

    // Application Log
    static const IMS_CHAR* AppMessageToString(IN IMS_UINT32 nType);
    static const IMS_CHAR* AppPendingToString(IN IMS_UINT32 nType);
    static const IMS_CHAR* AppRequestToString(IN IMS_UINT32 nType);
    static const IMS_CHAR* AppStateToString(IN IMS_UINT32 nType);
    static const IMS_CHAR* AppTimerToString(IN IMS_UINT32 nType);

    // Registration Log
    static const IMS_CHAR* RegMessageToString(IN IMS_UINT32 nType);
    static const IMS_CHAR* RegModeToString(IN IMS_UINT32 nType);
    static const IMS_CHAR* RegPendingToString(IN IMS_UINT32 nType);
    static const IMS_CHAR* RegReasonToString(IN IMS_SINT32 nType);
    static const IMS_CHAR* RegStateToString(IN IMS_UINT32 nType);
    static const IMS_CHAR* RegTimerToString(IN IMS_UINT32 nType);

    // Event Log
    static const IMS_CHAR* EventToString(IN IMS_SINT32 nEvent);
};
#endif // AOS_LOG_H_
