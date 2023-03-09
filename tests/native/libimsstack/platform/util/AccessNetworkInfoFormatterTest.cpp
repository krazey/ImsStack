/*
 * Copyright (C) 2023 The Android Open Source Project
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
#include <gtest/gtest.h>

#include "AccessNetworkInfoFormatter.h"
#include "ImsAccessNetworkInfoType.h"

namespace android
{

class AccessNetworkInfoFormatterTest : public ::testing::Test
{
protected:
    virtual void SetUp() override {}

    virtual void TearDown() override {}
};

TEST_F(AccessNetworkInfoFormatterTest, EncodeUsingAccessType)
{
    AString strHeader;
    AccessNetworkInfo objAnInfo;
    AString strCellInfo("4058610011003b932");
    // LTE info
    objAnInfo.nType = AccessNetworkInfo::TYPE_3GPP_E_UTRAN_FDD;

    IMS_MEM_Memcpy(&objAnInfo.uniAI.utran_cell_id_3gpp.acUTRAN_CELL_ID[0], strCellInfo.GetStr(),
            strCellInfo.GetLength());
    ASSERT_TRUE(AccessNetworkInfoFormatter::Encode(objAnInfo, strHeader));

    AString strAccessNetworkValue("3GPP-E-UTRAN-FDD;utran-cell-id-3gpp=4058610011003b932");
    EXPECT_EQ(strHeader, strAccessNetworkValue);

    IMS_MEM_Memset(&objAnInfo.uniAI, 0, sizeof(objAnInfo.uniAI));
    ASSERT_TRUE(AccessNetworkInfoFormatter::Encode(objAnInfo, strHeader, strCellInfo));
    EXPECT_EQ(strHeader, strAccessNetworkValue);

    // NR info
    objAnInfo.nType = AccessNetworkInfo::TYPE_3GPP_NR_TDD;

    IMS_MEM_Memset(&objAnInfo.uniAI, 0, sizeof(objAnInfo.uniAI));
    IMS_MEM_Memcpy(&objAnInfo.uniAI.nr_utran_cell_id_3gpp.acUTRAN_CELL_ID[0], strCellInfo.GetStr(),
            strCellInfo.GetLength());
    ASSERT_TRUE(AccessNetworkInfoFormatter::Encode(objAnInfo, strHeader));

    strAccessNetworkValue = "3GPP-NR-TDD;utran-cell-id-3gpp=4058610011003b932";
    EXPECT_EQ(strHeader, strAccessNetworkValue);

    IMS_MEM_Memset(&objAnInfo.uniAI, 0, sizeof(objAnInfo.uniAI));
    ASSERT_TRUE(AccessNetworkInfoFormatter::Encode(objAnInfo, strHeader, strCellInfo));
    EXPECT_EQ(strHeader, strAccessNetworkValue);

    // 3GPP2_HRPD info
    objAnInfo.nType = AccessNetworkInfo::TYPE_3GPP2_1X_HRPD;

    IMS_MEM_Memset(&objAnInfo.uniAI, 0, sizeof(objAnInfo.uniAI));
    IMS_MEM_Memcpy(
            &objAnInfo.uniAI.ci_3gpp2.acCI[0], strCellInfo.GetStr(), strCellInfo.GetLength());
    ASSERT_TRUE(AccessNetworkInfoFormatter::Encode(objAnInfo, strHeader));

    strAccessNetworkValue = "3GPP2-1X-HRPD;ci-3gpp2=4058610011003b932";
    EXPECT_EQ(strHeader, strAccessNetworkValue);

    IMS_MEM_Memset(&objAnInfo.uniAI, 0, sizeof(objAnInfo.uniAI));
    ASSERT_TRUE(AccessNetworkInfoFormatter::Encode(objAnInfo, strHeader, strCellInfo));
    EXPECT_EQ(strHeader, strAccessNetworkValue);

    IMS_MEM_Memset(&objAnInfo.uniAI, 0, sizeof(objAnInfo.uniAI));
    IMS_MEM_Memset(&objAnInfo.uniAI.ci_3gpp2.acCI[0], 0, ANI_3GPP2_CI_MAX_TOTAL_LEN);
    objAnInfo.uniAI.ci_3gpp2.aSectorId[0] = 1;
    objAnInfo.uniAI.ci_3gpp2.aSectorId[1] = 2;
    objAnInfo.uniAI.ci_3gpp2.aSectorId[2] = 3;
    objAnInfo.uniAI.ci_3gpp2.aSectorId[3] = 4;
    objAnInfo.uniAI.ci_3gpp2.aSectorId[4] = 5;
    objAnInfo.uniAI.ci_3gpp2.aSectorId[5] = 6;

    objAnInfo.uniAI.ci_3gpp2.aSubnetLength[0] = 32;

    objAnInfo.uniAI.ci_3gpp2.aCarrierId[0] = 64;
    objAnInfo.uniAI.ci_3gpp2.aCarrierId[1] = 88;
    objAnInfo.uniAI.ci_3gpp2.aCarrierId[2] = 97;
    strAccessNetworkValue = "3GPP2-1X-HRPD;ci-3gpp2=0102030405060000000000000000000020405861";
    ASSERT_TRUE(AccessNetworkInfoFormatter::Encode(objAnInfo, strHeader));
    EXPECT_EQ(strHeader, strAccessNetworkValue);

    // WLAN info
    objAnInfo.nType = AccessNetworkInfo::TYPE_IEEE_802_11A;

    IMS_MEM_Memset(&objAnInfo.uniAI, 0, sizeof(objAnInfo.uniAI));
    ASSERT_TRUE(AccessNetworkInfoFormatter::Encode(objAnInfo, strHeader, "283b82a9d534"));

    strAccessNetworkValue = "IEEE-802.11a;i-wlan-node-id=283b82a9d534";
    EXPECT_EQ(strHeader, strAccessNetworkValue);

    IMS_MEM_Memset(&objAnInfo.uniAI, 0, sizeof(objAnInfo.uniAI));
    objAnInfo.uniAI.i_wlan_node_id.aMAC[0] = 40;
    objAnInfo.uniAI.i_wlan_node_id.aMAC[1] = 59;
    objAnInfo.uniAI.i_wlan_node_id.aMAC[2] = 130;
    objAnInfo.uniAI.i_wlan_node_id.aMAC[3] = 169;
    objAnInfo.uniAI.i_wlan_node_id.aMAC[4] = 213;
    objAnInfo.uniAI.i_wlan_node_id.aMAC[5] = 52;
    ASSERT_TRUE(AccessNetworkInfoFormatter::Encode(objAnInfo, strHeader));
    EXPECT_EQ(strHeader, strAccessNetworkValue);

    // GERAN info
    objAnInfo.nType = AccessNetworkInfo::TYPE_3GPP_GERAN;

    IMS_MEM_Memset(&objAnInfo.uniAI, 0, sizeof(objAnInfo.uniAI));
    IMS_MEM_Memcpy(
            &objAnInfo.uniAI.cgi_3gpp.acCGI[0], strCellInfo.GetStr(), strCellInfo.GetLength());
    ASSERT_TRUE(AccessNetworkInfoFormatter::Encode(objAnInfo, strHeader));

    strAccessNetworkValue = "3GPP-GERAN;cgi-3gpp=4058610011003b932";
    EXPECT_EQ(strHeader, strAccessNetworkValue);

    IMS_MEM_Memset(&objAnInfo.uniAI, 0, sizeof(objAnInfo.uniAI));
    ASSERT_TRUE(AccessNetworkInfoFormatter::Encode(objAnInfo, strHeader, strCellInfo));
    EXPECT_EQ(strHeader, strAccessNetworkValue);

    objAnInfo.bIsAccessInfoRequired = IMS_FALSE;
    IMS_MEM_Memset(&objAnInfo.uniAI, 0, sizeof(objAnInfo.uniAI));
    ASSERT_TRUE(AccessNetworkInfoFormatter::Encode(objAnInfo, strHeader));
    strAccessNetworkValue = "3GPP-GERAN";
    EXPECT_EQ(strHeader, strAccessNetworkValue);
}

TEST_F(AccessNetworkInfoFormatterTest, EncodeUsingClass)
{
    AString strCellInfo("4058610011003b932");
    AString strHeader;
    AccessNetworkInfo objAnInfo;
    // GERAN info
    objAnInfo.nClass = AccessNetworkInfo::CLASS_3GPP_GERAN;

    IMS_MEM_Memset(&objAnInfo.uniAI, 0, sizeof(objAnInfo.uniAI));
    IMS_MEM_Memcpy(
            &objAnInfo.uniAI.cgi_3gpp.acCGI[0], strCellInfo.GetStr(), strCellInfo.GetLength());
    ASSERT_TRUE(AccessNetworkInfoFormatter::Encode(objAnInfo, strHeader));

    AString strAccessNetworkValue("3GPP-GERAN;cgi-3gpp=4058610011003b932");
    EXPECT_EQ(strHeader, strAccessNetworkValue);

    IMS_MEM_Memset(&objAnInfo.uniAI, 0, sizeof(objAnInfo.uniAI));
    objAnInfo.uniAI.cgi_3gpp.aPLMNId[0] = 64;
    objAnInfo.uniAI.cgi_3gpp.aPLMNId[1] = 88;
    objAnInfo.uniAI.cgi_3gpp.aPLMNId[2] = 97;

    objAnInfo.uniAI.cgi_3gpp.aLAC[0] = 16;
    objAnInfo.uniAI.cgi_3gpp.aLAC[1] = 01;
    objAnInfo.uniAI.cgi_3gpp.aCI[0] = 16;
    objAnInfo.uniAI.cgi_3gpp.aCI[1] = 03;

    strAccessNetworkValue = "3GPP-GERAN;cgi-3gpp=40586110011003";
    ASSERT_TRUE(AccessNetworkInfoFormatter::Encode(objAnInfo, strHeader));
    EXPECT_EQ(strHeader, strAccessNetworkValue);

    // LTE info
    objAnInfo.nClass = AccessNetworkInfo::CLASS_3GPP_E_UTRAN;

    IMS_MEM_Memset(&objAnInfo.uniAI, 0, sizeof(objAnInfo.uniAI));
    IMS_MEM_Memcpy(&objAnInfo.uniAI.utran_cell_id_3gpp.acUTRAN_CELL_ID[0], strCellInfo.GetStr(),
            strCellInfo.GetLength());
    ASSERT_TRUE(AccessNetworkInfoFormatter::Encode(objAnInfo, strHeader));

    strAccessNetworkValue = "3GPP-E-UTRAN;utran-cell-id-3gpp=4058610011003b932";
    EXPECT_EQ(strHeader, strAccessNetworkValue);

    IMS_MEM_Memset(&objAnInfo.uniAI, 0, sizeof(objAnInfo.uniAI));
    objAnInfo.uniAI.utran_cell_id_3gpp.aPLMNId[0] = 64;
    objAnInfo.uniAI.utran_cell_id_3gpp.aPLMNId[1] = 88;
    objAnInfo.uniAI.utran_cell_id_3gpp.aPLMNId[2] = 97;

    objAnInfo.uniAI.utran_cell_id_3gpp.aLACorTAC[0] = 00;
    objAnInfo.uniAI.utran_cell_id_3gpp.aLACorTAC[1] = 17;

    objAnInfo.uniAI.utran_cell_id_3gpp.aCellId[0] = 00;
    objAnInfo.uniAI.utran_cell_id_3gpp.aCellId[1] = 59;
    objAnInfo.uniAI.utran_cell_id_3gpp.aCellId[2] = 147;
    objAnInfo.uniAI.utran_cell_id_3gpp.aCellId[3] = 02;

    ASSERT_TRUE(AccessNetworkInfoFormatter::Encode(objAnInfo, strHeader));
    EXPECT_EQ(strHeader, strAccessNetworkValue);

    // NR info
    objAnInfo.nClass = AccessNetworkInfo::CLASS_3GPP_NR;

    IMS_MEM_Memset(&objAnInfo.uniAI, 0, sizeof(objAnInfo.uniAI));
    IMS_MEM_Memcpy(&objAnInfo.uniAI.nr_utran_cell_id_3gpp.acUTRAN_CELL_ID[0], strCellInfo.GetStr(),
            strCellInfo.GetLength());
    ASSERT_TRUE(AccessNetworkInfoFormatter::Encode(objAnInfo, strHeader));

    strAccessNetworkValue = "3GPP-NR;utran-cell-id-3gpp=4058610011003b932";
    EXPECT_EQ(strHeader, strAccessNetworkValue);

    IMS_MEM_Memset(&objAnInfo.uniAI, 0, sizeof(objAnInfo.uniAI));
    objAnInfo.uniAI.nr_utran_cell_id_3gpp.aPLMNId[0] = 64;
    objAnInfo.uniAI.nr_utran_cell_id_3gpp.aPLMNId[1] = 88;
    objAnInfo.uniAI.nr_utran_cell_id_3gpp.aPLMNId[2] = 97;

    objAnInfo.uniAI.nr_utran_cell_id_3gpp.aTAC[0] = 00;
    objAnInfo.uniAI.nr_utran_cell_id_3gpp.aTAC[1] = 17;
    objAnInfo.uniAI.nr_utran_cell_id_3gpp.aTAC[2] = 00;

    objAnInfo.uniAI.nr_utran_cell_id_3gpp.aCellId[0] = 59;
    objAnInfo.uniAI.nr_utran_cell_id_3gpp.aCellId[1] = 147;
    objAnInfo.uniAI.nr_utran_cell_id_3gpp.aCellId[2] = 02;
    objAnInfo.uniAI.nr_utran_cell_id_3gpp.aCellId[3] = 02;
    objAnInfo.uniAI.nr_utran_cell_id_3gpp.aCellId[4] = 04;

    strAccessNetworkValue = "3GPP-NR;utran-cell-id-3gpp=4058610011003b9302024";
    ASSERT_TRUE(AccessNetworkInfoFormatter::Encode(objAnInfo, strHeader));
    EXPECT_EQ(strHeader, strAccessNetworkValue);

    // WLan info
    objAnInfo.nClass = AccessNetworkInfo::CLASS_3GPP_WLAN;

    IMS_MEM_Memset(&objAnInfo.uniAI, 0, sizeof(objAnInfo.uniAI));
    ASSERT_TRUE(AccessNetworkInfoFormatter::Encode(objAnInfo, strHeader, "283b82a9d534"));

    strAccessNetworkValue = "3GPP-WLAN;i-wlan-node-id=283b82a9d534";
    EXPECT_EQ(strHeader, strAccessNetworkValue);

    IMS_MEM_Memset(&objAnInfo.uniAI, 0, sizeof(objAnInfo.uniAI));
    IMS_MEM_Memcpy(
            &objAnInfo.uniAI.i_wlan_node_id.acMAC[0], "283b82a9d534", strlen("283b82a9d534"));
    ASSERT_TRUE(AccessNetworkInfoFormatter::Encode(objAnInfo, strHeader));
    EXPECT_EQ(strHeader, strAccessNetworkValue);

    objAnInfo.bIsAccessInfoRequired = IMS_FALSE;
    IMS_MEM_Memset(&objAnInfo.uniAI, 0, sizeof(objAnInfo.uniAI));
    ASSERT_TRUE(AccessNetworkInfoFormatter::Encode(objAnInfo, strHeader));
    strAccessNetworkValue = "3GPP-WLAN";
    EXPECT_EQ(strHeader, strAccessNetworkValue);
}

TEST_F(AccessNetworkInfoFormatterTest, EncodeUsingMaxValues)
{
    AString strCellInfo("4058610011003b932");
    AString strHeader;
    AccessNetworkInfo objAnInfo;
    objAnInfo.nType = AccessNetworkInfo::TYPE_MAX;

    IMS_MEM_Memcpy(&objAnInfo.uniAI.nr_utran_cell_id_3gpp.acUTRAN_CELL_ID[0], strCellInfo.GetStr(),
            strCellInfo.GetLength());
    ASSERT_FALSE(AccessNetworkInfoFormatter::Encode(objAnInfo, strHeader));

    EXPECT_TRUE(strHeader.IsNull());

    objAnInfo.nType = AccessNetworkInfo::TYPE_3GPP_E_UTRAN_FDD;
    objAnInfo.nClass = AccessNetworkInfo::CLASS_MAX;

    ASSERT_FALSE(AccessNetworkInfoFormatter::Encode(objAnInfo, strHeader));
    EXPECT_TRUE(strHeader.IsNull());

    objAnInfo.nType = AccessNetworkInfo::TYPE_NONE;
    objAnInfo.nClass = AccessNetworkInfo::CLASS_NONE;

    ASSERT_FALSE(AccessNetworkInfoFormatter::Encode(objAnInfo, strHeader));
    EXPECT_TRUE(strHeader.IsNull());
}

}  // namespace android
