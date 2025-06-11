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

#ifndef MOCK_I_MTS_SERVICE_H_
#define MOCK_I_MTS_SERVICE_H_

#include <gmock/gmock.h>
#include "IMtsService.h"
#include "MtsDef.h"

class ICoreService;
class IMtsServiceState;

class MockIMtsService : public IMtsService
{
public:
    virtual ~MockIMtsService() {}

    MOCK_METHOD(ICoreService*, GetICoreService, (), (const, override));
    MOCK_METHOD(IMtsServiceState*, GetIMtsServiceState, (), (const, override));
    MOCK_METHOD(
            void, RequestRegistrationRecovery, (IN IMS_UINT32 nRecoveryType), (const, override));
    MOCK_METHOD(void, RequestRegisterWithNextPcscf, (IN const IMS_UINT32 nRetryAfterValue),
            (const, override));
    MOCK_METHOD(void, SendMoSms,
            (IN SmsFormatType eSmsFormat, IN ByteArray* pContent, IN const AString& strAddress,
                    IN IMS_SINT32 nSeqId, IN IMS_BOOL bEmergency, IN IMS_UINT32 nRetryCount),
            (override));
};

#endif
