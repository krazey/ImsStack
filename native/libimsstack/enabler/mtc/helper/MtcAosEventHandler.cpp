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

#include "AString.h"
#include "IIpcan.h"
#include "IJniMtcServiceThread.h"
#include "IMtcService.h"
#include "ImsAosParameter.h"
#include "ImsTypeDef.h"
#include "IuMtcService.h"
#include "ServiceTrace.h"
#include "configuration/MtcConfigurationProxy.h"
#include "helper/IMtcAosStateListener.h"
#include "helper/MtcAosEventHandler.h"

__IMS_TRACE_TAG_COM_MTC__;

PUBLIC
MtcAosEventHandler::MtcAosEventHandler(
        IN IMtcService& objService, IN MtcConfigurationProxy& objConfiguration) :
        m_objService(objService),
        m_objConfiguration(objConfiguration),
        m_objListeners(ImsList<IMtcAosStateListener*>())
{
    IMS_TRACE_I("+MtcAosEventHandler", 0, 0, 0);
}

PUBLIC
MtcAosEventHandler::~MtcAosEventHandler()
{
    IMS_TRACE_I("~MtcAosEventHandler", 0, 0, 0);
    m_objListeners.Clear();
}

PUBLIC
void MtcAosEventHandler::AddListener(IN IMtcAosStateListener* piListener)
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
void MtcAosEventHandler::RemoveListener(IN IMtcAosStateListener* piListener)
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
void MtcAosEventHandler::OnConnected(IN IMS_UINT32 nFeatures)
{
    IMS_TRACE_I("OnConnected emergency[%s]", _TRACE_B_(m_objService.IsEmergency()), 0, 0);

    if (!m_objService.IsEmergency())
    {
        IJniMtcServiceThread* pThread = m_objService.GetJniServiceThread();
        if (pThread)
        {
            pThread->OnServiceChanged(ConvertAosFeatureToServiceState(nFeatures), 0);
        }
        // TODO: this must be called when registration is refreshed?
        m_objConfiguration.OnRegistrationRefreshed();
    }

    NotifyStateChanged(MtcAosState::CONNECTED, ImsAosReason::NONE);
}

PUBLIC
void MtcAosEventHandler::OnDisconnecting(IN IMS_UINT32 nReason)
{
    IMS_TRACE_I("OnDisconnecting emergency[%s] nReason[%d]", _TRACE_B_(m_objService.IsEmergency()),
            nReason, 0);

    NotifyStateChanged(MtcAosState::DISCONNECTING, nReason);
}

PUBLIC
void MtcAosEventHandler::OnDisconnected(IN IMS_UINT32 nReason)
{
    IMS_TRACE_I("OnDisconnected emergency[%s] nReason[%d]", _TRACE_B_(m_objService.IsEmergency()),
            nReason, 0);

    if (!m_objService.IsEmergency())
    {
        IJniMtcServiceThread* pThread = m_objService.GetJniServiceThread();
        if (pThread)
        {
            pThread->OnServiceChanged(IuMtcService::ServiceState::SERVICE_NONE, 0);
        }
    }

    NotifyStateChanged(MtcAosState::DISCONNECTED, nReason);
}

PUBLIC
void MtcAosEventHandler::OnSuspended(IN IMS_UINT32 nReason)
{
    IMS_TRACE_I("OnSuspended emergency[%s] nReason[%d]", _TRACE_B_(m_objService.IsEmergency()),
            nReason, 0);
    NotifyStateChanged(MtcAosState::SUSPENDED, nReason);
}

PUBLIC
void MtcAosEventHandler::OnResumed()
{
    IMS_TRACE_I("OnResumed emergency[%s]", _TRACE_B_(m_objService.IsEmergency()), 0, 0);
    NotifyStateChanged(MtcAosState::CONNECTED, ImsAosReason::NONE);
}

PUBLIC
void MtcAosEventHandler::OnServiceConnected(IN IMS_UINT32 nServices, IN IMS_UINT32 nIpcan)
{
    IMS_TRACE_I("OnServiceConnected emergency[%s] nServices[%d] nIpcan[%d]",
            _TRACE_B_(m_objService.IsEmergency()), nServices, nIpcan);
}

PUBLIC
void MtcAosEventHandler::OnEventNotify(IN IMS_UINT32 nType, IN IMS_UINT32 nState)
{
    IMS_TRACE_I("OnEventNotify emergency[%s] nType[%d] nState[%d]",
            _TRACE_B_(m_objService.IsEmergency()), nType, nState);
}

PRIVATE
void MtcAosEventHandler::NotifyStateChanged(IN MtcAosState eState, IN IMS_UINT32 eAosReason) const
{
    for (IMS_UINT32 i = 0; i < m_objListeners.GetSize(); i++)
    {
        m_objListeners.GetAt(i)->OnAosStateChanged(m_objService, eState, eAosReason);
    }
}

PRIVATE
IuMtcService::ServiceState MtcAosEventHandler::ConvertAosFeatureToServiceState(
        IMS_UINT32 nFeatures) const
{
    IMS_BOOL bMmtelConnected = (nFeatures & ImsAosFeature::MMTEL);
    IMS_BOOL bVideoConnected = (nFeatures & ImsAosFeature::VIDEO);

    if (bMmtelConnected && bVideoConnected)
    {
        return IuMtcService::ServiceState::SERVICE_UC;
    }
    else if (bVideoConnected)
    {
        return IuMtcService::ServiceState::SERVICE_VT;
    }
    else if (bMmtelConnected)
    {
        return IuMtcService::ServiceState::SERVICE_VOIP;
    }
    return IuMtcService::ServiceState::SERVICE_NONE;
}
