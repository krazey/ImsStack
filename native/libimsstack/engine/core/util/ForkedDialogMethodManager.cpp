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
#include "util/ForkedDialogMethodManager.h"
#include "util/IForkedDialogMethod.h"

__IMS_TRACE_TAG_IMS_CORE__;

PRIVATE
ForkedDialogMethodManager::ForkedDialogMethodManager() :
        m_piLock(IMS_NULL),
        m_objDialogMethods(ImsMap<AString, IForkedDialogMethod*>())
{
    m_piLock = MutexService::GetMutexService()->CreateMutex();
}

PRIVATE
ForkedDialogMethodManager::~ForkedDialogMethodManager()
{
    {
        LockGuard objLock(m_piLock);
        m_objDialogMethods.Clear();
    }

    MutexService::GetMutexService()->DestroyMutex(m_piLock);
}

PUBLIC
IMS_BOOL ForkedDialogMethodManager::AddMethod(
        IN const AString& strName, IN IForkedDialogMethod* piMethod)
{
    LockGuard objLock(m_piLock);

    return m_objDialogMethods.SetValue(strName, piMethod);
}

PUBLIC
void ForkedDialogMethodManager::RemoveMethod(IN const AString& strName)
{
    LockGuard objLock(m_piLock);

    m_objDialogMethods.Remove(strName);
}

PUBLIC GLOBAL ForkedDialogMethodManager* ForkedDialogMethodManager::GetInstance()
{
    static ForkedDialogMethodManager* s_pForkedDialogMethodMngr = IMS_NULL;

    if (s_pForkedDialogMethodMngr == IMS_NULL)
    {
        s_pForkedDialogMethodMngr = new ForkedDialogMethodManager();
    }

    return s_pForkedDialogMethodMngr;
}

PRIVATE
IMS_BOOL ForkedDialogMethodManager::HandleRequestWithinDialog(
        IN ISipServerConnection* piSsc, IN ISipDialog* piOrigDialog)
{
    LockGuard objLock(m_piLock);

    if (m_objDialogMethods.IsEmpty())
    {
        IMS_TRACE_D("There is no method to handle SIP request within a dialog (2nd)", 0, 0, 0);
        return IMS_FALSE;
    }

    IForkedDialogMethod* piMethod = IMS_NULL;

    for (IMS_UINT32 i = 0; i < m_objDialogMethods.GetSize(); ++i)
    {
        piMethod = m_objDialogMethods.GetValueAt(i);

        if (piMethod->ForkedDialog_Compare(piOrigDialog))
        {
            break;
        }

        piMethod = IMS_NULL;
    }

    if (piMethod == IMS_NULL)
    {
        IMS_TRACE_E(0, "No matched dialog method", 0, 0, 0);
        return IMS_FALSE;
    }

    if (!piMethod->ForkedDialog_NotifyRequest(piSsc))
    {
        IMS_TRACE_E(0, "Handling an incoming SIP forked request within a dialog failed", 0, 0, 0);
    }

    return IMS_TRUE;
}
