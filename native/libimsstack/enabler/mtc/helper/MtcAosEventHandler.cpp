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
#include "ServiceTrace.h"
#include "JniMtcServiceThread.h"
#include "IMtcCallController.h"
#include "AString.h"
#include "IIpcan.h"
#include "IMtcService.h"
#include "ImsAosParameter.h"
#include "IuMtcService.h"
#include "MtcEmergencyServiceManager.h"
#include "configuration/MtcConfigurationProxy.h"
#include "helper/MtcAosEventHandler.h"

__IMS_TRACE_TAG_COM_MTC__;

PUBLIC
MtcAosEventHandler::MtcAosEventHandler(
        IN IMtcService& objService, IN MtcConfigurationProxy& objConfiguration) :
        m_objService(objService),
        m_objConfiguration(objConfiguration),
        m_nIpcan(IIpcan::CATEGORY_MOBILE)
{
    IMS_TRACE_I("+MtcAosEventHandler", 0, 0, 0);
}

PUBLIC
MtcAosEventHandler::~MtcAosEventHandler()
{
    IMS_TRACE_I("~MtcAosEventHandler", 0, 0, 0);
}

PUBLIC
void MtcAosEventHandler::OnConnected(IN IMS_UINT32 nFeatures, IN IMS_UINT32 nIpcan,
        IN JniMtcServiceThread* pServiceThread,
        IN MtcEmergencyServiceManager* /* pEmergencyServiceManager */,
        IN IMtcCallController& objCallController)
{
    IMS_TRACE_I("OnConnected emergency[%s] nIpcan[%d]", _TRACE_B_(m_objService.IsEmergency()),
            nIpcan, 0);

    IMS_UINT32 nMmtelConnected =
            nFeatures & ImsAosFeature::MMTEL ? ImsAosFeature::MMTEL : ImsAosFeature::NONE;
    IMS_UINT32 nVideoConnected =
            nFeatures & ImsAosFeature::VIDEO ? ImsAosFeature::VIDEO : ImsAosFeature::NONE;

    if (m_objService.IsEmergency())
    {
        // pEmergencyServiceManager->ProcessServiceStatus(ServiceStatus::SERVICE_ACTIVE,
        //         pServiceThread);
    }
    else
    {
        if (pServiceThread)
        {
            pServiceThread->OnServiceChanged(nMmtelConnected + nVideoConnected, 0);
        }
        // TODO: this must be called when registration is refreshed?
        m_objConfiguration.OnRegistrationRefreshed();
    }

    if (m_nIpcan != nIpcan)
    {
        m_nIpcan = nIpcan;
        if (m_objConfiguration.Is(Feature::ENABLE_SEND_REINVITE_ON_RAT_CHANGE))
        {
            objCallController.HandleIpcanChanged();
        }
    }
}

PUBLIC
void MtcAosEventHandler::OnDisconnecting(
        IN IMS_UINT32 nReason, IN IMtcCallController& objCallController)
{
    IMS_TRACE_I("OnDisconnecting emergency[%s] nReason[%d]", _TRACE_B_(m_objService.IsEmergency()),
            nReason, 0);

    Key nKey;
    nKey.eServiceType = m_objService.GetServiceType();
    if (m_objConfiguration.Is(
                Feature::REGISTRATION_DISCONNECT_REASON_TO_TERMINATE_ONGOING_CALL, nReason))
    {
        objCallController.TerminateCalls(KeyType::SERVICE_TYPE, nKey, nReason);
    }
}

PUBLIC
void MtcAosEventHandler::OnDisconnected(IN IMS_UINT32 nReason,
        IN IMtcCallController& objCallController, IN JniMtcServiceThread* pServiceThread,
        IN MtcEmergencyServiceManager* /* pEmergencyServiceManager */)
{
    IMS_TRACE_I("OnDisconnected emergency[%s] nReason[%d]", _TRACE_B_(m_objService.IsEmergency()),
            nReason, 0);

    Key nKey;
    nKey.eServiceType = m_objService.GetServiceType();
    if (m_objConfiguration.Is(
                Feature::REGISTRATION_DISCONNECT_REASON_TO_TERMINATE_ONGOING_CALL, nReason))
    {
        objCallController.TerminateCalls(KeyType::SERVICE_TYPE, nKey, nReason);
    }

    if (m_objService.IsEmergency())
    {
        // pEmergencyServiceManager->ProcessServiceStatus(ServiceStatus::SERVICE_IDLE,
        //         pServiceThread);
    }
    else
    {
        if (pServiceThread)
        {
            pServiceThread->OnServiceChanged(IuMtcService::SERVICE_NONE, 0);
        }
    }
}

PUBLIC
void MtcAosEventHandler::OnSuspended(
        IN IMS_UINT32 nReason, IN IMtcCallController& /*objCallController*/)
{
    IMS_TRACE_I("OnSuspended emergency[%s] nReason[%d]", _TRACE_B_(m_objService.IsEmergency()),
            nReason, 0);
}

PUBLIC
void MtcAosEventHandler::OnResumed()
{
    IMS_TRACE_I("OnResumed emergency[%s]", _TRACE_B_(m_objService.IsEmergency()), 0, 0);
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
