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

#include "base/Method.h"
#include "util/MethodManager.h"

PUBLIC
MethodManager::MethodManager() :
        RCObject(),
        m_objMethods(IMSList<Method*>())
{
}

PUBLIC
MethodManager::MethodManager(IN const MethodManager& other) :
        RCObject(other),
        m_objMethods(other.m_objMethods)
{
}

PUBLIC
MethodManager::~MethodManager() {}

PUBLIC
IMS_BOOL MethodManager::AddMethod(IN Method* pMethod)
{
    for (IMS_UINT32 i = 0; i < m_objMethods.GetSize(); ++i)
    {
        const Method* pTempMethod = m_objMethods.GetAt(i);

        if (pTempMethod->Equals(pMethod))
        {
            return IMS_TRUE;
        }
    }

    // If not found, adds a new method ...

    return m_objMethods.Append(pMethod);
}

PUBLIC
void MethodManager::RemoveMethod(IN Method* pMethod)
{
    for (IMS_UINT32 i = 0; i < m_objMethods.GetSize(); ++i)
    {
        const Method* pTempMethod = m_objMethods.GetAt(i);

        if (pTempMethod->Equals(pMethod))
        {
            m_objMethods.RemoveAt(i);
            return;
        }
    }
}
