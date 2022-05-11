package com.android.imsstack.core.agents.agentif;

public interface ITelephonySubscriber extends IAgent {
    /**
     * Returns MCC/MNC value from SIM card if fromSim is 'true'
     * Returns MCC/MNC value from network operator if fromSim is 'false'
     */
    String getMccMnc(boolean fromSim);

    /**
     * Returns MCC/MNC value from target SIM card
     */
    String getMccMnc(int subId);

    /**
     * Returns MCC value from SIM card if fromSim is 'true'
     * Returns MCC value from network operator if fromSim is 'false'
     */
    String getMcc(boolean fromSim);

    /**
     * Returns MNC value from SIM card if fromSim is 'true'
     * Returns MNC value from network operator if fromSim is 'false'
     */
    String getMnc(boolean fromSim);

    /**
     * Returns Operator value based on MCC
     */
    String getSimOperatorInternal();

    /**
     * Returns Country Iso value via TelephonyManager
     * If it is not possible, return Country Iso value stored in TelephonySubscriberAgent.java class
     */
    String getCountryIso(boolean fromSim);

    /**
     * Returns line1number via TelephonyManager.
     */
    String getPhoneNumber();

    /**
     * Returns SimSerialNumber via TelephonyManager
     */
    String getSimSerialNumber();

    /**
     * Returns SubscriberId based on Active subId via TelephonyManager
     */
    String getSubscriberId();

    /**
     * Returns deviceId via TelephonyManager.
     */
    String getDeviceId();

    /**
     * Returns gid1 value from SIM.
     */
    String getGroupIdLevel1();

    /**
     * Returns operator name from SIM.
     */
    String getSimOperatorName();
}
