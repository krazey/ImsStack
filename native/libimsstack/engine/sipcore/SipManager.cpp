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

#include "SipClientConnectionImpl.h"
#include "SipConfigProxy.h"
#include "SipConnectionNotifier.h"
#include "SipConnectionNotifierImpl.h"
#include "SipDebug.h"
#include "SipFactoryProxy.h"
#include "SipManager.h"
#include "SipMessageHandler.h"
#include "SipPrivate.h"
#include "SipStack.h"
#include "SipStackState.h"
#include "SipTransportHelper.h"
#include "SipUtils.h"
#include "StaticSip.h"

__IMS_TRACE_TAG_SIP_CORE__;

PRIVATE
SipManager::SipManager() :
        m_nState(STATE_INACTIVE)
{
    SipPrivate::Init();
}

PRIVATE
SipManager::~SipManager()
{
    CleanUp();
}

PUBLIC
IMS_BOOL SipManager::AttachDialogState(IN SipDialogState* pDState)
{
    if (m_nState != STATE_ACTIVE)
    {
        IMS_TRACE_E(0, "SIP MANAGER IS NOT ACTIVE", 0, 0, 0);
        return IMS_FALSE;
    }

    if (pDState == IMS_NULL)
    {
        return IMS_FALSE;
    }

    IMS_TRACE_I("___ Attach: DialogState(%s)",
            SipDebug::GetCharA1(pDState->GetCallId().GetStr(), 8, '@'), 0, 0);

    return m_objDialogStates.Append(pDState);
}

PUBLIC
void SipManager::DetachDialogState(IN const SipDialogState* pDState)
{
    if (m_nState != STATE_ACTIVE)
    {
        IMS_TRACE_E(0, "SIP MANAGER IS NOT ACTIVE", 0, 0, 0);
        return;
    }

    for (IMS_UINT32 i = 0; i < m_objDialogStates.GetSize(); ++i)
    {
        const SipDialogState* pTempDState = m_objDialogStates.GetAt(i);

        if (pTempDState != IMS_NULL)
        {
            if (pTempDState == pDState)
            {
                IMS_TRACE_I("___ Detach: DialogState(%s)",
                        SipDebug::GetCharA1(pDState->GetCallId().GetStr(), 8, '@'), 0, 0);

                m_objDialogStates.RemoveAt(i);
                return;
            }
        }
    }
}

PUBLIC
RcPtr<SipDialogState> SipManager::LookupDialogState(IN SipDialogState* pDState,
        IN ::SipMessage* pSipMsg, IN IMS_BOOL bCheckForked /*= IMS_FALSE*/,
        OUT IMS_BOOL* pbIsForked /*= IMS_NULL*/)
{
    if (m_nState != STATE_ACTIVE)
    {
        IMS_TRACE_E(0, "SIP MANAGER IS NOT ACTIVE", 0, 0, 0);
        return IMS_NULL;
    }

    if (pbIsForked != IMS_NULL)
    {
        (*pbIsForked) = IMS_FALSE;
    }

    for (IMS_UINT32 i = 0; i < m_objDialogStates.GetSize(); ++i)
    {
        SipDialogState* pTempDState = m_objDialogStates.GetAt(i);

        if (pTempDState != IMS_NULL)
        {
            IMS_SINT32 nComparisonResult = pTempDState->CompareTo(pDState, pSipMsg, bCheckForked);

            switch (nComparisonResult)
            {
                case SipDialogState::MATCHED:
                    return pTempDState;
                case SipDialogState::MATCHED_FORKED_SUBSCRIBE:
                    if (pbIsForked != IMS_NULL)
                    {
                        (*pbIsForked) = IMS_TRUE;
                    }

                    IMS_TRACE_I("___ FORKED NOTIFY RECEIVED ___", 0, 0, 0);
                    return pTempDState;
                case SipDialogState::MATCHED_EARLY_NOTIFY:
                    IMS_TRACE_I("___ EARLY NOTIFY RECEIVED ___", 0, 0, 0);
                    return pTempDState;
                case SipDialogState::NOT_MATCHED:              // FALL-THROUGH
                case SipDialogState::MATCHED_DIFFERENT:        // FALL-THROUGH
                case SipDialogState::MATCHED_OVERLAP_DIALING:  // FALL-THROUGH
                default:
                    break;
            }
        }
    }

    return IMS_NULL;
}

PUBLIC
IMS_BOOL SipManager::AttachConnectionNotifier(IN SipConnectionNotifier* pScn)
{
    if (m_nState != STATE_ACTIVE)
    {
        IMS_TRACE_E(0, "SIP MANAGER IS NOT ACTIVE", 0, 0, 0);
        return IMS_FALSE;
    }

    return m_objScns.Append(pScn);
}

PUBLIC
void SipManager::DetachConnectionNotifier(IN const SipConnectionNotifier* pScn)
{
    if (m_nState != STATE_ACTIVE)
    {
        IMS_TRACE_E(0, "SIP MANAGER IS NOT ACTIVE", 0, 0, 0);
        return;
    }

    for (IMS_UINT32 i = 0; i < m_objScns.GetSize(); ++i)
    {
        const SipConnectionNotifier* pTempScn = m_objScns.GetAt(i);

        if (pTempScn != IMS_NULL)
        {
            if (pTempScn == pScn)
            {
                m_objScns.RemoveAt(i);
                return;
            }
        }
    }
}

PUBLIC
SipConnectionNotifier* SipManager::LookupConnectionNotifier(IN const SipTransportAddress& objTAddr,
        IN const AString& strFilter /*= AString::ConstNull()*/)
{
    (void)strFilter;

    if (m_nState != STATE_ACTIVE)
    {
        IMS_TRACE_E(0, "SIP MANAGER IS NOT ACTIVE", 0, 0, 0);
        return IMS_NULL;
    }

    for (IMS_UINT32 i = 0; i < m_objScns.GetSize(); ++i)
    {
        SipConnectionNotifier* pTempScn = m_objScns.GetAt(i);

        if (pTempScn != IMS_NULL)
        {
            if (pTempScn->IsSameConnectionNotifier(objTAddr))
            {
                return pTempScn;
            }
        }
    }

    return IMS_NULL;
}

PUBLIC GLOBAL SipManager* SipManager::GetInstance()
{
    static SipManager* s_pSipMngr = IMS_NULL;

    if (s_pSipMngr == IMS_NULL)
    {
        s_pSipMngr = new SipManager();
    }

    return s_pSipMngr;
}

PRIVATE
IMS_BOOL SipManager::StartUp()
{
    if (m_nState == STATE_ACTIVE)
    {
        IMS_TRACE_E(0, "SIP MANAGER IS ALREADY ACTIVE", 0, 0, 0);
        return IMS_TRUE;
    }

    if (m_nState == STATE_PENDING)
    {
        return IMS_FALSE;
    }

    // Initialize the SIP stack & transaction layer
    SipStackState::GetInstance()->StartUp();

    SipDebug::InitLogging();

    m_nState = STATE_ACTIVE;

    return IMS_TRUE;
}

PRIVATE
void SipManager::CleanUp()
{
    if ((m_nState == STATE_INACTIVE) || (m_nState == STATE_PENDING))
    {
        IMS_TRACE_E(0, "SIP MANAGER IS NOT ACTIVE", 0, 0, 0);
        return;
    }

    m_nState = STATE_PENDING;

    // Delete the dialog state info.
    IMS_TRACE_D("___ Number of DialogState (%d)", m_objDialogStates.GetSize(), 0, 0);

    if (!m_objDialogStates.IsEmpty())
    {
        for (IMS_UINT32 i = 0; i < m_objDialogStates.GetSize(); ++i)
        {
            SipDialogState* pDState = m_objDialogStates.GetAt(i);

            if (pDState != IMS_NULL)
            {
                pDState->RemoveReference();
            }
        }

        m_objDialogStates.Clear();
    }

    IMS_TRACE_D("___ Number of ConnectionNotifier (%d)", m_objScns.GetSize(), 0, 0);

    if (!m_objScns.IsEmpty())
    {
        for (IMS_UINT32 i = 0; i < m_objScns.GetSize(); ++i)
        {
            SipConnectionNotifier* pScn = m_objScns.GetAt(i);

            if (pScn != IMS_NULL)
            {
                delete pScn;
            }
        }

        m_objScns.Clear();
    }

    SipStackState::GetInstance()->CleanUp();

    m_nState = STATE_INACTIVE;
}

PUBLIC GLOBAL IMS_BOOL StaticSip::Initialize()
{
    if (!SipManager::GetInstance()->StartUp())
    {
        return IMS_FALSE;  // throw exception
    }

    IMS_TRACE_D(">>> SIP ENGINE IS SUCCESSFULLY LOADED <<<", 0, 0, 0);

    return IMS_TRUE;
}

PUBLIC GLOBAL void StaticSip::Uninitialize()
{
    SipManager::GetInstance()->CleanUp();

    IMS_TRACE_D(">>> SIP ENGINE IS SUCCESSFULLY UNLOADED <<<", 0, 0, 0);
}

PUBLIC GLOBAL void StaticSip::InitializeForSlot(IN IMS_SINT32 nSlotId)
{
    IMS_TRACE_D("InitializeForSlot: slotId=%d", nSlotId, 0, 0);

    IMS_SINT32 nOptions = SipPrivate::OPTIONS_E;

    if (SipConfigProxy::IsCompactFormConfigured(nSlotId))
    {
        nOptions |= SipPrivate::OPT_E_SHORTFORM;
    }
    else
    {
        nOptions |= SipPrivate::OPT_E_FULLFORM;
    }

    SipPrivate::Init(nSlotId, nOptions);

    SipUtils::Init(nSlotId);

    SipTransportHelper* pTransportHelper =
            SipFactoryProxy::GetInstance()->GetTransportHelper(nSlotId);

    // Attach the message handler from the network
    pTransportHelper->SetMessageListener(SipMessageHandler::GetInstance());
}

PUBLIC GLOBAL void StaticSip::UninitializeForSlot(IN IMS_SINT32 nSlotId)
{
    (void)nSlotId;

    IMS_TRACE_D("UninitializeForSlot: slotId=%d", nSlotId, 0, 0);
}
