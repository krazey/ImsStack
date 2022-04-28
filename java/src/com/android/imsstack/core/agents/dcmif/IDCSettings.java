package com.android.imsstack.core.agents.dcmif;

public interface IDCSettings extends IDC {

    /**
     * Return service is allowed in roaming state.
     */
    boolean isRoamingAllowed();

    /**
     * Return VoPS value should be checked when request to use Ims Apn
     */
    boolean isVopsRequired();

    /**
     * Return list of RAT technologies on which IMS is supported
     */
    int[] getImsSupportedRats();

    /**
     * Return preferred IP version for connection
     */
    int getPreferredIpVersion();

    /**
     * Return preferred IP version for emergency connection
     */
    int getEmergencyPreferredIpVersion();

    /**
     * Return whether the casueCode should be handled as permanent failure
     */
    boolean isPermanentFailure(EApnType apnType, int causeCode);
}
