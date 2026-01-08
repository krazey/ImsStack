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
#ifndef INTERFACE_AOS_TRANSACTION_H_
#define INTERFACE_AOS_TRANSACTION_H_

#include "ImsTypeDef.h"

class IAosTransactionListener
{
public:
    virtual ~IAosTransactionListener(){};

    virtual void Transaction_OnConnectionFailed(IN IMS_UINT32 nFailureReason,
            IN IMS_UINT32 nCauseCode, IN IMS_UINT32 nWaitTimeMillis) = 0;

    virtual void Transaction_OnConnectionSetupPrepared() = 0;

    virtual void Transaction_OnTrafficPriorityChanged() = 0;
};

class IAosTransaction
{
public:
    virtual ~IAosTransaction(){};

    virtual void SetListener(IN IMS_UINT32 nType, IN IAosTransactionListener* piListener) = 0;
    virtual void RemoveListener(IN IMS_UINT32 nType, IN IAosTransactionListener* piListener) = 0;

    virtual IMS_BOOL IsTransactionAllowed(IN IMS_UINT32 nType) = 0;

    virtual IMS_BOOL StartTraffic(IN IMS_UINT32 nType, IN IMS_UINT32 nRadioType) = 0;
    virtual void StartEmergencyTraffic(IN IMS_UINT32 nRadioType) = 0;
    virtual void StopTraffic(IN IMS_UINT32 nType) = 0;
    virtual void StopEmergencyTraffic() = 0;

    virtual void SetWlan(IN IMS_BOOL bEnabled) = 0;

    enum
    {
        TYPE_NONE = 0,
        TYPE_REG = (0x0001),
        TYPE_SUB = (0x0002),
        TYPE_EMERGENCY = (0x0004),
        TYPE_DEREG = (0x0008)
    };
};

#endif
