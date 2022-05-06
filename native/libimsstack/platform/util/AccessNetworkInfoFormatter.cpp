/*
    Author
    <table>
    date      author                    description
    --------  --------------            ----------
    20151001  hwangoo.park@             Created
    </table>

    Description

*/

#include "ServiceMemory.h"
#include "ServiceTrace.h"
#include "AStringBuffer.h"
#include "AccessNetworkInfoFormatter.h"

__IMS_TRACE_TAG_ADAPT__;

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

LOCAL
void GetAccessInfo(
        IN CONST CGI_3GPP& stAI, IN CONST AString& strCellInfo, OUT AStringBuffer& objBuffer)
{
    if (strCellInfo.GetLength() > 0)
    {
        objBuffer.Sprintf("cgi-3gpp=%s", strCellInfo.GetStr());
        return;
    }

    if (stAI.acCGI[0] != '\0')
    {
        objBuffer.Sprintf("cgi-3gpp=%s", &(stAI.acCGI[0]));
    }
    else
    {
        if ((stAI.aPLMNId[2] & 0x0F) == 0x0F)
        {
            // mnc is 2digits
            objBuffer.Sprintf("cgi-3gpp=%x%x%x%x%x%02x%02x%02x%02x", (stAI.aPLMNId[0] >> 4) & 0x0F,
                    stAI.aPLMNId[0] & 0x0F, (stAI.aPLMNId[1] >> 4) & 0x0F, stAI.aPLMNId[1] & 0x0F,
                    (stAI.aPLMNId[2] >> 4) & 0x0F, stAI.aLAC[0], stAI.aLAC[1], stAI.aCI[0],
                    stAI.aCI[1]);
        }
        else
        {
            objBuffer.Sprintf("cgi-3gpp=%x%x%x%x%x%x%02x%02x%02x%02x",
                    (stAI.aPLMNId[0] >> 4) & 0x0F, stAI.aPLMNId[0] & 0x0F,
                    (stAI.aPLMNId[1] >> 4) & 0x0F, stAI.aPLMNId[1] & 0x0F,
                    (stAI.aPLMNId[2] >> 4) & 0x0F, stAI.aPLMNId[2] & 0x0F, stAI.aLAC[0],
                    stAI.aLAC[1], stAI.aCI[0], stAI.aCI[1]);
        }
    }
}

LOCAL
void GetAccessInfo(IN CONST UTRAN_CELL_ID_3GPP& stAI, IN CONST AString& strCellInfo,
        OUT AStringBuffer& objBuffer)
{
    if (strCellInfo.GetLength() > 0)
    {
        objBuffer.Sprintf("utran-cell-id-3gpp=%s", strCellInfo.GetStr());
        return;
    }

    if (stAI.acUTRAN_CELL_ID[0] != '\0')
    {
        objBuffer.Sprintf("utran-cell-id-3gpp=%s", &(stAI.acUTRAN_CELL_ID[0]));
    }
    else
    {
        if ((stAI.aPLMNId[2] & 0x0F) == 0x0F)
        {
            // mnc is 2digits
            objBuffer.Sprintf("utran-cell-id-3gpp=%x%x%x%x%x%02x%02x%02x%02x%02x%x",
                    (stAI.aPLMNId[0] >> 4) & 0x0F, stAI.aPLMNId[0] & 0x0F,
                    (stAI.aPLMNId[1] >> 4) & 0x0F, stAI.aPLMNId[1] & 0x0F,
                    (stAI.aPLMNId[2] >> 4) & 0x0F, stAI.aLACorTAC[0], stAI.aLACorTAC[1],
                    stAI.aCellId[0], stAI.aCellId[1], stAI.aCellId[2], (stAI.aCellId[3] & 0x0F));
        }
        else
        {
            objBuffer.Sprintf("utran-cell-id-3gpp=%x%x%x%x%x%x%02x%02x%02x%02x%02x%x",
                    (stAI.aPLMNId[0] >> 4) & 0x0F, stAI.aPLMNId[0] & 0x0F,
                    (stAI.aPLMNId[1] >> 4) & 0x0F, stAI.aPLMNId[1] & 0x0F,
                    (stAI.aPLMNId[2] >> 4) & 0x0F, stAI.aPLMNId[2] & 0x0F, stAI.aLACorTAC[0],
                    stAI.aLACorTAC[1], stAI.aCellId[0], stAI.aCellId[1], stAI.aCellId[2],
                    (stAI.aCellId[3] & 0x0F));
        }
    }
}

LOCAL
void GetAccessInfo(IN CONST NR_UTRAN_CELL_ID_3GPP& stAI, IN CONST AString& strCellInfo,
        OUT AStringBuffer& objBuffer)
{
    if (strCellInfo.GetLength() > 0)
    {
        objBuffer.Sprintf("utran-cell-id-3gpp=%s", strCellInfo.GetStr());
        return;
    }

    if (stAI.acUTRAN_CELL_ID[0] != '\0')
    {
        objBuffer.Sprintf("utran-cell-id-3gpp=%s", &(stAI.acUTRAN_CELL_ID[0]));
    }
    else
    {
        if ((stAI.aPLMNId[2] & 0x0F) == 0x0F)
        {
            // mnc is 2digits
            objBuffer.Sprintf("utran-cell-id-3gpp=%x%x%x%x%x%02x%02x%02x%02x%02x%02x%02x%x",
                    (stAI.aPLMNId[0] >> 4) & 0x0F, stAI.aPLMNId[0] & 0x0F,
                    (stAI.aPLMNId[1] >> 4) & 0x0F, stAI.aPLMNId[1] & 0x0F,
                    (stAI.aPLMNId[2] >> 4) & 0x0F, stAI.aTAC[0], stAI.aTAC[1], stAI.aTAC[2],
                    stAI.aCellId[0], stAI.aCellId[1], stAI.aCellId[2], stAI.aCellId[3],
                    (stAI.aCellId[4] & 0x0F));
        }
        else
        {
            objBuffer.Sprintf("utran-cell-id-3gpp=%x%x%x%x%x%x%02x%02x%02x%02x%02x%02x%02x%x",
                    (stAI.aPLMNId[0] >> 4) & 0x0F, stAI.aPLMNId[0] & 0x0F,
                    (stAI.aPLMNId[1] >> 4) & 0x0F, stAI.aPLMNId[1] & 0x0F,
                    (stAI.aPLMNId[2] >> 4) & 0x0F, stAI.aPLMNId[2] & 0x0F, stAI.aTAC[0],
                    stAI.aTAC[1], stAI.aTAC[2], stAI.aCellId[0], stAI.aCellId[1], stAI.aCellId[2],
                    stAI.aCellId[3], (stAI.aCellId[4] & 0x0F));
        }
    }
}

LOCAL
void GetAccessInfo(
        IN CONST CI_3GPP2& stAI, IN CONST AString& strCellInfo, OUT AStringBuffer& objBuffer)
{
    if (strCellInfo.GetLength() > 0)
    {
        objBuffer.Sprintf("ci-3gpp2=%s", strCellInfo.GetStr());
        return;
    }

    if (stAI.acCI[0] != '\0')
    {
        objBuffer.Sprintf("ci-3gpp2=%s", &(stAI.acCI[0]));
    }
    else
    {
        AString strHEX;

        objBuffer.Append("ci-3gpp2=");

        // sector id
        for (IMS_SINT32 i = 0; i < 16; ++i)
        {
            strHEX.Sprintf("%02X", stAI.aSectorId[i]);
            objBuffer.Append(strHEX);
        }

        // subnet length
        strHEX.Sprintf("%02X", stAI.aSubnetLength[0]);
        objBuffer.Append(strHEX);

        // carrier id - plmn id (mcc + mnc)
        if ((stAI.aCarrierId[0] != 0x00) || (stAI.aCarrierId[1] != 0x00) ||
                (stAI.aCarrierId[2] != 0x00))
        {
            strHEX.Sprintf(
                    "%02X%02X%02X", stAI.aCarrierId[0], stAI.aCarrierId[1], stAI.aCarrierId[2]);
            objBuffer.Append(strHEX);
        }
    }
}

LOCAL
void GetAccessInfo(
        IN CONST I_WLAN_NODE_ID& stAI, IN CONST AString& strCellInfo, OUT AStringBuffer& objBuffer)
{
    if (strCellInfo.GetLength() > 0)
    {
        objBuffer.Sprintf("i-wlan-node-id=%s", strCellInfo.GetStr());
        return;
    }

    if (stAI.acMAC[0] != '\0')
    {
        objBuffer.Sprintf("i-wlan-node-id=%s", &(stAI.acMAC[0]));
    }
    else
    {
        objBuffer.Sprintf("i-wlan-node-id=%02x%02x%02x%02x%02x%02x", stAI.aMAC[0], stAI.aMAC[1],
                stAI.aMAC[2], stAI.aMAC[3], stAI.aMAC[4], stAI.aMAC[5]);
    }
}

LOCAL
IMS_BOOL GetHeaderFromAccessClass(IN CONST AccessNetworkInfo& objANInfo,
        IN CONST AString& strCellInfo, OUT AString& strHeader)
{
    if (!objANInfo.bIsAccessInfoRequired)
    {
        strHeader = ACCESS_CLASS[objANInfo.nClass];
        return IMS_TRUE;
    }

    AStringBuffer objBuffer(64);

    switch (objANInfo.nClass)
    {
        case AccessNetworkInfo::CLASS_3GPP_GERAN:  // FALL-THROUGH
        case AccessNetworkInfo::CLASS_3GPP_GAN:
            GetAccessInfo(objANInfo.uniAI.cgi_3gpp, strCellInfo, objBuffer);
            break;

        case AccessNetworkInfo::CLASS_3GPP_HSPA:   // FALL-THROUGH
        case AccessNetworkInfo::CLASS_3GPP_UTRAN:  // FALL-THROUGH
        case AccessNetworkInfo::CLASS_3GPP_E_UTRAN:
            GetAccessInfo(objANInfo.uniAI.utran_cell_id_3gpp, strCellInfo, objBuffer);
            break;

        case AccessNetworkInfo::CLASS_3GPP_NR:
            GetAccessInfo(objANInfo.uniAI.nr_utran_cell_id_3gpp, strCellInfo, objBuffer);
            break;

        case AccessNetworkInfo::CLASS_3GPP_WLAN:
            GetAccessInfo(objANInfo.uniAI.i_wlan_node_id, strCellInfo, objBuffer);
            break;

        default:
            return IMS_FALSE;
    }

    strHeader.Sprintf("%s;%s", ACCESS_CLASS[objANInfo.nClass],
            static_cast<const AStringBuffer&>(objBuffer).GetString().GetStr());

    return IMS_TRUE;
}

LOCAL
IMS_BOOL GetHeaderFromAccessType(IN CONST AccessNetworkInfo& objANInfo,
        IN CONST AString& strCellInfo, OUT AString& strHeader)
{
    if (!objANInfo.bIsAccessInfoRequired)
    {
        strHeader = ACCESS_TYPE[objANInfo.nType];
        return IMS_TRUE;
    }

    AStringBuffer objBuffer(64);

    switch (objANInfo.nType)
    {
        case AccessNetworkInfo::TYPE_3GPP_GERAN:
            GetAccessInfo(objANInfo.uniAI.cgi_3gpp, strCellInfo, objBuffer);
            break;

        case AccessNetworkInfo::TYPE_3GPP_UTRAN_FDD:    // FALL-THROUGH
        case AccessNetworkInfo::TYPE_3GPP_UTRAN_TDD:    // FALL-THROUGH
        case AccessNetworkInfo::TYPE_3GPP_E_UTRAN_FDD:  // FALL-THROUGH
        case AccessNetworkInfo::TYPE_3GPP_E_UTRAN_TDD:
            GetAccessInfo(objANInfo.uniAI.utran_cell_id_3gpp, strCellInfo, objBuffer);
            break;

        case AccessNetworkInfo::TYPE_3GPP_NR_FDD:  // FALL-THROUGH
        case AccessNetworkInfo::TYPE_3GPP_NR_TDD:
            GetAccessInfo(objANInfo.uniAI.nr_utran_cell_id_3gpp, strCellInfo, objBuffer);
            break;

            // 3GPP2
        case AccessNetworkInfo::TYPE_3GPP2_1X:       // FALL-THROUGH
        case AccessNetworkInfo::TYPE_3GPP2_1X_HRPD:  // FALL-THROUGH
        case AccessNetworkInfo::TYPE_3GPP2_UMB:
            GetAccessInfo(objANInfo.uniAI.ci_3gpp2, strCellInfo, objBuffer);
            break;

            // WIFI
        case AccessNetworkInfo::TYPE_IEEE_802_11:   // FALL-THROUGH
        case AccessNetworkInfo::TYPE_IEEE_802_11A:  // FALL-THROUGH
        case AccessNetworkInfo::TYPE_IEEE_802_11B:  // FALL-THROUGH
        case AccessNetworkInfo::TYPE_IEEE_802_11G:  // FALL-THROUGH
        case AccessNetworkInfo::TYPE_IEEE_802_11N:
            GetAccessInfo(objANInfo.uniAI.i_wlan_node_id, strCellInfo, objBuffer);
            break;

        default:
            return IMS_FALSE;
    }

    strHeader.Sprintf("%s;%s", ACCESS_TYPE[objANInfo.nType],
            static_cast<const AStringBuffer&>(objBuffer).GetString().GetStr());

    return IMS_TRUE;
}

PUBLIC GLOBAL IMS_BOOL AccessNetworkInfoFormatter::Encode(IN CONST AccessNetworkInfo& objANInfo,
        OUT AString& strHeader, IN CONST AString& strCellInfo /* = AString::ConstNull()*/)
{
    if ((objANInfo.nType < AccessNetworkInfo::TYPE_NONE) ||
            (objANInfo.nType >= AccessNetworkInfo::TYPE_MAX))
    {
        IMS_TRACE_D("ANInfo - invalid type", 0, 0, 0);
        return IMS_FALSE;
    }

    if ((objANInfo.nClass < AccessNetworkInfo::CLASS_NONE) ||
            (objANInfo.nClass >= AccessNetworkInfo::CLASS_MAX))
    {
        IMS_TRACE_D("ANInfo - invalid class", 0, 0, 0);
        return IMS_FALSE;
    }

    if ((objANInfo.nType == AccessNetworkInfo::TYPE_NONE) &&
            (objANInfo.nClass == AccessNetworkInfo::CLASS_NONE))
    {
        IMS_TRACE_D("ANInfo - no network type & class", 0, 0, 0);
        return IMS_FALSE;
    }

    // Type info. will be preferred...
    if (objANInfo.nType != AccessNetworkInfo::TYPE_NONE)
    {
        return GetHeaderFromAccessType(objANInfo, strCellInfo, strHeader);
    }
    else if (objANInfo.nClass != AccessNetworkInfo::CLASS_NONE)
    {
        return GetHeaderFromAccessClass(objANInfo, strCellInfo, strHeader);
    }

    return IMS_FALSE;
}
