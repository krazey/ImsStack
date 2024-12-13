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
#include "transport/SipTransportInfo.h"

SipTransportInfo::SipTransportInfo(
        SipTransportParameter* pTranspParam, SipTransportBuffer* pTransSipBuffer) :
        m_cNumTimeReqSent(SIP_ZERO),
        m_pActualDestParam(SIP_NULL),
        m_pTranspParam(SIP_NULL),
        m_pSentBuffer(SIP_NULL),
        m_pSentSipMsg(SIP_NULL)
{
    if (pTranspParam == SIP_NULL)
    {
        return;
    }
    m_pTranspParam = new SipTransportParameter(pTranspParam);
    m_pSentBuffer = pTransSipBuffer;
}

SipTransportInfo::~SipTransportInfo()
{
    if (m_pActualDestParam != SIP_NULL)
    {
        delete m_pActualDestParam;
        m_pActualDestParam = SIP_NULL;
    }

    if (m_pTranspParam != SIP_NULL)
    {
        delete m_pTranspParam;
    }

    if (m_pSentBuffer != SIP_NULL)
    {
        delete m_pSentBuffer;
    }

    if (m_pSentSipMsg != SIP_NULL)
    {
        m_pSentSipMsg->SipDelete();
    }
}

SipTransportParameter* SipTransportInfo::GetMsgSentTranspParam()
{
    return m_pActualDestParam;
}

SipTransportBuffer* SipTransportInfo::GetTranspSipBuffer()
{
    return m_pSentBuffer;
}

SipMessage* SipTransportInfo::GetSentSipMsg()
{
    return m_pSentSipMsg;
}

SIP_VOID SipTransportInfo::SetSentSipMsg(SipMessage* _pSipMsg)
{
    if (m_pSentSipMsg != SIP_NULL)
    {
        m_pSentSipMsg->SipDelete();
        m_pSentSipMsg = SIP_NULL;
    }
    m_pSentSipMsg = _pSipMsg;
}
