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
        mCallInfo.emergencyType = ci.emergencyType;
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

    public int getEmergencyType() {
        return getEmergencyType(mCallInfo);
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

    /**
     * Set emergency type.
     *
     * @param emergencyType emergency type
     */
    public void setEmergencyType(int emergencyType) {
        setEmergencyType(mCallInfo, emergencyType);
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
        dest.emergencyType = src.emergencyType;
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

    /**
     * Get emergency type from given {@code CallInfo}.
     *
     * @param ci call info
     * @return emergency type
     */
    public static int getEmergencyType(CallInfo ci) {
        return ci.emergencyType;
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

    /**
     * Set emergency type to given {@code CallInfo}.
     *
     * @param ci call info
     * @param emergencyType emergency type
     */
    public static void setEmergencyType(CallInfo ci, int emergencyType) {
        ci.emergencyType = emergencyType;
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
