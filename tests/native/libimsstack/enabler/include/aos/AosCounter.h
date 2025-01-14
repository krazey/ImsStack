/*
 * Copyright (C) 2024 The Android Open Source Project
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
#ifndef AOS_COUNTER_H_
#define AOS_COUNTER_H_

#include "AString.h"
#include "ImsMap.h"

class AosCounter
{
public:
    inline AosCounter() :
            m_objCountMap(ImsMap<AString, IMS_UINT32>())
    {
    }

    inline void AddCount(IN const AString& strName)
    {
        if (m_objCountMap.GetIndexOfKey(strName) < 0)
        {
            m_objCountMap.Add(strName, 1);
        }
        else
        {
            m_objCountMap.GetValue(strName)++;
        }
    }

    inline IMS_UINT32 GetCount(IN const AString& strName)
    {
        return (m_objCountMap.GetIndexOfKey(strName) >= 0) ? m_objCountMap.GetValue(strName) : 0;
    }

private:
    ImsMap<AString, IMS_UINT32> m_objCountMap;
};

#endif  // AOS_COUNTER_H_
