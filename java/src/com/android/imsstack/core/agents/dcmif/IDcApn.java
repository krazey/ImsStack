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
     * Try to connect the given type of apn. Scenario 1. Usually, always on type
     * of APN such as "ims" automatically is opened/connected by android data
     * kennel layer. However in certain case(abnormally) ; eg. when IMS process
     * crashed by another apps, the specific APN probably remains the zombie and
     * Android system may disconnect the apn. Under this kind of situation, we
     * need to re-establish APN asap. Scenario 2. In case of
     * "on demand type of APN" such as "emergency", we have to open/connect apn
     * before use it.
     *
     * @param apnType EApnType.IMS.getType()
     *             EApnType.EMERGENCY.getType()
     *             EApnType.INTERNET.getType()
     * @param ipcanType IApn.IPCAN_CATEGORY_MOBILE
     *             IApn.IPCAN_CATEGORY_WLAN
     * @return true: No need to open. It already opened or we called
     *         "startUsingNetworkFeature" (android api) regardless of result to
     *         connect apn. false: we cannot call "startUsingNetworkFeature". It
     *         is expected current apn type to be disconnected at this moment.
     *         false:
     */
    boolean connect(int apnType, int ipcanType);

    /**
     * disconnect
     *
     * @see EApnType#IMS
     * @see EApnType#EMERGENCY
     * @see EApnType#INTERNET
     *
     * @param apnType EApnType.IMS...
     * @param nTimeAfterRecover
     *                 > 0 : re-connect APN after millisecond
     *                 == 0 : re-connect APN after 1000 millisecond
     *                 < 0 : disconnect && DO NOT Recover this PDN before call connect EXPLICITLY.
     * @param ipcanType
     *                 IApn.IPCAN_CATEGORY_MOBILE or IApn.IPCAN_CATEGORY_WLAN
     * @return
     */
    void disconnect(int apnType, int nTimeAfterRecover, int ipcanType);

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

    // jaesik.kong 20140319 - get APN IfaceName START
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
    // jaesik.kong 20140319 - get APN IfaceName END

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
     *
     * @return
     */
    void setApn(int apnType, IApn apn);

    /**
     * getApnControl
     *
     * @return
     */
    IApn getApnControl(int apnType);

    /**
     * getNetworkByCapability
     *
     * getNetworkForType() API in connectivityManager is going to deprecated in API level 23
     * This API can be replace with getAllNetworks() in ConnectivityManager.
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

    /**
     * getNetworkByCapabilityWithTransportType
     *
     * This API is mode to get a certain network associated with capability and transport type.
     * It could be used when the transport type should be considered.
     * ex. In case of both Wi-Fi and INTERNET PDN are connected.
     *
     * @param
     *         apnType
     *             EApnType.IMS.getType()
     *             EApnType.INTERNET.getType()
     *             EApnType.XCAP.getType()
     *             EApnType.EMERGENCY.getType()
     *          transportType
     *             NetworkCapabilities.TRANSPORT_CELLULAR = 0;
     *             NetworkCapabilities.TRANSPORT_WIFI = 1;
     *             NetworkCapabilities.TRANSPORT_BLUETOOTH = 2;
     *             NetworkCapabilities.TRANSPORT_ETHERNET = 3;
     *             NetworkCapabilities.TRANSPORT_VPN = 4;
     *             NetworkCapabilities.TRANSPORT_WIFI_AWARE = 5;
     *             NetworkCapabilities.TRANSPORT_LOWPAN = 6;
     *             NetworkCapabilities.TRANSPORT_TEST = 7;
     * @return
     *         Network object
     */
    Network getNetworkByCapabilityWithTransportType(int apnType, int transportType);

    /**
     * changeApnPermission
     *
     * This API is made to change Apn Permission for operator/service specific.
     * This API can be called whenever after DCApn & Apn objected is created.
     * It change block reason in target apn's connect() request.
     * so, if apn is not employed, that apn doesn't request network to connect.
     *
     * @param
     *         EApnType apnType
     *         boolean enable
     * @return
     */
    void changeApnEmployState(EApnType apnType, boolean enable);
}
