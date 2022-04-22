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

__IMS_TRACE_TAG_USER_DECL__("AOS");

#define AOSTAG m_strTag.GetStr()

PRIVATE GLOBAL
IMSList<IMS_UINT32> AosServiceAvailable::m_objBlockReasonsLastNotified = IMSList<IMS_UINT32>();

PROTECTED
AosServiceAvailable::AosServiceAvailable(AString strName)
    : m_piAppContext(IMS_NULL)
    , m_nSlotId(IMS_SLOT_0)
    , m_strName(strName)
    , m_bAirplaneMode(IMS_FALSE)
    , m_bRoamingState(IMS_FALSE)
    , m_bAvailableLastNotified(IMS_FALSE)
    , m_objListeners(IMSList<IAosServiceAvailableListener*>())
{
    IMS_TRACE_MEM("AOS_MEM", "AOS_M : AosServiceAvailable = %" PFLS_u "/%" PFLS_x,
            sizeof(AosServiceAvailable), this, 0);
}

PROTECTED VIRTUAL
AosServiceAvailable::~AosServiceAvailable()
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
        IAosServiceAvailableListener* piTemp = m_objListeners.GetAt(nIndex);

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
        IAosServiceAvailableListener* piTemp = m_objListeners.GetAt(nIndex);

        if (piTemp == piListener)
        {
            m_objListeners.RemoveAt(nIndex);
            return;
        }
    }
}

PUBLIC
void AosServiceAvailable::RefreshServiceAvailablility()
{
    A_IMS_TRACE_I(AOSTAG, "RefreshServiceAvailablility", 0, 0, 0);
    Notify();
}

PUBLIC
IMS_BOOL AosServiceAvailable::IsAvailable()
{
    return m_bAvailableLastNotified;
}

PUBLIC VIRTUAL
IMS_BOOL AosServiceAvailable::StopToCheckNetworkConnection(
        IN IMS_BOOL bNeedToCheckAvailable /*= IMS_TRUE*/)
{
    (void) bNeedToCheckAvailable;
    return IMS_FALSE;
}

PUBLIC VIRTUAL
void AosServiceAvailable::HandleEvent(IN IMS_UINT32 eEvent, IN IMS_UINT32 nState,
        IN IMS_SINT32 nStateEx)
{
    A_IMS_TRACE_I(AOSTAG, "HandleEvent :: E(%s)/S1(%d)/S2(%d)",
            EventToString(eEvent), nState, nStateEx);

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

        case EVENT_AIRPLANE:
            HandleAirplaneModeChanged(nState);
            break;

        case EVENT_ROAMING:
            HandleRoamingChanged(nState);
            break;

        case EVENT_VOLTE_SETTING:
            HandleVolteSettingChanged(nState);
            break;

        case EVENT_VOPS:
            HandleVopsChanged(nState);
            break;

        case EVENT_WFC_SETTING:
            HandleWfcSettingChanged(nState);
            break;

        case EVENT_LOCATION:
            HandleLocationInfoChanged();
            break;

        case EVENT_WIFI_STATE:
            HandleWiFiConnectionChanged();
            break;

        default:
            break;
    }

    Notify();
}

PROTECTED VIRTUAL
void AosServiceAvailable::HandleCallStateChanged(IN IMS_UINT32 nState, IN IMS_SINT32 nStateEx)
{
    A_IMS_TRACE_I(AOSTAG, "HandleCallStateChanged :: nState(%d) nStateEx(%d)",
            nState, nStateEx, 0);
}

PROTECTED VIRTUAL
void AosServiceAvailable::HandleNetworkStateChanged()
{
    A_IMS_TRACE_I(AOSTAG, "HandleNetworkStateChanged", 0, 0, 0);
}

PRIVATE VIRTUAL
void AosServiceAvailable::HandleBlockChanged(IN IMS_UINT32 nState, IN IMS_UINT32 nStateEx)
{
    A_IMS_TRACE_I(AOSTAG, "HandleBlockChanged :: Reason(%s) - %s",
            AosBlock::BlockReasonToString(nState), (nStateEx > 0) ? "BLOCK" : "NOT_BLOCK" , 0);
}

PROTECTED VIRTUAL
void AosServiceAvailable::HandleRoamingChanged(IN IMS_UINT32 nState)
{
    A_IMS_TRACE_I(AOSTAG, "HandleRoamingChanged :: nState(%d)", nState, 0, 0);

    m_bRoamingState = (nState == IMS_ROAMING_STATE_ON) ? IMS_TRUE : IMS_FALSE;
}

PROTECTED VIRTUAL
void AosServiceAvailable::HandleAirplaneModeChanged(IN IMS_UINT32 nState)
{
    A_IMS_TRACE_I(AOSTAG, "HandleAirplaneModeChanged :: nState(%d)", nState, 0, 0);

    m_bAirplaneMode = (nState > 0) ? IMS_TRUE : IMS_FALSE;
}

PROTECTED VIRTUAL
void AosServiceAvailable::HandleVolteSettingChanged(IN IMS_UINT32 nState)
{
    A_IMS_TRACE_I(AOSTAG, "HandleVolteSettingChanged :: nState(%d)", nState, 0, 0);
}

PROTECTED VIRTUAL
void AosServiceAvailable::HandleVopsChanged(IN IMS_UINT32 nState)
{
    A_IMS_TRACE_I(AOSTAG, "HandleVopsChanged :: nState(%d)", nState, 0, 0);
}

PROTECTED VIRTUAL
void AosServiceAvailable::HandleWfcSettingChanged(IN IMS_UINT32 nState)
{
    A_IMS_TRACE_I(AOSTAG, "HandleWfcSettingChanged :: nState(%d)", nState, 0, 0);
}

PROTECTED VIRTUAL
void AosServiceAvailable::HandleWiFiConnectionChanged()
{
    A_IMS_TRACE_I(AOSTAG, "HandleWiFiConnectionChanged", 0, 0, 0);
}

PROTECTED VIRTUAL
void AosServiceAvailable::HandleLocationInfoChanged()
{
    A_IMS_TRACE_I(AOSTAG, "HandleLocationInfoChanged", 0, 0, 0);
}

PROTECTED VIRTUAL
IMS_BOOL AosServiceAvailable::CheckServiceAvailable()
{
    return IMS_TRUE;
}

PROTECTED
void AosServiceAvailable::Notify()
{
    IMS_BOOL bAvailable = CheckServiceAvailable();

    if ((m_bAvailableLastNotified == bAvailable) && IsSameAsBeforeUnavailableReason())
    {
        return;
    }

    A_IMS_TRACE_I(AOSTAG, "Notify", 0, 0, 0);

    m_bAvailableLastNotified = bAvailable;
    m_piAppContext->GetBlock()->GetBlockReasons(m_objBlockReasonsLastNotified);

    m_piAppContext->GetBlock()->PrintBlockReasons();

    for (IMS_UINT32 nIndex = 0; nIndex < m_objListeners.GetSize(); nIndex++)
    {
        IAosServiceAvailableListener* piListener = m_objListeners.GetAt(nIndex);

        if (piListener != IMS_NULL)
        {
            piListener->ServiceAvailable_Changed();
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
    IMSList<IMS_UINT32> objCurrReason;
    m_piAppContext->GetBlock()->GetBlockReasons(objCurrReason);

    return AosUtil::GetInstance()->IsListEqual(objCurrReason,
            m_objBlockReasonsLastNotified, IMS_TRUE);
}

PROTECTED GLOBAL
const IMS_CHAR* AosServiceAvailable::EventToString(IN IMS_UINT32 eEvent)
{
    switch (eEvent)
    {
        case EVENT_AIRPLANE:
            return "EVENT_AIRPLANE";

        case EVENT_ROAMING:
             return "EVENT_ROAMING";

        case EVENT_VOLTE_SETTING:
            return "EVENT_VOLTE_SETTING";

        case EVENT_VOPS:
            return "EVENT_VOPS";

        case EVENT_WFC_SETTING:
            return "EVENT_WFC_SETTING";

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

        default:
            return "EVENT_INVALID";
    }
}
