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
#ifndef SIP_MESSAGE_INFO_H_
#define SIP_MESSAGE_INFO_H_

#include "SipStackHeaders.h"

/**
 * @brief This class includes an information of the current processing SIP Message.
 */
class SipMessageInfo
{
public:
    inline SipMessageInfo(IN IMS_SINT32 nSlotId, IN const SipMethod& objMethod,
            IN ::SipMessage* pSipMsg, IN IMS_SINT32 nDirection) :
            m_nSlotId(nSlotId),
            m_objMethod(objMethod),
            m_nDirection(nDirection),
            m_pSipMsg(pSipMsg)
    {
    }

    inline ~SipMessageInfo() {}

    SipMessageInfo(IN const SipMessageInfo&) = delete;
    SipMessageInfo& operator=(IN const SipMessageInfo&) = delete;

public:
    inline IMS_SINT32 GetDirection() const { return m_nDirection; }
    inline ::SipMessage* GetMessage() const { return m_pSipMsg; }
    inline const SipMethod& GetMethod() const { return m_objMethod; }
    inline IMS_SINT32 GetSlotId() const { return m_nSlotId; }
    inline IMS_BOOL IsOutgoingMessage() const { return (m_nDirection == DIRECTION_OUTGOING); }

public:
    enum
    {
        DIRECTION_OUTGOING = 0,
        DIRECTION_INCOMING
    };

private:
    IMS_SINT32 m_nSlotId;
    const SipMethod& m_objMethod;
    IMS_SINT32 m_nDirection;
    ::SipMessage* m_pSipMsg;
};

#endif
