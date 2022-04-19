package com.android.imsstack.core.agents.dcmif;

import android.os.Handler;

public interface IDCNetWatcher extends IDC {

    class NotiObj {
        public EApnType   eApnType;
        public EDataState eDataState;

        public NotiObj(EApnType apnType, EDataState dataState) {
            this.eApnType = apnType;
            this.eDataState = dataState;
        }
    }

    /**
     *
     * Notify data state is changed to target apn object
     *
     * Only subject class can invoke this API.
     * DO NOT allow to accessed by observer class
     *
     * @param
     * @return
     */
    void notifyResult(EApnType eApnType, EDataState eDataState);

    /**
     * Return service is available or not based on
     * 1) RAT policy configuration
     * 2) Current RAT information
     */
    boolean isRatPolicyAvailable();

    /**
     * Return call state stored in DCNetWatcher object
     */
    int getCallState();

    /**
     * Return call state stored in DCNetWatcher object
     */
    int getPreciseCallState();

    /**
     * Return data service state stored in DCNetWatcher object
     */
    int getDataServiceState();

    /**
     * Return NR state stored in DCNetWatcher object
     */
    int getNrState();

    /**
     * Return network type stored in DCNetWatcher object
     */
    int getNetworkType();

    /**
     * Return voice network type stored in DCNetWatcher object
     */
    int getVoiceNetworkType();

    /**
     * Return voice service state stored in DCNetWatcher object
     */
    int getVoiceServiceState();

    /**
     * Return LTE Detach reason code stored in DCNetWatcher object
     */
    int getLteStateDetachReasonCause();

    /**
     * Return NR registration info stored in DCNetWatcher object
     */
    int getNrRegistrationInfo();

    /**
     * Return MOCNPLMN info stored in DCNetWatcher object
     */
    int getMocnPlmnInfo();

    /**
     * Return operator info (numeric type) stored in DCNetWatcher object
     */
    String getOperatorNumeric();

    /**
     * Return airplane mode availability stored in DCNetWatcher object
     */
    boolean isAirplaneMode();

    /**
     * Return if LTE is attached as emergency case
     */
    boolean isLteEmergencyOnly();

    /**
     * Returns whether emergency service is supported by the network
     */
    boolean isEmergencyServiceSupported();

    /**
     * Return UE is under roaming state
     */
    boolean isRoaming();

    /**
     * Return voice roaming state of device
     */
    boolean isVoiceRoaming();

    /**
     * Return Voice roaming type of device
     */
    int getVoiceRoamingType();

    /**
     * Return Data roaming type of device
     */
    int getDataRoamingType();

    /**
     * Return VoPS value stored in DCNetWatcher object
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
     *     Register/De-register listener to receive data state changed event
     *
     *        Register "h" of Handler
     *        The "h" will get message with "what" of event
     *        and "obj" of IDCNetWatcher.NotiObj
     *
     *        Typical usage.
     *        class Example extends Handler{
     *            Example{
     *                registerForDataStateChanged(this, EVENT_NAME, null);
     *            }
     *
     *            @Override
     *            public void handleMessage(Message msg) {
     *                AsyncResult ar = (AsyncResult)msg.obj;

     *                IDCNetWatcher.NotiObj res = (IDCNetWatcher.NotiObj) ar.result;
     *                EApnType apnType = res.eApnType;
     *                EDataState state = res.eDataState;
            }
     * @param
     * @return
     */
    void registerForDataStateChanged(Handler h, int what, Object obj);
    void unregisterForDataStateChanged(Handler h);

    /**
     * Register/De-register listener to receive data service state changed event
     */
    void registerForDataServiceStateChanged(Handler h, int what, Object obj);
    void unregisterForDataServiceStateChanged(Handler h);

    /**
     * Register/De-register listener to receive RAT changed event
     */
    void registerForRatChanged(Handler h, int what, Object obj);
    void unregisterForRatChanged(Handler h);

    /**
     * Register/De-register listener to receive voice RAT changed event
     */
    void registerForVoiceRatChanged(Handler h, int what, Object obj);
    void unregisterForVoiceRatChanged(Handler h);

    /**
     * Register/De-register listener to receive roaming state changed event
     */
    void registerForRoamingStateChanged(Handler h, int what, Object obj);
    void unregisterForRoamingStateChanged(Handler h);

    /**
     * Register/De-register listener to receive voice roaming state changed event
     */
    void registerForVoiceRoamingStateChanged(Handler h, int what, Object obj);
    void unregisterForVoiceRoamingStateChanged(Handler h);

     /**
     * Register/De-register listener to receive voice roaming type changed event
     */
    void registerForVoiceRoamingTypeChanged(Handler h, int what, Object obj);
    void unregisterForVoiceRoamingTypeChanged(Handler h);

     /**
     * Register/De-register listener to receive data roaming type changed event
     */
    void registerForDataRoamingTypeChanged(Handler h, int what, Object obj);
    void unregisterForDataRoamingTypeChanged(Handler h);

    /**
     * Register/De-register listener to receive airplane mode changed event
     */
    void registerForAirplaneModeChanged(Handler h, int what, Object obj);
    void unregisterForAirplaneModeChanged(Handler h);

    /**
     * Register/De-register listener to receive network operator changed event
     */
    void registerForNetworkOperatorChanged(Handler h, int what, Object obj);
    void unregisterForNetworkOperatorChanged(Handler h);

    /**
     * Register/De-register listener to receive CS call state changed event
     */
    void registerForCsCallStatusChanged(Handler h, int what, Object obj);
    void unregisterForCsCallStatusChanged(Handler h);

    /**
     * Register/De-register listener to receive precise CS call state changed event
     */
    void registerForPreciseCsCallStatusChanged(Handler h, int what, Object obj);
    void unregisterForPreciseCsCallStatusChanged(Handler h);

    /**
     * Register/De-register listener to receive Voice of PS changed event
     */
    void registerForImsVopsChanged(Handler h, int what, Object obj);
    void unregisterForImsVopsChanged(Handler h);

    /**
     * Register/De-register listener to receive power off changed event
     */
    void registerForPowerOffChanged(Handler h, int what, Object obj);
    void unregisterForPowerOffChanged(Handler h);

    /**
     * Register/De-register listener to receive pdn connection fail event
     * This api is used to handle XCAP pdn connection fail case, for now.
     */
    void registerForPdnConnectionFailed(Handler h, int what, Object obj);
    void unregisterForPdnConnectionFailed(Handler h);

    /**
     * Notify pdn connection failed event to listeners
     */
    void notifyPdnConnectionFailed(EApnType eApnType);
}
