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
#ifndef MOCK_I_REFERENCE_H_
#define MOCK_I_REFERENCE_H_

#include <gmock/gmock.h>

#include "IReference.h"
#include "ByteArray.h"
#include "IServiceMethod.h"

class INotificationListener;
class IReferenceListener;

class MockIReference : public IReference
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

    // IReference
    MOCK_METHOD(IMS_RESULT, Accept, (), (override));
    MOCK_METHOD(IMS_RESULT, ConnectReferMethod, (IN IServiceMethod * piServiceMethod), (override));
    MOCK_METHOD(const AString&, GetReferMethod, (), (const, override));
    MOCK_METHOD(const AString&, GetReferToUserId, (), (const, override));
    MOCK_METHOD(const AString&, GetReplaces, (), (const, override));
    MOCK_METHOD(IMS_SINT32, GetState, (), (const, override));
    MOCK_METHOD(IMS_RESULT, Refer, (IN IMS_BOOL bImplicitSubscription), (override));
    MOCK_METHOD(IMS_RESULT, Reject, (), (override));
    MOCK_METHOD(void, SetListener, (IN IReferenceListener * piListener), (override));
    MOCK_METHOD(IMS_RESULT, SetReplaces, (IN const AString& strSessionId), (override));
    MOCK_METHOD(
            IMS_RESULT, AcceptEx, (IN IMS_SINT32 nStatusCode, IN IMS_BOOL b100Trying), (override));
    MOCK_METHOD(IMS_RESULT, ReferEx,
            (IN IMS_BOOL bImplicitSubscription, IN const AString& strHeadersForReferTo),
            (override));
    MOCK_METHOD(IMS_RESULT, RejectEx, (IN IMS_SINT32 nStatusCode), (override));
    MOCK_METHOD(IMS_RESULT, SendNotification,
            (IN IMS_SINT32 nSubState, IN const ByteArray& objContent, IN IMS_SINT32 nReason,
                    IN IMS_SINT32 nExpires),
            (override));
    MOCK_METHOD(void, SetNotificationListener, (IN INotificationListener * piListener), (override));
    MOCK_METHOD(void, SetImplicitRoutingRequired, (IN IMS_BOOL bFlag), (override));
};

#endif
