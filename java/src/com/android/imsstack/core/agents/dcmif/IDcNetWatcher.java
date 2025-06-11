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

/**
 * this class is the interface about data connection watcher
 */
public interface IDcNetWatcher extends IDc {
    int REGISTRATION_REJECT_CAUSE_NONE = 0;

    /**
     * Return service is available or not based on
     * 1) RAT policy configuration
     * 2) Current RAT information
     */
    boolean isRatPolicyAvailable();

    /**
     * Return data service state stored in DcNetWatcher object
     */
    int getDataServiceState();

    /**
     * Return network type stored in DcNetWatcher object
     */
    int getNetworkType();

    /**
     * Return voice network type stored in DcNetWatcher object
     */
    int getVoiceNetworkType();

    /**
     * Return voice service state stored in DcNetWatcher object
     */
    int getVoiceServiceState();

    /**
     * Return MOCNPLMN info stored in DcNetWatcher object
     */
    int getMocnPlmnInfo();

    /**
     * Return network registration reject cause stored in DcNetWatcher object
     */
    int getNetworkRegistrationRejectCause();

    /**
     * Clear network registration reject cause stored in DcNetWatcher object.
     */
    void clearNetworkRegistrationRejectCause();

    /**
     * Return operator info (numeric type) stored in DcNetWatcher object
     */
    String getOperatorNumeric();

    /**
     * Return airplane mode availability stored in DcNetWatcher object
     */
    boolean isAirplaneMode();

    /**
     * Return if the registration state is
     * {@code NetworkRegistrationInfo#REGISTRATION_STATE_EMERGENCY}.
     */
    boolean isEmergencyOnly();

    /**
     * Return whether emergency service is supported by the network
     */
    boolean isEmergencyServiceSupported();

    /**
     * Return current roaming state
     * (This roaming state could be overridden by the carrier config)
     * @see CarrierConfigManager#KEY_FORCE_HOME_NETWORK_BOOL
     * @see CarrierConfigManager#KEY_GSM_ROAMING_NETWORKS_STRING_ARRAY
     * @see CarrierConfigManager#KEY_GSM_NONROAMING_NETWORKS_STRING_ARRAY
     * @see CarrierConfigManager#KEY_CDMA_ROAMING_NETWORKS_STRING_ARRAY
     * @see CarrierConfigManager#KEY_CDMA_NONROAMING_NETWORKS_STRING_ARRAY
     */
    boolean isRoaming();

    /**
     * Return voice roaming state of device
     */
    boolean isVoiceRoaming();

    /**
     * Return whether mobile data is registered on roaming network
     * (This value is not affected by any carrier config or resource overlay override)
     */
    boolean isDataNetworkRoaming();

    /**
     * Return Voice roaming type of device
     */
    int getVoiceRoamingType();

    /**
     * Return Data roaming type of device
     */
    int getDataRoamingType();

    /**
     * Returns whether VoPS is supported that cached in DcNetWatcher object.
     * If {@link IDcSettings#isVopsIgnored} is true it always returns VoPS is supported.
     */
    boolean isVopsSupported();

    /**
     * Return LTE duplex mode stored in ServiceState object
     */
    int getLteDuplexMode();

    /**
     * For check mismatch of DATA tech type between ServiceState and TelephonyManager
     */
    void setRatFromTelephonyManager(int nRat);

    /**
     * For check mismatch of Voice tech type between ServiceState and TelephonyManager
     */
    void setVoiceRatFromTelephonyManager(int nVoiceRat);

    /**
     * Return current RAT is belong to 1xRTT RAT category
     */
    boolean is1xRtt();

    /**
     * Return current RAT is belong to 2G RAT category
     */
    boolean is2G();

    /**
     * Return current RAT is belong to 3G RAT category
     */
    boolean is3G();

    /**
     * Return current RAT is belong to 4G RAT category
     */
    boolean is4G();

    /**
     * Return current RAT is belong to 5G RAT category
     */
    boolean is5G();

    /**
     * Return condition if 5G RAT is supported
     */
    boolean is5GRequired();

    /**
     * Return current RAT is belong to HRPD RAT category
     */
    boolean isEhrpd();

    /**
     * Return current RAT is belong to EVDO RAT category
     */
    boolean isEvdo();

     /**
      * Return current RAT is belong to 4G Voice RAT category
      */
    boolean isVoiceRat4G();

     /**
      * Return current RAT is belong to 4G Voice RAT category
      */
    boolean isVoiceRat5G();

    /**
     * Listener interface to receive the change notification of network status.
     */
    interface Listener {
        /**
         * Invoked when data connection state is changed.
         */
        default void onDataConnectionStateChanged(EApnType apnType, EDataState dataState) {
        }

        /**
         * Invoked when data service state is changed.
         */
        default void onDataServiceStateChanged(int state) {
        }

        /**
         * Invoked when data network type is changed.
         */
        default void onDataNetworkTypeChanged() {
        }

        /**
         * Invoked when voice network type is changed.
         */
        default void onVoiceNetworkTypeChanged() {
        }

        /**
         * Invoked when the numeric ID of the network operator is changed.
         */
        default void onNetworkOperatorChanged(String networkOperator) {
        }

        /**
         * Invoked when roaming state is changed.
         */
        default void onRoamingStateChanged(boolean roaming) {
        }

        /**
         * Invoked when airplane mode is changed.
         */
        default void onAirplaneModeChanged(boolean airplaneMode) {
        }

        /**
         * Invoked when VoPS state is changed.
         */
        default void onVopsStateChanged(int state, String plmn) {
        }

        /**
         * Invoked when data connection attempt is failed.
         */
        default void onPdnConnectionFailed(EApnType apnType, int smCause) {
        }
    }

    /**
     * Adds a listener to monitor the network status change.
     *
     * @param listener The listener to be set.
     */
    void addListener(Listener listener);

    /**
     * Removes the listener that was previously set.
     *
     * @param listener The listener to be removed.
     */
    void removeListener(Listener listener);

    /**
     * Notifies data connection state is changed
     */
    void notifyDataConnectionState(EApnType apnType, EDataState dataState);

    /**
     * Notifies data connection attempt is failed with the cause
     */
    void notifyPdnConnectionFailed(EApnType apnType, int smCause);
}
