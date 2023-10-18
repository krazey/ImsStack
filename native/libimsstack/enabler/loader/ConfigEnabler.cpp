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
#include "ITraceOption.h"
#include "ServiceConfig.h"
#include "ServiceMemory.h"
#include "ServiceUtil.h"

#include "offeranswer/SdpProfile.h"

#include "Configuration.h"

#include "ISipRtConfigHelper.h"
#include "SipFactory.h"

#include "ConfigAppFactory.h"
#include "ConfigEnabler.h"
#include "GeolocationHelper.h"

PUBLIC
ConfigEnabler::ConfigEnabler(IN IMS_SINT32 nSlotId) :
        Enabler(nSlotId),
        m_pConfigApp(IMS_NULL),
        m_bUseResetWhenClosingSipTcpConnection(IMS_FALSE)
{
}

PUBLIC VIRTUAL ConfigEnabler::~ConfigEnabler()
{
    if (m_pConfigApp != IMS_NULL)
    {
        ConfigAppFactory::Destroy(m_pConfigApp);
    }
}

PRIVATE VIRTUAL void ConfigEnabler::Start()
{
    ICarrierConfig* piCc = ConfigService::GetConfigService()->GetCarrierConfig(GetSlotId());
    ISipRtConfigHelper* piRtConfigHelper = SipFactory::GetRtConfigHelper(GetSlotId());

    if (piRtConfigHelper != IMS_NULL)
    {
        // Set a socket option (TCP socket address reuse)
        SipRtConfig::SocketOption objSocketOption;
        objSocketOption.nValue = 1;

        piRtConfigHelper->SetConfig(SipRtConfig::CONFIG_I_REUSEADDR, &objSocketOption);

        Configuration* pConfiguration = Configuration::GetInstance();

        if (((pConfiguration->GetTraceOption(GetSlotId()) & ITraceOption::OPT_HIDE_PRIVACY) ==
                    ITraceOption::OPT_HIDE_PRIVACY) ||
                IMS_UTIL_SYS_PROP_IS_SERVER_INFO_HIDDEN_IN_LOG())
        {
            // LOG_EXCLUDING_SERVER_INFO
            SipRtConfig::LogMask objLogMask;
            objLogMask.nValue |= SipRtConfig::LogMask::I_MESSAGE_HIDDEN;
            objLogMask.nValue |= SipRtConfig::LogMask::I_ROUTING_INFO_HIDDEN;

            piRtConfigHelper->SetConfig(SipRtConfig::CONFIG_I_LOG_MASK, &objLogMask);
        }

        if (piCc->GetBoolean(
                    CarrierConfig::Assets::KEY_USE_RESET_WHEN_CLOSING_SIP_TCP_CONNECTION_BOOL))
        {
            m_bUseResetWhenClosingSipTcpConnection = IMS_TRUE;
            objSocketOption.nValue = 0;
            piRtConfigHelper->SetConfig(SipRtConfig::CONFIG_I_LINGER, &objSocketOption);

            objSocketOption.nValue = 3;  // no shutdown
            piRtConfigHelper->SetConfig(SipRtConfig::CONFIG_I_SHUTDOWN, &objSocketOption);
        }

        // It's based on the Verizon's requirement, but it will be applied for all the carriers.
        SipRtConfig::TcpPortRange objTcpPortRange;

        objTcpPortRange.nPortStart = 40000;
        objTcpPortRange.nPortEnd = 50000;
        piRtConfigHelper->SetConfig(SipRtConfig::CONFIG_I_TCP_PORT_RANGE, &objTcpPortRange);
    }

    if (m_pConfigApp != IMS_NULL)
    {
        ConfigAppFactory::Destroy(m_pConfigApp);
    }

    m_pConfigApp = ConfigAppFactory::Create(GetSlotId());

    GeolocationPidfCreator* pPidfCreator =
            GeolocationHelper::GetInstance()->GetPidfCreator(GetSlotId());

    if (pPidfCreator != IMS_NULL)
    {
        IMS_SINT32 nFeatures = 0;

        if (piCc->GetBoolean(
                    CarrierConfig::Assets::KEY_USE_TUPLE_ELEMENT_FOR_GEOLOCATION_PIDF_BOOL))
        {
            nFeatures |= GeolocationPidfCreator::FEATURE_FORMAT_TUPLE;
        }

        if (!piCc->GetBoolean(CarrierConfig::Assets::
                            KEY_ALLOW_UNKNOWN_COUNTRY_ELEMENT_FOR_GEOLOCATION_PIDF_BOOL))
        {
            nFeatures |= GeolocationPidfCreator::FEATURE_NO_COUNTRY_IF_UNKNOWN;
        }

        pPidfCreator->SetFeatures(nFeatures);
    }

    IMS_SINT32 nSdpFeatures = SdpProfile::FEATURE_NONE;

    if (piCc->GetBoolean(CarrierConfig::Assets::KEY_SUPPORT_SDP_PRECONDITION_BOOL))
    {
        nSdpFeatures |= SdpProfile::FEATURE_A_PRECONDITION_SUPPORTED;
    }

    if (piCc->GetBoolean(
                CarrierConfig::Assets::KEY_SET_SDP_DIRECTION_ATTRIBUTE_FOR_REMOVED_MEDIA_BOOL))
    {
        nSdpFeatures |= SdpProfile::FEATURE_A_DIRECTION_REQUIRED_FOR_REMOVED_MEDIA;
    }

    SdpProfile::GetInstance()->InitFeatures(GetSlotId(), nSdpFeatures);
}

PRIVATE VIRTUAL void ConfigEnabler::Stop()
{
    GeolocationPidfCreator* pPidfCreator =
            GeolocationHelper::GetInstance()->GetPidfCreator(GetSlotId());

    if (pPidfCreator != IMS_NULL)
    {
        pPidfCreator->ClearFeatures(GeolocationPidfCreator::FEATURE_ALL);
        pPidfCreator->SetTupleId(AString::ConstNull());
    }

    if (m_pConfigApp != IMS_NULL)
    {
        ConfigAppFactory::Destroy(m_pConfigApp);
    }

    ISipRtConfigHelper* piRtConfigHelper = SipFactory::GetRtConfigHelper(GetSlotId());

    if (piRtConfigHelper != IMS_NULL)
    {
        SipRtConfig::SocketOption objSocketOption;

        objSocketOption.nValue = 1;
        piRtConfigHelper->RemoveConfig(SipRtConfig::CONFIG_I_REUSEADDR, &objSocketOption);
        piRtConfigHelper->RemoveConfig(SipRtConfig::CONFIG_I_LOG_MASK, IMS_NULL);

        if (m_bUseResetWhenClosingSipTcpConnection)
        {
            objSocketOption.nValue = 0;
            piRtConfigHelper->RemoveConfig(SipRtConfig::CONFIG_I_LINGER, &objSocketOption);

            objSocketOption.nValue = 3;
            piRtConfigHelper->RemoveConfig(SipRtConfig::CONFIG_I_SHUTDOWN, &objSocketOption);
        }

        piRtConfigHelper->RemoveConfig(SipRtConfig::CONFIG_I_TCP_PORT_RANGE, IMS_NULL);
    }
}
