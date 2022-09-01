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

#include "ImsTypeDef.h"
#include "helper/SrvccStateManager.h"
#include "ServiceTrace.h"
#include "helper/ISrvccStateListener.h"
#include "ImsList.h"

__IMS_TRACE_TAG_COM_MTC__;

PUBLIC
SrvccStateManager::SrvccStateManager() :
        m_eState(SrvccState::IDLE),
        m_objListeners(IMSList<ISrvccStateListener*>())
{
    IMS_TRACE_I("+SrvccStateManager", 0, 0, 0);
}

PUBLIC
SrvccStateManager::~SrvccStateManager()
{
    IMS_TRACE_I("~SrvccStateManager", 0, 0, 0);
    m_objListeners.Clear();
}

PUBLIC
void SrvccStateManager::AddListener(IN ISrvccStateListener* piListener)
{
    for (IMS_UINT32 i = 0; i < m_objListeners.GetSize(); i++)
    {
        if (m_objListeners.GetAt(i) == piListener)
        {
            return;
        }
    }
    m_objListeners.Append(piListener);
}

PUBLIC
void SrvccStateManager::RemoveListener(IN ISrvccStateListener* piListener)
{
    for (IMS_UINT32 i = 0; i < m_objListeners.GetSize(); i++)
    {
        if (m_objListeners.GetAt(i) == piListener)
        {
            m_objListeners.RemoveAt(i);
            return;
        }
    }
}

PUBLIC
void SrvccStateManager::UpdateSrvccState(IN SrvccState eState)
{
    if (m_eState == eState)
    {
        IMS_TRACE_E(0, "State is not changed", 0, 0, 0);
        return;
    }

    m_eState = eState;

    NotifyListeners();

    if (eState != SrvccState::STARTED)
    {
        m_eState = SrvccState::IDLE;
    }
}

PRIVATE
void SrvccStateManager::NotifyListeners()
{
    for (IMS_UINT32 i = 0; i < m_objListeners.GetSize(); i++)
    {
        m_objListeners.GetAt(i)->OnSrvccStateUpdated(m_eState);
    }
}
