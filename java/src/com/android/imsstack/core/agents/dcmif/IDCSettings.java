package com.android.imsstack.core.agents.dcmif;

public interface IDCSettings extends IDC {

    /**
     * Return whether VoLTE service is allowed in roaming state
     */
    boolean isRoamingAllowed();

    /**
     * Return whether VoPS value should be notified when the VoPS is not supported
     */
    boolean isVopsRequired();

    /**
     * Return whether VoPS value should be checked when request IMS PDN
     */
    boolean isVopsRequiredForPdn();

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
