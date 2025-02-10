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
package com.android.imsstack.base;

import android.annotation.CallbackExecutor;
import android.net.Uri;
import android.telephony.Annotation.NetworkType;
import android.telephony.Annotation.UiccAppType;
import android.telephony.Annotation.UiccAppTypeExt;
import android.telephony.CellInfo;
import android.telephony.ServiceState;
import android.telephony.TelephonyCallback;
import android.telephony.TelephonyManager.AuthType;
import android.telephony.TelephonyManager.BootstrapAuthenticationCallback;
import android.telephony.TelephonyManager.CellInfoCallback;
import android.telephony.TelephonyManager.HalService;
import android.telephony.TelephonyManager.SimState;
import android.telephony.gba.UaSecurityProtocolIdentifier;
import android.util.Pair;

import androidx.annotation.NonNull;
import androidx.annotation.Nullable;

import java.util.List;
import java.util.concurrent.Executor;

/**
 * A proxy interface to access the APIs of {@link TelephonyManager}.
 */
public interface TelephonyManagerProxy {
    /**
     * Creates a new {@link TelephonyManagerProxy} object pinned to the given {@code subId}.
     *
     * @return A {@link TelephonyManagerProxy} object that uses the given {@code subId}.
     */
    @NonNull TelephonyManagerProxy createForSubscriptionId(int subId);

    /**
     * Registers a callback object to receive notification of changes in specified telephony states.
     *
     * To register a callback, pass a {@link TelephonyCallback} which implements
     * interfaces of events. For example,
     * FakeServiceStateCallback extends {@link TelephonyCallback} implements
     * {@link TelephonyCallback.ServiceStateListener}.
     *
     * At registration, and when a specified telephony state changes, the telephony manager invokes
     * the appropriate callback method on the callback object and passes the current (updated)
     * values.
     */
    void registerTelephonyCallback(@NonNull @CallbackExecutor Executor executor,
            @NonNull TelephonyCallback callback);

    /**
     * Unregister an existing {@link TelephonyCallback}.
     *
     * @param callback The {@link TelephonyCallback} object to unregister.
     */
    void unregisterTelephonyCallback(@NonNull TelephonyCallback callback);

    /**
     * Returns the number of logial modems currently configured to be activated.
     */
    int getActiveModemCount();

    /**
     * Return how many logical modem can be potentially active simultaneously, in terms of hardware
     * capability.
     * It might return different value from {@link #getActiveModemCount}. For example, for a
     * dual-SIM capable device operating in single SIM mode (only one logical modem is turned on),
     * {@link #getActiveModemCount} returns 1 while this API returns 2.
     */
    int getSupportedModemCount();

    /**
     * Returns the IMEI (International Mobile Equipment Identity). Return null if IMEI is not
     * available.
     *
     * @param slotIndex The slot index of which IMEI is returned.
     */
    String getImei(int slotIndex);

    /**
     * Returns the software version number for the device, for example,
     * the IMEI/SV for GSM phones. Return null if the software version is not available.
     *
     * @param slotIndex The slot index of which the device software version is returned.
     */
    String getDeviceSoftwareVersion(int slotIndex);

    /**
     * Identifies if the supplied phone number is an emergency number that matches a known
     * emergency number based on current locale, SIM card(s), Android database, modem, network,
     * or defaults.
     *
     * @param number - the number to look up
     * @return {@code true} if the given number is an emergency number based on current locale,
     *         SIM card(s), Android database, modem, network or defaults; {@code false} otherwise.
     */
    boolean isEmergencyNumber(@NonNull String number);

    /**
     * Checks if there is an ICC card present in the device.
     *
     * An ICC card is a smart card that contains a subscriber identity module (SIM) and is used
     * to identify and authenticate users to a mobile network.
     *
     * Note: In case of embedded SIM there is an ICC card always present irrespective
     * of whether an active SIM profile is present or not so this API would always return
     * {@code true}.
     *
     * @return {@code true} if a ICC card is present, {@code false} otherwise.
     */
    boolean hasIccCard();

    /**
     * Returns {@code true} if the specified type of application
     * (e.g. {@link TelephonyManager#APPTYPE_CSIM}) is present on the UICC card.
     *
     * @param appType The UICC application type like {@link TelephonyManager#APPTYPE_ISIM}.
     * @return {@code true} if the specified type of application in UICC CARD or
     *         {@code false} if no UICC or error.
     */
    boolean isApplicationOnUicc(@UiccAppType int appType);

    /**
     * Returns a constant indicating the state of the card applications on the default SIM card.
     */
    @SimState int getSimApplicationState();

    /**
     * Returns a constant indicating the state of the device SIM card in a logical slot.
     *
     * @param slotIndex A logical slot index.
     */
    @SimState int getSimState(int slotIndex);

    /**
     * Returns a constant indicating the state of the default SIM card.
     */
    @SimState int getSimCardState();

    /**
     * Returns carrier id of the current subscription.
     *
     * To recognize a carrier (including MVNO) as a first-class identity, Android assigns each
     * carrier with a canonical integer a.k.a. carrier id. The carrier ID is an Android
     * platform-wide identifier for a carrier.
     *
     * Apps which have carrier-specific configurations or business logic can use the carrier id
     * as an Android platform-wide identifier for carriers.
     *
     * @return Carrier id of the current subscription.
     *         Returns {@link TelephonyManager#UNKNOWN_CARRIER_ID} if the subscription is
     *         unavailable or the carrier cannot be identified.
     */
    int getSimCarrierId();

    /**
     * Returns carrier id name of the current subscription.
     *
     * Carrier id name is a user-facing name of carrier id returned by
     * {@link #getSimCarrierId()}, usually the brand name of the subsidiary
     * (e.g. T-Mobile). Each carrier could configure multiple {@link #getSimOperatorName() SPN} but
     * should have a single carrier name. Carrier name is not a canonical identity,
     * use {@link #getSimCarrierId()} instead.
     * The returned carrier name is unlocalized.
     *
     * @return Carrier name of the current subscription. Returns null if the subscription is
     *         unavailable or the carrier cannot be identified.
     */
    @Nullable CharSequence getSimCarrierIdName();

    /**
     * Returns fine-grained carrier ID of the current subscription.
     *
     * A specific carrier ID can represent the fact that a carrier may be in effect an aggregation
     * of other carriers (ie in an MVNO type scenario) where each of these specific carriers which
     * are used to make up the actual carrier service may have different carrier configurations.
     * A specific carrier ID could also be used, for example, in a scenario where a carrier requires
     * different carrier configuration for different service offering such as a prepaid plan.
     *
     * The specific carrier ID would be used for configuration purposes, but apps wishing to know
     * about the carrier itself should use the regular carrier ID returned by
     * {@link #getSimCarrierId()}.
     *
     * e.g. Tracfone SIMs could return different specific carrier ID based on IMSI from current
     * subscription while carrier ID remains the same.
     *
     * @return Returns fine-grained carrier id of the current subscription.
     *         Returns {@link TelephonyManager#UNKNOWN_CARRIER_ID} if the subscription is
     *         unavailable or the carrier cannot be identified.
     */
    int getSimSpecificCarrierId();

    /**
     * Returns carrier id based on sim MCCMNC (returned by {@link #getSimOperator()}) only.
     * This is used for fallback when configurations/logic for exact carrier id
     * {@link #getSimCarrierId()} are not found.
     *
     * @return Returns matching carrier id from sim MCCMNC.
     *         Returns {@link TelephonyManager#UNKNOWN_CARRIER_ID} if the subscription is
     *         unavailable or the carrier cannot be identified.
     */
    int getCarrierIdFromSimMccMnc();

    /**
     * Returns the unique subscriber ID, for example, the IMSI for a GSM phone.
     * Returns null if it is unavailable.
     */
    String getSubscriberId();

    /**
     * Returns the MCC+MNC (mobile country code + mobile network code) of the
     * provider of the SIM. 5 or 6 decimal digits.
     *
     * Availability: SIM state must be {@link TelephonyManager#SIM_STATE_READY}.
     */
    String getSimOperator();

    /**
     * Returns the ISO-3166-1 alpha-2 country code equivalent for the SIM provider's country code.
     * The ISO-3166-1 alpha-2 country code is provided in lowercase 2 character format.
     *
     * @return The lowercase 2 character ISO-3166-1 alpha-2 country code, or empty string is not
     *         available.
     */
    String getSimCountryIso();

    /**
     * Returns the serial number of the SIM, if applicable. Return null if it is unavailable.
     */
    String getSimSerialNumber();

    /**
     * Returns the Group Identifier Level1 for a GSM phone. Returns null if it is unavailable.
     */
    String getGroupIdLevel1();

    /**
     * Returns the Service Provider Name (SPN).
     *
     * Availability: SIM state must be {@link TelephonyManager#SIM_STATE_READY}.
     */
    String getSimOperatorName();

    /**
     * Fetches the sim service table from the EFUST/EFIST based on the application type
     * {@link TelephonyManager#APPTYPE_USIM} or {@link TelephonyManager#APPTYPE_ISIM}.
     * The return value is a byte array.
     * Each bit of every byte indicates which optional services are available
     * for the given application type.
     *
     * @param appType The UICC application type.
     * @return The byte array representing sim service table or an empty byte array
     *         if not available.
     */
    @NonNull byte[] getSimServiceTable(@UiccAppType int appType);

    /**
     * Returns the IMS home network domain name that was loaded from the ISIM
     * {@link TelephonyManager#APPTYPE_ISIM}.
     *
     * @return The IMS domain name. Returns null if ISIM hasn't been loaded or IMS domain
     *         hasn't been loaded or isn't present on the ISIM.
     */
    String getIsimDomain();

    /**
     * Returns the IMS private user identity (IMPI) of the subscription that was loaded from the
     * ISIM records {@link TelephonyManager#APPTYPE_ISIM}.
     *
     * @return IMPI (IMS private user identity) of type string.
     */
    String getImsPrivateUserIdentity();

    /**
     * Returns the IMS public user identities (IMPU) of the subscription that was loaded from the
     * ISIM records {@link TelephonyManager#APPTYPE_ISIM}.
     *
     * @return List of public user identities of type {@link Uri} or
     *         empty list if EF_IMPU is not available.
     */
    List<Uri> getImsPublicUserIdentities();

    /**
     * Returns the response of authentication for the default subscription.
     * Returns null if the authentication hasn't been successful.
     *
     * @param appType The UICC application type, like {@link TelephonyManager#APPTYPE_USIM}
     * @param authType The authentication type, any one of
     *                 {@link TelephonyManager#AUTHTYPE_EAP_AKA} or
     *                 {@link TelephonyManager#AUTHTYPE_EAP_SIM} or
     *                 {@link TelephonyManager#AUTHTYPE_GBA_BOOTSTRAP} or
     *                 {@link TelephonyManager#AUTHTYPE_GBA_NAF_KEY_EXTERNAL}
     * @param data Authentication challenge data, base64 encoded.
     * @return The response of authentication. This value will be null in the following cases:
     *         Authentication error, incorrect MAC
     *         Authentication error, security context not supported
     *         Key freshness failure
     *         Authentication error, no memory space available
     *         Authentication error, no memory space available in EFMUK
     */
    String getIccAuthentication(@UiccAppType int appType, @AuthType int authType, String data);

    /**
     * Send ENVELOPE to the SIM and return the response.
     *
     * @param content String containing SAT/USAT response in hexadecimal
     *                format starting with command tag. See TS 102 223 for details.
     * @return The APDU response from the ICC card in hexadecimal format
     *         with the last 4 bytes being the status word. If the command fails,
     *         returns an empty string.
     */
    String sendEnvelopeWithStatus(String content);

    /**
     * Returns whether mobile data is enabled or not per user setting. There are other factors
     * that could disable mobile data, but they are not considered here.
     *
     * @return {@code true} if mobile data is enabled, {@code false} otherwise.
     */
    boolean isDataEnabled();

    /**
     * Returns whether mobile data roaming is enabled on the subscription.
     *
     * @return {@code true} if the data roaming is enabled on the subscription,
     *         {@code false} otherwise.
     */
    boolean isDataRoamingEnabled();

    /**
     * Returns the current {@link ServiceState} information.
     */
    @Nullable ServiceState getServiceState();

    /**
     * Returns a constant indicating the radio technology (network type)
     * currently in use on the device for data transmission.
     *
     * @return The network type.
     */
    @NetworkType int getDataNetworkType();

    /**
     * Returns the NETWORK_TYPE_xxxx for voice.
     *
     * @return The network type.
     */
    @NetworkType int getVoiceNetworkType();

    /**
     * Returns the numeric name (MCC+MNC) of current registered operator.
     */
    String getNetworkOperator();

    /**
     * Returns the ISO-3166-1 alpha-2 country code equivalent of the MCC (Mobile Country Code) of
     * the current registered operator or the cell nearby, if available.
     *
     * @return The lowercase 2 character ISO-3166-1 alpha-2 country code, or empty string if not
     *         available.
     */
    String getNetworkCountryIso();

    /**
     * Requests all available cell information from all radios on the device including the
     * camped/registered, serving, and neighboring cells.
     *
     * @return List of {@link CellInfo}; null if cell information is unavailable.
     */
    List<CellInfo> getAllCellInfo();

    /**
     * Requests all available cell information from the current subscription for observed
     * camped/registered, serving, and neighboring cells.
     *
     * @param executor The executor on which callback will be invoked.
     * @param callback A callback to receive {@link CellInfo}.
     */
    void requestCellInfoUpdate(@NonNull @CallbackExecutor Executor executor,
            @NonNull CellInfoCallback callback);

    /**
     * Used to get the Generic Bootstrapping Architecture authentication keys
     * KsNAF/Ks_ext_NAF for a particular NAF as defined in 3GPP spec TS 33.220 for
     * the specified subscription.
     *
     * @param appType The UICC application type, like {@link TelephonyManager#APPTYPE_USIM} or
     *                {@link TelephonyManager#APPTYPE_ISIM}.
     * @param nafId A URI to specify Network Application Function(NAF) fully qualified domain
     *              name (FQDN) and the selected GBA mode. The authority of the URI must contain
     *              two parts delimited by "@" sign. The first part is the constant string
     *              "3GPP-bootstrapping" (GBA_ME), "3GPP-bootstrapping-uicc" (GBA_ U), or
     *              "3GPP-bootstrapping-digest" (GBA_Digest).
     * @param securityProtocol Security protocol identifier between UE and NAF.
     *                         See 3GPP TS 33.220 Annex H. Application can use
     *                         {@link UaSecurityProtocolIdentifier#createDefaultUaSpId},
     *                         {@link UaSecurityProtocolIdentifier#create3GppUaSpId},
     *                         to create the ua security protocol identifier as needed.
     * @param forceBootStrapping true=force bootstrapping, false=do not force bootstrapping.
     *                           Bootstrapping shouldn't be forced unless the application sees
     *                           authentication errors from the server.
     * @param executor The {@link Executor} that will be used to call the Gba callback.
     * @param callback A callback called on the supplied {@link Executor} that will contain
     *                 the GBA Ks_NAF/Ks_ext_NAF when available. If the NAF keys are available
     *                 and valid at the time of call and bootstrapping is not requested,
     *                 then the callback shall be invoked with the available keys.
     */
    void bootstrapAuthenticationRequest(@UiccAppTypeExt int appType, @NonNull Uri nafId,
            @NonNull UaSecurityProtocolIdentifier securityProtocol, boolean forceBootStrapping,
            @NonNull Executor executor, @NonNull BootstrapAuthenticationCallback callback);

    /**
     * Retrieves the HAL Version of a specific service for this device.
     *
     * Get the HAL version for a specific HAL interface for test purposes.
     *
     * @param halService The service id to query.
     * @return A Pair of (major version, minor version), HAL_VERSION_UNKNOWN if unknown
     *         or HAL_VERSION_UNSUPPORTED if unsupported.
     */
    @NonNull Pair<Integer, Integer> getHalVersion(@HalService int halService);
}
