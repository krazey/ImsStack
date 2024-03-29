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

/**
 * An interface that notifies the native layer of system events.
 */
public interface ISystem {
    /**
     * Sets the {@link SystemCallInterface} to perform the system call from the native layer.
     *
     * @param systemCall The {@link SystemCallInterface} to be set.
     */
    void setSystemCallInterface(SystemCallInterface systemCall);

    /**
     * Notifies the changes of airplane mode in the phone settings.
     *
     * @param airplaneMode the current airplane mode status 0: Airplane mode
     *            OFF, 1: Airplane mode ON
     */
    void notifyAirplaneModeChanged(int airplaneMode);

    /**
     * Notifies the failure result to connect a data connection of the specified
     * APN type.
     *
     * @param apnType the APN type (1: ims, 2: internet, 3: xcap, 9: emergency,
     *            21: wifi)
     */
    void notifyDataConnectionFailed(int apnType);

    /**
     * Notifies the IPCAN category of the attached data connection.
     *
     * @param apnType the APN type (1: ims, 2: internet, 3: xcap, 9: emergency,
     *            21: wifi)
     * @param ipcanCategory the IPCAN category (0: MOBILE, 1: WLAN); Refer to IIPCAN.h
     */
    void notifyDataConnectionIpcanChanged(int apnType, int ipcanCategory);

    /**
     * Notifies the state of the specified data connection.
     *
     * @param apnType the APN type (1: ims, 2: internet, 3: xcap, 9: emergency,
     *            21: wifi)
     * @param state the data state (TelephonyManager.DATA_*)
     *            {@link TelephonyManager.DATA_UNKNOWN} (-1)
     *            {@link TelephonyManager.DATA_DISCONNECTED} (0)
     *            {@link TelephonyManager.DATA_CONNECTING} (1)
     *            {@link TelephonyManager.DATA_CONNECTED} (2)
     *            {@link TelephonyManager.DATA_SUSPENDED} (3)
     */
    void notifyDataConnectionStateChanged(int apnType, int state);

    /**
     * Notifies the network type which the device is attached.
     *
     * @param networkType the network type (TelephonyManager.NETWORK_TYPE_*)
     *          {@link RAT_NONET} (0)
     *          {@link RAT_EHRPD} (1)
     *          {@link RAT_4G} (2)
     *          {@link RAT_3G} (3)
     *          {@link RAT_1XRTT} (4)
     *          {@link RAT_2G} (5)
     *          {@link RAT_EVDO} (6)
     *          {@link RAT_5G} (7)
     */
    void notifyNetworkTypeChanged(int networkType);

    /**
     * Notifies the voice network type which the device is attached.
     *
     * @param networkType defined in DcNetWatcher
     *          {@link RAT_NONET} (0)
     *          {@link RAT_EHRPD} (1)
     *          {@link RAT_4G} (2)
     *          {@link RAT_3G} (3)
     *          {@link RAT_1XRTT} (4)
     *          {@link RAT_2G} (5)
     *          {@link RAT_EVDO} (6)
     *          {@link RAT_5G} (7)
     */
    void notifyVoiceNetworkTypeChanged(int networkType);

    /**
     * Notifies the service state related to the attached network.
     *
     * @param serviceState the service state (ServiceState.STATE_*)
     *            {@link ServiceState.STATE_IN_SERVICE} (0)
     *            {@link ServiceState.STATE_OUT_OF_SERVICE} (1)
     *            {@link ServiceState.STATE_EMERGENCY_ONLY} (2)
     *            {@link ServiceState.STATE_POWER_OFF} (3)
     */
    void notifyServiceStateChanged(int serviceState);

    /**
     * Notifies the voice call (CS / IMS) state.
     *
     * @param state the call state (TelephonyManager.CALL_STATE_*)
     *            {@link TelephonyManager.CALL_STATE_IDLE} (0)
     *            {@link TelephonyManager.CALL_STATE_RINGING} (1)
     *            {@link TelephonyManager.CALL_STATE_OFFHOOK} (2)
     */
    void notifyVoiceCallStateChanged(int state);

    /**
     * Notifies the changes of the IMS configuration.
     *
     * @param configs the configuration items to be updated
     */
    void notifyConfigurationChanged(int configs);

    /**
     * Notifies the events which are registered by the native modules.
     *
     * @param event the current event
     * @param param1 the parameter related to the current event
     * @param param2 the additional parameter related to the current event
     */
    void notifyEvent(int event, int param1, int param2);

    /**
     * Notifies the ISIM state to the native module.
     *
     * @param event The current event.
     * @param state The current ISIM state.
     */
    void notifyIsimState(int event, String state);

    /**
     * Notifies the ISIM file attributes response to the native module.
     *
     * @param event The current event.
     * @param fileId The file id to be responded.
     * @param size The size of the specified file id.
     * @param values The content of the specified file id.
     */
    void notifyIsimFileAttributesResponse(int event, int fileId, int size, String[] values);

    /**
     * Notifies the ISIM record response to the native module.
     *
     * @param event The current event.
     * @param fileId The file id to be responded.
     * @param index The index of the specified file id.
     * @param value The content of the specified file id.
     */
    void notifyIsimRecordResponse(int event, int fileId, int index, String value);

    /**
     * Notifies the ISIM authentication response to the native module.
     *
     * @param event The current event.
     * @param response The ISIM authentication response.
     * @param owner The owner of this request.
     */
    void notifyIsimAuthenticationResponse(int event, String response, long owner);

    /**
     * Notifies the USIM authentication response to the native module.
     *
     * @param event The current event.
     * @param response The USIM authentication response.
     * @param owner The owner of this request.
     */
    void notifyUsimAuthenticationResponse(int event, String response, long owner);

    /**
     * Notifies the reason of the connection setup failure corresponding with the IMS traffic type.
     *
     * @param event The current event.
     * @param id The identification for IMS traffic.
     * @param failureReason The reason of connection failure based on IMS traffic type.
     * @param causeCode Failure cause code from network or modem specific to the failure.
     * @param waitTimeMillis Retry wait time provided by network in milliseconds
     */
    void notifyRadioConnectionFailed(int event, int id, int failureReason, int causeCode,
            int waitTimeMillis);

    /**
     * Notifies the preparation of the connection setup corresponding with the IMS traffic type.
     *
     * @param event The current event.
     * @param id The identification for IMS traffic.
     */
    void notifyRadioConnectionSetupPrepared(int event, int id);

    /**
     * Notifies the SSAC information.
     *
     * @param event The current event.
     * @param voiceFactor  The conditional barring factor as a percentage 0-100, which is the
     *                     probability of a random device being barred for the voice type.
     * @param voiceTimeSec The conditional barring time seconds, which is the interval between
     *                     successive evaluations for conditional barring based on the voice
     *                     barring factor.
     * @param videoFactor  The conditional barring factor for the video type.
     * @param videoTimeSec The conditional barring time seconds for the video type.
     */
    void notifySsacInfo(int event, int voiceFactor, int voiceTimeSec, int videoFactor,
            int videoTimeSec);

    /**
     * Notifies modem's simultaneous calling support information.
     *
     * @param event The current event.
     * @param isSupported The information whether modem supports simultaneous calling.
     */
    void notifySimultaneousCallingSupportChanged(int event, boolean isSupported);
}
