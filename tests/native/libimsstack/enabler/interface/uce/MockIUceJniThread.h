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

#ifndef MOCK_I_UCE_JNI_THREAD_H_
#define MOCK_I_UCE_JNI_THREAD_H_

#include <gmock/gmock.h>
#include "IUceJniThread.h"
class AString;

class MockIUceJniThread : public IUceJniThread
{
public:
    virtual ~MockIUceJniThread() {}
    MOCK_METHOD(IMS_BOOL, NotifyImsDeregistered, (), (override));
    MOCK_METHOD(IMS_BOOL, NotifyImsRegistered, (IMS_UINT32, IMS_SINT32), (override));
    MOCK_METHOD(IMS_BOOL, PublishResponseInd,
            (IMS_UINT32, IMS_UINT32, IMS_UINT32, AString, IMS_UINT32, AString, AString, IMS_UINT32),
            (override));
    MOCK_METHOD(IMS_BOOL, PublishUpdatedInd,
            (IMS_UINT32, IMS_SINT32, AString, IMS_SINT32, AString, AString, IMS_UINT32),
            (override));
    MOCK_METHOD(IMS_BOOL, PublishErrorInd, (IMS_UINT32 key, IMS_UINT32 commandError), (override));
    MOCK_METHOD(IMS_BOOL, UnPublishedInd, (), (override));
    MOCK_METHOD(IMS_BOOL, SubscribeResponseInd,
            (IMS_UINT32, IMS_SINT32, AString, IMS_SINT32, AString), (override));
    MOCK_METHOD(IMS_BOOL, NotifyInd, (IMS_UINT32, IMS_UINT32, IMSList<AString>), (override));

    MOCK_METHOD(IMS_BOOL, SubscribeErrorInd, (IMS_UINT32, IMS_UINT32), (override));
    MOCK_METHOD(IMS_BOOL, SubscribeResourceTerminatedInd,
            (IMS_UINT32, IMS_UINT32, IMSList<IUceTerminatedReason*>), (override));
    MOCK_METHOD(IMS_BOOL, SubscribeTerminatedInd, (IMS_UINT32, AString, IMS_UINT32), (override));
    MOCK_METHOD(IMS_BOOL, OptionsResponseInd, (IMS_UINT32, IMS_UINT32, AString, IMS_UINT32),
            (override));
    MOCK_METHOD(IMS_BOOL, OptionsErrorInd, (IMS_UINT32, IMS_UINT32), (override));
    MOCK_METHOD(IMS_BOOL, OptionsReceivedInd, (IMS_UINT32, AString, IMS_UINT32), (override));
    MOCK_METHOD(IMS_BOOL, NotifyImsRegiRefreshed, (IMS_SINT32), (override));
    MOCK_METHOD(IMS_BOOL, NotifyNetworkChanged, (IMS_SINT32), (override));
};

#endif
