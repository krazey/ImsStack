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

package com.android.imsstack.core.agents.dcmif;

import android.net.Network;

import java.io.FileDescriptor;

/**
 * This provides interface related with operations for APN.
 */
public interface IDcApn extends IDc {
    /**
     * Try to connect the given type of apn.
     * Scenario 1. Basically, IMS type APN is requested to connect
     * as a always on concept if IMS service is enabled.
     * Scenario 2. In case of "on demand type of APN" such as "emergency",
     * we have to open/connect apn before use it.
     *
     * @param apnType DcConstants.TYPE_IMS
     *                DcConstants.TYPE_EMERGENCY
     *                DcConstants.TYPE_INTERNET
     *                DcConstants.TYPE_XCAP
     * @return true: It already opened, or we called "requestNetwork" (android api)
     *               regardless of result to connect apn.
     *         false: we cannot call "requestNetwork".
     *                It is expected current apn type to be disconnected at this moment.
     */
    boolean connect(int apnType);

    /**
     * disconnect
     *
     * @see EApnType#IMS
     * @see EApnType#EMERGENCY
     * @see EApnType#INTERNET
     *
     * @param apnType DcConstants.TYPE_IMS
     *                DcConstants.TYPE_EMERGENCY
     *                DcConstants.TYPE_INTERNET
     *                DcConstants.TYPE_XCAP
     * @return true: we called "releaseNetwork" (android api) to disconnect.
     *         false: This type of apn has never been requested to connect.
     */
    boolean disconnect(int apnType);

    /**
     * get data state of APN
     *
     * @param apnType EApnType.IMS.getType()
     *             EApnType.EMERGENCY.getType()
     *             EApnType.INTERNET.getType()
     * @return
     *         EDataState.DATA_STATE_CONNECTED.getState()
     *         EDataState.DATA_STATE_DISCONNECTED.getState()
     *         EDataState.DATA_STATE_CONNECT_FAILED.getState()
     *         EDataState.DATA_STATE_IP_CHANGED.getState()
     */
    int getDataState(int apnType);


    /**
     *
     * @param apnType EApnType.IMS.getType()
     *             EApnType.EMERGENCY.getType()
     *             EApnType.INTERNET.getType()
     * @return
     *         boolean
     */
    boolean isConnected(int apnType);


    /**
     * getApn
     *
     * @param apnType
     *             EApnType.IMS.getType()
     *             EApnType.EMERGENCY.getType()
     *             EApnType.INTERNET.getType()
     * @return
     *         apn string
     *
     */
    String getApn(int apnType);

    /**
     * getHostByName
     *
     * @param apnType
     *             EApnType.IMS.getType()
     *             EApnType.EMERGENCY.getType()
     *             EApnType.INTERNET.getType()
     * @param
     *         ipVersion
     *             EIpVersion.IPV4 (4)
     *             EIpVersion.IPV6 (6)
     * @return
     *         the resolved IP addresses
     *
     */
    String[] getHostByName(int apnType, int ipVersion, String host);

    /**
     * getIfaceId
     *
     * @param apnType
     *             EApnType.IMS.getType()
     *             EApnType.EMERGENCY.getType()
     *             EApnType.INTERNET.getType()
     * @return
     *         network interface identifier
     *
     */
    int getIfaceId(int apnType);

    /**
     * getIfaceName
     *
     * @param apnType
     *             EApnType.IMS.getType()
     *             EApnType.EMERGENCY.getType()
     *             EApnType.INTERNET.getType()
     * @return
     *         iface string
     *
     */
    String getIfaceName(int apnType);

    /**
     * Returns IPCAN category of this APN.
     *
     * @param
     *         apnType
     *             EApnType.IMS.getType()
     *             EApnType.EMERGENCY.getType()
     *             EApnType.INTERNET.getType()
     * @return
     *         IApn.IPCAN_CATEGORY_MOBILE
     *         IApn.IPCAN_CATEGORY_WLAN
     */
    int getIpcanCategory(int apnType);

    /**
     *
     *
     * @param
     *         apnType
     *             EApnType.IMS.getType()
     *             EApnType.EMERGENCY.getType()
     *             EApnType.INTERNET.getType()
     * @param
     *         ipVersion
     *             EIpVersion.IPV4 (4)
     *             EIpVersion.IPV6 (6)
     *             EIpVersion.IPV4V6 (46)
     *             EIpVersion.IPV6V4 (64)
     * @return
     *         ipv4 or ipv6 address string
     */
    String getLocalAddress(int apnType, int ipVersion);

    /**
     *
     *
     * @param
     *         apnType
     *             EApnType.IMS.getType()
     *             EApnType.EMERGENCY.getType()
     *             EApnType.INTERNET.getType()
     * @return
     *         ipv4 or ipv6 address string
     */
    String getCachedLocalAddress(int apnType);

    /**
     *
     *
     * @param
     *         apnType
     *             EApnType.IMS.getType()
     *             EApnType.EMERGENCY.getType()
     *             EApnType.INTERNET.getType()
     * @param
     *         ipVersion
     *             EIpVersion.IPV4 (4)
     *             EIpVersion.IPV6 (6)
     *             EIpVersion.IPV4V6 (46)
     *             EIpVersion.IPV6V4 (64)
     * @return
     *         string array which contains PCSCF addresses
     *
     */
    String[] getPcscfAddress(int apnType, int ipVersion);

    /**
     *
     *
     * @param
     *         apnType
     *             EApnType.IMS.getType()
     *             EApnType.EMERGENCY.getType()
     *             EApnType.INTERNET.getType()
     * @return
     *         MTU (maximum transmission unit) size of the apn
     *
     */
    int getMtu(int apnType);

    /**
     * Binds the specified {@link FileDescriptor} to this APN. All data traffic on the
     * socket represented by this file descriptor will be sent on this APN.
     *
     * @param apnType The APN type to be bound
     *                #EApnType#IMS#getType()
     *                #EApnType#EMERGENCY#getType()
     *                #EApnType#INTERNET#getType()
     * @param sockFd The file descriptor for socket
     * @return Returns 1 if the operation is succeeded. Otherwise, returns 0.
     */
    int bindSocket(int apnType, FileDescriptor sockFd);

    /**
     * setApn
     */
    void setApn(int apnType, IApn apn);

    /**
     * getApnControl
     *
     * @return Returns Apn according to apnType, such as
     * {@link com.android.imsstack.core.agents.dcm.ApnIms} for IMS type or
     * {@link com.android.imsstack.core.agents.dcm.ApnEmergency} for EMERGENCY type.
     * @see EApnType
     */
    IApn getApnControl(int apnType);

    /**
     * getNetworkByCapability
     *
     * getNetworkForType() API in connectivityManager is going to be deprecated in API level 23
     * This API can be replaced with getAllNetworks() in ConnectivityManager.
     *
     * Network capability information is required to use new API as getNetworkForType() API.
     * And Network capability information is added in EApnType class.
     *
     * @param
     *         apnType
     *             EApnType.IMS.getType()
     *             EApnType.INTERNET.getType()
     *             EApnType.XCAP.getType()
     *             EApnType.EMERGENCY.getType()
     * @return
     *         Network object
     */
    Network getNetworkByCapability(int apnType);
}
