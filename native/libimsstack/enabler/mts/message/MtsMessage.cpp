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

#include "IPageMessage.h"
#include "MtsStringDef.h"
#include "ServiceTrace.h"
#include "message/MtsMessage.h"

__IMS_TRACE_TAG_COM_MTS__;

PUBLIC
MtsMessage::MtsMessage(IN IMS_SINT32 nSlotId) :
        m_piPageMessage(IMS_NULL),
        m_strDestination(AString::ConstNull()),
        m_strImpu(AString::ConstNull()),
        m_nMrOfRp(-1),
        m_nMti(SMS_MTI_NONE),
        m_nRetryCount(0),
        m_nSeqId(-1),
        m_nSlotId(nSlotId),
        m_nSmSize(0),
        m_eSmsFormat(SmsFormatType::SMSFORMAT_INVALID),
        m_eTransactionType(MtsTransactionType::MESSAGE_TYPE_INVALID)
{
    IMS_TRACE_I("+MtsMessage [slot_%d]", m_nSlotId, 0, 0);
}

PUBLIC
MtsMessage::~MtsMessage()
{
    IMS_TRACE_I("~MtsMessage [slot_%d]", m_nSlotId, 0, 0);

    if (m_piPageMessage != IMS_NULL)
    {
        m_piPageMessage->Destroy();
        m_piPageMessage = IMS_NULL;
    }
    else
    {
        IMS_TRACE_E(0, "IPageMessage is null", 0, 0, 0);
    }
}

PUBLIC
void MtsMessage::PrintInfo()
{
    if (m_eSmsFormat == SmsFormatType::SMSFORMAT_3GPP)
    {
        IMS_TRACE_I("PrintInfo : 3GPP(%s), RP_MR(%d), DataSize(%d)", PS_MtiStringFrom3gpp(m_nMti),
                m_nMrOfRp, m_nSmSize);
    }
    else if (m_eSmsFormat == SmsFormatType::SMSFORMAT_3GPP2)
    {
        IMS_TRACE_I(
                "PrintInfo : 3GPP2(%s), DataSize(%d)", PS_MtiStringFrom3gpp2(m_nMti), m_nSmSize, 0);
    }
    else
    {
        IMS_TRACE_E(0, "PrintInfo : SMS Format INFO INVALID", 0, 0, 0);
    }
}
