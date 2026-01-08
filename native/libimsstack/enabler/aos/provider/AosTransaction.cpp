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

#include "IImsTraffic.h"
#include "INetworkWatcher.h"
#include "ServiceImsRadio.h"
#include "ServiceTimer.h"
#include "ServiceTrace.h"

#include "interface/IAosAppContext.h"
#include "interface/IAosNetTracker.h"
#include "provider/AosUtil.h"
#include "provider/AosTransaction.h"

__IMS_TRACE_TAG_AOS__;

#define AOSTAG m_strTag.GetStr()

PUBLIC
AosTransaction::AosTransaction(IN IMS_SINT32 nSlotId) :
        m_piImsRadio(IMS_NULL),
        m_piStopTimer(IMS_NULL),
        m_bIsEmergencyStartUpdated(IMS_FALSE),
        m_bIsStartUpdated(IMS_FALSE),
        m_bIsTrafficResponseWaiting(IMS_FALSE),
        m_nResponseWaitingTraffics(TYPE_NONE),
        m_nSlotId(nSlotId),
        m_nStartType(TYPE_NONE),
        m_nTraffics(TYPE_NONE),
        m_objListeners(ImsMap<IMS_UINT32, ImsList<IAosTransactionListener*>>()),
        m_objTraffics(ImsMap<IMS_UINT32, AosTraffic*>())
{
    m_strTag.Sprintf("%d", m_nSlotId);

    m_piImsRadio = ImsRadioService::GetImsRadioService()->GetImsRadio(m_nSlotId);
    if (m_piImsRadio != IMS_NULL)
    {
        m_piImsRadio->AddListenerForTrafficPriority(this);
    }

    m_objTraffics.Add(TYPE_REG, new AosTraffic(TYPE_REG, this));
    m_objTraffics.Add(TYPE_SUB, new AosTraffic(TYPE_SUB, this));
    m_objTraffics.Add(TYPE_EMERGENCY, new AosTraffic(TYPE_EMERGENCY, this));
    m_objTraffics.Add(TYPE_DEREG, new AosTraffic(TYPE_DEREG, this));
}

PUBLIC VIRTUAL AosTransaction::~AosTransaction()
{
    A_IMS_TRACE_D(AOSTAG, "~AosTransaction()", 0, 0, 0);

    StopTimer();

    for (IMS_UINT32 i = 0; i < m_objTraffics.GetSize(); ++i)
    {
        AosTraffic* pTraffic = m_objTraffics.GetValueAt(i);

        if (pTraffic != IMS_NULL)
        {
            if (m_piImsRadio != IMS_NULL)
            {
                m_piImsRadio->StopImsTraffic(pTraffic);
            }

            delete pTraffic;
        }
    }

    m_objTraffics.Clear();
    m_objListeners.Clear();

    IImsTraffic* piImsTraffic = ImsRadioService::GetImsRadioService()->GetImsTraffic();
    if (piImsTraffic != IMS_NULL)
    {
        piImsTraffic->Disable(m_nSlotId);
    }

    if (m_piImsRadio != IMS_NULL)
    {
        m_piImsRadio->RemoveListenerForTrafficPriority(this);
    }
}

PUBLIC VIRTUAL void AosTransaction::SetListener(
        IN IMS_UINT32 nType, IN IAosTransactionListener* piListener)
{
    if (piListener == IMS_NULL)
    {
        return;
    }

    IMS_SLONG nIndex = m_objListeners.GetIndexOfKey(nType);

    if (nIndex < 0)
    {
        ImsList<IAosTransactionListener*> objListeners;

        objListeners.Append(piListener);
        m_objListeners.Add(nType, objListeners);

        A_IMS_TRACE_D(AOSTAG, "AosTransactionListener :: add - %d / %d / %p", nType,
                objListeners.GetSize(), piListener);

        return;
    }

    ImsList<IAosTransactionListener*>& objListeners = m_objListeners.GetValueAt(nIndex);

    for (IMS_UINT32 i = 0; i < objListeners.GetSize(); ++i)
    {
        IAosTransactionListener* piTmpListener = objListeners.GetAt(i);

        if (piTmpListener == IMS_NULL)
        {
            continue;
        }

        if (piListener == piTmpListener)
        {
            return;
        }
    }

    objListeners.Append(piListener);

    A_IMS_TRACE_D(AOSTAG, "AosTransactionListener :: add - %d / %d / %p", nType,
            objListeners.GetSize(), piListener);
}

PUBLIC VIRTUAL void AosTransaction::RemoveListener(
        IN IMS_UINT32 nType, IN IAosTransactionListener* piListener)
{
    if (piListener == IMS_NULL)
    {
        return;
    }

    IMS_SLONG nIndex = m_objListeners.GetIndexOfKey(nType);

    if (nIndex < 0)
    {
        return;
    }

    ImsList<IAosTransactionListener*>& objListeners = m_objListeners.GetValueAt(nIndex);

    for (IMS_UINT32 i = 0; i < objListeners.GetSize(); ++i)
    {
        IAosTransactionListener* piTmpListener = objListeners.GetAt(i);

        if (piTmpListener == IMS_NULL)
        {
            continue;
        }

        if (piListener == piTmpListener)
        {
            A_IMS_TRACE_D(AOSTAG, "AosTransactionListener :: remove - %d / %d / %p", nType,
                    m_objListeners.GetSize(), piListener);
            objListeners.RemoveAt(i);
            break;
        }
    }
}

PUBLIC VIRTUAL IMS_BOOL AosTransaction::IsTransactionAllowed(IN IMS_UINT32 nType)
{
    if (m_piImsRadio != IMS_NULL)
    {
        return m_piImsRadio->IsImsTrafficAllowed((nType != TYPE_EMERGENCY)
                        ? IImsRadio::TRAFFIC_TYPE_REGISTRATION
                        : IImsRadio::TRAFFIC_TYPE_EMERGENCY);
    }

    return IMS_FALSE;
}

PUBLIC VIRTUAL IMS_BOOL AosTransaction::StartTraffic(IN IMS_UINT32 nType, IN IMS_UINT32 nRadioType)
{
    if (m_piImsRadio == IMS_NULL)
    {
        return IMS_TRUE;
    }

    if (IsStarted(nType))
    {
        return IMS_TRUE;
    }

    Start(nType);

    if (IsStartUpdated())
    {
        if (m_piStopTimer != IMS_NULL)
        {
            StopTimer();
            return IMS_TRUE;
        }

        if (IsTrafficResponseWaiting())
        {
            AddForWaitingResponse(nType);
        }
        else
        {
            A_IMS_TRACE_I(AOSTAG, "StartTraffic :: already got traffic callback", 0, 0, 0);
            return IMS_TRUE;
        }
    }
    else
    {
        A_IMS_TRACE_I(AOSTAG, "StartTraffic :: type(%d), radio(%d)", nType, nRadioType, 0);

        m_bIsStartUpdated = IMS_TRUE;
        m_bIsTrafficResponseWaiting = IMS_TRUE;
        m_nStartType = nType;

        IMS_SLONG nIndex = m_objTraffics.GetIndexOfKey(nType);

        if (nIndex < 0)
        {
            return IMS_TRUE;
        }

        m_piImsRadio->StartImsTraffic(IImsRadio::TRAFFIC_TYPE_REGISTRATION,
                GetAccessNetworkType(nRadioType), IImsRadio::DIRECTION_MO,
                m_objTraffics.GetValueAt(nIndex));
    }

    return IMS_FALSE;
}

PUBLIC VIRTUAL void AosTransaction::StartEmergencyTraffic(IN IMS_UINT32 nRadioType)
{
    if (m_piImsRadio == IMS_NULL)
    {
        return;
    }

    if (!m_bIsEmergencyStartUpdated)
    {
        m_bIsEmergencyStartUpdated = IMS_TRUE;

        IMS_SLONG nIndex = m_objTraffics.GetIndexOfKey(TYPE_EMERGENCY);

        if (nIndex < 0)
        {
            return;
        }

        m_piImsRadio->StartImsTraffic(IImsRadio::TRAFFIC_TYPE_EMERGENCY,
                GetAccessNetworkType(nRadioType), IImsRadio::DIRECTION_MO,
                m_objTraffics.GetValueAt(nIndex));
    }
}

PUBLIC VIRTUAL void AosTransaction::StopTraffic(IN IMS_UINT32 nType)
{
    if (!IsStarted(nType))
    {
        return;
    }

    Stop(nType);
    RemoveForWaitingResponse(nType);

    if (IsStartUpdated())
    {
        if (!IsStarted())
        {
            if (nType == TYPE_DEREG)
            {
                StopTimer();
                ProcessTimerExpired();
            }
            else
            {
                StartTimer(TIME_STOP_DELAY);
            }
        }
    }
}

PUBLIC VIRTUAL void AosTransaction::StopEmergencyTraffic()
{
    if (m_bIsEmergencyStartUpdated)
    {
        m_bIsEmergencyStartUpdated = IMS_FALSE;

        IMS_SLONG nIndex = m_objTraffics.GetIndexOfKey(TYPE_EMERGENCY);

        if (nIndex >= 0)
        {
            m_piImsRadio->StopImsTraffic(m_objTraffics.GetValueAt(nIndex));
        }
    }
}

PUBLIC VIRTUAL void AosTransaction::SetWlan(IN IMS_BOOL bEnabled)
{
    IImsTraffic* piImsTraffic = ImsRadioService::GetImsRadioService()->GetImsTraffic();
    if (piImsTraffic != IMS_NULL)
    {
        piImsTraffic->SetWlan(m_nSlotId, bEnabled);
    }
}

PROTECTED IMS_BOOL AosTransaction::IsResponseWaiting(IN IMS_UINT32 nType) const
{
    return (m_nResponseWaitingTraffics & nType);
}

PROTECTED IMS_BOOL AosTransaction::IsStarted() const
{
    return (m_nTraffics != TYPE_NONE);
}

PROTECTED IMS_BOOL AosTransaction::IsStarted(IN IMS_UINT32 nType) const
{
    return (m_nTraffics & nType);
}

PROTECTED IMS_BOOL AosTransaction::IsStartUpdated() const
{
    return m_bIsStartUpdated;
}

PROTECTED IMS_BOOL AosTransaction::IsTimerRunning() const
{
    return (m_piStopTimer != IMS_NULL);
}

PROTECTED IMS_BOOL AosTransaction::IsTrafficResponseWaiting() const
{
    return m_bIsTrafficResponseWaiting;
}

PROTECTED IMS_UINT32 AosTransaction::GetAccessNetworkType(IN IMS_UINT32 nRadioType)
{
    switch (nRadioType)
    {
        case NW_REPORT_RADIO_NR:
            return IImsRadio::ACCESS_NETWORK_TYPE_NGRAN;

        case NW_REPORT_RADIO_LTE:
            return IImsRadio::ACCESS_NETWORK_TYPE_EUTRAN;

        case NW_REPORT_RADIO_EHRPD:  // FALL-THROUGH
        case NW_REPORT_RADIO_WCDMA:  // FALL-THROUGH
        case NW_REPORT_RADIO_HSPA:
            return IImsRadio::ACCESS_NETWORK_TYPE_UTRAN;

        case NW_REPORT_RADIO_WLAN:
            return IImsRadio::ACCESS_NETWORK_TYPE_IWLAN;

        default:
            return IImsRadio::ACCESS_NETWORK_TYPE_UNKNOWN;
    }
}

PROTECTED VIRTUAL void AosTransaction::Timer_TimerExpired(IN ITimer* piTimer)
{
    if (piTimer == IMS_NULL)
    {
        return;
    }

    if (piTimer != m_piStopTimer)
    {
        return;
    }

    StopTimer();

    ProcessTimerExpired();
}

PROTECTED VIRTUAL void AosTransaction::Traffic_OnConnectionFailed(IN IMS_UINT32 nType,
        IN IMS_UINT32 nFailureReason, IN IMS_UINT32 nCauseCode, IN IMS_UINT32 nWaitTimeMillis)
{
    if (nType != TYPE_DEREG)
    {
        NotifyConnectionFailed(nType, nFailureReason, nCauseCode, nWaitTimeMillis);
    }

    if (IsResponseWaiting(TYPE_REG))
    {
        NotifyConnectionFailed(TYPE_REG, nFailureReason, nCauseCode, nWaitTimeMillis);
        RemoveForWaitingResponse(TYPE_REG);
    }

    if (IsResponseWaiting(TYPE_SUB))
    {
        NotifyConnectionFailed(TYPE_SUB, nFailureReason, nCauseCode, nWaitTimeMillis);
        RemoveForWaitingResponse(TYPE_SUB);
    }

    if (IsResponseWaiting(TYPE_DEREG))
    {
        // Connection callback is not required.
        RemoveForWaitingResponse(TYPE_DEREG);
    }

    m_bIsTrafficResponseWaiting = IMS_FALSE;
}

PROTECTED VIRTUAL void AosTransaction::Traffic_OnConnectionSetupPrepared(IN IMS_UINT32 nType)
{
    if (nType != TYPE_DEREG)
    {
        NotifyConnectionSetupPrepared(nType);
    }

    if (IsResponseWaiting(TYPE_REG))
    {
        NotifyConnectionSetupPrepared(TYPE_REG);
        RemoveForWaitingResponse(TYPE_REG);
    }

    if (IsResponseWaiting(TYPE_SUB))
    {
        NotifyConnectionSetupPrepared(TYPE_SUB);
        RemoveForWaitingResponse(TYPE_SUB);
    }

    if (IsResponseWaiting(TYPE_DEREG))
    {
        // Connection callback is not required.
        RemoveForWaitingResponse(TYPE_DEREG);
    }

    m_bIsTrafficResponseWaiting = IMS_FALSE;
}

PROTECTED VIRTUAL void AosTransaction::ImsRadio_OnTrafficPriorityChanged()
{
    NotifyTrafficPriorityChanged(TYPE_REG);
    NotifyTrafficPriorityChanged(TYPE_SUB);
    NotifyTrafficPriorityChanged(TYPE_DEREG);
}

PRIVATE void AosTransaction::NotifyConnectionFailed(IN IN IMS_UINT32 nType,
        IN IMS_UINT32 nFailureReason, IN IMS_UINT32 nCauseCode, IN IMS_UINT32 nWaitTimeMillis)
{
    A_IMS_TRACE_D(AOSTAG, "NotifyConnectionFailed :: type(%d), reason(%d), wtm(%d)", nType,
            nFailureReason, nWaitTimeMillis);

    IMS_SLONG nIndex = m_objListeners.GetIndexOfKey(nType);

    if (nIndex < 0)
    {
        return;
    }

    ImsList<IAosTransactionListener*>& objTxnListeners = m_objListeners.GetValueAt(nIndex);

    for (IMS_UINT32 i = 0; i < objTxnListeners.GetSize(); ++i)
    {
        IAosTransactionListener* piListener = objTxnListeners.GetAt(i);

        if (piListener == IMS_NULL)
        {
            continue;
        }

        piListener->Transaction_OnConnectionFailed(nFailureReason, nCauseCode, nWaitTimeMillis);
    }
}

PRIVATE void AosTransaction::NotifyConnectionSetupPrepared(IN IMS_UINT32 nType)
{
    IMS_SLONG nIndex = m_objListeners.GetIndexOfKey(nType);

    if (nIndex < 0)
    {
        return;
    }

    ImsList<IAosTransactionListener*>& objTxnListeners = m_objListeners.GetValueAt(nIndex);

    A_IMS_TRACE_D(AOSTAG, "NotifyConnectionSetupPrepared :: type(%d), index(%d), size(%d)", nType,
            nIndex, objTxnListeners.GetSize());

    for (IMS_UINT32 i = 0; i < objTxnListeners.GetSize(); ++i)
    {
        IAosTransactionListener* piListener = objTxnListeners.GetAt(i);

        if (piListener == IMS_NULL)
        {
            continue;
        }

        piListener->Transaction_OnConnectionSetupPrepared();
    }
}

PRIVATE void AosTransaction::NotifyTrafficPriorityChanged(IN IMS_UINT32 nType)
{
    IMS_SLONG nIndex = m_objListeners.GetIndexOfKey(nType);

    if (nIndex >= 0)
    {
        ImsList<IAosTransactionListener*>& objTxnListeners = m_objListeners.GetValueAt(nIndex);

        for (IMS_UINT32 i = 0; i < objTxnListeners.GetSize(); ++i)
        {
            IAosTransactionListener* piListener = objTxnListeners.GetAt(i);

            if (piListener == IMS_NULL)
            {
                continue;
            }

            piListener->Transaction_OnTrafficPriorityChanged();
        }
    }
}

PRIVATE void AosTransaction::Start(IN IMS_UINT32 nType)
{
    m_nTraffics |= nType;
}

PRIVATE void AosTransaction::Stop(IN IMS_UINT32 nType)
{
    m_nTraffics &= (~nType);
}

PRIVATE void AosTransaction::AddForWaitingResponse(IN IMS_UINT32 nType)
{
    m_nResponseWaitingTraffics |= nType;
}

PRIVATE void AosTransaction::RemoveForWaitingResponse(IN IMS_UINT32 nType)
{
    m_nResponseWaitingTraffics &= (~nType);
}

PRIVATE void AosTransaction::StartTimer(IN IMS_UINT32 nDuration)
{
    if (m_piStopTimer != IMS_NULL)
    {
        StopTimer();
    }

    m_piStopTimer = AosUtil::GetInstance()->StartTimer(nDuration, this, "TRANSACTION_START_TIMER");
}

PRIVATE void AosTransaction::StopTimer()
{
    if (m_piStopTimer == IMS_NULL)
    {
        return;
    }

    AosUtil::GetInstance()->StopTimer(m_piStopTimer, "TRANSACTION_STOP_TIMER");
}

PRIVATE void AosTransaction::ProcessTimerExpired()
{
    if (IsStartUpdated())
    {
        if (!IsStarted())
        {
            A_IMS_TRACE_I(AOSTAG, "ProcessTimerExpired :: set end to transaction", 0, 0, 0);
            m_bIsStartUpdated = IMS_FALSE;

            IMS_SLONG nIndex = m_objTraffics.GetIndexOfKey(m_nStartType);

            if (nIndex >= 0)
            {
                m_piImsRadio->StopImsTraffic(m_objTraffics.GetValueAt(nIndex));
            }
        }
    }
}
