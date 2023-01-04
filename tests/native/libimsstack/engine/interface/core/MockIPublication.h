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
#ifndef MOCK_I_PUBLICATION_H_
#define MOCK_I_PUBLICATION_H_

#include <gmock/gmock.h>

#include "IPublication.h"

class IPublicationListener;
class IRefreshListener;

class MockIPublication : public IPublication
{
public:
    // IMethod
    MOCK_METHOD(void, Destroy, (), (override));
    MOCK_METHOD(void, SetMessageMediator, (IN IMessageMediator * piMediator), (override));

    // IServiceMethod
    MOCK_METHOD(IMessage*, GetNextRequest, (), (override));
    MOCK_METHOD(IMessage*, GetNextResponse, (), (override));
    MOCK_METHOD(IMessage*, GetPreviousRequest, (IN IMS_SINT32 nServiceMethod), (const, override));
    MOCK_METHOD(IMessage*, GetPreviousResponse, (IN IMS_SINT32 nServiceMethod), (const, override));
    MOCK_METHOD(ImsList<IMessage*>, GetPreviousResponses, (IN IMS_SINT32 nServiceMethod),
            (const, override));
    MOCK_METHOD(ImsList<AString>, GetRemoteUserId, (), (const, override));

    // IPublication
    MOCK_METHOD(const AString&, GetEvent, (), (const, override));
    MOCK_METHOD(IMS_SINT32, GetState, (), (const, override));
    MOCK_METHOD(IMS_RESULT, Publish,
            (IN const ByteArray& objState, IN const AString& strContentType), (override));
    MOCK_METHOD(void, SetListener, (IN IPublicationListener * piListener), (override));
    MOCK_METHOD(IMS_RESULT, Unpublish, (), (override));
    MOCK_METHOD(void, SetRefreshListener, (IN IRefreshListener * piListener), (override));
    MOCK_METHOD(void, SetRefreshPolicy,
            (IN IMS_SINT32 nPolicy, IN IMS_SINT32 nCriteriaInterval, IN IMS_SINT32 nValueEorLt,
                    IN IMS_SINT32 nValueGt),
            (override));
};
#endif
