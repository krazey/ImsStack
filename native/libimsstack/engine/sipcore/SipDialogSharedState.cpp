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
#include "ServiceTrace.h"

#include "SipDebug.h"
#include "SipDialogEx.h"
#include "SipDialogSharedState.h"
#include "SipManager.h"
#include "SipPrivate.h"

__IMS_TRACE_TAG_SIP_CORE__;

PUBLIC
SipDialogSharedState::~SipDialogSharedState()
{
#ifdef __IMS_SIP_DEBUG__
    IMS_TRACE_D("dtor: SipDialogSharedState (state=%d)", m_nSharedState, 0, 0);
#endif
}

PRIVATE
IMS_BOOL SipDialogSharedState::AddDialog(IN SipDialogEx* pDialogEx)
{
    if (pDialogEx == IMS_NULL)
    {
        return IMS_FALSE;
    }

    if (m_nSharedState == SHARED_STATE_TERMINATED)
    {
        IMS_TRACE_D("INVALID STATE : adding a dialog usage for dialog (%s)",
                SipDebug::GetCharA1(pDialogEx->GetDialogState()->GetCallId().GetStr(), 8, '@'), 0,
                0);
        return IMS_FALSE;
    }

    if (!m_objDialogExs.Append(pDialogEx))
    {
        IMS_TRACE_E(0, "Adding a dialog usage for dialog (%s) failed",
                SipDebug::GetCharA1(pDialogEx->GetDialogState()->GetCallId().GetStr(), 8, '@'), 0,
                0);
        return IMS_FALSE;
    }

    if (m_nSharedState == SHARED_STATE_INIT)
    {
        m_nSharedState = SHARED_STATE_ACTIVE;

        // Attach a dialog state : make it a permanent dialog state
        SipManager::GetInstance()->AttachDialogState(pDialogEx->GetDialogState());
    }

    return IMS_TRUE;
}

PRIVATE
void SipDialogSharedState::RemoveDialog(IN SipDialogEx* pDialogEx)
{
    if (pDialogEx == IMS_NULL)
    {
        return;
    }

    if (m_nSharedState != SHARED_STATE_ACTIVE)
    {
        IMS_TRACE_D("INVALID STATE : removing a dialog usage for dialog (%s)",
                SipDebug::GetCharA1(pDialogEx->GetDialogState()->GetCallId().GetStr(), 8, '@'), 0,
                0);
        return;
    }

    for (IMS_UINT32 i = 0; i < m_objDialogExs.GetSize(); ++i)
    {
        const SipDialogEx* pTempDialogEx = m_objDialogExs.GetAt(i);

        if (pTempDialogEx->Equals(pDialogEx))
        {
            m_objDialogExs.RemoveAt(i);

            if (m_objDialogExs.IsEmpty())
            {
                m_nSharedState = SHARED_STATE_TERMINATED;

                // Detach a dialog state : dialog will be destroyed after a few minutes
                SipManager::GetInstance()->DetachDialogState(pDialogEx->GetDialogState());
            }

            return;
        }
    }
}

PRIVATE
SipDialogEx* SipDialogSharedState::GetDialog(IN const SipMessageInfo& objMsgInfo)
{
    if (m_nSharedState != SHARED_STATE_ACTIVE)
    {
        return IMS_NULL;
    }

    for (IMS_UINT32 i = 0; i < m_objDialogExs.GetSize(); ++i)
    {
        SipDialogEx* pDialogEx = m_objDialogExs.GetAt(i);

        if (pDialogEx->CompareTo(objMsgInfo))
        {
            return pDialogEx;
        }
    }

    return IMS_NULL;
}

PRIVATE
IMS_BOOL SipDialogSharedState::HasMultipleDialogUsages() const
{
    if (m_nSharedState != SHARED_STATE_ACTIVE)
    {
        return IMS_FALSE;
    }

    if (m_objDialogExs.GetSize() < 2)
    {
        return IMS_FALSE;
    }

    IMS_BOOL bHasInviteUsage = IMS_FALSE;
    IMS_BOOL bHasSubscribeUsage = IMS_FALSE;

    for (IMS_UINT32 i = 0; i < m_objDialogExs.GetSize(); ++i)
    {
        const SipDialogEx* pDialogEx = m_objDialogExs.GetAt(i);

        if (pDialogEx == IMS_NULL)
        {
            continue;
        }

        if (pDialogEx->IsInviteUsage())
        {
            bHasInviteUsage = IMS_TRUE;
        }
        else
        {
            bHasSubscribeUsage = IMS_TRUE;
        }
    }

    return (bHasInviteUsage && bHasSubscribeUsage);
}
