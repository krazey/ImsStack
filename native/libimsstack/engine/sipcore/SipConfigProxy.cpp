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
#include "private/ConfigurationManager.h"
#include "private/SipConfig.h"

#include "SipConfigProxy.h"

PUBLIC GLOBAL IMS_SINT32 SipConfigProxy::GetDeviceId(
        IN IMS_SINT32 nSlotId, IN const SipProfile* pProfile /* = IMS_NULL*/)
{
    if (pProfile != IMS_NULL)
    {
        IMS_SINT32 nDeviceId = pProfile->GetDeviceId();

        if (nDeviceId != SipProfile::NOT_PROVISIONED)
        {
            return nDeviceId;
        }
    }

    const SipConfig* pSipConfig = ConfigurationManager::GetInstance()->GetSipConfig(nSlotId);

    return pSipConfig->GetDeviceId();
}

PUBLIC GLOBAL const AString& SipConfigProxy::GetPredefinedDeviceId(
        IN IMS_SINT32 nSlotId, IN const SipProfile* pProfile /* = IMS_NULL*/)
{
    if (pProfile != IMS_NULL)
    {
        const AString& strPredefinedDeviceId = pProfile->GetPredefinedDeviceId();

        if (strPredefinedDeviceId.GetLength() > 0)
        {
            return strPredefinedDeviceId;
        }
    }

    const SipConfig* pSipConfig = ConfigurationManager::GetInstance()->GetSipConfig(nSlotId);

    return pSipConfig->GetPredefinedDeviceId();
}

PUBLIC GLOBAL IMS_SINT32 SipConfigProxy::GetPort(
        IN IMS_SINT32 nSlotId, IN const SipProfile* pProfile /* = IMS_NULL*/)
{
    if (pProfile != IMS_NULL)
    {
        IMS_SINT32 nPort = pProfile->GetPort();

        if (nPort != SipProfile::NOT_PROVISIONED)
        {
            return nPort;
        }
    }

    const SipConfig* pSipConfig = ConfigurationManager::GetInstance()->GetSipConfig(nSlotId);

    return pSipConfig->GetPort();
}

PUBLIC GLOBAL IMS_UINT32 SipConfigProxy::GetSipFeatureCaps(
        IN IMS_SINT32 nSlotId, IN const SipProfile* pProfile /* = IMS_NULL*/)
{
    if (pProfile != IMS_NULL)
    {
        IMS_SINT32 nSipFeatures = pProfile->GetSipFeatureCaps();

        if (nSipFeatures != SipProfile::NOT_PROVISIONED)
        {
            return nSipFeatures;
        }
    }

    const SipConfig* pSipConfig = ConfigurationManager::GetInstance()->GetSipConfig(nSlotId);

    return pSipConfig->GetSipFeatureCaps();
}

PUBLIC GLOBAL const AString& SipConfigProxy::GetTagPrefix(
        IN IMS_SINT32 nSlotId, IN const SipProfile* pProfile /* = IMS_NULL*/)
{
    if (pProfile != IMS_NULL)
    {
        const AString& strTagPrefix = pProfile->GetTagPrefix();

        if (!strTagPrefix.IsNULL())
        {
            return strTagPrefix;
        }
    }

    const SipConfig* pSipConfig = ConfigurationManager::GetInstance()->GetSipConfig(nSlotId);

    return pSipConfig->GetTagPrefix();
}

PUBLIC GLOBAL IMS_SINT32 SipConfigProxy::GetTcpCriterionLength(
        IN IMS_SINT32 nSlotId, IN const SipProfile* pProfile /* = IMS_NULL*/)
{
    if (pProfile != IMS_NULL)
    {
        IMS_SINT32 nTcpCriterionLength = pProfile->GetTcpCriterionLength();

        if (nTcpCriterionLength != SipProfile::NOT_PROVISIONED)
        {
            return nTcpCriterionLength;
        }
    }

    const SipConfig* pSipConfig = ConfigurationManager::GetInstance()->GetSipConfig(nSlotId);

    return pSipConfig->GetTcpCriterionLength();
}

PUBLIC GLOBAL IMS_SINT32 SipConfigProxy::GetTransportType(
        IN IMS_SINT32 nSlotId, IN const SipProfile* /*pProfile = IMS_NULL*/)
{
    const SipConfig* pSipConfig = ConfigurationManager::GetInstance()->GetSipConfig(nSlotId);

    return pSipConfig->GetTransportType();
}

PUBLIC GLOBAL AString SipConfigProxy::GetUaString(
        IN IMS_SINT32 nSlotId, IN const SipProfile* pProfile /* = IMS_NULL*/)
{
    if (pProfile != IMS_NULL)
    {
        const AString& strUaString = pProfile->GetUaString();

        if (!strUaString.IsNULL())
        {
            return strUaString;
        }
    }

    const SipConfig* pSipConfig = ConfigurationManager::GetInstance()->GetSipConfig(nSlotId);

    return pSipConfig->GetUaVersion();
}

// "reg"
PUBLIC GLOBAL const AStringArray& SipConfigProxy::GetRegAllowMethods(
        IN IMS_SINT32 nSlotId, IN const SipProfile* pProfile /* = IMS_NULL*/)
{
    if (pProfile != IMS_NULL)
    {
        const AStringArray& objAllowMethods = pProfile->GetRegAllowMethods();

        if (!objAllowMethods.IsEmpty())
        {
            return objAllowMethods;
        }
    }

    const SipConfig* pSipConfig = ConfigurationManager::GetInstance()->GetSipConfig(nSlotId);

    return pSipConfig->GetRegAllowMethods();
}

PUBLIC GLOBAL IMS_SINT32 SipConfigProxy::GetRegContactUserInfoPart(IN IMS_SINT32 nSlotId)
{
    const SipConfig* pSipConfig = ConfigurationManager::GetInstance()->GetSipConfig(nSlotId);
    return pSipConfig->GetRegContactUserInfoPart();
}

PUBLIC GLOBAL IMS_SINT32 SipConfigProxy::GetRegExpires(
        IN IMS_SINT32 nSlotId, IN const SipProfile* pProfile /* = IMS_NULL*/)
{
    if (pProfile != IMS_NULL)
    {
        IMS_SINT32 nRegExpires = pProfile->GetRegExpires();

        if (nRegExpires != SipProfile::NOT_PROVISIONED)
        {
            return nRegExpires;
        }
    }

    const SipConfig* pSipConfig = ConfigurationManager::GetInstance()->GetSipConfig(nSlotId);

    return pSipConfig->GetRegExpiration();
}

PUBLIC GLOBAL IMS_SINT32 SipConfigProxy::GetRegSubExpires(
        IN IMS_SINT32 nSlotId, IN const SipProfile* pProfile /* = IMS_NULL*/)
{
    if (pProfile != IMS_NULL)
    {
        IMS_SINT32 nRegSubExpires = pProfile->GetRegSubExpires();

        if (nRegSubExpires != SipProfile::NOT_PROVISIONED)
        {
            return nRegSubExpires;
        }
    }

    const SipConfig* pSipConfig = ConfigurationManager::GetInstance()->GetSipConfig(nSlotId);

    return pSipConfig->GetRegSubExpiration();
}

PUBLIC GLOBAL AString SipConfigProxy::GetRegUaString(
        IN IMS_SINT32 nSlotId, IN const SipProfile* pProfile /* = IMS_NULL*/)
{
    if (pProfile != IMS_NULL)
    {
        const AString& strRegUaString = pProfile->GetRegUaString();

        if (!strRegUaString.IsNULL())
        {
            return strRegUaString;
        }
    }

    const SipConfig* pSipConfig = ConfigurationManager::GetInstance()->GetSipConfig(nSlotId);

    return pSipConfig->GetUaVersion();
}

PUBLIC GLOBAL IMS_BOOL SipConfigProxy::IsRegExpiresConfigured(
        IN IMS_SINT32 nSlotId, IN const SipProfile* pProfile /* = IMS_NULL*/)
{
    if (pProfile != IMS_NULL)
    {
        if (pProfile->GetRegExpires() != SipProfile::NOT_PROVISIONED)
        {
            return IMS_TRUE;
        }
    }

    const SipConfig* pSipConfig = ConfigurationManager::GetInstance()->GetSipConfig(nSlotId);

    return pSipConfig->IsRegExpirationConfigured();
}

PUBLIC GLOBAL IMS_BOOL SipConfigProxy::IsRegSubExpiresConfigured(
        IN IMS_SINT32 nSlotId, IN const SipProfile* pProfile /* = IMS_NULL*/)
{
    if (pProfile != IMS_NULL)
    {
        if (pProfile->GetRegSubExpires() != SipProfile::NOT_PROVISIONED)
        {
            return IMS_TRUE;
        }
    }

    const SipConfig* pSipConfig = ConfigurationManager::GetInstance()->GetSipConfig(nSlotId);

    return pSipConfig->IsRegSubExpirationConfigured();
}

PUBLIC GLOBAL IMS_BOOL SipConfigProxy::IsRegSubscriptionConfigured(
        IN IMS_SINT32 nSlotId, IN const SipProfile* pProfile /* = IMS_NULL*/)
{
    if (pProfile != IMS_NULL)
    {
        IMS_SINT32 nRegSubscription = pProfile->GetRegSubscription();

        if (nRegSubscription != SipProfile::NOT_PROVISIONED)
        {
            return (nRegSubscription != 0);
        }
    }

    const SipConfig* pSipConfig = ConfigurationManager::GetInstance()->GetSipConfig(nSlotId);

    return pSipConfig->IsRegSubscriptionConfigured();
}

PUBLIC GLOBAL const ISipConfigV* SipConfigProxy::GetSipConfigV(IN IMS_SINT32 nSlotId)
{
    const SipConfig* pSipConfig = ConfigurationManager::GetInstance()->GetSipConfig(nSlotId);

    return (pSipConfig != IMS_NULL) ? pSipConfig->GetSipConfigV() : IMS_NULL;
}

// SIP_FEATURES {
// FIXME: If the SIP features in SipProfile is zero, how to handle it?
// Do we need to check the default configuration??
PUBLIC GLOBAL IMS_BOOL SipConfigProxy::IsAuthenticationAlgorithmRequired(
        IN IMS_SINT32 nSlotId, IN const SipProfile* pProfile /* = IMS_NULL*/)
{
    if (pProfile != IMS_NULL)
    {
        if (pProfile->IsSipFeatureProvisioned())
        {
            return pProfile->IsAuthenticationAlgorithmRequired();
        }
    }

    const SipConfig* pSipConfig = ConfigurationManager::GetInstance()->GetSipConfig(nSlotId);

    return pSipConfig->IsAuthenticationAlgorithmRequired();
}

PUBLIC GLOBAL IMS_BOOL SipConfigProxy::IsCellularNetworkInfoHeaderRequired(
        IN IMS_SINT32 nSlotId, IN const SipProfile* pProfile /* = IMS_NULL*/)
{
    if (pProfile != IMS_NULL)
    {
        if (pProfile->IsSipFeatureProvisioned())
        {
            return pProfile->IsCellularNetworkInfoHeaderRequired();
        }
    }

    const SipConfig* pSipConfig = ConfigurationManager::GetInstance()->GetSipConfig(nSlotId);
    return pSipConfig->HasFeature(ISipConfig::SIP_FEATURE_CAPS_CELLULAR_NETWORK_INFO_HEADER);
}

PUBLIC GLOBAL IMS_BOOL SipConfigProxy::IsCompactFormConfigured(
        IN IMS_SINT32 nSlotId, IN const SipProfile* /*pProfile = IMS_NULL*/)
{
    const SipConfig* pSipConfig = ConfigurationManager::GetInstance()->GetSipConfig(nSlotId);

    if (pSipConfig != IMS_NULL)
    {
        return pSipConfig->IsCompactFormConfigured();
    }

    return IMS_FALSE;
}

PUBLIC GLOBAL IMS_BOOL SipConfigProxy::IsContactInAll1xxRequired(
        IN IMS_SINT32 nSlotId, IN const SipProfile* pProfile /* = IMS_NULL*/)
{
    if (pProfile != IMS_NULL)
    {
        if (pProfile->IsSipFeatureProvisioned())
        {
            return pProfile->IsContactInAll1xxRequired();
        }
    }

    const SipConfig* pSipConfig = ConfigurationManager::GetInstance()->GetSipConfig(nSlotId);

    return pSipConfig->IsContactInAll1xxRequired();
}

PUBLIC GLOBAL IMS_BOOL SipConfigProxy::IsCountryParameterSupportedInPaniHeader(
        IN IMS_SINT32 nSlotId, IN const SipProfile* pProfile /* = IMS_NULL*/)
{
    if (pProfile != IMS_NULL)
    {
        if (pProfile->IsSipFeatureProvisioned())
        {
            return pProfile->IsCountryParameterSupportedInPaniHeader();
        }
    }

    const SipConfig* pSipConfig = ConfigurationManager::GetInstance()->GetSipConfig(nSlotId);
    return pSipConfig->HasFeature(ISipConfig::SIP_FEATURE_CAPS_COUNTRY_PARAM_IN_PANI_HEADER);
}

PUBLIC GLOBAL IMS_BOOL SipConfigProxy::IsDisplayNameDquotRequired(
        IN IMS_SINT32 nSlotId, IN const SipProfile* pProfile /* = IMS_NULL*/)
{
    if (pProfile != IMS_NULL)
    {
        if (pProfile->IsSipFeatureProvisioned())
        {
            return pProfile->IsDisplayNameDquotRequired();
        }
    }

    const SipConfig* pSipConfig = ConfigurationManager::GetInstance()->GetSipConfig(nSlotId);

    if (pSipConfig != IMS_NULL)
    {
        return pSipConfig->IsDisplayNameDquotRequired();
    }

    return IMS_FALSE;
}

PUBLIC GLOBAL IMS_BOOL SipConfigProxy::IsExpiresHeaderInRegRequired(
        IN IMS_SINT32 nSlotId, IN const SipProfile* pProfile /* = IMS_NULL*/)
{
    if (pProfile != IMS_NULL)
    {
        if (pProfile->IsSipFeatureProvisioned())
        {
            return pProfile->IsExpiresHeaderInRegRequired();
        }
    }

    const SipConfig* pSipConfig = ConfigurationManager::GetInstance()->GetSipConfig(nSlotId);

    return pSipConfig->IsExpiresHeaderInRegRequired();
}

PUBLIC GLOBAL IMS_BOOL SipConfigProxy::IsIpSecConfigured(
        IN IMS_SINT32 nSlotId, IN const SipProfile* pProfile /* = IMS_NULL*/)
{
    if (pProfile != IMS_NULL)
    {
        if (pProfile->IsSipFeatureProvisioned())
        {
            return pProfile->IsIpSecConfigured();
        }
    }

    const SipConfig* pSipConfig = ConfigurationManager::GetInstance()->GetSipConfig(nSlotId);

    return pSipConfig->IsIpSecConfigured();
}

PUBLIC GLOBAL IMS_BOOL SipConfigProxy::IsGruuConfigured(
        IN IMS_SINT32 nSlotId, IN const SipProfile* pProfile /* = IMS_NULL*/)
{
    if (pProfile != IMS_NULL)
    {
        if (pProfile->IsSipFeatureProvisioned())
        {
            return pProfile->IsGruuConfigured();
        }
    }

    const SipConfig* pSipConfig = ConfigurationManager::GetInstance()->GetSipConfig(nSlotId);

    return pSipConfig->IsGruuConfigured();
}

PUBLIC GLOBAL IMS_BOOL SipConfigProxy::IsKeepAliveConfigured(
        IN IMS_SINT32 nSlotId, IN const SipProfile* pProfile /* = IMS_NULL*/)
{
    if (pProfile != IMS_NULL)
    {
        if (pProfile->IsSipFeatureProvisioned())
        {
            return pProfile->IsKeepAliveConfigured();
        }
    }

    const SipConfig* pSipConfig = ConfigurationManager::GetInstance()->GetSipConfig(nSlotId);

    return pSipConfig->IsKeepAliveConfigured();
}

PUBLIC GLOBAL IMS_BOOL SipConfigProxy::IsMultipleRegConfigured(IN IMS_SINT32 nSlotId)
{
    const SipConfig* pSipConfig = ConfigurationManager::GetInstance()->GetSipConfig(nSlotId);
    return pSipConfig->IsMultipleRegConfigured();
}

PUBLIC GLOBAL IMS_BOOL SipConfigProxy::IsRegIdParameterConfigured(IN IMS_SINT32 nSlotId)
{
    const SipConfig* pSipConfig = ConfigurationManager::GetInstance()->GetSipConfig(nSlotId);
    return pSipConfig->IsRegIdParameterConfigured();
}

PUBLIC GLOBAL IMS_BOOL SipConfigProxy::IsNoAcceptContactHeaderInBye(
        IN IMS_SINT32 nSlotId, IN const SipProfile* pProfile /* = IMS_NULL*/)
{
    if (pProfile != IMS_NULL)
    {
        if (pProfile->IsSipFeatureProvisioned())
        {
            return pProfile->IsNoAcceptContactHeaderInBye();
        }
    }

    const SipConfig* pSipConfig = ConfigurationManager::GetInstance()->GetSipConfig(nSlotId);

    return pSipConfig->IsNoAcceptContactHeaderInBYE();
}

PUBLIC GLOBAL IMS_BOOL SipConfigProxy::IsPanInfoInInitialRegRequired(
        IN IMS_SINT32 nSlotId, IN const SipProfile* pProfile /* = IMS_NULL*/)
{
    if (pProfile != IMS_NULL)
    {
        if (pProfile->IsSipFeatureProvisioned())
        {
            return pProfile->IsPanInfoInInitialRegRequired();
        }
    }

    const SipConfig* pSipConfig = ConfigurationManager::GetInstance()->GetSipConfig(nSlotId);

    return pSipConfig->IsPaniHeaderInInitialRegRequired();
}

PUBLIC GLOBAL IMS_BOOL SipConfigProxy::IsPPreferredIdInRegSubRequired(
        IN IMS_SINT32 nSlotId, IN const SipProfile* pProfile /* = IMS_NULL*/)
{
    if (pProfile != IMS_NULL)
    {
        if (pProfile->IsSipFeatureProvisioned())
        {
            return pProfile->IsPPreferredIdInRegSubRequired();
        }
    }

    const SipConfig* pSipConfig = ConfigurationManager::GetInstance()->GetSipConfig(nSlotId);

    return pSipConfig->IsPPreferredIdInRegSubRequired();
}

PUBLIC GLOBAL IMS_BOOL SipConfigProxy::IsRouteHeaderInRegRequired(
        IN IMS_SINT32 nSlotId, IN const SipProfile* pProfile /* = IMS_NULL*/)
{
    if (pProfile != IMS_NULL)
    {
        if (pProfile->IsSipFeatureProvisioned())
        {
            return pProfile->IsRouteHeaderInRegRequired();
        }
    }

    const SipConfig* pSipConfig = ConfigurationManager::GetInstance()->GetSipConfig(nSlotId);

    return pSipConfig->IsRouteHeaderInRegRequired();
}

PUBLIC GLOBAL IMS_BOOL SipConfigProxy::IsRportConfigured(
        IN IMS_SINT32 nSlotId, IN const SipProfile* pProfile /* = IMS_NULL*/)
{
    if (pProfile != IMS_NULL)
    {
        if (pProfile->IsSipFeatureProvisioned())
        {
            return pProfile->IsRportConfigured();
        }
    }

    const SipConfig* pSipConfig = ConfigurationManager::GetInstance()->GetSipConfig(nSlotId);

    return pSipConfig->IsRportConfigured();
}

PUBLIC GLOBAL IMS_BOOL SipConfigProxy::IsTransportErrorReportOnTxnRequired(
        IN IMS_SINT32 nSlotId, IN const SipProfile* pProfile /* = IMS_NULL*/)
{
    if (pProfile != IMS_NULL)
    {
        if (pProfile->IsSipFeatureProvisioned())
        {
            return pProfile->IsTransportErrorReportOnTxnRequired();
        }
    }

    const SipConfig* pSipConfig = ConfigurationManager::GetInstance()->GetSipConfig(nSlotId);

    return pSipConfig->IsTransportErrorReportOnTxnRequired();
}

PUBLIC GLOBAL IMS_BOOL SipConfigProxy::IsTrustDomainConfigured(
        IN IMS_SINT32 nSlotId, IN const SipProfile* pProfile /* = IMS_NULL*/)
{
    if (pProfile != IMS_NULL)
    {
        if (pProfile->IsSipFeatureProvisioned())
        {
            return pProfile->IsTrustDomainConfigured();
        }
    }

    const SipConfig* pSipConfig = ConfigurationManager::GetInstance()->GetSipConfig(nSlotId);

    return pSipConfig->IsTrustDomainConfigured();
}

PUBLIC GLOBAL IMS_BOOL SipConfigProxy::IsUdpFallbackConfigured(
        IN IMS_SINT32 nSlotId, IN const SipProfile* pProfile /* = IMS_NULL*/)
{
    if (pProfile != IMS_NULL)
    {
        if (pProfile->IsSipFeatureProvisioned())
        {
            return pProfile->IsUdpFallbackConfigured();
        }
    }

    const SipConfig* pSipConfig = ConfigurationManager::GetInstance()->GetSipConfig(nSlotId);

    return pSipConfig->IsUdpFallbackConfigured();
}

PUBLIC GLOBAL IMS_BOOL SipConfigProxy::IsUserAgentConfigured(
        IN IMS_SINT32 nSlotId, IN const SipProfile* pProfile /* = IMS_NULL*/)
{
    if (pProfile != IMS_NULL)
    {
        if (pProfile->IsSipFeatureProvisioned())
        {
            return pProfile->IsUserAgentConfigured();
        }
    }

    const SipConfig* pSipConfig = ConfigurationManager::GetInstance()->GetSipConfig(nSlotId);

    return pSipConfig->IsUserAgentConfigured();
}

PUBLIC GLOBAL IMS_BOOL SipConfigProxy::IsUserAgentSetByContext(
        IN IMS_SINT32 nSlotId, IN const SipProfile* pProfile /* = IMS_NULL*/)
{
    if (pProfile != IMS_NULL)
    {
        if (pProfile->IsSipFeatureProvisioned())
        {
            return pProfile->IsUserAgentSetByContext();
        }
    }

    const SipConfig* pSipConfig = ConfigurationManager::GetInstance()->GetSipConfig(nSlotId);

    return pSipConfig->IsUserAgentSetByContext();
}

PUBLIC GLOBAL IMS_BOOL SipConfigProxy::IsSdpNegotiationRequiredForNonRpr(
        IN IMS_SINT32 nSlotId, IN const SipProfile* pProfile /* = IMS_NULL*/)
{
    if (pProfile != IMS_NULL)
    {
        if (pProfile->IsSipFeatureProvisioned())
        {
            return pProfile->IsSdpNegotiationRequiredForNonRpr();
        }
    }

    const SipConfig* pSipConfig = ConfigurationManager::GetInstance()->GetSipConfig(nSlotId);

    return pSipConfig->IsSdpNegotiationRequiredForNonRpr();
}

PUBLIC GLOBAL IMS_BOOL SipConfigProxy::IsRequestUriValidationRequiredInMidDialog(
        IN IMS_SINT32 nSlotId, IN const SipProfile* pProfile /* = IMS_NULL*/)
{
    if (pProfile != IMS_NULL)
    {
        if (pProfile->IsSipFeatureProvisioned())
        {
            return pProfile->IsRequestUriValidationRequiredInMidDialog();
        }
    }

    const SipConfig* pSipConfig = ConfigurationManager::GetInstance()->GetSipConfig(nSlotId);

    return pSipConfig->IsRequestUriValidationRequiredInMidDialog();
}

PUBLIC GLOBAL IMS_BOOL SipConfigProxy::IsSessionTimerUpdateRequiredByReInvite(
        IN IMS_SINT32 nSlotId, IN const SipProfile* pProfile /* = IMS_NULL*/)
{
    if (pProfile != IMS_NULL)
    {
        if (pProfile->IsSipFeatureProvisioned())
        {
            return pProfile->IsSessionTimerUpdateRequiredByReInvite();
        }
    }

    const SipConfig* pSipConfig = ConfigurationManager::GetInstance()->GetSipConfig(nSlotId);

    return pSipConfig->IsSessionTimerUpdateRequiredByReInvite();
}

PUBLIC GLOBAL IMS_BOOL SipConfigProxy::IsSipInstanceParamRequiredInContactForNonRegisterRequest(
        IN IMS_SINT32 nSlotId, IN const SipProfile* pProfile /* = IMS_NULL*/)
{
    if (pProfile != IMS_NULL)
    {
        if (pProfile->IsSipFeatureProvisioned())
        {
            return pProfile->IsSipInstanceParamRequiredInContactForNonRegisterRequest();
        }
    }

    const SipConfig* pSipConfig = ConfigurationManager::GetInstance()->GetSipConfig(nSlotId);

    return pSipConfig->IsSipInstanceParamRequiredInContactForNonRegisterRequest();
}

PUBLIC GLOBAL IMS_BOOL SipConfigProxy::IsSessionIdHeaderSupported(
        IN IMS_SINT32 nSlotId, IN const SipProfile* pProfile /* = IMS_NULL*/)
{
    if (pProfile != IMS_NULL)
    {
        if (pProfile->IsSipFeatureProvisioned())
        {
            return pProfile->IsSessionIdHeaderSupported();
        }
    }

    const SipConfig* pSipConfig = ConfigurationManager::GetInstance()->GetSipConfig(nSlotId);

    return pSipConfig->IsSessionIdHeaderSupported();
}

PUBLIC GLOBAL IMS_SINT32 SipConfigProxy::GetHideMacInPaniHeaderPolicy(
        IN IMS_SINT32 nSlotId, IN const SipProfile* pProfile /* = IMS_NULL*/)
{
    if (pProfile != IMS_NULL)
    {
        if (pProfile->IsSipFeatureProvisioned())
        {
            return pProfile->GetHideMacInPaniHeaderPolicy();
        }
    }

    const SipConfig* pSipConfig = ConfigurationManager::GetInstance()->GetSipConfig(nSlotId);
    return pSipConfig->GetHideMacInPaniHeaderPolicy();
}

PUBLIC GLOBAL IMS_BOOL SipConfigProxy::IsLocalTimezoneParameterSupportedInPaniHeader(
        IN IMS_SINT32 nSlotId, IN const SipProfile* pProfile /* = IMS_NULL*/)
{
    if (pProfile != IMS_NULL)
    {
        if (pProfile->IsSipFeatureProvisioned())
        {
            return pProfile->IsLocalTimezoneParameterSupportedInPaniHeader();
        }
    }

    const SipConfig* pSipConfig = ConfigurationManager::GetInstance()->GetSipConfig(nSlotId);

    return pSipConfig->IsLocalTimezoneParameterSupportedInPaniHeader();
}
// SIP_FEATURES }

// SIP_TIMERS {
PUBLIC GLOBAL IMS_SINT32 SipConfigProxy::GetTimerValue100Trying(IN IMS_SINT32 nSlotId)
{
    const SipConfig* pSipConfig = ConfigurationManager::GetInstance()->GetSipConfig(nSlotId);

    return pSipConfig->GetTimerValue100Trying();
}

PUBLIC GLOBAL IMS_SINT32 SipConfigProxy::GetTimerValueT1(IN IMS_SINT32 nSlotId)
{
    const SipConfig* pSipConfig = ConfigurationManager::GetInstance()->GetSipConfig(nSlotId);

    return (pSipConfig != IMS_NULL) ? pSipConfig->GetTimerValueT1() : 2000;
}

PUBLIC GLOBAL IMS_SINT32 SipConfigProxy::GetTimerValueT2(IN IMS_SINT32 nSlotId)
{
    const SipConfig* pSipConfig = ConfigurationManager::GetInstance()->GetSipConfig(nSlotId);

    return (pSipConfig != IMS_NULL) ? pSipConfig->GetTimerValueT2() : 16000;
}

PUBLIC GLOBAL IMS_SINT32 SipConfigProxy::GetTimerValueT1(IN IMS_SINT32 nSlotId,
        IN const SipProfile* pProfile, IN const ISipConfigV* piSipConfigV /* = IMS_NULL*/)
{
    if (pProfile != IMS_NULL)
    {
        const SipTimerValues& objTv = pProfile->GetTimerValues();

        if (objTv.IsSet(SipTimerValues::TIMER_T1))
        {
            return objTv.GetValue(SipTimerValues::TIMER_T1);
        }
    }

    const SipConfigV* pSipConfigV = DYNAMIC_CAST(const SipConfigV*, piSipConfigV);

    if (pSipConfigV != IMS_NULL)
    {
        if (pSipConfigV->GetTimerValueT1() > 0)
        {
            return pSipConfigV->GetTimerValueT1();
        }
    }

    return GetTimerValueT1(nSlotId);
}

PUBLIC GLOBAL IMS_SINT32 SipConfigProxy::GetTimerValueT2(IN IMS_SINT32 nSlotId,
        IN const SipProfile* pProfile, IN const ISipConfigV* piSipConfigV /* = IMS_NULL*/)
{
    if (pProfile != IMS_NULL)
    {
        const SipTimerValues& objTv = pProfile->GetTimerValues();

        if (objTv.IsSet(SipTimerValues::TIMER_T2))
        {
            return objTv.GetValue(SipTimerValues::TIMER_T2);
        }
    }

    const SipConfigV* pSipConfigV = DYNAMIC_CAST(const SipConfigV*, piSipConfigV);

    if (pSipConfigV != IMS_NULL)
    {
        if (pSipConfigV->GetTimerValueT2() > 0)
        {
            return pSipConfigV->GetTimerValueT2();
        }
    }

    return GetTimerValueT2(nSlotId);
}

PUBLIC GLOBAL IMS_SINT32 SipConfigProxy::GetTimerValueT4(IN IMS_SINT32 nSlotId,
        IN const SipProfile* pProfile, IN const ISipConfigV* piSipConfigV,
        IN IMS_BOOL bDefaultRequired /* = IMS_TRUE*/)
{
    if (pProfile != IMS_NULL)
    {
        const SipTimerValues& objTv = pProfile->GetTimerValues();

        if (objTv.IsSet(SipTimerValues::TIMER_T2))
        {
            // T4 = T2 + 1s
            return objTv.GetValue(SipTimerValues::TIMER_T2) + 1000;
        }
    }

    const SipConfigV* pSipConfigV = DYNAMIC_CAST(const SipConfigV*, piSipConfigV);

    if (pSipConfigV != IMS_NULL)
    {
        if (pSipConfigV->GetTimerValueT4() > 0)
        {
            return pSipConfigV->GetTimerValueT4();
        }
    }

    // T4 = T2 + 1s
    return bDefaultRequired ? (GetTimerValueT2(nSlotId, pProfile, piSipConfigV) + 1000) : (-1);
}

PUBLIC GLOBAL IMS_SINT32 SipConfigProxy::GetTimerValueA(IN IMS_SINT32 nSlotId,
        IN const SipProfile* pProfile, IN const ISipConfigV* piSipConfigV,
        IN IMS_BOOL bDefaultRequired /* = IMS_TRUE*/)
{
    if (pProfile != IMS_NULL)
    {
        const SipTimerValues& objTv = pProfile->GetTimerValues();

        if (objTv.IsSet(SipTimerValues::TIMER_T1))
        {
            return objTv.GetValue(SipTimerValues::TIMER_T1);
        }
    }

    const SipConfigV* pSipConfigV = DYNAMIC_CAST(const SipConfigV*, piSipConfigV);

    if (pSipConfigV != IMS_NULL)
    {
        if (pSipConfigV->GetTimerValueA() > 0)
        {
            return pSipConfigV->GetTimerValueA();
        }
    }

    return bDefaultRequired ? GetTimerValueT1(nSlotId, pProfile, piSipConfigV) : (-1);
}

PUBLIC GLOBAL IMS_SINT32 SipConfigProxy::GetTimerValueB(IN IMS_SINT32 nSlotId,
        IN const SipProfile* pProfile, IN const ISipConfigV* piSipConfigV,
        IN IMS_BOOL bDefaultRequired /* = IMS_TRUE*/)
{
    if (pProfile != IMS_NULL)
    {
        const SipTimerValues& objTv = pProfile->GetTimerValues();

        if (objTv.IsSet(SipTimerValues::TIMER_B))
        {
            return objTv.GetValue(SipTimerValues::TIMER_B);
        }
    }

    const SipConfigV* pSipConfigV = DYNAMIC_CAST(const SipConfigV*, piSipConfigV);

    if (pSipConfigV != IMS_NULL)
    {
        if (pSipConfigV->GetTimerValueB() > 0)
        {
            return pSipConfigV->GetTimerValueB();
        }
    }

    return bDefaultRequired ? (GetTimerValueT1(nSlotId, pProfile, piSipConfigV) * 64) : (-1);
}

PUBLIC GLOBAL IMS_SINT32 SipConfigProxy::GetTimerValueD(IN IMS_SINT32 nSlotId,
        IN const SipProfile* pProfile, IN const ISipConfigV* piSipConfigV,
        IN IMS_BOOL bDefaultRequired /* = IMS_TRUE*/)
{
    if (pProfile != IMS_NULL)
    {
        const SipTimerValues& objTv = pProfile->GetTimerValues();

        if (objTv.IsSet(SipTimerValues::TIMER_D))
        {
            return objTv.GetValue(SipTimerValues::TIMER_D);
        }
    }

    const SipConfigV* pSipConfigV = DYNAMIC_CAST(const SipConfigV*, piSipConfigV);

    if (pSipConfigV != IMS_NULL)
    {
        if (pSipConfigV->GetTimerValueD() > 0)
        {
            return pSipConfigV->GetTimerValueD();
        }
    }

    return bDefaultRequired ? (GetTimerValueT1(nSlotId, pProfile, piSipConfigV) * 64) : (-1);
}

PUBLIC GLOBAL IMS_SINT32 SipConfigProxy::GetTimerValueE(IN IMS_SINT32 nSlotId,
        IN const SipProfile* pProfile, IN const ISipConfigV* piSipConfigV,
        IN IMS_BOOL bDefaultRequired /* = IMS_TRUE*/)
{
    if (pProfile != IMS_NULL)
    {
        const SipTimerValues& objTv = pProfile->GetTimerValues();

        if (objTv.IsSet(SipTimerValues::TIMER_T1))
        {
            return objTv.GetValue(SipTimerValues::TIMER_T1);
        }
    }

    const SipConfigV* pSipConfigV = DYNAMIC_CAST(const SipConfigV*, piSipConfigV);

    if (pSipConfigV != IMS_NULL)
    {
        if (pSipConfigV->GetTimerValueE() > 0)
        {
            return pSipConfigV->GetTimerValueE();
        }
    }

    return bDefaultRequired ? GetTimerValueT1(nSlotId, pProfile, piSipConfigV) : (-1);
}

PUBLIC GLOBAL IMS_SINT32 SipConfigProxy::GetTimerValueF(IN IMS_SINT32 nSlotId,
        IN const SipProfile* pProfile, IN const ISipConfigV* piSipConfigV,
        IN IMS_BOOL bDefaultRequired /* = IMS_TRUE*/)
{
    if (pProfile != IMS_NULL)
    {
        const SipTimerValues& objTv = pProfile->GetTimerValues();

        if (objTv.IsSet(SipTimerValues::TIMER_F))
        {
            return objTv.GetValue(SipTimerValues::TIMER_F);
        }
    }

    const SipConfigV* pSipConfigV = DYNAMIC_CAST(const SipConfigV*, piSipConfigV);

    if (pSipConfigV != IMS_NULL)
    {
        if (pSipConfigV->GetTimerValueF() > 0)
        {
            return pSipConfigV->GetTimerValueF();
        }
    }

    return bDefaultRequired ? (GetTimerValueT1(nSlotId, pProfile, piSipConfigV) * 64) : (-1);
}

PUBLIC GLOBAL IMS_SINT32 SipConfigProxy::GetTimerValueG(IN IMS_SINT32 nSlotId,
        IN const SipProfile* pProfile, IN const ISipConfigV* piSipConfigV,
        IN IMS_BOOL bDefaultRequired /* = IMS_TRUE*/)
{
    if (pProfile != IMS_NULL)
    {
        const SipTimerValues& objTv = pProfile->GetTimerValues();

        if (objTv.IsSet(SipTimerValues::TIMER_T1))
        {
            return objTv.GetValue(SipTimerValues::TIMER_T1);
        }
    }

    const SipConfigV* pSipConfigV = DYNAMIC_CAST(const SipConfigV*, piSipConfigV);

    if (pSipConfigV != IMS_NULL)
    {
        if (pSipConfigV->GetTimerValueG() > 0)
        {
            return pSipConfigV->GetTimerValueG();
        }
    }

    return bDefaultRequired ? GetTimerValueT1(nSlotId, pProfile, piSipConfigV) : (-1);
}

PUBLIC GLOBAL IMS_SINT32 SipConfigProxy::GetTimerValueH(IN IMS_SINT32 nSlotId,
        IN const SipProfile* pProfile, IN const ISipConfigV* piSipConfigV,
        IN IMS_BOOL bDefaultRequired /* = IMS_TRUE*/)
{
    if (pProfile != IMS_NULL)
    {
        const SipTimerValues& objTv = pProfile->GetTimerValues();

        if (objTv.IsSet(SipTimerValues::TIMER_H))
        {
            return objTv.GetValue(SipTimerValues::TIMER_H);
        }
    }

    const SipConfigV* pSipConfigV = DYNAMIC_CAST(const SipConfigV*, piSipConfigV);

    if (pSipConfigV != IMS_NULL)
    {
        if (pSipConfigV->GetTimerValueH() > 0)
        {
            return pSipConfigV->GetTimerValueH();
        }
    }

    return bDefaultRequired ? (GetTimerValueT1(nSlotId, pProfile, piSipConfigV) * 64) : (-1);
}

PUBLIC GLOBAL IMS_SINT32 SipConfigProxy::GetTimerValueI(IN IMS_SINT32 nSlotId,
        IN const SipProfile* pProfile, IN const ISipConfigV* piSipConfigV,
        IN IMS_BOOL bDefaultRequired /* = IMS_TRUE*/)
{
    if (pProfile != IMS_NULL)
    {
        const SipTimerValues& objTv = pProfile->GetTimerValues();

        if (objTv.IsSet(SipTimerValues::TIMER_I))
        {
            return objTv.GetValue(SipTimerValues::TIMER_I);
        }
    }

    const SipConfigV* pSipConfigV = DYNAMIC_CAST(const SipConfigV*, piSipConfigV);

    if (pSipConfigV != IMS_NULL)
    {
        if (pSipConfigV->GetTimerValueI() > 0)
        {
            return pSipConfigV->GetTimerValueI();
        }
    }

    // T4
    return bDefaultRequired ? (GetTimerValueT2(nSlotId, pProfile, piSipConfigV) + 1000) : (-1);
}

PUBLIC GLOBAL IMS_SINT32 SipConfigProxy::GetTimerValueJ(IN IMS_SINT32 nSlotId,
        IN const SipProfile* pProfile, IN const ISipConfigV* piSipConfigV,
        IN IMS_BOOL bDefaultRequired /* = IMS_TRUE*/)
{
    if (pProfile != IMS_NULL)
    {
        const SipTimerValues& objTv = pProfile->GetTimerValues();

        if (objTv.IsSet(SipTimerValues::TIMER_J))
        {
            return objTv.GetValue(SipTimerValues::TIMER_J);
        }
    }

    const SipConfigV* pSipConfigV = DYNAMIC_CAST(const SipConfigV*, piSipConfigV);

    if (pSipConfigV != IMS_NULL)
    {
        if (pSipConfigV->GetTimerValueJ() > 0)
        {
            return pSipConfigV->GetTimerValueJ();
        }
    }

    return bDefaultRequired ? (GetTimerValueT1(nSlotId, pProfile, piSipConfigV) * 64) : (-1);
}

PUBLIC GLOBAL IMS_SINT32 SipConfigProxy::GetTimerValueK(IN IMS_SINT32 nSlotId,
        IN const SipProfile* pProfile, IN const ISipConfigV* piSipConfigV,
        IN IMS_BOOL bDefaultRequired /* = IMS_TRUE*/)
{
    if (pProfile != IMS_NULL)
    {
        const SipTimerValues& objTv = pProfile->GetTimerValues();

        if (objTv.IsSet(SipTimerValues::TIMER_K))
        {
            return objTv.GetValue(SipTimerValues::TIMER_K);
        }
    }

    const SipConfigV* pSipConfigV = DYNAMIC_CAST(const SipConfigV*, piSipConfigV);

    if (pSipConfigV != IMS_NULL)
    {
        if (pSipConfigV->GetTimerValueK() > 0)
        {
            return pSipConfigV->GetTimerValueK();
        }
    }

    // T4
    return bDefaultRequired ? (GetTimerValueT2(nSlotId, pProfile, piSipConfigV) + 1000) : (-1);
}
// SIP_TIMERS }
