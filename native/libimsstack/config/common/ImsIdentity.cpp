/*
    Author
    <table>
    date      author                    description
    --------  --------------            ----------
    20100905  hwangoo.park@             Created
    </table>

    Description

*/

#include "ServiceMemory.h"
#include "ServicePhoneInfo.h"
#include "AStringBuffer.h"
#include "ImsAccessNetworkInfoType.h"
#include "private/ConfigurationManager.h"
#include "private/SubscriberConfig.h"
#include "ImsIdentity.h"

/*

Remarks

*/
PUBLIC GLOBAL AString ImsIdentity::CreateSIPUserId(
        IN IMS_SINT32 nSlotId /* = IMS_SLOT_0*/, IN IMS_BOOL bUserPhoneParam /* = IMS_FALSE */)
{
    // "sip:<global number>@<home domain name>;user=phone" ; it will be derived from the MSISDN
    ISubscriberInfo* piSubsInfo =
            PhoneInfoService::GetPhoneInfoService()->GetSubscriberInfo(nSlotId);

    if (piSubsInfo == IMS_NULL)
    {
        return AString::ConstNull();
    }

    AString strMSISDN;

    piSubsInfo->GetPhoneNumber(strMSISDN);

    return CreateSIPUserId(strMSISDN, nSlotId, bUserPhoneParam);
}

/*

Remarks

*/
PUBLIC GLOBAL AString ImsIdentity::CreateSIPUserId(IN const AString& strDialString,
        IN IMS_SINT32 nSlotId /* = IMS_SLOT_0*/, IN IMS_BOOL bUserPhoneParam /* = IMS_FALSE */,
        IN const AString& strPhoneContext /* = AString::ConstNull() */)
{
    // "sip:<global number>@<home domain name>;user=phone" ;
    // It will be derived from the specified number

    if (strDialString.GetLength() == 0)
    {
        return CreateSIPUserId(nSlotId);
    }

    AStringBuffer objURI(64);

    objURI.Append("sip:");
    objURI.Append(strDialString);

    if (strDialString.Contains('#'))
    {
        objURI.Replace('#', "%23");
    }

    objURI.Append('@');
    objURI.Append(
            (strPhoneContext.GetLength() == 0) ? GetHomeDomainName(nSlotId) : strPhoneContext);

    if (bUserPhoneParam)
    {
        objURI.Append(";user=phone");
    }

    return static_cast<const AStringBuffer&>(objURI).GetString();
}

/*

Remarks

*/
PUBLIC GLOBAL AString ImsIdentity::CreateSIPUserIdWithDialString(IN const AString& strDialString,
        IN IMS_SINT32 nSlotId /* = IMS_SLOT_0*/,
        IN const AString& strPhoneContext /* = AString::ConstNull() */)
{
    // "sip:<dialstring>;phone-context=<home domain name>@<home domain name>;user=dialstring";

    if (strDialString.GetLength() == 0)
    {
        return AString::ConstNull();
    }

    AStringBuffer objURI(64);

    objURI.Append("sip:");
    objURI.Append(strDialString);

    if (strDialString.Contains('#'))
    {
        objURI.Replace('#', "%23");
    }

    objURI.Append(";phone-context=");

    if (strPhoneContext.GetLength() == 0)
    {
        const AString strDefaultPC = GetPhoneContext(DIALING_POLICY_HOME_LOCAL, nSlotId);
        objURI.Append(strDefaultPC);
    }
    else
    {
        objURI.Append(strPhoneContext);
    }

    objURI.Append('@');
    objURI.Append(GetHomeDomainName(nSlotId));
    objURI.Append(";user=dialstring");

    return static_cast<const AStringBuffer&>(objURI).GetString();
}

/*

Remarks

*/
PUBLIC GLOBAL AString ImsIdentity::CreateSIPUserIdWithPhone(IN const AString& strDialString,
        IN IMS_SINT32 nSlotId /* = IMS_SLOT_0*/,
        IN const AString& strPhoneContext /* = AString::ConstNull() */)
{
    // "sip:<dialstring>;phone-context=<home domain name>@<home domain name>;user=phone";

    if (strDialString.GetLength() == 0)
    {
        return AString::ConstNull();
    }

    AStringBuffer objURI(64);

    objURI.Append("sip:");
    objURI.Append(strDialString);
    objURI.Append(";phone-context=");

    if (strPhoneContext.GetLength() == 0)
    {
        const AString strDefaultPC = GetPhoneContext(DIALING_POLICY_HOME_LOCAL, nSlotId);
        objURI.Append(strDefaultPC);
    }
    else
    {
        objURI.Append(strPhoneContext);
    }

    objURI.Append('@');
    objURI.Append(GetHomeDomainName(nSlotId));
    objURI.Append(";user=phone");

    return static_cast<const AStringBuffer&>(objURI).GetString();
}

/*

Remarks

*/
PUBLIC GLOBAL AString ImsIdentity::CreateTelUserId(
        IN const AString& strPhoneContext, IN IMS_SINT32 nSlotId /* = IMS_SLOT_0*/)
{
    // "tel:<global number>" ; it will be derived from the MSISDN
    ISubscriberInfo* piSubsInfo =
            PhoneInfoService::GetPhoneInfoService()->GetSubscriberInfo(nSlotId);

    if (piSubsInfo == IMS_NULL)
    {
        return AString::ConstNull();
    }

    AString strMSISDN;

    piSubsInfo->GetPhoneNumber(strMSISDN);

    return CreateTelUserId(strMSISDN, strPhoneContext, nSlotId);
}

/*

Remarks

*/
PUBLIC GLOBAL AString ImsIdentity::CreateTelUserId(IN const AString& strDialString,
        IN const AString& strPhoneContext, IN IMS_SINT32 nSlotId /* = IMS_SLOT_0*/)
{
    // "tel:<global number>" ; it will be derived from the MSISDN

    if (strDialString.GetLength() == 0)
    {
        return CreateTelUserId(strPhoneContext, nSlotId);
    }

    AStringBuffer objURI(64);

    objURI.Append("tel:");
    objURI.Append(strDialString);

    if (!strDialString.StartsWith('+'))
    {
        objURI.Append(";phone-context=");

        if (strPhoneContext.GetLength() == 0)
        {
            const AString strDefaultPC = GetPhoneContext(DIALING_POLICY_HOME_LOCAL, nSlotId);
            objURI.Append(strDefaultPC);
        }
        else
        {
            objURI.Append(strPhoneContext);
        }
    }

    return static_cast<const AStringBuffer&>(objURI).GetString();
}

/*

Remarks

*/
PUBLIC GLOBAL AString ImsIdentity::CreateTemporaryHomeDomainName(
        IN IMS_SINT32 nSlotId /* = IMS_SLOT_0*/)
{
    // "ims.mnc<MNC>.mcc<MCC>.3gppnetwork.org"
    // WLAN : "wlan.mnc<MNC>.mcc<MCC>.3gppnetwork.org"
    // Emergency : "sos.wlan.mnc<MNC>.mcc<MCC>.3gppnetwork.org"
    ISubscriberInfo* piSubsInfo =
            PhoneInfoService::GetPhoneInfoService()->GetSubscriberInfo(nSlotId);

    if (piSubsInfo == IMS_NULL)
    {
        return AString::ConstNull();
    }

    AString strMCC;
    AString strMNC;

    piSubsInfo->GetMcc(strMCC);
    piSubsInfo->GetMnc(strMNC);

    if ((strMCC.GetLength() == 0) || (strMNC.GetLength() == 0))
    {
        return AString::ConstNull();
    }

    if (strMNC.GetLength() == 2)
    {
        strMNC.Prepend('0');
    }

    AString strTHDN;

    strTHDN.Sprintf("ims.mnc%s.mcc%s.3gppnetwork.org", strMNC.GetStr(), strMCC.GetStr());

    return strTHDN;
}

/*

Remarks

*/
PUBLIC GLOBAL AString ImsIdentity::CreateTemporaryPrivateUserId(
        IN IMS_SINT32 nSlotId /* = IMS_SLOT_0*/)
{
    // "<IMSI>@ims.mnc<MNC>.mcc<MCC>.3gppnetwork.org"

    // Root NAI
    // EAP AKA authentication : "0<IMSI>@wlan.mnc<MNC>.mcc<MCC>.3gppnetwork.org"
    // EAP SIM authentication : "1<IMSI>@wlan.mnc<MNC>.mcc<MCC>.3gppnetwork.org"

    // Decorated NAI
    // EAP AKA authentication :
    //  "wlan.mnc<HomeMNC>.mcc<HomeMCC>.3gppnetwork.org!0<IMSI>@
    //  wlan.mnc<VisitedMNC>.mcc<VisitedMCC>.3gppnetwork.org"
    // EAP SIM authentication :
    //  "wlan.mnc<HomeMNC>.mcc<HomeMCC>.3gppnetwork.org!1<IMSI>@
    //  wlan.mnc<VisitedMNC>.mcc<VisitedMCC>.3gppnetwork.org"

    ISubscriberInfo* piSubsInfo =
            PhoneInfoService::GetPhoneInfoService()->GetSubscriberInfo(nSlotId);

    if (piSubsInfo == IMS_NULL)
    {
        return AString::ConstNull();
    }

    AString strIMSI;
    AString strMCC;
    AString strMNC;

    piSubsInfo->GetSubscriberId(strIMSI);
    piSubsInfo->GetMcc(strMCC);
    piSubsInfo->GetMnc(strMNC);

    if ((strIMSI.GetLength() == 0) || (strMCC.GetLength() == 0) || (strMNC.GetLength() == 0))
    {
        return AString::ConstNull();
    }

    if (strMNC.GetLength() == 2)
    {
        strMNC.Prepend('0');
    }

    AString strTIMPI;

    strTIMPI.Sprintf("%s@ims.mnc%s.mcc%s.3gppnetwork.org", strIMSI.GetStr(), strMNC.GetStr(),
            strMCC.GetStr());

    return strTIMPI;
}

/*

Remarks

*/
PUBLIC GLOBAL AString ImsIdentity::CreateTemporaryPublicUserId(
        IN IMS_SINT32 nSlotId /* = IMS_SLOT_0*/)
{
    // "sip:<IMSI>@ims.mnc<MNC>.mcc<MCC>.3gppnetwork.org"

    ISubscriberInfo* piSubsInfo =
            PhoneInfoService::GetPhoneInfoService()->GetSubscriberInfo(nSlotId);

    if (piSubsInfo == IMS_NULL)
    {
        return AString::ConstNull();
    }

    AString strIMSI;
    AString strMCC;
    AString strMNC;

    piSubsInfo->GetSubscriberId(strIMSI);
    piSubsInfo->GetMcc(strMCC);
    piSubsInfo->GetMnc(strMNC);

    if ((strIMSI.GetLength() == 0) || (strMCC.GetLength() == 0) || (strMNC.GetLength() == 0))
    {
        return AString::ConstNull();
    }

    if (strMNC.GetLength() == 2)
    {
        strMNC.Prepend('0');
    }

    AString strTIMPU;

    strTIMPU.Sprintf("sip:%s@ims.mnc%s.mcc%s.3gppnetwork.org", strIMSI.GetStr(), strMNC.GetStr(),
            strMCC.GetStr());

    return strTIMPU;
}

/*

Remarks

*/
PUBLIC GLOBAL const AString& ImsIdentity::GetAnonymousUserId()
{
    // "sip:anonymous@anonymous.invalid"
    static const AString ANONYMOUS_USER_ID("sip:anonymous@anonymous.invalid");

    return ANONYMOUS_USER_ID;
}

/*

Remarks

*/
PRIVATE GLOBAL const AString& ImsIdentity::GetHomeDomainName(
        IN IMS_SINT32 nSlotId /* = IMS_SLOT_0*/,
        IN const AString& strSubscriberId /* = AString::ConstNull() */)
{
    const SubscriberConfig* pSubsConfig = IMS_NULL;

    if (strSubscriberId.GetLength() == 0)
    {
        const AString& strId = SubscriberConfig::GetDefaultId();
        pSubsConfig = ConfigurationManager::GetInstance()->GetSubscriberConfig(strId, nSlotId);
    }
    else
    {
        pSubsConfig =
                ConfigurationManager::GetInstance()->GetSubscriberConfig(strSubscriberId, nSlotId);
    }

    if (pSubsConfig == IMS_NULL)
    {
        return AString::ConstNull();
    }

    return pSubsConfig->GetHomeDomainName();
}

/*

Remarks

*/
PUBLIC GLOBAL IMS_BOOL ImsIdentity::GetMccMnc(IN const AString& strPLMN, IN IMS_SINT32 nMncDigits,
        OUT AString& strMcc, OUT AString& strMnc, IN IMS_SINT32 nSlotId /* = IMS_SLOT_0*/)
{
    if (strPLMN.GetLength() == 0)
    {
        ISubscriberInfo* piSubsInfo =
                PhoneInfoService::GetPhoneInfoService()->GetSubscriberInfo(nSlotId);

        if (piSubsInfo == IMS_NULL)
        {
            return IMS_FALSE;
        }

        piSubsInfo->GetMcc(strMcc);
        piSubsInfo->GetMnc(strMnc);
    }
    else
    {
        if ((nMncDigits != 2) && (nMncDigits != 3))
        {
            // As default, assign 3
            nMncDigits = 3;
        }

        if (strPLMN.GetLength() < (nMncDigits + 3))
        {
            return IMS_FALSE;
        }

        strMcc = strPLMN.GetSubStr(0, 3);
        strMnc = strPLMN.GetSubStr(3, nMncDigits);
    }

    if ((strMcc.GetLength() == 0) || (strMnc.GetLength() == 0))
    {
        strMcc = AString::ConstNull();
        strMnc = AString::ConstNull();
        return IMS_FALSE;
    }

    return IMS_TRUE;
}

/*

Remarks

*/
PUBLIC GLOBAL const AString ImsIdentity::GetPhoneContext(IN IMS_SINT32 nDialingPolicy,
        IN IMS_SINT32 nSlotId /* = IMS_SLOT_0*/, IN AccessNetworkInfo* pANI /* = IMS_NULL */,
        IN const AString& strSubscriberId /* = AString::ConstNull() */)
{
    const SubscriberConfig* pSubsConfig = IMS_NULL;

    if (strSubscriberId.GetLength() == 0)
    {
        const AString& strId = SubscriberConfig::GetDefaultId();
        pSubsConfig = ConfigurationManager::GetInstance()->GetSubscriberConfig(strId, nSlotId);
    }
    else
    {
        pSubsConfig =
                ConfigurationManager::GetInstance()->GetSubscriberConfig(strSubscriberId, nSlotId);
    }

    if (pSubsConfig == IMS_NULL)
    {
        return AString::ConstNull();
    }

    const AString& strHDN = pSubsConfig->GetHomeDomainName();

    if (nDialingPolicy == DIALING_POLICY_GEO_LOCAL)
    {
        AString strPC(AString::ConstNull());

        if ((pANI == IMS_NULL) ||
                ((pANI != IMS_NULL) &&
                        (((pANI->nClass == AccessNetworkInfo::CLASS_NONE) &&
                                 (pANI->nType == AccessNetworkInfo::TYPE_NONE)) ||
                                ((pANI->nClass != AccessNetworkInfo::CLASS_NONE) &&
                                        (pANI->nType != AccessNetworkInfo::TYPE_NONE)))))
        {
            strPC.Sprintf("geo-local.%s", strHDN.GetStr());
        }
        else
        {
            if ((pANI->nClass == AccessNetworkInfo::CLASS_3GPP_E_UTRAN) ||
                    (pANI->nType == AccessNetworkInfo::TYPE_3GPP_E_UTRAN_FDD) ||
                    (pANI->nType == AccessNetworkInfo::TYPE_3GPP_E_UTRAN_TDD))
            {
                // EPS
                AString strMcc;
                AString strMnc;
                UTRAN_CELL_ID_3GPP* pAI = &(pANI->uniAI.utran_cell_id_3gpp);

                if (pAI->acUTRAN_CELL_ID[0] != '\0')
                {
                    AString strAI(pAI->acUTRAN_CELL_ID);
                    IMS_SINT32 nMncDigits =
                            (strAI.GetLength() == (ANI_3GPP_UTRAN_CELL_ID_MAX_TOTAL_LEN - 1)) ? 2
                                                                                              : 3;

                    GetMccMnc(strAI, nMncDigits, strMcc, strMnc, nSlotId);
                }
                else
                {
                    strMcc.Sprintf("%x%x%x", (pAI->aPLMNId[0] >> 4) & 0x0F, pAI->aPLMNId[0] & 0x0F,
                            (pAI->aPLMNId[1] >> 4) & 0x0F);

                    if ((pAI->aPLMNId[2] & 0x0F) == 0x0F)
                    {
                        strMnc.Sprintf(
                                "%x%x", pAI->aPLMNId[1] & 0x0F, (pAI->aPLMNId[2] >> 4) & 0x0F);
                    }
                    else
                    {
                        strMnc.Sprintf("%x%x%x", pAI->aPLMNId[1] & 0x0F,
                                (pAI->aPLMNId[2] >> 4) & 0x0F, pAI->aPLMNId[2] & 0x0F);
                    }
                }

                if (strMcc.Equals("000") && (strMnc.Equals("00") || strMnc.Equals("000")))
                {
                    GetMccMnc(AString::ConstNull(), 3, strMcc, strMnc, nSlotId);
                }

                if ((strMcc.GetLength() != 0) && (strMnc.GetLength() != 0))
                {
                    strPC.Sprintf(
                            "%s.%s.eps.%s", strMcc.GetStr(), strMnc.GetStr(), strHDN.GetStr());
                }
            }
            else if ((pANI->nClass == AccessNetworkInfo::CLASS_3GPP_NR) ||
                    (pANI->nType == AccessNetworkInfo::TYPE_3GPP_NR_FDD) ||
                    (pANI->nType == AccessNetworkInfo::TYPE_3GPP_NR_TDD))
            {
                // 5GS
                AString strMcc;
                AString strMnc;
                NR_UTRAN_CELL_ID_3GPP* pAI = &(pANI->uniAI.nr_utran_cell_id_3gpp);

                if (pAI->acUTRAN_CELL_ID[0] != '\0')
                {
                    AString strAI(pAI->acUTRAN_CELL_ID);
                    IMS_SINT32 nMncDigits =
                            (strAI.GetLength() == (ANI_3GPP_NR_UTRAN_CELL_ID_MAX_TOTAL_LEN - 1))
                            ? 2
                            : 3;

                    GetMccMnc(strAI, nMncDigits, strMcc, strMnc, nSlotId);
                }
                else
                {
                    strMcc.Sprintf("%x%x%x", (pAI->aPLMNId[0] >> 4) & 0x0F, pAI->aPLMNId[0] & 0x0F,
                            (pAI->aPLMNId[1] >> 4) & 0x0F);

                    if ((pAI->aPLMNId[2] & 0x0F) == 0x0F)
                    {
                        strMnc.Sprintf(
                                "%x%x", pAI->aPLMNId[1] & 0x0F, (pAI->aPLMNId[2] >> 4) & 0x0F);
                    }
                    else
                    {
                        strMnc.Sprintf("%x%x%x", pAI->aPLMNId[1] & 0x0F,
                                (pAI->aPLMNId[2] >> 4) & 0x0F, pAI->aPLMNId[2] & 0x0F);
                    }
                }

                if (strMcc.Equals("000") && (strMnc.Equals("00") || strMnc.Equals("000")))
                {
                    GetMccMnc(AString::ConstNull(), 3, strMcc, strMnc, nSlotId);
                }

                if ((strMcc.GetLength() != 0) && (strMnc.GetLength() != 0))
                {
                    strPC.Sprintf(
                            "%s.%s.5gs.%s", strMcc.GetStr(), strMnc.GetStr(), strHDN.GetStr());
                }
            }
            else if ((pANI->nClass == AccessNetworkInfo::CLASS_3GPP_WLAN) ||
                    (pANI->nType == AccessNetworkInfo::TYPE_IEEE_802_11) ||
                    (pANI->nType == AccessNetworkInfo::TYPE_IEEE_802_11A) ||
                    (pANI->nType == AccessNetworkInfo::TYPE_IEEE_802_11B) ||
                    (pANI->nType == AccessNetworkInfo::TYPE_IEEE_802_11G) ||
                    (pANI->nType == AccessNetworkInfo::TYPE_IEEE_802_11N))
            {
                // I-WLAN
                AString strMac;
                I_WLAN_NODE_ID* pAI = &(pANI->uniAI.i_wlan_node_id);

                if (pAI->acMAC[0] != '\0')
                {
                    strMac = pAI->acMAC;
                }
                else
                {
                    strMac.Sprintf("%02x%02x%02x%02x%02x%02x", pAI->aMAC[0], pAI->aMAC[1],
                            pAI->aMAC[2], pAI->aMAC[3], pAI->aMAC[4], pAI->aMAC[5]);
                }

                AString strSsid(pANI->uniAI.i_wlan_node_id.acSSID);

                strSsid = strSsid.MakeLower();

                if ((strMac.GetLength() != 0) && (strSsid.GetLength() != 0))
                {
                    strPC.Sprintf(
                            "%s.%s.i-wlan.%s", strSsid.GetStr(), strMac.GetStr(), strHDN.GetStr());
                }
            }
            else if ((pANI->nType == AccessNetworkInfo::TYPE_3GPP2_1X) ||
                    (pANI->nType == AccessNetworkInfo::TYPE_3GPP2_1X_HRPD) ||
                    (pANI->nType == AccessNetworkInfo::TYPE_3GPP2_UMB))
            {
                // CDMA2000
                // FIXME: I don't know what is the subnet id...
            }
            else if ((pANI->nClass == AccessNetworkInfo::CLASS_3GPP_GERAN) ||
                    (pANI->nClass == AccessNetworkInfo::CLASS_3GPP_UTRAN) ||
                    (pANI->nType == AccessNetworkInfo::TYPE_3GPP_GERAN) ||
                    (pANI->nType == AccessNetworkInfo::TYPE_3GPP_UTRAN_FDD) ||
                    (pANI->nType == AccessNetworkInfo::TYPE_3GPP_UTRAN_TDD))
            {
                // GPRS
                AString strMcc;
                AString strMnc;

                if ((pANI->nClass == AccessNetworkInfo::CLASS_3GPP_GERAN) ||
                        (pANI->nType == AccessNetworkInfo::TYPE_3GPP_GERAN))
                {
                    CGI_3GPP* pAI = &(pANI->uniAI.cgi_3gpp);

                    if (pAI->acCGI[0] != '\0')
                    {
                        AString strAI(pAI->acCGI);
                        IMS_SINT32 nMncDigits =
                                (strAI.GetLength() == (ANI_3GPP_CGI_MAX_TOTAL_LEN - 1)) ? 2 : 3;

                        GetMccMnc(strAI, nMncDigits, strMcc, strMnc, nSlotId);
                    }
                    else
                    {
                        strMcc.Sprintf("%x%x%x", (pAI->aPLMNId[0] >> 4) & 0x0F,
                                pAI->aPLMNId[0] & 0x0F, (pAI->aPLMNId[1] >> 4) & 0x0F);

                        if ((pAI->aPLMNId[2] & 0x0F) == 0x0F)
                        {
                            strMnc.Sprintf(
                                    "%x%x", pAI->aPLMNId[1] & 0x0F, (pAI->aPLMNId[2] >> 4) & 0x0F);
                        }
                        else
                        {
                            strMnc.Sprintf("%x%x%x", pAI->aPLMNId[1] & 0x0F,
                                    (pAI->aPLMNId[2] >> 4) & 0x0F, pAI->aPLMNId[2] & 0x0F);
                        }
                    }
                }
                else
                {
                    UTRAN_CELL_ID_3GPP* pAI = &(pANI->uniAI.utran_cell_id_3gpp);

                    if (pAI->acUTRAN_CELL_ID[0] != '\0')
                    {
                        AString strAI(pAI->acUTRAN_CELL_ID);
                        IMS_SINT32 nMncDigits =
                                (strAI.GetLength() == (ANI_3GPP_UTRAN_CELL_ID_MAX_TOTAL_LEN - 1))
                                ? 2
                                : 3;

                        GetMccMnc(strAI, nMncDigits, strMcc, strMnc, nSlotId);
                    }
                    else
                    {
                        strMcc.Sprintf("%x%x%x", (pAI->aPLMNId[0] >> 4) & 0x0F,
                                pAI->aPLMNId[0] & 0x0F, (pAI->aPLMNId[1] >> 4) & 0x0F);

                        if ((pAI->aPLMNId[2] & 0x0F) == 0x0F)
                        {
                            strMnc.Sprintf(
                                    "%x%x", pAI->aPLMNId[1] & 0x0F, (pAI->aPLMNId[2] >> 4) & 0x0F);
                        }
                        else
                        {
                            strMnc.Sprintf("%x%x%x", pAI->aPLMNId[1] & 0x0F,
                                    (pAI->aPLMNId[2] >> 4) & 0x0F, pAI->aPLMNId[2] & 0x0F);
                        }
                    }
                }

                if (strMcc.Equals("000") && (strMnc.Equals("00") || strMnc.Equals("000")))
                {
                    GetMccMnc(AString::ConstNull(), 3, strMcc, strMnc, nSlotId);
                }

                if ((strMcc.GetLength() != 0) && (strMnc.GetLength() != 0))
                {
                    strPC.Sprintf(
                            "%s.%s.gprs.%s", strMcc.GetStr(), strMnc.GetStr(), strHDN.GetStr());
                }
            }

            if (strPC.GetLength() == 0)
            {
                strPC.Sprintf("geo-local.%s", strHDN.GetStr());
            }
        }

        return strPC;
    }
    else
    {
        const AString& strPC = pSubsConfig->GetPhoneContext();
        return (strPC.GetLength() > 0) ? strPC : strHDN;
    }
}

/*

Remarks

*/
PUBLIC GLOBAL const AString& ImsIdentity::GetUnavailableUserId()
{
    // "sip:unavailable@unknown.invalid"
    static const AString UNAVAILABLE_USER_ID("sip:unavailable@unknown.invalid");

    return UNAVAILABLE_USER_ID;
}
