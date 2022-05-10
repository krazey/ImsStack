#include "AString.h"
#include "IMSLib.h"
#include "IMSStrLib.h"
#include "ServiceTrace.h"
#include "SipAddress.h"
#include "SipParameter.h"
#include "IPAddress.h"
#include "helper/dialing/MtsDialingPlan.h"
#include "utility/MtsSipFormUtils.h"
#include "utility/MtsSmUtils.h"

#include "Configuration.h"

__IMS_TRACE_TAG_COM_SMS__;

PUBLIC
MtsSipFormUtils::MtsSipFormUtils(IN IMS_SINT32 nSlotId) :
        m_nMtsFormat(MtsSmUtils::MTS_SMS_FORMAT_INVALID),
        m_pMtsDialingPlan(IMS_NULL),
        m_strPsi(AString::ConstNull()),
        m_nSlotId(nSlotId)
{
    IMS_TRACE_I("+MtsSipFormUtils", 0, 0, 0);

    // TODO: Need to check "header_info_target_scheme" and "header_info_target_scheme"
    m_pMtsDialingPlan = new MtsDialingPlan(nSlotId, MtsDialingPlan::NUMBER_FORMAT_GLOBAL, "tel");
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

PUBLIC
MtsSipFormUtils* MtsSipFormUtils::GetInstance(IN IMS_SINT32 nSlotId)
{
    static MtsSipFormUtils* s_pMtsSipFormUtils = IMS_NULL;

    if (s_pMtsSipFormUtils == IMS_NULL)
    {
        s_pMtsSipFormUtils = new MtsSipFormUtils(nSlotId);
    }

    return s_pMtsSipFormUtils;
}

PUBLIC VIRTUAL IMS_BOOL MtsSipFormUtils::FormDestination(IN const IMS_CHAR* szMDN,
        IN const IMS_BOOL bIsAckorError, IN const AString& strLastIpSmgw, OUT AString& strDest)
{
    IMS_TRACE_I("FormDestination: szMDN = %s", szMDN, 0, 0);

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
        AString strMDN_(szMDN);

        if (strMDN_.IsEmpty())
        {
            IMS_TRACE_E(0, "FormDestination : Peer MDN is not valid", 0, 0, 0);
            return IMS_FALSE;
        }

        // Update the format
        UpdateFormatFromDb();

        if (MtsSipFormUtils::UpdatePsiFromDb() == IMS_FALSE)
        {
            IMS_TRACE_I(
                    "FormDestination - PSI from SIM is wrong, so we wil make PSI by SMSC", 0, 0, 0);
            // TODO: AT&T Operator needs tel URI when PSI is not available
            strDest = m_pMtsDialingPlan->Translate(strMDN_, IMS_TRUE);
        }
        else
        {
            IMS_TRACE_I("FormDestination - Use PSI from SIM", 0, 0, 0);
            /*
             * TODO:
             * LGU, KT, KDDI only use SIP URI and KT also needs home domain name.
             * The depreciated method(ValidateAndUpdatePsi) can be referred.
             */
            strDest = ValidateAndUpdatePsi();
        }

        if (!strDest.IsEmpty())
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
AString MtsSipFormUtils::FormContentTypeEnumToStr(IN IMS_UINT32 nType)
{
    AString strContentType;

    switch (nType)
    {
        case MtsSmUtils::MTS_SMS_FORMAT_3GPP:
            strContentType = AString("application/vnd.3gpp.sms");
            break;

        case MtsSmUtils::MTS_SMS_FORMAT_3GPP2:
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
IMS_UINT32 MtsSipFormUtils::FormContentTypeStrToEnum(IN AString strContentType)
{
    IMS_UINT32 nType = MtsSmUtils::MTS_SMS_FORMAT_INVALID;

    if (IMS_TRUE == strContentType.MakeLower().Contains("application/vnd.3gpp.sms"))
    {
        nType = MtsSmUtils::MTS_SMS_FORMAT_3GPP;
    }
    else if (IMS_TRUE == strContentType.MakeLower().Contains("application/vnd.3gpp2.sms"))
    {
        nType = MtsSmUtils::MTS_SMS_FORMAT_3GPP2;
    }
    else
    {
        IMS_TRACE_E(0, "This ContentType Str (%s) is not supported", 0, 0, 0);
    }

    IMS_TRACE_D("FormContentTypeStrToEnum (%s) to (%d)", strContentType.GetStr(), nType, 0);

    return nType;
}

PUBLIC VIRTUAL void MtsSipFormUtils::UpdateFormatFromDb()
{
    // TODO: this method is deprecated. It will be removed.
    IMS_TRACE_I("UpdateFormatFromDb", 0, 0, 0);

    if (m_nMtsFormat == MtsSmUtils::MTS_SMS_FORMAT_3GPP ||
            m_nMtsFormat == MtsSmUtils::MTS_SMS_FORMAT_3GPP2)
    {
        IMS_TRACE_I("UpdateFormatFromDb : m_nMtsFormat is already set", 0, 0, 0);
        return;
    }

    AString strFormat = "3gpp";

    if (strFormat == "3gpp")
    {
        m_nMtsFormat = MtsSmUtils::MTS_SMS_FORMAT_3GPP;
    }
    else if (strFormat == "3gpp2")
    {
        m_nMtsFormat = MtsSmUtils::MTS_SMS_FORMAT_3GPP2;
    }
    else
    {
        m_nMtsFormat = MtsSmUtils::MTS_SMS_FORMAT_INVALID;
    }

    IMS_TRACE_D("MtsSipFormUtils::UpdateFormatFromDb: Format value (%d)", m_nMtsFormat, 0, 0);
}

PUBLIC VIRTUAL IMS_BOOL MtsSipFormUtils::UpdatePsiFromDb()
{
    // TODO: This is a deprecated method. There will be new implementation for getting PSI value
    IMS_TRACE_I("UpdatePsiFromDb slot ID [%d]", m_nSlotId, 0, 0);
    return IMS_FALSE;
}

PUBLIC VIRTUAL IMS_SINT32 MtsSipFormUtils::GetSlotId()
{
    IMS_TRACE_D("GetSlotId[%d]", m_nSlotId, 0, 0);
    return m_nSlotId;
}

PUBLIC VIRTUAL AString MtsSipFormUtils::ValidateAndUpdatePsi()
{
    AStringBuffer objURI(128);
    IMS_SINT32 strSheme = CheckScheme(m_strPsi);

    IMS_TRACE_I("MtsSipFormUtils::ValidateAndUpdatePsi", 0, 0, 0);

    if (strSheme == SCHEME_UNKNOWN)
    {
        objURI.Append(m_strPsi);

        if (!IsIpAddress(m_strPsi) && IsNumberFormat(m_strPsi))
        {
            objURI.Prepend("tel:");
        }
        else
        {
            objURI.Prepend("sip:");
        }
    }
    else if (strSheme == SCHEME_SIP || strSheme == SCHEME_SIPS)
    {
        // General form - sip:user:password@host:port;uri-parameters
        SipAddress* pTempDest = new SipAddress(m_strPsi);

        if (pTempDest == IMS_NULL)
        {
            IMS_TRACE_E(0,
                    "MtsSipFormUtils::ValidateAndUpdatePsi: Fail to get Host. "
                    "PSI is not updated",
                    0, 0, 0);
            objURI.Append(m_strPsi);
            return static_cast<const AStringBuffer&>(objURI).GetString();
        }

        if ((pTempDest->GetHost() != IMS_NULL) && !IsIpAddress(pTempDest->GetHost().GetStr()) &&
                IsNumberFormat(pTempDest->GetHost().GetStr()))
        {
            // Host which is number format means that user info is empty, so try to re-form
            const ISubscriberConfig* piSubsConfig =
                    Configuration::GetInstance()->GetSubscriberConfig(m_nSlotId);
            IMSList<SipParameter*> objParameters = pTempDest->GetParameters();

            if (piSubsConfig != IMS_NULL)
            {
                if (objParameters.IsEmpty())
                {
                    // Add HomeDomain to existing PSI
                    objURI.Sprintf(
                            "%s@%s", m_strPsi.GetStr(), piSubsConfig->GetHomeDomainName().GetStr());
                }
                else
                {
                    // Re-form. Add scheme & user
                    objURI.Sprintf("%s:%s", pTempDest->GetScheme().GetStr(),
                            pTempDest->GetHost().GetStr());

                    for (IMS_UINT32 i = 0; i < objParameters.GetSize(); ++i)
                    {
                        SipParameter* pParameter = objParameters.GetAt(i);

                        if (pParameter == IMS_NULL)
                            continue;

                        if (IsTelUrlParam(pParameter->GetName()))
                        {
                            // Add tel url parameter in user part. phone-context, postd, isub, tsp
                            objURI.Sprintf("%s;%s", objURI.GetString().GetStr(),
                                    pParameter->ToString().GetStr());
                        }
                    }
                    // Add HomeDomain
                    objURI.Sprintf("%s@%s", objURI.GetString().GetStr(),
                            piSubsConfig->GetHomeDomainName().GetStr());
                }
                // Add user param because user part is number format
                objURI.Append(";user=phone");
            }
        }
        delete pTempDest;
    }

    if (objURI.IsEmpty())
    {
        IMS_TRACE_I("PSI is not updated", 0, 0, 0);
        objURI.Append(m_strPsi);
    }

    return static_cast<const AStringBuffer&>(objURI).GetString();
}

PUBLIC VIRTUAL IMS_BOOL MtsSipFormUtils::IsTelUrlParam(IN const AString& strParam) const
{
    if (strParam.Equals("isub") || strParam.Equals("postd") || strParam.Equals("phone-context") ||
            strParam.Equals("tsp"))
    {
        return IMS_TRUE;
    }
    return IMS_FALSE;
}

PUBLIC VIRTUAL IMS_BOOL MtsSipFormUtils::IsNumberFormat(IN const AString& strDial) const
{
    // global-number-digits := "+" *phonedigit DIGIT *phonedigit
    // local-number-digits := *phonedigit-hex (HEXDIG / "*" / "#") *phonedigit-hex
    // phonedigit := DIGIT / [visual-separator]
    // phonedigit-hex := HEXDIG / "*" / "#" / [visual-separator]

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

PUBLIC VIRTUAL IMS_BOOL MtsSipFormUtils::IsIpAddress(IN const AString& strIp) const
{
    IPAddress objHost;

    if (objHost.Parse(strIp))
    {
        return IMS_TRUE;
    }

    return IMS_FALSE;
}

PUBLIC
IMS_SINT32 MtsSipFormUtils::CheckScheme(IN const AString& strNumber) const
{
    if (strNumber.StartsWith('s') || strNumber.StartsWith('t') || strNumber.StartsWith('S') ||
            strNumber.StartsWith('T'))
    {
        AString strTmp = strNumber.GetSubStr(0, 5).MakeLower();

        if (strTmp.StartsWith("tel"))
        {
            return SCHEME_TEL;
        }
        else if (strTmp.StartsWith("sip"))
        {
            return SCHEME_SIP;
        }
        else if (strTmp.StartsWith("sips"))
        {
            return SCHEME_SIPS;
        }
    }
    return SCHEME_UNKNOWN;
}

PROTECTED VIRTUAL MtsDialingPlan* MtsSipFormUtils::GetDialingPlan(IN IMS_SINT32 nSlotId)
{
    IMS_TRACE_E(0, "MtsSipFormUtils::GetDialingPlan : nSlotId[%d]", nSlotId, 0, 0);
    return m_pMtsDialingPlan;
}

PRIVATE GLOBAL IMS_BOOL MtsSipFormUtils::IsVisualSeparator(IN IMS_CHAR ch)
{
    // "-", ".", "(", ")"
    if ((ch == '-') || (ch == '.') || (ch == '(') || (ch == ')'))
    {
        return IMS_TRUE;
    }
    return IMS_FALSE;
}
