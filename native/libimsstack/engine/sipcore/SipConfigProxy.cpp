/*
    Author
    <table>
    date      author                    description
    --------  --------------            ----------
    20150405  hwangoo.park@             Created
    </table>

    Description

*/
#include "ServiceMemory.h"
#include "private/ConfigurationManager.h"
#include "private/SipConfig.h"
#include "SipConfigProxy.h"



PUBLIC GLOBAL
IMS_SINT32 SIPConfigProxy::GetDeviceId(IN IMS_SINT32 nSlotId,
        IN CONST SIPProfile *pProfile/* = IMS_NULL*/)
{
    if (pProfile != IMS_NULL)
    {
        IMS_SINT32 nDeviceId = pProfile->GetDeviceId();

        if (nDeviceId != SIPProfile::NOT_PROVISIONED)
        {
            return nDeviceId;
        }
    }

    const SipConfig *pSipConfig = ConfigurationManager::GetInstance()->GetSipConfig(nSlotId);

    return pSipConfig->GetDeviceId();
}

PUBLIC GLOBAL
const AString& SIPConfigProxy::GetPredefinedDeviceId(IN IMS_SINT32 nSlotId,
        IN CONST SIPProfile *pProfile/* = IMS_NULL*/)
{
    if (pProfile != IMS_NULL)
    {
        const AString &strPredefinedDeviceId = pProfile->GetPredefinedDeviceId();

        if (strPredefinedDeviceId.GetLength() > 0)
        {
            return strPredefinedDeviceId;
        }
    }

    const SipConfig *pSipConfig = ConfigurationManager::GetInstance()->GetSipConfig(nSlotId);

    return pSipConfig->GetPredefinedDeviceId();
}

PUBLIC GLOBAL
IMS_SINT32 SIPConfigProxy::GetPort(IN IMS_SINT32 nSlotId,
        IN CONST SIPProfile *pProfile/* = IMS_NULL*/)
{
    if (pProfile != IMS_NULL)
    {
        IMS_SINT32 nPort = pProfile->GetPort();

        if (nPort != SIPProfile::NOT_PROVISIONED)
        {
            return nPort;
        }
    }

    const SipConfig *pSipConfig = ConfigurationManager::GetInstance()->GetSipConfig(nSlotId);

    return pSipConfig->GetPort();
}

PUBLIC GLOBAL
IMS_UINT32 SIPConfigProxy::GetSipFeatureCaps(IN IMS_SINT32 nSlotId,
        IN CONST SIPProfile *pProfile/* = IMS_NULL*/)
{
    if (pProfile != IMS_NULL)
    {
        IMS_SINT32 nSIPFeatures = pProfile->GetSipFeatureCaps();

        if (nSIPFeatures != SIPProfile::NOT_PROVISIONED)
        {
            return nSIPFeatures;
        }
    }

    const SipConfig *pSipConfig = ConfigurationManager::GetInstance()->GetSipConfig(nSlotId);

    return pSipConfig->GetSipFeatureCaps();
}

PUBLIC GLOBAL
const AString& SIPConfigProxy::GetTagPrefix(IN IMS_SINT32 nSlotId,
        IN CONST SIPProfile *pProfile/* = IMS_NULL*/)
{
    if (pProfile != IMS_NULL)
    {
        const AString &strTagPrefix = pProfile->GetTagPrefix();

        if (!strTagPrefix.IsNULL())
        {
            return strTagPrefix;
        }
    }

    const SipConfig *pSipConfig = ConfigurationManager::GetInstance()->GetSipConfig(nSlotId);

    return pSipConfig->GetTagPrefix();
}

PUBLIC GLOBAL
IMS_SINT32 SIPConfigProxy::GetTcpCriterionLength(IN IMS_SINT32 nSlotId,
        IN CONST SIPProfile* pProfile/* = IMS_NULL*/)
{
    if (pProfile != IMS_NULL)
    {
        IMS_SINT32 nTcpCriterionLength = pProfile->GetTcpCriterionLength();

        if (nTcpCriterionLength != SIPProfile::NOT_PROVISIONED)
        {
            return nTcpCriterionLength;
        }
    }

    const SipConfig* pSipConfig = ConfigurationManager::GetInstance()->GetSipConfig(nSlotId);

    return pSipConfig->GetTcpCriterionLength();
}

PUBLIC GLOBAL
IMS_SINT32 SIPConfigProxy::GetTransportType(IN IMS_SINT32 nSlotId,
        IN const SIPProfile* /*pProfile = IMS_NULL*/)
{
    const SipConfig* pSipConfig = ConfigurationManager::GetInstance()->GetSipConfig(nSlotId);

    return pSipConfig->GetTransportType();
}

PUBLIC GLOBAL
AString SIPConfigProxy::GetUaString(IN IMS_SINT32 nSlotId,
        IN CONST SIPProfile *pProfile/* = IMS_NULL*/)
{
    if (pProfile != IMS_NULL)
    {
        const AString &strUAString = pProfile->GetUaString();

        if (!strUAString.IsNULL())
        {
            return strUAString;
        }
    }

    const SipConfig *pSipConfig = ConfigurationManager::GetInstance()->GetSipConfig(nSlotId);

    return pSipConfig->GetUaVersion();
}

// "reg"
PUBLIC GLOBAL
const AStringArray& SIPConfigProxy::GetRegAllowMethods(IN IMS_SINT32 nSlotId,
        IN CONST SIPProfile *pProfile/* = IMS_NULL*/)
{
    if (pProfile != IMS_NULL)
    {
        const AStringArray &objAllowMethods = pProfile->GetRegAllowMethods();

        if (!objAllowMethods.IsEmpty())
        {
            return objAllowMethods;
        }
    }

    const SipConfig *pSipConfig = ConfigurationManager::GetInstance()->GetSipConfig(nSlotId);

    return pSipConfig->GetRegAllowMethods();
}

PUBLIC GLOBAL
IMS_SINT32 SIPConfigProxy::GetRegExpires(IN IMS_SINT32 nSlotId,
        IN CONST SIPProfile *pProfile/* = IMS_NULL*/)
{
    if (pProfile != IMS_NULL)
    {
        IMS_SINT32 nRegExpires = pProfile->GetRegExpires();

        if (nRegExpires != SIPProfile::NOT_PROVISIONED)
        {
            return nRegExpires;
        }
    }

    const SipConfig *pSipConfig = ConfigurationManager::GetInstance()->GetSipConfig(nSlotId);

    return pSipConfig->GetRegExpiration();
}

PUBLIC GLOBAL
IMS_SINT32 SIPConfigProxy::GetRegSubExpires(IN IMS_SINT32 nSlotId,
        IN CONST SIPProfile *pProfile/* = IMS_NULL*/)
{
    if (pProfile != IMS_NULL)
    {
        IMS_SINT32 nRegSubExpires = pProfile->GetRegSubExpires();

        if (nRegSubExpires != SIPProfile::NOT_PROVISIONED)
        {
            return nRegSubExpires;
        }
    }

    const SipConfig *pSipConfig = ConfigurationManager::GetInstance()->GetSipConfig(nSlotId);

    return pSipConfig->GetRegSubExpiration();
}

PUBLIC GLOBAL
AString SIPConfigProxy::GetRegUaString(IN IMS_SINT32 nSlotId,
        IN CONST SIPProfile* pProfile/* = IMS_NULL*/)
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

PUBLIC GLOBAL
IMS_BOOL SIPConfigProxy::IsRegExpiresConfigured(IN IMS_SINT32 nSlotId,
        IN CONST SIPProfile *pProfile/* = IMS_NULL*/)
{
    if (pProfile != IMS_NULL)
    {
        if (pProfile->GetRegExpires() != SIPProfile::NOT_PROVISIONED)
        {
            return IMS_TRUE;
        }
    }

    const SipConfig *pSipConfig = ConfigurationManager::GetInstance()->GetSipConfig(nSlotId);

    return pSipConfig->IsRegExpirationConfigured();
}

PUBLIC GLOBAL
IMS_BOOL SIPConfigProxy::IsRegSubExpiresConfigured(IN IMS_SINT32 nSlotId,
        IN CONST SIPProfile *pProfile/* = IMS_NULL*/)
{
    if (pProfile != IMS_NULL)
    {
        if (pProfile->GetRegSubExpires() != SIPProfile::NOT_PROVISIONED)
        {
            return IMS_TRUE;
        }
    }

    const SipConfig *pSipConfig = ConfigurationManager::GetInstance()->GetSipConfig(nSlotId);

    return pSipConfig->IsRegSubExpirationConfigured();
}

PUBLIC GLOBAL
IMS_BOOL SIPConfigProxy::IsRegSubscriptionConfigured(IN IMS_SINT32 nSlotId,
        IN CONST SIPProfile *pProfile/* = IMS_NULL*/)
{
    if (pProfile != IMS_NULL)
    {
        IMS_SINT32 nRegSubscription = pProfile->GetRegSubscription();

        if (nRegSubscription != SIPProfile::NOT_PROVISIONED)
        {
            return (nRegSubscription != 0);
        }
    }

    const SipConfig *pSipConfig = ConfigurationManager::GetInstance()->GetSipConfig(nSlotId);

    return pSipConfig->IsRegSubscriptionConfigured();
}

PUBLIC GLOBAL
const ISipConfigV* SIPConfigProxy::GetSipConfigV(IN IMS_SINT32 nSlotId)
{
    const SipConfig* pSipConfig = ConfigurationManager::GetInstance()->GetSipConfig(nSlotId);

    return (pSipConfig != IMS_NULL) ? pSipConfig->GetSipConfigV() : IMS_NULL;
}

// SIP_FEATURES {
// FIXME: If the SIP features in SIPProfile is zero, how to handle it?
// Do we need to check the default configuration??
PUBLIC GLOBAL
IMS_BOOL SIPConfigProxy::IsAuthenticationAlgorithmRequired(IN IMS_SINT32 nSlotId,
        IN CONST SIPProfile *pProfile/* = IMS_NULL*/)
{
    if (pProfile != IMS_NULL)
    {
        if (pProfile->IsSIPFeatureProvisioned())
        {
            return pProfile->IsAuthenticationAlgorithmRequired();
        }
    }

    const SipConfig *pSipConfig = ConfigurationManager::GetInstance()->GetSipConfig(nSlotId);

    return pSipConfig->IsAuthenticationAlgorithmRequired();
}

PUBLIC GLOBAL
IMS_BOOL SIPConfigProxy::IsCellularNetworkInfoHeaderRequired(IN IMS_SINT32 nSlotId,
        IN CONST SIPProfile *pProfile/* = IMS_NULL*/)
{
    if (pProfile != IMS_NULL)
    {
        if (pProfile->IsSIPFeatureProvisioned())
        {
            return pProfile->IsCellularNetworkInfoHeaderRequired();
        }
    }

    const SipConfig *pSipConfig = ConfigurationManager::GetInstance()->GetSipConfig(nSlotId);
    return pSipConfig->HasFeature(ISipConfig::SIP_FEATURE_CAPS_CELLULAR_NETWORK_INFO_HEADER);
}

PUBLIC GLOBAL
IMS_BOOL SIPConfigProxy::IsCompactFormConfigured(IN IMS_SINT32 nSlotId,
        IN CONST SIPProfile */*pProfile = IMS_NULL*/)
{
    const SipConfig *pSipConfig = ConfigurationManager::GetInstance()->GetSipConfig(nSlotId);

    if (pSipConfig != IMS_NULL)
    {
        return pSipConfig->IsCompactFormConfigured();
    }

    return IMS_FALSE;
}

PUBLIC GLOBAL
IMS_BOOL SIPConfigProxy::IsContactInAll1xxRequired(IN IMS_SINT32 nSlotId,
        IN CONST SIPProfile *pProfile/* = IMS_NULL*/)
{
    if (pProfile != IMS_NULL)
    {
        if (pProfile->IsSIPFeatureProvisioned())
        {
            return pProfile->IsContactInAll1xxRequired();
        }
    }

    const SipConfig *pSipConfig = ConfigurationManager::GetInstance()->GetSipConfig(nSlotId);

    return pSipConfig->IsContactInAll1xxRequired();
}

PUBLIC GLOBAL
IMS_BOOL SIPConfigProxy::IsCountryInfoRequiredInPANIHeader(IN IMS_SINT32 nSlotId,
        IN CONST SIPProfile *pProfile/* = IMS_NULL*/)
{
    if (pProfile != IMS_NULL)
    {
        if (pProfile->IsSIPFeatureProvisioned())
        {
            return pProfile->IsCountryInfoRequiredInPANIHeader();
        }
    }

    const SipConfig *pSipConfig = ConfigurationManager::GetInstance()->GetSipConfig(nSlotId);
    return pSipConfig->HasFeature(ISipConfig::SIP_FEATURE_CAPS_COUNTRY_INFO_IN_PANI_HEADER);
}

PUBLIC GLOBAL
IMS_BOOL SIPConfigProxy::IsDisplayNameDQUOTRequired(IN IMS_SINT32 nSlotId,
        IN CONST SIPProfile *pProfile/* = IMS_NULL*/)
{
    if (pProfile != IMS_NULL)
    {
        if (pProfile->IsSIPFeatureProvisioned())
        {
            return pProfile->IsDisplayNameDQUOTRequired();
        }
    }

    const SipConfig *pSipConfig = ConfigurationManager::GetInstance()->GetSipConfig(nSlotId);

    if (pSipConfig != IMS_NULL)
    {
        return pSipConfig->IsDisplayNameDQUOTRequired();
    }

    return IMS_FALSE;
}

PUBLIC GLOBAL
IMS_BOOL SIPConfigProxy::IsExpiresHeaderInRegRequired(IN IMS_SINT32 nSlotId,
        IN CONST SIPProfile *pProfile/* = IMS_NULL*/)
{
    if (pProfile != IMS_NULL)
    {
        if (pProfile->IsSIPFeatureProvisioned())
        {
            return pProfile->IsExpiresHeaderInRegRequired();
        }
    }

    const SipConfig *pSipConfig = ConfigurationManager::GetInstance()->GetSipConfig(nSlotId);

    return pSipConfig->IsExpiresHeaderInRegRequired();
}

PUBLIC GLOBAL
IMS_BOOL SIPConfigProxy::IsIpSecConfigured(IN IMS_SINT32 nSlotId,
        IN const SIPProfile* pProfile/* = IMS_NULL*/)
{
    if (pProfile != IMS_NULL)
    {
        if (pProfile->IsSIPFeatureProvisioned())
        {
            return pProfile->IsIpSecConfigured();
        }
    }

    const SipConfig* pSipConfig = ConfigurationManager::GetInstance()->GetSipConfig(nSlotId);

    return pSipConfig->IsIpSecConfigured();
}

PUBLIC GLOBAL
IMS_BOOL SIPConfigProxy::IsGRUUConfigured(IN IMS_SINT32 nSlotId,
        IN CONST SIPProfile *pProfile/* = IMS_NULL*/)
{
    if (pProfile != IMS_NULL)
    {
        if (pProfile->IsSIPFeatureProvisioned())
        {
            return pProfile->IsGRUUConfigured();
        }
    }

    const SipConfig *pSipConfig = ConfigurationManager::GetInstance()->GetSipConfig(nSlotId);

    return pSipConfig->IsGruuConfigured();
}

PUBLIC GLOBAL
IMS_BOOL SIPConfigProxy::IsKeepAliveConfigured(IN IMS_SINT32 nSlotId,
        IN CONST SIPProfile *pProfile/* = IMS_NULL*/)
{
    if (pProfile != IMS_NULL)
    {
        if (pProfile->IsSIPFeatureProvisioned())
        {
            return pProfile->IsKeepAliveConfigured();
        }
    }

    const SipConfig *pSipConfig = ConfigurationManager::GetInstance()->GetSipConfig(nSlotId);

    return pSipConfig->IsKeepAliveConfigured();
}

PUBLIC GLOBAL
IMS_BOOL SIPConfigProxy::IsMultipleRegConfigured(IN IMS_SINT32 nSlotId,
        IN CONST SIPProfile *pProfile/* = IMS_NULL*/)
{
    if (pProfile != IMS_NULL)
    {
        if (pProfile->IsSIPFeatureProvisioned())
        {
            return pProfile->IsMultipleRegConfigured();
        }
    }

    const SipConfig *pSipConfig = ConfigurationManager::GetInstance()->GetSipConfig(nSlotId);

    return pSipConfig->IsMultipleRegConfigured();
}

PUBLIC GLOBAL
IMS_BOOL SIPConfigProxy::IsNoAcceptContactHeaderInBYE(IN IMS_SINT32 nSlotId,
        IN CONST SIPProfile *pProfile/* = IMS_NULL*/)
{
    if (pProfile != IMS_NULL)
    {
        if (pProfile->IsSIPFeatureProvisioned())
        {
            return pProfile->IsNoAcceptContactHeaderInBYE();
        }
    }

    const SipConfig *pSipConfig = ConfigurationManager::GetInstance()->GetSipConfig(nSlotId);

    return pSipConfig->IsNoAcceptContactHeaderInBYE();
}

PUBLIC GLOBAL
IMS_BOOL SIPConfigProxy::IsPANInfoInInitialRegRequired(IN IMS_SINT32 nSlotId,
        IN CONST SIPProfile *pProfile/* = IMS_NULL*/)
{
    if (pProfile != IMS_NULL)
    {
        if (pProfile->IsSIPFeatureProvisioned())
        {
            return pProfile->IsPANInfoInInitialRegRequired();
        }
    }

    const SipConfig *pSipConfig = ConfigurationManager::GetInstance()->GetSipConfig(nSlotId);

    return pSipConfig->IsPANInfoInInitialRegRequired();
}

PUBLIC GLOBAL
IMS_BOOL SIPConfigProxy::IsPPreferredIdInRegSubRequired(IN IMS_SINT32 nSlotId,
        IN CONST SIPProfile *pProfile/* = IMS_NULL*/)
{
    if (pProfile != IMS_NULL)
    {
        if (pProfile->IsSIPFeatureProvisioned())
        {
            return pProfile->IsPPreferredIdInRegSubRequired();
        }
    }

    const SipConfig *pSipConfig = ConfigurationManager::GetInstance()->GetSipConfig(nSlotId);

    return pSipConfig->IsPPreferredIdInRegSubRequired();
}

PUBLIC GLOBAL
IMS_BOOL SIPConfigProxy::IsRouteHeaderInRegRequired(IN IMS_SINT32 nSlotId,
        IN CONST SIPProfile *pProfile/* = IMS_NULL*/)
{
    if (pProfile != IMS_NULL)
    {
        if (pProfile->IsSIPFeatureProvisioned())
        {
            return pProfile->IsRouteHeaderInRegRequired();
        }
    }

    const SipConfig *pSipConfig = ConfigurationManager::GetInstance()->GetSipConfig(nSlotId);

    return pSipConfig->IsRouteHeaderInRegRequired();
}

PUBLIC GLOBAL
IMS_BOOL SIPConfigProxy::IsRportConfigured(IN IMS_SINT32 nSlotId,
        IN CONST SIPProfile *pProfile/* = IMS_NULL*/)
{
    if (pProfile != IMS_NULL)
    {
        if (pProfile->IsSIPFeatureProvisioned())
        {
            return pProfile->IsRportConfigured();
        }
    }

    const SipConfig *pSipConfig = ConfigurationManager::GetInstance()->GetSipConfig(nSlotId);

    return pSipConfig->IsRportConfigured();
}

PUBLIC GLOBAL
IMS_BOOL SIPConfigProxy::IsTransportErrorReportOnTxnRequired(IN IMS_SINT32 nSlotId,
        IN CONST SIPProfile *pProfile/* = IMS_NULL*/)
{
    if (pProfile != IMS_NULL)
    {
        if (pProfile->IsSIPFeatureProvisioned())
        {
            return pProfile->IsTransportErrorReportOnTxnRequired();
        }
    }

    const SipConfig *pSipConfig = ConfigurationManager::GetInstance()->GetSipConfig(nSlotId);

    return pSipConfig->IsTransportErrorReportOnTxnRequired();
}

PUBLIC GLOBAL
IMS_BOOL SIPConfigProxy::IsTrustDomainConfigured(IN IMS_SINT32 nSlotId,
        IN CONST SIPProfile *pProfile/* = IMS_NULL*/)
{
    if (pProfile != IMS_NULL)
    {
        if (pProfile->IsSIPFeatureProvisioned())
        {
            return pProfile->IsTrustDomainConfigured();
        }
    }

    const SipConfig *pSipConfig = ConfigurationManager::GetInstance()->GetSipConfig(nSlotId);

    return pSipConfig->IsTrustDomainConfigured();
}

PUBLIC GLOBAL
IMS_BOOL SIPConfigProxy::IsUdpFallbackConfigured(IN IMS_SINT32 nSlotId,
        IN CONST SIPProfile *pProfile/* = IMS_NULL*/)
{
    if (pProfile != IMS_NULL)
    {
        if (pProfile->IsSIPFeatureProvisioned())
        {
            return pProfile->IsUdpFallbackConfigured();
        }
    }

    const SipConfig *pSipConfig = ConfigurationManager::GetInstance()->GetSipConfig(nSlotId);

    return pSipConfig->IsUdpFallbackConfigured();
}

PUBLIC GLOBAL
IMS_BOOL SIPConfigProxy::IsUserAgentConfigured(IN IMS_SINT32 nSlotId,
        IN CONST SIPProfile *pProfile/* = IMS_NULL*/)
{
    if (pProfile != IMS_NULL)
    {
        if (pProfile->IsSIPFeatureProvisioned())
        {
            return pProfile->IsUserAgentConfigured();
        }
    }

    const SipConfig *pSipConfig = ConfigurationManager::GetInstance()->GetSipConfig(nSlotId);

    return pSipConfig->IsUserAgentConfigured();
}

PUBLIC GLOBAL
IMS_BOOL SIPConfigProxy::IsUserAgentSetByContext(IN IMS_SINT32 nSlotId,
        IN CONST SIPProfile *pProfile/* = IMS_NULL*/)
{
    if (pProfile != IMS_NULL)
    {
        if (pProfile->IsSIPFeatureProvisioned())
        {
            return pProfile->IsUserAgentSetByContext();
        }
    }

    const SipConfig *pSipConfig = ConfigurationManager::GetInstance()->GetSipConfig(nSlotId);

    return pSipConfig->IsUserAgentSetByContext();
}

PUBLIC GLOBAL
IMS_BOOL SIPConfigProxy::IsSdpNegotiationRequiredForNonRpr(IN IMS_SINT32 nSlotId,
        IN const SIPProfile* pProfile/* = IMS_NULL*/)
{
    if (pProfile != IMS_NULL)
    {
        if (pProfile->IsSIPFeatureProvisioned())
        {
            return pProfile->IsSdpNegotiationRequiredForNonRpr();
        }
    }

    const SipConfig* pSipConfig = ConfigurationManager::GetInstance()->GetSipConfig(nSlotId);

    return pSipConfig->IsSdpNegotiationRequiredForNonRpr();
}

PUBLIC GLOBAL
IMS_BOOL SIPConfigProxy::IsRequestUriValidationRequiredInMidDialog(IN IMS_SINT32 nSlotId,
        IN const SIPProfile* pProfile/* = IMS_NULL*/)
{
    if (pProfile != IMS_NULL)
    {
        if (pProfile->IsSIPFeatureProvisioned())
        {
            return pProfile->IsRequestUriValidationRequiredInMidDialog();
        }
    }

    const SipConfig* pSipConfig = ConfigurationManager::GetInstance()->GetSipConfig(nSlotId);

    return pSipConfig->IsRequestUriValidationRequiredInMidDialog();
}

PUBLIC GLOBAL
IMS_BOOL SIPConfigProxy::IsSessionTimerUpdateRequiredByReInvite(
        IN IMS_SINT32 nSlotId, IN const SIPProfile* pProfile/* = IMS_NULL*/)
{
    if (pProfile != IMS_NULL)
    {
        if (pProfile->IsSIPFeatureProvisioned())
        {
            return pProfile->IsSessionTimerUpdateRequiredByReInvite();
        }
    }

    const SipConfig* pSipConfig = ConfigurationManager::GetInstance()->GetSipConfig(nSlotId);

    return pSipConfig->IsSessionTimerUpdateRequiredByReInvite();
}

PUBLIC GLOBAL
IMS_BOOL SIPConfigProxy::IsSipInstanceParamRequiredInContactForNonRegisterRequest(
        IN IMS_SINT32 nSlotId, IN const SIPProfile* pProfile/* = IMS_NULL*/)
{
    if (pProfile != IMS_NULL)
    {
        if (pProfile->IsSIPFeatureProvisioned())
        {
            return pProfile->IsSipInstanceParamRequiredInContactForNonRegisterRequest();
        }
    }

    const SipConfig* pSipConfig = ConfigurationManager::GetInstance()->GetSipConfig(nSlotId);

    return pSipConfig->IsSipInstanceParamRequiredInContactForNonRegisterRequest();
}

PUBLIC GLOBAL
IMS_BOOL SIPConfigProxy::IsSessionIdHeaderSupported(IN IMS_SINT32 nSlotId,
        IN const SIPProfile* pProfile/* = IMS_NULL*/)
{
    if (pProfile != IMS_NULL)
    {
        if (pProfile->IsSIPFeatureProvisioned())
        {
            return pProfile->IsSessionIdHeaderSupported();
        }
    }

    const SipConfig* pSipConfig = ConfigurationManager::GetInstance()->GetSipConfig(nSlotId);

    return pSipConfig->IsSessionIdHeaderSupported();
}

PUBLIC GLOBAL
IMS_BOOL SIPConfigProxy::IsInvalidMacAddressRequiredInPaniHeader(IN IMS_SINT32 nSlotId,
        IN const SIPProfile* pProfile/* = IMS_NULL*/)
{
    if (pProfile != IMS_NULL)
    {
        if (pProfile->IsSIPFeatureProvisioned())
        {
            return pProfile->IsInvalidMacAddressRequiredInPaniHeader();
        }
    }

    const SipConfig* pSipConfig = ConfigurationManager::GetInstance()->GetSipConfig(nSlotId);

    return pSipConfig->IsInvalidMacAddressRequiredInPaniHeader();
}

PUBLIC GLOBAL
IMS_BOOL SIPConfigProxy::IsLocalTimeZoneRequiredInPaniHeader(IN IMS_SINT32 nSlotId,
        IN const SIPProfile* pProfile/* = IMS_NULL*/)
{
    if (pProfile != IMS_NULL)
    {
        if (pProfile->IsSIPFeatureProvisioned())
        {
            return pProfile->IsLocalTimeZoneRequiredInPaniHeader();
        }
    }

    const SipConfig* pSipConfig = ConfigurationManager::GetInstance()->GetSipConfig(nSlotId);

    return pSipConfig->IsLocalTimeZoneRequiredInPaniHeader();
}
// SIP_FEATURES }

// SIP_TIMERS {
PUBLIC GLOBAL
IMS_SINT32 SIPConfigProxy::GetTimerValue100Trying(IN IMS_SINT32 nSlotId)
{
    const SipConfig *pSipConfig = ConfigurationManager::GetInstance()->GetSipConfig(nSlotId);

    return pSipConfig->GetTimerValue100Trying();
}

PUBLIC GLOBAL
IMS_SINT32 SIPConfigProxy::GetTimerValueT1(IN IMS_SINT32 nSlotId)
{
    const SipConfig *pSipConfig = ConfigurationManager::GetInstance()->GetSipConfig(nSlotId);

    return (pSipConfig != IMS_NULL) ? pSipConfig->GetTimerValueT1() : 2000;
}

PUBLIC GLOBAL
IMS_SINT32 SIPConfigProxy::GetTimerValueT2(IN IMS_SINT32 nSlotId)
{
    const SipConfig *pSipConfig = ConfigurationManager::GetInstance()->GetSipConfig(nSlotId);

    return (pSipConfig != IMS_NULL) ? pSipConfig->GetTimerValueT2() : 16000;
}

PUBLIC GLOBAL
IMS_SINT32 SIPConfigProxy::GetTimerValueT1(IN IMS_SINT32 nSlotId, IN CONST SIPProfile *pProfile,
        IN CONST ISipConfigV *piSipConfigV/* = IMS_NULL*/)
{
    if (pProfile != IMS_NULL)
    {
        const SIPTimerValues& objTVs = pProfile->GetTimerValues();

        if (objTVs.IsSet(SIPTimerValues::TIMER_T1))
        {
            return objTVs.GetValue(SIPTimerValues::TIMER_T1);
        }
    }

    const SipConfigV *pSipConfigV = DYNAMIC_CAST(const SipConfigV*, piSipConfigV);

    if (pSipConfigV != IMS_NULL)
    {
        if (pSipConfigV->GetTimerValueT1() > 0)
        {
            return pSipConfigV->GetTimerValueT1();
        }
    }

    return GetTimerValueT1(nSlotId);
}

PUBLIC GLOBAL
IMS_SINT32 SIPConfigProxy::GetTimerValueT2(IN IMS_SINT32 nSlotId, IN CONST SIPProfile *pProfile,
        IN CONST ISipConfigV *piSipConfigV/* = IMS_NULL*/)
{
    if (pProfile != IMS_NULL)
    {
        const SIPTimerValues& objTVs = pProfile->GetTimerValues();

        if (objTVs.IsSet(SIPTimerValues::TIMER_T2))
        {
            return objTVs.GetValue(SIPTimerValues::TIMER_T2);
        }
    }

    const SipConfigV *pSipConfigV = DYNAMIC_CAST(const SipConfigV*, piSipConfigV);

    if (pSipConfigV != IMS_NULL)
    {
        if (pSipConfigV->GetTimerValueT2() > 0)
        {
            return pSipConfigV->GetTimerValueT2();
        }
    }

    return GetTimerValueT2(nSlotId);
}

PUBLIC GLOBAL
IMS_SINT32 SIPConfigProxy::GetTimerValueT4(IN IMS_SINT32 nSlotId, IN CONST SIPProfile *pProfile,
        IN CONST ISipConfigV *piSipConfigV, IN IMS_BOOL bDefaultRequired/* = IMS_TRUE*/)
{
    if (pProfile != IMS_NULL)
    {
        const SIPTimerValues& objTVs = pProfile->GetTimerValues();

        if (objTVs.IsSet(SIPTimerValues::TIMER_T2))
        {
            // T4 = T2 + 1s
            return objTVs.GetValue(SIPTimerValues::TIMER_T2) + 1000;
        }
    }

    const SipConfigV *pSipConfigV = DYNAMIC_CAST(const SipConfigV*, piSipConfigV);

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

PUBLIC GLOBAL
IMS_SINT32 SIPConfigProxy::GetTimerValueTA(IN IMS_SINT32 nSlotId, IN CONST SIPProfile *pProfile,
        IN CONST ISipConfigV *piSipConfigV, IN IMS_BOOL bDefaultRequired/* = IMS_TRUE*/)
{
    if (pProfile != IMS_NULL)
    {
        const SIPTimerValues& objTVs = pProfile->GetTimerValues();

        if (objTVs.IsSet(SIPTimerValues::TIMER_T1))
        {
            return objTVs.GetValue(SIPTimerValues::TIMER_T1);
        }
    }

    const SipConfigV *pSipConfigV = DYNAMIC_CAST(const SipConfigV*, piSipConfigV);

    if (pSipConfigV != IMS_NULL)
    {
        if (pSipConfigV->GetTimerValueTA() > 0)
        {
            return pSipConfigV->GetTimerValueTA();
        }
    }

    return bDefaultRequired ? GetTimerValueT1(nSlotId, pProfile, piSipConfigV) : (-1);
}

PUBLIC GLOBAL
IMS_SINT32 SIPConfigProxy::GetTimerValueTB(IN IMS_SINT32 nSlotId, IN CONST SIPProfile *pProfile,
        IN CONST ISipConfigV *piSipConfigV, IN IMS_BOOL bDefaultRequired/* = IMS_TRUE*/)
{
    if (pProfile != IMS_NULL)
    {
        const SIPTimerValues& objTVs = pProfile->GetTimerValues();

        if (objTVs.IsSet(SIPTimerValues::TV_TIMER_B))
        {
            return objTVs.GetValue(SIPTimerValues::TV_TIMER_B);
        }
    }

    const SipConfigV *pSipConfigV = DYNAMIC_CAST(const SipConfigV*, piSipConfigV);

    if (pSipConfigV != IMS_NULL)
    {
        if (pSipConfigV->GetTimerValueTB() > 0)
        {
            return pSipConfigV->GetTimerValueTB();
        }
    }

    return bDefaultRequired ? (GetTimerValueT1(nSlotId, pProfile, piSipConfigV) * 64) : (-1);
}

PUBLIC GLOBAL
IMS_SINT32 SIPConfigProxy::GetTimerValueTD(IN IMS_SINT32 nSlotId, IN CONST SIPProfile *pProfile,
        IN CONST ISipConfigV *piSipConfigV, IN IMS_BOOL bDefaultRequired/* = IMS_TRUE*/)
{
    if (pProfile != IMS_NULL)
    {
        const SIPTimerValues& objTVs = pProfile->GetTimerValues();

        if (objTVs.IsSet(SIPTimerValues::TV_TIMER_D))
        {
            return objTVs.GetValue(SIPTimerValues::TV_TIMER_D);
        }
    }

    const SipConfigV *pSipConfigV = DYNAMIC_CAST(const SipConfigV*, piSipConfigV);

    if (pSipConfigV != IMS_NULL)
    {
        if (pSipConfigV->GetTimerValueTD() > 0)
        {
            return pSipConfigV->GetTimerValueTD();
        }
    }

    return bDefaultRequired ? (GetTimerValueT1(nSlotId, pProfile, piSipConfigV) * 64) : (-1);
}

PUBLIC GLOBAL
IMS_SINT32 SIPConfigProxy::GetTimerValueTE(IN IMS_SINT32 nSlotId, IN CONST SIPProfile *pProfile,
        IN CONST ISipConfigV *piSipConfigV, IN IMS_BOOL bDefaultRequired/* = IMS_TRUE*/)
{
    if (pProfile != IMS_NULL)
    {
        const SIPTimerValues& objTVs = pProfile->GetTimerValues();

        if (objTVs.IsSet(SIPTimerValues::TIMER_T1))
        {
            return objTVs.GetValue(SIPTimerValues::TIMER_T1);
        }
    }

    const SipConfigV *pSipConfigV = DYNAMIC_CAST(const SipConfigV*, piSipConfigV);

    if (pSipConfigV != IMS_NULL)
    {
        if (pSipConfigV->GetTimerValueTE() > 0)
        {
            return pSipConfigV->GetTimerValueTE();
        }
    }

    return bDefaultRequired ? GetTimerValueT1(nSlotId, pProfile, piSipConfigV) : (-1);
}

PUBLIC GLOBAL
IMS_SINT32 SIPConfigProxy::GetTimerValueTF(IN IMS_SINT32 nSlotId, IN CONST SIPProfile *pProfile,
        IN CONST ISipConfigV *piSipConfigV, IN IMS_BOOL bDefaultRequired/* = IMS_TRUE*/)
{
    if (pProfile != IMS_NULL)
    {
        const SIPTimerValues& objTVs = pProfile->GetTimerValues();

        if (objTVs.IsSet(SIPTimerValues::TV_TIMER_F))
        {
            return objTVs.GetValue(SIPTimerValues::TV_TIMER_F);
        }
    }

    const SipConfigV *pSipConfigV = DYNAMIC_CAST(const SipConfigV*, piSipConfigV);

    if (pSipConfigV != IMS_NULL)
    {
        if (pSipConfigV->GetTimerValueTF() > 0)
        {
            return pSipConfigV->GetTimerValueTF();
        }
    }

    return bDefaultRequired ? (GetTimerValueT1(nSlotId, pProfile, piSipConfigV) * 64) : (-1);
}

PUBLIC GLOBAL
IMS_SINT32 SIPConfigProxy::GetTimerValueTG(IN IMS_SINT32 nSlotId, IN CONST SIPProfile *pProfile,
        IN CONST ISipConfigV *piSipConfigV, IN IMS_BOOL bDefaultRequired/* = IMS_TRUE*/)
{
    if (pProfile != IMS_NULL)
    {
        const SIPTimerValues& objTVs = pProfile->GetTimerValues();

        if (objTVs.IsSet(SIPTimerValues::TIMER_T1))
        {
            return objTVs.GetValue(SIPTimerValues::TIMER_T1);
        }
    }

    const SipConfigV *pSipConfigV = DYNAMIC_CAST(const SipConfigV*, piSipConfigV);

    if (pSipConfigV != IMS_NULL)
    {
        if (pSipConfigV->GetTimerValueTG() > 0)
        {
            return pSipConfigV->GetTimerValueTG();
        }
    }

    return bDefaultRequired ? GetTimerValueT1(nSlotId, pProfile, piSipConfigV) : (-1);
}

PUBLIC GLOBAL
IMS_SINT32 SIPConfigProxy::GetTimerValueTH(IN IMS_SINT32 nSlotId, IN CONST SIPProfile *pProfile,
        IN CONST ISipConfigV *piSipConfigV, IN IMS_BOOL bDefaultRequired/* = IMS_TRUE*/)
{
    if (pProfile != IMS_NULL)
    {
        const SIPTimerValues& objTVs = pProfile->GetTimerValues();

        if (objTVs.IsSet(SIPTimerValues::TV_TIMER_H))
        {
            return objTVs.GetValue(SIPTimerValues::TV_TIMER_H);
        }
    }

    const SipConfigV *pSipConfigV = DYNAMIC_CAST(const SipConfigV*, piSipConfigV);

    if (pSipConfigV != IMS_NULL)
    {
        if (pSipConfigV->GetTimerValueTH() > 0)
        {
            return pSipConfigV->GetTimerValueTH();
        }
    }

    return bDefaultRequired ? (GetTimerValueT1(nSlotId, pProfile, piSipConfigV) * 64) : (-1);
}

PUBLIC GLOBAL
IMS_SINT32 SIPConfigProxy::GetTimerValueTI(IN IMS_SINT32 nSlotId, IN CONST SIPProfile *pProfile,
        IN CONST ISipConfigV *piSipConfigV, IN IMS_BOOL bDefaultRequired/* = IMS_TRUE*/)
{
    if (pProfile != IMS_NULL)
    {
        const SIPTimerValues& objTVs = pProfile->GetTimerValues();

        if (objTVs.IsSet(SIPTimerValues::TV_TIMER_I))
        {
            return objTVs.GetValue(SIPTimerValues::TV_TIMER_I);
        }
    }

    const SipConfigV *pSipConfigV = DYNAMIC_CAST(const SipConfigV*, piSipConfigV);

    if (pSipConfigV != IMS_NULL)
    {
        if (pSipConfigV->GetTimerValueTI() > 0)
        {
            return pSipConfigV->GetTimerValueTI();
        }
    }

    // T4
    return bDefaultRequired ? (GetTimerValueT2(nSlotId, pProfile, piSipConfigV) + 1000) : (-1);
}

PUBLIC GLOBAL
IMS_SINT32 SIPConfigProxy::GetTimerValueTJ(IN IMS_SINT32 nSlotId, IN CONST SIPProfile *pProfile,
        IN CONST ISipConfigV *piSipConfigV, IN IMS_BOOL bDefaultRequired/* = IMS_TRUE*/)
{
    if (pProfile != IMS_NULL)
    {
        const SIPTimerValues& objTVs = pProfile->GetTimerValues();

        if (objTVs.IsSet(SIPTimerValues::TV_TIMER_J))
        {
            return objTVs.GetValue(SIPTimerValues::TV_TIMER_J);
        }
    }

    const SipConfigV *pSipConfigV = DYNAMIC_CAST(const SipConfigV*, piSipConfigV);

    if (pSipConfigV != IMS_NULL)
    {
        if (pSipConfigV->GetTimerValueTJ() > 0)
        {
            return pSipConfigV->GetTimerValueTJ();
        }
    }

    return bDefaultRequired ? (GetTimerValueT1(nSlotId, pProfile, piSipConfigV) * 64) : (-1);
}

PUBLIC GLOBAL
IMS_SINT32 SIPConfigProxy::GetTimerValueTK(IN IMS_SINT32 nSlotId, IN CONST SIPProfile *pProfile,
        IN CONST ISipConfigV *piSipConfigV, IN IMS_BOOL bDefaultRequired/* = IMS_TRUE*/)
{
    if (pProfile != IMS_NULL)
    {
        const SIPTimerValues& objTVs = pProfile->GetTimerValues();

        if (objTVs.IsSet(SIPTimerValues::TV_TIMER_K))
        {
            return objTVs.GetValue(SIPTimerValues::TV_TIMER_K);
        }
    }

    const SipConfigV *pSipConfigV = DYNAMIC_CAST(const SipConfigV*, piSipConfigV);

    if (pSipConfigV != IMS_NULL)
    {
        if (pSipConfigV->GetTimerValueTK() > 0)
        {
            return pSipConfigV->GetTimerValueTK();
        }
    }

    // T4
    return bDefaultRequired ? (GetTimerValueT2(nSlotId, pProfile, piSipConfigV) + 1000) : (-1);
}
// SIP_TIMERS }
