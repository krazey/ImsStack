/**
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

#include "ServiceTrace.h"
#include "ServiceConfig.h"
#include "config/MediaSessionConfig.h"
#include "config/MediaSessionConfigFactory.h"
#include "ICarrierConfig.h"
#include "config/AudioConfiguration.h"
#include "config/VideoConfiguration.h"
#include "config/TextConfiguration.h"

__IMS_TRACE_TAG_MEDIA__;

PUBLIC
MediaSessionConfig::MediaSessionConfig(IN IMS_SINT32 nSlotId, IN MEDIA_SERVICE_TYPE serviceType) :
        m_pAudioConfig(IMS_NULL),
        m_pVideoConfig(IMS_NULL),
        m_pTextConfig(IMS_NULL),
        m_nServiceType(serviceType),
        m_bIsSessLevelBW(DEFAULT_SESSION_LEVEL_BW),
        m_bAnbrSupported(DEFAULT_ANBR_CAPABILITY),
        m_bSupportMultiConfigInEarlySession(DEFAULT_SUPPORT_MULTICONFIG),
        m_bSdpReofferFullCapability(IMS_TRUE)
{
    IMS_TRACE_I("+MediaSessionConfig() - SlotId[%d], ServiceType[%d]", nSlotId, serviceType, 0);
}

PUBLIC VIRTUAL MediaSessionConfig::~MediaSessionConfig()
{
    IMS_TRACE_I("~MediaSessionConfig()", 0, 0, 0);

    Clear();
    MediaSessionConfigFactory::GetInstance()->DestroySessionConfig(this);
}

PUBLIC
IMS_BOOL MediaSessionConfig::Create(IN IMS_SINT32 nSlotId)
{
    IMS_TRACE_D("Create", 0, 0, 0);

    ICarrierConfig* piCc = ConfigService::GetConfigService()->GetCarrierConfig(nSlotId);

    if (piCc == IMS_NULL)
    {
        return IMS_FALSE;
    }
    piCc->AddListener(this);

    m_bIsSessLevelBW =
            piCc->GetBoolean(CarrierConfig::ImsVoice::KEY_MEDIA_SESSION_LEVEL_BANDWIDTH_BOOL);
    m_bAnbrSupported =
            piCc->GetBoolean(CarrierConfig::ImsVoice::KEY_MEDIA_ANBR_CAPABILITY_IN_MODEM_BOOL);
    m_bSupportMultiConfigInEarlySession = piCc->GetBoolean(
            CarrierConfig::ImsVoice::KEY_SUPPORT_MULTI_CONFIG_IN_EARLY_SESSION_BOOL);
    m_bSdpReofferFullCapability =
            piCc->GetBoolean(CarrierConfig::ImsVoice::KEY_SDP_REOFFER_FULL_CAPABILITY_BOOL);

    if (m_pAudioConfig == IMS_NULL)
    {
        CreateAudioConfiguration(piCc);
    }
    if (m_pVideoConfig == IMS_NULL)
    {
        CreateVideoConfiguration(piCc);
    }
    if (m_pTextConfig == IMS_NULL)
    {
        CreateTextConfiguration(piCc);
    }

    ToDebugString();

    return IMS_TRUE;
}

PUBLIC
void MediaSessionConfig::SetServiceType(IN MEDIA_SERVICE_TYPE serviceType)
{
    IMS_TRACE_D("SetServiceType - ServiceType[%d]", serviceType, 0, 0);
    m_nServiceType = serviceType;
}

PUBLIC
void MediaSessionConfig::ToDebugString() const
{
    IMS_TRACE_D("ServiceType[%d], IsSessLevelBandwidth[%d], AnbrSupported[%d]", m_nServiceType,
            m_bIsSessLevelBW, m_bAnbrSupported);
    IMS_TRACE_D("SupportMultiConfigInEarlySession[%d], SdpReofferFullCapability[%d]",
            m_bSupportMultiConfigInEarlySession, m_bSdpReofferFullCapability, 0);
}

PUBLIC
AudioConfiguration* MediaSessionConfig::GetAudioConfiguration() const
{
    return m_pAudioConfig;
}

PUBLIC
VideoConfiguration* MediaSessionConfig::GetVideoConfiguration() const
{
    return m_pVideoConfig;
}

PUBLIC
TextConfiguration* MediaSessionConfig::GetTextConfiguration() const
{
    return m_pTextConfig;
}

PUBLIC
MEDIA_SERVICE_TYPE MediaSessionConfig::GetServiceType() const
{
    return m_nServiceType;
}

PUBLIC
IMS_BOOL MediaSessionConfig::IsSessionLevelBandwidth() const
{
    return m_bIsSessLevelBW;
}

PUBLIC
IMS_BOOL MediaSessionConfig::IsAnbrSupported() const
{
    return m_bAnbrSupported;
}

PUBLIC
IMS_BOOL MediaSessionConfig::IsSupportMultiConfigInEarlySession() const
{
    return m_bSupportMultiConfigInEarlySession;
}

PUBLIC
IMS_BOOL MediaSessionConfig::IsSdpReofferFullCapability() const
{
    return m_bSdpReofferFullCapability;
}

PRIVATE
void MediaSessionConfig::Clear()
{
    IMS_TRACE_D("Clear()", 0, 0, 0);

    if (m_pAudioConfig != IMS_NULL)
    {
        delete m_pAudioConfig;
        m_pAudioConfig = IMS_NULL;
    }

    if (m_pVideoConfig != IMS_NULL)
    {
        delete m_pVideoConfig;
        m_pVideoConfig = IMS_NULL;
    }

    if (m_pTextConfig != IMS_NULL)
    {
        delete m_pTextConfig;
        m_pTextConfig = IMS_NULL;
    }
}

PRIVATE
void MediaSessionConfig::ResetMediaConfigurations(IN IMS_SINT32 nSlotId)
{
    IMS_TRACE_D("ResetMediaConfigurations()", 0, 0, 0);
    Clear();
    Create(nSlotId);
}

PRIVATE VIRTUAL void MediaSessionConfig::CarrierConfig_NotifyConfigChanged(IN IMS_SINT32 nSlotId)
{
    if (MediaSessionConfigFactory::GetInstance()->FindMediaSessionConfig(nSlotId, m_nServiceType) ==
            this)
    {
        ResetMediaConfigurations(nSlotId);
    }
}

PROTECTED
IMS_BOOL MediaSessionConfig::CreateAudioConfiguration(IN ICarrierConfig* piCc)
{
    IMS_TRACE_D("CreateAudioConfiguration()", 0, 0, 0);

    AudioConfiguration* pConfig = new AudioConfiguration(MEDIA_TYPE_AUDIO);

    if (pConfig == IMS_NULL)
    {
        return IMS_FALSE;
    }

    if (!pConfig->Create(piCc))
    {
        IMS_TRACE_E(0, "CreateAudioConfiguration - failed", 0, 0, 0);

        delete pConfig;
        return IMS_FALSE;
    }

    m_pAudioConfig = pConfig;

    return IMS_TRUE;
}

PROTECTED
IMS_BOOL MediaSessionConfig::CreateVideoConfiguration(IN ICarrierConfig* piCc)
{
    IMS_TRACE_D("CreateVideoConfiguration()", 0, 0, 0);

    VideoConfiguration* pConfig = new VideoConfiguration(MEDIA_TYPE_AUDIOVIDEO);

    if (pConfig == IMS_NULL)
    {
        return IMS_FALSE;
    }

    if (!pConfig->Create(piCc))
    {
        IMS_TRACE_E(0, "CreateVideoConfiguration - failed", 0, 0, 0);

        delete pConfig;
        return IMS_FALSE;
    }

    m_pVideoConfig = pConfig;

    return IMS_TRUE;
}

PROTECTED
IMS_BOOL MediaSessionConfig::CreateTextConfiguration(IN ICarrierConfig* piCc)
{
    IMS_TRACE_D("CreateTextConfiguration()", 0, 0, 0);

    TextConfiguration* pConfig = new TextConfiguration(MEDIA_TYPE_TEXT);

    if (pConfig == IMS_NULL)
    {
        return IMS_FALSE;
    }

    if (!pConfig->Create(piCc))
    {
        IMS_TRACE_E(0, "CreateTextConfiguration - failed", 0, 0, 0);

        delete pConfig;
        return IMS_FALSE;
    }

    m_pTextConfig = pConfig;

    return IMS_TRUE;
}
