package com.android.imsstack.imsservice.mmtel;

import android.content.Context;
import android.telephony.ims.ImsCallProfile;

import com.android.imsstack.core.ImsGlobal;
import com.android.imsstack.enabler.IBaseContext;
import com.android.imsstack.imsservice.mmtel.base.ICallLocationPolicy;
import com.android.imsstack.util.ImsLog;

import java.util.HashSet;
import java.util.Set;

/**
 * A class that provides any policy to determine whether geolocation is required for the call.
 */
public final class ImsCallLocationPolicy implements ICallLocationPolicy {
    /** 10 seconds (as milli-second unit) */
    private static final int LOCATION_UPDATES_WAITING_TIME = 10000;
    /* As default, 30 minutes (nanos) */
    private static final long LOCATION_VALIDITY_PERIOD = 30 * 60 * 1000 * 1000000L;

    /* Indicates that geolocation information is required to make a call */
    private static final int FLAG_LOCATION_REQUIRED = 0x00000001;
    /* Indicates that positioning information (latitude/longitude) is required */
    private static final int FLAG_POSITION_INFO_REQUIRED = 0x00000002;
    /* Indicates that geolocation information is required for emergency call only */
    private static final int FLAG_EMERGENCY_CALL_ONLY = 0x00000004;
    /* Indicates that geolocation information is required for Wi-Fi call only */
    private static final int FLAG_WIFI_CALL_ONLY = 0x00000008;
    /* Indicates that number list is specified and emergency call needs the location */
    private static final int FLAG_NUMBER_LIST_AND_EMERGENCY_CALL = 0x00000010;

    /**
     * Flags for debug logs.
     */
    private static final int LOG_E_CALL = 0x0001;
    private static final int LOG_WIFI_CALL = 0x0002;
    private static final int LOG_NUMBER_LIST = 0x0004;

    private final IBaseContext mContext;
    /**
     * A default configuration.
     */
    private int mFlags = FLAG_POSITION_INFO_REQUIRED;
    private long mValidityPeriod = LOCATION_VALIDITY_PERIOD;
    private long mWaitingTimeForLocationFix = LOCATION_UPDATES_WAITING_TIME;

    private int mDebugLog = 0;
    private Set<String> mNumberSet = new HashSet<String>();

    public ImsCallLocationPolicy(IBaseContext context) {
        mContext = context;

        initForGlobal();

        logi("LocationPolicy :: slotId=" + mContext.getSlotId()
                + ", flags=0x" + Integer.toHexString(mFlags));
    }

    public static boolean isLocationRequired(Context context, int slotId) {
        String op = ImsGlobal.getOperator(slotId);
        String co = ImsGlobal.getCountry(slotId);

        return ImsGlobal.equalsOperatorCountry("ORG", "FR", op, co)
                || ImsGlobal.equalsOperatorCountry("ORG", "SK", op, co)
                || ImsGlobal.equalsOperatorCountry("EEO", "GB", op, co)
                || ImsGlobal.equalsOperatorCountry("BT", "GB", op, co);
    }

    @Override
    public boolean isLocationRequired(String callee, ImsCallProfile profile) {
        mDebugLog = 0;

        if (isLocationRequiredInternal(callee, profile)) {
            displayConditionsForLocationRequired();
            return true;
        }

        return false;
    }

    @Override
    public boolean isPositionInfoRequired() {
        return hasFlag(FLAG_POSITION_INFO_REQUIRED);
    }

    @Override
    public long getValidityPeriod() {
        return mValidityPeriod;
    }

    @Override
    public long getWaitingTimeForLocationFix() {
        return mWaitingTimeForLocationFix;
    }

    private void initForGlobal() {
        String op = ImsGlobal.getOperator(mContext.getSlotId());
        String co = ImsGlobal.getCountry(mContext.getSlotId());

        if (ImsGlobal.equalsOperatorCountry("ORG", "FR", op, co)) {
            setFlag(FLAG_LOCATION_REQUIRED);
            setFlag(FLAG_WIFI_CALL_ONLY);

            // 24 hours (as nanos)
            mValidityPeriod = 24 * 60 * 60 * 1000 * 1000000L;
            // 5 seconds (as milli-seconds)
            mWaitingTimeForLocationFix = 5 * 1000L;

            final String[] numbers = { "112", "15", "17", "18", "115", "119",
                    "191", "196", "197", "116000", "116117" };

            for (int i = 0; i < numbers.length; ++i) {
                mNumberSet.add(numbers[i]);
            }
        } else if (ImsGlobal.equalsOperatorCountry("ORG", "SK", op, co)) {
            setFlag(FLAG_LOCATION_REQUIRED);
            setFlag(FLAG_WIFI_CALL_ONLY);

            // 24 hours
            mValidityPeriod = 24 * 60 * 60 * 1000 * 1000000L;
            // 5 seconds (as milli-seconds)
            mWaitingTimeForLocationFix = 5 * 1000L;

            final String[] numbers = { "112", "150", "155", "158", "159" };

            for (int i = 0; i < numbers.length; ++i) {
                mNumberSet.add(numbers[i]);
            }
        } else if (ImsGlobal.equalsOperatorCountry("EEO", "GB", op, co)
                  || ImsGlobal.equalsOperatorCountry("BT", "GB", op, co)) {
            setFlag(FLAG_LOCATION_REQUIRED);
            setFlag(FLAG_WIFI_CALL_ONLY);
            setFlag(FLAG_EMERGENCY_CALL_ONLY);

            // 24 hours
            mValidityPeriod = 24 * 60 * 60 * 1000 * 1000000L;
            // 5 seconds (as milli-seconds)
            mWaitingTimeForLocationFix = 5 * 1000L;
        }
    }

    private boolean isLocationRequiredForECallOnly() {
        return hasFlag(FLAG_EMERGENCY_CALL_ONLY);
    }

    private boolean isLocationRequiredForWifiCallOnly() {
        return hasFlag(FLAG_WIFI_CALL_ONLY);
    }

    private boolean isLocationRequiredForNumberListAndECall() {
        return hasFlag(FLAG_NUMBER_LIST_AND_EMERGENCY_CALL);
    }

    private boolean isLocationRequiredFromCallInfo(ImsCallProfile profile) {
        if (isLocationRequiredForECallOnly() && isLocationRequiredForWifiCallOnly()) {
            mDebugLog |= (LOG_E_CALL | LOG_WIFI_CALL);
            return isEmergencyCall(profile) && isWifiCall(mContext, profile);
        } else if (isLocationRequiredForECallOnly() && !isLocationRequiredForNumberListAndECall()) {
            mDebugLog |= LOG_E_CALL;
            return isEmergencyCall(profile);
        } else if (isLocationRequiredForWifiCallOnly()) {
            mDebugLog |= LOG_WIFI_CALL;
            return isWifiCall(mContext, profile);
        }

        return true;
    }

    private boolean isLocationRequiredInternal(String callee, ImsCallProfile profile) {
        if (!isLocationRequired(mContext.getContext(), mContext.getSlotId())) {
            return false;
        }

        if (mNumberSet.isEmpty()) {
            return isLocationRequiredFromCallInfo(profile);
        }

        if (mNumberSet.contains(callee)) {
            mDebugLog |= LOG_NUMBER_LIST;
            return isLocationRequiredFromCallInfo(profile);
        } else if (isLocationRequiredForNumberListAndECall()) {
            if (isLocationRequiredForWifiCallOnly()) {
                mDebugLog |= (LOG_E_CALL | LOG_WIFI_CALL);
                return isWifiCall(mContext, profile) && isEmergencyCall(profile);
            } else {
                mDebugLog |= LOG_E_CALL;
                return isEmergencyCall(profile);
            }
        }

        return false;
    }

    private boolean hasFlag(int flag) {
        return (mFlags & flag) == flag;
    }

    private void clearFlag(int flag) {
        mFlags &= (~flag);
    }

    private void setFlag(int flag) {
        mFlags |= flag;
    }

    private void displayConditionsForLocationRequired() {
        if (ImsLog.isDebuggable()) {
            StringBuilder sb = new StringBuilder();

            sb.append("LocationPolicy :: ");

            sb.append("e-call=");
            sb.append(((mDebugLog & LOG_E_CALL) != 0) ? "true" : "false");

            sb.append(", wifi-call=");
            sb.append(((mDebugLog & LOG_WIFI_CALL) != 0) ? "true" : "false");

            sb.append(", number-list=");
            sb.append(((mDebugLog & LOG_NUMBER_LIST) != 0) ? "true" : "false");

            logi(sb.toString());
        }
    }

    private static boolean isEmergencyCall(ImsCallProfile profile) {
        if ((profile.getServiceType() == ImsCallProfile.SERVICE_TYPE_EMERGENCY)
                || profile.getCallExtraBoolean(ImsCallProfile.EXTRA_EMERGENCY_CALL, false)) {
            return true;
        }

        return false;
    }

    private static boolean isWifiCall(IBaseContext context, ImsCallProfile profile) {
        if (profile.getServiceType() == ImsCallProfile.SERVICE_TYPE_EMERGENCY) {
            if (ImsCallUtils.isEmergencyCallViaWfc(profile)) {
                return true;
            } else {
                return false;
            }
        } else if (ImsRegUtils.isImsRegisteredOnWifi(context)) {
            return true;
        }

        return false;
    }

    private static void logi(String s) {
        ImsLog.i("[GII-IMPL] " + s);
    }
}
