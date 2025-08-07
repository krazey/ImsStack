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

#include "CarrierConfig.h"
#include "Engine.h"
#include "IConfiguration.h"
#include "IJniEnabler.h"
#include "IJniMtsAppThread.h"
#include "IMtsServiceState.h"
#include "ImsServiceConfig.h"
#include "JniEnablerConnector.h"
#include "MtsApp.h"
#include "MtsService.h"
#include "MtsNetworkTracker.h"
#include "ServiceConfig.h"
#include "ServiceTrace.h"
#include "message/MtsMessageController.h"
#include "utility/MtsDynamicLoader.h"

__IMS_TRACE_TAG_COM_MTS__;

LOCAL const IMS_CHAR MTS_APP_NAME[] = "MtsApp";

PUBLIC
MtsApp::MtsApp(IN IMS_SINT32 nSlotId) :
        ImsApp(MTS_APP_NAME),
        m_nSlotId(nSlotId),
        m_objNormalService(MtsService(*this, MtsServiceType::NORMAL)),
        m_objEmergencyService(MtsService(*this, MtsServiceType::EMERGENCY)),
        m_objMtsMessageController(MtsMessageController(*this)),
        m_objMtsNetworkTracker(MtsNetworkTracker(*this)),
        m_objMtsDynamicLoader(MtsDynamicLoader(*this))
{
    IMS_TRACE_I("+MtsApp [slot_%d]", m_nSlotId, 0, 0);

    Engine::GetConfiguration()->SetAppConfig(
            ImsServiceConfig::GetAppName(ImsAppId::MTS), m_nSlotId);
}

PUBLIC MtsApp::~MtsApp()
{
    IMS_TRACE_I("~MtsApp [slot_%d]", m_nSlotId, 0, 0);

    JniEnablerConnector::GetInstance().SetNativeEnabler(m_nSlotId, EnablerType::MTS, IMS_NULL);
}

PUBLIC VIRTUAL void MtsApp::Start()
{
    IMS_TRACE_I("SMS Start [slot_%d]", m_nSlotId, 0, 0);

    m_objNormalService.Init();
    m_objEmergencyService.Init();

    AttachJni();
}

PUBLIC VIRTUAL void MtsApp::Stop()
{
    IMS_TRACE_I("SMS Stop [slot_%d]", m_nSlotId, 0, 0);

    m_objNormalService.GetIMtsServiceState()->SetImsRegConnected(IMS_FALSE);
    m_objEmergencyService.GetIMtsServiceState()->SetImsRegConnected(IMS_FALSE);
}

PUBLIC VIRTUAL const IMtsService& MtsApp::GetService(IN MtsServiceType eServiceType) const
{
    if (eServiceType == MtsServiceType::NORMAL)
    {
        return m_objNormalService;
    }
    else
    {
        return m_objEmergencyService;
    }
}

PUBLIC VIRTUAL IJniMtsAppThread* MtsApp::GetJniAppThread() const
{
    const IJniEnabler* piJniEnabler =
            JniEnablerConnector::GetInstance().GetJniEnabler(m_nSlotId, EnablerType::MTS);
    if (piJniEnabler == IMS_NULL)
    {
        IMS_TRACE_E(0, "JniMtsAppThread is null", 0, 0, 0);
        return IMS_NULL;
    }

    return reinterpret_cast<IJniMtsAppThread*>(piJniEnabler->GetJniThread());
}

PUBLIC VIRTUAL void MtsApp::SendMoSmsByServiceType(IN SmsFormatType eSmsFormat,
        IN ByteArray* pContent, IN const AString& strAddress, IN IMS_SINT32 nSeqId,
        IN IMS_BOOL bEmergencyNumber, IN IMS_UINT32 nRetryCount)
{
    if (bEmergencyNumber && IsEmergencySmsOverImsSupported())
    {
        m_objEmergencyService.SendMoSms(
                eSmsFormat, pContent, strAddress, nSeqId, bEmergencyNumber, nRetryCount);
    }
    else
    {
        m_objNormalService.SendMoSms(
                eSmsFormat, pContent, strAddress, nSeqId, bEmergencyNumber, nRetryCount);
    }
}

PRIVATE void MtsApp::AttachJni()
{
    JniEnablerConnector::GetInstance().SetNativeEnabler(m_nSlotId, EnablerType::MTS, this);
}

PRIVATE
IMS_BOOL MtsApp::IsEmergencySmsOverImsSupported() const
{
    return ConfigService::GetConfigService()->GetCarrierConfig(m_nSlotId)->GetBoolean(
            CarrierConfig::KEY_SUPPORT_EMERGENCY_SMS_OVER_IMS_BOOL);
}
