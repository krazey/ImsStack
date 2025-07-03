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

import android.telephony.Annotation.CallState;
import android.telephony.Annotation.NetworkType;

import androidx.annotation.NonNull;

import com.android.imsstack.core.agents.dcmif.IDcUtils;
import com.android.imsstack.core.config.CarrierConfig;

import java.io.FileDescriptor;
import java.util.List;

/** An interface for providing the system call. */
public interface SystemCallInterface {
    /** Result code of execution with no error. */
    int RESULT_OK = 1;
    /** Result code of execution for operation failure. */
    int RESULT_FAIL = 0;
    /** Result code of execution with a specific error. */
    int RESULT_ERROR = -1;

    //// ConfigInterface {
    /**
     * Returns the carrier configuration.
     *
     * @return A CarrierConfig object.
     */
    CarrierConfig getCarrierConfig();
    //// }

    //// IpSecInterface {
    /**
     * Add an IpSec security association parameter.
     *
     * @param param The IpSec SA parameter.
     * @return One of {@link #RESULT_ERROR} or {@link #RESULT_OK}.
     */
    int addIpSecSaParameter(IpSecSaParameter param);

    /**
     * Remove an IpSec security association parameter with a specified identifier.
     *
     * @param ipSecId The identifier to identify IpSec SA parameter.
     */
    void removeIpSecSaParameter(int ipSecId);

    /**
     * Apply the IpSec security association with a specified identifier.
     *
     * @param ipSecId The identifier to identify IpSec SA parameter.
     * @param spi The security parameter index.
     * @param intFd The integer representation of socket descriptor.
     * @param socketFd The socket descriptor.
     * @return One of {@link #RESULT_ERROR} or {@link #RESULT_OK}.
     */
    int applyIpSecSa(int ipSecId, int spi, int intFd, FileDescriptor socketFd);

    /**
     * Remove the IpSec security association with a specified identifier.
     *
     * @param ipSecId The identifier to identify IpSec SA parameter.
     * @param spi The security parameter index.
     * @param intFd The integer representation of socket descriptor.
     * @param socketFd The socket descriptor.
     */
    void removeIpSecSa(int ipSecId, int spi, int intFd, FileDescriptor socketFd);
    ////}

    //// SIM interface {
    /**
     * Returns the ISIM state as a string.
     *
     * @return The ISIM state string.
     */
    String getIsimState();

    /**
     * Returns the list of the specified ISIM record.
     *
     * @param fileId The file id to be read.
     * @return The list of ISIM record.
     */
    @NonNull List<String> getIsimRecord(int fileId);

    /**
     * Returns the response of ISIM authentication for the specified application type.
     *
     * @param nonce The authentication challenge data, base64 encoded.
     * @param owner The owner of this request.
     * @return One of {@link #RESULT_FAIL} or {@link #RESULT_OK}.
     */
    int requestIsimAuthentication(String nonce, long owner);

    /**
     * Returns the response of USIM authentication for the specified application type.
     *
     * @param nonce The authentication challenge data, base64 encoded.
     * @param owner The owner of this request.
     * @return One of {@link #RESULT_FAIL} or {@link #RESULT_OK}.
     */
    int requestUsimAuthentication(String nonce, long owner);
    ////}

    //// Telephony interface {
    /**
     * Returns the Telephony call state for CS calls.
     *
     * @return The Telephony call state.
     */
    @CallState int getCsCallState();

    /**
     * Returns the Telephony call state for CS calls of other slots.
     *
     * @return The Telephony call state of other slot.
     */
    @CallState int getCsCallStateInOtherSlot();

    /**
     * Returns the current data network type.
     *
     * @return The network type.
     */
    @NetworkType int getNetworkType();

    /**
     * Returns the current voice network type.
     *
     * @return The voice network type.
     */
    @NetworkType int getVoiceNetworkType();

    /**
     * Returns the IMEI (International Mobile Equipment Identity).
     * Returns null if IMEI is not available.
     */
    String getImei();

    /**
     * Returns the software version number for the device, for example, the IMEI/SV for GSM phones.
     * Returns null if the software version is not available.
     */
    String getDeviceSoftwareVersion();

    /**
     * Returns the phone number, or an empty string if not available.
     */
    String getPhoneNumber();

    /**
     * Returns the unique subscriber ID, for example, the IMSI for a GSM phone.
     * Returns null if it is unavailable.
     */
    String getSubscriberId();

    /**
     * Returns the MCC (Mobile Country Code) of the provider of the SIM.
     */
    String getSimMcc();

    /**
     * Returns the MNC (Mobile Network Code) of the provider of the SIM.
     */
    String getSimMnc();

    /**
     * Returns the ISO-3166-1 alpha-2 country code equivalent for the SIM provider's country code.
     */
    String getSimCountryIso();

    /**
     * Returns the MCC+MNC (Mobile Country Code + Mobile Network Code) of the current registered
     * operator. 5 or 6 decimal digits.
     */
    String getNetworkOperator();

    /**
     * Returns the ISO-3166-1 alpha-2 country code equivalent of the MCC (Mobile Country Code) of
     * the current registered operator or the cell nearby, if available.
     */
    String getNetworkCountryIso();

    /**
     * Checks whether the specified number is an emergency number or not.
     *
     * @return {@code true} if the number is an emergency number, {@code false} otherwise.
     */
    boolean isEmergencyNumber(String number);
    ////}

    //// Data interface {
    /**
     * Requests the data connection with the specified APN.
     *
     * @param apnType An APN type.
     *                {@link EApnType#IMS},
     *                {@link EApnType#INTERNET},
     *                {@link EApnType#EMERGENCY}
     * @return {@code true} if this operation is successfully performed, {@code false} otherwise.
     */
    boolean requestNetwork(int apnType);

    /**
     * Releases the data connection with the specified APN.
     *
     * @param apnType An APN type.
     *                {@link EApnType#IMS},
     *                {@link EApnType#INTERNET},
     *                {@link EApnType#EMERGENCY}
     * @return {@code true} if this operation is successfully performed, {@code false} otherwise.
     */
    boolean releaseNetwork(int apnType);

    /**
     * Returns the APN name of the specified APN.
     *
     * @param apnType An APN type.
     *                {@link EApnType#IMS},
     *                {@link EApnType#INTERNET},
     *                {@link EApnType#EMERGENCY}
     * @return An APN name if present or empty string.
     */
    String getApnName(int apnType);

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
    int getDataConnectionState(int apnType);

    /**
     * Returns the network interface identifier of the specified APN.
     *
     * @param apnType An APN type.
     *                {@link EApnType#IMS},
     *                {@link EApnType#INTERNET},
     *                {@link EApnType#EMERGENCY}
     * @return A network interface identifier or {@link #RESULT_ERROR} if an error occurs.
     */
    int getIfaceId(int apnType);

    /**
     * Returns the network interface name of the specified APN.
     *
     * @param apnType An APN type.
     *                {@link EApnType#IMS},
     *                {@link EApnType#INTERNET},
     *                {@link EApnType#EMERGENCY}
     * @return A network interface name or empty string.
     */
    String getIfaceName(int apnType);

    /**
     * Returns the MTU size of the specified APN.
     *
     * @param apnType An APN type.
     *                {@link EApnType#IMS},
     *                {@link EApnType#INTERNET},
     *                {@link EApnType#EMERGENCY}
     * @return An MTU size.
     */
    int getMtu(int apnType);

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
    int getIpcanCategory(int apnType);

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
     * @return A local IP address or empty string.
     */
    String getLocalAddress(int apnType, int ipVersion);

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
     * @return The P-CSCF addresses or empty array.
     */
    @NonNull String[] getPcscfAddresses(int apnType, int ipVersion);

    /**
     * Checks whether the IPv6 is preferred or not for the specified APN.
     *
     * @param apnType An APN type.
     *                {@link EApnType#IMS},
     *                {@link EApnType#INTERNET},
     *                {@link EApnType#EMERGENCY}
     * @return {@code true} if the specified APN prefers IPv6 address, {@code false} otherwise.
     */
    boolean isIpv6Preferred(int apnType);

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
    String[] getHostByName(int apnType, int ipVersion, String host);

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
    boolean bindSocket(int apnType, FileDescriptor sockFd);

    /**
     * Returns the service state of the current voice network.
     *
     * @return A service state.
     *         {@link ServiceState#STATE_IN_SERVICE},
     *         {@link ServiceState#STATE_OUT_OF_SERVICE},
     *         {@link ServiceState#STATE_EMERGENCY_ONLY},
     *         {@link ServiceState#STATE_POWER_OFF}
     */
    int getVoiceServiceState();

    /**
     * Returns the roaming type of the current voice network.
     *
     * @return A roaming type.
     *         {@link ServiceState#ROAMING_TYPE_NOT_ROAMING},
     *         {@link ServiceState#ROAMING_TYPE_UNKNOWN},
     *         {@link ServiceState#ROAMING_TYPE_DOMESTIC},
     *         {@link ServiceState#ROAMING_TYPE_INTERNATIONAL}
     */
    int getVoiceRoamingType();

    /**
     * Returns the service state of the current cellular data network.
     *
     * @return A service state.
     *         {@link ServiceState#STATE_IN_SERVICE},
     *         {@link ServiceState#STATE_OUT_OF_SERVICE},
     *         {@link ServiceState#STATE_EMERGENCY_ONLY},
     *         {@link ServiceState#STATE_POWER_OFF}
     */
    int getCellularDataServiceState();

    /**
     * Returns the service state of the current data network.
     *
     * @return A service state.
     *         {@link ServiceState#STATE_IN_SERVICE},
     *         {@link ServiceState#STATE_OUT_OF_SERVICE},
     *         {@link ServiceState#STATE_EMERGENCY_ONLY},
     *         {@link ServiceState#STATE_POWER_OFF}
     */
    int getDataServiceState();

    /**
     * Returns the roaming type of the current data network.
     *
     * @return A roaming type.
     *         {@link ServiceState#ROAMING_TYPE_NOT_ROAMING},
     *         {@link ServiceState#ROAMING_TYPE_UNKNOWN},
     *         {@link ServiceState#ROAMING_TYPE_DOMESTIC},
     *         {@link ServiceState#ROAMING_TYPE_INTERNATIONAL}
     */
    int getDataRoamingType();

    /**
     * Returns the PLMN information of MOCN.
     *
     * @return A PLMN info. of MOCN.
     */
    int getMocnPlmnInfo();

    /**
     * Checks whether the current network is attached as roaming.
     *
     * @return {@code true} if the network is in roaming, {@code false} otherwise.
     */
    boolean isNetworkRoaming();

    /**
     * Checks whether the emergency is only available in the network.
     *
     * @return {@code true} if the emergency is only available, {@code false} otherwise.
     */
    boolean isEmergencyOnly();

    /**
     * Checks whether the emergency attach is supported or not.
     *
     * @return {@code true} if emergency attach is supported, {@code false} otherwise.
     */
    boolean isEmergencyAttachSupported();

    /**
     * Checks whether the mobile data setting is enabled or not.
     *
     * @return {@code true} if the mobile data setting is enabled, {@code false} otherwise.
     */
    boolean isMobileDataEnabled();

    /**
     * Returns the access network information of the network that the IMS is registering
     * or was registered.
     *
     * @param defaultNetworkType The default network type to be used when the network is unknown.
     * @return The access network information.
     */
    @NonNull IDcUtils.AccessNetworkInfo getAccessNetworkInfo(@NetworkType int defaultNetworkType);

    /**
     * Returns the last known access network information for the specified network.
     *
     * @param networkType A network type.
     * @return The last known access network information.
     */
    @NonNull String[] getLastAccessNetworkInfo(@NetworkType int networkType);
    ////}

    /**
     * Indicates NAS and RRC layers of the modem that the upcoming IMS traffic is
     * for the service mentioned in the traffic type.
     *
     * @param id The identification for IMS traffic
     * @param trafficType The type for IMS traffic
     * @param accessNetworkType The type for radio access network type
     * @param direction The direction for IMS traffic
     */
    void startImsTraffic(int id, int trafficType, int accessNetworkType, int direction);

    /**
     * Indicates IMS traffic has been stopped. For all IMS traffic,
     * notified with startImsTraffic, IMS service shall notify stopImsTraffic
     * when it completes the traffic. The reference listener corresponding to id is removed.
     *
     * @param id The identification to be removed
     */
    void stopImsTraffic(int id);

    /**
     * Triggers the EPS fallback procedure by UE for the case where the user is trying to
     * place a voice call in NR network and the voice call is not established
     * within several seconds.
     *
     * @param reason Specifies the reason that causes EPS fallback
     * @return {@code true} if the operation is successfully performed, {@code false} otherwise.
     */
    boolean triggerEpsFallback(int reason);

    /**
     * Returns the flag specifying whether the IMS voice call(vops) is supported on the LTE network.
     *
     * @return true if the IMS voice call is supported, false otherwise.
     */
    boolean isImsVoiceCallSupported();

    /**
     * Updates the native service ready state.
     *
     * @param serviceReady A flag specifying whether the native service is ready or not.
     * @return One of {@link #RESULT_FAIL} or {@link #RESULT_OK}.
     */
    int updateNativeServiceReady(boolean serviceReady);

    /**
     * Returns the best location information from the last known location.
     *
     * @param category The location category. Possible values are:
     *                 {@link LocationInterface#LOCATION_CATEGORY_ALL},
     *                 {@link LocationInterface#LOCATION_CATEGORY_POSITION_N_COUNTRY},
     *                 {@link LocationInterface#LOCATION_CATEGORY_POSITION}
     * @return The last known location information.
     */
    @NonNull String[] getLastKnownLocation(int category);

    /**
     * Starts listening the location information with the given interval.
     *
     * @param updateIntervalSec The location update interval in seconds.
     */
    void startListeningForLocation(int updateIntervalSec);

    /**
     * Stops listening the location information.
     */
    void stopListeningForLocation();

    /**
     * Requests a location update (one-time update).
     *
     * @param waitTimeMs A wait time to fix the location in milli-seconds.
     * @return The request identifier for event handling for location update completion and
     *         cancellation.
     *         0(zero) indicates that the location update request cannot be performed,
     *         otherwise an integer value greater than 0 is returned.
     */
    int requestLocationUpdate(int waitTimeMs);

    /**
     * Cancels a previously requested location update.
     *
     * @param requestId A request identifier returned from {@link #requestLocationUpdate(int)}.
     */
    void cancelLocationUpdate(int requestId);

    /**
     * Returns the reject cause for the network registration.
     *
     * @return A reject cause.
     */
    int getNetworkRegistrationRejectCause();
}
