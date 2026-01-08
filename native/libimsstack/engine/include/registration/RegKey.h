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
#ifndef REG_KEY_H_
#define REG_KEY_H_

#include "ImsTypeDef.h"

class RegKey
{
public:
    inline RegKey() :
            m_nSlotId(IMS_SLOT_ANY),
            m_nFlowId(0)
    {
    }
    inline RegKey(IN IMS_SINT32 nSlotId, IN IMS_UINT32 nFlowId) :
            m_nSlotId(nSlotId),
            m_nFlowId(nFlowId)
    {
    }
    inline RegKey(IN const RegKey& other) :
            m_nSlotId(other.m_nSlotId),
            m_nFlowId(other.m_nFlowId)
    {
    }
    ~RegKey() = default;

public:
    inline RegKey& operator=(IN const RegKey& other)
    {
        if (this != &other)
        {
            m_nFlowId = other.m_nFlowId;
            m_nSlotId = other.m_nSlotId;
        }

        return (*this);
    }

    // For ImsMap class
    inline IMS_BOOL operator<(IN const RegKey& other)
    {
        return GetHashCode() < other.GetHashCode();
    }

public:
    inline IMS_BOOL Equals(IN const RegKey& other) const
    {
        return (m_nSlotId == other.m_nSlotId) && (m_nFlowId == other.m_nFlowId);
    }
    inline IMS_UINT32 GetHashCode() const { return (m_nSlotId * 997) + (m_nFlowId * 13); }

    inline IMS_UINT32 GetFlowId() const { return m_nFlowId; }
    inline IMS_SINT32 GetSlotId() const { return m_nSlotId; }

    inline void Invalidate()
    {
        m_nSlotId = IMS_SLOT_ANY;
        m_nFlowId = 0;
    }

private:
    IMS_SINT32 m_nSlotId;
    IMS_UINT32 m_nFlowId;
};

inline IMS_BOOL operator==(IN const RegKey& objK1, IN const RegKey& objK2)
{
    return objK1.Equals(objK2);
}

inline IMS_BOOL operator!=(IN const RegKey& objK1, IN const RegKey& objK2)
{
    return !objK1.Equals(objK2);
}

inline IMS_BOOL operator<(IN const RegKey& objK1, IN const RegKey& objK2)
{
    return objK1.GetHashCode() < objK2.GetHashCode();
}

#endif
