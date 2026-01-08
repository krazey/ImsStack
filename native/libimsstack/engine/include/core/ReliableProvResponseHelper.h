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
#ifndef RELIABLE_PROV_RESPONSE_HELPER_H_
#define RELIABLE_PROV_RESPONSE_HELPER_H_

#include "SipMethod.h"

class ReliableProvResponseHelper
{
public:
    explicit ReliableProvResponseHelper(IN IMS_BOOL bIsMobileOriginated);
    ~ReliableProvResponseHelper();

    ReliableProvResponseHelper(IN const ReliableProvResponseHelper&) = delete;
    ReliableProvResponseHelper& operator=(IN const ReliableProvResponseHelper&) = delete;

public:
    inline IMS_SINT32 GetState() const { return m_nState; }
    void Initialize(IN const ISipMessage* piSipMsg);
    IMS_BOOL SetRAckHeader(IN_OUT ISipMessage*& piSipMsg) const;
    IMS_BOOL SetRSeqHeader(IN_OUT ISipMessage*& piSipMsg) const;
    IMS_BOOL UpdateOnMessageReceived(IN const ISipMessage* piSipMsg);
    IMS_BOOL UpdateOnMessageSent(IN const ISipMessage* piSipMsg);
    IMS_BOOL UpdateOnOperationFailed();

private:
    void SetState(IN IMS_SINT32 nState);

    static const IMS_CHAR* StateToString(IN IMS_SINT32 nState);

public:
    enum
    {
        STATE_IDLE = 0,

        /// UAC behavior
        STATE_RPR_RECEIVED,
        STATE_PRACK_SENT,

        /// UAS behavior
        STATE_RPR_SENT,
        STATE_PRACK_RECEIVED,

        STATE_MAX
    };

private:
    IMS_BOOL m_bIsMobileOriginated;
    IMS_SINT32 m_nState;
    // The first value will selectes between 1 and 2^31 - 1
    IMS_UINT32 m_nRSeqNumber;
    // CSeq number
    IMS_UINT32 m_nCSeqNumber;
    SipMethod m_objMethod;
};

#endif
