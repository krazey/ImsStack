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

#ifndef MOCK_I_MTS_MESSAGE_H_
#define MOCK_I_MTS_MESSAGE_H_

#include "MtsDef.h"
#include "message/IMtsMessage.h"
#include <gmock/gmock.h>

class MockIMtsMessage : public IMtsMessage
{
public:
    virtual ~MockIMtsMessage() {}

    MOCK_METHOD(AString&, GetDestination, (), (override));
    MOCK_METHOD(void, SetDestination, (IN const AString& strDestination), (override));
    MOCK_METHOD(AString&, GetImpu, (), (override));
    MOCK_METHOD(void, SetImpu, (IN const AString& strImpu), (override));
    MOCK_METHOD(IMS_SINT32, GetMessageReference, (), (override));
    MOCK_METHOD(void, SetMessageReference, (IN IMS_SINT32 nMrOfRp), (override));
    MOCK_METHOD(IMS_SINT32, GetMti, (), (override));
    MOCK_METHOD(void, SetMti, (IN IMS_SINT32 nMti), (override));
    MOCK_METHOD(IPageMessage*, GetPageMessage, (), (const, override));
    MOCK_METHOD(void, SetPageMessage, (IN IPageMessage * piPageMessage), (override));
    MOCK_METHOD(IMS_UINT32, GetRetryCount, (), (const, override));
    MOCK_METHOD(void, SetRetryCount, (IN IMS_UINT32 nRetryCount), (override));
    MOCK_METHOD(IMS_SINT32, GetSeqId, (), (override));
    MOCK_METHOD(void, SetSeqId, (IN IMS_SINT32 nSeqId), (override));
    MOCK_METHOD(IMS_SINT32, GetSlotId, (), (override));
    MOCK_METHOD(void, SetSlotId, (IN IMS_SINT32 nSlotId), (override));
    MOCK_METHOD(SmsFormatType, GetSmsFormat, (), (override));
    MOCK_METHOD(void, SetSmsFormat, (IN SmsFormatType eSmsFormat), (override));
    MOCK_METHOD(IMS_SINT32, GetSmSize, (), (override));
    MOCK_METHOD(void, SetSmSize, (IN IMS_SINT32 nSmSize), (override));
    MOCK_METHOD(MtsTransactionType, GetTransactionType, (), (override));
    MOCK_METHOD(void, SetTransactionType, (IN MtsTransactionType eTransactionType), (override));
    MOCK_METHOD(void, PrintInfo, (), (override));
};

#endif
