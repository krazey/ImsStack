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
#include "ServiceEvent.h"
#include "ServiceTrace.h"
#include "interface/IAosAppContext.h"
#include "interface/IAosBlock.h"
#include "interface/IAosNetTracker.h"
#include "interface/IAosServiceAvailableListener.h"
#include "provider/AosProvider.h"
#include "provider/AosUtil.h"
#include "condition/AosBlock.h"
#include "condition/AosServiceAvailable.h"

__IMS_TRACE_TAG_AOS__;

#define AOSTAG m_strTag.GetStr()

PROTECTED
AosServiceAvailable::AosServiceAvailable(const AString& strName) :
        m_piAppContext(IMS_NULL),
        m_nSlotId(IMS_SLOT_0),
        m_piBlock(IMS_NULL),
        m_piRegistration(IMS_NULL),
        m_piConnection(IMS_NULL),
        m_piCallTracker(IMS_NULL),
        m_strName(strName),
        m_bAirplaneMode(IMS_FALSE),
        m_bRoamingState(IMS_FALSE),
        m_bAvailableLastNotified(IMS_FALSE),
        m_objBlockReasonsLastNotified(ImsList<IMS_UINT32>()),
        m_objListeners(ImsList<IAosServiceAvailableListener*>())
{
    IMS_TRACE_MEM("AOS_MEM", "AOS_M : AosServiceAvailable = %" PFLS_u "/%" PFLS_x,
            sizeof(AosServiceAvailable), this, 0);
}

PROTECTED VIRTUAL AosServiceAvailable::~AosServiceAvailable()
{
    IMS_TRACE_MEM("AOS_MEM", "AOS_F : AosServiceAvailable = %" PFLS_u "/%" PFLS_x,
            sizeof(AosServiceAvailable), this, 0);
}

PUBLIC
void AosServiceAvailable::Init(IN IAosAppContext* piAppContext)
{
    if (m_piAppContext != IMS_NULL)
    {
        return;
    }

    m_piAppContext = piAppContext;
    m_nSlotId = m_piAppContext->GetSlotId();
    m_piBlock = m_piAppContext->GetBlock();
    m_piRegistration = m_piAppContext->GetRegistration();
    m_piConnection = m_piAppContext->GetConnection();
    m_piCallTracker = AosProvider::GetInstance()->GetCallTracker(m_nSlotId);

    m_strTag.Sprintf("%d", m_nSlotId);

    A_IMS_TRACE_D(AOSTAG, "Init", 0, 0, 0);

    RegisterListener();
}

PUBLIC
void AosServiceAvailable::CleanUp()
{
    A_IMS_TRACE_D(AOSTAG, "CleanUp", 0, 0, 0);

    m_objBlockReasonsLastNotified.Clear();
    m_objListeners.Clear();

    DeregisterListener();

    StopToCheckNetworkConnection(IMS_FALSE);

    m_piAppContext = IMS_NULL;
}

PUBLIC
void AosServiceAvailable::SetListener(IN IAosServiceAvailableListener* piListener)
{
    A_IMS_TRACE_D(AOSTAG, "SetListener :: (%" PFLS_x ") is set", piListener, 0, 0);

    if (piListener == IMS_NULL)
    {
        return;
    }

    for (IMS_UINT32 nIndex = 0; nIndex < m_objListeners.GetSize(); nIndex++)
    {
        const IAosServiceAvailableListener* piTemp = m_objListeners.GetAt(nIndex);

        if (piTemp == piListener)
        {
            A_IMS_TRACE_D(AOSTAG, "Already registered listener", 0, 0, 0);
            return;
        }
    }

    m_objListeners.Append(piListener);
}

PUBLIC
void AosServiceAvailable::RemoveListener(IN IAosServiceAvailableListener* piListener)
{
    A_IMS_TRACE_D(AOSTAG, "RemoveListener :: (%" PFLS_x ") is set", piListener, 0, 0);

    if (piListener == IMS_NULL)
    {
        return;
    }

    for (IMS_UINT32 nIndex = 0; nIndex < m_objListeners.GetSize(); nIndex++)
    {
        const IAosServiceAvailableListener* piTemp = m_objListeners.GetAt(nIndex);

        if (piTemp == piListener)
        {
            m_objListeners.RemoveAt(nIndex);
            return;
        }
    }
}

PUBLIC
void AosServiceAvailable::RefreshServiceAvailability()
{
    A_IMS_TRACE_I(AOSTAG, "RefreshServiceAvailability", 0, 0, 0);
    Notify();
}

PUBLIC
IMS_BOOL AosServiceAvailable::IsAvailable()
{
    return m_bAvailableLastNotified;
}

PUBLIC
void AosServiceAvailable::HandleEvent(
        IN IMS_UINT32 eEvent, IN IMS_UINT32 nState, IN IMS_SINT32 nStateEx)
{
    A_IMS_TRACE_I(
            AOSTAG, "HandleEvent :: E(%s)/S1(%d)/S2(%d)", EventToString(eEvent), nState, nStateEx);

    IMS_BOOL bNotify = IMS_TRUE;

    switch (eEvent)
    {
        case EVENT_CALL:
            HandleCallStateChanged(nState, nStateEx);
            break;

        case EVENT_NETWORK:
            HandleNetworkStateChanged();
            break;

        case EVENT_BLOCK:
            HandleBlockChanged(nState, nStateEx);
            break;

        case EVENT_BLOCK_SILENT:
            HandleBlockChanged(nState, nStateEx);
            bNotify = IMS_FALSE;
            break;

        case EVENT_AIRPLANE:
            HandleAirplaneModeChanged(nState);
            break;

        case EVENT_ROAMING:
            HandleRoamingChanged(nState);
            break;

        case EVENT_VOPS:
            HandleVopsChanged(nState);
            break;

        case EVENT_LOCATION:
            HandleLocationInfoChanged();
            break;

        case EVENT_WIFI_STATE:
            HandleWifiConnectionChanged();
            break;

        default:
            break;
    }

    Notify(bNotify);
}

PUBLIC VIRTUAL IMS_BOOL AosServiceAvailable::StopToCheckNetworkConnection(
        IN IMS_BOOL /*bNeedToCheckAvailable*/ /*= IMS_TRUE*/)
{
    return IMS_FALSE;
}

PUBLIC void AosServiceAvailable::SetBlock(IN IAosBlock* piBlock)
{
    m_piBlock = piBlock;
}

PUBLIC IMS_BOOL AosServiceAvailable::IsRoaming()
{
    return m_bRoamingState;
}

PROTECTED VIRTUAL void AosServiceAvailable::HandleCallStateChanged(
        IN IMS_UINT32 nState, IN IMS_SINT32 nStateEx)
{
    A_IMS_TRACE_I(AOSTAG, "HandleCallStateChanged :: nState(%d) nStateEx(%d)", nState, nStateEx, 0);
}

PROTECTED VIRTUAL void AosServiceAvailable::HandleNetworkStateChanged()
{
    A_IMS_TRACE_I(AOSTAG, "HandleNetworkStateChanged", 0, 0, 0);
}

PRIVATE VIRTUAL void AosServiceAvailable::HandleBlockChanged(
        IN IMS_UINT32 nState, IN IMS_UINT32 nStateEx)
{
    A_IMS_TRACE_I(AOSTAG, "HandleBlockChanged :: Reason(%s) - %s",
            AosBlock::BlockReasonToString(nState), (nStateEx > 0) ? "BLOCK" : "NOT_BLOCK", 0);
}

PROTECTED VIRTUAL void AosServiceAvailable::HandleRoamingChanged(IN IMS_UINT32 nState)
{
    A_IMS_TRACE_I(AOSTAG, "HandleRoamingChanged :: nState(%d)", nState, 0, 0);

    m_bRoamingState = (nState == IMS_ROAMING_STATE_ON) ? IMS_TRUE : IMS_FALSE;
}

PROTECTED VIRTUAL void AosServiceAvailable::HandleAirplaneModeChanged(IN IMS_UINT32 nState)
{
    A_IMS_TRACE_I(AOSTAG, "HandleAirplaneModeChanged :: nState(%d)", nState, 0, 0);

    m_bAirplaneMode = (nState > 0) ? IMS_TRUE : IMS_FALSE;
}

PROTECTED VIRTUAL void AosServiceAvailable::HandleVopsChanged(IN IMS_UINT32 nState)
{
    A_IMS_TRACE_I(AOSTAG, "HandleVopsChanged :: nState(%d)", nState, 0, 0);
}

PROTECTED VIRTUAL void AosServiceAvailable::HandleWifiConnectionChanged()
{
    A_IMS_TRACE_I(AOSTAG, "HandleWifiConnectionChanged", 0, 0, 0);
}

PROTECTED VIRTUAL void AosServiceAvailable::HandleLocationInfoChanged()
{
    A_IMS_TRACE_I(AOSTAG, "HandleLocationInfoChanged", 0, 0, 0);
}

PROTECTED VIRTUAL IMS_BOOL AosServiceAvailable::CheckServiceAvailable()
{
    return IMS_TRUE;
}

PROTECTED
void AosServiceAvailable::Notify(IN IMS_BOOL bNotify /*=IMS_TRUE*/)
{
    IMS_BOOL bAvailable = CheckServiceAvailable();

    if ((m_bAvailableLastNotified == bAvailable) && IsSameAsBeforeUnavailableReason())
    {
        return;
    }

    A_IMS_TRACE_I(AOSTAG, "Notify", 0, 0, 0);

    m_bAvailableLastNotified = bAvailable;

    if (m_piBlock != IMS_NULL)
    {
        m_piBlock->GetBlockReasons(m_objBlockReasonsLastNotified);
        m_piBlock->PrintBlockReasons();
    }

    for (IMS_UINT32 nIndex = 0; nIndex < m_objListeners.GetSize(); nIndex++)
    {
        IAosServiceAvailableListener* piListener = m_objListeners.GetAt(nIndex);

        if (piListener != IMS_NULL)
        {
            piListener->ServiceAvailable_Changed(bNotify);
        }
    }
}

PROTECTED
void AosServiceAvailable::RequestCommand(IN IMS_UINT32 nCommand, IN IMS_UINT32 nReason)
{
    for (IMS_UINT32 nIndex = 0; nIndex < m_objListeners.GetSize(); nIndex++)
    {
        IAosServiceAvailableListener* piListener = m_objListeners.GetAt(nIndex);

        if (piListener != IMS_NULL)
        {
            piListener->ServiceAvailable_RequestCommand(nCommand, nReason);
        }
    }
}

PROTECTED
IMS_BOOL AosServiceAvailable::IsSameAsBeforeUnavailableReason()
{
    if (m_piBlock == IMS_NULL)
    {
        return IMS_FALSE;
    }

    ImsList<IMS_UINT32> objCurrReason;
    m_piBlock->GetBlockReasons(objCurrReason);

    return AosUtil::GetInstance()->IsListEqual(
            objCurrReason, m_objBlockReasonsLastNotified, IMS_TRUE);
}

PROTECTED GLOBAL const IMS_CHAR* AosServiceAvailable::EventToString(IN IMS_UINT32 eEvent)
{
    switch (eEvent)
    {
        case EVENT_AIRPLANE:
            return "EVENT_AIRPLANE";

        case EVENT_ROAMING:
            return "EVENT_ROAMING";

        case EVENT_VOPS:
            return "EVENT_VOPS";

        case EVENT_LOCATION:
            return "EVENT_LOCATION";

        case EVENT_CALL:
            return "EVENT_CALL";

        case EVENT_NETWORK:
            return "EVENT_NETWORK";

        case EVENT_WIFI_STATE:
            return "EVENT_WIFI_STATE";

        case EVENT_BLOCK:
            return "EVENT_BLOCK";

        case EVENT_BLOCK_SILENT:
            return "EVENT_BLOCK_SILENT";

        default:
            return "EVENT_INVALID";
    }
}
