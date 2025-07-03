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
     * Returns whether one of the {@link IDcSettings#getImsSupportedRats} is available.
     */
    boolean isRatPolicyAvailable();

    /**
     * Returns the service state of the PS domain (cellular only).
     *
     * The {@link #getDataServiceState} reads the service state of the current device's data
     * network (including IWLAN), while this method reads the service state of the current device's
     * cellular data network.
     */
    int getCellularDataServiceState();

    /**
     * Returns service state of the PS domain (cellular + IWLAN).
     */
    int getDataServiceState();

    /**
     * Returns data network type.
     */
    int getNetworkType();

    /**
     * Returns voice network type.
     */
    int getVoiceNetworkType();

    /**
     * Returns voice service state.
     */
    int getVoiceServiceState();

    /**
     * Returns MOCN(multi-operator core network) PLMN information.
     */
    int getMocnPlmnInfo();

    /**
     * Returns network registration reject cause stored in DcNetWatcher object
     */
    int getNetworkRegistrationRejectCause();

    /**
     * Clears network registration reject cause stored in DcNetWatcher object.
     */
    void clearNetworkRegistrationRejectCause();

    /**
     * Returns current registered operator numeric id.
     */
    String getOperatorNumeric();

    /**
     * Returns whether airplane mode is enabled.
     */
    boolean isAirplaneMode();

    /**
     * Returns whether in an emergency attached state.
     */
    boolean isEmergencyOnly();

    /**
     * Returns whether emergency service is supported by the network
     */
    boolean isEmergencyServiceSupported();

    /**
     * Returns current roaming indicator of phone.
     * This roaming state could be overridden by the resource overlay or carrier config.
     */
    boolean isRoaming();

    /**
     * Returns whether the voice network is roaming.
     * This roaming state could be overridden by the resource overlay or carrier config.
     */
    boolean isVoiceRoaming();

    /**
     * Returns whether mobile data is registered on roaming network.
     * This value is not affected by any carrier config or resource overlay override.
     */
    boolean isDataNetworkRoaming();

    /**
     * Returns voice roaming type.
     * This roaming type could be overridden by the resource overlay or carrier config.
     */
    int getVoiceRoamingType();

    /**
     * Returns data roaming type.
     * This roaming type could be overridden by the resource overlay or carrier config.
     */
    int getDataRoamingType();

    /**
     * Returns whether VoPS is supported that stored in DcNetWatcher object.
     * If {@link IDcSettings#isVopsIgnored} is true it always returns VoPS is supported.
     */
    boolean isVopsSupported();

    /**
     * Returns duplex mode for the phone.
     */
    int getLteDuplexMode();

    /**
     * Stores the network type obtained from the TelephonyManager.
     * It is used for checking network type mismatch between ServiceState and TelephonyManager.
     */
    void setRatFromTelephonyManager(int nRat);

    /**
     * Stores the voice network type obtained from the TelephonyManager.
     * It is used for checking network type mismatch between ServiceState and TelephonyManager.
     */
    void setVoiceRatFromTelephonyManager(int nVoiceRat);

    /**
     * Returns whether the current network type belongs to the 1xRTT category.
     */
    boolean is1xRtt();

    /**
     * Returns whether the current network type belongs to the 2G category.
     */
    boolean is2G();

    /**
     * Returns whether the current network type belongs to the 3G category.
     */
    boolean is3G();

    /**
     * Returns whether the current network type belongs to the 4G category.
     */
    boolean is4G();

    /**
     * Returns whether the current network type belongs to the 5G category.
     */
    boolean is5G();

    /**
     * Returns whether IMS service is supported in the 5G network.
     */
    boolean is5GRequired();

    /**
     * Returns whether the current network type belongs to the HRPD category.
     */
    boolean isEhrpd();

    /**
     * Returns whether the current network type belongs to the EVDO category.
     */
    boolean isEvdo();

     /**
      * Returns whether the current voice network type belongs to the 4G category.
      */
    boolean isVoiceRat4G();

     /**
      * Returns whether the current voice network type belongs to 5G category.
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
