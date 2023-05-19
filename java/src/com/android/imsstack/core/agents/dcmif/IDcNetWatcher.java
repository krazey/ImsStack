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

import android.os.Handler;

/**
 * this class is the inferface about data connection watcher
 */
public interface IDcNetWatcher extends IDc {

    /**
     * this class is data object to notify
     */
    class NotiObj {
        public EApnType   eApnType;
        public EDataState eDataState;
        public int mSmCause;

        public NotiObj(EApnType apnType, EDataState dataState, int smCause) {
            this.eApnType = apnType;
            this.eDataState = dataState;
            this.mSmCause = smCause;
        }
    }

    /**
     *
     * Notify data state is changed to target apn object
     *
     * Only subject class can invoke this API.
     * DO NOT allow to accessed by observer class
     */
    void notifyResult(EApnType eApnType, EDataState eDataState);

    /**
     * Return service is available or not based on
     * 1) RAT policy configuration
     * 2) Current RAT information
     */
    boolean isRatPolicyAvailable();

    /**
     * Return call state stored in DcNetWatcher object
     */
    int getCallState();

    /**
     * Return call state stored in DcNetWatcher object
     */
    int getPreciseCallState();

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
     * Return NR registration info stored in DcNetWatcher object
     */
    int getNrRegistrationInfo();

    /**
     * Return MOCNPLMN info stored in DcNetWatcher object
     */
    int getMocnPlmnInfo();

    /**
     * Return operator info (numeric type) stored in DcNetWatcher object
     */
    String getOperatorNumeric();

    /**
     * Return airplane mode availability stored in DcNetWatcher object
     */
    boolean isAirplaneMode();

    /**
     * Return if LTE is attached as emergency case
     */
    boolean isLteEmergencyOnly();

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
     * Return VoPS value stored in DcNetWatcher object
     */
    boolean isVops();

    /**
     * Return LTE duplex mode stored in ServiceState object
     */
    int getLteDuplexMode();

    /**
     * Set if "Intent.ACTION_REBOOT" was delivered to IMS
     * Special operator has requirement in this case related with data connection
     */
    void setDoingOffRadio(boolean b);

    /**
     * Return if "Intent.ACTION_REBOOT" was delivered to IMS or not
     */
    boolean isDoingOffRadio();

    /**
     * For check mismatch of DATA tech type between ServiceState and TelephonyManager
     */
    void setRatFromTelephonyManager(int nRat);

    /**
     * For check mismatch of Voice tech type between ServiceState and TelephonyManager
     */
    void setVoiceRatFromTelephonyManager(int nVoiceRat);

    /**
     * Set NR registration information
     */
    void setNrRegistrationInfo(int state, int reason);

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
     *     registerForDataStateChanged
     *
     *     Register listener to receive data state changed event
     *
     *        Register "h" of Handler
     *        The "h" will get message with "what" of event
     *        and "obj" of IDcNetWatcher.NotiObj
     *
     *        Typical usage.
     *        {@code class Example extends Handler{
     *            Example{
     *                registerForDataStateChanged(this, EVENT_NAME, null);
     *            }
     *
     *            @Override
     *            public void handleMessage(Message msg) {
     *                AsyncResult ar = (AsyncResult)msg.obj;

     *                IDcNetWatcher.NotiObj res = (IDcNetWatcher.NotiObj) ar.result;
     *                EApnType apnType = res.eApnType;
     *                EDataState state = res.eDataState;
     *                int smCause = res.mSmCause;
     *            }
     *        }}
     */
    void registerForDataStateChanged(Handler h, int what, Object obj);

    /**
     * De-register listener to receive data state changed event
     */
    void unregisterForDataStateChanged(Handler h);

    /**
     * Registerlistener to receive data service state changed event
     */
    void registerForDataServiceStateChanged(Handler h, int what, Object obj);

    /**
     * De-register listener to receive data service state changed event
     */
    void unregisterForDataServiceStateChanged(Handler h);

    /**
     * Register listener to receive RAT changed event
     */
    void registerForRatChanged(Handler h, int what, Object obj);

    /**
     * De-register listener to receive RAT changed event
     */
    void unregisterForRatChanged(Handler h);

    /**
     * Register listener to receive voice RAT changed event
     */
    void registerForVoiceRatChanged(Handler h, int what, Object obj);

    /**
     * De-register listener to receive voice RAT changed event
     */
    void unregisterForVoiceRatChanged(Handler h);

    /**
     * Register listener to receive roaming state changed event
     */
    void registerForRoamingStateChanged(Handler h, int what, Object obj);

    /**
     * De-register listener to receive roaming state changed event
     */
    void unregisterForRoamingStateChanged(Handler h);

    /**
     * Register listener to receive voice roaming state changed event
     */
    void registerForVoiceRoamingStateChanged(Handler h, int what, Object obj);

    /**
     * De-register listener to receive voice roaming state changed event
     */
    void unregisterForVoiceRoamingStateChanged(Handler h);

     /**
     * Register listener to receive voice roaming type changed event
     */
    void registerForVoiceRoamingTypeChanged(Handler h, int what, Object obj);

    /**
     * De-register listener to receive voice roaming type changed event
     */
    void unregisterForVoiceRoamingTypeChanged(Handler h);

     /**
     * Register listener to receive data roaming type changed event
     */
    void registerForDataRoamingTypeChanged(Handler h, int what, Object obj);

    /**
     * De-register listener to receive data roaming type changed event
     */
    void unregisterForDataRoamingTypeChanged(Handler h);

    /**
     * Register listener to receive airplane mode changed event
     */
    void registerForAirplaneModeChanged(Handler h, int what, Object obj);

    /**
     * De-register listener to receive airplane mode changed event
     */
    void unregisterForAirplaneModeChanged(Handler h);

    /**
     * Register listener to receive network operator changed event
     */
    void registerForNetworkOperatorChanged(Handler h, int what, Object obj);

    /**
     * De-register listener to receive network operator changed event
     */
    void unregisterForNetworkOperatorChanged(Handler h);

    /**
     * Register listener to receive CS call state changed event
     */
    void registerForCsCallStatusChanged(Handler h, int what, Object obj);

    /**
     * De-register listener to receive CS call state changed event
     */
    void unregisterForCsCallStatusChanged(Handler h);

    /**
     * Register listener to receive precise CS call state changed event
     */
    void registerForPreciseCsCallStatusChanged(Handler h, int what, Object obj);

    /**
     * De-register listener to receive precise CS call state changed event
     */
    void unregisterForPreciseCsCallStatusChanged(Handler h);

    /**
     * Register listener to receive Voice of PS changed event
     */
    void registerForImsVopsChanged(Handler h, int what, Object obj);

    /**
     * De-register listener to receive Voice of PS changed event
     */
    void unregisterForImsVopsChanged(Handler h);

    /**
     * Register listener to receive power off changed event
     */
    void registerForPowerOffChanged(Handler h, int what, Object obj);

    /**
     * De-register listener to receive power off changed event
     */
    void unregisterForPowerOffChanged(Handler h);

    /**
     * Register listener to receive pdn connection fail event
     * This api is used to handle XCAP pdn connection fail case, for now.
     */
    void registerForPdnConnectionFailed(Handler h, int what, Object obj);

    /**
     * De-register listener to receive pdn connection fail event
     * This api is used to handle XCAP pdn connection fail case, for now.
     */
    void unregisterForPdnConnectionFailed(Handler h);

    /**
     * Notify pdn connection failed event with sm cause to listeners
     */
    void notifyPdnConnectionFailed(EApnType eApnType, int smCause);
}
