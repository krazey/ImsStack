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

#ifndef JNI_EXTERNAL_CALL_H_
#define JNI_EXTERNAL_CALL_H_

#include "ImsTypeDef.h"

struct JniExternalCall
{
public:
    JniExternalCall() :
            m_strCallId(AString::ConstNull()),
            m_strAddress(AString::ConstNull()),
            m_strLocalAddress(AString::ConstNull()),
            m_bIsPullable(IMS_FALSE),
            m_nCallState(0),
            m_nCallType(0),
            m_bIsHeld(IMS_FALSE)
    {
    }

    JniExternalCall(IN const JniExternalCall& objRhs) :
            m_strCallId(objRhs.m_strCallId),
            m_strAddress(objRhs.m_strAddress),
            m_strLocalAddress(objRhs.m_strLocalAddress),
            m_bIsPullable(objRhs.m_bIsPullable),
            m_nCallState(objRhs.m_nCallState),
            m_nCallType(objRhs.m_nCallType),
            m_bIsHeld(objRhs.m_bIsHeld)
    {
    }

    JniExternalCall& operator=(IN const JniExternalCall& objRhs)
    {
        if (this != &objRhs)
        {
            m_strCallId = objRhs.m_strCallId;
            m_strAddress = objRhs.m_strAddress;
            m_strLocalAddress = objRhs.m_strLocalAddress;
            m_bIsPullable = objRhs.m_bIsPullable;
            m_nCallState = objRhs.m_nCallState;
            m_nCallType = objRhs.m_nCallType;
            m_bIsHeld = objRhs.m_bIsHeld;
        }

        return *this;
    }

public:
    AString m_strCallId;
    AString m_strAddress;
    AString m_strLocalAddress;
    IMS_BOOL m_bIsPullable;
    IMS_UINT32 m_nCallState;
    IMS_UINT32 m_nCallType;
    IMS_BOOL m_bIsHeld;
};

#endif
