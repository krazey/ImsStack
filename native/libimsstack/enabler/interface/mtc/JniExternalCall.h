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

#include "AString.h"
#include "ImsTypeDef.h"
#include "ServiceTrace.h"

/**
 * @brief Represents the state of an external call.
 *
 * This data structure is used to pass information across the JNI boundary about a call that is
 * active on another one of a user's devices. It is used for Multi-Endpoint and Call-Pull
 * features.
 */
struct JniExternalCall
{
public:
    /**
     * @brief Default constructor.
     */
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

    /**
     * @brief Copy constructor.
     */
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

    /**
     * @brief Assignment operator.
     */
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

    /**
     * @brief Generates a string representation of the object for debugging.
     * @return An AString containing the object's state.
     */
    inline const AString ToString() const
    {
        AString strLog;
        strLog.Sprintf("CallId: %s, Address: %s, LocalAddress: %s, IsPullable: %s, CallState: %d, \
                CallType: %d",
                m_strCallId.GetStr(), m_strAddress.GetStr(), m_strLocalAddress.GetStr(),
                _TRACE_B_(m_bIsPullable), m_nCallState, m_nCallType);
        return strLog;
    }

public:
    /** @brief Call has been established. */
    static const IMS_UINT32 CALL_STATE_CONFIRMED = 1;
    /** @brief Call has been terminated. */
    static const IMS_UINT32 CALL_STATE_TERMINATED = 2;

    /** @brief A unique identifier for the external call. */
    AString m_strCallId;
    /** @brief The address of the remote party (e.g., phone number). */
    AString m_strAddress;
    /** @brief The address of the local party on the other device. */
    AString m_strLocalAddress;
    /** @brief Whether the call can be transferred (pulled) to the current device. */
    IMS_BOOL m_bIsPullable;
    /** @brief The current state of the call (e.g., CALL_STATE_CONFIRMED). */
    IMS_UINT32 m_nCallState;
    /** @brief The type of call (e.g., voice, video). */
    IMS_UINT32 m_nCallType;
    /** @brief Whether the call is currently on hold on the external device. */
    IMS_BOOL m_bIsHeld;
};

#endif
