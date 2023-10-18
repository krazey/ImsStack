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
#include "AStringBuffer.h"
#include "ImsAccessNetworkInfoType.h"
#include "ServiceMemory.h"
#include "ServicePhoneInfo.h"

#include "ImsIdentity.h"
#include "private/ConfigurationManager.h"
#include "private/SubscriberConfig.h"

PUBLIC GLOBAL AString ImsIdentity::CreateSipUserId(
        IN IMS_SINT32 nSlotId, IN IMS_BOOL bUserPhoneParam /*= IMS_FALSE*/)
{
    // "sip:<global number>@<home domain name>;user=phone" ; it will be derived from the MSISDN
    ISubscriberInfo* piSubsInfo =
            PhoneInfoService::GetPhoneInfoService()->GetSubscriberInfo(nSlotId);

    if (piSubsInfo == IMS_NULL)
    {
        return AString::ConstNull();
    }

    AString strMsisdn;

    piSubsInfo->GetPhoneNumber(strMsisdn);

    return CreateSipUserId(strMsisdn, nSlotId, bUserPhoneParam);
}

PUBLIC GLOBAL AString ImsIdentity::CreateSipUserId(IN const AString& strDialString,
        IN IMS_SINT32 nSlotId, IN IMS_BOOL bUserPhoneParam /*= IMS_FALSE*/,
        IN const AString& strPhoneContext /*= AString::ConstNull()*/)
{
    // "sip:<global number>@<home domain name>;user=phone" ;
    // It will be derived from the specified number

    if (strDialString.GetLength() == 0)
    {
        return CreateSipUserId(nSlotId);
    }

    AStringBuffer objUri(64);

    objUri.Append("sip:");
    objUri.Append(strDialString);

    if (strDialString.Contains('#'))
    {
        objUri.Replace('#', "%23");
    }

    objUri.Append('@');
    objUri.Append(
            (strPhoneContext.GetLength() == 0) ? GetHomeDomainName(nSlotId) : strPhoneContext);

    if (bUserPhoneParam)
    {
        objUri.Append(";user=phone");
    }

    return static_cast<const AStringBuffer&>(objUri).GetString();
}

PUBLIC GLOBAL AString ImsIdentity::CreateSipUserIdWithDialString(IN const AString& strDialString,
        IN IMS_SINT32 nSlotId, IN const AString& strPhoneContext /*= AString::ConstNull()*/)
{
    // "sip:<dialstring>;phone-context=<home domain name>@<home domain name>;user=dialstring";

    if (strDialString.GetLength() == 0)
    {
        return AString::ConstNull();
    }

    AStringBuffer objUri(64);

    objUri.Append("sip:");
    objUri.Append(strDialString);

    if (strDialString.Contains('#'))
    {
        objUri.Replace('#', "%23");
    }

    objUri.Append(";phone-context=");

    if (strPhoneContext.GetLength() == 0)
    {
        const AString strDefaultPc = GetPhoneContext(DIALING_POLICY_HOME_LOCAL, nSlotId);
        objUri.Append(strDefaultPc);
    }
    else
    {
        objUri.Append(strPhoneContext);
    }

    objUri.Append('@');
    objUri.Append(GetHomeDomainName(nSlotId));
    objUri.Append(";user=dialstring");

    return static_cast<const AStringBuffer&>(objUri).GetString();
}

PUBLIC GLOBAL AString ImsIdentity::CreateSipUserIdWithPhone(IN const AString& strDialString,
        IN IMS_SINT32 nSlotId, IN const AString& strPhoneContext /*= AString::ConstNull()*/)
{
    // "sip:<dialstring>;phone-context=<home domain name>@<home domain name>;user=phone";

    if (strDialString.GetLength() == 0)
    {
        return AString::ConstNull();
    }

    AStringBuffer objUri(64);

    objUri.Append("sip:");
    objUri.Append(strDialString);

    if (strDialString.Contains('#'))
    {
        objUri.Replace('#', "%23");
    }

    objUri.Append(";phone-context=");

    if (strPhoneContext.GetLength() == 0)
    {
        const AString strDefaultPc = GetPhoneContext(DIALING_POLICY_HOME_LOCAL, nSlotId);
        objUri.Append(strDefaultPc);
    }
    else
    {
        objUri.Append(strPhoneContext);
    }

    objUri.Append('@');
    objUri.Append(GetHomeDomainName(nSlotId));
    objUri.Append(";user=phone");

    return static_cast<const AStringBuffer&>(objUri).GetString();
}

PUBLIC GLOBAL AString ImsIdentity::CreateTelUserId(
        IN const AString& strPhoneContext, IN IMS_SINT32 nSlotId)
{
    // "tel:<global number>" ; it will be derived from the MSISDN
    ISubscriberInfo* piSubsInfo =
            PhoneInfoService::GetPhoneInfoService()->GetSubscriberInfo(nSlotId);

    if (piSubsInfo == IMS_NULL)
    {
        return AString::ConstNull();
    }

    AString strMsisdn;

    piSubsInfo->GetPhoneNumber(strMsisdn);

    return CreateTelUserId(strMsisdn, strPhoneContext, nSlotId);
}

PUBLIC GLOBAL AString ImsIdentity::CreateTelUserId(
        IN const AString& strDialString, IN const AString& strPhoneContext, IN IMS_SINT32 nSlotId)
{
    // "tel:<global number>" ; it will be derived from the MSISDN

    if (strDialString.GetLength() == 0)
    {
        return CreateTelUserId(strPhoneContext, nSlotId);
    }

    AStringBuffer objUri(64);

    objUri.Append("tel:");
    objUri.Append(strDialString);

    if (!strDialString.StartsWith('+'))
    {
        objUri.Append(";phone-context=");

        if (strPhoneContext.GetLength() == 0)
        {
            const AString strDefaultPc = GetPhoneContext(DIALING_POLICY_HOME_LOCAL, nSlotId);
            objUri.Append(strDefaultPc);
        }
        else
        {
            objUri.Append(strPhoneContext);
        }
    }

    return static_cast<const AStringBuffer&>(objUri).GetString();
}

PUBLIC GLOBAL AString ImsIdentity::CreateTemporaryHomeDomainName(IN IMS_SINT32 nSlotId)
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

    AString strMcc;
    AString strMnc;

    piSubsInfo->GetSimMcc(strMcc);
    piSubsInfo->GetSimMnc(strMnc);

    if ((strMcc.GetLength() == 0) || (strMnc.GetLength() == 0))
    {
        return AString::ConstNull();
    }

    if (strMnc.GetLength() == 2)
    {
        strMnc.Prepend('0');
    }

    AString strHdn;

    strHdn.Sprintf("ims.mnc%s.mcc%s.3gppnetwork.org", strMnc.GetStr(), strMcc.GetStr());

    return strHdn;
}

PUBLIC GLOBAL AString ImsIdentity::CreateTemporaryPrivateUserId(IN IMS_SINT32 nSlotId)
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

    AString strImsi;
    AString strMcc;
    AString strMnc;

    piSubsInfo->GetSubscriberId(strImsi);
    piSubsInfo->GetSimMcc(strMcc);
    piSubsInfo->GetSimMnc(strMnc);

    if ((strImsi.GetLength() == 0) || (strMcc.GetLength() == 0) || (strMnc.GetLength() == 0))
    {
        return AString::ConstNull();
    }

    if (strMnc.GetLength() == 2)
    {
        strMnc.Prepend('0');
    }

    AString strImpi;

    strImpi.Sprintf("%s@ims.mnc%s.mcc%s.3gppnetwork.org", strImsi.GetStr(), strMnc.GetStr(),
            strMcc.GetStr());

    return strImpi;
}

PUBLIC GLOBAL AString ImsIdentity::CreateTemporaryPublicUserId(IN IMS_SINT32 nSlotId)
{
    // "sip:<IMSI>@ims.mnc<MNC>.mcc<MCC>.3gppnetwork.org"

    ISubscriberInfo* piSubsInfo =
            PhoneInfoService::GetPhoneInfoService()->GetSubscriberInfo(nSlotId);

    if (piSubsInfo == IMS_NULL)
    {
        return AString::ConstNull();
    }

    AString strImsi;
    AString strMcc;
    AString strMnc;

    piSubsInfo->GetSubscriberId(strImsi);
    piSubsInfo->GetSimMcc(strMcc);
    piSubsInfo->GetSimMnc(strMnc);

    if ((strImsi.GetLength() == 0) || (strMcc.GetLength() == 0) || (strMnc.GetLength() == 0))
    {
        return AString::ConstNull();
    }

    if (strMnc.GetLength() == 2)
    {
        strMnc.Prepend('0');
    }

    AString strImpu;

    strImpu.Sprintf("sip:%s@ims.mnc%s.mcc%s.3gppnetwork.org", strImsi.GetStr(), strMnc.GetStr(),
            strMcc.GetStr());

    return strImpu;
}

PUBLIC GLOBAL const AString& ImsIdentity::GetAnonymousUserId()
{
    // "sip:anonymous@anonymous.invalid"
    static const AString ANONYMOUS_USER_ID("sip:anonymous@anonymous.invalid");

    return ANONYMOUS_USER_ID;
}

PRIVATE GLOBAL const AString& ImsIdentity::GetHomeDomainName(
        IN IMS_SINT32 nSlotId, IN const AString& strSubscriberId /*= AString::ConstNull()*/)
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

PUBLIC GLOBAL IMS_BOOL ImsIdentity::GetMccMnc(IN const AString& strPlmn, IN IMS_SINT32 nMncDigits,
        OUT AString& strMcc, OUT AString& strMnc, IN IMS_SINT32 nSlotId)
{
    if (strPlmn.GetLength() == 0)
    {
        ISubscriberInfo* piSubsInfo =
                PhoneInfoService::GetPhoneInfoService()->GetSubscriberInfo(nSlotId);

        if (piSubsInfo == IMS_NULL)
        {
            return IMS_FALSE;
        }

        piSubsInfo->GetSimMcc(strMcc);
        piSubsInfo->GetSimMnc(strMnc);
    }
    else
    {
        if ((nMncDigits != 2) && (nMncDigits != 3))
        {
            // As default, assign 3
            nMncDigits = 3;
        }

        if (strPlmn.GetLength() < (nMncDigits + 3))
        {
            return IMS_FALSE;
        }

        strMcc = strPlmn.GetSubStr(0, 3);
        strMnc = strPlmn.GetSubStr(3, nMncDigits);
    }

    if ((strMcc.GetLength() == 0) || (strMnc.GetLength() == 0))
    {
        strMcc = AString::ConstNull();
        strMnc = AString::ConstNull();
        return IMS_FALSE;
    }

    return IMS_TRUE;
}

PUBLIC GLOBAL const AString ImsIdentity::GetPhoneContext(IN IMS_SINT32 nDialingPolicy,
        IN IMS_SINT32 nSlotId, IN AccessNetworkInfo* pAni /*= IMS_NULL*/,
        IN const AString& strSubscriberId /*= AString::ConstNull()*/)
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

    const AString& strHdn = pSubsConfig->GetHomeDomainName();

    if (nDialingPolicy == DIALING_POLICY_GEO_LOCAL)
    {
        AString strPc(AString::ConstNull());

        if ((pAni == IMS_NULL) ||
                ((pAni->nClass == AccessNetworkInfo::CLASS_NONE) &&
                        (pAni->nType == AccessNetworkInfo::TYPE_NONE)) ||
                ((pAni->nClass != AccessNetworkInfo::CLASS_NONE) &&
                        (pAni->nType != AccessNetworkInfo::TYPE_NONE)))
        {
            strPc.Sprintf("geo-local.%s", strHdn.GetStr());
        }
        else
        {
            if ((pAni->nClass == AccessNetworkInfo::CLASS_3GPP_E_UTRAN) ||
                    (pAni->nType == AccessNetworkInfo::TYPE_3GPP_E_UTRAN_FDD) ||
                    (pAni->nType == AccessNetworkInfo::TYPE_3GPP_E_UTRAN_TDD))
            {
                // EPS
                AString strMcc;
                AString strMnc;
                UTRAN_CELL_ID_3GPP* pAi = &(pAni->uniAI.utran_cell_id_3gpp);

                if (pAi->acUTRAN_CELL_ID[0] != '\0')
                {
                    AString strAi(pAi->acUTRAN_CELL_ID);
                    IMS_SINT32 nMncDigits =
                            (strAi.GetLength() == (ANI_3GPP_UTRAN_CELL_ID_MAX_TOTAL_LEN - 1)) ? 2
                                                                                              : 3;

                    GetMccMnc(strAi, nMncDigits, strMcc, strMnc, nSlotId);
                }
                else
                {
                    strMcc.Sprintf("%x%x%x", (pAi->aPLMNId[0] >> 4) & 0x0F, pAi->aPLMNId[0] & 0x0F,
                            (pAi->aPLMNId[1] >> 4) & 0x0F);

                    if ((pAi->aPLMNId[2] & 0x0F) == 0x0F)
                    {
                        strMnc.Sprintf(
                                "%x%x", pAi->aPLMNId[1] & 0x0F, (pAi->aPLMNId[2] >> 4) & 0x0F);
                    }
                    else
                    {
                        strMnc.Sprintf("%x%x%x", pAi->aPLMNId[1] & 0x0F,
                                (pAi->aPLMNId[2] >> 4) & 0x0F, pAi->aPLMNId[2] & 0x0F);
                    }
                }

                if (strMcc.Equals("000") && (strMnc.Equals("00") || strMnc.Equals("000")))
                {
                    GetMccMnc(AString::ConstNull(), 3, strMcc, strMnc, nSlotId);
                }

                if ((strMcc.GetLength() != 0) && (strMnc.GetLength() != 0))
                {
                    strPc.Sprintf(
                            "%s.%s.eps.%s", strMcc.GetStr(), strMnc.GetStr(), strHdn.GetStr());
                }
            }
            else if ((pAni->nClass == AccessNetworkInfo::CLASS_3GPP_NR) ||
                    (pAni->nType == AccessNetworkInfo::TYPE_3GPP_NR_FDD) ||
                    (pAni->nType == AccessNetworkInfo::TYPE_3GPP_NR_TDD))
            {
                // 5GS
                AString strMcc;
                AString strMnc;
                NR_UTRAN_CELL_ID_3GPP* pAi = &(pAni->uniAI.nr_utran_cell_id_3gpp);

                if (pAi->acUTRAN_CELL_ID[0] != '\0')
                {
                    AString strAi(pAi->acUTRAN_CELL_ID);
                    IMS_SINT32 nMncDigits =
                            (strAi.GetLength() == (ANI_3GPP_NR_UTRAN_CELL_ID_MAX_TOTAL_LEN - 1))
                            ? 2
                            : 3;

                    GetMccMnc(strAi, nMncDigits, strMcc, strMnc, nSlotId);
                }
                else
                {
                    strMcc.Sprintf("%x%x%x", (pAi->aPLMNId[0] >> 4) & 0x0F, pAi->aPLMNId[0] & 0x0F,
                            (pAi->aPLMNId[1] >> 4) & 0x0F);

                    if ((pAi->aPLMNId[2] & 0x0F) == 0x0F)
                    {
                        strMnc.Sprintf(
                                "%x%x", pAi->aPLMNId[1] & 0x0F, (pAi->aPLMNId[2] >> 4) & 0x0F);
                    }
                    else
                    {
                        strMnc.Sprintf("%x%x%x", pAi->aPLMNId[1] & 0x0F,
                                (pAi->aPLMNId[2] >> 4) & 0x0F, pAi->aPLMNId[2] & 0x0F);
                    }
                }

                if (strMcc.Equals("000") && (strMnc.Equals("00") || strMnc.Equals("000")))
                {
                    GetMccMnc(AString::ConstNull(), 3, strMcc, strMnc, nSlotId);
                }

                if ((strMcc.GetLength() != 0) && (strMnc.GetLength() != 0))
                {
                    strPc.Sprintf(
                            "%s.%s.5gs.%s", strMcc.GetStr(), strMnc.GetStr(), strHdn.GetStr());
                }
            }
            else if ((pAni->nClass == AccessNetworkInfo::CLASS_3GPP_WLAN) ||
                    (pAni->nType == AccessNetworkInfo::TYPE_IEEE_802_11) ||
                    (pAni->nType == AccessNetworkInfo::TYPE_IEEE_802_11A) ||
                    (pAni->nType == AccessNetworkInfo::TYPE_IEEE_802_11B) ||
                    (pAni->nType == AccessNetworkInfo::TYPE_IEEE_802_11G) ||
                    (pAni->nType == AccessNetworkInfo::TYPE_IEEE_802_11N))
            {
                // I-WLAN
                AString strMac;
                I_WLAN_NODE_ID* pAi = &(pAni->uniAI.i_wlan_node_id);

                if (pAi->acMAC[0] != '\0')
                {
                    strMac = pAi->acMAC;
                }
                else
                {
                    strMac.Sprintf("%02x%02x%02x%02x%02x%02x", pAi->aMAC[0], pAi->aMAC[1],
                            pAi->aMAC[2], pAi->aMAC[3], pAi->aMAC[4], pAi->aMAC[5]);
                }

                AString strSsid(pAni->uniAI.i_wlan_node_id.acSSID);

                strSsid = strSsid.MakeLower();

                if ((strMac.GetLength() != 0) && (strSsid.GetLength() != 0))
                {
                    strPc.Sprintf(
                            "%s.%s.i-wlan.%s", strSsid.GetStr(), strMac.GetStr(), strHdn.GetStr());
                }
            }
            else if ((pAni->nType == AccessNetworkInfo::TYPE_3GPP2_1X) ||
                    (pAni->nType == AccessNetworkInfo::TYPE_3GPP2_1X_HRPD) ||
                    (pAni->nType == AccessNetworkInfo::TYPE_3GPP2_UMB))
            {
                // CDMA2000
                // FIXME: I don't know what is the subnet id...
            }
            else if ((pAni->nClass == AccessNetworkInfo::CLASS_3GPP_GERAN) ||
                    (pAni->nClass == AccessNetworkInfo::CLASS_3GPP_UTRAN) ||
                    (pAni->nType == AccessNetworkInfo::TYPE_3GPP_GERAN) ||
                    (pAni->nType == AccessNetworkInfo::TYPE_3GPP_UTRAN_FDD) ||
                    (pAni->nType == AccessNetworkInfo::TYPE_3GPP_UTRAN_TDD))
            {
                // GPRS
                AString strMcc;
                AString strMnc;

                if ((pAni->nClass == AccessNetworkInfo::CLASS_3GPP_GERAN) ||
                        (pAni->nType == AccessNetworkInfo::TYPE_3GPP_GERAN))
                {
                    CGI_3GPP* pAi = &(pAni->uniAI.cgi_3gpp);

                    if (pAi->acCGI[0] != '\0')
                    {
                        AString strAi(pAi->acCGI);
                        IMS_SINT32 nMncDigits =
                                (strAi.GetLength() == (ANI_3GPP_CGI_MAX_TOTAL_LEN - 1)) ? 2 : 3;

                        GetMccMnc(strAi, nMncDigits, strMcc, strMnc, nSlotId);
                    }
                    else
                    {
                        strMcc.Sprintf("%x%x%x", (pAi->aPLMNId[0] >> 4) & 0x0F,
                                pAi->aPLMNId[0] & 0x0F, (pAi->aPLMNId[1] >> 4) & 0x0F);

                        if ((pAi->aPLMNId[2] & 0x0F) == 0x0F)
                        {
                            strMnc.Sprintf(
                                    "%x%x", pAi->aPLMNId[1] & 0x0F, (pAi->aPLMNId[2] >> 4) & 0x0F);
                        }
                        else
                        {
                            strMnc.Sprintf("%x%x%x", pAi->aPLMNId[1] & 0x0F,
                                    (pAi->aPLMNId[2] >> 4) & 0x0F, pAi->aPLMNId[2] & 0x0F);
                        }
                    }
                }
                else
                {
                    UTRAN_CELL_ID_3GPP* pAi = &(pAni->uniAI.utran_cell_id_3gpp);

                    if (pAi->acUTRAN_CELL_ID[0] != '\0')
                    {
                        AString strAi(pAi->acUTRAN_CELL_ID);
                        IMS_SINT32 nMncDigits =
                                (strAi.GetLength() == (ANI_3GPP_UTRAN_CELL_ID_MAX_TOTAL_LEN - 1))
                                ? 2
                                : 3;

                        GetMccMnc(strAi, nMncDigits, strMcc, strMnc, nSlotId);
                    }
                    else
                    {
                        strMcc.Sprintf("%x%x%x", (pAi->aPLMNId[0] >> 4) & 0x0F,
                                pAi->aPLMNId[0] & 0x0F, (pAi->aPLMNId[1] >> 4) & 0x0F);

                        if ((pAi->aPLMNId[2] & 0x0F) == 0x0F)
                        {
                            strMnc.Sprintf(
                                    "%x%x", pAi->aPLMNId[1] & 0x0F, (pAi->aPLMNId[2] >> 4) & 0x0F);
                        }
                        else
                        {
                            strMnc.Sprintf("%x%x%x", pAi->aPLMNId[1] & 0x0F,
                                    (pAi->aPLMNId[2] >> 4) & 0x0F, pAi->aPLMNId[2] & 0x0F);
                        }
                    }
                }

                if (strMcc.Equals("000") && (strMnc.Equals("00") || strMnc.Equals("000")))
                {
                    GetMccMnc(AString::ConstNull(), 3, strMcc, strMnc, nSlotId);
                }

                if ((strMcc.GetLength() != 0) && (strMnc.GetLength() != 0))
                {
                    strPc.Sprintf(
                            "%s.%s.gprs.%s", strMcc.GetStr(), strMnc.GetStr(), strHdn.GetStr());
                }
            }

            if (strPc.GetLength() == 0)
            {
                strPc.Sprintf("geo-local.%s", strHdn.GetStr());
            }
        }

        return strPc;
    }
    else
    {
        const AString& strPc = pSubsConfig->GetPhoneContext();
        return (strPc.GetLength() > 0) ? strPc : strHdn;
    }
}

PUBLIC GLOBAL const AString& ImsIdentity::GetUnavailableUserId()
{
    // "sip:unavailable@unknown.invalid"
    static const AString UNAVAILABLE_USER_ID("sip:unavailable@unknown.invalid");

    return UNAVAILABLE_USER_ID;
}
