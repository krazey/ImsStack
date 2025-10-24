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
#include "AccessNetworkInfoFormatter.h"
#include "AStringBuffer.h"
#include "ServiceMemory.h"
#include "ServiceTrace.h"

__IMS_TRACE_TAG_BASE__;

static const IMS_CHAR* ACCESS_CLASS[] = {
        "",
        "3GPP-GERAN",
        "3GPP-UTRAN",
        "3GPP-E-UTRAN",
        "3GPP-NR",
        "3GPP-WLAN",
        "3GPP-GAN",
        "3GPP-HSPA",
};

static const IMS_CHAR* ACCESS_TYPE[] = {
        "",
        // 3GPP
        "3GPP-GERAN",
        "3GPP-UTRAN-FDD",
        "3GPP-UTRAN-TDD",
        "3GPP-E-UTRAN-FDD",
        "3GPP-E-UTRAN-TDD",
        "3GPP-NR-FDD",
        "3GPP-NR-TDD",

        // 3GPP2
        "3GPP2-1X",
        "3GPP2-1X-HRPD",
        "3GPP2-UMB",

        // WIFI
        "IEEE-802.11",
        "IEEE-802.11a",
        "IEEE-802.11b",
        "IEEE-802.11g",
        "IEEE-802.11n",

        "DOCSIS",
};

static void GetAccessInfo(
        IN const CGI_3GPP& objAi, IN const AString& strCellInfo, OUT AStringBuffer& objBuffer)
{
    if (strCellInfo.GetLength() > 0)
    {
        objBuffer.Sprintf("cgi-3gpp=%s", strCellInfo.GetStr());
        return;
    }

    if (objAi.acCGI[0] != '\0')
    {
        objBuffer.Sprintf("cgi-3gpp=%s", &(objAi.acCGI[0]));
    }
    else
    {
        if ((objAi.aPLMNId[2] & 0x0F) == 0x0F)
        {
            // mnc is 2digits
            objBuffer.Sprintf("cgi-3gpp=%x%x%x%x%x%02x%02x%02x%02x", (objAi.aPLMNId[0] >> 4) & 0x0F,
                    objAi.aPLMNId[0] & 0x0F, (objAi.aPLMNId[1] >> 4) & 0x0F,
                    objAi.aPLMNId[1] & 0x0F, (objAi.aPLMNId[2] >> 4) & 0x0F, objAi.aLAC[0],
                    objAi.aLAC[1], objAi.aCI[0], objAi.aCI[1]);
        }
        else
        {
            objBuffer.Sprintf("cgi-3gpp=%x%x%x%x%x%x%02x%02x%02x%02x",
                    (objAi.aPLMNId[0] >> 4) & 0x0F, objAi.aPLMNId[0] & 0x0F,
                    (objAi.aPLMNId[1] >> 4) & 0x0F, objAi.aPLMNId[1] & 0x0F,
                    (objAi.aPLMNId[2] >> 4) & 0x0F, objAi.aPLMNId[2] & 0x0F, objAi.aLAC[0],
                    objAi.aLAC[1], objAi.aCI[0], objAi.aCI[1]);
        }
    }
}

static void GetAccessInfo(IN const UTRAN_CELL_ID_3GPP& objAi, IN const AString& strCellInfo,
        OUT AStringBuffer& objBuffer)
{
    if (strCellInfo.GetLength() > 0)
    {
        objBuffer.Sprintf("utran-cell-id-3gpp=%s", strCellInfo.GetStr());
        return;
    }

    if (objAi.acUTRAN_CELL_ID[0] != '\0')
    {
        objBuffer.Sprintf("utran-cell-id-3gpp=%s", &(objAi.acUTRAN_CELL_ID[0]));
    }
    else
    {
        if ((objAi.aPLMNId[2] & 0x0F) == 0x0F)
        {
            // mnc is 2digits
            objBuffer.Sprintf("utran-cell-id-3gpp=%x%x%x%x%x%02x%02x%02x%02x%02x%x",
                    (objAi.aPLMNId[0] >> 4) & 0x0F, objAi.aPLMNId[0] & 0x0F,
                    (objAi.aPLMNId[1] >> 4) & 0x0F, objAi.aPLMNId[1] & 0x0F,
                    (objAi.aPLMNId[2] >> 4) & 0x0F, objAi.aLACorTAC[0], objAi.aLACorTAC[1],
                    objAi.aCellId[0], objAi.aCellId[1], objAi.aCellId[2],
                    (objAi.aCellId[3] & 0x0F));
        }
        else
        {
            objBuffer.Sprintf("utran-cell-id-3gpp=%x%x%x%x%x%x%02x%02x%02x%02x%02x%x",
                    (objAi.aPLMNId[0] >> 4) & 0x0F, objAi.aPLMNId[0] & 0x0F,
                    (objAi.aPLMNId[1] >> 4) & 0x0F, objAi.aPLMNId[1] & 0x0F,
                    (objAi.aPLMNId[2] >> 4) & 0x0F, objAi.aPLMNId[2] & 0x0F, objAi.aLACorTAC[0],
                    objAi.aLACorTAC[1], objAi.aCellId[0], objAi.aCellId[1], objAi.aCellId[2],
                    (objAi.aCellId[3] & 0x0F));
        }
    }
}

static void GetAccessInfo(IN const NR_UTRAN_CELL_ID_3GPP& objAi, IN const AString& strCellInfo,
        OUT AStringBuffer& objBuffer)
{
    if (strCellInfo.GetLength() > 0)
    {
        objBuffer.Sprintf("utran-cell-id-3gpp=%s", strCellInfo.GetStr());
        return;
    }

    if (objAi.acUTRAN_CELL_ID[0] != '\0')
    {
        objBuffer.Sprintf("utran-cell-id-3gpp=%s", &(objAi.acUTRAN_CELL_ID[0]));
    }
    else
    {
        if ((objAi.aPLMNId[2] & 0x0F) == 0x0F)
        {
            // mnc is 2digits
            objBuffer.Sprintf("utran-cell-id-3gpp=%x%x%x%x%x%02x%02x%02x%02x%02x%02x%02x%x",
                    (objAi.aPLMNId[0] >> 4) & 0x0F, objAi.aPLMNId[0] & 0x0F,
                    (objAi.aPLMNId[1] >> 4) & 0x0F, objAi.aPLMNId[1] & 0x0F,
                    (objAi.aPLMNId[2] >> 4) & 0x0F, objAi.aTAC[0], objAi.aTAC[1], objAi.aTAC[2],
                    objAi.aCellId[0], objAi.aCellId[1], objAi.aCellId[2], objAi.aCellId[3],
                    (objAi.aCellId[4] & 0x0F));
        }
        else
        {
            objBuffer.Sprintf("utran-cell-id-3gpp=%x%x%x%x%x%x%02x%02x%02x%02x%02x%02x%02x%x",
                    (objAi.aPLMNId[0] >> 4) & 0x0F, objAi.aPLMNId[0] & 0x0F,
                    (objAi.aPLMNId[1] >> 4) & 0x0F, objAi.aPLMNId[1] & 0x0F,
                    (objAi.aPLMNId[2] >> 4) & 0x0F, objAi.aPLMNId[2] & 0x0F, objAi.aTAC[0],
                    objAi.aTAC[1], objAi.aTAC[2], objAi.aCellId[0], objAi.aCellId[1],
                    objAi.aCellId[2], objAi.aCellId[3], (objAi.aCellId[4] & 0x0F));
        }
    }
}

static void GetAccessInfo(
        IN const CI_3GPP2& objAi, IN const AString& strCellInfo, OUT AStringBuffer& objBuffer)
{
    if (strCellInfo.GetLength() > 0)
    {
        objBuffer.Sprintf("ci-3gpp2=%s", strCellInfo.GetStr());
        return;
    }

    if (objAi.acCI[0] != '\0')
    {
        objBuffer.Sprintf("ci-3gpp2=%s", &(objAi.acCI[0]));
    }
    else
    {
        AString strHex;

        objBuffer.Append("ci-3gpp2=");

        // sector id
        for (IMS_SINT32 i = 0; i < 16; ++i)
        {
            strHex.Sprintf("%02X", objAi.aSectorId[i]);
            objBuffer.Append(strHex);
        }

        // subnet length
        strHex.Sprintf("%02X", objAi.aSubnetLength[0]);
        objBuffer.Append(strHex);

        // carrier id - plmn id (mcc + mnc)
        if ((objAi.aCarrierId[0] != 0x00) || (objAi.aCarrierId[1] != 0x00) ||
                (objAi.aCarrierId[2] != 0x00))
        {
            strHex.Sprintf(
                    "%02X%02X%02X", objAi.aCarrierId[0], objAi.aCarrierId[1], objAi.aCarrierId[2]);
            objBuffer.Append(strHex);
        }
    }
}

static void GetAccessInfo(
        IN const I_WLAN_NODE_ID& objAi, IN const AString& strCellInfo, OUT AStringBuffer& objBuffer)
{
    if (strCellInfo.GetLength() > 0)
    {
        objBuffer.Sprintf("i-wlan-node-id=%s", strCellInfo.GetStr());
        return;
    }

    if (objAi.acMAC[0] != '\0')
    {
        objBuffer.Sprintf("i-wlan-node-id=%s", &(objAi.acMAC[0]));
    }
    else
    {
        objBuffer.Sprintf("i-wlan-node-id=%02x%02x%02x%02x%02x%02x", objAi.aMAC[0], objAi.aMAC[1],
                objAi.aMAC[2], objAi.aMAC[3], objAi.aMAC[4], objAi.aMAC[5]);
    }
}

static IMS_BOOL GetHeaderFromAccessClass(
        IN const AccessNetworkInfo& objAni, IN const AString& strCellInfo, OUT AString& strHeader)
{
    if (!objAni.bIsAccessInfoRequired)
    {
        strHeader = ACCESS_CLASS[objAni.nClass];
        return IMS_TRUE;
    }

    AStringBuffer objBuffer(64);

    switch (objAni.nClass)
    {
        case AccessNetworkInfo::CLASS_3GPP_GERAN:  // FALL-THROUGH
        case AccessNetworkInfo::CLASS_3GPP_GAN:
            GetAccessInfo(objAni.uniAI.cgi_3gpp, strCellInfo, objBuffer);
            break;

        case AccessNetworkInfo::CLASS_3GPP_HSPA:   // FALL-THROUGH
        case AccessNetworkInfo::CLASS_3GPP_UTRAN:  // FALL-THROUGH
        case AccessNetworkInfo::CLASS_3GPP_E_UTRAN:
            GetAccessInfo(objAni.uniAI.utran_cell_id_3gpp, strCellInfo, objBuffer);
            break;

        case AccessNetworkInfo::CLASS_3GPP_NR:
            GetAccessInfo(objAni.uniAI.nr_utran_cell_id_3gpp, strCellInfo, objBuffer);
            break;

        case AccessNetworkInfo::CLASS_3GPP_WLAN:
            GetAccessInfo(objAni.uniAI.i_wlan_node_id, strCellInfo, objBuffer);
            break;

        default:
            return IMS_FALSE;
    }

    strHeader.Sprintf("%s;%s", ACCESS_CLASS[objAni.nClass],
            static_cast<const AStringBuffer&>(objBuffer).GetString().GetStr());

    return IMS_TRUE;
}

static IMS_BOOL GetHeaderFromAccessType(
        IN const AccessNetworkInfo& objAni, IN const AString& strCellInfo, OUT AString& strHeader)
{
    if (!objAni.bIsAccessInfoRequired)
    {
        strHeader = ACCESS_TYPE[objAni.nType];
        return IMS_TRUE;
    }

    AStringBuffer objBuffer(64);

    switch (objAni.nType)
    {
        case AccessNetworkInfo::TYPE_3GPP_GERAN:
            GetAccessInfo(objAni.uniAI.cgi_3gpp, strCellInfo, objBuffer);
            break;

        case AccessNetworkInfo::TYPE_3GPP_UTRAN_FDD:    // FALL-THROUGH
        case AccessNetworkInfo::TYPE_3GPP_UTRAN_TDD:    // FALL-THROUGH
        case AccessNetworkInfo::TYPE_3GPP_E_UTRAN_FDD:  // FALL-THROUGH
        case AccessNetworkInfo::TYPE_3GPP_E_UTRAN_TDD:
            GetAccessInfo(objAni.uniAI.utran_cell_id_3gpp, strCellInfo, objBuffer);
            break;

        case AccessNetworkInfo::TYPE_3GPP_NR_FDD:  // FALL-THROUGH
        case AccessNetworkInfo::TYPE_3GPP_NR_TDD:
            GetAccessInfo(objAni.uniAI.nr_utran_cell_id_3gpp, strCellInfo, objBuffer);
            break;

            // 3GPP2
        case AccessNetworkInfo::TYPE_3GPP2_1X:       // FALL-THROUGH
        case AccessNetworkInfo::TYPE_3GPP2_1X_HRPD:  // FALL-THROUGH
        case AccessNetworkInfo::TYPE_3GPP2_UMB:
            GetAccessInfo(objAni.uniAI.ci_3gpp2, strCellInfo, objBuffer);
            break;

            // WIFI
        case AccessNetworkInfo::TYPE_IEEE_802_11:   // FALL-THROUGH
        case AccessNetworkInfo::TYPE_IEEE_802_11A:  // FALL-THROUGH
        case AccessNetworkInfo::TYPE_IEEE_802_11B:  // FALL-THROUGH
        case AccessNetworkInfo::TYPE_IEEE_802_11G:  // FALL-THROUGH
        case AccessNetworkInfo::TYPE_IEEE_802_11N:
            GetAccessInfo(objAni.uniAI.i_wlan_node_id, strCellInfo, objBuffer);
            break;

        default:
            return IMS_FALSE;
    }

    strHeader.Sprintf("%s;%s", ACCESS_TYPE[objAni.nType],
            static_cast<const AStringBuffer&>(objBuffer).GetString().GetStr());

    return IMS_TRUE;
}

PUBLIC GLOBAL IMS_BOOL AccessNetworkInfoFormatter::Encode(IN const AccessNetworkInfo& objAni,
        OUT AString& strHeader, IN const AString& strCellInfo /*= AString::ConstNull()*/)
{
    if ((objAni.nType < AccessNetworkInfo::TYPE_NONE) ||
            (objAni.nType >= AccessNetworkInfo::TYPE_MAX))
    {
        IMS_TRACE_D("ANInfo - invalid type", 0, 0, 0);
        return IMS_FALSE;
    }

    if ((objAni.nClass < AccessNetworkInfo::CLASS_NONE) ||
            (objAni.nClass >= AccessNetworkInfo::CLASS_MAX))
    {
        IMS_TRACE_D("ANInfo - invalid class", 0, 0, 0);
        return IMS_FALSE;
    }

    if ((objAni.nType == AccessNetworkInfo::TYPE_NONE) &&
            (objAni.nClass == AccessNetworkInfo::CLASS_NONE))
    {
        IMS_TRACE_D("ANInfo - no network type & class", 0, 0, 0);
        return IMS_FALSE;
    }

    // Type info. will be preferred...
    if (objAni.nType != AccessNetworkInfo::TYPE_NONE)
    {
        return GetHeaderFromAccessType(objAni, strCellInfo, strHeader);
    }
    else if (objAni.nClass != AccessNetworkInfo::CLASS_NONE)
    {
        return GetHeaderFromAccessClass(objAni, strCellInfo, strHeader);
    }

    return IMS_FALSE;
}
