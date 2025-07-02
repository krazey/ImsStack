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

#ifndef MOCK_UDP_KEEP_ALIVE_SENDER_H_
#define MOCK_UDP_KEEP_ALIVE_SENDER_H_

#include "ImsTypeDef.h"
#include "helper/UdpKeepAliveSender.h"
#include <gmock/gmock.h>

class IMtcCallContext;
class ITimer;

class MockUdpKeepAliveSender : public UdpKeepAliveSender
{
public:
    explicit MockUdpKeepAliveSender(
            IN ISipKeepAliveHelper* pKeepAliveHelper, IN IMtcCallContext& objContext) :
            UdpKeepAliveSender(pKeepAliveHelper, objContext)
    {
    }
    virtual ~MockUdpKeepAliveSender() override {}

    MOCK_METHOD(void, Timer_TimerExpired, (IN ITimer*), (override));
    MOCK_METHOD(void, Start, (), (override));
    MOCK_METHOD(void, Stop, (), (override));
};

#endif
