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

#ifndef INTERFACE_MTS_SERVICE_STATE_H_
#define INTERFACE_MTS_SERVICE_STATE_H_

#include "ImsTypeDef.h"

class IImsAos;

class IMtsServiceState
{
public:
    virtual ~IMtsServiceState() {}

    // MtsService
    virtual void Init(IN IImsAos* piImsAos) = 0;
    virtual IMS_SINT32 GetState() const = 0;
    virtual void OnImsConnected() = 0;
    virtual void OnImsDisconnected(IN IMS_UINT32 nReason) = 0;
    virtual void OnImsDisconnecting(IN IMS_UINT32 nReason) = 0;
    virtual void OnImsSuspended(IN IMS_UINT32 nReason) = 0;
    virtual void OnImsResumed() = 0;
    virtual void SetImsRegConnected(IN IMS_BOOL bConnected) = 0;

    // MtsMessageController
    virtual IMS_BOOL IsMoServiceBlocked() const = 0;
    virtual IMS_BOOL IsMtServiceBlocked() const = 0;
};

#endif
