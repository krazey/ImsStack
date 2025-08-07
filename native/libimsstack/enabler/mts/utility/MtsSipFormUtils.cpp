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
#include "IMessage.h"
#include "ISipHeader.h"
#include "ImsLib.h"
#include "IpAddress.h"
#include "MtsDef.h"
#include "ServiceConfig.h"
#include "ServiceTrace.h"
#include "SipHeaderName.h"
#include "SipParsingHelper.h"
#include "helper/dialing/MtsDialingPlan.h"
#include "utility/MtsSipFormUtils.h"

__IMS_TRACE_TAG_COM_MTS__;

PUBLIC
MtsSipFormUtils::MtsSipFormUtils(IN IMS_SINT32 nSlotId) :
        m_pMtsDialingPlan(IMS_NULL),
        m_nSlotId(nSlotId)
{
    IMS_TRACE_I("+MtsSipFormUtils [slot_%d]", m_nSlotId, 0, 0);

    const ICarrierConfig* piCc = ConfigService::GetConfigService()->GetCarrierConfig(m_nSlotId);
    IMS_SINT32 nRequestUriType = GetRequestUriType();
    AString strUriScheme;

    if (nRequestUriType == CarrierConfig::Ims::REQUEST_URI_FORMAT_TEL)
    {
        strUriScheme = "tel";
    }
    else if (nRequestUriType == CarrierConfig::Ims::REQUEST_URI_FORMAT_SIP)
    {
        strUriScheme = "sip";
    }

    m_pMtsDialingPlan = new MtsDialingPlan(nSlotId, strUriScheme,
            piCc->GetInt(CarrierConfig::ImsVoice::KEY_POLICY_OF_LOCAL_NUMBERS_INT));
}

PUBLIC
MtsSipFormUtils::~MtsSipFormUtils()
{
    if (m_pMtsDialingPlan != IMS_NULL)
    {
        delete m_pMtsDialingPlan;
        m_pMtsDialingPlan = IMS_NULL;
    }
}

PUBLIC VIRTUAL IMS_BOOL MtsSipFormUtils::FormDestination(IN const AString& strTargetAddress,
        IN const IMS_BOOL bIsAckorError, IN const AString& strLastIpSmgw, OUT AString& strDest)
{
    IMS_TRACE_I("FormDestination: strTargetAddress = %s", strTargetAddress.GetStr(), 0, 0);

    if (bIsAckorError == IMS_TRUE)
    {
        /*
         * If the sending message is sms acknowledgment, then we set the destination address,
         * the Request-Uri to SMS GW address which was included in P-Asserted-Identity in
         * the received message.
         */
        strDest = strLastIpSmgw;

        if (strDest.IsEmpty() || strDest.IsNULL())
        {
            IMS_TRACE_E(0, "SMSGW for SMS Acknowledgment is EMPTY!!", 0, 0, 0);
            return IMS_FALSE;
        }

        IMS_TRACE_D("Destination address = %s", strDest.GetStr(), 0, 0);

        return IMS_TRUE;
    }
    else
    {
        if (strTargetAddress.GetLength() == 0)
        {
            IMS_TRACE_E(0, "FormDestination : Target address is not valid", 0, 0, 0);
            return IMS_FALSE;
        }

        strDest = m_pMtsDialingPlan->Translate(strTargetAddress, IMS_TRUE);

        if (strDest.GetLength() != 0)
        {
            IMS_TRACE_D("Destination address = %s", strDest.GetStr(), 0, 0);
            return IMS_TRUE;
        }
        else
        {
            IMS_TRACE_E(0, "Form Destination Failed!!", 0, 0, 0);
            return IMS_FALSE;
        }
    }
}

PUBLIC
AString MtsSipFormUtils::FormContentTypeEnumToStr(IN SmsFormatType nType)
{
    AString strContentType;

    switch (nType)
    {
        case SmsFormatType::SMSFORMAT_3GPP:
            strContentType = AString("application/vnd.3gpp.sms");
            break;

        case SmsFormatType::SMSFORMAT_3GPP2:
            strContentType = AString("application/vnd.3gpp2.sms");
            break;

        default:
            IMS_TRACE_E(0, "This ContentType Enum vaule is not supportive", 0, 0, 0);
            break;
    }

    IMS_TRACE_D("FormContentTypeEnumToStr (%d) to (%s)", nType, strContentType.GetStr(), 0);

    return strContentType;
}

PUBLIC
SmsFormatType MtsSipFormUtils::FormContentTypeStrToEnum(IN const AString& strContentType)
{
    SmsFormatType eSmsFormat = SmsFormatType::SMSFORMAT_INVALID;

    if (IMS_TRUE == strContentType.MakeLower().Contains("application/vnd.3gpp.sms"))
    {
        eSmsFormat = SmsFormatType::SMSFORMAT_3GPP;
    }
    else if (IMS_TRUE == strContentType.MakeLower().Contains("application/vnd.3gpp2.sms"))
    {
        eSmsFormat = SmsFormatType::SMSFORMAT_3GPP2;
    }
    else
    {
        IMS_TRACE_E(0, "This ContentType Str (%s) is not supported", strContentType.GetStr(), 0, 0);
    }

    IMS_TRACE_D("FormContentTypeStrToEnum (%s) to (%d)", strContentType.GetStr(), eSmsFormat, 0);

    return eSmsFormat;
}

PUBLIC VIRTUAL IMS_BOOL MtsSipFormUtils::IsTelUrlParam(IN const AString& strParam)
{
    if (strParam.Equals("isub") || strParam.Equals("postd") || strParam.Equals("phone-context") ||
            strParam.Equals("tsp"))
    {
        return IMS_TRUE;
    }
    return IMS_FALSE;
}

PUBLIC VIRTUAL IMS_BOOL MtsSipFormUtils::IsNumberFormat(IN const AString& strDial)
{
    /*
     * global-number-digits := "+" *phonedigit DIGIT *phonedigit
     * local-number-digits := *phonedigit-hex (HEXDIG / "*" / "#") *phonedigit-hex
     * phonedigit := DIGIT / [visual-separator]
     * phonedigit-hex := HEXDIG / "*" / "#" / [visual-separator]
     */

    if (strDial.Equals('+'))
    {
        return IMS_FALSE;
    }

    if (strDial.StartsWith('+'))
    {
        for (IMS_SINT32 i = 1; i < strDial.GetLength(); ++i)
        {
            const IMS_CHAR c = strDial[i];

            if (!IMS_ISDIGIT(c) && !IsVisualSeparator(c))
            {
                return IMS_FALSE;
            }
        }
        return IMS_TRUE;
    }
    else
    {
        for (IMS_SINT32 i = 0; i < strDial.GetLength(); ++i)
        {
            const IMS_CHAR c = strDial[i];

            if (!IMS_ISDIGIT(c) && !IsVisualSeparator(c) && (c != '*') && (c != '#') &&
                    !((c >= 'A') && (c <= 'F')))
            {
                return IMS_FALSE;
            }
        }
        return IMS_TRUE;
    }
}

PUBLIC VIRTUAL IMS_BOOL MtsSipFormUtils::IsIpAddress(IN const AString& strIp)
{
    IpAddress objHost;

    if (objHost.Parse(strIp))
    {
        return IMS_TRUE;
    }

    return IMS_FALSE;
}

PUBLIC
IMS_SINT32 MtsSipFormUtils::CheckScheme(IN const AString& strTargetAddress)
{
    if (strTargetAddress.StartsWith('s') || strTargetAddress.StartsWith('t') ||
            strTargetAddress.StartsWith('S') || strTargetAddress.StartsWith('T'))
    {
        AString strTmp = strTargetAddress.GetSubStr(0, 5).MakeLower();

        if (strTmp.StartsWith("tel"))
        {
            return URI_SCHEME_TEL;
        }
        else if (strTmp.StartsWith("sips"))
        {
            return URI_SCHEME_SIPS;
        }
        else if (strTmp.StartsWith("sip"))
        {
            return URI_SCHEME_SIP;
        }
    }
    return URI_SCHEME_UNKNOWN;
}

PUBLIC
IMS_SINT32 MtsSipFormUtils::GetRetryAfterValue(IN const IMessage* piMessage) const
{
    if (piMessage == IMS_NULL)
    {
        return -1;
    }

    ImsList<AString> objHeaderList = piMessage->GetHeaders(SipHeaderName::RETRY_AFTER);
    if (objHeaderList.IsEmpty())
    {
        IMS_TRACE_E(0, "Error Response Message has not Retry-After Header", 0, 0, 0);
        return -1;
    }

    AString strHeader = objHeaderList.GetAt(objHeaderList.GetSize() - 1);
    if (strHeader.GetLength() == 0)
    {
        return -1;
    }

    ISipHeader* piHeader = SipParsingHelper::CreateHeader(ISipHeader::RETRY_AFTER_SEC, strHeader);
    if (piHeader == IMS_NULL)
    {
        return -1;
    }

    IMS_SINT32 nValue = piHeader->GetValueInt();
    piHeader->Destroy();

    IMS_TRACE_I("GetRetryAfterValue : Retry-After[%d]", nValue, 0, 0);

    return nValue;
}

PRIVATE
IMS_SINT32 MtsSipFormUtils::GetRequestUriType()
{
    const ICarrierConfig* piCc = ConfigService::GetConfigService()->GetCarrierConfig(m_nSlotId);
    IMS_SINT32 nValue = piCc->GetInt(CarrierConfig::ImsSms::KEY_SMS_PREFERRED_PSI_URI_TYPE_INT);

    if (nValue == URI_SCHEME_UNKNOWN)
    {
        return piCc->GetInt(CarrierConfig::Ims::KEY_REQUEST_URI_TYPE_INT);
    }
    else
    {
        return nValue;
    }
}

PRIVATE
IMS_BOOL MtsSipFormUtils::IsVisualSeparator(IN const IMS_CHAR ch)
{
    // "-", ".", "(", ")"
    if ((ch == '-') || (ch == '.') || (ch == '(') || (ch == ')'))
    {
        return IMS_TRUE;
    }
    return IMS_FALSE;
}
