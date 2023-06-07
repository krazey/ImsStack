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
#include "PlatformContext.h"
#include "ServiceMemory.h"
#include "ServicePhoneInfo.h"
#include "ServiceTrace.h"
#include "ServiceUtil.h"
#include "network/OsIpcan.h"
#include "network/OsNetworkConstants.h"

__IMS_TRACE_TAG_ADAPT__;

PUBLIC
OsIpcan::OsIpcan() {}

PUBLIC VIRTUAL OsIpcan::~OsIpcan() {}

PROTECTED VIRTUAL void OsIpcan::GetAccessInfo(
        IN IMS_SINT32 nSlotId, IN_OUT AccessNetworkInfo& objAni)
{
    IMS_SINT32 nDefaultNetworkType = TYPE_UNKNOWN;

    if ((objAni.nType == AccessNetworkInfo::TYPE_3GPP_E_UTRAN_FDD) ||
            (objAni.nType == AccessNetworkInfo::TYPE_3GPP_E_UTRAN_TDD))
    {
        nDefaultNetworkType = TYPE_LTE;
    }
    else if ((objAni.nType == AccessNetworkInfo::TYPE_3GPP_NR_FDD) ||
            (objAni.nType == AccessNetworkInfo::TYPE_3GPP_NR_TDD))
    {
        nDefaultNetworkType = TYPE_NR;
    }

    IMS_SINT32 nNetworkType = nDefaultNetworkType;
    AStringArray objCellIdentities;
    PlatformContext::GetInstance()->GetSystem()->GetAccessNetworkInfo(
            nDefaultNetworkType, nNetworkType, objCellIdentities, nSlotId);

    objAni = CreateAccessNetworkInfo(nNetworkType, objCellIdentities);
}

PROTECTED VIRTUAL void OsIpcan::GetAccessInfoForWiFi(OUT AccessNetworkInfo& objAni)
{
    AString strMacAddress = PlatformContext::GetInstance()->GetSystem()->GetWifiBssId();
    ImsList<AString> objTokens = strMacAddress.Split(':');

    if (objTokens.GetSize() == ANI_WLAN_MAX_MAC)
    {
        for (IMS_UINT32 i = 0; i < objTokens.GetSize(); i++)
        {
            const AString& strByte = objTokens.GetAt(i);
            IMS_BOOL bOk = IMS_FALSE;
            IMS_UINT16 nByte = strByte.ToUInt16(&bOk, 16);

            if (bOk)
            {
                objAni.uniAI.i_wlan_node_id.aMAC[i] = static_cast<IMS_UINT8>(0xFF & nByte);
            }
            else
            {
                IMS_TRACE_E(0, "Invalid MAC address value (%s, %d)", strByte.GetStr(), i, 0);
                objAni.uniAI.i_wlan_node_id.aMAC[i] = 0xFF;
            }
        }

        // FIXME: obtain the proper access type
        objAni.nType = AccessNetworkInfo::TYPE_IEEE_802_11;
        objAni.nClass = AccessNetworkInfo::CLASS_NONE;
    }
    else
    {
        objAni.nType = AccessNetworkInfo::TYPE_NONE;
        objAni.nClass = AccessNetworkInfo::CLASS_NONE;
    }
}

PROTECTED VIRTUAL void OsIpcan::GetLastAccessInfo(IN IMS_SINT32 nSlotId,
        OUT AccessNetworkInfo& objAni, OUT AString& strTimestamp, OUT AString& strCellInfoAge)
{
    AStringArray objCellIdentities =
            PlatformContext::GetInstance()->GetSystem()->GetLastAccessNetworkInfo(
                    RADIOTECH_TYPE_UNKNOWN, nSlotId);

    // 0 : network type
    // 1 : timestamp as UTC format
    // 2 : cell age as seconds format
    // 3..7 : same information in GetAccessNetworkInfo

    if (objCellIdentities.IsEmpty() || (objCellIdentities.GetCount() < 4))
    {
        strTimestamp = AString::ConstNull();
        strCellInfoAge = AString::ConstNull();
        return;
    }

    const AString& strNetworkType = objCellIdentities.GetElementAt(0);
    IMS_SINT32 nNetworkType = strNetworkType.ToInt32();

    strTimestamp = objCellIdentities.GetElementAt(1);
    strCellInfoAge = objCellIdentities.GetElementAt(2);

    // Removes the 1st and 2nd and 3rd element
    objCellIdentities.RemoveElementAt(0);
    objCellIdentities.RemoveElementAt(0);
    objCellIdentities.RemoveElementAt(0);

    objAni = CreateAccessNetworkInfo(nNetworkType, objCellIdentities);
}

PROTECTED VIRTUAL void OsIpcan::GetLastAccessInfoForWiFi(
        OUT AccessNetworkInfo& objAni, OUT AString& strTimestamp, OUT AString& strCellInfoAge)
{
    // FIXME: consider to implement the below logic later
    strTimestamp = AString::ConstNull();
    strCellInfoAge = AString::ConstNull();

    objAni.nType = AccessNetworkInfo::TYPE_NONE;
    objAni.nClass = AccessNetworkInfo::CLASS_NONE;
}

PROTECTED VIRTUAL IMS_SINT32 OsIpcan::GetNetworkType(IN IMS_SINT32 nSlotId)
{
    return PlatformContext::GetInstance()->GetSystem()->GetNetworkType(nSlotId);
}

PRIVATE GLOBAL AccessNetworkInfo OsIpcan::CreateAccessNetworkInfo(
        IN IMS_SINT32 nNetworkType, IN const AStringArray& objCellIdentities)
{
#define ANI_ITEM_COUNT 4

#define ANI_ITEM_MCC_I 0
#define ANI_ITEM_MNC_I 1
#define ANI_ITEM_CELL_ID_I 2
#define ANI_ITEM_LAC_I 3
#define ANI_ITEM_TAC_I 3
#define ANI_ITEM_MODE_I 4

#define ANI_ITEM_COUNT_EHRPD 2
#define ANI_ITEM_SECTOR_ID_I 0
#define ANI_ITEM_SUBNET_LEN_I 1

    AccessNetworkInfo objAni;
    IMS_BOOL bIsNetworkTypeLte = ((nNetworkType == TYPE_LTE) || (nNetworkType == TYPE_LTE_CA));

    if (bIsNetworkTypeLte || (nNetworkType == TYPE_UMTS) || (nNetworkType == TYPE_HSDPA) ||
            (nNetworkType == TYPE_HSUPA) || (nNetworkType == TYPE_HSPA) ||
            (nNetworkType == TYPE_HSPAP))
    {
        if (bIsNetworkTypeLte)
        {
            objAni.nType = AccessNetworkInfo::TYPE_3GPP_E_UTRAN_FDD;
        }
        else
        {
            objAni.nType = AccessNetworkInfo::TYPE_3GPP_UTRAN_FDD;
        }

        // MCC(3);MNC(2 or 3);CellID(7);TAC(4)
        // 450;06F;1177a00;00d1

        if (objCellIdentities.GetCount() < ANI_ITEM_COUNT)
        {
            objAni.bIsAccessInfoRequired = IMS_FALSE;
            return objAni;
        }

        // MCC / MNC / TAC (Tracking Area Code) / Cell Identity
        const AString& strMcc = objCellIdentities.GetElementAt(ANI_ITEM_MCC_I);
        const AString& strMnc = objCellIdentities.GetElementAt(ANI_ITEM_MNC_I);
        const AString& strTac = objCellIdentities.GetElementAt(ANI_ITEM_TAC_I);
        const AString& strCellId = objCellIdentities.GetElementAt(ANI_ITEM_CELL_ID_I);

        // LTE frequency mode: FDD / TDD
        if (bIsNetworkTypeLte && (objCellIdentities.GetCount() > ANI_ITEM_COUNT))
        {
            const AString& strLteMode = objCellIdentities.GetElementAt(ANI_ITEM_MODE_I);

            if (strLteMode.EqualsIgnoreCase(ANI_MODE_TDD))
            {
                objAni.nType = AccessNetworkInfo::TYPE_3GPP_E_UTRAN_TDD;
            }
        }

        if ((strMcc.GetLength() > ANI_3GPP_MCC_MAX_LEN) ||
                (strMnc.GetLength() > ANI_3GPP_MNC_MAX_LEN))
        {
            objAni.bIsAccessInfoRequired = IMS_FALSE;
            IMS_TRACE_D("PANI :: Invalid length (MCC=%d, MNC=%d)", strMcc.GetLength(),
                    strMnc.GetLength(), 0);
            return objAni;
        }

        if ((strTac.GetLength() > ANI_3GPP_TAC_MAX_LEN) ||
                (strCellId.GetLength() > ANI_3GPP_UTRAN_CELL_ID_MAX_LEN))
        {
            objAni.bIsAccessInfoRequired = IMS_FALSE;
            IMS_TRACE_D("PANI :: Invalid length (TAC=%d, CellIdentity=%d)", strTac.GetLength(),
                    strCellId.GetLength(), 0);
            return objAni;
        }

        IMS_SINT32 nIndex = 0;

        // MCC (3 digits)
        for (IMS_SINT32 i = 0; i < strMcc.GetLength(); ++i)
        {
            objAni.uniAI.utran_cell_id_3gpp.acUTRAN_CELL_ID[nIndex] = strMcc[i];
            nIndex++;
        }

        // MNC (2 or 3 digits)
        if (strMnc.GetLength() == ANI_3GPP_MNC_MAX_LEN)
        {
            if ((strMnc[2] == 'F') || (strMnc[2] == 'f') || (strMnc[2] == 0xFF))
            {
                objAni.uniAI.utran_cell_id_3gpp.acUTRAN_CELL_ID[nIndex] = strMnc[0];
                nIndex++;
                objAni.uniAI.utran_cell_id_3gpp.acUTRAN_CELL_ID[nIndex] = strMnc[1];
                nIndex++;
            }
            else
            {
                objAni.uniAI.utran_cell_id_3gpp.acUTRAN_CELL_ID[nIndex] = strMnc[0];
                nIndex++;
                objAni.uniAI.utran_cell_id_3gpp.acUTRAN_CELL_ID[nIndex] = strMnc[1];
                nIndex++;
                objAni.uniAI.utran_cell_id_3gpp.acUTRAN_CELL_ID[nIndex] = strMnc[2];
                nIndex++;
            }
        }
        else
        {
            for (IMS_SINT32 i = 0; i < strMnc.GetLength(); ++i)
            {
                objAni.uniAI.utran_cell_id_3gpp.acUTRAN_CELL_ID[nIndex] = strMnc[i];
                nIndex++;
            }
        }

        IMS_SINT32 nZeroPadding;

        // TAC (Tracking Area Code, 16bits, 4hex)
        nZeroPadding = ANI_3GPP_TAC_MAX_LEN - strTac.GetLength();

        for (IMS_SINT32 i = 0; i < nZeroPadding; i++)
        {
            objAni.uniAI.utran_cell_id_3gpp.acUTRAN_CELL_ID[nIndex] = '0';
            nIndex++;
        }

        for (IMS_SINT32 i = 0; i < strTac.GetLength(); ++i)
        {
            objAni.uniAI.utran_cell_id_3gpp.acUTRAN_CELL_ID[nIndex] = strTac[i];
            nIndex++;
        }

        // Cell Identity (28bits, 7hex)
        nZeroPadding = ANI_3GPP_UTRAN_CELL_ID_MAX_LEN - strCellId.GetLength();

        for (IMS_SINT32 i = 0; i < nZeroPadding; i++)
        {
            objAni.uniAI.utran_cell_id_3gpp.acUTRAN_CELL_ID[nIndex] = '0';
            nIndex++;
        }

        for (IMS_SINT32 i = 0; i < strCellId.GetLength(); ++i)
        {
            objAni.uniAI.utran_cell_id_3gpp.acUTRAN_CELL_ID[nIndex] = strCellId[i];
            nIndex++;
        }

        IMS_TRACE_I("PANI :: %s (%s, %d)", bIsNetworkTypeLte ? "LTE" : "3G",
                IMS_UTIL_SYS_PROP_IS_DEBUG_MODE() ? objAni.uniAI.utran_cell_id_3gpp.acUTRAN_CELL_ID
                                                  : "xxx",
                nIndex);
    }
    else if (nNetworkType == TYPE_NR)
    {
        objAni.nType = AccessNetworkInfo::TYPE_3GPP_NR_FDD;

        // MCC(3);MNC(2 or 3);CellID(9);TAC(6)
        // 450;06F;1177a0000;0000d1

        if (objCellIdentities.GetCount() < ANI_ITEM_COUNT)
        {
            objAni.bIsAccessInfoRequired = IMS_FALSE;
            return objAni;
        }

        // MCC / MNC / TAC (Tracking Area Code) / Cell Identity
        const AString& strMcc = objCellIdentities.GetElementAt(ANI_ITEM_MCC_I);
        const AString& strMnc = objCellIdentities.GetElementAt(ANI_ITEM_MNC_I);
        const AString& strTac = objCellIdentities.GetElementAt(ANI_ITEM_TAC_I);
        const AString& strCellId = objCellIdentities.GetElementAt(ANI_ITEM_CELL_ID_I);

        // Mode: FDD / TDD
        if (objCellIdentities.GetCount() > ANI_ITEM_COUNT)
        {
            const AString& strMode = objCellIdentities.GetElementAt(ANI_ITEM_MODE_I);

            if (strMode.EqualsIgnoreCase(ANI_MODE_TDD))
            {
                objAni.nType = AccessNetworkInfo::TYPE_3GPP_NR_TDD;
            }
        }

        if ((strMcc.GetLength() > ANI_3GPP_MCC_MAX_LEN) ||
                (strMnc.GetLength() > ANI_3GPP_MNC_MAX_LEN))
        {
            objAni.bIsAccessInfoRequired = IMS_FALSE;
            IMS_TRACE_D("PANI :: Invalid length (MCC=%d, MNC=%d)", strMcc.GetLength(),
                    strMnc.GetLength(), 0);
            return objAni;
        }

        if ((strTac.GetLength() > ANI_3GPP_NR_TAC_MAX_LEN) ||
                (strCellId.GetLength() > ANI_3GPP_NR_CELL_ID_MAX_LEN))
        {
            objAni.bIsAccessInfoRequired = IMS_FALSE;
            IMS_TRACE_D("PANI :: Invalid length (TAC=%d, CellIdentity=%d)", strTac.GetLength(),
                    strCellId.GetLength(), 0);
            return objAni;
        }

        IMS_SINT32 nIndex = 0;

        // MCC (3 digits)
        for (IMS_SINT32 i = 0; i < strMcc.GetLength(); ++i)
        {
            objAni.uniAI.nr_utran_cell_id_3gpp.acUTRAN_CELL_ID[nIndex] = strMcc[i];
            nIndex++;
        }

        // MNC (2 or 3 digits)
        if (strMnc.GetLength() == ANI_3GPP_MNC_MAX_LEN)
        {
            if ((strMnc[2] == 'F') || (strMnc[2] == 'f') || (strMnc[2] == 0xFF))
            {
                objAni.uniAI.nr_utran_cell_id_3gpp.acUTRAN_CELL_ID[nIndex] = strMnc[0];
                nIndex++;
                objAni.uniAI.nr_utran_cell_id_3gpp.acUTRAN_CELL_ID[nIndex] = strMnc[1];
                nIndex++;
            }
            else
            {
                objAni.uniAI.nr_utran_cell_id_3gpp.acUTRAN_CELL_ID[nIndex] = strMnc[0];
                nIndex++;
                objAni.uniAI.nr_utran_cell_id_3gpp.acUTRAN_CELL_ID[nIndex] = strMnc[1];
                nIndex++;
                objAni.uniAI.nr_utran_cell_id_3gpp.acUTRAN_CELL_ID[nIndex] = strMnc[2];
                nIndex++;
            }
        }
        else
        {
            for (IMS_SINT32 i = 0; i < strMnc.GetLength(); ++i)
            {
                objAni.uniAI.nr_utran_cell_id_3gpp.acUTRAN_CELL_ID[nIndex] = strMnc[i];
                nIndex++;
            }
        }

        IMS_SINT32 nZeroPadding;

        // TAC (Tracking Area Code, 24bits, 6hex)
        nZeroPadding = ANI_3GPP_NR_TAC_MAX_LEN - strTac.GetLength();

        for (IMS_SINT32 i = 0; i < nZeroPadding; i++)
        {
            objAni.uniAI.nr_utran_cell_id_3gpp.acUTRAN_CELL_ID[nIndex] = '0';
            nIndex++;
        }

        for (IMS_SINT32 i = 0; i < strTac.GetLength(); ++i)
        {
            objAni.uniAI.nr_utran_cell_id_3gpp.acUTRAN_CELL_ID[nIndex] = strTac[i];
            nIndex++;
        }

        // Cell Identity (36bits, 9hex)
        nZeroPadding = ANI_3GPP_NR_CELL_ID_MAX_LEN - strCellId.GetLength();

        for (IMS_SINT32 i = 0; i < nZeroPadding; i++)
        {
            objAni.uniAI.nr_utran_cell_id_3gpp.acUTRAN_CELL_ID[nIndex] = '0';
            nIndex++;
        }

        for (IMS_SINT32 i = 0; i < strCellId.GetLength(); ++i)
        {
            objAni.uniAI.nr_utran_cell_id_3gpp.acUTRAN_CELL_ID[nIndex] = strCellId[i];
            nIndex++;
        }

        IMS_TRACE_I("PANI :: NR (%s, %d)",
                IMS_UTIL_SYS_PROP_IS_DEBUG_MODE()
                        ? objAni.uniAI.nr_utran_cell_id_3gpp.acUTRAN_CELL_ID
                        : "xxx",
                nIndex, 0);
    }
    else if ((nNetworkType == TYPE_GPRS) || (nNetworkType == TYPE_EDGE))
    {
        objAni.nType = AccessNetworkInfo::TYPE_3GPP_GERAN;

        // MCC(3);MNC(2 or 3);CellID(4);LAC(4)
        if (objCellIdentities.GetCount() < ANI_ITEM_COUNT)
        {
            objAni.bIsAccessInfoRequired = IMS_FALSE;
            return objAni;
        }

        // MCC / MNC / LAC (Location Area Code) / Cell Identity
        const AString& strMcc = objCellIdentities.GetElementAt(ANI_ITEM_MCC_I);
        const AString& strMnc = objCellIdentities.GetElementAt(ANI_ITEM_MNC_I);
        const AString& strLac = objCellIdentities.GetElementAt(ANI_ITEM_LAC_I);
        const AString& strCellId = objCellIdentities.GetElementAt(ANI_ITEM_CELL_ID_I);

        if ((strMcc.GetLength() > ANI_3GPP_MCC_MAX_LEN) ||
                (strMnc.GetLength() > ANI_3GPP_MNC_MAX_LEN))
        {
            objAni.bIsAccessInfoRequired = IMS_FALSE;
            IMS_TRACE_D("PANI :: Invalid length (MCC=%d, MNC=%d)", strMcc.GetLength(),
                    strMnc.GetLength(), 0);
            return objAni;
        }

        if ((strLac.GetLength() > ANI_3GPP_LAC_MAX_LEN) ||
                (strCellId.GetLength() > ANI_3GPP_GERAN_CELL_ID_MAX_LEN))
        {
            objAni.bIsAccessInfoRequired = IMS_FALSE;
            IMS_TRACE_D("PANI :: Invalid length (LAC=%d, CellIdentity=%d)", strLac.GetLength(),
                    strCellId.GetLength(), 0);
            return objAni;
        }

        IMS_SINT32 nIndex = 0;

        for (IMS_SINT32 i = 0; i < strMcc.GetLength(); ++i)
        {
            objAni.uniAI.cgi_3gpp.acCGI[nIndex] = strMcc[i];
            nIndex++;
        }

        // MNC (2 or 3 digits)
        if (strMnc.GetLength() == ANI_3GPP_MNC_MAX_LEN)
        {
            if ((strMnc[2] == 'F') || (strMnc[2] == 'f') || (strMnc[2] == 0xFF))
            {
                objAni.uniAI.cgi_3gpp.acCGI[nIndex] = strMnc[0];
                nIndex++;
                objAni.uniAI.cgi_3gpp.acCGI[nIndex] = strMnc[1];
                nIndex++;
            }
            else
            {
                objAni.uniAI.cgi_3gpp.acCGI[nIndex] = strMnc[0];
                nIndex++;
                objAni.uniAI.cgi_3gpp.acCGI[nIndex] = strMnc[1];
                nIndex++;
                objAni.uniAI.cgi_3gpp.acCGI[nIndex] = strMnc[2];
                nIndex++;
            }
        }
        else
        {
            for (IMS_SINT32 i = 0; i < strMnc.GetLength(); ++i)
            {
                objAni.uniAI.cgi_3gpp.acCGI[nIndex] = strMnc[i];
                nIndex++;
            }
        }

        // LAC (Location Area Code, 16bits, 4hex)
        IMS_SINT32 nZeroPadding = ANI_3GPP_LAC_MAX_LEN - strLac.GetLength();

        for (IMS_SINT32 i = 0; i < nZeroPadding; i++)
        {
            objAni.uniAI.cgi_3gpp.acCGI[nIndex] = '0';
            nIndex++;
        }

        for (IMS_SINT32 i = 0; i < strLac.GetLength(); ++i)
        {
            objAni.uniAI.cgi_3gpp.acCGI[nIndex] = strLac[i];
            nIndex++;
        }

        // Cell Identity (16bits, 4hex)
        nZeroPadding = ANI_3GPP_GERAN_CELL_ID_MAX_LEN - strCellId.GetLength();

        for (IMS_SINT32 i = 0; i < nZeroPadding; i++)
        {
            objAni.uniAI.cgi_3gpp.acCGI[nIndex] = '0';
            nIndex++;
        }

        for (IMS_SINT32 i = 0; i < strCellId.GetLength(); ++i)
        {
            objAni.uniAI.cgi_3gpp.acCGI[nIndex] = strCellId[i];
            nIndex++;
        }

        IMS_TRACE_I("PANI :: GERAN (%s, %d)",
                IMS_UTIL_SYS_PROP_IS_DEBUG_MODE() ? objAni.uniAI.cgi_3gpp.acCGI : "xxx", nIndex, 0);
    }
    else if (nNetworkType == TYPE_EHRPD)
    {
        objAni.nType = AccessNetworkInfo::TYPE_3GPP2_1X_HRPD;

        // Sector ID (32);SunetLength(2)
        // 8e0a0a0a0a0a0a0a0a0a0a0a0a022382;68
        // ril.ehrpd.netinfo
        if (objCellIdentities.GetCount() < ANI_ITEM_COUNT_EHRPD)
        {
            objAni.bIsAccessInfoRequired = IMS_FALSE;
            return objAni;
        }

        // Sector Id / Subnet Length
        const AString& strSector = objCellIdentities.GetElementAt(ANI_ITEM_SECTOR_ID_I);
        const AString& strSubnet = objCellIdentities.GetElementAt(ANI_ITEM_SUBNET_LEN_I);

        if ((strSector.GetLength() > ANI_3GPP2_SECTOR_ID_MAX_LEN) ||
                (strSubnet.GetLength() > ANI_3GPP2_SUBNET_LENGTH_MAX_LEN))
        {
            objAni.bIsAccessInfoRequired = IMS_FALSE;
            IMS_TRACE_D("PANI :: Invalid length (sectorId=%d, subnetLength=%d)",
                    strSector.GetLength(), strSubnet.GetLength(), 0);
            return objAni;
        }

        IMS_SINT32 nIndex = 0;

        // Sector Id (128bits, 32hex)
        IMS_SINT32 nZeroPadding = ANI_3GPP2_SECTOR_ID_MAX_LEN - strSector.GetLength();

        for (IMS_SINT32 i = 0; i < nZeroPadding; i++)
        {
            objAni.uniAI.ci_3gpp2.acCI[nIndex] = '0';
            nIndex++;
        }

        for (IMS_SINT32 i = 0; i < strSector.GetLength(); ++i)
        {
            objAni.uniAI.ci_3gpp2.acCI[nIndex] = strSector[i];
            nIndex++;
        }

        // Subnet Length (8bits, 2hex)
        nZeroPadding = ANI_3GPP2_SUBNET_LENGTH_MAX_LEN - strSubnet.GetLength();

        for (IMS_SINT32 i = 0; i < nZeroPadding; i++)
        {
            objAni.uniAI.ci_3gpp2.acCI[nIndex] = '0';
            nIndex++;
        }

        for (IMS_SINT32 i = 0; i < strSubnet.GetLength(); ++i)
        {
            objAni.uniAI.ci_3gpp2.acCI[nIndex] = strSubnet[i];
            nIndex++;
        }

        IMS_TRACE_I("PANI :: eHRPD (%s, %d)",
                IMS_UTIL_SYS_PROP_IS_DEBUG_MODE() ? objAni.uniAI.ci_3gpp2.acCI : "xxx", nIndex, 0);
    }
    else
    {
        IMS_TRACE_E(0, "Invalid network type (%d)", nNetworkType, 0, 0);
    }

    return objAni;
}
