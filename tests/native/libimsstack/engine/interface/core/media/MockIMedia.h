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
#ifndef MOCK_I_MEDIA_H_
#define MOCK_I_MEDIA_H_

#include <gmock/gmock.h>
#include "core/media/IMedia.h"

class MockIMedia : public IMedia
{
public:
    MOCK_METHOD(IMS_SINT32, GetDirection, (), (const, override));
    MOCK_METHOD(IMSList<IMediaDescriptor*>, GetMediaDescriptors, (), (const, override));
    MOCK_METHOD(IMedia*, GetProposal, (), (const, override));
    MOCK_METHOD(IMS_SINT32, GetState, (), (const, override));
    MOCK_METHOD(IMS_SINT32, GetUpdateState, (), (const, override));
    MOCK_METHOD(IMS_RESULT, SetDirection, (IN IMS_SINT32 nDirection), (override));
    MOCK_METHOD(IMediaDescriptor*, GetMediaDescriptor, (), (const, override));
    MOCK_METHOD(IMS_SINT32, GetType, (), (const, override));
    MOCK_METHOD(void, RemoveMediaDescriptor, (IN IMS_UINT32 nPosition), (override));
};

#endif
