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
#include "ServiceConfig.h"
#include "ServiceTrace.h"
#include "ServiceNetwork.h"
#include "ServiceNetworkPolicy.h"
#include "ServiceUtil.h"
#include "ImsLib.h"
#include "ImsIdentity.h"
#include "helper/dialing/MtsDialingPlan.h"
#include "utility/MtsSipFormUtils.h"

__IMS_TRACE_TAG_COM_MTS__;

PUBLIC
MtsDialingPlan::MtsDialingPlan(
        IN IMS_SINT32 nSlotId, IN const AString& strScheme, IN IMS_SINT32 nDialingPolicy) :
        m_nSlotId(nSlotId),
        m_strScheme(strScheme),
        m_nDialingPolicy(nDialingPolicy),
        m_strNetworkProfile(AString::ConstNull())
{
    if (m_strScheme.GetLength() == 0)
    {
        // As a default URI scheme, "tel" will be used
        m_strScheme = "tel";
    }
    IMS_TRACE_D("+MtsDialingPlan [slot_%d][scheme:%s][dialogPolicy:%d]", m_nSlotId,
            m_strScheme.GetStr(), m_nDialingPolicy);
}

PUBLIC
MtsDialingPlan::~MtsDialingPlan() {}

PUBLIC
AString MtsDialingPlan::Translate(
        IN const AString& strTargetAddress, IN IMS_BOOL bAquot /* = IMS_TRUE */)
{
    if (strTargetAddress.GetLength() == 0)
    {
        IMS_TRACE_E(0, "Number is null or empty", 0, 0, 0);
        return AString::ConstNull();
    }

    AString strLog;

    IMS_TRACE_D("Dialed number :: %s",
            UtilService::GetLogString(strTargetAddress, strLog, 3).GetStr(), 0, 0);

    AStringBuffer objUri(128);

    // name-addr format
    if (strTargetAddress.Contains('<') && strTargetAddress.Contains('>'))
    {
        return strTargetAddress;
    }
    // addr-spec format
    else if (strTargetAddress.StartsWith('s') || strTargetAddress.StartsWith('t') ||
            strTargetAddress.StartsWith('S') || strTargetAddress.StartsWith('T'))
    {
        AString strTmp = strTargetAddress.GetSubStr(0, 5).MakeLower();

        if (strTmp.StartsWith("sip:") || strTmp.StartsWith("sips:") || strTmp.StartsWith("tel:"))
        {
            if (bAquot && strTargetAddress.Contains(';'))
            {
                objUri.Append('<');
                objUri.Append(strTargetAddress);
                objUri.Append('>');

                return static_cast<const AStringBuffer&>(objUri).GetString();
            }

            return strTargetAddress;
        }
    }

    IMS_SINT32 nScheme = TranslateScheme();

    if (nScheme != URI_SCHEME_TEL)
    {
        FormNonTelUri(strTargetAddress, bAquot, objUri);
    }
    else
    {
        FormTelUri(strTargetAddress, objUri);

        // Adds the URI scheme
        objUri.Prepend(':');
        objUri.Prepend(m_strScheme);

        if (bAquot && static_cast<const AStringBuffer&>(objUri).GetString().Contains(';'))
        {
            objUri.Prepend('<');
            objUri.Append('>');
        }
    }

    const AString& strURI = static_cast<const AStringBuffer&>(objUri).GetString();

    IMS_TRACE_D("Dialed URI :: %s", UtilService::GetLogString(strURI, strLog, 7).GetStr(), 0, 0);

    return strURI;
}

PRIVATE
AccessNetworkInfo* MtsDialingPlan::GetAccessNetworkInfo(IN_OUT AccessNetworkInfo& objAni)
{
    // TODO: service type. normal or emergency.
    INetworkConnection* piConnection =
            NetworkService::GetNetworkService()->FindConnection(NetworkPolicy::APN_IMS, m_nSlotId);

    if (piConnection == IMS_NULL)
    {
        return IMS_NULL;
    }

    piConnection->GetAccessNetworkInfo(objAni);

    return &objAni;
}

PRIVATE
IMS_BOOL MtsDialingPlan::FormNonTelUri(
        IN const AString& strTargetAddress, IN IMS_BOOL bAquot, OUT AStringBuffer& objUri)
{
    IMS_TRACE_I("FormNonTelUri", 0, 0, 0);

    objUri.Append(GetScheme());
    objUri.Append(':');
    objUri.Append(strTargetAddress);

    IMS_SINT32 nNumberFormat = GetNumberFormat(strTargetAddress);

    if (strTargetAddress.Contains('#'))
    {
        /*
         * The rule for "telephone-subscriber" and RFC2396 is overridden by RFC3261.
         * '#' MUST be escaped in userinfo part of SIP address.
         * if ((nDialingPolicy == ImsIdentity::DIALING_POLICY_OTHER)
         *        || (nNumberFormat == NUMBER_FORMAT_NON_TEL))
         */
        {
            objUri.Replace('#', "%23");
        }
    }

    if (nNumberFormat == NUMBER_FORMAT_NON_TEL)
    {
        // do not add "phone-context" and "user" parameters when the context is not number format
        return IMS_TRUE;
    }

    // "phone-context" parameter
    if ((m_nDialingPolicy != ImsIdentity::DIALING_POLICY_OTHER) &&
            (nNumberFormat == NUMBER_FORMAT_LOCAL))
    {
        objUri.Append(";phone-context=");

        // Provides the access network info. for geo-local number
        AccessNetworkInfo objAni;

        objUri.Append(ImsIdentity::GetPhoneContext(
                m_nDialingPolicy, m_nSlotId, GetAccessNetworkInfo(objAni)));
    }

    objUri.Append('@');
    objUri.Append(ImsIdentity::GetHomeDomainName(m_nSlotId));

    // "user" parameter
    ICarrierConfig* piCc = ConfigService::GetConfigService()->GetCarrierConfig(m_nSlotId);
    if (piCc->GetBoolean(CarrierConfig::ImsSms::KEY_SMS_USE_DIALED_NUMBER_FOR_REQUEST_URI_BOOL))
    // if ((m_nDialingPolicy != ImsIdentity::DIALING_POLICY_OTHER) &&
    //         (nNumberFormat == NUMBER_FORMAT_NON_TEL))
    {
        objUri.Append(";user=dialstring");
    }
    else
    {
        objUri.Append(";user=phone");
    }

    if (bAquot)
    {
        objUri.Prepend('<');
        objUri.Append('>');
    }

    return IMS_TRUE;
}

PRIVATE
IMS_BOOL MtsDialingPlan::FormTelUri(IN const AString& strTargetAddress, OUT AStringBuffer& objUri)
{
    IMS_TRACE_I("FormTelUri", 0, 0, 0);

    if (strTargetAddress.StartsWith('+'))
    {
        objUri.Append(strTargetAddress);
        return IMS_TRUE;
    }

    objUri.Append(strTargetAddress);
    objUri.Append(";phone-context=");
    objUri.Append(ImsIdentity::GetPhoneContext(GetDialingPolicy(), m_nSlotId));

    return IMS_TRUE;
}

PRIVATE
IMS_SINT32 MtsDialingPlan::TranslateScheme() const
{
    if (m_strScheme.EqualsIgnoreCase("tel"))
    {
        return URI_SCHEME_TEL;
    }
    else if (m_strScheme.EqualsIgnoreCase("sip"))
    {
        return URI_SCHEME_SIP;
    }
    else if (m_strScheme.EqualsIgnoreCase("sips"))
    {
        return URI_SCHEME_SIPS;
    }

    return URI_SCHEME_UNKNOWN;
}

PRIVATE GLOBAL IMS_SINT32 MtsDialingPlan::GetNumberFormat(IN const AString& strTargetAddress)
{
    /*
     * global-number-digits := "+" *phonedigit DIGIT *phonedigit
     * local-number-digits := *phonedigit-hex (HEXDIG / "*" / "#") *phonedigit-hex
     * phonedigit := DIGIT / [visual-separator]
     * phonedigit-hex := HEXDIG / "*" / "#" / [visual-separator]
     */

    if (strTargetAddress.Equals('+'))
    {
        return NUMBER_FORMAT_NON_TEL;
    }

    if (strTargetAddress.StartsWith('+'))
    {
        for (IMS_SINT32 nIndex = 1; nIndex < strTargetAddress.GetLength(); ++nIndex)
        {
            const IMS_CHAR szCh = strTargetAddress[nIndex];

            if (!IMS_ISDIGIT(szCh) && !IsVisualSeparator(szCh))
            {
                return NUMBER_FORMAT_NON_TEL;
            }
        }

        return NUMBER_FORMAT_GLOBAL;
    }
    else
    {
        for (IMS_SINT32 nIndex = 0; nIndex < strTargetAddress.GetLength(); ++nIndex)
        {
            const IMS_CHAR szCh = strTargetAddress[nIndex];

            if (!IMS_ISDIGIT(szCh) && !IsVisualSeparator(szCh) && (szCh != '*') && (szCh != '#') &&
                    !((szCh >= 'A') && (szCh <= 'F')))
            {
                return NUMBER_FORMAT_NON_TEL;
            }
        }

        return NUMBER_FORMAT_LOCAL;
    }
}

PRIVATE GLOBAL IMS_BOOL MtsDialingPlan::IsVisualSeparator(IN IMS_CHAR szCh)
{
    // "-", ".", "(", ")"
    if ((szCh == '-') || (szCh == '.') || (szCh == '(') || (szCh == ')'))
        return IMS_TRUE;

    return IMS_FALSE;
}
