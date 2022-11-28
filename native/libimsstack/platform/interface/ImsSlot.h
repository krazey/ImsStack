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
#ifndef IMS_SLOT_H_
#define IMS_SLOT_H_

#include "ImsTypeDef.h"

class ImsSlot
{
public:
    inline explicit ImsSlot(IN IMS_SINT32 nSlotId) :
            m_nSlotId(nSlotId)
    {
    }
    inline ImsSlot(IN const ImsSlot& other) :
            m_nSlotId(other.m_nSlotId)
    {
    }
    inline virtual ~ImsSlot() {}

public:
    inline ImsSlot& operator=(IN const ImsSlot& other)
    {
        if (this != &other)
        {
            m_nSlotId = other.m_nSlotId;
        }

        return *this;
    }

public:
    inline IMS_SINT32 GetSlotId() const { return m_nSlotId; }

private:
    IMS_SINT32 m_nSlotId;
};

#endif
