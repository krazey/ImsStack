package com.android.imsstack.system;

import android.os.Parcel;

import com.android.imsstack.jni.JNIIms;

/**
 * This class indicates the system configuration.
 * It provides the configuration to identify the operator, country, and features.
 */
public final class SystemConfig {
    public static final int EVENT_DEVICE_CONFIG = 0;
    public static final int EVENT_ON_BOOT = 1;
    // When system configuration & subscriber are changed
    public static final int EVENT_SUBSCRIPTION_CHANGED = 2;
    // When all the configuration need to be reset including system configuration
    public static final int EVENT_ALL_CONFIGURATION_CHANGED = 3;
    // When service features are changed
    public static final int EVENT_FEATURE_CHANGED = 4;
    // Special case: DDS device (non-multi-IMS) : When DDS is changed
    public static final int EVENT_DDS_CHANGED = 5;
    // Special case: IMS feature permissions changed (i.e. when Google Fi SIM inserted)
    // (System configuration is not required)
    public static final int EVENT_FEATURE_PERMISSIONS_CHANGED = 6;

    // Extra info.
    public static final int EXTRA_INFO_NONE = 0;
    public static final int EXTRA_INFO_SIM_MOBILITY = 0x00000001;
    public static final int EXTRA_INFO_KR_ENABLER = 0x00000002;
    /** Special case: UE capability - VoNR enabled (not EPS-FB) */
    public static final int EXTRA_INFO_NR_UE_CAPABILITY_VONR = 0x01000000;
    /** Special case: NSA mode when VoNR is enabled */
    public static final int EXTRA_INFO_NR_NSA_MODE = 0x10000000;
    /** Special case: DDS device (non-multi-IMS) */
    public static final int EXTRA_INFO_DDS = 0x20000000;
    /** Special case: To identify no SIM or SIM-REMOVED status */
    public static final int EXTRA_INFO_NO_UICC = 0x40000000;

    // Features
    //// Features for functional level
    public static final int FEATURE_IPSEC = 0x00000001;
    public static final int FEATURE_TLS = 0x00000002;
    public static final int FEATURE_AUTH_SIP_DIGEST = 0x00000004;
    public static final int FEATURE_SDP_PRECONDITION = 0x00000008;
    public static final int FEATURE_GRUU = 0x00000010;
    public static final int FEATURE_MULTIPLE_REGISTRATION = 0x00000020;
    public static final int FEATURE_REQUEST_URI_VALIDATION_IN_MID_DIALOG = 0x00000040;
    public static final int FEATURE_NO_SESSION_REFRESH_BY_REINVITE = 0x00000080;
    public static final int FEATURE_INVITE_TXN_HANDLING_CORRECTION = 0x00000100;
    public static final int FEATURE_GEOLOCATION = 0x00000200;
    public static final int FEATURE_VOLTE_IN_ROAMING = 0x00000400;
    public static final int FEATURE_VT_IN_ROAMING = 0x00000800;

    //// Features for service level
    public static final int FEATURE_S_VOLTE = 0x00000001;
    public static final int FEATURE_S_VOWIFI = 0x00000002;
    public static final int FEATURE_S_VT = 0x00000004;
    public static final int FEATURE_S_SMS = 0x00000008;
    public static final int FEATURE_S_VOLTE_EMERGENCY = 0x00000010;
    public static final int FEATURE_S_UCE = 0x00000100;

    // Slot id
    private final int mSlotId;

    // Platform configuration to identify the operator
    private final String mOperator;
    private final String mCountry;

    // Enabler configuration
    private final String mEnablerType;
    private final int mExtraInfo;

    // Runtime features
    private final int mFeatures;
    private final int mServiceFeatures;

    public SystemConfig(int slotId, String operator, String country,
            String enablerType, int extraInfo,
            int features, int serviceFeatures) {
        mSlotId = slotId;

        mOperator = (operator == null) ? "" : operator;
        mCountry = (country == null) ? "" : country;

        mEnablerType = (enablerType == null) ? "" : enablerType;
        mExtraInfo = extraInfo;

        mFeatures = features;
        mServiceFeatures = serviceFeatures;
    }

    @Override
    public boolean equals(Object o) {
        if (!(o instanceof SystemConfig)) {
            return false;
        }

        SystemConfig sc = (SystemConfig)o;

        return (mSlotId == sc.mSlotId)
                && mOperator.equalsIgnoreCase(sc.mOperator)
                && mCountry.equalsIgnoreCase(sc.mCountry)
                && mEnablerType.equalsIgnoreCase(sc.mEnablerType)
                && (mExtraInfo == sc.mExtraInfo)
                && (mFeatures == sc.mFeatures)
                && (mServiceFeatures == sc.mServiceFeatures);
    }

    @Override
    public int hashCode() {
        int code = 17;

        code = 31 * code + mSlotId;

        code = 31 * code + mOperator.hashCode();
        code = 31 * code + mCountry.hashCode();

        code = 31 * code + mEnablerType.hashCode();
        code = 31 * code + mExtraInfo;

        code = 31 * code + mFeatures;
        code = 31 * code + mServiceFeatures;

        return code;
    }

    public int getSlotId() {
        return mSlotId;
    }

    public String getOperator() {
        return mOperator;
    }

    public String getCountry() {
        return mCountry;
    }

    public String getEnablerType() {
        return mEnablerType;
    }

    public int getExtraInfo() {
        return mExtraInfo;
    }

    public int getFeatures() {
        return mFeatures;
    }

    public int getServiceFeatures() {
        return mServiceFeatures;
    }

    public void setConfiguration(int event) {
        Parcel p = Parcel.obtain();

        p.writeInt(1);
        p = toParcel(p);

        setConfiguration(event, p);
    }

    private Parcel toParcel(Parcel parcel) {
        parcel.writeInt(mSlotId);

        parcel.writeString(mOperator);
        parcel.writeString(mCountry);

        parcel.writeString(mEnablerType);
        parcel.writeInt(mExtraInfo);

        parcel.writeInt(mFeatures);
        parcel.writeInt(mServiceFeatures);

        return parcel;
    }

    public static void setConfigurationEvent(int event) {
        // 2nd argument: unused data
        JNIIms.setConfiguration(event, new byte[] {0});
    }

    public static void setConfiguration(int event, SystemConfig[] scs) {
        if (scs == null) {
            return;
        }

        Parcel p = Parcel.obtain();

        p.writeInt(scs.length);

        for (int i = 0; i < scs.length; ++i) {
            SystemConfig sc = scs[i];

            if (sc != null) {
                p = sc.toParcel(p);
            }
        }

        setConfiguration(event, p);
    }

    public static void setDeviceConfig(int activeModemCount, boolean imsEmergencyEnabled,
            boolean voLteEnabled, boolean vtEnabled, boolean wfcEnabled) {
        Parcel p = Parcel.obtain();

        p.writeInt(activeModemCount);
        p.writeInt(imsEmergencyEnabled ? 1 : 0);
        p.writeInt(voLteEnabled ? 1 : 0);
        p.writeInt(vtEnabled ? 1 : 0);
        p.writeInt(wfcEnabled ? 1 : 0);

        setConfiguration(EVENT_DEVICE_CONFIG, p);
    }

    private static void setConfiguration(int event, Parcel p) {
        if (p == null) {
            return;
        }

        byte[] data = p.marshall();
        p.recycle();
        p = null;

        JNIIms.setConfiguration(event, data);
    }
}
