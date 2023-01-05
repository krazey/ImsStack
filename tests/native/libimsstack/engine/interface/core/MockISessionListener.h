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
#ifndef MOCK_I_SESSION_LISTENER_H_
#define MOCK_I_SESSION_LISTENER_H_

#include <gmock/gmock.h>

#include "ISession.h"
#include "ISessionListener.h"

class IReference;
class ISipServerConnection;

class MockISessionListener : public ISessionListener
{
public:
    MOCK_METHOD(void, SessionAlerting, (IN ISession* piSession), (override));
    MOCK_METHOD(void, SessionReferenceReceived,
            (IN ISession* piSession, IN IReference* piReference), (override));
    MOCK_METHOD(void, SessionStarted, (IN ISession* piSession), (override));
    MOCK_METHOD(void, SessionStartFailed, (IN ISession* piSession), (override));
    MOCK_METHOD(void, SessionTerminated, (IN ISession* piSession), (override));
    MOCK_METHOD(void, SessionUpdated, (IN ISession* piSession), (override));
    MOCK_METHOD(void, SessionUpdateFailed, (IN ISession* piSession), (override));
    MOCK_METHOD(void, SessionUpdateReceived, (IN ISession* piSession), (override));
    MOCK_METHOD(void, SessionCancelDelivered, (IN ISession* piSession), (override));
    MOCK_METHOD(void, SessionCancelDeliveryFailed, (IN ISession* piSession), (override));
    MOCK_METHOD(void, SessionEarlyMediaUpdated, (IN ISession* piSession), (override));
    MOCK_METHOD(void, SessionEarlyMediaUpdateFailed, (IN ISession* piSession), (override));
    MOCK_METHOD(void, SessionEarlyMediaUpdateReceived, (IN ISession* piSession), (override));
    MOCK_METHOD(void, SessionForkedResponseReceived,
            (IN ISession* piSession, IN ISession* piForkedSession), (override));
    MOCK_METHOD(void, SessionPrackDelivered, (IN ISession* piSession), (override));
    MOCK_METHOD(void, SessionPrackDeliveryFailed, (IN ISession* piSession), (override));
    MOCK_METHOD(void, SessionPrackReceived, (IN ISession* piSession), (override));
    MOCK_METHOD(void, SessionProvisionalResponseReceived,
            (IN ISession* piSession, IN IMS_UINT32 nIndex), (override));
    MOCK_METHOD(void, SessionRprDeliveryFailed, (IN ISession* piSession), (override));
    MOCK_METHOD(
            void, SessionRprReceived, (IN ISession* piSession, IN IMS_UINT32 nIndex), (override));
    MOCK_METHOD(void, SessionTransactionReceived,
            (IN ISession* piSession, IN ISipServerConnection* piSsc), (override));
};

#endif
