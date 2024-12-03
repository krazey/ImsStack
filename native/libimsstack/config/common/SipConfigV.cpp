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

#include "CarrierConfig.h"
#include "private/SipConfigV.h"

__IMS_TRACE_TAG_CONF__;

PUBLIC
SipConfigV::SipConfigV(IN IMS_SINT32 nSlotId) :
        ConfigBase(nSlotId),
        m_nTargetNumberFormat(TARGET_NUMBER_FORMAT_LOCAL),
        m_nTargetScheme(TARGET_SCHEME_TEL),
        m_nPreferredId(PREFERRED_ID_DEFAULT),
        m_strServiceVersion(AString::ConstNull()),
        m_strPredefinedPaniForEutran(AString::ConstNull()),
        m_strPredefinedPaniForWlan(AString::ConstNull()),
        m_strPredefinedPaniForUtran(AString::ConstNull()),
        m_nFeatureTagOptions(FEATURE_TAG_MMTEL_DEFAULT),
        m_bIsTimerValueConfigured(IMS_TRUE),
        m_nTimerValueT1(DEFAULT_TIMER_T1),
        m_nTimerValueT2(DEFAULT_TIMER_T2),
        m_nTimerValueT4(DEFAULT_TIMER_T4),
        m_nTimerValueA(DEFAULT_TIMER_T1),
        m_nTimerValueB(DEFAULT_TIMER_T1 * 64),
        m_nTimerValueC(DEFAULT_TIMER_C),
        m_nTimerValueD(DEFAULT_TIMER_D),
        m_nTimerValueE(DEFAULT_TIMER_T1),
        m_nTimerValueF(DEFAULT_TIMER_T1 * 64),
        m_nTimerValueG(DEFAULT_TIMER_T1),
        m_nTimerValueH(DEFAULT_TIMER_T1 * 64),
        m_nTimerValueI(DEFAULT_TIMER_T4),
        m_nTimerValueJ(DEFAULT_TIMER_T1 * 64),
        m_nTimerValueK(DEFAULT_TIMER_T4),
        m_objSession(Session()),
        m_bRespByAppForCapabilities(IMS_TRUE),
        m_bRespByAppForPageMessage(IMS_TRUE),
        m_bRespByAppForReference(IMS_TRUE),
        m_pConfigurable(IMS_NULL)
{
    m_pConfigurable = new Configurable(this);
}

PUBLIC VIRTUAL SipConfigV::~SipConfigV()
{
    ICarrierConfig* piCc = GetCarrierConfig();
    piCc->RemoveListener(this);

    if (m_pConfigurable != IMS_NULL)
    {
        delete m_pConfigurable;
        m_pConfigurable = IMS_NULL;
    }
}

PUBLIC VIRTUAL IMS_BOOL SipConfigV::Init()
{
    ICarrierConfig* piCc = GetCarrierConfig();
    piCc->AddListener(this);

    return ConfigBase::Init();
}

PUBLIC VIRTUAL void SipConfigV::Refresh()
{
    ReadFrom();
}

PUBLIC VIRTUAL IMS_SINT32 SipConfigV::GetTimerValue(IN IMS_SINT32 nType) const
{
    switch (nType)
    {
        case TIMER_T1:
            return GetTimerValueT1();
        case TIMER_T2:
            return GetTimerValueT2();
        case TIMER_T4:
            return GetTimerValueT4();
        case TIMER_A:
            return GetTimerValueA();
        case TIMER_B:
            return GetTimerValueB();
        case TIMER_C:
            return GetTimerValueC();
        case TIMER_D:
            return GetTimerValueD();
        case TIMER_E:
            return GetTimerValueE();
        case TIMER_F:
            return GetTimerValueF();
        case TIMER_G:
            return GetTimerValueG();
        case TIMER_H:
            return GetTimerValueH();
        case TIMER_I:
            return GetTimerValueI();
        case TIMER_J:
            return GetTimerValueJ();
        case TIMER_K:
            return GetTimerValueK();
        default:
            break;
    }

    return (-1);
}

PROTECTED VIRTUAL IMS_BOOL SipConfigV::ReadFrom()
{
    ICarrierConfig* piCc = GetCarrierConfig();

    IMS_SINT32 nRequestUriType = piCc->GetInt(CarrierConfig::Ims::KEY_REQUEST_URI_TYPE_INT);

    if (nRequestUriType == CarrierConfig::Ims::REQUEST_URI_FORMAT_SIP)
    {
        m_nTargetScheme = TARGET_SCHEME_SIP;
    }
    else
    {
        m_nTargetScheme = TARGET_SCHEME_TEL;
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

    m_strServiceVersion = piCc->GetString(CarrierConfig::Ims::KEY_IMS_USER_AGENT_STRING);

    m_nTimerValueT1 =
            GetTimerValue(piCc, CarrierConfig::Ims::KEY_SIP_TIMER_T1_MILLIS_INT, DEFAULT_TIMER_T1);

    m_nTimerValueT2 =
            GetTimerValue(piCc, CarrierConfig::Ims::KEY_SIP_TIMER_T2_MILLIS_INT, DEFAULT_TIMER_T2);

    m_nTimerValueT4 =
            GetTimerValue(piCc, CarrierConfig::Ims::KEY_SIP_TIMER_T4_MILLIS_INT, DEFAULT_TIMER_T4);

    m_nTimerValueA = m_nTimerValueT1;

    m_nTimerValueB = GetTimerValue(
            piCc, CarrierConfig::Ims::KEY_SIP_TIMER_B_MILLIS_INT, m_nTimerValueT1 * 64);

    m_nTimerValueC =
            GetTimerValue(piCc, CarrierConfig::Ims::KEY_SIP_TIMER_C_MILLIS_INT, DEFAULT_TIMER_C);

    m_nTimerValueD =
            GetTimerValue(piCc, CarrierConfig::Ims::KEY_SIP_TIMER_D_MILLIS_INT, DEFAULT_TIMER_D);

    m_nTimerValueE = m_nTimerValueT1;

    m_nTimerValueF = GetTimerValue(
            piCc, CarrierConfig::Ims::KEY_SIP_TIMER_F_MILLIS_INT, m_nTimerValueT1 * 64);

    m_nTimerValueG = m_nTimerValueT1;

    m_nTimerValueH = GetTimerValue(
            piCc, CarrierConfig::Ims::KEY_SIP_TIMER_H_MILLIS_INT, m_nTimerValueT1 * 64);

    m_nTimerValueI = m_nTimerValueT4;

    m_nTimerValueJ = GetTimerValue(
            piCc, CarrierConfig::Ims::KEY_SIP_TIMER_J_MILLIS_INT, m_nTimerValueT1 * 64);

    m_nTimerValueK = m_nTimerValueT4;

    m_objSession.bSessionTimerSupported =
            piCc->GetBoolean(CarrierConfig::ImsVoice::KEY_SESSION_TIMER_SUPPORTED_BOOL);

    IMS_SINT32 nRefresherType =
            piCc->GetInt(CarrierConfig::ImsVoice::KEY_SESSION_REFRESHER_TYPE_INT);

    if (nRefresherType == CarrierConfig::ImsVoice::SESSION_REFRESHER_TYPE_UAC)
    {
        m_objSession.nRefresher = SESSION_REFRESHER_LOCAL;
    }
    else if (nRefresherType == CarrierConfig::ImsVoice::SESSION_REFRESHER_TYPE_UAS)
    {
        m_objSession.nRefresher = SESSION_REFRESHER_REMOTE;
    }
    else
    {
        m_objSession.nRefresher = SESSION_REFRESHER_NONE;
    }

    IMS_SINT32 nRefreshMethod =
            piCc->GetInt(CarrierConfig::ImsVoice::KEY_SESSION_REFRESH_METHOD_INT);

    if (nRefreshMethod == CarrierConfig::ImsVoice::SESSION_REFRESH_METHOD_INVITE)
    {
        m_objSession.nRefreshMethod = SESSION_REFRESH_INVITE;
    }
    else if (nRefreshMethod == CarrierConfig::ImsVoice::SESSION_REFRESH_METHOD_UPDATE_PREFERRED)
    {
        m_objSession.nRefreshMethod = SESSION_REFRESH_UPDATE;
    }
    else
    {
        m_objSession.nRefreshMethod = SESSION_REFRESH_ANY;
    }

    m_objSession.nMinSe =
            piCc->GetInt(CarrierConfig::ImsVoice::KEY_MINIMUM_SESSION_EXPIRES_TIMER_SEC_INT);

    m_objSession.nSessionExpires =
            piCc->GetInt(CarrierConfig::ImsVoice::KEY_SESSION_EXPIRES_TIMER_SEC_INT);

    m_objSession.nHeaders = SESSION_HEADER_SESSION_EXPIRES | SESSION_HEADER_MIN_SE |
            SESSION_HEADER_CHECK_SESSION_EXPIRES;

    if (piCc->GetBoolean(CarrierConfig::Ims::KEY_SUPPORT_LOCAL_SESSION_TIMER_BOOL))
    {
        m_objSession.nHeaders |= SESSION_HEADER_LOCAL_TIMER_REQUIRED;
    }

    m_objSession.bNoRefreshByReInvite = !piCc->GetBoolean(CarrierConfig::Ims::
                    KEY_SESSION_TIMER_UPDATE_REQUIRED_IN_SESSION_UPDATE_BY_REINVITE_BOOL);

    m_objSession.bSdpVersionCheckSupported = IMS_TRUE;

    m_objSession.bSdpNonRprAllowed =
            piCc->GetBoolean(CarrierConfig::Ims::KEY_SDP_NEGOTIATION_REQUIRED_FOR_NON_RPR_BOOL);

    m_bRespByAppForCapabilities = IMS_TRUE;
    m_bRespByAppForPageMessage = IMS_TRUE;
    m_bRespByAppForReference = IMS_TRUE;

    return IMS_TRUE;
}

PROTECTED VIRTUAL IMS_BOOL SipConfigV::Update(
        IN IMS_SINT32 nCpi, IN const AString& strValue /*= AString::ConstNull()*/)
{
    IMS_BOOL bUpdateResult = IMS_TRUE;

    switch (nCpi)
    {
        case IConfigurable::CP_I_START_SIP_V:
        case IConfigurable::CP_I_END_SIP_V:
            // Control messages MUST be notified to the application...
            break;

        case IConfigurable::CP_I_TIMER_T1:
        {
            if (!GetTimerValueForUpdate(CarrierConfig::Ims::KEY_SIP_TIMER_T1_MILLIS_INT,
                        DEFAULT_TIMER_T1, strValue, m_nTimerValueT1))
            {
                return IMS_FALSE;
            }

            IMS_TRACE_D("TIMER_T1 :: %d", m_nTimerValueT1, 0, 0);
            break;
        }
        case IConfigurable::CP_I_TIMER_T2:
        {
            if (!GetTimerValueForUpdate(CarrierConfig::Ims::KEY_SIP_TIMER_T2_MILLIS_INT,
                        DEFAULT_TIMER_T2, strValue, m_nTimerValueT2))
            {
                return IMS_FALSE;
            }

            IMS_TRACE_D("TIMER_T2 :: %d", m_nTimerValueT2, 0, 0);
            break;
        }
        case IConfigurable::CP_I_TIMER_T4:
        {
            if (!GetTimerValueForUpdate(CarrierConfig::Ims::KEY_SIP_TIMER_T4_MILLIS_INT,
                        DEFAULT_TIMER_T4, strValue, m_nTimerValueT4))
            {
                return IMS_FALSE;
            }

            IMS_TRACE_D("TIMER_T4 :: %d", m_nTimerValueT4, 0, 0);
            break;
        }
        case IConfigurable::CP_I_TIMER_A:
        {
            if (!GetTimerValueForUpdate("", m_nTimerValueT1, strValue, m_nTimerValueA))
            {
                return IMS_FALSE;
            }

            IMS_TRACE_D("TIMER_A :: %d", m_nTimerValueA, 0, 0);
            break;
        }
        case IConfigurable::CP_I_TIMER_B:
        {
            if (!GetTimerValueForUpdate(CarrierConfig::Ims::KEY_SIP_TIMER_B_MILLIS_INT,
                        m_nTimerValueT1 * 64, strValue, m_nTimerValueB))
            {
                return IMS_FALSE;
            }

            IMS_TRACE_D("TIMER_B :: %d", m_nTimerValueB, 0, 0);
            break;
        }
        case IConfigurable::CP_I_TIMER_C:
        {
            if (!GetTimerValueForUpdate(CarrierConfig::Ims::KEY_SIP_TIMER_C_MILLIS_INT,
                        DEFAULT_TIMER_C, strValue, m_nTimerValueC))
            {
                return IMS_FALSE;
            }

            IMS_TRACE_D("TIMER_C :: %d", m_nTimerValueC, 0, 0);
            break;
        }
        case IConfigurable::CP_I_TIMER_D:
        {
            if (!GetTimerValueForUpdate(CarrierConfig::Ims::KEY_SIP_TIMER_D_MILLIS_INT,
                        DEFAULT_TIMER_D, strValue, m_nTimerValueD))
            {
                return IMS_FALSE;
            }

            IMS_TRACE_D("TIMER_D :: %d", m_nTimerValueD, 0, 0);
            break;
        }
        case IConfigurable::CP_I_TIMER_E:
        {
            if (!GetTimerValueForUpdate("", m_nTimerValueT1, strValue, m_nTimerValueE))
            {
                return IMS_FALSE;
            }

            IMS_TRACE_D("TIMER_E :: %d", m_nTimerValueE, 0, 0);
            break;
        }
        case IConfigurable::CP_I_TIMER_F:
        {
            if (!GetTimerValueForUpdate(CarrierConfig::Ims::KEY_SIP_TIMER_F_MILLIS_INT,
                        m_nTimerValueT1 * 64, strValue, m_nTimerValueF))
            {
                return IMS_FALSE;
            }

            IMS_TRACE_D("TIMER_F :: %d", m_nTimerValueF, 0, 0);
            break;
        }
        case IConfigurable::CP_I_TIMER_G:
        {
            if (!GetTimerValueForUpdate("", m_nTimerValueT1, strValue, m_nTimerValueG))
            {
                return IMS_FALSE;
            }

            IMS_TRACE_D("TIMER_G :: %d", m_nTimerValueG, 0, 0);
            break;
        }
        case IConfigurable::CP_I_TIMER_H:
        {
            if (!GetTimerValueForUpdate(CarrierConfig::Ims::KEY_SIP_TIMER_H_MILLIS_INT,
                        m_nTimerValueT1 * 64, strValue, m_nTimerValueH))
            {
                return IMS_FALSE;
            }

            IMS_TRACE_D("TIMER_H :: %d", m_nTimerValueH, 0, 0);
            break;
        }
        case IConfigurable::CP_I_TIMER_I:
        {
            if (!GetTimerValueForUpdate("", m_nTimerValueT4, strValue, m_nTimerValueI))
            {
                return IMS_FALSE;
            }

            IMS_TRACE_D("TIMER_I :: %d", m_nTimerValueI, 0, 0);
            break;
        }
        case IConfigurable::CP_I_TIMER_J:
        {
            if (!GetTimerValueForUpdate(CarrierConfig::Ims::KEY_SIP_TIMER_J_MILLIS_INT,
                        m_nTimerValueT1 * 64, strValue, m_nTimerValueJ))
            {
                return IMS_FALSE;
            }

            IMS_TRACE_D("TIMER_J :: %d", m_nTimerValueJ, 0, 0);
            break;
        }
        case IConfigurable::CP_I_TIMER_K:
        {
            if (!GetTimerValueForUpdate("", m_nTimerValueT4, strValue, m_nTimerValueK))
            {
                return IMS_FALSE;
            }

            IMS_TRACE_D("TIMER_K :: %d", m_nTimerValueK, 0, 0);
            break;
        }
        case IConfigurable::CP_I_UA_VERSION:
        {
            if (!strValue.IsNULL())
            {
                m_strServiceVersion = strValue;
            }
            else
            {
                ICarrierConfig* piCc = GetCarrierConfig();
                m_strServiceVersion =
                        piCc->GetString(CarrierConfig::Ims::KEY_IMS_USER_AGENT_STRING);
            }

            IMS_TRACE_D("SERVICE_VERSION :: %s", m_strServiceVersion.GetStr(), 0, 0);
            break;
        }
        case IConfigurable::CP_I_FEATURE_TAG_OPTIONS:
        {
            if (strValue.GetLength() > 0)
            {
                m_nFeatureTagOptions = strValue.ToUInt32(IMS_NULL, 16);
            }
            else
            {
                m_nFeatureTagOptions = FEATURE_TAG_DEFAULT;
            }

            IMS_TRACE_D("FEATURE_TAGS :: %08X", m_nFeatureTagOptions, 0, 0);
            break;
        }
        case IConfigurable::CP_I_SESSION_MINSE:
        {
            if (!GetTimerValueForUpdate(
                        CarrierConfig::ImsVoice::KEY_MINIMUM_SESSION_EXPIRES_TIMER_SEC_INT, -1,
                        strValue, m_objSession.nMinSe))
            {
                return IMS_FALSE;
            }

            IMS_TRACE_D("SESSION_MINSE :: %d", m_objSession.nMinSe, 0, 0);
            break;
        }
        case IConfigurable::CP_I_SESSION_EXPIRES:
        {
            if (!GetTimerValueForUpdate(CarrierConfig::ImsVoice::KEY_SESSION_EXPIRES_TIMER_SEC_INT,
                        -1, strValue, m_objSession.nSessionExpires))
            {
                return IMS_FALSE;
            }

            IMS_TRACE_D("SESSION_EXPIRES :: %d", m_objSession.nSessionExpires, 0, 0);
            break;
        }
        case IConfigurable::CP_I_SIP_ALL:
        {
            UpdateAllConfigs();
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

PROTECTED VIRTUAL void SipConfigV::CarrierConfig_NotifyConfigChanged(IN IMS_SINT32 nSlotId)
{
    if (nSlotId != GetSlotId())
    {
        return;
    }

    IMS_TRACE_D("SipConfig: CarrierConfigChanged(%d)", nSlotId, 0, 0);

    ReadFrom();

    NotifyUpdate(IConfigurable::CP_I_SIP_ALL);
}

PRIVATE
IMS_BOOL SipConfigV::GetTimerValueForUpdate(IN const IMS_CHAR* pszKey, IN IMS_SINT32 nDefaultValue,
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
void SipConfigV::UpdateAllConfigs()
{
    if (!ReadFrom())
    {
        IMS_TRACE_E(0, "Updating all the configs failed", 0, 0, 0);
    }
}

PRIVATE GLOBAL IMS_SINT32 SipConfigV::GetTimerValue(
        IN ICarrierConfig* piCc, IN const IMS_CHAR* pszKey, IN IMS_SINT32 nDefaultValue)
{
    IMS_SINT32 nTimerValue = piCc->GetInt(pszKey, nDefaultValue);

    if (nTimerValue <= 0)
    {
        return nDefaultValue;
    }

    return nTimerValue;
}
