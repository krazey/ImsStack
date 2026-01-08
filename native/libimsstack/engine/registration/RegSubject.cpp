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

#include "RegSubject.h"

PUBLIC
RegSubject::RegSubject() {}

PUBLIC VIRTUAL RegSubject::~RegSubject() {}

PUBLIC VIRTUAL void RegSubject::RegisterObserver(IN RegObserver* pObserver)
{
    for (IMS_UINT32 i = 0; i < m_objObservers.GetSize(); ++i)
    {
        const RegObserver* pTmpObserver = m_objObservers.GetAt(i);

        if (pObserver == pTmpObserver)
        {
            return;
        }
    }

    m_objObservers.Append(pObserver);
}

PUBLIC VIRTUAL void RegSubject::RemoveObserver(IN RegObserver* pObserver)
{
    for (IMS_UINT32 i = 0; i < m_objObservers.GetSize(); ++i)
    {
        const RegObserver* pTmpObserver = m_objObservers.GetAt(i);

        if (pObserver == pTmpObserver)
        {
            m_objObservers.RemoveAt(i);
            return;
        }
    }
}

PROTECTED VIRTUAL void RegSubject::NotifyObservers(IN IMS_SINT32 nWhat)
{
    for (IMS_UINT32 i = 0; i < m_objObservers.GetSize(); ++i)
    {
        RegObserver* pObserver = m_objObservers.GetAt(i);

        if (pObserver == IMS_NULL)
        {
            continue;
        }

        pObserver->Update(nWhat);
    }
}
