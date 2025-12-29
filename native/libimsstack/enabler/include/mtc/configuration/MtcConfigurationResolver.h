/*
 * Copyright (C) 2024 The Android Open Source Project
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

#ifndef MTC_CONFIGURATION_RESOLVER_H_
#define MTC_CONFIGURATION_RESOLVER_H_

#include "AString.h"
#include "CallReasonInfo.h"
#include "CarrierConfig.h"
#include "ConfigDef.h"
#include "IImsAosInfo.h"
#include "INetworkWatcher.h"
#include "ImsList.h"
#include "ImsTypeDef.h"
#include "ImsVector.h"
#include "TextParser.h"

class MtcConfigurationProxy;

class MtcConfigurationResolver
{
public:
    MtcConfigurationResolver() = delete;
    ~MtcConfigurationResolver() = delete;
    MtcConfigurationResolver(IN const MtcConfigurationResolver&) = delete;
    MtcConfigurationResolver& operator=(IN const MtcConfigurationResolver&) = delete;

    inline static IMS_SINT32 GetGeolocationLevel(IN const MtcConfigurationProxy& objProxy,
            IN IMS_SINT32 eGeolocationPidfAllowedType, IN const AString& strRemoteNumber)
    {
        // CDR-WiFi-1320 / CDR-WiFi-1460
        if ((eGeolocationPidfAllowedType ==
                            CarrierConfig::Ims::GEOLOCATION_PIDF_FOR_NON_EMERGENCY_ON_WIFI ||
                    eGeolocationPidfAllowedType ==
                            CarrierConfig::Ims::
                                    GEOLOCATION_PIDF_FOR_NORMAL_ROUTING_EMERGENCY_ON_WIFI) &&
                objProxy.Contains(
                        ConfigWfc::KEY_PIDF_SHORT_CODE_STRING_ARRAY, strRemoteNumber.GetStr()))
        {
            return ConfigVoice::GEOLOCATION_PIDF_INFO_COUNTRY_CODE_AND_STATE;
        }
        return objProxy.GetIntFromArray(
                ConfigIms::KEY_INFORMATION_LEVEL_OF_GEOLOCATION_PIDF_INT_ARRAY,
                eGeolocationPidfAllowedType - 1);
    }

    inline static AString GetTerminateReasonHeader(
            IN const MtcConfigurationProxy& objProxy, IN TerminateType eType)
    {
        switch (eType)
        {
            case TerminateType::USER_ENDS_CALL:
                return objProxy.GetString(
                        ConfigVoice::KEY_CALL_TERMINATE_REASON_HEADER_USER_ENDS_CALL_STRING);
            case TerminateType::RTP_TIMEOUT:
                return objProxy.GetString(
                        ConfigVoice::KEY_CALL_TERMINATE_REASON_HEADER_RTP_TIMEOUT_STRING);
            case TerminateType::USER_ENDS_CALL_AND_RTP_TIMEOUT:
                return objProxy.GetString(ConfigVoice::
                                KEY_CALL_TERMINATE_REASON_HEADER_USER_ENDS_AND_RTP_TIMEOUT_STRING);
            case TerminateType::MEDIA_BEARER_LOSS:
                return objProxy.GetString(
                        ConfigVoice::KEY_CALL_TERMINATE_REASON_HEADER_MEDIA_BEARER_LOSS_STRING);
            case TerminateType::SIP_TIMEOUT:
                return objProxy.GetString(
                        ConfigVoice::KEY_CALL_TERMINATE_REASON_HEADER_SIP_TIMEOUT_STRING);
            case TerminateType::SIP_RESPONSE_TIMEOUT:
                return objProxy.GetString(
                        ConfigVoice::KEY_CALL_TERMINATE_REASON_HEADER_SIP_RESPONSE_TIMEOUT_STRING);
            case TerminateType::USER_ENDS_AND_SIP_RESPONSE_TIMEOUT:
                return objProxy.GetString(ConfigVoice::
                                KEY_CALL_TERMINATE_REASON_HEADER_USER_ENDS_AND_SIP_RESPONSE_TIMEOUT_STRING);
            case TerminateType::CALL_SETUP_TIMEOUT:
                return objProxy.GetString(
                        ConfigVoice::KEY_CALL_TERMINATE_REASON_HEADER_CALL_SETUP_TIMEOUT_STRING);
            case TerminateType::TERMINATING_EARLY_DIALOG:
                return objProxy.GetString(ConfigVoice::
                                KEY_CALL_TERMINATE_REASON_HEADER_TERMINATING_EARLYDIALOG_STRING);
            case TerminateType::SESSION_REFRESH_FAILURE:
                return objProxy.GetString(ConfigVoice::
                                KEY_CALL_TERMINATE_REASON_HEADER_SESSION_REFRESH_FAILURE_STRING);
            case TerminateType::CONFERENCE_CALL_JOINED:
                return objProxy.GetString(ConfigVoice::
                                KEY_CALL_TERMINATE_REASON_HEADER_CONFERENCE_CALL_JOINED_STRING);
            case TerminateType::NETWORK_LOST:
                return objProxy.GetString(
                        ConfigVoice::KEY_CALL_TERMINATE_REASON_HEADER_NETWORK_LOST_STRING);
            case TerminateType::MEDIA_NOT_SUPPORTED:
                return objProxy.GetString(
                        ConfigVoice::KEY_CALL_TERMINATE_REASON_HEADER_MEDIA_NOT_SUPPORTED_STRING);
            case TerminateType::MEDIA_BEARER_NOT_MET:
                return objProxy.GetString(
                        ConfigVoice::KEY_CALL_TERMINATE_REASON_HEADER_MEDIA_BEARER_NOT_MET_STRING);

            default:
                return AString::ConstNull();
        }
    }

    inline static AString GetRejectReasonPhrase(
            IN const MtcConfigurationProxy& objProxy, IN RejectType eType)
    {
        switch (eType)
        {
            case RejectType::ON_CS_CALL:
                return objProxy.GetString(
                        ConfigVoice::KEY_CALL_REJECT_REASON_PHRASE_ON_CSCALL_STRING);
            case RejectType::ON_VI_LTE_AND_NO_LTE:
                return objProxy.GetString(
                        ConfigVoice::KEY_CALL_REJECT_REASON_PHRASE_ON_VILTE_AND_NO_LTE_STRING);
            case RejectType::ON_CONNECTING_CALL:
                return objProxy.GetString(
                        ConfigVoice::KEY_CALL_REJECT_REASON_PHRASE_ON_CONNECTING_CALL_STRING);
            case RejectType::EXCEEDS_MAX_CALL:
                return objProxy.GetString(
                        ConfigVoice::KEY_CALL_REJECT_REASON_PHRASE_EXCEEDS_MAX_CALL_COUNT_STRING);
            case RejectType::ON_CONVERTING:
                return objProxy.GetString(
                        ConfigVoice::KEY_CALL_REJECT_REASON_PHRASE_ON_CONVERTING_STRING);
            case RejectType::NEGOTIATION_FAILURE:
                return objProxy.GetString(
                        ConfigVoice::KEY_CALL_REJECT_REASON_PHRASE_NEGOTIATION_FAILURE_STRING);
            case RejectType::NO_ANSWER_BY_USER:
                return objProxy.GetString(
                        ConfigVoice::KEY_CALL_REJECT_REASON_PHRASE_NO_ANSWER_BY_USER_STRING);
            case RejectType::VOWIFI_OFF:
                return objProxy.GetString(
                        ConfigVoice::KEY_CALL_REJECT_REASON_PHRASE_VOWIFI_OFF_STRING);
            case RejectType::USER_REJECT:
                return objProxy.GetString(
                        ConfigVoice::KEY_CALL_REJECT_REASON_PHRASE_USER_REJECT_STRING);
            case RejectType::ACCESS_CLASS_BLOCKED:
                return objProxy.GetString(
                        ConfigVoice::KEY_CALL_REJECT_REASON_PHRASE_ACCESS_CLASS_BLOCKED_STRING);
            case RejectType::VOPS_OFF:
                return objProxy.GetString(
                        ConfigVoice::KEY_CALL_REJECT_REASON_PHRASE_VOPS_OFF_STRING);
            default:
                return AString::ConstNull();
        }
    }

    inline static IMS_SINT32 GetRegistrationTo18xTimer(
            IN const MtcConfigurationProxy& objProxy, IN IMS_BOOL bWifi)
    {
        if (!bWifi)
        {
            return objProxy.GetIntFromArray(
                    ConfigVoice::KEY_REGISTRATION_TO_18X_TIMER_MILLIS_INT_ARRAY, 0);
        }
        else
        {
            return objProxy.GetIntFromArray(
                    ConfigVoice::KEY_REGISTRATION_TO_18X_TIMER_MILLIS_INT_ARRAY, 1);
        }
    }

    inline static IMS_SINT32 GetCallInitiationTo18xTimer(
            IN const MtcConfigurationProxy& objProxy, IN IMS_BOOL bWifi)
    {
        if (!bWifi)
        {
            return objProxy.GetIntFromArray(
                    ConfigVoice::KEY_CALL_INITIATION_TO_18X_TIMER_MILLIS_INT_ARRAY, 0);
        }
        else
        {
            return objProxy.GetIntFromArray(
                    ConfigVoice::KEY_CALL_INITIATION_TO_18X_TIMER_MILLIS_INT_ARRAY, 1);
        }
    }

    inline static ImsVector<IMS_SINT32> LookupActionForStatusCode(
            IN const MtcConfigurationProxy& objProxy, IN const IMS_CHAR* pszConfigName,
            IN IMS_SINT32 nStatusCode)
    {
        ImsVector<IMS_SINT32> objActions;
        AString strActions = ExtractConfigValue(objProxy, pszConfigName, nStatusCode);
        if (strActions.GetLength() > 0)
        {
            ImsList<AString> strActionArray = strActions.Split(TextParser::CHAR_COMMA);
            for (IMS_UINT32 j = 0; j < strActionArray.GetSize(); ++j)
            {
                objActions.Add(strActionArray.GetAt(j).ToInt32());
            }
        }
        return objActions;
    }

    inline static AString GetRequiredReasonTextForEmergencyTermination(
            IN const MtcConfigurationProxy& objProxy, IN IMS_SINT32 nStatusCode)
    {
        return ExtractConfigValue(objProxy,
                ConfigEmergency::
                        KEY_REJECT_CODE_AND_REASON_REQUIRE_IMMEDIATE_TERMINATION_STRING_ARRAY,
                nStatusCode);
    }

    inline static IMS_SINT32 LookupReasonCodeByStatusCodeForNormal(
            IN const MtcConfigurationProxy& objProxy, IN IMS_SINT32 nStatusCode)
    {
        AString strReasonCode = ExtractConfigValue(objProxy,
                ConfigVoice::KEY_REJECT_CODE_AND_REASON_CODE_SET_STRING_ARRAY, nStatusCode);
        if (strReasonCode.GetLength() > 0)
        {
            return strReasonCode.ToInt32();
        }
        return CODE_NONE;
    }

    inline static IMS_SINT32 LookupReasonCodeByStatusCodeForEmergency(
            IN const MtcConfigurationProxy& objProxy, IN IMS_SINT32 nStatusCode)
    {
        AString strReasonCode = ExtractConfigValue(objProxy,
                ConfigEmergency::KEY_REJECT_CODE_AND_REASON_CODE_SET_STRING_ARRAY, nStatusCode);
        if (strReasonCode.GetLength() > 0)
        {
            return strReasonCode.ToInt32();
        }
        return CODE_NONE;
    }

    inline static AString GetContactHeaderAddressInInviteForEmergency(
            IN const MtcConfigurationProxy& objProxy, IN IMS_UINT32 eAosRegMode,
            IN IMS_SINT32 eRoamingType)
    {
        IMS_UINT32 nBaseIndex;
        if (eRoamingType == INetworkWatcher::ROAMING_TYPE_NOT_ROAMING)
        {
            nBaseIndex = CONTACT_HEADER_ADDRESS_OFFSET_NON_ROAMING;
        }
        else if (eRoamingType == INetworkWatcher::ROAMING_TYPE_DOMESTIC)
        {
            nBaseIndex = CONTACT_HEADER_ADDRESS_OFFSET_DOMESTIC_ROAMING;
        }
        else
        {
            nBaseIndex = CONTACT_HEADER_ADDRESS_OFFSET_INTERNATIONAL_ROAMING;
        }

        ImsVector<AString> lstConfig = objProxy.GetStringArray(
                ConfigEmergency::KEY_CONTACT_HEADER_ADDRESS_IN_INVITE_STRING_ARRAY);
        switch (eAosRegMode)
        {
            case IImsAosInfo::REG_MODE_NORMAL:
                return lstConfig[nBaseIndex + REG_MODE_INDEX_NORMAL];
            case IImsAosInfo::REG_MODE_ADMIN:
                return lstConfig[nBaseIndex + REG_MODE_INDEX_ADMIN];
            case IImsAosInfo::REG_MODE_INTERNAL:
                return lstConfig[nBaseIndex + REG_MODE_INDEX_INTERNAL];
            case IImsAosInfo::REG_MODE_NOUICC:
                return lstConfig[nBaseIndex + REG_MODE_INDEX_NOUICC];
            default:
                return AString::ConstEmpty();
        }
    }

    inline static AString GetPPreferredIdentityHeaderInInviteForEmergency(
            IN const MtcConfigurationProxy& objProxy, IN IMS_UINT32 eAosRegMode)
    {
        ImsVector<AString> lstConfig = objProxy.GetStringArray(
                ConfigEmergency::KEY_P_PREFERRED_IDENTITY_INFO_HEADER_IN_INVITE_STRING_ARRAY);
        switch (eAosRegMode)
        {
            case IImsAosInfo::REG_MODE_NORMAL:
                return lstConfig[REG_MODE_INDEX_NORMAL];
            case IImsAosInfo::REG_MODE_ADMIN:
                return lstConfig[REG_MODE_INDEX_ADMIN];
            case IImsAosInfo::REG_MODE_INTERNAL:
                return lstConfig[REG_MODE_INDEX_INTERNAL];
            case IImsAosInfo::REG_MODE_NOUICC:
                return lstConfig[REG_MODE_INDEX_NOUICC];
            default:
                return AString::ConstEmpty();
        }
    }

    inline static IMS_BOOL IsCallHandoverAllowed(IN const MtcConfigurationProxy& objProxy,
            IN const AString& strSource, IN const AString& strTarget, IN const IMS_BOOL bEmergency,
            IN const IMS_BOOL bRoaming)
    {
        ImsVector<AString> lstRules =
                objProxy.GetStringArray(CarrierConfig::KEY_IWLAN_HANDOVER_POLICY_STRING_ARRAY);
        for (IMS_SINT32 i = static_cast<IMS_SINT32>(lstRules.GetSize()) - 1; i >= 0; i--)
        {
            ImsList<AString> lstRule = lstRules[i].Split(TextParser::CHAR_COMMA);
            AString strSourceRule;
            AString strTargetRule;
            AString strTypeRule;
            AString strCapabilitiesRule;
            IMS_BOOL bRoamingOnlyRule = IMS_FALSE;

            for (IMS_UINT32 j = 0; j < lstRule.GetSize(); j++)
            {
                AString strKey;
                AString strValue;
                lstRule.GetAt(j).SplitF(TextParser::CHAR_EQUAL, strKey, strValue);

                if (strKey.Equals("source"))
                {
                    strSourceRule = strValue;
                }
                else if (strKey.Equals("target"))
                {
                    strTargetRule = strValue;
                }
                else if (strKey.Equals("type"))
                {
                    strTypeRule = strValue;
                }
                else if (strKey.Equals("capabilities"))
                {
                    strCapabilitiesRule = strValue;
                }
                else if (strKey.Equals("roaming"))
                {
                    bRoamingOnlyRule = strValue.EqualsIgnoreCase("true");
                }
            }

            if ((bRoamingOnlyRule && !bRoaming) ||
                    (bEmergency && !strCapabilitiesRule.Contains("EIMS")) ||
                    (!bEmergency && !strCapabilitiesRule.Contains("IMS")) ||
                    !strSourceRule.Contains(strSource) || !strTargetRule.Contains(strTarget))
            {
                continue;  // Not matching, find other rules
            }

            if (strTypeRule.Equals("allowed"))
            {
                return IMS_TRUE;
            }
            else if (strTypeRule.Equals("disallowed"))
            {
                return IMS_FALSE;
            }
        }

        return IMS_FALSE;
    }

private:
    inline static AString ExtractConfigValue(IN const MtcConfigurationProxy& objProxy,
            IN const IMS_CHAR* pszConfigKey, IN IMS_SINT32 nKey)
    {
        ImsVector<AString> objKeyValueArray = objProxy.GetStringArray(pszConfigKey);
        for (IMS_UINT32 i = 0; i < objKeyValueArray.GetSize(); ++i)
        {
            AString strItem = objKeyValueArray.GetAt(i);
            AString strKey;
            AString strValue;
            strItem.SplitF(TextParser::CHAR_COLON, strKey, strValue);
            if (strKey.ToInt32() == nKey)
            {
                return strValue;
            }
        }
        return AString::ConstNull();
    }

    static const IMS_UINT32 CONTACT_HEADER_ADDRESS_OFFSET_NON_ROAMING = 0;
    static const IMS_UINT32 CONTACT_HEADER_ADDRESS_OFFSET_DOMESTIC_ROAMING = 4;
    static const IMS_UINT32 CONTACT_HEADER_ADDRESS_OFFSET_INTERNATIONAL_ROAMING = 8;
    static const IMS_UINT32 REG_MODE_INDEX_NORMAL = 0;
    static const IMS_UINT32 REG_MODE_INDEX_ADMIN = 1;
    static const IMS_UINT32 REG_MODE_INDEX_INTERNAL = 2;
    static const IMS_UINT32 REG_MODE_INDEX_NOUICC = 3;
};

#endif
