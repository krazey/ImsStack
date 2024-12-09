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
#include "ServiceUtil.h"

#include "private/SipConfig.h"

__IMS_TRACE_TAG_CONF__;

PRIVATE GLOBAL const IMS_CHAR SipConfig::TAG_PREFIX[] = "9009";

// As default, TCP connection for SIP engine is not actively closed
// even though the socket is not used to send or receive SIP packets for a long time.
// So, m_nTvKeepAlive is set to TcpTimerValues::PERMANENT.
PUBLIC
SipConfig::TcpTimerValues::TcpTimerValues() :
        m_nTvConnectionWaiting(-1),
        m_nTvKeepAlive(TcpTimerValues::PERMANENT),
        m_nTvWouldblockWaiting(-1)
{
}

PUBLIC
SipConfig::TcpTimerValues::TcpTimerValues(IN const SipConfig::TcpTimerValues& other) :
        m_nTvConnectionWaiting(other.m_nTvConnectionWaiting),
        m_nTvKeepAlive(other.m_nTvKeepAlive),
        m_nTvWouldblockWaiting(other.m_nTvWouldblockWaiting)
{
}

PUBLIC
SipConfig::TcpTimerValues::~TcpTimerValues() {}

PUBLIC
SipConfig::TcpTimerValues& SipConfig::TcpTimerValues::operator=(
        IN const SipConfig::TcpTimerValues& other)
{
    if (this != &other)
    {
        m_nTvConnectionWaiting = other.m_nTvConnectionWaiting;
        m_nTvKeepAlive = other.m_nTvKeepAlive;
        m_nTvWouldblockWaiting = other.m_nTvWouldblockWaiting;
    }

    return (*this);
}

PUBLIC
SipConfig::SipConfig(IN IMS_SINT32 nSlotId) :
        ConfigBase(nSlotId),
        m_bCompactFormConfigured(IMS_TRUE),
        m_nSipFeatureCaps(SIP_FEATURE_CAPS_UA_SET_BY_CONTEXT),
        m_nDeviceId(DEVICE_ID_UUID_IMEI_SHA1),
        m_strDeviceId(AString::ConstNull()),
        m_strScheme("sip"),
        m_strTagPrefix(TAG_PREFIX),
        m_nListenChannel(DEFAULT_CHANNEL),
        m_nTransportType(TRANSPORT_TYPE_UDP),
        m_nTimerValue100Trying(200),
        m_nTcpCriterionLength(TCP_CRITERION_LEN),
        m_objTcpTimerValues(SipConfig::TcpTimerValues()),
        m_nHideMacInPaniHeader(HIDE_MAC_IN_PANI),
        m_nRegExpiresMask(EXPIRES_NONE),
        m_nRegExpiration(DEFAULT_EXPIRATION),
        m_bRegSubscription(IMS_TRUE),
        m_nRegSubExpiration(DEFAULT_EXPIRATION),
        m_pSipConfigV(IMS_NULL),
        m_pConfigurable(IMS_NULL)
{
    m_pSipConfigV = new SipConfigV(nSlotId);
    m_pConfigurable = new Configurable(this);
}

PUBLIC VIRTUAL SipConfig::~SipConfig()
{
    ICarrierConfig* piCc = GetCarrierConfig();
    piCc->RemoveListener(this);

    Clear();

    if (m_pConfigurable != IMS_NULL)
    {
        delete m_pConfigurable;
        m_pConfigurable = IMS_NULL;
    }
}

PUBLIC VIRTUAL const AString& SipConfig::GetUaVersion() const
{
    if (!IsUserAgentConfigured())
    {
        return AString::ConstNull();
    }

    if (m_pSipConfigV == IMS_NULL)
    {
        return AString::ConstNull();
    }

    return m_pSipConfigV->GetServiceVersion();
}

PUBLIC VIRTUAL IMS_BOOL SipConfig::Init()
{
    ICarrierConfig* piCc = GetCarrierConfig();
    piCc->AddListener(this);

    IMS_BOOL bInitResult = ConfigBase::Init();

    if (!m_pSipConfigV->Init())
    {
        IMS_TRACE_E(0, "Loading a service specific SIP config failed", 0, 0, 0);
        bInitResult = IMS_FALSE;
    }

    // Update TCP related timer values based on a default SIP configuration
    // if it's not provisioned
    UpdateTcpTimerValues();

    return bInitResult;
}

PUBLIC VIRTUAL void SipConfig::Refresh()
{
    ReadFrom();

    m_pSipConfigV->Refresh();

    // Update TCP related timer values based on a default SIP configuration
    // if it's not provisioned
    UpdateTcpTimerValues();
}

PUBLIC
IMS_SINT32 SipConfig::GetTimerValueT1() const
{
    const SipConfigV* pSipConfigV = GetServiceConfig();

    return (pSipConfigV != IMS_NULL) ? pSipConfigV->GetTimerValueT1()
                                     : SipConfigV::DEFAULT_TIMER_T1;
}

PUBLIC
IMS_SINT32 SipConfig::GetTimerValueT2() const
{
    const SipConfigV* pSipConfigV = GetServiceConfig();

    return (pSipConfigV != IMS_NULL) ? pSipConfigV->GetTimerValueT2()
                                     : SipConfigV::DEFAULT_TIMER_T2;
}

PROTECTED VIRTUAL IMS_BOOL SipConfig::ReadFrom()
{
    ICarrierConfig* piCc = GetCarrierConfig();

    m_bCompactFormConfigured =
            piCc->GetBoolean(CarrierConfig::Ims::KEY_SIP_COMPACT_FORM_ENABLED_BOOL);

    m_nSipFeatureCaps = ReadSipFeatureCaps(piCc);

    m_nDeviceId = DEVICE_ID_GSMA_IMEI;

    m_nTimerValue100Trying = piCc->GetInt(CarrierConfig::Ims::KEY_SIP_TIMER_100_TRYING_MILLIS_INT);

    IImsPrivateProperty* piProperty = UtilService::GetUtilService()->GetPrivateProperty();

    m_strDeviceId = piProperty->GetPersistent(
            ImsPrivateProperties::Persistent::KEY_SIP_DEVICE_ID, GetSlotId());

    m_nTcpCriterionLength = piCc->GetInt(CarrierConfig::Ims::KEY_IPV6_SIP_MTU_SIZE_CELLULAR_INT);

    m_nTransportType = piCc->GetInt(CarrierConfig::Ims::KEY_SIP_PREFERRED_TRANSPORT_INT);

    m_nHideMacInPaniHeader =
            piCc->GetInt(CarrierConfig::Ims::KEY_HIDE_MAC_ADDRESS_IN_PANI_HEADER_INT);

    m_nRegExpiration = piCc->GetInt(CarrierConfig::Ims::KEY_REGISTRATION_EXPIRY_TIMER_SEC_INT);
    m_nRegExpiresMask = EXPIRES_NONE;

    if (m_nRegExpiration > 0)
    {
        m_nRegExpiresMask |= EXPIRES_REG;
    }
    else
    {
        m_nRegExpiration = DEFAULT_EXPIRATION;
    }

    m_objAllowMethods.RemoveAllElements();
    m_objAllowMethods.AddElement("INVITE");
    m_objAllowMethods.AddElement("BYE");
    m_objAllowMethods.AddElement("CANCEL");
    m_objAllowMethods.AddElement("ACK");
    m_objAllowMethods.AddElement("NOTIFY");
    m_objAllowMethods.AddElement("UPDATE");
    m_objAllowMethods.AddElement("REFER");
    m_objAllowMethods.AddElement("PRACK");
    m_objAllowMethods.AddElement("INFO");
    m_objAllowMethods.AddElement("MESSAGE");
    m_objAllowMethods.AddElement("OPTIONS");

    m_bRegSubscription =
            piCc->GetBoolean(CarrierConfig::Ims::KEY_REGISTRATION_EVENT_PACKAGE_SUPPORTED_BOOL);

    m_nRegSubExpiration =
            piCc->GetInt(CarrierConfig::Ims::KEY_REGISTRATION_SUBSCRIBE_EXPIRY_TIMER_SEC_INT);

    if (m_nRegSubExpiration > 0)
    {
        m_nRegExpiresMask |= EXPIRES_REG_SUB;
    }
    else
    {
        m_nRegSubExpiration = DEFAULT_EXPIRATION;
    }

    IMS_TRACE_D("SIP feature caps: %08X", m_nSipFeatureCaps, 0, 0);

    return IMS_TRUE;
}

PROTECTED VIRTUAL IMS_BOOL SipConfig::Update(
        IN IMS_SINT32 nCpi, IN const AString& strValue /*= AString::ConstNull()*/)
{
    IMS_BOOL bUpdateResult = IMS_TRUE;

    (void)strValue;

    switch (nCpi)
    {
        case IConfigurable::CP_I_START_SIP:
        case IConfigurable::CP_I_END_SIP:
            // Control messages MUST be notified to the application...
            break;

        case IConfigurable::CP_I_TIMER_T1:  // FALL-THROUGH
        case IConfigurable::CP_I_TIMER_T2:
        {
            const ISipConfigV* piSipConfigV = GetSipConfigV();

            if (piSipConfigV != IMS_NULL)
            {
                piSipConfigV->GetConfigurable()->Update(nCpi, strValue);
            }
            break;
        }
        case IConfigurable::CP_I_TIMER_100_TRYING:
        {
            if (strValue.GetLength() > 0)
            {
                m_nTimerValue100Trying = strValue.ToInt32();
            }

            IMS_TRACE_D("Timer value for 100 TRYING :: %d", m_nTimerValue100Trying, 0, 0);
            break;
        }
        case IConfigurable::CP_I_SIP_FEATURES:
        {
            if (strValue.GetLength() > 0)
            {
                m_nSipFeatureCaps = static_cast<IMS_SINT32>(strValue.ToUInt32(IMS_NULL, 16));
            }
            else
            {
                m_nSipFeatureCaps = ReadSipFeatureCaps(GetCarrierConfig());
            }

            IMS_TRACE_D("SIP_FEATURES :: %08X", m_nSipFeatureCaps, 0, 0);
            break;
        }
        case IConfigurable::CP_I_TCP_CRITERION_LENGTH:
        {
            if (strValue.GetLength() > 0)
            {
                m_nTcpCriterionLength = strValue.ToInt32();
            }
            else
            {
                ICarrierConfig* piCc = GetCarrierConfig();
                m_nTcpCriterionLength =
                        piCc->GetInt(CarrierConfig::Ims::KEY_IPV6_SIP_MTU_SIZE_CELLULAR_INT);
            }

            IMS_TRACE_D("TCP_CRITERION_LENGTH :: %d", m_nTcpCriterionLength, 0, 0);
            break;
        }
        case IConfigurable::CP_I_REG_EXPIRES:
        {
            if (!GetTimerValueForUpdate(CarrierConfig::Ims::KEY_REGISTRATION_EXPIRY_TIMER_SEC_INT,
                        DEFAULT_EXPIRATION, strValue, m_nRegExpiration))
            {
                return IMS_FALSE;
            }

            IMS_TRACE_D("REG_EXPIRES :: %d", m_nRegExpiration, 0, 0);
            break;
        }
        case IConfigurable::CP_I_REG_SUB:
        {
            if (strValue.GetLength() > 0)
            {
                m_bRegSubscription = strValue.EqualsIgnoreCase("true");
            }
            else
            {
                ICarrierConfig* piCc = GetCarrierConfig();
                m_bRegSubscription = piCc->GetBoolean(
                        CarrierConfig::Ims::KEY_REGISTRATION_EVENT_PACKAGE_SUPPORTED_BOOL);
            }

            IMS_TRACE_D("REG_SUB :: %s", _TRACE_B_(m_bRegSubscription), 0, 0);
            break;
        }
        case IConfigurable::CP_I_REG_SUB_EXPIRES:
        {
            if (!GetTimerValueForUpdate(
                        CarrierConfig::Ims::KEY_REGISTRATION_SUBSCRIBE_EXPIRY_TIMER_SEC_INT,
                        DEFAULT_EXPIRATION, strValue, m_nRegSubExpiration))
            {
                return IMS_FALSE;
            }

            IMS_TRACE_D("REG_SUB_EXPIRES :: %d", m_nRegSubExpiration, 0, 0);
            break;
        }
        default:
        {
            bUpdateResult = IMS_FALSE;
            IMS_TRACE_D("No configurable parameter item (%d)", nCpi, 0, 0);
            break;
        }
    }

    if (bUpdateResult)
    {
        NotifyUpdate(nCpi);
    }

    return bUpdateResult;
}

PROTECTED VIRTUAL void SipConfig::CarrierConfig_NotifyConfigChanged(IN IMS_SINT32 nSlotId)
{
    if (nSlotId != GetSlotId())
    {
        return;
    }

    ReadFrom();
}

PRIVATE
void SipConfig::Clear()
{
    if (m_pSipConfigV != IMS_NULL)
    {
        delete m_pSipConfigV;
        m_pSipConfigV = IMS_NULL;
    }
}

PRIVATE
IMS_BOOL SipConfig::GetTimerValueForUpdate(IN const IMS_CHAR* pszKey, IN IMS_SINT32 nDefaultValue,
        IN const AString& strUpdateTimerValue, OUT IMS_SINT32& nTimerValue)
{
    if (strUpdateTimerValue.GetLength() > 0)
    {
        IMS_SINT32 nUpdateTimerValue = strUpdateTimerValue.ToInt32();

        if (nUpdateTimerValue <= 0)
        {
            IMS_TRACE_D(
                    "SIP timer(%s) is invalid; value=%s", pszKey, strUpdateTimerValue.GetStr(), 0);
            return IMS_FALSE;
        }

        nTimerValue = nUpdateTimerValue;
    }
    else
    {
        ICarrierConfig* piCc = GetCarrierConfig();
        nTimerValue = piCc->GetInt(pszKey, nDefaultValue);
    }

    return IMS_TRUE;
}

PRIVATE
void SipConfig::UpdateTcpTimerValues()
{
    IMS_SINT32 nTimerValueT1 = GetTimerValueT1();

    // Re-compute the TCP related timer values based on SIP transaction timer value
    if ((m_objTcpTimerValues.m_nTvKeepAlive < 0) &&
            (m_objTcpTimerValues.m_nTvKeepAlive != TcpTimerValues::PERMANENT))
    {
        m_objTcpTimerValues.m_nTvKeepAlive = nTimerValueT1 * 64;
    }

    if (m_objTcpTimerValues.m_nTvConnectionWaiting < 0)
    {
        m_objTcpTimerValues.m_nTvConnectionWaiting = nTimerValueT1 * 32;
    }

    if (m_objTcpTimerValues.m_nTvWouldblockWaiting < 0)
    {
        m_objTcpTimerValues.m_nTvWouldblockWaiting = nTimerValueT1 * 32;
    }

    IMS_TRACE_D("TcpTimerValues :: ka=%d, cw=%d, ww=%d", m_objTcpTimerValues.m_nTvKeepAlive,
            m_objTcpTimerValues.m_nTvConnectionWaiting, m_objTcpTimerValues.m_nTvWouldblockWaiting);
}

PRIVATE GLOBAL IMS_SINT32 SipConfig::ReadSipFeatureCaps(IN ICarrierConfig* piCc)
{
    IMS_SINT32 nSipFeatureCaps = SIP_FEATURE_CAPS_RPORT;

    nSipFeatureCaps |= SIP_FEATURE_CAPS_TRUST_DOMAIN;
    nSipFeatureCaps |= SIP_FEATURE_CAPS_PPI_HEADER_IN_REG_SUB;
    nSipFeatureCaps |= SIP_FEATURE_CAPS_EXPIRES_HEADER_IN_REG;
    nSipFeatureCaps |= SIP_FEATURE_CAPS_SIP_INSTANCE_FOR_CALLER_PREFERENCE;
    nSipFeatureCaps |= SIP_FEATURE_CAPS_CELLULAR_NETWORK_INFO_HEADER;
    nSipFeatureCaps |= SIP_FEATURE_CAPS_USER_AGENT;
    nSipFeatureCaps |= SIP_FEATURE_CAPS_CONTACT_IN_ALL_1XX;

    if (piCc->GetBoolean(CarrierConfig::Ims::KEY_GRUU_ENABLED_BOOL))
    {
        nSipFeatureCaps |= SIP_FEATURE_CAPS_GRUU;
    }

    if (piCc->GetBoolean(CarrierConfig::Ims::KEY_SIP_OVER_IPSEC_ENABLED_BOOL))
    {
        nSipFeatureCaps |= SIP_FEATURE_CAPS_IPSEC;
    }

    if (piCc->GetBoolean(
                CarrierConfig::Ims::KEY_ALLOW_SIP_UDP_FALLBACK_ON_TCP_CONNECTION_SETUP_FAILED_BOOL))
    {
        nSipFeatureCaps |= SIP_FEATURE_CAPS_UDP_FALLBACK;
    }

    if (piCc->GetBoolean(CarrierConfig::Ims::KEY_SDP_NEGOTIATION_REQUIRED_FOR_NON_RPR_BOOL))
    {
        nSipFeatureCaps |= SIP_FEATURE_CAPS_SDP_NEGOTIATION_REQUIRED_FOR_NON_RPR;
    }

    if (piCc->GetBoolean(
                CarrierConfig::Ims::KEY_REQUEST_URI_VALIDATION_REQUIRED_IN_MID_DIALOG_BOOL))
    {
        nSipFeatureCaps |= SIP_FEATURE_CAPS_REQUEST_URI_VALIDATION_REQUIRED_IN_MID_DIALOG;
    }

    if (piCc->GetBoolean(CarrierConfig::Ims::
                        KEY_SESSION_TIMER_UPDATE_REQUIRED_IN_SESSION_UPDATE_BY_REINVITE_BOOL))
    {
        nSipFeatureCaps |= SIP_FEATURE_CAPS_SESSION_TIMER_UPDATE_REQUIRED_BY_REINVITE;
    }

    if (piCc->GetBoolean(CarrierConfig::Ims::
                        KEY_ALLOW_SIP_INSTANCE_PARAM_IN_CONTACT_FOR_NON_REGISTER_REQUEST_BOOL))
    {
        nSipFeatureCaps |=
                SIP_FEATURE_CAPS_SIP_INSTANCE_PARAM_REQUIRED_IN_CONTACT_FOR_NON_REGISTER_REQUEST;
    }

    if (piCc->GetBoolean(CarrierConfig::Ims::KEY_SUPPORT_COUNTRY_PARAM_IN_PANI_HEADER_BOOL))
    {
        nSipFeatureCaps |= SIP_FEATURE_CAPS_COUNTRY_PARAM_IN_PANI_HEADER;
    }

    if (piCc->GetBoolean(CarrierConfig::Ims::KEY_SUPPORT_SIP_SESSION_ID_HEADER_BOOL))
    {
        nSipFeatureCaps |= SIP_FEATURE_CAPS_SUPPORT_SESSION_ID_HEADER;
    }

    if (piCc->GetBoolean(CarrierConfig::Ims::
                        KEY_ALLOW_SIP_P_ACCESS_NETWORK_INFO_HEADER_IN_INITIAL_REGISTER_BOOL))
    {
        nSipFeatureCaps |= SIP_FEATURE_CAPS_PANI_HEADER_IN_INITIAL_REG;
    }

    if (piCc->GetBoolean(
                CarrierConfig::Ims::KEY_ALLOW_ALGORITHM_PARAM_IN_SIP_AUTHORIZATION_HEADER_BOOL))
    {
        nSipFeatureCaps |= SIP_FEATURE_CAPS_AUTHENTICATION_ALGORITHM_PARAMETER;
    }

    if (!piCc->GetBoolean(CarrierConfig::Ims::KEY_USE_SIP_USER_AGENT_HEADER_IN_UA_STRING_BOOL))
    {
        nSipFeatureCaps |= SIP_FEATURE_CAPS_UA_SET_BY_CONTEXT;
    }

    return nSipFeatureCaps;
}
