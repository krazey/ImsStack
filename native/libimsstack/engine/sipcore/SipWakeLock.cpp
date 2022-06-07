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
#include "ServiceEvent.h"
#include "ServiceMemory.h"

#include "SipWakeLock.h"

PUBLIC GLOBAL void SipWakeLock::Acquire(
        IN const SipMethod& objMethod, IN IMS_UINT32 /*nTimeout = 0*/)
{
    if (objMethod.Equals(SipMethod::INVITE))
    {
        // 3 seconds
        IMS_EVENT_SendEventForSlotId(IMS_EVENT_WAKE_LOCK, 0, 3000, IMS_SLOT_0);
    }
    else if (objMethod.Equals(SipMethod::UPDATE))
    {
        // 2 seconds
        IMS_EVENT_SendEventForSlotId(IMS_EVENT_WAKE_LOCK, 0, 2000, IMS_SLOT_0);
    }
    else if (objMethod.Equals(SipMethod::BYE))
    {
        // 2 seconds
        IMS_EVENT_SendEventForSlotId(IMS_EVENT_WAKE_LOCK, 0, 2000, IMS_SLOT_0);
    }
    else if (objMethod.Equals(SipMethod::ACK) || objMethod.Equals(SipMethod::PRACK) ||
            objMethod.Equals(SipMethod::OPTIONS) || objMethod.Equals(SipMethod::MESSAGE) ||
            objMethod.Equals(SipMethod::REFER) || objMethod.Equals(SipMethod::NOTIFY) ||
            objMethod.Equals(SipMethod::INFO))
    {
        // 1 seconds
        IMS_EVENT_SendEventForSlotId(IMS_EVENT_WAKE_LOCK, 0, 1000, IMS_SLOT_0);
    }
}

PUBLIC GLOBAL IMS_BOOL SipWakeLock::IsSupported()
{
    return IMS_TRUE;
}

PUBLIC GLOBAL void SipWakeLock::Release() {}
