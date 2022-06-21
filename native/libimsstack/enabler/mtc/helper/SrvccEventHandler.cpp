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

#include "IMSTypeDef.h"
#include "helper/SrvccEventHandler.h"
#include "ServiceTrace.h"
#include "helper/ISrvccStateListener.h"
#include "IMSList.h"
#include "IMtcContext.h"

__IMS_TRACE_TAG_COM_MTC__;

PUBLIC
SrvccEventHandler::SrvccEventHandler(IN IMtcContext& objContext) :
        m_objContext(objContext),
        m_eState(SrvccState::IDLE),
        m_objListeners(IMSList<ISrvccStateListener*>())
{
    IMS_TRACE_I("+SrvccEventHandler", 0, 0, 0);
}

PUBLIC
SrvccEventHandler::~SrvccEventHandler()
{
    IMS_TRACE_I("~SrvccEventHandler", 0, 0, 0);
    m_objListeners.Clear();
}

PUBLIC
void SrvccEventHandler::AddListener(IN ISrvccStateListener* piListener)
{
    // no duplication check.
    m_objListeners.Append(piListener);
}

PUBLIC
void SrvccEventHandler::RemoveListener(IN ISrvccStateListener* piListener)
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
void SrvccEventHandler::UpdateSrvccState(IN SrvccState eState)
{
    if (m_eState == eState)
    {
        IMS_TRACE_E(0, "State is not chnged", 0, 0, 0);
        return;
    }

    m_eState = eState;

    NotifyListeners();
    HandleCalls();
}

PRIVATE
void SrvccEventHandler::NotifyListeners()
{
    for (IMS_UINT32 i = 0; i < m_objListeners.GetSize(); i++)
    {
        m_objListeners.GetAt(i)->OnStateUpdated(m_eState);
    }
}

PRIVATE
void SrvccEventHandler::HandleCalls()
{
    switch (m_eState)
    {
        case SrvccState::SUCCEEDED:
            m_objContext.GetCallController();  // avoid build error.
            // m_objContext.GetCallController().RemoveCalls(KeyType::NONE, Key::...);
            return;
        case SrvccState::CANCELED:
            // m_objContext.GetCallController().UpdateCalls(Reason SrvccCanceled...);
            return;
        case SrvccState::FAILED:
            // m_objContext.GetCallController().UpdateCalls(Reason SrvccFailed...);
            return;
        default:
            // do nothing
            return;
    }
}
