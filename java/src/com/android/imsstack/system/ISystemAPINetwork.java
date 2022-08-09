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
package com.android.imsstack.system;

import com.android.imsstack.core.agents.dcmif.IDcUtils;

import java.io.FileDescriptor;

public interface ISystemAPINetwork {

    /**
     * Try to activate data connection for the given type of apn
     *
     * @param apnType apn type to connect
     * @return 1 if it requested to framework
     */
    int activateDataConnection4Sys(int apnType);

    /**
     * Try to deactivate data connection for the given type of apn
     *
     * @param apnType apn type to connect
     * @return 1 if it requested to framework
     */
    int deactivateDataConnection4Sys(int apnType);

    /**
     * Returns the access network information of the network that the IMS is registering
     * or was registered.
     *
     * @param defaultNetworkType The default network type when the network is unknown.
     * @return The access network information.
     */
    IDcUtils.AccessNetworkInfo getAccessNetworkInfo4Sys(int defaultNetworkType);
    String getApnName4Sys(int apnType);
    int getDataConnectionState4Sys(int apnType);
    String[] getHostByName4Sys(int apnType, int ipVersion, String host);
    int getIfaceId4Sys(int apnType);
    String getIfaceName4Sys(int apnType);
    int getIpcanCategory4Sys(int apnType);
    String[] getLastAccessNetworkInfo4Sys(int networkType);
    String getLocalAddress4Sys(int apnType, int ipVersion);
    int getLteRsrpStrength4Sys();
    String[] getPcscfAddresses4Sys(int apnType, int ipVersion);
    int getRoamingState4Sys();
    int getVoiceRoamingType4Sys();
    int getDataRoamingType4Sys();
    int getServiceState4Sys();
    int getVoiceServiceState4Sys();
    int isLteEmergencyOnly4Sys();
    int isEmergencyAttachSupported4Sys();
    int getMocnPlmnInfo4Sys();
    boolean isMobileDataEnabled();
    int getMtu4Sys(int apnType);
    int bindSocket(int apnType, FileDescriptor sockFd);
}
