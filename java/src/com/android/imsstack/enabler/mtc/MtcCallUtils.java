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

import com.android.imsstack.core.ImsGlobal;
import com.android.imsstack.enabler.IBaseContext;
import com.android.imsstack.enabler.mtc.conf.UsersInfo;
import com.android.imsstack.util.ImsLog;

import java.util.Hashtable;

public class MtcCallUtils {
    /** Indication for media control */
    public static final int INFO_TYPE_MEDIA_VIDEO_NO_DATA = 110;
    public static final int INFO_TYPE_MEDIA_VIDEO_DATA_RECEIVED = 111;
    public static final int INFO_TYPE_MEDIA_CVO_CAPABILITY = 112;

    /** Extra values for info-type */
    public static final int MEDIA_CVO_DISABLED = 0;
    public static final int MEDIA_CVO_ENABLED = 1;

    private static final Hashtable<Integer, Integer> sUserStatusCodeToSIPStatusCode;

    public static void addUser(UsersInfo usersInfo, long callId, String target) {
        if (usersInfo != null) {
            usersInfo.addUser(callId, target, "", "", "",
                    UsersInfo.USER_STATUS_IDLE, UsersInfo.CCTYPE_TO, false);
        }
    }

    public static void addUser(UsersInfo usersInfo, long callId, String target,
            String userEntity, String endpointEntity) {
        if (usersInfo != null) {
            usersInfo.addUser(callId, target, userEntity, endpointEntity, "",
                    UsersInfo.USER_STATUS_IDLE, UsersInfo.CCTYPE_TO, false);
        }
    }

    public static void copyMediaInfo(MediaInfo src, MediaInfo dest) {
        dest.AQuality = src.AQuality;
        dest.VQuality = src.VQuality;
        dest.ADir = src.ADir;
        dest.VDir = src.VDir;
        dest.TDir = src.TDir;
        dest.GTTMode = src.GTTMode;

        if ((dest.VQuality == MediaInfo.VIDEO_QUALITY_NONE)
                || (dest.VQuality == MediaInfo.VIDEO_QUALITY_NOTUSED)) {
            dest.VDir = MediaInfo.DIRECTION_INVALID;
        }
    }

    public static UsersInfo createUsersInfo(String[] participants) {
        UsersInfo usersInfo = new UsersInfo();

        for (String participant : participants) {
            // FIXME: is it possible to use "entity" field as a conference id for this user???
            usersInfo.addUser(0L, participant, "", "", "",
                    UsersInfo.USER_STATUS_IDLE, UsersInfo.CCTYPE_TO, false);
        }

        return usersInfo;
    }

    /**
     * Create a MediaInfo to hold a call.
     * As a default, video direction will be set to INACITVE.
     */
    public static MediaInfo createHoldMedia(final CallInfo ci, final MediaInfo mi,
            boolean isVideoDirectionInactiveOnVideoCallHold,
            boolean isTextDirectionInactiveOnRttCallHold) {
        MediaInfo mediaInfo = new MediaInfo(
                    mi.AQuality, mi.VQuality, mi.ADir, mi.VDir, mi.TDir, mi.GTTMode);

        mediaInfo.ADir = getHoldDirection(mi.ADir);

        if (hasVideo(MtcCallInfo.getCallType(ci))) {
            if (isVideoDirectionInactiveOnVideoCallHold) {
                mediaInfo.VDir = MediaInfo.DIRECTION_INACTIVE;
            } else {
                mediaInfo.VDir = getHoldDirection(mi.VDir);
            }
        }

        if (MtcCallInfo.isConference(ci)) {
            mediaInfo.TDir = MediaInfo.DIRECTION_INVALID;
            mediaInfo.GTTMode = MediaInfo.GTTMODE_INVALID;
        } else if (isGttEnabled(mediaInfo.GTTMode)) {
            if (isTextDirectionInactiveOnRttCallHold) {
                mediaInfo.TDir = MediaInfo.DIRECTION_INACTIVE;
            } else {
                mediaInfo.TDir = MediaInfo.DIRECTION_SEND;
            }
        }

        return mediaInfo;
    }

    /**
     * Create a MediaInfo to unhold a call.
     * As a default, video direction will be set to SENDRECV.
     */
    public static MediaInfo createUnholdMedia(final CallInfo ci, final MediaInfo mi,
            boolean isVideoDirectionInactiveOnVideoCallHold) {
        MediaInfo mediaInfo = new MediaInfo(
                    mi.AQuality, mi.VQuality, mi.ADir, mi.VDir, mi.TDir, mi.GTTMode);

        mediaInfo.ADir = getUnholdDirection(mi.ADir);

        if (hasVideo(MtcCallInfo.getCallType(ci))) {
            if (isVideoDirectionInactiveOnVideoCallHold) {
                mediaInfo.VDir = MediaInfo.DIRECTION_SEND_RECEIVE;
            } else {
                mediaInfo.VDir = getUnholdDirection(mi.VDir);
            }
        }

        if (MtcCallInfo.isConference(ci)) {
            mediaInfo.TDir = MediaInfo.DIRECTION_INVALID;
            mediaInfo.GTTMode = MediaInfo.GTTMODE_INVALID;
        } else if (isGttEnabled(mediaInfo.GTTMode)) {
            mediaInfo.TDir = MediaInfo.DIRECTION_SEND_RECEIVE;
        }

        return mediaInfo;
    }

    public static int getSIPStatusCodeFromUserStatusCode(int statusCode) {
        Integer code = sUserStatusCodeToSIPStatusCode.get(statusCode);
        return (code != null) ? code.intValue() : statusCode;
    }

    public static boolean hasVideoQuality(final MediaInfo mi) {
        return (mi.VQuality != MediaInfo.VIDEO_QUALITY_NONE)
                && (mi.VQuality != MediaInfo.VIDEO_QUALITY_NOTUSED);
    }

    public static boolean hasVideo(final int callType) {
        return callType == IUMtcCall.CALLTYPE_VT || callType == IUMtcCall.CALLTYPE_VIDEO_RTT;
    }

    public static boolean is1WayVideo(MediaInfo mi) {
        return (mi.ADir == MediaInfo.DIRECTION_SEND_RECEIVE)
                && (mi.VDir == MediaInfo.DIRECTION_SEND);
    }

    public static boolean is1WayVideoByRemoteEnd(MediaInfo mi) {
        return (mi.ADir == MediaInfo.DIRECTION_SEND_RECEIVE)
                && (mi.VDir == MediaInfo.DIRECTION_RECEIVE);
    }

    public static boolean isAudioEvsCategory(int audioQuality) {
        return (audioQuality == MediaInfo.AUDIO_QUALITY_EVS)
                || (audioQuality == MediaInfo.AUDIO_QUALITY_EVS_NB)
                || (audioQuality == MediaInfo.AUDIO_QUALITY_EVS_WB)
                || (audioQuality == MediaInfo.AUDIO_QUALITY_EVS_SWB)
                || (audioQuality == MediaInfo.AUDIO_QUALITY_EVS_FB);
    }

    public static boolean isAudioHDQuality(int audioQuality) {
        return (audioQuality == MediaInfo.AUDIO_QUALITY_AMR_WB)
                || (audioQuality == MediaInfo.AUDIO_QUALITY_EVS_WB);
    }

    public static boolean isAudioUHDQuality(int audioQuality) {
        return (audioQuality == MediaInfo.AUDIO_QUALITY_EVS)
                || (audioQuality == MediaInfo.AUDIO_QUALITY_EVS_SWB)
                || (audioQuality == MediaInfo.AUDIO_QUALITY_EVS_FB);
    }

    /**
    * Checks if the call is disconnected because of the transfer.
    *
    * @param reasonInfo checks based on this information
    */
    public static boolean isCallTerminatedByCallForward(CallReasonInfo reasonInfo) {
        return (reasonInfo.mCode == CallReasonInfo.CODE_USER_TERMINATED)
                && (reasonInfo.mExtraCode == CallReasonInfo.EXTRA_USER_TERMINATED_ECT);
    }

    public static boolean isCallTerminatedByCSRetry(int reason) {
        return (reason == CallReasonInfo.CODE_LOCAL_CALL_CS_RETRY_REQUIRED);
    }

    public static boolean isCallTerminatedByECallRetry(int reason) {
        return false;
        // TODO : need to modify this after emergency domain selection policy is decided.
        /*(reason == IUMtcCall.Fail_Reason.FAIL_REASON_SESSION_RETRY_E_1X)
                || (reason == IUMtcCall.Fail_Reason.FAIL_REASON_SESSION_RETRY_E_VOLTE)
                || (reason == IUMtcCall.Fail_Reason.FAIL_REASON_SESSION_RETRY_R_RAT)*/
    }

    public static boolean isCallTerminatedByJoiningConference(int reason) {
        return (reason == CallReasonInfo.CODE_LOCAL_ENDED_BY_CONFERENCE_MERGE);
    }

    public static boolean isCallWaitingEnabled(SuppInfo si) {
        SuppInfo.SuppService ss = (si != null) ? si.getService(SuppInfo.TYPE_CW) : null;
        return (ss != null) && ss.boolValue;
    }

    public static boolean isEmergencyServiceStateForCSRetry(IBaseContext context,
            int serviceState, int reason) {
        if (serviceState == IUMtcService.ES_UNAVAILABLE) {
            if (reason == IUMtcService.ES_UNAVAILABLE_REASON_SSAC) {
                return false;
            } else if (ImsGlobal.isOperator(context.getSlotId(), "KDDI")) {
                return false;
            } else if (reason == IUMtcService.ES_UNAVAILABLE_REASON_NO_CSFB) {
                return false;
            }
        }

        return true;
    }

    public static boolean isGttEnabled(int gttMode) {
        return (gttMode == MediaInfo.GTTMODE_FULL)
                || (gttMode == MediaInfo.GTTMODE_HCO)
                || (gttMode == MediaInfo.GTTMODE_VCO);
    }

    public static boolean isInfoTypeForMediaSession(int infoType) {
        // FIXME: common or operator-specific?
        return (infoType == INFO_TYPE_MEDIA_VIDEO_NO_DATA)
                || (infoType == INFO_TYPE_MEDIA_VIDEO_DATA_RECEIVED)
                || (infoType == INFO_TYPE_MEDIA_CVO_CAPABILITY);
    }

    /**
     * In the aspect of UAC (triggered by the local end).
     */
    public static boolean isHoldMediaOnVoiceCall(MediaInfo mi) {
        return isHoldMedia(mi.ADir);
    }

    /**
     * In the aspect of UAS (triggered by the remote end).
     */
    public static boolean isHoldMediaOnVoiceCallByRemoteEnd(MediaInfo mi) {
        return isHoldMediaByRemoteEnd(mi.ADir);
    }

    /**
     * In the aspect of UAC (triggered by the local end).
     */
    public static boolean isHoldMediaOnVideoCall(MediaInfo mi,
            boolean isVideoDirectionInactiveOnVideoCallHold) {
        return isHoldMedia(mi.ADir)
                && isHoldVideo(mi.VDir, isVideoDirectionInactiveOnVideoCallHold);
    }

    /**
     * In the aspect of UAS (triggered by the remote end).
     */
    public static boolean isHoldMediaOnVideoCallByRemoteEnd(MediaInfo mi,
            boolean isVideoDirectionInactiveOnVideoCallHold) {
        return isHoldMediaByRemoteEnd(mi.ADir)
                && isHoldVideoByRemoteEnd(mi.VDir, isVideoDirectionInactiveOnVideoCallHold);
    }

    /**
     * In the aspect of UAC (triggered by the local end).
     */
    public static boolean isUnholdMediaOnVideoCall(MediaInfo mi,
            boolean isVideoDirectionInactiveOnVideoCallHold) {
        return isUnholdMedia(mi.ADir)
                && isUnholdVideo(mi.VDir, isVideoDirectionInactiveOnVideoCallHold);
    }

    /**
     * In the aspect of UAS (triggered by the remote end).
     */
    public static boolean isUnholdMediaOnVideoCallByRemoteEnd(MediaInfo mi) {
        return isUnholdMediaByRemoteEnd(mi.ADir) && isUnholdMediaByRemoteEnd(mi.VDir);
    }

    public static boolean isLocalHoldToneEnforced(SuppInfo si) {
        for (SuppInfo.SuppService ss : si.objSuppService) {
            if (ss.type == SuppInfo.TYPE_ENFORCE_LT) {
                return ss.boolValue;
            }
        }

        return false;
    }

    /**
    * checks if the call is barred.
    *
    * @param  callReasonInfo detailed reason of the call barred
    * @return true if the call is barred
    */
    public static boolean isOutgoingCallsBarred(CallReasonInfo callReasonInfo) {
        // Q.850 : cause=53 (Outgoing calls barred within CUG)
        return (callReasonInfo.mCode == CallReasonInfo.CODE_SIP_USER_REJECTED)
                && (callReasonInfo.mExtraCode == 53);
    }

    public static boolean isSuppInfoBoolean(int type) {
        return SuppInfoUtils.isValueBoolean(type);
    }

    public static boolean isSuppInfoInt(int type) {
        return SuppInfoUtils.isValueInt(type);
    }

    public static boolean isSuppInfoString(int type) {
        return SuppInfoUtils.isValueString(type);
    }

    /**
     * Reverse the audio/video direction as local or remote capability.
     */
    public static void reverseMediaDirection(MediaInfo mi) {
        mi.ADir = reverseMediaDirection(mi.ADir);
        mi.VDir = reverseMediaDirection(mi.VDir);
    }

    /**
     * Reverse the direction as local or remote capability.
     */
    public static int reverseMediaDirection(int direction) {
        switch (direction) {
        case MediaInfo.DIRECTION_SEND:
            return MediaInfo.DIRECTION_RECEIVE;
        case MediaInfo.DIRECTION_RECEIVE:
            return MediaInfo.DIRECTION_SEND;
        default:
            return direction;
        }
    }

    /**
     * Provides a string representation of the {@code CallReasonInfo}. Primarily intended for use
     * in log statements.
     *
     * @return String representation of the {@code CallReasonInfo}.
     */
    public static String toString(final CallReasonInfo callReasonInfo) {
        StringBuilder sb = new StringBuilder();

        sb.append("[ CallReasonInfo: code=");
        sb.append(callReasonInfo.mCode);
        sb.append(", extraCode=");
        sb.append(callReasonInfo.mExtraCode);
        sb.append(", phrase=");
        sb.append(callReasonInfo.mExtraMessage);
        sb.append(" ]");

        return sb.toString();
    }

    public static String toString(final CallInfo callInfo) {
        StringBuilder sb = new StringBuilder();

        sb.append("[ CallInfo: service=");
        sb.append(MtcCallInfo.getServiceType(callInfo));
        sb.append(", call=");
        sb.append(MtcCallInfo.getCallType(callInfo));
        sb.append(", conf=");
        sb.append(MtcCallInfo.isConference(callInfo) ? "Y" : "N");
        sb.append(", conf-avail=");
        sb.append(MtcCallInfo.isConferenceAvailable(callInfo) ? "Y" : "N");
        sb.append(", conf-event=");
        sb.append(MtcCallInfo.isConferenceEventSupported(callInfo) ? "Y" : "N");
        sb.append(" ]");

        return sb.toString();
    }

    public static String toString(final MediaInfo mediaInfo) {
        StringBuilder sb = new StringBuilder();

        sb.append("[ MediaInfo: AQ=");
        sb.append(MtcMediaInfo.getAudioQuality(mediaInfo));
        sb.append(", AD=");
        sb.append(MtcMediaInfo.getAudioDirection(mediaInfo));
        sb.append(", VQ=");
        sb.append(MtcMediaInfo.getVideoQuality(mediaInfo));
        sb.append(", VD=");
        sb.append(MtcMediaInfo.getVideoDirection(mediaInfo));
        sb.append(", TD=");
        sb.append(MtcMediaInfo.getTextDirection(mediaInfo));
        sb.append(", GTT=");
        sb.append(MtcMediaInfo.getGttMode(mediaInfo));
        sb.append(" ]");

        return sb.toString();
    }

    public static String toString(final SuppInfo suppInfo) {
        int size = suppInfo.objSuppService.size();

        if (size == 0) {
            return "[ SuppInfo: size=0 ]";
        }

        StringBuilder sb = new StringBuilder();

        sb.append("[ SuppInfo(T,V): size=");
        sb.append(size);
        sb.append(", ");

        if (size > 0)
        {
            SuppInfo.SuppService ss = suppInfo.objSuppService.get(0);

            sb.append("{ ");
            sb.append(ss.type);
            sb.append(",");

            if (isSuppInfoBoolean(ss.type)) {
                sb.append(ss.boolValue);
            } else if (isSuppInfoInt(ss.type)) {
                sb.append(ss.intValue);
            } else {
                sb.append(ImsLog.hiddenString(ss.strValue));
            }

            sb.append(" }");
        }

        for (int i = 1; i < size; i++) {
            SuppInfo.SuppService ss = suppInfo.objSuppService.get(i);

            sb.append(", { ");
            sb.append(ss.type);
            sb.append(",");

            if (isSuppInfoBoolean(ss.type)) {
                sb.append(ss.boolValue);
            } else if (isSuppInfoInt(ss.type)) {
                sb.append(ss.intValue);
            } else {
                sb.append(ImsLog.hiddenString(ss.strValue));
            }

            sb.append(" }");
        }

        sb.append(" ]");

        return sb.toString();
    }

    public static String toString(final UsersInfo usersInfo) {
        int size = usersInfo.Users.size();

        if (size == 0) {
            return "[ UsersInfo: size=0 ]";
        }

        StringBuilder sb = new StringBuilder();

        sb.append("[ UsersInfo(cid,target,user,endpoint,displayN,status,sipCode): size=");
        sb.append(size);
        sb.append(", ");

        if (size > 0)
        {
            UsersInfo.User user = usersInfo.Users.get(0);

            sb.append("{ ");
            sb.append(user.callID);
            sb.append(",");
            sb.append(ImsLog.hiddenString(user.target));
            sb.append(",");
            sb.append(ImsLog.hiddenString(user.userEntity));
            sb.append(",");
            sb.append(ImsLog.hiddenString(user.epEntity));
            sb.append(",");
            sb.append(ImsLog.hiddenString(user.displayName));
            sb.append(",");
            sb.append(user.status);
            sb.append(",");
            sb.append(user.statusCode);
            sb.append(" }");
        }

        for (int i = 1; i < size; i++) {
            UsersInfo.User user = usersInfo.Users.get(i);

            sb.append(", { ");
            sb.append(user.callID);
            sb.append(",");
            sb.append(ImsLog.hiddenString(user.target));
            sb.append(",");
            sb.append(ImsLog.hiddenString(user.userEntity));
            sb.append(",");
            sb.append(ImsLog.hiddenString(user.epEntity));
            sb.append(",");
            sb.append(ImsLog.hiddenString(user.displayName));
            sb.append(",");
            sb.append(user.status);
            sb.append(",");
            sb.append(user.statusCode);
            sb.append(" }");
        }

        sb.append(" ]");

        return sb.toString();
    }

    private static int getHoldDirection(int direction) {
        if (direction == MediaInfo.DIRECTION_SEND_RECEIVE) {
            return MediaInfo.DIRECTION_SEND;
        } else if (direction == MediaInfo.DIRECTION_RECEIVE) {
            return MediaInfo.DIRECTION_INACTIVE;
        }

        return direction;
    }

    private static int getUnholdDirection(int direction) {
        if (direction == MediaInfo.DIRECTION_SEND) {
            return MediaInfo.DIRECTION_SEND_RECEIVE;
        } else if (direction == MediaInfo.DIRECTION_INACTIVE) {
            return MediaInfo.DIRECTION_RECEIVE;
        }

        return direction;
    }

    private static boolean isHoldMedia(int direction) {
        return (direction == MediaInfo.DIRECTION_INACTIVE)
                || (direction == MediaInfo.DIRECTION_SEND);
    }

    private static boolean isHoldMediaByRemoteEnd(int direction) {
        return (direction == MediaInfo.DIRECTION_INACTIVE)
                || (direction == MediaInfo.DIRECTION_RECEIVE);
    }

    private static boolean isHoldVideo(int direction,
            boolean isVideoDirectionInactiveOnVideoCallHold) {
        if (isVideoDirectionInactiveOnVideoCallHold) {
            return (direction == MediaInfo.DIRECTION_INACTIVE);
        }

        return isHoldMedia(direction);
    }

    private static boolean isHoldVideoByRemoteEnd(int direction,
            boolean isVideoDirectionInactiveOnVideoCallHold) {
        if (isVideoDirectionInactiveOnVideoCallHold) {
            return (direction == MediaInfo.DIRECTION_INACTIVE);
        }

        return isHoldMediaByRemoteEnd(direction);
    }

    private static boolean isUnholdMedia(int direction) {
        return (direction == MediaInfo.DIRECTION_SEND_RECEIVE)
                || (direction == MediaInfo.DIRECTION_RECEIVE);
    }

    private static boolean isUnholdMediaByRemoteEnd(int direction) {
        return (direction == MediaInfo.DIRECTION_SEND_RECEIVE)
                || (direction == MediaInfo.DIRECTION_SEND);
    }

    private static boolean isUnholdVideo(int direction,
            boolean isVideoDirectionInactiveOnVideoCallHold) {
        if (isVideoDirectionInactiveOnVideoCallHold) {
            return isUnholdMedia(direction) || (direction == MediaInfo.DIRECTION_INACTIVE);
        }

        return isUnholdMedia(direction);
    }

    static {
        sUserStatusCodeToSIPStatusCode = new Hashtable<Integer, Integer>();
        sUserStatusCodeToSIPStatusCode.put(UsersInfo.USER_STATUS_REJECT, 603);
        sUserStatusCodeToSIPStatusCode.put(UsersInfo.USER_STATUS_BUSY, 486);
        // 400 ?
        sUserStatusCodeToSIPStatusCode.put(UsersInfo.USER_STATUS_SERVERERROR, 503);
        // 404 ?
        sUserStatusCodeToSIPStatusCode.put(UsersInfo.USER_STATUS_NOTSUPPORTED, 415);
        sUserStatusCodeToSIPStatusCode.put(UsersInfo.USER_STATUS_NOTACCEPTABLE, 606);
        sUserStatusCodeToSIPStatusCode.put(UsersInfo.USER_STATUS_NOANSWER, 408);
        sUserStatusCodeToSIPStatusCode.put(UsersInfo.USER_STATUS_NOTREACHABLE, 499);
        sUserStatusCodeToSIPStatusCode.put(UsersInfo.USER_STATUS_LOWBATTERY, 480);
        sUserStatusCodeToSIPStatusCode.put(UsersInfo.USER_STATUS_FORBIDDEN, 403);
        sUserStatusCodeToSIPStatusCode.put(UsersInfo.USER_STATUS_INTSERVERERROR, 500);
    }
}
