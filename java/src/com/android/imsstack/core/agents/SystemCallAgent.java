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

package com.android.imsstack.core.agents;

import static android.telephony.TelephonyManager.CALL_STATE_IDLE;
import static android.telephony.TelephonyManager.NETWORK_TYPE_UNKNOWN;

import android.telephony.Annotation.CallState;
import android.telephony.Annotation.NetworkType;
import android.telephony.ServiceState;

import com.android.imsstack.core.agents.dcm.DcFactory;
import com.android.imsstack.core.agents.dcmif.EDataState;
import com.android.imsstack.core.agents.dcmif.IApn;
import com.android.imsstack.core.agents.dcmif.IDcApn;
import com.android.imsstack.core.agents.dcmif.IDcNetWatcher;
import com.android.imsstack.core.agents.dcmif.IDcUtils;
import com.android.imsstack.core.config.CarrierConfig;
import com.android.imsstack.system.ISystem;
import com.android.imsstack.system.IpSecSaParameter;
import com.android.imsstack.system.SystemCallInterface;
import com.android.imsstack.system.SystemInterface;

import java.io.FileDescriptor;

public class SystemCallAgent implements SystemCallInterface {
    private final int mSlotId;

    public SystemCallAgent(int slotId) {
        mSlotId = slotId;

        setSystemCallInterface(this);
    }

    public void destroy() {
        setSystemCallInterface(null);
    }

    public void setSystemCallInterface(SystemCallInterface systemCall) {
        ISystem system = SystemInterface.getInstance().getSystem(mSlotId);

        if (system != null) {
            system.setSystemCallInterface(systemCall);
        }
    }

    /**
     * Returns the carrier configuration.
     *
     * @return A carrier configuration object.
     */
    @Override
    public CarrierConfig getCarrierConfig() {
        ConfigInterface config = AgentFactory.getInstance().getAgent(
                ConfigInterface.class, mSlotId);
        return (config != null) ? config.getCarrierConfig() : null;
    }

    /**
     * Add an IpSec security association parameter.
     *
     * @param param The IpSec SA parameter.
     * @return One of {@link #RESULT_ERROR} or {@link #RESULT_OK}.
     */
    @Override
    public int addIpSecSaParameter(IpSecSaParameter param) {
        IpSecInterface ipsec = AgentFactory.getInstance().getAgent(
                IpSecInterface.class, mSlotId);
        return (ipsec != null) ? ipsec.addIpSecSaParameter(param) : RESULT_ERROR;
    }

    /**
     * Remove an IpSec security association parameter with a specified identifier.
     *
     * @param ipSecId The identifier to identify IpSec SA parameter.
     */
    @Override
    public void removeIpSecSaParameter(int ipSecId) {
        IpSecInterface ipsec = AgentFactory.getInstance().getAgent(
                IpSecInterface.class, mSlotId);

        if (ipsec != null) {
            ipsec.removeIpSecSaParameter(ipSecId);
        }
    }

    /**
     * Apply the IpSec security association with a specified identifier.
     *
     * @param ipSecId The identifier to identify IpSec SA parameter.
     * @param spi The security parameter index.
     * @param intFd The integer representation of socket descriptor.
     * @param socketFd The socket descriptor.
     * @return One of {@link #RESULT_ERROR} or {@link #RESULT_OK}.
     */
    @Override
    public int applyIpSecSa(int ipSecId, int spi, int intFd, FileDescriptor socketFd) {
        IpSecInterface ipsec = AgentFactory.getInstance().getAgent(
                IpSecInterface.class, mSlotId);
        return (ipsec != null) ? ipsec.applyIpSecSa(ipSecId, spi, intFd, socketFd) : RESULT_ERROR;
    }

    /**
     * Remove the IpSec security association with a specified identifier.
     *
     * @param ipSecId The identifier to identify IpSec SA parameter.
     * @param spi The security parameter index.
     * @param intFd The integer representation of socket descriptor.
     * @param socketFd The socket descriptor.
     */
    @Override
    public void removeIpSecSa(int ipSecId, int spi, int intFd, FileDescriptor socketFd) {
        IpSecInterface ipsec = AgentFactory.getInstance().getAgent(
                IpSecInterface.class, mSlotId);

        if (ipsec != null) {
            ipsec.removeIpSecSa(ipSecId, spi, intFd, socketFd);
        }
    }

    /**
     * Returns the ISIM state as a string.
     *
     * @return The ISIM state string.
     */
    @Override
    public String getIsimState() {
        SimAgent sim = (SimAgent) AgentFactory.getInstance().getAgent(
                SimInterface.class, mSlotId);
        return (sim != null) ? sim.getIsimStateString() : "UNKNOWN";
    }

    /**
     * Reads the file attributes of the specified ISIM record.
     *
     * @param fileId The file id to be read.
     * @return One of {@link #RESULT_FAIL} or {@link #RESULT_OK}.
     */
    @Override
    public int readIsimFileAttributes(int fileId) {
        SimAgent sim = (SimAgent) AgentFactory.getInstance().getAgent(
                SimInterface.class, mSlotId);

        if (sim != null) {
            sim.readIsimFileAttributes(fileId);
            return RESULT_OK;
        }

        return RESULT_FAIL;
    }

    /**
     * Reads the value of the specified ISIM record.
     *
     * @param fileId The file id to be read.
     * @param index The index of the record for the given file.
     * @return One of {@link #RESULT_FAIL} or {@link #RESULT_OK}.
     */
    @Override
    public int readIsimRecord(int fileId, int index) {
        SimAgent sim = (SimAgent) AgentFactory.getInstance().getAgent(
                SimInterface.class, mSlotId);

        if (sim != null) {
            sim.readIsimRecord(fileId, index);
            return RESULT_OK;
        }

        return RESULT_FAIL;
    }

    /**
     * Returns the response of ISIM authentication for the specified application type.
     *
     * @param nonce The authentication challenge data, base64 encoded.
     * @param owner The owner of this request.
     * @return One of {@link #RESULT_FAIL} or {@link #RESULT_OK}.
     */
    @Override
    public int requestIsimAuthentication(String nonce, long owner) {
        SimAgent sim = (SimAgent) AgentFactory.getInstance().getAgent(
                SimInterface.class, mSlotId);

        if (sim != null) {
            sim.requestSimAuthentication(Sim.APP_TYPE_ISIM, nonce, owner);
            return RESULT_OK;
        }

        return RESULT_FAIL;
    }

    /**
     * Returns the response of USIM authentication for the specified application type.
     *
     * @param nonce The authentication challenge data, base64 encoded.
     * @param owner The owner of this request.
     * @return One of {@link #RESULT_FAIL} or {@link #RESULT_OK}.
     */
    @Override
    public int requestUsimAuthentication(String nonce, long owner) {
        SimAgent sim = (SimAgent) AgentFactory.getInstance().getAgent(
                SimInterface.class, mSlotId);

        if (sim != null) {
            sim.requestSimAuthentication(Sim.APP_TYPE_USIM, nonce, owner);
            return RESULT_OK;
        }

        return RESULT_FAIL;
    }

    /**
     * Returns the Telephony call state for CS calls.
     *
     * @return The Telephony call state.
     */
    @Override
    public @CallState int getCsCallState() {
        TelephonyInterface telephony = AgentFactory.getInstance().getAgent(
                TelephonyInterface.class, mSlotId);
        return (telephony != null) ? telephony.getCsCallState() : CALL_STATE_IDLE;
    }

    /**
     * Returns the Telephony call state for CS calls of other slots.
     *
     * @return The Telephony call state of other slot.
     */
    @Override
    public @CallState int getCsCallStateInOtherSlot() {
        TelephonyInterface telephony = AgentFactory.getInstance().getAgent(
                TelephonyInterface.class, mSlotId);
        return (telephony != null) ? telephony.getCsCallStateInOtherSlot() : CALL_STATE_IDLE;
    }

    /**
     * Returns the current data network type.
     *
     * @return The network type.
     */
    @Override
    public @NetworkType int getNetworkType() {
        TelephonyInterface telephony = AgentFactory.getInstance().getAgent(
                TelephonyInterface.class, mSlotId);
        return (telephony != null) ? telephony.getNetworkType() : NETWORK_TYPE_UNKNOWN;
    }

    /**
     * Returns the current voice network type.
     *
     * @return The voice network type.
     */
    @Override
    public @NetworkType int getVoiceNetworkType() {
        TelephonyInterface telephony = AgentFactory.getInstance().getAgent(
                TelephonyInterface.class, mSlotId);
        return (telephony != null) ? telephony.getVoiceNetworkType() : NETWORK_TYPE_UNKNOWN;
    }

    /**
     * Returns the IMEI (International Mobile Equipment Identity).
     * Returns null if IMEI is not available.
     */
    @Override
    public String getImei() {
        TelephonyInterface telephony = AgentFactory.getInstance().getAgent(
                TelephonyInterface.class, mSlotId);
        return (telephony != null) ? telephony.getImei() : null;
    }

    /**
     * Returns the software version number for the device, for example, the IMEI/SV for GSM phones.
     * Returns null if the software version is not available.
     */
    @Override
    public String getDeviceSoftwareVersion() {
        TelephonyInterface telephony = AgentFactory.getInstance().getAgent(
                TelephonyInterface.class, mSlotId);
        return (telephony != null) ? telephony.getDeviceSoftwareVersion() : null;
    }

    /**
     * Returns the phone number, or an empty string if not available.
     */
    @Override
    public String getPhoneNumber() {
        TelephonyInterface telephony = AgentFactory.getInstance().getAgent(
                TelephonyInterface.class, mSlotId);
        return (telephony != null) ? telephony.getPhoneNumber() : "";
    }

    /**
     * Returns the unique subscriber ID, for example, the IMSI for a GSM phone.
     * Returns null if it is unavailable.
     */
    @Override
    public String getSubscriberId() {
        TelephonyInterface telephony = AgentFactory.getInstance().getAgent(
                TelephonyInterface.class, mSlotId);
        return (telephony != null) ? telephony.getSubscriberId() : null;
    }

    /**
     * Returns the MCC (Mobile Country Code) of the provider of the SIM.
     */
    @Override
    public String getSimMcc() {
        TelephonyInterface telephony = AgentFactory.getInstance().getAgent(
                TelephonyInterface.class, mSlotId);
        return (telephony != null) ? telephony.getSimMcc() : null;
    }

    /**
     * Returns the MNC (Mobile Network Code) of the provider of the SIM.
     */
    @Override
    public String getSimMnc() {
        TelephonyInterface telephony = AgentFactory.getInstance().getAgent(
                TelephonyInterface.class, mSlotId);
        return (telephony != null) ? telephony.getSimMnc() : null;
    }

    /**
     * Returns the ISO-3166-1 alpha-2 country code equivalent for the SIM provider's country code.
     */
    @Override
    public String getSimCountryIso() {
        TelephonyInterface telephony = AgentFactory.getInstance().getAgent(
                TelephonyInterface.class, mSlotId);
        return (telephony != null) ? telephony.getSimCountryIso() : "";
    }

    /**
     * Returns the ISO-3166-1 alpha-2 country code equivalent of the MCC (Mobile Country Code) of
     * the current registered operator or the cell nearby, if available.
     */
    @Override
    public String getNetworkCountryIso() {
        TelephonyInterface telephony = AgentFactory.getInstance().getAgent(
                TelephonyInterface.class, mSlotId);
        return (telephony != null) ? telephony.getNetworkCountryIso() : "";
    }

    /**
     * Checks whether the specified number is an emergency number or not.
     *
     * @return {@code true} if the number is an emergency number, {@code false} otherwise.
     */
    @Override
    public boolean isEmergencyNumber(String number) {
        TelephonyInterface telephony = AgentFactory.getInstance().getAgent(
                TelephonyInterface.class, mSlotId);
        return (telephony != null) ? telephony.isEmergencyNumber(number) : false;
    }

    /**
     * Requests the data connection with the specified APN.
     *
     * @param apnType An APN type.
     *                {@link EApnType#IMS},
     *                {@link EApnType#INTERNET},
     *                {@link EApnType#EMERGENCY}
     * @return {@code true} if this operation is successfully performed, {@code false} otherwise.
     */
    @Override
    public boolean requestNetwork(int apnType) {
        IDcApn apn = getDcApn();
        return (apn != null) ? apn.connect(apnType) : false;
    }

    /**
     * Releases the data connection with the specified APN.
     *
     * @param apnType An APN type.
     *                {@link EApnType#IMS},
     *                {@link EApnType#INTERNET},
     *                {@link EApnType#EMERGENCY}
     * @return {@code true} if this operation is successfully performed, {@code false} otherwise.
     */
    @Override
    public boolean releaseNetwork(int apnType) {
        IDcApn apn = getDcApn();
        return (apn != null) ? apn.disconnect(apnType) : false;
    }

    /**
     * Returns the APN name of the specified APN.
     *
     * @param apnType An APN type.
     *                {@link EApnType#IMS},
     *                {@link EApnType#INTERNET},
     *                {@link EApnType#EMERGENCY}
     * @return An APN name if present or empty string.
     */
    @Override
    public String getApnName(int apnType) {
        IDcApn apn = getDcApn();
        return (apn != null) ? apn.getApn(apnType) : "";
    }

    /**
     * Returns the current connection state of the specified APN.
     *
     * @param apnType An APN type.
     *                {@link EApnType#IMS},
     *                {@link EApnType#INTERNET},
     *                {@link EApnType#EMERGENCY}
     * @return The connection state.
     *         {@link EDataState#DATA_STATE_DISCONNECTED},
     *         {@link EDataState#DATA_STATE_CONNECTED},
     *         {@link EDataState#DATA_STATE_CONNECT_FAILED},
     *         {@link EDataState#DATA_STATE_IP_CHANGED},
     *         {@link EDataState#DATA_STATE_PCSCF_CHANGED}
     */
    @Override
    public int getDataConnectionState(int apnType) {
        IDcApn apn = getDcApn();
        return (apn != null)
                ? apn.getDataState(apnType)
                : EDataState.DATA_STATE_DISCONNECTED.getState();
    }

    /**
     * Returns the network interface identifier of the specified APN.
     *
     * @param apnType An APN type.
     *                {@link EApnType#IMS},
     *                {@link EApnType#INTERNET},
     *                {@link EApnType#EMERGENCY}
     * @return A network interface identifier or {@link #RESULT_ERROR} if an error occurs.
     */
    @Override
    public int getIfaceId(int apnType) {
        IDcApn apn = getDcApn();
        return (apn != null) ? apn.getIfaceId(apnType) : RESULT_ERROR;
    }

    /**
     * Returns the network interface name of the specified APN.
     *
     * @param apnType An APN type.
     *                {@link EApnType#IMS},
     *                {@link EApnType#INTERNET},
     *                {@link EApnType#EMERGENCY}
     * @return A network interface name or empty string.
     */
    @Override
    public String getIfaceName(int apnType) {
        IDcApn apn = getDcApn();
        return (apn != null) ? apn.getIfaceName(apnType) : "";
    }

    /**
     * Returns the MTU size of the specified APN.
     *
     * @param apnType An APN type.
     *                {@link EApnType#IMS},
     *                {@link EApnType#INTERNET},
     *                {@link EApnType#EMERGENCY}
     * @return An MTU size.
     */
    @Override
    public int getMtu(int apnType) {
        IDcApn apn = getDcApn();
        return (apn != null) ? apn.getMtu(apnType) : 0;
    }

    /**
     * Returns the IPCAN category of the specified APN.
     *
     * @param apnType An APN type.
     *                {@link EApnType#IMS},
     *                {@link EApnType#INTERNET},
     *                {@link EApnType#EMERGENCY}
     * @return An IPCAN category.
     *         {@link IApn#IPCAN_CATEGORY_WLAN},
     *         {@link IApn#IPCAN_CATEGORY_MOBILE}
     */
    @Override
    public int getIpcanCategory(int apnType) {
        IDcApn apn = getDcApn();
        return (apn != null) ? apn.getIpcanCategory(apnType) : IApn.IPCAN_CATEGORY_MOBILE;
    }

    /**
     * Returns the local IP address of the specified APN.
     *
     * @param apnType An APN type.
     *                {@link EApnType#IMS},
     *                {@link EApnType#INTERNET},
     *                {@link EApnType#EMERGENCY}
     * @param ipVersion An IP version type.
     *                  {@link EIpVersion#IPV4},
     *                  {@link EIpVersion#IPV6},
     *                  {@link EIpVersion#IPV4V6},
     *                  {@link EIpVersion#IPV6V4}
     * @return A local IP address.
     */
    @Override
    public String getLocalAddress(int apnType, int ipVersion) {
        IDcApn apn = getDcApn();
        return (apn != null) ? apn.getLocalAddress(apnType, ipVersion) : "";
    }

    /**
     * Returns the P-CSCF address of the specified APN.
     *
     * @param apnType An APN type.
     *                {@link EApnType#IMS},
     *                {@link EApnType#INTERNET},
     *                {@link EApnType#EMERGENCY}
     * @param ipVersion An IP version type.
     *                  {@link EIpVersion#IPV4},
     *                  {@link EIpVersion#IPV6},
     *                  {@link EIpVersion#IPV4V6},
     *                  {@link EIpVersion#IPV6V4}
     * @return The P-CSCF addresses or null.
     */
    @Override
    public String[] getPcscfAddresses(int apnType, int ipVersion) {
        IDcApn apn = getDcApn();
        return (apn != null) ? apn.getPcscfAddress(apnType, ipVersion) : null;
    }

    /**
     * Checks whether the IPv6 is preferred or not for the specified APN.
     *
     * @param apnType An APN type.
     *                {@link EApnType#IMS},
     *                {@link EApnType#INTERNET},
     *                {@link EApnType#EMERGENCY}
     * @return {@code true} if the specified APN prefers IPv6 address, {@code false} otherwise.
     */
    @Override
    public boolean isIpv6Preferred(int apnType) {
        IDcApn apn = getDcApn();
        return (apn != null) ? apn.isIpv6Preferred(apnType) : false;
    }

    /**
     * Returns the numeric IP address from the specified host name.
     *
     * @param apnType An APN type.
     *                {@link EApnType#IMS},
     *                {@link EApnType#INTERNET},
     *                {@link EApnType#EMERGENCY}
     * @param ipVersion An IP version type.
     *                  {@link EIpVersion#IPV4},
     *                  {@link EIpVersion#IPV6},
     *                  {@link EIpVersion#IPV4V6},
     *                  {@link EIpVersion#IPV6V4}
     * @param host A host name to be resolved.
     * @return The numeric IP addresses or null.
     */
    @Override
    public String[] getHostByName(int apnType, int ipVersion, String host) {
        IDcApn apn = getDcApn();
        return (apn != null) ? apn.getHostByName(apnType, ipVersion, host) : null;
    }

    /**
     * Binds the specified socket descriptor to the specified network.
     *
     * @param apnType An APN type.
     *                {@link EApnType#IMS},
     *                {@link EApnType#INTERNET},
     *                {@link EApnType#EMERGENCY}
     * @param sockFd A socket FD.
     * @return {@code true} if the operation is successfully performed, {@code false} otherwise.
     */
    @Override
    public boolean bindSocket(int apnType, FileDescriptor sockFd) {
        IDcApn apn = getDcApn();
        return (apn != null) ? apn.bindSocket(apnType, sockFd) : false;
    }

    /**
     * Returns the service state of the current voice network.
     *
     * @return A service state.
     *         {@link ServiceState#STATE_IN_SERVICE},
     *         {@link ServiceState#STATE_OUT_OF_SERVICE},
     *         {@link ServiceState#STATE_EMERGENCY_ONLY},
     *         {@link ServiceState#STATE_POWER_OFF}
     */
    @Override
    public int getVoiceServiceState() {
        IDcNetWatcher netWatcher = getDcNetWatcher();
        return (netWatcher != null)
                ? netWatcher.getVoiceServiceState()
                : ServiceState.STATE_OUT_OF_SERVICE;
    }

    /**
     * Returns the roaming type of the current voice network.
     *
     * @return A roaming type.
     *         {@link ServiceState#ROAMING_TYPE_NOT_ROAMING},
     *         {@link ServiceState#ROAMING_TYPE_UNKNOWN},
     *         {@link ServiceState#ROAMING_TYPE_DOMESTIC},
     *         {@link ServiceState#ROAMING_TYPE_INTERNATIONAL}
     */
    @Override
    public int getVoiceRoamingType() {
        IDcNetWatcher netWatcher = getDcNetWatcher();
        return (netWatcher != null)
                ? netWatcher.getVoiceRoamingType()
                : ServiceState.ROAMING_TYPE_NOT_ROAMING;
    }

    /**
     * Returns the service state of the current data network.
     *
     * @return A service state.
     *         {@link ServiceState#STATE_IN_SERVICE},
     *         {@link ServiceState#STATE_OUT_OF_SERVICE},
     *         {@link ServiceState#STATE_EMERGENCY_ONLY},
     *         {@link ServiceState#STATE_POWER_OFF}
     */
    @Override
    public int getDataServiceState() {
        IDcNetWatcher netWatcher = getDcNetWatcher();
        return (netWatcher != null)
                ? netWatcher.getDataServiceState()
                : ServiceState.STATE_OUT_OF_SERVICE;
    }

    /**
     * Returns the roaming type of the current data network.
     *
     * @return A roaming type.
     *         {@link ServiceState#ROAMING_TYPE_NOT_ROAMING},
     *         {@link ServiceState#ROAMING_TYPE_UNKNOWN},
     *         {@link ServiceState#ROAMING_TYPE_DOMESTIC},
     *         {@link ServiceState#ROAMING_TYPE_INTERNATIONAL}
     */
    @Override
    public int getDataRoamingType() {
        IDcNetWatcher netWatcher = getDcNetWatcher();
        return (netWatcher != null)
                ? netWatcher.getDataRoamingType()
                : ServiceState.ROAMING_TYPE_NOT_ROAMING;
    }

    /**
     * Returns the PLMN information of MOCN.
     *
     * @return A PLMN info. of MOCN.
     */
    @Override
    public int getMocnPlmnInfo() {
        IDcNetWatcher netWatcher = getDcNetWatcher();
        return (netWatcher != null) ? netWatcher.getMocnPlmnInfo() : 0;
    }

    /**
     * Checks whether the current network is attached as roaming.
     *
     * @return {@code true} if the network is in roaming, {@code false} otherwise.
     */
    @Override
    public boolean isNetworkRoaming() {
        IDcNetWatcher netWatcher = getDcNetWatcher();
        return (netWatcher != null) ? netWatcher.isRoaming() : false;
    }

    /**
     * Checks whether the emergency is only available in the LTE network.
     *
     * @return {@code true} if the emergency is only available, {@code false} otherwise.
     */
    @Override
    public boolean isLteEmergencyOnly() {
        IDcNetWatcher netWatcher = getDcNetWatcher();
        return (netWatcher != null) ? netWatcher.isLteEmergencyOnly() : false;
    }

    /**
     * Checks whether the emergency attach is supported or not.
     *
     * @return {@code true} if emergency attach is supported, {@code false} otherwise.
     */
    @Override
    public boolean isEmergencyAttachSupported() {
        IDcNetWatcher netWatcher = getDcNetWatcher();
        return (netWatcher != null) ? netWatcher.isEmergencyServiceSupported() : false;
    }

    /**
     * Checks whether the mobile data setting is enabled or not.
     *
     * @return {@code true} if the mobile data setting is enabled, {@code false} otherwise.
     */
    @Override
    public boolean isMobileDataEnabled() {
        IDcUtils util = (IDcUtils) DcFactory.getDc(DcFactory.UTIL, mSlotId);
        return (util != null) ? util.isMobileDataEnabled() : false;
    }

    /**
     * Returns the access network information of the network that the IMS is registering
     * or was registered.
     *
     * @param defaultNetworkType The default network type to be used when the network is unknown.
     * @return The access network information or null.
     */
    @Override
    public IDcUtils.AccessNetworkInfo getAccessNetworkInfo(@NetworkType int defaultNetworkType) {
        IDcUtils util = (IDcUtils) DcFactory.getDc(DcFactory.UTIL, mSlotId);
        return (util != null) ? util.getAccessNetworkInfo(defaultNetworkType) : null;
    }

    /**
     * Returns the last known access network information for the specified network.
     *
     * @param networkType A network type.
     * @return A last known access network information.
     */
    @Override
    public String[] getLastAccessNetworkInfo(@NetworkType int networkType) {
        CellInfoInterface cellInfo = AgentFactory.getInstance().getAgent(
                CellInfoInterface.class, mSlotId);

        if (cellInfo == null) {
            return null;
        }

        if (networkType == NETWORK_TYPE_UNKNOWN) {
            return cellInfo.getAccessNetworkInfo();
        } else {
            return cellInfo.getAccessNetworkInfo(networkType);
        }
    }

    /**
     * Returns the flag specifying whether the IMS voice call is supported on the LTE network.
     */
    @Override
    public boolean isImsVoiceCallSupported() {
        IDcNetWatcher netWatcher = getDcNetWatcher();
        return (netWatcher != null) ? netWatcher.isVops() : false;
    }

    /**
     * Updates the native service ready state.
     *
     * @param serviceReady A flag specifying whether the native service is ready or not.
     * @return One of {@link #RESULT_FAIL} or {@link #RESULT_OK}.
     */
    @Override
    public int updateNativeServiceReady(boolean serviceReady) {
        NativeStateAgent nsa = (NativeStateAgent) AgentFactory.getInstance().getAgent(
                NativeStateInterface.class, mSlotId);
        if (nsa != null) {
            nsa.updateServiceReady(serviceReady);
            return RESULT_OK;
        }
        return RESULT_FAIL;
    }

    /**
     * Returns the best location information from the last known location.
     *
     * @param category The location category. Possible values are:
     *                 {@link LocationInterface#LOCATION_CATEGORY_ALL},
     *                 {@link LocationInterface#LOCATION_CATEGORY_POSITION_N_COUNTRY},
     *                 {@link LocationInterface#LOCATION_CATEGORY_POSITION}
     */
    @Override
    public String[] getLastKnownLocation(int category) {
        LocationInterface location = AgentFactory.getInstance().getAgent(
                LocationInterface.class, mSlotId);
        return (location != null) ? location.getLastKnownLocation(category) : null;
    }

    /**
     * Starts listening the location information with the given interval.
     *
     * @param updateIntervalSec The location update interval in seconds.
     */
    @Override
    public void startListeningForLocation(int updateIntervalSec) {
        LocationInterface location = AgentFactory.getInstance().getAgent(
                LocationInterface.class, mSlotId);
        if (location != null) {
            location.startListeningForLocation(updateIntervalSec);
        }
    }

    /**
     * Stops listening the location information.
     */
    @Override
    public void stopListeningForLocation() {
        LocationInterface location = AgentFactory.getInstance().getAgent(
                LocationInterface.class, mSlotId);
        if (location != null) {
            location.stopListeningForLocation();
        }
    }

    /**
     * Starts an instant location update (one-time update).
     */
    @Override
    public void startInstantLocationUpdate() {
        LocationInterface location = AgentFactory.getInstance().getAgent(
                LocationInterface.class, mSlotId);
        if (location != null) {
            location.startInstantLocationUpdate();
        }
    }

    private IDcApn getDcApn() {
        return (IDcApn) DcFactory.getDc(DcFactory.APN, mSlotId);
    }

    private IDcNetWatcher getDcNetWatcher() {
        return (IDcNetWatcher) DcFactory.getDc(DcFactory.NETWORK_WATCHER, mSlotId);
    }
}
