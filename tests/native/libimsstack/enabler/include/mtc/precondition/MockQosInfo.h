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

#ifndef MOCK_QOS_INFO_H_
#define MOCK_QOS_INFO_H_

#include "ImsTypeDef.h"
#include "precondition/MtcPreconditionManager.h"
#include <gmock/gmock.h>

class IQosTimerListener;
class QosStatusTable;
class QosTimer;
enum class QosStatus;

class MockQosInfo : public QosInfo
{
public:
    explicit MockQosInfo(IN IQosTimerListener* pListener) :
            QosInfo(pListener)
    {
    }
    ~MockQosInfo() {}
    MOCK_METHOD(QosStatus, GetAudioStatus, (), (const, override));
    MOCK_METHOD(QosStatus, GetVideoStatus, (), (const, override));
    MOCK_METHOD(QosStatus, GetTextStatus, (), (const, override));
    MOCK_METHOD(QosTimer&, GetTimer, (), (override));
    MOCK_METHOD(QosStatusTable&, GetStatusTable, (), (override));
    MOCK_METHOD(IMS_BOOL, IsPreconditionSupported, (), (const, override));
    MOCK_METHOD(void, SetAudioStatus, (IN QosStatus eStatus), (override));
    MOCK_METHOD(void, SetVideoStatus, (IN QosStatus eStatus), (override));
    MOCK_METHOD(void, SetTextStatus, (IN QosStatus eStatus), (override));
    MOCK_METHOD(void, SetSupportingPrecondition, (IN IMS_BOOL bSupported), (override));
};

#endif
