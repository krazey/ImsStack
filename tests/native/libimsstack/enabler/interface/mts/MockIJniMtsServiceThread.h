/*
 * Copyright (C) 2025 The Android Open Source Project
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

#ifndef MOCK_I_JNI_MTS_SERVICE_THREAD_H_
#define MOCK_I_JNI_MTS_SERVICE_THREAD_H_

#include <gmock/gmock.h>

#include "ByteArray.h"
#include "IJniMtsServiceThread.h"
#include "MtsDef.h"

class MockIJniMtsServiceThread : public IJniMtsServiceThread
{
public:
    inline virtual ~MockIJniMtsServiceThread() {}

    MOCK_METHOD(void, ReportMoStatus,
            (IN IMS_SINT32, IN SmsFormatType, IN IMS_SINT32, IN IMS_SINT32), (override));
    MOCK_METHOD(
            void, ReportMtSms, (IN SmsFormatType, IN const ByteArray&, IN IMS_SINT32), (override));
};

#endif
