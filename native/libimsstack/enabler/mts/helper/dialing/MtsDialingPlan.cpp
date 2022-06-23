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

#include "ServiceTrace.h"
#include "ServiceNetwork.h"
#include "ServiceUtil.h"
#include "ImsLib.h"
#include "ImsIdentity.h"
#include "helper/dialing/MtsDialingPlan.h"
#include "utility/MtsSipFormUtils.h"

__IMS_TRACE_TAG_COM_SMS__;

PUBLIC
MtsDialingPlan::MtsDialingPlan(
        IN IMS_SINT32 nSlotId, IN IMS_SINT32 nNumberFormat, IN const AString& strScheme) :
        m_nSlotId(nSlotId),
        m_nNumberFormat(nNumberFormat),
        m_strScheme(strScheme),
        m_nDialingPolicy(ImsIdentity::DIALING_POLICY_HOME_LOCAL),
        m_strNetworkProfile(AString::ConstNull())
{
    if (m_strScheme.GetLength() == 0)
    {
        // As a default URI scheme, "tel" will be used
        m_strScheme = "tel";
    }
}

PUBLIC
MtsDialingPlan::~MtsDialingPlan() {}

PUBLIC
AString MtsDialingPlan::Translate(IN const AString& strNumber, IN IMS_BOOL bAquot /* = IMS_TRUE */,
        IN IMS_BOOL bUssi /* = IMS_FALSE*/)
{
    if (strNumber.GetLength() == 0)
    {
        IMS_TRACE_E(0, "Number is null or empty", 0, 0, 0);
        return AString::ConstNull();
    }

    AString strLog;

    IMS_TRACE_D(
            "Dialed number :: %s", UtilService::GetLogString(strNumber, strLog, 3).GetStr(), 0, 0);

    AStringBuffer objUri(128);

    // If the dial string equals to the pre-configured service URN,
    // then the service URN will be used.
    if (TranslateAsServiceUrn(strNumber, objUri))
    {
        return static_cast<const AStringBuffer&>(objUri).GetString();
    }

    // name-addr format
    if (strNumber.Contains('<') && strNumber.Contains('>'))
    {
        return strNumber;
    }
    // addr-spec format
    else if (strNumber.StartsWith('s') || strNumber.StartsWith('t') || strNumber.StartsWith('S') ||
            strNumber.StartsWith('T'))
    {
        AString strTmp = strNumber.GetSubStr(0, 5).MakeLower();

        if (strTmp.StartsWith("sip:") || strTmp.StartsWith("sips:") || strTmp.StartsWith("tel:"))
        {
            if (bAquot && strNumber.Contains(';'))
            {
                objUri.Append('<');
                objUri.Append(strNumber);
                objUri.Append('>');

                return static_cast<const AStringBuffer&>(objUri).GetString();
            }

            return strNumber;
        }
    }
    else if (!(strNumber.StartsWith('s')) && !(strNumber.StartsWith('t')) &&
            !(strNumber.StartsWith('S')) && !(strNumber.StartsWith('T')))
    {
        objUri.Prepend(':');
        objUri.Prepend(m_strScheme);
        objUri.Append(strNumber);
        return static_cast<const AStringBuffer&>(objUri).GetString();
    }

    IMS_SINT32 nScheme = TranslateScheme(strNumber);

    if (nScheme != MtsSipFormUtils::SCHEME_TEL)
    {
        if (bUssi)
        {
            if (!FormUssiNonTelUri(strNumber, objUri, m_strScheme))
            {
                return AString::ConstNull();
            }
        }
        else
        {
            if (!FormNonTelUri(strNumber, bAquot, objUri))
            {
                return AString::ConstNull();
            }
        }
    }
    else
    {
        IMS_BOOL bOK = IMS_FALSE;

        if (m_nNumberFormat == NUMBER_FORMAT_LOCAL)
        {
            bOK = TranslateAsLocal(strNumber, objUri);
        }
        else if (m_nNumberFormat == NUMBER_FORMAT_GLOBAL)
        {
            // If the dialed number is already a global number, returns it directly.
            if (strNumber.StartsWith('+'))
            {
                objUri.Append(strNumber);
                bOK = IMS_TRUE;
            }
            else
            {
                bOK = TranslateAsGlobal(strNumber, objUri);
            }
        }

        if (!bOK)
        {
            IMS_TRACE_E(0, "Translating a number (%d) failed", m_nNumberFormat, 0, 0);
            return AString::ConstNull();
        }

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

/*

Remarks

*/
PUBLIC
AString MtsDialingPlan::Translate(IN const AString& strNumber, IN const AString& strScheme,
        IN IMS_BOOL bAquot /* = IMS_TRUE */)
{
    if (strScheme.GetLength() == 0)
    {
        return Translate(strNumber, bAquot);
    }

    if (strNumber.GetLength() == 0)
    {
        IMS_TRACE_E(0, "Number is null or empty", 0, 0, 0);
        return AString::ConstNull();
    }

    AString strLog;

    (void)strLog;

    IMS_TRACE_D(
            "Dialed number :: %s", UtilService::GetLogString(strNumber, strLog, 3).GetStr(), 0, 0);

    AStringBuffer objUri(128);

    // If the dial string equals to the pre-configured service URN,
    // then the service URN will be used.
    if (TranslateAsServiceUrn(strNumber, objUri))
    {
        return static_cast<const AStringBuffer&>(objUri).GetString();
    }

    // name-addr format
    if (strNumber.Contains('<') && strNumber.Contains('>'))
    {
        return strNumber;
    }
    // addr-spec format
    else if (strNumber.StartsWith('s') || strNumber.StartsWith('t') || strNumber.StartsWith('S') ||
            strNumber.StartsWith('T'))
    {
        AString strTmp = strNumber.GetSubStr(0, 5).MakeLower();

        if (strTmp.StartsWith("sip:") || strTmp.StartsWith("sips:") || strTmp.StartsWith("tel:"))
        {
            if (bAquot && strNumber.Contains(';'))
            {
                objUri.Append('<');
                objUri.Append(strNumber);
                objUri.Append('>');

                return static_cast<const AStringBuffer&>(objUri).GetString();
            }

            return strNumber;
        }
    }

    if (!m_strScheme.EqualsIgnoreCase("tel"))
    {
        if (!FormNonTelUri(strNumber, bAquot, objUri, m_strScheme))
        {
            return AString::ConstNull();
        }
    }
    else
    {
        IMS_BOOL bOK = IMS_FALSE;

        if (m_nNumberFormat == NUMBER_FORMAT_LOCAL)
        {
            bOK = TranslateAsLocal(strNumber, objUri);
        }
        else if (m_nNumberFormat == NUMBER_FORMAT_GLOBAL)
        {
            // If the dialed number is already a global number, returns it directly.
            if (strNumber.StartsWith('+'))
            {
                objUri.Append(strNumber);
                bOK = IMS_TRUE;
            }
            else
            {
                bOK = TranslateAsGlobal(strNumber, objUri);
            }
        }

        if (!bOK)
        {
            IMS_TRACE_E(0, "Translating a number (%d) failed", m_nNumberFormat, 0, 0);
            return AString::ConstNull();
        }

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

PUBLIC
AString MtsDialingPlan::TranslateEx(IN const AString& strNumber,
        IN IMS_SINT32 nFlags /*= FLAG_NONE */, IN IMS_BOOL bAquot /*= IMS_TRUE*/,
        IN IMS_BOOL bUssi /*= IMS_FALSE*/)
{
    (void)nFlags;
    return Translate(strNumber, bAquot, bUssi);
}

PUBLIC
AString MtsDialingPlan::TranslateEx(IN const AString& strNumber, IN const AString& strScheme,
        IN IMS_SINT32 nFlags /*= FLAG_NONE*/, IN IMS_BOOL bAquot /*= IMS_TRUE*/)
{
    (void)nFlags;
    return Translate(strNumber, strScheme, bAquot);
}

PUBLIC
IMS_SINT32 MtsDialingPlan::GetDialingPolicy() const
{
    return m_nDialingPolicy;
}

PUBLIC
IMS_SINT32 MtsDialingPlan::GetNumberFormat() const
{
    return m_nNumberFormat;
}

PUBLIC
const AString& MtsDialingPlan::GetNetworkProfile() const
{
    return m_strNetworkProfile;
}

PUBLIC
const AString& MtsDialingPlan::GetScheme() const
{
    return m_strScheme;
}

PUBLIC
void MtsDialingPlan::SetDialingPolicy(IN IMS_SINT32 nPolicy)
{
    m_nDialingPolicy = nPolicy;
}

PUBLIC
void MtsDialingPlan::SetNumberFormat(IN IMS_SINT32 nNumberFormat)
{
    if ((nNumberFormat != NUMBER_FORMAT_LOCAL) && (nNumberFormat != NUMBER_FORMAT_GLOBAL))
    {
        return;
    }

    m_nNumberFormat = nNumberFormat;
}

PUBLIC
void MtsDialingPlan::SetNetworkProfile(IN const AString& strNetworkProfile)
{
    m_strNetworkProfile = strNetworkProfile;
}

PUBLIC
void MtsDialingPlan::SetScheme(IN const AString& strScheme)
{
    m_strScheme = strScheme;
}

PROTECTED
AccessNetworkInfo* MtsDialingPlan::GetAccessNetworkInfo(IN_OUT AccessNetworkInfo& objAni)
{
    const AString& strProfile = GetNetworkProfile();

    if (strProfile.GetLength() == 0)
    {
        return IMS_NULL;
    }

    INetworkConnection* piConnection =
            NetworkService::GetNetworkService()->FindConnection(strProfile, m_nSlotId);

    if (piConnection == IMS_NULL)
    {
        return IMS_NULL;
    }

    piConnection->GetAccessNetworkInfo(objAni);

    return &objAni;
}

PROTECTED
IMS_BOOL MtsDialingPlan::TranslateAsGlobal(IN const AString& strNumber, OUT AStringBuffer& objUri)
{
    /*
     * TODO:
     * Currenttly Request-URI set as the dialed number which is received form AP SMS stack.
     * When PSI value can be checked then this method needs to be changed as normal.
     */
    objUri.Append(strNumber);
    return IMS_TRUE;
}

PROTECTED
IMS_BOOL MtsDialingPlan::TranslateAsLocal(IN const AString& strNumber, OUT AStringBuffer& objUri)
{
    if (strNumber.StartsWith('+'))
    {
        objUri.Append(strNumber);
        return IMS_TRUE;
    }

    objUri.Append(strNumber);
    objUri.Append(";phone-context=");
    objUri.Append(ImsIdentity::GetPhoneContext(GetDialingPolicy(), m_nSlotId));

    return IMS_TRUE;
}

PROTECTED
IMS_BOOL MtsDialingPlan::FormNonTelUri(IN const AString& strNumber, IN IMS_BOOL bAquot,
        OUT AStringBuffer& objUri, IN const AString& strScheme /* = AString::ConstNull() */)
{
    if (strScheme.GetLength() != 0)
    {
        objUri.Append(strScheme);
    }
    else
    {
        if (GetScheme().EqualsIgnoreCase("tel"))
        {
            objUri.Append("sip");
        }
        else
        {
            objUri.Append(GetScheme());
        }
    }

    objUri.Append(':');
    objUri.Append(strNumber);

    IMS_SINT32 nDialedNumberFormat = GetDialedNumberFormat(strNumber);

    if (strNumber.Contains('#'))
    {
        /*
         * The rule for "telephone-subscriber" and RFC2396 is overridden by RFC3261.
         * '#' MUST be escaped in userinfo part of SIP address.
         * if ((nDialingPolicy == ImsIdentity::DIALING_POLICY_OTHER)
         *        || (nDialedNumberFormat == NUMBER_FORMAT_NON_TEL))
         */
        {
            objUri.Replace('#', "%23");
        }
    }

    // "phone-context" parameter
    if ((m_nDialingPolicy != ImsIdentity::DIALING_POLICY_OTHER) &&
            (nDialedNumberFormat != NUMBER_FORMAT_GLOBAL))
    {
        objUri.Append(";phone-context=");

        // Provides the access network info. for geo-local number
        AccessNetworkInfo objAni;

        objUri.Append(ImsIdentity::GetPhoneContext(
                m_nDialingPolicy, m_nSlotId, GetAccessNetworkInfo(objAni)));
    }

    objUri.Append('@');
    objUri.Append(ImsIdentity::GetHomeDomainName(m_nSlotId));

    if ((m_nDialingPolicy != ImsIdentity::DIALING_POLICY_OTHER) &&
            (nDialedNumberFormat == NUMBER_FORMAT_NON_TEL))
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

PROTECTED
IMS_BOOL MtsDialingPlan::FormUssiNonTelUri(IN const AString& strNumber, OUT AStringBuffer& objUri,
        IN const AString& strScheme /* = AString::ConstNull() */)
{
    IMS_TRACE_I("FormUssiNonTelUri", 0, 0, 0);

    if (strScheme.GetLength() != 0)
    {
        objUri.Append(strScheme);
    }
    else
    {
        if (GetScheme().EqualsIgnoreCase("tel"))
        {
            objUri.Append("sip");
        }
        else
        {
            objUri.Append(GetScheme());
        }
    }

    objUri.Append(':');
    objUri.Append(strNumber);

    if (strNumber.Contains('#'))
    {
        objUri.Replace('#', "%23");
    }

    objUri.Append(";phone-context=");

    objUri.Append(ImsIdentity::GetHomeDomainName(m_nSlotId));
    objUri.Append('@');
    objUri.Append(ImsIdentity::GetHomeDomainName(m_nSlotId));

    objUri.Append(";user=dialstring");
    objUri.Prepend('<');
    objUri.Append('>');

    return IMS_TRUE;
}

PROTECTED
IMS_BOOL MtsDialingPlan::TranslateAsServiceUrn(
        IN const AString& /* strNumber */, OUT AStringBuffer& /* objUri */)
{
    return IMS_FALSE;
}

PROTECTED
IMS_SINT32 MtsDialingPlan::TranslateScheme(IN const AString& /* strNumber */) const
{
    if (m_strScheme.EqualsIgnoreCase("tel"))
    {
        return MtsSipFormUtils::SCHEME_TEL;
    }
    else if (m_strScheme.EqualsIgnoreCase("sip"))
    {
        return MtsSipFormUtils::SCHEME_SIP;
    }
    else if (m_strScheme.EqualsIgnoreCase("sips"))
    {
        return MtsSipFormUtils::SCHEME_SIPS;
    }

    return MtsSipFormUtils::SCHEME_UNKNOWN;
}

PROTECTED GLOBAL IMS_SINT32 MtsDialingPlan::GetDialedNumberFormat(IN const AString& strDial)
{
    /*
     * global-number-digits := "+" *phonedigit DIGIT *phonedigit
     * local-number-digits := *phonedigit-hex (HEXDIG / "*" / "#") *phonedigit-hex
     * phonedigit := DIGIT / [visual-separator]
     * phonedigit-hex := HEXDIG / "*" / "#" / [visual-separator]
     */

    if (strDial.Equals('+'))
    {
        return NUMBER_FORMAT_NON_TEL;
    }

    if (strDial.StartsWith('+'))
    {
        for (IMS_SINT32 nIndex = 1; nIndex < strDial.GetLength(); ++nIndex)
        {
            const IMS_CHAR szCh = strDial[nIndex];

            if (!IMS_ISDIGIT(szCh) && !IsVisualSeparator(szCh))
            {
                return NUMBER_FORMAT_NON_TEL;
            }
        }

        return NUMBER_FORMAT_GLOBAL;
    }
    else
    {
        for (IMS_SINT32 nIndex = 0; nIndex < strDial.GetLength(); ++nIndex)
        {
            const IMS_CHAR szCh = strDial[nIndex];

            if (!IMS_ISDIGIT(szCh) && !IsVisualSeparator(szCh) && (szCh != '*') && (szCh != '#') &&
                    !((szCh >= 'A') && (szCh <= 'F')))
            {
                return NUMBER_FORMAT_NON_TEL;
            }
        }

        return NUMBER_FORMAT_LOCAL;
    }
}

PROTECTED GLOBAL IMS_BOOL MtsDialingPlan::IsVisualSeparator(IN IMS_CHAR szCh)
{
    // "-", ".", "(", ")"
    if ((szCh == '-') || (szCh == '.') || (szCh == '(') || (szCh == ')'))
        return IMS_TRUE;

    return IMS_FALSE;
}
