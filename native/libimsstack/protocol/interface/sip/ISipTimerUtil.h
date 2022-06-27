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
#ifndef __ISIPTIMERUTIL_H__
#define __ISIPTIMERUTIL_H__

#include "sip_pf_datatypes.h"

typedef SIP_VOID (*SipTimerCallback)(SIP_VOID* pvData, SIP_VOID* pvTimerId);

class ISipTimerUtil
{
public:
    ISipTimerUtil(){};
    virtual ~ISipTimerUtil(){};

    virtual SIP_BOOL StartTimer(SIP_VOID** ppvTimerId, SIP_UINT32 nDuration, SIP_UINT16 nResetFlag,
            SipTimerCallback pfnTimerCallback, SIP_VOID* pvData) = 0;

    virtual SIP_VOID* StopTimer(SIP_VOID* pvTimerId) = 0;

    virtual SIP_BOOL ResetTimer(SIP_VOID* pvTimerId, SIP_UINT32 nNewDuration) = 0;

    virtual SIP_VOID* StopTimerEx(SIP_VOID* pvTimerId) = 0;
};
#endif  //__ISIPTIMERUTIL_H__
