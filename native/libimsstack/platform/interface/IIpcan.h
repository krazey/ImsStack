/*
    Author
    <table>
    date      author                    description
    --------  --------------            ----------
    20140422  hwangoo.park@             Created
    </table>

    Description
     This class defines the APIs for the IP connectivity access network information.

*/

#ifndef _INTERFACE_IMS_IP_CAN_H_
#define _INTERFACE_IMS_IP_CAN_H_

#include "ByteArray.h"
#include "ImsAccessNetworkInfoType.h"



class IIPCAN
{
public:
    /*
     Returns the access network information for SIP signalling.

    Parameters
    <table>
    parameter               description
    ----------              ----------
    nSlotId                 Slot id
    objANI                  Access network info. of the current network
                            If nType is set to XXX_E_UTRAN_XXX, it will be used to identify
                            the current network type as LTE when network type is not identified.
    </table>

    Returns
    <table>
    return                  description
    ----------              ----------
    </table>
    */
    virtual void GetAccessInfo(IN IMS_SINT32 nSlotId, IN_OUT AccessNetworkInfo &objANI) = 0;

    /*
     Returns the WiFi access network information for SIP signalling.

    Parameters
    <table>
    parameter               description
    ----------              ----------
    objANI                  Access network info. of the current network
    </table>

    Returns
    <table>
    return                  description
    ----------              ----------
    </table>
    */
    virtual void GetAccessInfoForWiFi(OUT AccessNetworkInfo &objANI) = 0;

    /*
     Returns the last access network information for SIP signalling.

    Parameters
    <table>
    parameter               description
    ----------              ----------
    nSlotId                 Slot id
    objANI                  Access network info. of the current network
    strTimestamp            Timestamp(UTC) of last known access network info.
    strCellInfoAge          Cell-Info age as seconds
    </table>

    Returns
    <table>
    return                  description
    ----------              ----------
    </table>
    */
    virtual void GetLastAccessInfo(IN IMS_SINT32 nSlotId, OUT AccessNetworkInfo &objANI,
            OUT AString &strTimestamp, OUT AString &strCellInfoAge) = 0;

    /*
     Returns the WiFi access network information for SIP signalling.

    Parameters
    <table>
    parameter               description
    ----------              ----------
    objANI                  Access network info. of the current network
    strTimestamp            Timestamp(UTC) of last known access network info.
    strCellInfoAge          Cell-Info age as seconds
    </table>

    Returns
    <table>
    return                  description
    ----------              ----------
    </table>
    */
    virtual void GetLastAccessInfoForWiFi(OUT AccessNetworkInfo &objANI,
            OUT AString &strTimestamp, OUT AString &strCellInfoAge) = 0;

    /*
     Returns the current network type.

    Parameters
    <table>
    parameter               description
    ----------              ----------
    nSlotId                 Slot id
    </table>

    Returns
    <table>
    return                  description
    ----------              ----------
    IMS_SINT32              Network type (TYPE_XXX)
    </table>
    */
    virtual IMS_SINT32 GetNetworkType(IN IMS_SINT32 nSlotId) = 0;

public:
    // Category of access network
    enum
    {
        CATEGORY_MOBILE = 0,
        CATEGORY_WLAN,
        CATEGORY_ANY
    };

    // Type of access network
    enum
    {
        TYPE_INVALID = (-1),

        TYPE_UNKNOWN = 0,
        TYPE_GPRS = 1,
        TYPE_EDGE = 2,
        TYPE_UMTS = 3,
        TYPE_CDMA = 4,
        TYPE_EVDO_0 = 5,
        TYPE_EVDO_A = 6,
        TYPE_1XRTT = 7,
        TYPE_HSDPA = 8,
        TYPE_HSUPA = 9,
        TYPE_HSPA = 10,
        TYPE_IDEN = 11,
        TYPE_EVDO_B = 12,
        TYPE_LTE = 13,
        TYPE_EHRPD = 14,
        TYPE_HSPAP = 15,
        TYPE_GSM = 16,
        TYPE_TD_SCDMA = 17,
        TYPE_IWLAN = 18,
        TYPE_LTE_CA = 19,
        TYPE_NR = 20
    };
};

#endif // _INTERFACE_IMS_IP_CAN_H_
