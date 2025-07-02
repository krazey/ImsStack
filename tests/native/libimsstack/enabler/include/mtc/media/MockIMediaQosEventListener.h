/*
 * Copyright (C) 2022 The Android Open Source Project
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except compliance with the License.
 * You may obtain a copy of the License at
 *
 *      http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#ifndef MOCK_I_MEDIA_QOS_EVENT_LISTENER_H_
#define MOCK_I_MEDIA_QOS_EVENT_LISTENER_H_

#include "ImsTypeDef.h"
#include "media/IMediaQosEventListener.h"
#include "precondition/QosDef.h"
#include <gmock/gmock.h>

class ISession;

class MockIMediaQosEventListener : public IMediaQosEventListener
{
public:
    ~MockIMediaQosEventListener() override {}
    MOCK_METHOD(void, OnQosStatusChanged, (IN ISession*, IN QosStatus, IN IMS_UINT32), (override));
};

#endif
