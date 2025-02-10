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

#ifndef INTERFACE_MTS_MESSAGE_H_
#define INTERFACE_MTS_MESSAGE_H_

#include "MtsDef.h"

class IPageMessage;

class IMtsMessage
{
public:
    virtual ~IMtsMessage(){};

    virtual AString& GetDestination() = 0;
    virtual void SetDestination(IN const AString& strDestination) = 0;
    virtual AString& GetImpu() = 0;
    virtual void SetImpu(IN const AString& strImpu) = 0;
    virtual IMS_SINT32 GetMessageReference() = 0;
    virtual void SetMessageReference(IN IMS_SINT32 nMrOfRp) = 0;
    virtual IMS_SINT32 GetMti() = 0;
    virtual void SetMti(IN IMS_SINT32 nMti) = 0;
    virtual IPageMessage* GetPageMessage() const = 0;
    virtual void SetPageMessage(IN IPageMessage* piPageMessage) = 0;
    virtual IMS_SINT32 GetSeqId() = 0;
    virtual void SetSeqId(IN IMS_SINT32 nSeqId) = 0;
    virtual IMS_SINT32 GetSlotId() = 0;
    virtual void SetSlotId(IN IMS_SINT32 nSlotId) = 0;
    virtual SmsFormatType GetSmsFormat() = 0;
    virtual void SetSmsFormat(IN SmsFormatType eSmsFormat) = 0;
    virtual IMS_SINT32 GetSmSize() = 0;
    virtual void SetSmSize(IN IMS_SINT32 nSmSize) = 0;
    virtual MtsTransactionType GetTransactionType() = 0;
    virtual void SetTransactionType(IN MtsTransactionType eTransactionType) = 0;

    virtual void PrintInfo() = 0;
};

#endif
