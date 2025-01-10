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

#ifndef INTERFACE_MTS_SERVICE_H_
#define INTERFACE_MTS_SERVICE_H_

#include "ByteArray.h"
#include "INativeEnabler.h"
#include "ImsTypeDef.h"
#include "MtsDef.h"

class ICoreService;
class IMtsServiceState;

class IMtsService : public INativeEnabler
{
public:
    virtual ~IMtsService() {}

    virtual ICoreService* GetICoreService(IN IMS_BOOL bEmergency) const = 0;
    virtual IMtsServiceState* GetIMtsServiceState() = 0;
    virtual void ReportMoStatus(
            IN IMS_SINT32 nReason, IN SmsFormatType eSmsFormat, IN IMS_SINT32 nSeqId) = 0;
    virtual void ReportMtSms(IN SmsFormatType eSmsFormat, IN const ByteArray& objContent) = 0;
    virtual void RequestRegistrationRecovery(IN IMS_UINT32 nRecoveryType) = 0;
    virtual void RequestRegisterWithNextPcscf(IN const IMS_UINT32 nRetryAfterValue) = 0;
    virtual void SendMoSms(IN SmsFormatType eSmsFormat, IN ByteArray* pContent,
            IN const AString& strAddress, IN IMS_SINT32 nSeqId, IN IMS_BOOL bEmergency) = 0;
};

#endif
