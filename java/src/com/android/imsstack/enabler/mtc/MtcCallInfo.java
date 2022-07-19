/*
    Author
    <table>
    date        author                  description
    --------    --------------          ----------
    20150616    hwangoo.park@           Created
    </table>

    Description
*/

package com.android.imsstack.enabler.mtc;

/**
 * Wrapper class for CallInfo class.
 */
public final class MtcCallInfo {
    private final CallInfo mCallInfo;

    public MtcCallInfo() {
        mCallInfo = new CallInfo( IUMtcCall.SERVICETYPE_NORMAL, IUMtcCall.CALLTYPE_VOIP);
    }

    public MtcCallInfo(int serviceType, int callType, boolean isConference) {
        mCallInfo = new CallInfo(serviceType, callType, isConference);
    }

    public MtcCallInfo(CallInfo ci) {
        mCallInfo = ci;
    }

    public void copyFrom(CallInfo ci) {
        mCallInfo.serviceType = ci.serviceType;
        mCallInfo.callType = ci.callType;
        mCallInfo.emergency = ci.emergency;
        mCallInfo.offline = ci.offline;
        mCallInfo.ussi = ci.ussi;
        mCallInfo.isConf = ci.isConf;
        mCallInfo.enabledConf = ci.enabledConf;
        mCallInfo.confSub = ci.confSub;
        mCallInfo.rttCapable = ci.rttCapable;
        mCallInfo.videoCapable = ci.videoCapable;
    }

    public CallInfo getCallInfo() {
        return mCallInfo;
    }

    public int getServiceType() {
        return getServiceType(mCallInfo);
    }

    public int getCallType() {
        return getCallType(mCallInfo);
    }

    public boolean isEmergency() {
        return isEmergency(mCallInfo);
    }

    public boolean isOffline() {
        return isOffline(mCallInfo);
    }

    public boolean isUssi() {
        return isUssi(mCallInfo);
    }

    public boolean isConference() {
        return isConference(mCallInfo);
    }

    public boolean isConferenceAvailable() {
        return isConferenceAvailable(mCallInfo);
    }

    public boolean isConferenceEventSupported() {
        return isConferenceEventSupported(mCallInfo);
    }

    public boolean isRttCapable() {
        return isRttCapable(mCallInfo);
    }

    public boolean isVideoCapable() {
        return isVideoCapable(mCallInfo);
    }

    public void setServiceType(int serviceType) {
        setServiceType(mCallInfo, serviceType);
    }

    public void setCallType(int callType) {
        setCallType(mCallInfo, callType);
    }

    public void setEmergency(boolean enabled) {
        setEmergency(mCallInfo, enabled);
    }

    public void setOffline(boolean enabled) {
        setOffline(mCallInfo, enabled);
    }

    public void setUssi(boolean enabled) {
        setUssi(mCallInfo, enabled);
    }

    public void setConference(boolean enabled) {
        setConference(mCallInfo, enabled);
    }

    public void setConferenceAvailable(boolean enabled) {
        setConferenceAvailable(mCallInfo, enabled);
    }

    public void setConferenceEvent(boolean enabled) {
        setConferenceEvent(mCallInfo, enabled);
    }

    public void setRttCapable(boolean enabled) {
        setRttCapable(mCallInfo, enabled);
    }

    public void setVideoCapable(boolean enabled) {
        setVideoCapable(mCallInfo, enabled);
    }

    public static void copy(CallInfo src, CallInfo dest) {
        dest.serviceType = src.serviceType;
        dest.callType = src.callType;
        dest.emergency = src.emergency;
        dest.offline = src.offline;
        dest.ussi = src.ussi;
        dest.isConf = src.isConf;
        dest.enabledConf = src.enabledConf;
        dest.confSub = src.confSub;
        dest.rttCapable = src.rttCapable;
        dest.videoCapable = src.videoCapable;
    }

    public static int getServiceType(CallInfo ci) {
        return ci.serviceType;
    }

    public static int getCallType(CallInfo ci) {
        return ci.callType;
    }

    public static boolean isEmergency(CallInfo ci) {
        return ci.emergency;
    }

    public static boolean isOffline(CallInfo ci) {
        return ci.offline;
    }

    public static boolean isUssi(CallInfo ci) {
        return ci.ussi;
    }

    public static boolean isConference(CallInfo ci) {
        return ci.isConf;
    }

    public static boolean isConferenceAvailable(CallInfo ci) {
        return ci.enabledConf;
    }

    public static boolean isConferenceEventSupported(CallInfo ci) {
        return ci.confSub;
    }

    public static boolean isRttCapable(CallInfo ci) {
        return ci.rttCapable;
    }

    public static boolean isVideoCapable(CallInfo ci) {
        return ci.videoCapable;
    }

    public static void setServiceType(CallInfo ci, int serviceType) {
        ci.serviceType = serviceType;
    }

    public static void setCallType(CallInfo ci, int callType) {
        ci.callType = callType;
    }

    public static void setEmergency(CallInfo ci, boolean enabled) {
        ci.emergency = enabled;
    }

    public static void setOffline(CallInfo ci, boolean enabled) {
        ci.offline = enabled;
    }

    public static void setUssi(CallInfo ci, boolean enabled) {
        ci.ussi = enabled;
    }

    public static void setConference(CallInfo ci, boolean enabled) {
        ci.isConf = enabled;
    }

    public static void setConferenceAvailable(CallInfo ci, boolean enabled) {
        ci.enabledConf = enabled;
    }

    public static void setConferenceEvent(CallInfo ci, boolean enabled) {
        ci.confSub = enabled;
    }

    public static void setRttCapable(CallInfo ci, boolean enabled) {
        ci.rttCapable = enabled;
    }

    public static void setVideoCapable(CallInfo ci, boolean enabled) {
        ci.videoCapable = enabled;
    }
}
