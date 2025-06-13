/*
 * Copyright (C) 2025 The Android Open Source Project
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

#include "IPhoneInfoLocation.h"
#include "ServiceTrace.h"
#include "helper/MtcLocationRefresher.h"

__IMS_TRACE_TAG_COM_MTC__;

PUBLIC
MtcLocationRefresher::MtcLocationRefresher(IN ILocationInfo& objLocationInfo) :
        m_objLocationInfo(objLocationInfo),
        m_eState(LocationRefreshState::IDLE),
        m_lstListeners(ImsList<ILocationUpdateListener*>())
{
}

PUBLIC
MtcLocationRefresher::~MtcLocationRefresher()
{
    if (m_eState == LocationRefreshState::REFRESHING)
    {
        m_objLocationInfo.CancelLocationUpdate(this);
    }
}

PUBLIC
void MtcLocationRefresher::RequestUpdate(IN IMS_SINT32 nWaitTimeInMillis)
{
    if (m_eState == LocationRefreshState::REFRESHING)
    {
        IMS_TRACE_I("RequestUpdate : Already refreshing", 0, 0, 0);
        return;
    }

    m_eState = LocationRefreshState::REFRESHING;
    m_objLocationInfo.RequestLocationUpdate(nWaitTimeInMillis, this);
}

PUBLIC
void MtcLocationRefresher::AddListener(IN ILocationUpdateListener& objListener)
{
    if (m_lstListeners.Contains(&objListener))
    {
        IMS_TRACE_I("AddListener : Already added", 0, 0, 0);
        return;
    }
    m_lstListeners.Append(&objListener);
}

PUBLIC
void MtcLocationRefresher::RemoveListener(IN ILocationUpdateListener& objListener)
{
    m_lstListeners.Remove(&objListener);
}

PUBLIC
void MtcLocationRefresher::LocationUpdate_OnCompleted()
{
    m_eState = LocationRefreshState::IDLE;
    NotifyListeners();
}

PRIVATE
void MtcLocationRefresher::NotifyListeners()
{
    for (IMS_SINT32 i = static_cast<IMS_SINT32>(m_lstListeners.GetSize()) - 1; i >= 0; i--)
    {
        m_lstListeners.GetAt(i)->LocationUpdate_OnCompleted();
    }
}
