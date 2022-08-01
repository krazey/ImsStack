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

#ifndef UCE_NON_CAPABILITY_USER_H_
#define UCE_NON_CAPABILITY_USER_H_

#include "AString.h"

class UceNonCapabilityUser
{
public:
    UceNonCapabilityUser(AString& strId, AString& strReason)
    {
        m_strId = strId;
        m_strReason = strReason;
    }
    virtual ~UceNonCapabilityUser(){};

    AString& GetId() { return m_strId; }
    AString& GetReason() { return m_strReason; }

private:
    AString m_strId;
    AString m_strReason;
};

#endif /* UCE_NON_CAPABILITY_USER_H_ */
