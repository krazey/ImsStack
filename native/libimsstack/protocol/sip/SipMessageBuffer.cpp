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
#include "ServiceMemory.h"
#include "ServiceThread.h"
#include "SystemConfig.h"

#include "SipMessageBuffer.h"

PUBLIC
SipMessageBuffer::SipMessageBuffer() :
        RcObject(),
        m_ppBuffer(IMS_NULL)
{
    IMS_MEM_Memset(m_baBuffer, 0x00, MAX_MSG_SIZE);

    if (SystemConfig::IsMultiImsEnabled() || SystemConfig::IsMultiImsEnabledOnDssv())
    {
        IMS_SINT32 nSimCount = SystemConfig::GetMaxSimSlot();

        m_ppBuffer = new IMS_BYTE*[nSimCount];

        m_ppBuffer[0] = &(m_baBuffer[0]);

        for (IMS_SINT32 i = 1; i < nSimCount; ++i)
        {
            m_ppBuffer[i] = new IMS_BYTE[MAX_MSG_SIZE];
            IMS_MEM_Memset(m_ppBuffer[i], 0x00, MAX_MSG_SIZE);
        }
    }
}

PUBLIC
SipMessageBuffer::SipMessageBuffer(IN const SipMessageBuffer& other) :
        RcObject(other),
        m_ppBuffer(IMS_NULL)
{
    IMS_MEM_Memset(m_baBuffer, 0x00, MAX_MSG_SIZE);

    // NOTE: If reference count is not used, you MUST implement this copy constructor
}

PUBLIC VIRTUAL SipMessageBuffer::~SipMessageBuffer()
{
    if (m_ppBuffer != IMS_NULL)
    {
        IMS_SINT32 nSimCount = SystemConfig::GetMaxSimSlot();

        for (IMS_SINT32 i = 0; i < nSimCount; ++i)
        {
            if (m_ppBuffer[i] != IMS_NULL)
            {
                if (m_ppBuffer[i] != &(m_baBuffer[0]))
                {
                    delete[] m_ppBuffer[i];
                }
            }
        }

        delete[] m_ppBuffer;
    }
}

/**
 * @brief Returns a message buffer to form a SIP message (serialization).
 */
PUBLIC
IMS_BYTE* SipMessageBuffer::GetBuffer()
{
    if (SystemConfig::IsMultiImsEnabled() || SystemConfig::IsMultiImsEnabledOnDssv())
    {
        return GetBuffer(ThreadService::GetCurrentSlotId(IMS_SLOT_0));
    }

    return &(m_baBuffer[0]);
}

/**
 * @brief Returns a message buffer to form a SIP message (serialization) for given slot-id.
 */
PUBLIC
IMS_BYTE* SipMessageBuffer::GetBuffer(IN IMS_SINT32 nSlotId)
{
    if (SystemConfig::IsMultiImsEnabled() || SystemConfig::IsMultiImsEnabledOnDssv())
    {
        if (m_ppBuffer != IMS_NULL)
        {
            if ((nSlotId < IMS_SLOT_0) || (nSlotId >= SystemConfig::GetMaxSimSlot()))
            {
                nSlotId = IMS_SLOT_0;
            }

            if (m_ppBuffer[nSlotId] != IMS_NULL)
            {
                return m_ppBuffer[nSlotId];
            }
        }
    }

    return &(m_baBuffer[0]);
}

/**
 * @brief Returns a message buffer to form a SIP message (serialization).
 */
PUBLIC GLOBAL RcPtr<SipMessageBuffer> SipMessageBuffer::GetInstance()
{
    static SipMessageBuffer* s_pMessageBuffer = IMS_NULL;

    if (s_pMessageBuffer == IMS_NULL)
    {
        s_pMessageBuffer = new SipMessageBuffer();

        s_pMessageBuffer->AddReference();
    }

    return s_pMessageBuffer;
}
