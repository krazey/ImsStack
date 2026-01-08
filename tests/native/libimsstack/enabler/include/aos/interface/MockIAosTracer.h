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
#ifndef MOCK_I_AOS_TRACER_H_
#define MOCK_I_AOS_TRACER_H_

#include <gmock/gmock.h>
#include "interface/IAosTracer.h"

class MockIAosTracer : public IAosTracer
{
public:
    MOCK_METHOD(void, TraceStatus,
            (IN IAosAppContext * piContext, IN const AosStatusInfo& objStatus), (override));

    MOCK_METHOD(void, TraceEvent,
            (IN IAosAppContext * piContext, IN AosDomain eDomain, IN AosSeverity eSeverity,
                    IN const IMS_CHAR* pszEventName, IN const IMS_CHAR* pszEventValue),
            (override));

    MOCK_METHOD(void, TraceEvent,
            (IN IAosAppContext * piContext, IN AosDomain eDomain, IN AosSeverity eSeverity,
                    IN const IMS_CHAR* pszPrefix, IN const IMS_CHAR* pszEventName,
                    IN const IMS_CHAR* pszEventValue),
            (override));
};

#endif  // MOCK_I_AOS_TRACER_H_
