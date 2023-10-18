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
#ifndef INTERFACE_IPCAN_H_
#define INTERFACE_IPCAN_H_

#include "ByteArray.h"
#include "ImsAccessNetworkInfoType.h"

class IIpcan
{
protected:
    virtual ~IIpcan() = default;

public:
    /**
     * @brief Returns the access network information for SIP signalling.
     *
     * @param nSlotId The slot-id
     * @param objAni The access network information of the current network
     *               If nType is set to XXX_E_UTRAN_XXX, it will be used to identify
     *               the current network type as LTE when network type is not identified.
     */
    virtual void GetAccessInfo(IN IMS_SINT32 nSlotId, IN_OUT AccessNetworkInfo& objAni) = 0;

    /**
     * @brief Returns the WiFi access network information for SIP signalling.
     *
     * @param objAni The access network information of the current network
     */
    virtual void GetAccessInfoForWiFi(OUT AccessNetworkInfo& objAni) = 0;

    /**
     * @brief Returns the last access network information for SIP signalling.
     *
     * @param nSlotId The slot-id
     * @param objAni The access network information of the current network
     * @param strTimestamp The timestamp(UTC) of last known access network information
     * @param strCellInfoAge The cell-info age as seconds
     */
    virtual void GetLastAccessInfo(IN IMS_SINT32 nSlotId, OUT AccessNetworkInfo& objAni,
            OUT AString& strTimestamp, OUT AString& strCellInfoAge) = 0;

    /**
     * @brief Returns the WiFi access network information for SIP signalling.
     *
     * @param objAni The access network information of the current network
     * @param strTimestamp The timestamp(UTC) of last known access network information
     * @param strCellInfoAge The cell-info age as seconds
     */
    virtual void GetLastAccessInfoForWiFi(OUT AccessNetworkInfo& objAni, OUT AString& strTimestamp,
            OUT AString& strCellInfoAge) = 0;

    /**
     * @brief Returns the current network type.
     *
     * @param nSlotId The slot-id
     * @return The network type.\n
     *         #TYPE_UNKNOWN\n
     *         #TYPE_GPRS\n
     *         #TYPE_EDGE\n
     *         #TYPE_UMTS\n
     *         #TYPE_CDMA\n
     *         #TYPE_EVDO_0\n
     *         #TYPE_EVDO_A\n
     *         #TYPE_1XRTT\n
     *         #TYPE_HSDPA\n
     *         #TYPE_HSUPA\n
     *         #TYPE_HSPA\n
     *         #TYPE_IDEN\n
     *         #TYPE_EVDO_B\n
     *         #TYPE_LTE\n
     *         #TYPE_EHRPD\n
     *         #TYPE_HSPAP\n
     *         #TYPE_GSM\n
     *         #TYPE_TD_SCDMA\n
     *         #TYPE_IWLAN\n
     *         #TYPE_LTE_CA\n
     *         #TYPE_NR\n
     */
    virtual IMS_SINT32 GetNetworkType(IN IMS_SINT32 nSlotId) = 0;

public:
    /// Category of access network
    enum
    {
        CATEGORY_MOBILE = 0,
        CATEGORY_WLAN,
        CATEGORY_ANY
    };

    /// Type of access network
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

#endif
