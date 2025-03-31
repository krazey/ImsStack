/*
 * Copyright (C) 2025 The Android Open Source Project
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

#ifndef MOCK_I_MTS_MESSAGE_CONTROLLER_H_
#define MOCK_I_MTS_MESSAGE_CONTROLLER_H_

#include <gmock/gmock.h>
#include "message/IMtsMessageController.h"

class MockIMtsMessageController : public IMtsMessageController
{
public:
    virtual ~MockIMtsMessageController() {}

    MOCK_METHOD(IMS_BOOL, HasPendingMoSms, (), (const, override));
    MOCK_METHOD(void, ProcessMoSms,
            (IN SmsFormatType eSmsFormat, IN ByteArray* pContent, IN const AString& strAddress,
                    IN IMS_SINT32 nSeqId, IN IMS_BOOL bEmergency),
            (override));
    MOCK_METHOD(void, ProcessMtSms, (IN IPageMessage * piMessage), (override));
    MOCK_METHOD(void, ClearAllMessages, (), (override));
};

#endif
