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
#ifndef IMS_THREAD_H_
#define IMS_THREAD_H_

#include "IThread.h"

class ImsThread : public IThread
{
public:
    inline ImsThread() :
            m_nSlotId(IMS_SLOT_ANY)
    {
    }
    ~ImsThread() override = default;

public:
    // IThread class
    inline IMS_SINT32 GetSlotId() const override { return m_nSlotId; }

    virtual IMS_BOOL Create(IN const AString& strName) = 0;
    virtual IMS_ULONG GetThreadId() const = 0;

    inline IMS_BOOL Create(IN const AString& strName, IN IMS_SINT32 nSlotId)
    {
        m_nSlotId = nSlotId;
        return Create(strName);
    }

    inline IMS_SINT32 RemoveMessages(IN ImsMessage::IMessageCallback* /*piCallback*/,
            OUT ImsList<ImsMessage>* /*pImsMsgs = IMS_NULL*/) override
    {
        return 0;
    }

private:
    IMS_SINT32 m_nSlotId;
};

#endif
