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
#ifndef SIP_WAKE_LOCK_H_
#define SIP_WAKE_LOCK_H_

#include "SipMethod.h"

class SipWakeLock
{
private:
    SipWakeLock();

    SipWakeLock(IN const SipWakeLock&) = delete;
    SipWakeLock& operator=(IN const SipWakeLock&) = delete;

public:
    static void Acquire(IN const SipMethod& objMethod, IN IMS_UINT32 nTimeout = 0);
    static IMS_BOOL IsSupported();
    static void Release();
};

#endif
