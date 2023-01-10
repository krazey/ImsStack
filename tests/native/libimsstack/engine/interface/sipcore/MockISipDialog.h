/*
 * Copyright (C) 2023 The Android Open Source Project
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

#ifndef MOCK_I_SIP_DIALOG_H_
#define MOCK_I_SIP_DIALOG_H_

#include <gmock/gmock.h>

#include "ISipClientConnection.h"
#include "ISipConnection.h"
#include "ISipHeader.h"

#include "ISipDialog.h"

class MockISipDialog : public ISipDialog
{
public:
    inline MockISipDialog() = default;
    inline virtual ~MockISipDialog() = default;

    MOCK_METHOD(void, Destroy, (), (override));

    MOCK_METHOD(ISipDialog*, Clone, (), (const, override));
    MOCK_METHOD(IMS_BOOL, Equals, (IN const ISipDialog* piDialog), (override));
    MOCK_METHOD(AString, GetDialogId, (), (override));
    MOCK_METHOD(ISipClientConnection*, GetNewClientConnection, (IN const AString& strMethod),
            (override));
    MOCK_METHOD(IMS_SINT32, GetState, (), (const, override));
    MOCK_METHOD(IMS_BOOL, IsSameDialog, (IN const ISipConnection* piSc), (override));
    MOCK_METHOD(AString, GetComponent, (IN IMS_SINT32 nType), (const, override));
    MOCK_METHOD(AString, GetDialogIdEx, (), (override));
    MOCK_METHOD(const ISipHeader*, GetContactHeader, (), (const, override));
    MOCK_METHOD(IMS_RESULT, SetContactParameter,
            (IN const AString& strParameter, IN IMS_SINT32 nOperation), (override));
    MOCK_METHOD(void, TerminateDialogUsage, (), (override));
};

#endif
