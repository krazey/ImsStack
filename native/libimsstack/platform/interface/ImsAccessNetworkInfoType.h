/*
    Author
    <table>
    date      author                    description
    --------  --------------            ----------
    20101005  hwangoo.park@             Created
    </table>

    Description
    This file defines the data structures for Access Network Info. which the device is attached.
    It defines only the wireless requirements : 3GPP / 3GPP2 / WLAN.
    DSL / Ethernet related information is excluded in this file.
*/

#ifndef _IMS_ACCESS_NETWORK_INFO_TYPE_H_
#define _IMS_ACCESS_NETWORK_INFO_TYPE_H_

#include "IMSNew.h"

#define ANI_MODE_FDD "FDD"
#define ANI_MODE_TDD "TDD"

// 3GPP {
#define ANI_3GPP_MAX_PLMN 3
#define ANI_3GPP_MAX_LAC_TAC 2
#define ANI_3GPP_CGI_MAX_CI 2
#define ANI_3GPP_CGI_MAX_TOTAL_LEN 14 // hexadecimal digits excluding null character
#define ANI_3GPP_UTRAN_CELL_ID_MAX_CELL_ID 4
#define ANI_3GPP_UTRAN_CELL_ID_MAX_TOTAL_LEN 17 // hexadecimal digits excluding null character

// hexadecimal digits representations
#define ANI_3GPP_MCC_MAX_LEN 3
#define ANI_3GPP_MNC_MAX_LEN 3
#define ANI_3GPP_LAC_MAX_LEN 4
#define ANI_3GPP_GERAN_CELL_ID_MAX_LEN 4
#define ANI_3GPP_TAC_MAX_LEN 4
#define ANI_3GPP_UTRAN_CELL_ID_MAX_LEN 7

struct CGI_3GPP
{
    // 3GPP-GERAN

    // PLMN identity : MCC(3)  + MNC(2 or 3)
    IMS_UINT8 aPLMNId[ANI_3GPP_MAX_PLMN];
    // LAC : fixed length code of 16 bits using full hexadecimal representation
    IMS_UINT8 aLAC[ANI_3GPP_MAX_LAC_TAC];
    // CI : fixed length code of 16 bits using a full hexadecimal representation
    IMS_UINT8 aCI[ANI_3GPP_CGI_MAX_CI];

    // String value : MCC (3), MNC (2 ~ 3), LAC (4), CI (4), null character
    IMS_CHAR acCGI[ANI_3GPP_CGI_MAX_TOTAL_LEN + 1];
};

struct UTRAN_CELL_ID_3GPP
{
    // 3GPP-UTRAN-FDD / 3GPP-UTRAN-TDD / 3GPP-E-UTRAN-FDD / 3GPP-E-UTRAN-TDD

    // PLMN identity : MCC (3) + MNC (2 or 3)
    IMS_UINT8 aPLMNId[ANI_3GPP_MAX_PLMN];
    // 3GPP-UTRAN - LAC : fixed length code of 16 bits using full hexadecimal representation
    // 3GPP-E-UTRAN - TAC : fixed length code of 16 bits using full hexadecimal representation
    IMS_UINT8 aLACorTAC[ANI_3GPP_MAX_LAC_TAC];
    // 3GPP-UTRAN -
    // UMTS Cell Identity : fixed length code of 28 bits using a full hexadecimal representation
    // 3GPP-E-UTRAN -
    // Cell Identity : fixed length code of 28 bits using a full hexadecimal representation
    // Lower 4 bits of the last array value will not be used
    IMS_UINT8 aCellId[ANI_3GPP_UTRAN_CELL_ID_MAX_CELL_ID];

    // String value : MCC (3), MNC (2 ~ 3), LAC or TAC (4), Cell Id (7), null character
    IMS_CHAR acUTRAN_CELL_ID[ANI_3GPP_UTRAN_CELL_ID_MAX_TOTAL_LEN + 1];
};

#define ANI_3GPP_NR_MAX_TAC 3
#define ANI_3GPP_NR_MAX_CELL_ID 5
#define ANI_3GPP_NR_UTRAN_CELL_ID_MAX_TOTAL_LEN 21 // hexadecimal digits excluding null character

// hexadecimal digits representations
#define ANI_3GPP_NR_TAC_MAX_LEN 6
#define ANI_3GPP_NR_CELL_ID_MAX_LEN 9

struct NR_UTRAN_CELL_ID_3GPP
{
    // 3GPP-NR-FDD / 3GPP-NR-TDD

    // PLMN identity : MCC (3) + MNC (2 or 3)
    IMS_UINT8 aPLMNId[ANI_3GPP_MAX_PLMN];
    // TAC: fixed length code of 24 bits using full hexadecimal representation
    IMS_UINT8 aTAC[ANI_3GPP_NR_MAX_TAC];
    // NR Cell Identity : fixed length code of 36 bits using a full hexadecimal representation
    // Lower 4 bits of the last array value will not be used
    IMS_UINT8 aCellId[ANI_3GPP_NR_MAX_CELL_ID];

    // String value : MCC (3), MNC (2 ~ 3), TAC (6), Cell Id (9), null character
    IMS_CHAR acUTRAN_CELL_ID[ANI_3GPP_NR_UTRAN_CELL_ID_MAX_TOTAL_LEN + 1];
};
// }

// 3GPP2 {
#define ANI_3GPP2_MAX_SECTOR_ID 16
#define ANI_3GPP2_MAX_SUBNET_LENGTH 1
#define ANI_3GPP2_MAX_CARRIER_ID 3
#define ANI_3GPP2_CI_MAX_TOTAL_LEN 40 // hexadecimal digits excluding null character

// hexadecimal digits representations
#define ANI_3GPP2_SECTOR_ID_MAX_LEN 32
#define ANI_3GPP2_SUBNET_LENGTH_MAX_LEN 2

struct CI_3GPP2
{
    // Uppercase Hexadecimal Representation
    // 3GPP2-1X : SID (16bits), NID (16bits), PZID (8bits), BASE_ID (16bits)
    //            The first 7 bytes of aSectorId is only used
    // 3GPP2-1X-HRPD : All fields are used (34 or 40 hexadecimal)
    // 3GPP2-UMB : aSectorId is only used

    // Sector ID (128 bits) : hexadecimal representation
    IMS_UINT8 aSectorId[ANI_3GPP2_MAX_SECTOR_ID];
    // Subnet length : hexadecimal representation
    IMS_UINT8 aSubnetLength[ANI_3GPP2_MAX_SUBNET_LENGTH];
    // Carrier-ID (optional field) : MCC (3) + MNC (2 or 3) - hexadecimal representation
    IMS_UINT8 aCarrierId[ANI_3GPP2_MAX_CARRIER_ID];

    // String value : Sector Id (32), Subnet Length (2), Carrier-Id (5 ~ 6), null character
    IMS_CHAR acCI[ANI_3GPP2_CI_MAX_TOTAL_LEN + 1];
};
// }

// WLAN {
#define ANI_WLAN_MAX_MAC 6
#define ANI_WLAN_MAC_MAX_LEN 12 // hexadecimal digits excluding null character
#define ANI_WLAN_SSID_MAX_LEN 32 // string token excluding null character

struct I_WLAN_NODE_ID
{
    // IEEE-802.11 / IEEE-802.11a / IEEE-802.11b / IEEE-802.11g / IEEE-802.11n

    // AP's MAC w/o delimiter
    IMS_UINT8 aMAC[ANI_WLAN_MAX_MAC];

    // String value : MAC (12), null character
    IMS_CHAR acMAC[ANI_WLAN_MAC_MAX_LEN + 1];

    // String value : SSID (32), null character
    IMS_CHAR acSSID[ANI_WLAN_SSID_MAX_LEN + 1];
};
// }

// Accesss Network Info class
class AccessNetworkInfo
{
public:
    inline AccessNetworkInfo()
        : nType(TYPE_NONE)
        , nClass(CLASS_NONE)
        , bIsAccessInfoRequired(IMS_TRUE)
    {
        IMS_MEM_Memset(&uniAI, 0, sizeof(uniAI));
    }

public:
    // Access-type
    enum
    {
        TYPE_NONE = 0,

        // 3GPP
        TYPE_3GPP_GERAN,
        TYPE_3GPP_UTRAN_FDD,
        TYPE_3GPP_UTRAN_TDD,
        TYPE_3GPP_E_UTRAN_FDD,
        TYPE_3GPP_E_UTRAN_TDD,
        TYPE_3GPP_NR_FDD,
        TYPE_3GPP_NR_TDD,

        // 3GPP2
        TYPE_3GPP2_1X,
        TYPE_3GPP2_1X_HRPD,
        TYPE_3GPP2_UMB,

        // WIFI
        TYPE_IEEE_802_11,
        TYPE_IEEE_802_11A,
        TYPE_IEEE_802_11B,
        TYPE_IEEE_802_11G,
        TYPE_IEEE_802_11N,

        TYPE_DOCSIS,

        TYPE_MAX
    };

    // Access-class
    enum
    {
        CLASS_NONE = 0,

        CLASS_3GPP_GERAN,
        CLASS_3GPP_UTRAN,
        CLASS_3GPP_E_UTRAN,
        CLASS_3GPP_NR,
        CLASS_3GPP_WLAN,
        CLASS_3GPP_GAN,
        CLASS_3GPP_HSPA,

        CLASS_MAX
    };

    // Presence of string value; acAI[]

    // Access Type
    IMS_SINT32 nType;
    // Access Class
    IMS_SINT32 nClass;

    // Access Info
    union AccessInfo
    {
        CGI_3GPP cgi_3gpp;
        UTRAN_CELL_ID_3GPP utran_cell_id_3gpp;
        NR_UTRAN_CELL_ID_3GPP nr_utran_cell_id_3gpp;
        CI_3GPP2 ci_3gpp2;
        I_WLAN_NODE_ID i_wlan_node_id;
    };

    // Flag to indicate if the additional information should be added or not
    IMS_BOOL bIsAccessInfoRequired;

    AccessInfo uniAI;
};

#endif // _IMS_ACCESS_NETWORK_INFO_TYPE_H_
