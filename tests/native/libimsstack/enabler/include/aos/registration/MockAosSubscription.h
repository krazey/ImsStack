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
#ifndef MOCK_AOS_SUBSCRIPTION_H_
#define MOCK_AOS_SUBSCRIPTION_H_

#include <gmock/gmock.h>

#include "registration/AosSubscription.h"

class IAosSubscriptionListener;
class IRegInfoContact;

class MockAosSubscription : public AosSubscription
{
public:
    MockAosSubscription() :
            AosSubscription()
    {
    }
    ~MockAosSubscription() override {}
    MOCK_METHOD(void, Initialize, (), (override));
    MOCK_METHOD(IMS_BOOL, Start, (IN IMS_BOOL bIsRadioCheckRequired), (override));
    MOCK_METHOD(void, Stop, (), (override));
    MOCK_METHOD(void, Destroy, (), (override));
    MOCK_METHOD(void, SetListener, (IN IAosSubscriptionListener * piListener), (override));
    MOCK_METHOD(void, SetRetryTimer, (IN IMS_BOOL bCheckRetryAfter), (override));
    MOCK_METHOD(IMS_UINT32, GetState, (), (override));
    MOCK_METHOD(IMS_BOOL, SendSubscribe, (), (override));
    MOCK_METHOD(IMS_BOOL, ProcessFailureResponse_423, (IN IMS_BOOL bIsRefreshed), (override));
    MOCK_METHOD(IMS_BOOL, ProcessFailureResponse_503, (IN IMS_BOOL bIsRefreshed), (override));
    MOCK_METHOD(IMS_BOOL, ProcessFailureResponse_504, (IN IMS_BOOL bIsRefreshed), (override));
    MOCK_METHOD(IMS_BOOL, IsRetryActionDueToRetryCounter, (IN IMS_BOOL bIsRefreshed), (override));
    MOCK_METHOD(IMS_BOOL, IsSubscriptionTerminated, (IN IMS_SINT32 nStatusCode), (override));
    MOCK_METHOD(IMS_BOOL, IsInitialRegistrationRequired,
            (IN IMS_SINT32 nStatusCode, IN IMS_BOOL bIsRefreshed), (override));
    MOCK_METHOD(IMS_BOOL, IsInitialRegistrationWithNextPcscfRequired,
            (IN IMS_SINT32 nStatusCode, IN IMS_BOOL bIsRefreshed), (override));
    MOCK_METHOD(IMS_BOOL, IsInitialRegistrationRequiredInWifi,
            (IN IMS_SINT32 nStatusCode, IN IMS_BOOL bIsRefreshed), (override));
    MOCK_METHOD(IMS_BOOL, IsResubscriptionStopped, (IN IMS_SINT32 nStatusCode), (override));
    MOCK_METHOD(IMS_BOOL, IsRegRequiredByNotify, (IN IMS_UINT32 nFeature), (override));
    MOCK_METHOD(IMS_BOOL, IsRegAfterWaitRequiredByNotify, (IN IMS_UINT32 nFeature), (override));
    MOCK_METHOD(IMS_BOOL, IsWfcErrorMessageSupportedWithStateChecked, (IN IMS_SINT32 nError),
            (override));
    MOCK_METHOD(IMS_BOOL, ProcessFailed_StatusCode,
            (IN IMS_SINT32 nStatusCode, IN IMS_BOOL bIsRefreshed), (override));
    MOCK_METHOD(void, SetRequestCommand,
            (IN IMS_BOOL bIsRefreshed, IN IMS_SINT32 nCommand, IN IMS_SINT32 nRetryAfter),
            (override));
    MOCK_METHOD(void, RequestCommand,
            (IN IMS_SINT32 nReason, IN IMS_SINT32 nCommand, IN IMS_SINT32 nRetryAfter), (override));
    MOCK_METHOD(void, ProcessStartFailed_StatusCode, (IN IMS_SINT32 nStatusCode), (override));
    MOCK_METHOD(void, ProcessStartFailed_Others, (IN IMS_SINT32 nReason), (override));
    MOCK_METHOD(void, ProcessUpdateFailed_StatusCode, (IN IMS_SINT32 nStatusCode), (override));
    MOCK_METHOD(void, ProcessUpdateFailed_Others, (IN IMS_SINT32 nReason), (override));
    MOCK_METHOD(IMS_SINT32, GetRetryAfter, (), (override));
    MOCK_METHOD(IMS_SINT32, GetNextThrottlingTime, (IN const ImsVector<IMS_SINT32>& objInterval),
            (override));
    MOCK_METHOD(void, ProcessTimerExpired, (), (override));
    MOCK_METHOD(void, SetRefreshPolicy, (), (override));
    MOCK_METHOD(IRegInfoContact*, GetRegInfoContact,
            (IN const ImsList<IRegInfoContact*>& objContact), (override));
    MOCK_METHOD(
            IMS_BOOL, CompareUriAssociatedWithContact, (IN const SipAddress& objUri), (override));
    MOCK_METHOD(IMS_SINT32, ConvertRegInfoEvent, (IN IMS_SINT32 nEvent), (override));
    MOCK_METHOD(void, ProcessNotifyState_Terminated, (IN IMS_SINT32 nEvent), (override));
    MOCK_METHOD(void, ProcessNotifyState_Active, (IN IMS_SINT32 nState), (override));
    MOCK_METHOD(void, ProcessNotifyState_InvalidBody, (), (override));
    MOCK_METHOD(void, ProcessRegEventChange, (IN IMS_UINT32 nStatusCode), (override));
};

#endif  // MOCK_AOS_SUBSCRIPTION_H_
