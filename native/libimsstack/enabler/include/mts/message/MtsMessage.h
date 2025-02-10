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

#ifndef MTS_MESSAGE_H_
#define MTS_MESSAGE_H_

#include "message/IMtsMessage.h"

class MtsMessage final : public IMtsMessage
{
public:
    explicit MtsMessage(IN IMS_SINT32 nSlotId);
    ~MtsMessage();

    // IMtsMessage
    inline AString& GetDestination() override { return m_strDestination; }
    inline void SetDestination(IN const AString& strDestination) override
    {
        m_strDestination = strDestination;
    }
    inline AString& GetImpu() override { return m_strImpu; }
    inline void SetImpu(IN const AString& strImpu) override { m_strImpu = strImpu; }
    inline IMS_SINT32 GetMessageReference() override { return m_nMrOfRp; }
    inline void SetMessageReference(IN IMS_SINT32 nMrOfRp) override { m_nMrOfRp = nMrOfRp; }
    inline IMS_SINT32 GetMti() override { return m_nMti; }
    inline void SetMti(IN IMS_SINT32 nMti) override { m_nMti = nMti; }
    inline IPageMessage* GetPageMessage() const override { return m_piPageMessage; }
    inline void SetPageMessage(IN IPageMessage* piPageMessage) override
    {
        m_piPageMessage = piPageMessage;
    }
    inline IMS_SINT32 GetSeqId() override { return m_nSeqId; }
    inline void SetSeqId(IN IMS_SINT32 nSeqId) override { m_nSeqId = nSeqId; }
    inline IMS_SINT32 GetSlotId() override { return m_nSlotId; }
    inline void SetSlotId(IN IMS_SINT32 nSlotId) override { m_nSlotId = nSlotId; }
    inline SmsFormatType GetSmsFormat() override { return m_eSmsFormat; }
    inline void SetSmsFormat(IN SmsFormatType eSmsFormat) override { m_eSmsFormat = eSmsFormat; }
    inline IMS_SINT32 GetSmSize() override { return m_nSmSize; }
    inline void SetSmSize(IN IMS_SINT32 nSmSize) override { m_nSmSize = nSmSize; }
    inline MtsTransactionType GetTransactionType() override { return m_eTransactionType; }
    inline void SetTransactionType(IN MtsTransactionType eTransactionType) override
    {
        m_eTransactionType = eTransactionType;
    }

    void PrintInfo() override;

private:
    IPageMessage* m_piPageMessage;
    AString m_strDestination;
    AString m_strImpu;
    IMS_SINT32 m_nMrOfRp;
    IMS_SINT32 m_nMti;
    IMS_SINT32 m_nSeqId;
    IMS_SINT32 m_nSlotId;
    IMS_SINT32 m_nSmSize;
    SmsFormatType m_eSmsFormat;
    MtsTransactionType m_eTransactionType;
};

#endif
