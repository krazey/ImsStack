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
#include "ServiceMutex.h"
#include "ServiceTrace.h"

#include "ISipDialog.h"
#include "ISipServerConnection.h"
#include "util/CancellableMethodManager.h"
#include "util/ICancellableMethod.h"

__IMS_TRACE_TAG_IMS_CORE__;

PRIVATE
CancellableMethodManager::CancellableMethodManager() :
        m_piLock(IMS_NULL),
        m_objCancellableMethods(IMSMap<AString, ICancellableMethod*>())
{
    m_piLock = MutexService::GetMutexService()->CreateMutex();
}

PRIVATE
CancellableMethodManager::~CancellableMethodManager()
{
    {
        LockGuard objLock(m_piLock);
        m_objCancellableMethods.Clear();
    }

    MutexService::GetMutexService()->DestroyMutex(m_piLock);
}

PUBLIC
IMS_BOOL CancellableMethodManager::AddMethod(
        IN const AString& strName, IN ICancellableMethod* piMethod)
{
    LockGuard objLock(m_piLock);

    return m_objCancellableMethods.SetValue(strName, piMethod);
}

PUBLIC
void CancellableMethodManager::RemoveMethod(IN const AString& strName)
{
    LockGuard objLock(m_piLock);

    m_objCancellableMethods.Remove(strName);
}

PUBLIC GLOBAL CancellableMethodManager* CancellableMethodManager::GetInstance()
{
    static CancellableMethodManager* s_pCancellableMethodMngr = IMS_NULL;

    if (s_pCancellableMethodMngr == IMS_NULL)
    {
        s_pCancellableMethodMngr = new CancellableMethodManager();
    }

    return s_pCancellableMethodMngr;
}

PRIVATE
IMS_BOOL CancellableMethodManager::HandleCancelRequest(IN ISipServerConnection* piSsc)
{
    LockGuard objLock(m_piLock);

    if (m_objCancellableMethods.IsEmpty())
    {
        IMS_TRACE_D("There is no method to handle SIP CANCEL request", 0, 0, 0);
        return IMS_FALSE;
    }

    ICancellableMethod* piMethod = IMS_NULL;

    for (IMS_UINT32 i = 0; i < m_objCancellableMethods.GetSize(); ++i)
    {
        piMethod = m_objCancellableMethods.GetValueAt(i);

        if (piMethod->Cancellable_Compare(piSsc))
        {
            break;
        }

        piMethod = IMS_NULL;
    }

    if (piMethod == IMS_NULL)
    {
        IMS_TRACE_D("No matched cancellable method", 0, 0, 0);
        return IMS_FALSE;
    }

    if (!piMethod->Cancellable_NotifyRequest(piSsc))
    {
        IMS_TRACE_E(0, "Handling an incoming CANCEL request failed", 0, 0, 0);
    }

    return IMS_TRUE;
}
