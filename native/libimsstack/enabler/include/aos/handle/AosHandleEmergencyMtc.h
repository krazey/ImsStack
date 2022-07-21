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
#ifndef AOS_HANDLE_EMERGENCY_MTC_H_
#define AOS_HANDLE_EMERGENCY_MTC_H_

#include "handle/AosHandle.h"

class AosHandleEmergencyMtc : public AosHandle
{
public:
    AosHandleEmergencyMtc(IN IAosAppContext* piAppContext, IN const AString& strAppId,
            IN const AString& strServiceId, IN const IMS_SINT32 nServiceType);
    virtual ~AosHandleEmergencyMtc();

private:
    friend class AosHandleEmergencyMtcTest;
};
#endif  // AOS_HANDLE_EMERGENCY_MTC_H_
