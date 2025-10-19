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
#include "CarrierConfig.h"
#include "IMtcContext.h"
#include "IMtcService.h"
#include "ImsAccessNetworkInfoType.h"
#include "ImsIdentity.h"
#include "ImsLib.h"
#include "ServiceNetwork.h"
#include "ServiceTrace.h"
#include "ServiceUtil.h"
#include "configuration/ConfigDef.h"
#include "configuration/MtcConfigurationProxy.h"
#include "dialingplan/ImsIdentityProxy.h"
#include "dialingplan/NormalDialingPlan.h"
#include "helper/IMtcAosConnector.h"
#include "util/TextParser.h"

__IMS_TRACE_TAG_COM_MTC__;

PUBLIC GLOBAL AString& NormalDialingPlan::GetTranslatedUri(IN IMtcContext& objContext,
        IN AString& strNumber, Scheme eScheme, IN const ImsIdentityProxy& objIdentityProxy)
{
    return Translate(objContext, strNumber, eScheme, objIdentityProxy);
}

PUBLIC GLOBAL AString& NormalDialingPlan::GetTranslatedUriForEmergencyTestNumber(
        IN IMtcContext& objContext, IN AString& strNumber,
        IN const ImsIdentityProxy& objIdentityProxy)
{
    if (strNumber.GetLength() == 0)
    {
        IMS_TRACE_E(0, "Number is empty", 0, 0, 0);
        return strNumber;
    }

    if (objContext.GetConfigurationProxy().GetBoolean(
            ConfigEmergency::KEY_EMERGENCY_EXCLUDE_URI_PARAMETERS_FOR_EMERGENCY_TEST_NUMBER_BOOL))
    {
        strNumber = objIdentityProxy.CreateSipUserId(strNumber, objContext.GetSlotId());
    }
    else
    {
        FormSipUri(objContext, strNumber, objIdentityProxy);
    }

    return strNumber;
}

PUBLIC GLOBAL AString& NormalDialingPlan::GetTranslatedUriForDialString(
        IN const IMtcContext& objContext, IN AString& strNumber,
        IN const ImsIdentityProxy& objIdentityProxy)
{
    strNumber = objIdentityProxy.CreateSipUserIdWithDialString(strNumber, objContext.GetSlotId());
    return strNumber;
}

PRIVATE GLOBAL AString& NormalDialingPlan::Translate(IN IMtcContext& objContext,
        IN_OUT AString& strNumber, IN Scheme eScheme, IN const ImsIdentityProxy& objIdentityProxy)
{
    if (strNumber.GetLength() == 0)
    {
        IMS_TRACE_E(0, "Number is empty", 0, 0, 0);
        return strNumber;
    }

    AString strLog;
    IMS_TRACE_D("Translate Dialed number :: %s",
            UtilService::GetLogString(strNumber, strLog, 3).GetStr(), 0, 0);

    // TODO: validation check logic. if strNumber is not a number format, error.
    if (IsNameAddress(strNumber))
    {
        return strNumber;
    }

    if (IsAddressSpec(strNumber))
    {
        AddAquotIfRequired(strNumber);
        return strNumber;
    }

    if (eScheme == Scheme::TEL ||
            (eScheme == Scheme::UNKNOWN && GetScheme(objContext) == Scheme::TEL))
    {
        FormTelUri(objContext, strNumber, objIdentityProxy);
    }
    else
    {
        FormSipUri(objContext, strNumber, objIdentityProxy);
    }

    IMS_TRACE_D("Translate Dialed URI :: %s",
            UtilService::GetLogString(strNumber, strLog, 7).GetStr(), 0, 0);
    return strNumber;
}

PRIVATE GLOBAL void NormalDialingPlan::FormSipUri(IN IMtcContext& objContext,
        IN_OUT AString& strNumber, IN const ImsIdentityProxy& objIdentityProxy)
{
    IMS_TRACE_I("FormSipUri", 0, 0, 0);

    NumberFormat eDialedNumberFormat = GetDialedNumberFormat(strNumber);

    if (eDialedNumberFormat == NumberFormat::GLOBAL_FORMAT)
    {
        // global
        strNumber = objIdentityProxy.CreateSipUserId(strNumber, objContext.GetSlotId(), IMS_TRUE);
    }
    else
    {
        // local
        AccessNetworkInfo objAni;
        AString strPhoneContext = objIdentityProxy.GetPhoneContext(
                ConvertDialingPolicy(GetLocalNumberPolicy(objContext)), objContext.GetSlotId(),
                &GetAccessNetworkInfo(objContext, objAni));
        strNumber = objIdentityProxy.CreateSipUserIdWithPhone(
                strNumber, objContext.GetSlotId(), strPhoneContext);
    }
}

PRIVATE GLOBAL void NormalDialingPlan::FormTelUri(IN IMtcContext& objContext,
        IN_OUT AString& strNumber, IN const ImsIdentityProxy& objIdentityProxy)
{
    IMS_TRACE_I("FormTelUri", 0, 0, 0);

    if (IsLocalNumberFormat(strNumber))
    {
        AccessNetworkInfo objAni;
        strNumber.Append(";phone-context=");
        strNumber.Append(objIdentityProxy.GetPhoneContext(
                ConvertDialingPolicy(GetLocalNumberPolicy(objContext)), objContext.GetSlotId(),
                &GetAccessNetworkInfo(objContext, objAni)));
    }

    // Adds the URI scheme
    strNumber.Prepend(TextParser::CHAR_COLON);
    strNumber.Prepend("tel");

    AddAquotIfRequired(strNumber);
}

PRIVATE GLOBAL IMS_BOOL NormalDialingPlan::IsVisualSeparator(IN IMS_CHAR ch)
{
    // "-", ".", "(", ")"

    if ((ch == '-') || (ch == '.') || (ch == '(') || (ch == ')'))
    {
        return IMS_TRUE;
    }

    return IMS_FALSE;
}

PRIVATE GLOBAL IMS_BOOL NormalDialingPlan::IsNameAddress(IN const AString& strNumber)
{
    if (strNumber.Contains(TextParser::CHAR_LAQUOT) && strNumber.Contains(TextParser::CHAR_RAQUOT))
    {
        return IMS_TRUE;
    }

    return IMS_FALSE;
}

PRIVATE GLOBAL IMS_BOOL NormalDialingPlan::IsLocalNumberFormat(IN const AString& strNumber)
{
    return !strNumber.StartsWith(TextParser::CHAR_PLUS);
}

PRIVATE GLOBAL IMS_BOOL NormalDialingPlan::IsAddressSpec(IN const AString& strNumber)
{
    // addr-spec format
    AString strTmp = strNumber.MakeLower();

    if (strTmp.StartsWith("sip:") || strTmp.StartsWith("sips:") || strTmp.StartsWith("tel:"))
    {
        return IMS_TRUE;
    }

    return IMS_FALSE;
}

PRIVATE GLOBAL void NormalDialingPlan::AddAquotIfRequired(IN_OUT AString& strNumber)
{
    // TODO: can this be replaced by SipAddress apis?
    if (strNumber.Contains(TextParser::CHAR_SEMICOLON))
    {
        strNumber.Prepend(TextParser::CHAR_LAQUOT);
        strNumber.Append(TextParser::CHAR_RAQUOT);
    }
}

PRIVATE GLOBAL NormalDialingPlan::NumberFormat NormalDialingPlan::GetDialedNumberFormat(
        IN const AString& strNumber)
{
    // Ref. RFC 3966
    // global-number-digits := "+" *phonedigit DIGIT *phonedigit
    // local-number-digits := *phonedigit-hex (HEXDIG / "*" / "#") *phonedigit-hex
    // phonedigit := DIGIT / [visual-separator]
    // phonedigit-hex := HEXDIG / "*" / "#" / [visual-separator]

    if (strNumber.Equals(TextParser::CHAR_PLUS))
    {
        // TODO: "+" is possible? in which case is it possible?
        return NumberFormat::NON_NUMBER;
    }

    if (strNumber.StartsWith(TextParser::CHAR_PLUS))
    {
        for (IMS_SINT32 i = 1; i < strNumber.GetLength(); i++)
        {
            const IMS_CHAR c = strNumber[i];

            if (!IMS_ISDIGIT(c) && !IsVisualSeparator(c))
            {
                return NumberFormat::NON_NUMBER;
            }
        }

        return NumberFormat::GLOBAL_FORMAT;
    }
    else
    {
        for (IMS_SINT32 i = 0; i < strNumber.GetLength(); i++)
        {
            const IMS_CHAR c = strNumber[i];

            if (!IMS_ISDIGIT(c) && !IsVisualSeparator(c) && (c != TextParser::CHAR_ASTERISK) &&
                    (c != TextParser::CHAR_SHARP) && !((c >= 'A') && (c <= 'F')))
            {
                return NumberFormat::NON_NUMBER;
            }
        }

        return NumberFormat::LOCAL_FORMAT;
    }
}

PRIVATE GLOBAL AccessNetworkInfo& NormalDialingPlan::GetAccessNetworkInfo(
        IN IMtcContext& objContext, OUT AccessNetworkInfo& objAni)
{
    // TODO: service type. normal or emergency.
    IMS_SINT32 nApnType = objContext.GetServiceByType(ServiceType::NORMAL)
                                  ->GetAosConnector()
                                  ->GetConnectionType();

    INetworkConnection* piConnection =
            NetworkService::GetNetworkService()->FindConnection(nApnType, objContext.GetSlotId());

    if (piConnection != IMS_NULL)
    {
        piConnection->GetAccessNetworkInfo(objAni);
    }

    return objAni;
}

PRIVATE GLOBAL NormalDialingPlan::Scheme NormalDialingPlan::GetScheme(IN IMtcContext& objContext)
{
    IMS_SINT32 nValue =
            objContext.GetConfigurationProxy().GetInt(ConfigIms::KEY_REQUEST_URI_TYPE_INT);
    IMS_TRACE_D("GetScheme config=[%d]", nValue, 0, 0);
    switch (nValue)
    {
        case ConfigIms::REQUEST_URI_FORMAT_TEL:
            return Scheme::TEL;
        default:
            return Scheme::SIP;
    }
}

PRIVATE GLOBAL NormalDialingPlan::LocalNumberPolicy NormalDialingPlan::GetLocalNumberPolicy(
        IN IMtcContext& objContext)
{
    // For policy to consist of phone-context URI parameter;
    // Refer to ImsIdentity::DIALING_POLICY_XXX

    IMS_SINT32 nValue =
            objContext.GetConfigurationProxy().GetInt(ConfigVoice::KEY_POLICY_OF_LOCAL_NUMBERS_INT);
    IMS_TRACE_D("GetLocalNumberPolicy config=[%d]", nValue, 0, 0);
    switch (nValue)
    {
        case 0:
            return LocalNumberPolicy::HOME;
        case 1:
            return LocalNumberPolicy::GEO;
        case 2:
            return LocalNumberPolicy::GEO_LOCAL_ONLY_IN_ROAMING;
        default:
            return LocalNumberPolicy::HOME;
    }
}

PRIVATE GLOBAL IMS_UINT32 NormalDialingPlan::ConvertDialingPolicy(IN LocalNumberPolicy ePolicy)
{
    return ePolicy == LocalNumberPolicy::GEO ? ImsIdentity::DIALING_POLICY_GEO_LOCAL
                                             : ImsIdentity::DIALING_POLICY_HOME_LOCAL;
}
