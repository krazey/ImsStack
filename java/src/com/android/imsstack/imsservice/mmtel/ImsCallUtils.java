/*
    Author
    <table>
    date        author                  description
    --------    --------------          ----------
    20150310    hwangoo.park@           Created
    </table>

    Description
*/

package com.android.imsstack.imsservice.mmtel;

import android.net.Uri;
import android.os.Bundle;
import android.telephony.emergency.EmergencyNumber;
import android.telephony.emergency.EmergencyNumber.EmergencyServiceCategories;
import android.telephony.ims.ImsCallProfile;
import android.telephony.ims.ImsConferenceState;
import android.telephony.ims.ImsReasonInfo;
import android.telephony.ims.ImsStreamMediaProfile;
import android.text.TextUtils;

import com.android.imsstack.core.ImsGlobal;
import com.android.imsstack.core.config.ProviderInterface;
import com.android.imsstack.enabler.mtc.CallFeature;
import com.android.imsstack.enabler.mtc.CallInfo;
import com.android.imsstack.enabler.mtc.FailInfo;
import com.android.imsstack.enabler.mtc.IUMtcCall;
import com.android.imsstack.enabler.mtc.IncomingMtcCall;
import com.android.imsstack.enabler.mtc.MediaInfo;
import com.android.imsstack.enabler.mtc.MtcCallInfo;
import com.android.imsstack.enabler.mtc.MtcCallUtils;
import com.android.imsstack.enabler.mtc.SuppInfo;
import com.android.imsstack.enabler.mtc.conf.UsersInfo;
import com.android.imsstack.enabler.mtc.dialogs.DialogsInfo;
import com.android.imsstack.external.ims.ImsCallProfileEx;
import com.android.imsstack.external.ims.ImsConferenceStateEx;
import com.android.imsstack.external.ims.ImsDialog;
import com.android.imsstack.external.ims.ImsDialogState;
import com.android.imsstack.external.ims.ImsReasonInfoEx;
import com.android.imsstack.imsservice.mmtel.base.ICallContext;
import com.android.imsstack.util.ImsConstants;

import java.util.ArrayList;
import java.util.Hashtable;
import java.util.List;

/**
 * IMS call related utility methods.
 */
public class ImsCallUtils {
    /**
     * Flags to indicate the conversion of the reason code & extra code
     */
    public static final int FLAG_REASON_INFO_NONE = 0x00;
    public static final int FLAG_REASON_INFO_CODE = 0x01;
    public static final int FLAG_REASON_INFO_EXTRA_CODE = 0x02;
    public static final int FLAG_REASON_INFO_ALL = 0xFF;

    /**
     * Variable types.
     */
    public static final int VAR_TYPE_INT = 0;
    public static final int VAR_TYPE_STRING = 1;
    public static final int VAR_TYPE_BOOLEAN = 2;

    /**
     * Reasons for call termination.
     */
    public static final String REASON_CALL_DISCONNECTED_BY_MERGE_FAILED
            = "Call disconnected by merge-failed";
    public static final String REASON_CALL_DISCONNECTED_BY_USER
            = "Call disconnected by user";
    public static final String REASON_CALL_DISCONNECTED_FROM_CONFERENCE
            = "Call disconnected from conference";
    public static final String REASON_IMS_NOT_REGISTERED
            = "Service unavailable; IMS is not registered";

    /**
     * For call profile usage: QCOM-based interface for Telecom architecture
     */
    private static final boolean CALL_PROFILE_OEM_EXTRA_PREFERRED = true;
    /**
     * For call type translation: ImsCallProfile#getVideoStateFromImsCallProfile
     */
    private static final boolean CALL_TYPE_ABBREVIATION = true;
    /**
     * For call type conversion from the media information
     * L-OS: true
     * M-OS: false
     * JUMP: true
     * P-OS: true (AOSP based video state control - CALL_TYPE_VT_NODIR is not used)
     */
    private static final boolean CALL_TYPE_OVERRIDE_VT_FROM_MEDIA_INFO = true;

    /** "sos" URN for IMS emergency call */
    private static final String SOS_SERVICE_URN_POLICE = "urn:service:sos.police";
    private static final String SOS_SERVICE_URN_AMBULANCE = "urn:service:sos.ambulance";
    private static final String SOS_SERVICE_URN_FIRE = "urn:service:sos.fire";
    private static final String SOS_SERVICE_URN_MARINE = "urn:service:sos.marine";
    private static final String SOS_SERVICE_URN_MOUNTAIN = "urn:service:sos.mountain";
    private static final String SOS_SERVICE_URN_GENERIC = "urn:service:sos";

    private static final Hashtable<Integer, Integer> sMtcReasonToImsReason;
    private static final Hashtable<Integer, Integer> sReasonToSIPStatusCode;
    private static final Hashtable<Integer, String> sUserStatusToString;

    public static ImsCallProfile cloneCallProfile(final ImsCallProfile profile) {
        ImsCallProfile newProfile = new ImsCallProfile(profile.getServiceType(),
                profile.getCallType());
        newProfile.setCallRestrictCause(profile.getRestrictCause());
        newProfile.setCallerNumberVerificationStatus(profile.getCallerNumberVerificationStatus());
        newProfile.setEmergencyServiceCategories(profile.getEmergencyServiceCategories());
        newProfile.setEmergencyUrns(profile.getEmergencyUrns());
        newProfile.setEmergencyCallRouting(profile.getEmergencyCallRouting());
        newProfile.setEmergencyCallTesting(profile.isEmergencyCallTesting());
        newProfile.setHasKnownUserIntentEmergency(profile.hasKnownUserIntentEmergency());
        newProfile.updateCallExtras(profile);
        newProfile.getMediaProfile().copyFrom(profile.getMediaProfile());

        return newProfile;
    }

    public static ImsCallProfile createCallProfile(int serviceType, int callType,
            int audioQuality, int audioDirection, int videoQuality, int videoDirection) {
        ImsCallProfile profile = new ImsCallProfile(serviceType, callType);

        ImsCallMediaUtils.setMediaProfile(profile.getMediaProfile(),
                audioQuality, audioDirection, videoQuality, videoDirection);

        return profile;
    }

    public static ImsCallProfile createCallProfile(int serviceType, int callType,
            int audioQuality, int audioDirection, int videoQuality, int videoDirection,
            int rttMode) {
        ImsCallProfile profile = new ImsCallProfile(serviceType, callType);

        ImsCallMediaUtils.setMediaProfile(profile.getMediaProfile(),
                audioQuality, audioDirection, videoQuality, videoDirection, rttMode);

        return profile;
    }

    public static ImsCallProfile createCallProfileFromCallInfo(ICallContext context,
            final CallInfo ci, final MediaInfo mi) {

        int callType = getProfileCallTypeFromCallInfo(ci);
        int serviceType = MtcCallInfo.getServiceType(ci);

        switch (serviceType) {
            case IUMtcCall.SERVICETYPE_NORMAL:
                serviceType = ImsCallProfile.SERVICE_TYPE_NORMAL;
                break;
            case IUMtcCall.SERVICETYPE_EMERGENCY:
                serviceType = ImsCallProfile.SERVICE_TYPE_EMERGENCY;
                break;
            default:
                serviceType = ImsCallProfile.SERVICE_TYPE_NONE;
                break;
        }

        ImsStreamMediaProfile mediaProfile = ImsCallMediaUtils.getMediaProfileFromMediaInfo(mi);
        if (CALL_TYPE_OVERRIDE_VT_FROM_MEDIA_INFO
                && (callType == ImsCallProfile.CALL_TYPE_VT)) {
            callType = ImsCallMediaUtils.getVideoCallType(mediaProfile);
        }

        ImsCallProfile profile = new ImsCallProfile(serviceType, callType, new Bundle(),
                mediaProfile);

        profile.setCallExtraBoolean(ImsCallProfile.EXTRA_CONFERENCE,
                MtcCallInfo.isConference(ci));
        profile.setCallExtraBoolean(ImsCallProfile.EXTRA_CALL_MODE_CHANGEABLE,
                MtcCallInfo.isVideoCapable(ci));
        profile.setCallExtraBoolean(ImsCallProfileEx.EXTRA_CONFERENCE_EVENT,
                MtcCallInfo.isConferenceEventSupported(ci));

        boolean isAudioHD = MtcCallUtils.isAudioHDQuality(mi.AQuality);
        boolean isAudioUHD = MtcCallUtils.isAudioUHDQuality(mi.AQuality);

        if (isAudioHD || isAudioUHD) {
            profile.setCallRestrictCause(ImsCallProfile.CALL_RESTRICT_CAUSE_NONE);
        } else {
            profile.setCallRestrictCause(ImsCallProfile.CALL_RESTRICT_CAUSE_HD);
        }

        if (CallFeature.isRttSupported(context.getSlotId())) {
            profile.setCallExtraBoolean(ImsCallProfileEx.EXTRA_RTT_AVAIL, ci.rttCapable);
        }

        return profile;
    }

    public static ImsCallProfile createCallProfileFromIncomingCallInfo(
            ICallContext context, IncomingMtcCall incomingCall) {
        if (incomingCall == null) {
            return new ImsCallProfile();
        }

        ImsCallProfile profile = createCallProfileFromCallInfo(
                context, incomingCall.callInfo, incomingCall.mediaInfo);

        // Boolean extra information
        profile.setCallExtraBoolean(ImsCallProfile.EXTRA_VMS, false);
        profile.setCallExtraBoolean(ImsCallProfile.EXTRA_CONFERENCE_AVAIL, false);

        // Integer extra information
        int oir = ImsCallProfile.OIR_DEFAULT;

        if (incomingCall.OIPType == IncomingMtcCall.OIPTYPE_RESTRICTED) {
            oir = ImsCallProfile.OIR_PRESENTATION_RESTRICTED;
        } else if (incomingCall.OIPType == IncomingMtcCall.OIPTYPE_IDENTITY) {
            oir = ImsCallProfile.OIR_PRESENTATION_NOT_RESTRICTED;
        } else if (incomingCall.OIPType == IncomingMtcCall.OIPTYPE_UNKNOWN) {
            oir = ImsCallProfile.OIR_PRESENTATION_UNKNOWN;
        } else if (incomingCall.OIPType == (IncomingMtcCall.OIPTYPE_UNKNOWN + 1)) {
            // FIXME: The constant value SHOULD be defined if it's used...
            oir = ImsCallProfile.OIR_PRESENTATION_PAYPHONE;
        }

        profile.setCallExtraInt(ImsCallProfile.EXTRA_OIR, oir);
        profile.setCallExtraInt(ImsCallProfile.EXTRA_CNAP, oir);

        // String extra information
        profile.setCallExtra(ImsCallProfile.EXTRA_OI, incomingCall.callerPartyNum);
        // For child number
        if (!TextUtils.isEmpty(incomingCall.calleePartyNum)) {
            profile.setCallExtra(ImsCallProfile.EXTRA_CHILD_NUMBER, incomingCall.calleePartyNum);
        }


        profile.setCallExtra(ImsCallProfile.EXTRA_CALL_DISCONNECT_CAUSE,
                Integer.toString(getReasonFromMTC(incomingCall.rejectedReason)));

        updateCallProfileForEmergency(profile, incomingCall.callInfo);
        updateCallProfileFromCallInfo(context, profile, incomingCall.callInfo);
        updateCallProfileFromSuppInfo(context, profile, incomingCall.suppInfo);

        if (isCallOnNativeAppsAndCountryKR(context)) {
            // Do not set the conference extra info. in MT call setup.
            if (MtcCallInfo.isConference(incomingCall.callInfo)) {
                profile.setCallExtraBoolean(ImsCallProfile.EXTRA_CONFERENCE, false);
            }

            if (ImsGlobal.isOperator(context.getSlotId(), "LGU")) {
                removeCallExtra(profile, ImsCallProfile.EXTRA_CNAP);
            }
        }

        return profile;
    }

    public static Bundle createConferenceParticipant(String user, String endpoint,
            String displayText, String status, int sipStatusCode, int disconnectedCause) {
        Bundle participant = new Bundle();

        participant.putString(ImsConferenceState.STATUS, status);
        participant.putString(ImsConferenceState.USER, user);
        participant.putString(ImsConferenceState.ENDPOINT, endpoint);
        participant.putString(ImsConferenceState.DISPLAY_TEXT, displayText);
        participant.putInt(ImsConferenceState.SIP_STATUS_CODE, sipStatusCode);
        participant.putInt(ImsConferenceStateEx.DISCONNECTED_CAUSE, disconnectedCause);

        return participant;
    }

    public static ImsDialogState createDialogState(DialogsInfo di) {
        ArrayList<DialogsInfo.Dialog> dialogs = di.dialogs;
        ArrayList<ImsDialog> imsDialogs = new ArrayList<ImsDialog>();

        if (dialogs.isEmpty()) {
            return new ImsDialogState(imsDialogs);
        }

        for (int i = 0; i < dialogs.size(); ++i) {
            imsDialogs.add(createImsDialog(dialogs.get(i)));
        }

        return new ImsDialogState(imsDialogs);
    }

    public static ImsDialog createImsDialog(DialogsInfo.Dialog d) {
        int direction = d.initiator ?
                ImsDialog.DIRECTION_INITIATOR : ImsDialog.DIRECTION_RECIPIENT;

        ImsDialog.State state = new ImsDialog.State(
                getDialogStateFromDialogsInfo(d.state), d.reason, d.code);

        String number = (d.localNumber == null) ? "" : d.localNumber;
        ImsDialog.Participant local = new ImsDialog.Participant(
                d.localName, Uri.parse(number), d.isConf, null);

        number = (d.remoteNumber == null) ? "" : d.remoteNumber;
        ImsDialog.Participant remote = new ImsDialog.Participant(
                d.remoteName, Uri.parse(number), d.isConf, null);

        return new ImsDialog(d.id, direction,
                state, local, remote,
                d.enablePull, ImsCallMediaUtils.createMediaProfileFromMediaInfo(d.mediaInfo));
    }

    public static ImsReasonInfo createReasonInfo(final FailInfo fi, int flags) {
        return createReasonInfo(fi.Reason, fi.Code, fi.Phrase, flags);
    }

    public static ImsReasonInfo createReasonInfo(
            int code, int extraCode, String message, int flags) {
        return createReasonInfo(code, extraCode, message, flags, 0);
    }

    public static ImsReasonInfo createReasonInfo(
            int code, int extraCode, String message, int flags, int preferredCode) {
        // FIXME: convert reason to the proper code of ImsReasonInfo
        int convertedCode = (preferredCode <= 0) ? code : preferredCode;
        int convertedExtraCode = extraCode;

        if ((preferredCode <= 0) && ((flags & FLAG_REASON_INFO_CODE) != 0)) {
            convertedCode = getReasonFromMTC(code);
        }

        if ((flags & FLAG_REASON_INFO_EXTRA_CODE) != 0) {
            convertedExtraCode = getExtraCodeFromMtc(code, extraCode);
        }

        return new ImsReasonInfo(convertedCode, convertedExtraCode, message);
    }

    public static SuppInfo createSuppInfoFromCallProfile(
            ICallContext context, final ImsCallProfile profile) {
        SuppInfo si = new SuppInfo();

        // OIR (0 : default, 1 : presentation restricted, 2 : presentation not restricted)
        int oir = getSuppInfoTypeForOIR(profile.getCallExtraInt(ImsCallProfile.EXTRA_OIR, -1));

        if (oir != (-1)) {
            si.addService_int(SuppInfo.TYPE_CALLERID, oir);
        }

        Bundle oemExtras = profile.getCallExtras().getBundle(ImsCallProfile.EXTRA_OEM_EXTRAS);

        // CNAP (0 : default, 1 : presentation restricted, 2 : presentation not restricted)
        int cnap = getCallExtraInt(profile, oemExtras, ImsCallProfile.EXTRA_CNAP, -1);

        if (cnap != (-1) && (cnap != ImsCallProfile.OIR_PRESENTATION_RESTRICTED)) {
            String cna = getCallExtra(profile, oemExtras, ImsCallProfile.EXTRA_CNA, "");

            if (!TextUtils.isEmpty(cna)) {
                si.addService_str(SuppInfo.TYPE_CNAP, cna);
            }

            // KR operators: first 16 bytes of CNA
            String cna_ext = getCallExtra(profile, oemExtras,
                    ImsCallProfileEx.EXTRA_CNA_EXT, "");

            if (!TextUtils.isEmpty(cna_ext)) {
                si.addService_str(SuppInfo.TYPE_CNAPEX, cna_ext);
            }
        }

        // DIALSTRING (0 or not present: default (normal call), 1: SS configuration, 2: USSD)
        int dialstring = getSuppInfoTypeForDialString(
                getCallExtraInt(profile, null, ImsCallProfile.EXTRA_DIALSTRING, -1));

        if (dialstring != (-1)) {
            si.addService_int(SuppInfo.TYPE_DIALSTRING, dialstring);
        }

        // KR operators: MCID
        String mcid = getCallExtra(profile, oemExtras, ImsCallProfileEx.EXTRA_MCID, "");

        if (!TextUtils.isEmpty(mcid)) {
            si.addService_bool(SuppInfo.TYPE_MMC, true);
            si.addService_str(SuppInfo.TYPE_MCID, mcid);
        }

        // "sos" URN for IMS emergency call
        if (isEmergencyCall(profile)) {
            List<String> urns = profile.getEmergencyUrns();
            String urn = null;

            if (urns.isEmpty()) {
                urn = getSosUrnFromECallServiceCategory(profile.getEmergencyServiceCategories());
            } else {
                // The first item has priority ??
                urn = urns.get(0);
            }

            if (urn != null) {
                si.addService_str(SuppInfo.TYPE_TARGET_URI, urn);
            }
        }

        ImsSuppInfoUtils.addSuppInfoForIms(context, profile, si);

        return si;
    }

    public static String getCallExtraFromOemExtras(ImsCallProfile profile,
            String key, String defaultValue) {
        Bundle oemExtras = profile.getCallExtras().getBundle(ImsCallProfile.EXTRA_OEM_EXTRAS);
        return getCallExtra(profile, oemExtras, key, defaultValue);
    }

    public static boolean getCallExtraBooleanFromOemExtras(ImsCallProfile profile,
            String key, boolean defaultValue) {
        Bundle oemExtras = profile.getCallExtras().getBundle(ImsCallProfile.EXTRA_OEM_EXTRAS);
        return getCallExtraBoolean(profile, oemExtras, key, defaultValue);
    }

    public static int getCallTypeFromProfile(int callType, boolean isRttOn) {
        if (isVoiceCall(callType)) {
            if (!isRttOn) {
                return IUMtcCall.CALLTYPE_VOIP;
            } else {
                return IUMtcCall.CALLTYPE_RTT;
            }
        } else if (isVideoCall(callType)) {
            if (!isRttOn) {
                return IUMtcCall.CALLTYPE_VT;
            } else {
                return IUMtcCall.CALLTYPE_VIDEO_RTT;
            }
        } else {
            return IUMtcCall.CALLTYPE_VOIP;
        }
    }

    public static String getSosUrnFromECallServiceCategory(
            @EmergencyServiceCategories int category) {
        if ((category & EmergencyNumber.EMERGENCY_SERVICE_CATEGORY_POLICE) != 0) {
            return SOS_SERVICE_URN_POLICE;
        } else if ((category & EmergencyNumber.EMERGENCY_SERVICE_CATEGORY_AMBULANCE) != 0) {
            return SOS_SERVICE_URN_AMBULANCE;
        } else if ((category & EmergencyNumber.EMERGENCY_SERVICE_CATEGORY_FIRE_BRIGADE) != 0) {
            return SOS_SERVICE_URN_FIRE;
        } else if ((category & EmergencyNumber.EMERGENCY_SERVICE_CATEGORY_MARINE_GUARD) != 0) {
            return SOS_SERVICE_URN_MARINE;
        } else if ((category & EmergencyNumber.EMERGENCY_SERVICE_CATEGORY_MOUNTAIN_RESCUE) != 0) {
            return SOS_SERVICE_URN_MOUNTAIN;
        } else {
            return SOS_SERVICE_URN_GENERIC;
        }
    }

    public static String getStringFromUserStatus(int status) {
        // "progressing" / "connect-fail" will be handled by the application
        // comparing to the previous status,
        // Ims just passes "disconnected" status in both cases.
        String statusString = sUserStatusToString.get(status);
        return (statusString != null) ? statusString : "";
    }

    public static int getDisconnectedCauseFromUserStatus(int status) {
        switch (status) {
        case UsersInfo.USER_STATUS_REJECT:
            return ImsConferenceStateEx.DISCONNECTED_CAUSE_REJECT;
        case UsersInfo.USER_STATUS_BUSY:
            return ImsConferenceStateEx.DISCONNECTED_CAUSE_BUSY;
        case UsersInfo.USER_STATUS_SERVERERROR:
            return ImsConferenceStateEx.DISCONNECTED_CAUSE_SERVERERROR;
        case UsersInfo.USER_STATUS_NOTSUPPORTED:
            return ImsConferenceStateEx.DISCONNECTED_CAUSE_NOTSUPPORTED;
        case UsersInfo.USER_STATUS_NOTACCEPTABLE:
            return ImsConferenceStateEx.DISCONNECTED_CAUSE_NOTACCEPTABLE;
        case UsersInfo.USER_STATUS_NOANSWER:
            return ImsConferenceStateEx.DISCONNECTED_CAUSE_NOANSWER;
        case UsersInfo.USER_STATUS_NOTREACHABLE:
            return ImsConferenceStateEx.DISCONNECTED_CAUSE_NOTREACHABLE;
        case UsersInfo.USER_STATUS_LOWBATTERY:
            return ImsConferenceStateEx.DISCONNECTED_CAUSE_LOWBATTERY;
        case UsersInfo.USER_STATUS_FORBIDDEN:
            return ImsConferenceStateEx.DISCONNECTED_CAUSE_FORBIDDEN;
        case UsersInfo.USER_STATUS_INTSERVERERROR:
            return ImsConferenceStateEx.DISCONNECTED_CAUSE_INTSERVERERROR;
        default:
            return 0;
        }
    }

    public static int getExtraCodeFromMtc(int reason, int extraCode) {
        if (MtcCallUtils.isCallTerminatedByCSRetry(reason)) {
            if (extraCode == IUMtcCall.Fail_Reason.CODE_1XRETRY_SILENT_REDIAL) {
                return ImsReasonInfo.EXTRA_CODE_CALL_RETRY_SILENT_REDIAL;
            } else {
                // IUMtcCall.Fail_Reason.CODE_1XRETRY_PRIORITY_SET
                // IUMtcCall.Fail_Reason.CODE_1XRETRY_NORMAL
                // 150610, CODE_1XRETRY is not used by MTC enabler
                return (extraCode > 0) ? extraCode : ImsReasonInfo.CODE_UNSPECIFIED;
            }
        } else if (MtcCallUtils.isCallTerminatedByECallRetry(reason)) {
            return (extraCode >= 300) ? extraCode : getEmergencyServiceCode(extraCode);
        } else if (reason == IUMtcCall.Fail_Reason.FAIL_REASON_SESSION_RETRYVOLTE) {
            // SIP status code
            return (extraCode > 0) ? extraCode : ImsReasonInfo.CODE_UNSPECIFIED;
        } else if (extraCode > 0) {
            return extraCode;
        } else {
            Integer sipCode = sReasonToSIPStatusCode.get(reason);
            return (sipCode != null) ? sipCode.intValue() : ImsReasonInfo.CODE_UNSPECIFIED;
        }
    }

    public static int getReasonFromMTC(int reason) {
        Integer code = sMtcReasonToImsReason.get(reason);
        return (code != null) ? code.intValue() : ImsReasonInfo.CODE_UNSPECIFIED;
    }

    public static int getRejectReasonFromImsReasonInfo(int reason) {
        switch (reason) {
        case ImsReasonInfo.CODE_LOCAL_CALL_EXCEEDED:
            return IUMtcCall.Reject_Reason.REJECT_REASON_BUSY_MAXCALL;
        case ImsReasonInfo.CODE_LOCAL_CALL_BUSY:
            return IUMtcCall.Reject_Reason.REJECT_REASON_BUSY_NORMAL;
        case ImsReasonInfo.CODE_USER_NOANSWER:
            return IUMtcCall.Reject_Reason.REJECT_REASON_DECLINE_NOANSWER;
        case ImsReasonInfo.CODE_LOCAL_CALL_DECLINE: // FALL_THROUGH
        case ImsReasonInfo.CODE_USER_DECLINE:
            return IUMtcCall.Reject_Reason.REJECT_REASON_DECLINE_USER;
        case ImsReasonInfo.CODE_LOCAL_LOW_BATTERY: // FALL-THROUGH
        case ImsReasonInfo.CODE_LOW_BATTERY:
            return IUMtcCall.Reject_Reason.REJECT_REASON_DECLINE_NOBATTERY;
        case ImsReasonInfo.CODE_USER_IGNORE: // FALL-THROUGH
        default:
            return IUMtcCall.Reject_Reason.REJECT_REASON_DECLINE_NORMAL;
        }
    }

    public static int getProfileCallTypeFromCallInfo(CallInfo ci) {
        return getProfileCallTypeFromCallInfo(MtcCallInfo.isVideoCapable(ci),
                MtcCallInfo.getCallType(ci));
    }

    public static int getProfileCallTypeFromCallInfo(boolean videoCapable, int callType) {
        int profileCallType = ImsCallProfile.CALL_TYPE_VOICE;

        switch (callType) {
        case IUMtcCall.CALLTYPE_VOIP:
        case IUMtcCall.CALLTYPE_RTT:
            if (videoCapable) {
                if (CALL_TYPE_ABBREVIATION) {
                    profileCallType = ImsCallProfile.CALL_TYPE_VOICE;
                } else {
                    profileCallType = ImsCallProfile.CALL_TYPE_VOICE_N_VIDEO;
                }
            } else {
                profileCallType = ImsCallProfile.CALL_TYPE_VOICE;
            }
            break;
        case IUMtcCall.CALLTYPE_VT:
        case IUMtcCall.CALLTYPE_VIDEO_RTT:
            if (videoCapable) {
                if (CALL_TYPE_ABBREVIATION) {
                    profileCallType = ImsCallProfile.CALL_TYPE_VT;
                } else {
                    profileCallType = ImsCallProfile.CALL_TYPE_VIDEO_N_VOICE;
                }
            } else {
                profileCallType = ImsCallProfile.CALL_TYPE_VT;
            }
            break;
        default:
            break;
        }

        return profileCallType;
    }

    public static int getTerminateReasonFromImsReasonInfo(int reason) {
        switch (reason) {
        case ImsReasonInfo.CODE_LOCAL_CALL_DECLINE: // FALL_THROUGH
        case ImsReasonInfo.CODE_USER_DECLINE: // FALL_THROUGH
        case ImsReasonInfo.CODE_USER_TERMINATED:
            return IUMtcCall.Terminate_Reason.TERMINATE_REASON_USER;
        case ImsReasonInfo.CODE_LOCAL_POWER_OFF:
            return IUMtcCall.Terminate_Reason.TERMINATE_REASON_POWER_OFF;
        case ImsReasonInfo.CODE_LOCAL_LOW_BATTERY: // FALL-THROUGH
        case ImsReasonInfo.CODE_LOW_BATTERY:
            return IUMtcCall.Terminate_Reason.TERMINATE_REASON_LOW_BATTERY;
        case ImsReasonInfoEx.CODE_LOCAL_CALL_TERMINATED_BY_SRVCC:
            return IUMtcCall.Terminate_Reason.TERMINATE_REASON_VCC;
        default:
            return IUMtcCall.Terminate_Reason.TERMINATE_REASON_NORMAL;
        }
    }

    public static boolean isGoogleNativeCompliant(ICallContext context) {
        return ImsConstants.USE_GOOGLE_NATIVE_APPS;
    }

    public static boolean isCallOnNativeAppsAndCountryKR(ICallContext context) {
        return ImsConstants.USE_GOOGLE_NATIVE_APPS
                && ImsGlobal.isCountry(context.getSlotId(), "KR");
    }

    public static boolean isCallTypeChanged(int callType, int otherCallType) {
        return (isVoiceCall(callType) && isVideoCall(otherCallType))
                || (isVideoCall(callType) && isVoiceCall(otherCallType));
    }

    public static boolean isCallTypeChanged(ImsCallProfile profile, CallInfo ci) {
        return ((profile == null) || (ci == null)) ? false :
                isCallTypeChanged(profile.getCallType(), getProfileCallTypeFromCallInfo(ci));
    }

    public static boolean isCsSilentRedialRequired(ImsReasonInfo info) {
        return info.getCode() == ImsReasonInfo.CODE_LOCAL_CALL_CS_RETRY_REQUIRED
                && info.getExtraCode() == ImsReasonInfo.EXTRA_CODE_CALL_RETRY_SILENT_REDIAL;
    }

    public static boolean isEmergencyCall(ImsCallProfile profile) {
        if ((profile.getServiceType() == ImsCallProfile.SERVICE_TYPE_EMERGENCY)
                || profile.getCallExtraBoolean(ImsCallProfile.EXTRA_EMERGENCY_CALL, false)) {
            return true;
        }

        return false;
    }

    public static boolean isEmergencyCallViaWfc(ImsCallProfile profile) {
        Bundle oemExtras = profile.getCallExtras().getBundle(ImsCallProfile.EXTRA_OEM_EXTRAS);
        boolean wifiCall = getCallExtraBoolean(profile,
                oemExtras, ImsCallProfileEx.EXTRA_WIFI_CALL, false);

        return wifiCall;
    }

    public static boolean isEmergencyCallRedialed(ImsCallProfile profile) {
        return getCallExtraBooleanFromOemExtras(profile,
                ImsCallProfileEx.EXTRA_RAT_CHANGED, false);
    }

    public static boolean isHoldMediaOnVideoCall(ICallContext context,
            ImsCallProfile profile, CallInfo ci, MediaInfo mi) {
        if ((profile == null) || (ci == null)) {
            return false;
        }

        if (!isVideoCall(profile.getCallType())
                || !isVideoCall(getProfileCallTypeFromCallInfo(ci))) {
            return false;
        }

        return MtcCallUtils.isHoldMediaOnVideoCall(mi,
                CallFeature.isVideoDirectionInactiveOnVideoCallHold(context.getSlotId()));
    }

    public static boolean isUnholdMediaOnVideoCall(ICallContext context,
            ImsCallProfile profile, CallInfo ci, MediaInfo mi) {
        if ((profile == null) || (ci == null)) {
            return false;
        }

        if (!isVideoCall(profile.getCallType())
                || !isVideoCall(getProfileCallTypeFromCallInfo(ci))) {
            return false;
        }

        return MtcCallUtils.isUnholdMediaOnVideoCall(mi,
                CallFeature.isVideoDirectionInactiveOnVideoCallHold(context.getSlotId()));
    }

    public static boolean isVideoCall(int callType) {
        return (callType == ImsCallProfile.CALL_TYPE_VIDEO_N_VOICE)
                || (callType == ImsCallProfile.CALL_TYPE_VT)
                || (callType == ImsCallProfile.CALL_TYPE_VT_TX)
                || (callType == ImsCallProfile.CALL_TYPE_VT_RX)
                || (callType == ImsCallProfile.CALL_TYPE_VT_NODIR);
    }

    public static boolean isVoiceCall(int callType) {
        return (callType == ImsCallProfile.CALL_TYPE_VOICE_N_VIDEO)
                || (callType == ImsCallProfile.CALL_TYPE_VOICE);
    }

    public static boolean isVideoProfileChanged(ImsCallProfile profile,
            CallInfo ci, MediaInfo mi) {
        if ((profile == null) || (ci == null)) {
            return false;
        }

        if (!isVideoCall(profile.getCallType())
                || !isVideoCall(getProfileCallTypeFromCallInfo(ci))) {
            return false;
        }

        return ImsCallMediaUtils.isVideoProfileChanged(profile.getMediaProfile(), mi);
    }

    public static void refineFailInfoForExtraCode(ImsCallProfile profile, FailInfo failInfo) {
        if (MtcCallUtils.isCallTerminatedByCSRetry(failInfo.Reason)
                && ImsCallUtils.isVoiceCall(profile.getCallType())
                && (failInfo.Code <= IUMtcCall.Fail_Reason.CODE_1XRETRY_NONE)) {
            failInfo.Code = IUMtcCall.Fail_Reason.CODE_1XRETRY_SILENT_REDIAL;
        }
    }

    public static void refineFailInfoForReason(ICallContext context,
            ImsCallProfile profile, FailInfo failInfo) {
        if ((failInfo.Reason == IUMtcCall.Fail_Reason.FAIL_REASON_SESSION_NOTSUPPORTED)
                && (failInfo.Code == 415)
                && ImsCallUtils.isVideoCall(profile.getCallType())
                && ImsGlobal.isOperator(context.getSlotId(), "LGU")) {
            failInfo.Reason = IUMtcCall.Fail_Reason.FAIL_REASON_SESSION_RETRYVOLTE;
        }
    }

    public static int getPreferredCodeFromFailInfo(ICallContext callContext, FailInfo failInfo) {
        if (failInfo.Reason == IUMtcCall.Fail_Reason.FAIL_REASON_NETWORK_ROAMING) {
            if ((failInfo.Code == IUMtcCall.Fail_Reason.CODE_SETUPFAILED_BLOCK_1XRETRY)
                    && ImsGlobal.isOperator(callContext.getSlotId(), "ORG")) {
                return ImsReasonInfoEx.CODE_LOCAL_IMS_CALL_NOT_ALLOWED_IN_ROAMING;
            }
        }
        return 0;
    }

    public static void removeCallExtra(ImsCallProfile profile, String key) {
        if (profile == null) {
            return;
        }

        profile.getCallExtras().remove(key);
    }

    public static void setCallExtraIfPresent(ImsCallProfile inProfile,
            String key, int type, ImsCallProfile outProfile) {
        if ((inProfile == null) || (outProfile == null)) {
            return;
        }

        if (!inProfile.getCallExtras().containsKey(key)) {
            // no-op if key does not exist
            return;
        }

        if (type == VAR_TYPE_INT) {
            outProfile.setCallExtraInt(key, inProfile.getCallExtraInt(key));
        } else if (type == VAR_TYPE_BOOLEAN) {
            outProfile.setCallExtraBoolean(key, inProfile.getCallExtraBoolean(key));
        } else {
            outProfile.setCallExtra(key, inProfile.getCallExtra(key));
        }
    }

    public static void updateCallProfileForEmergency(ImsCallProfile profile,
            final CallInfo ci) {
        profile.setCallExtraBoolean(ImsCallProfile.EXTRA_EMERGENCY_CALL,
                (MtcCallInfo.getServiceType(ci) == IUMtcCall.SERVICETYPE_EMERGENCY) ?
                    true : false);
    }

    public static void updateCallProfileFromCallInfo(ICallContext context,
            ImsCallProfile profile, final CallInfo ci) {
        profile.setCallExtraBoolean(ImsCallProfile.EXTRA_CONFERENCE,
                MtcCallInfo.isConference(ci));
        profile.setCallExtraBoolean(ImsCallProfile.EXTRA_CALL_MODE_CHANGEABLE,
                MtcCallInfo.isVideoCapable(ci));
        profile.setCallExtraBoolean(ImsCallProfileEx.EXTRA_CONFERENCE_EVENT,
                MtcCallInfo.isConferenceEventSupported(ci));
        if (CallFeature.isRttSupported(context.getSlotId())) {
            profile.setCallExtraBoolean(ImsCallProfileEx.EXTRA_RTT_AVAIL,
                MtcCallInfo.isRttCapable(ci));
        }

        int callType = getProfileCallTypeFromCallInfo(ci);

        if (profile.getCallType() != callType) {
            profile.updateCallType(new ImsCallProfile(profile.getServiceType(), callType));
        }
    }

    public static void updateCallProfileOnSessionStarted(ImsCallProfile profile,
            final SuppInfo suppInfo) {
        SuppInfo.SuppService ss = suppInfo.getService(SuppInfo.TYPE_TIP);
        if (ss != null) {
            if (ss.intValue != SuppInfo.TIP_NONE) {
                profile.setCallExtraInt(ImsCallProfile.EXTRA_OIR,
                        getOIRTypeFromTIPType(ss.intValue));
                profile.setCallExtraInt(ImsCallProfile.EXTRA_CNAP,
                        getOIRTypeFromTIPType(ss.intValue));

                String tip = ss.strValue;
                String number = "";
                String name = "";
                if (!tip.isEmpty()) {
                    int splitIndex = tip.indexOf(",");
                    number = tip.substring(0, splitIndex);
                    name = tip.substring(splitIndex + 1, tip.length());
                }
                profile.setCallExtra(ImsCallProfile.EXTRA_OI, number);
                profile.setCallExtra(ImsCallProfile.EXTRA_CNA, name);
            }
        }
    }

    public static void updateCallProfileFromSuppInfo(ICallContext context,
            ImsCallProfile profile, final SuppInfo si) {
        // FIXME: need to improve to handle MMC field for U+
        boolean mmcPresent = false;

        for (SuppInfo.SuppService ss : si.objSuppService) {
            if (ss.type == SuppInfo.TYPE_MMC) {
                mmcPresent = true;
            } else if (MtcCallUtils.isSuppInfoBoolean(ss.type)) {
                String key = getCallExtraNameForBoolean(context, ss.type);

                if (key != null) {
                    profile.setCallExtraBoolean(key, ss.boolValue);
                }
            } else if (MtcCallUtils.isSuppInfoInt(ss.type)) {
                String key = getCallExtraNameForInt(context, ss.type);

                if (key != null) {
                    profile.setCallExtraInt(key,
                            getCallExtraValueFromSuppInfo(ss.type, ss.intValue));
                }
            } else if (MtcCallUtils.isSuppInfoString(ss.type)) {
                String key = getCallExtraNameForString(context, ss.type);

                if (key != null) {
                    profile.setCallExtra(key, ss.strValue);
                }
            }
        }

        if (mmcPresent && profile.getCallExtra(ImsCallProfileEx.EXTRA_MCID, null) == null) {
            // LGU+ case: "19;CNAP" / "30;CNAP2"
            if (ImsGlobal.isOperator(context.getSlotId(), "LGU")) {
                profile.setCallExtra(ImsCallProfileEx.EXTRA_MCID, "30;CNAP2");
            }
        }

        // STIR/SHAKEN
        int verStat = getVerificationStatusFromOIVerStatus(
                profile.getCallExtraInt(ImsCallProfileEx.EXTRA_OI_VER_STATUS, -1));

        if (verStat != -1) {
            profile.setCallerNumberVerificationStatus(verStat);
        }
    }

    public static void updateCallProfileFromSuppInfoExtension(ICallContext context,
            ImsCallProfile profile, final SuppInfo suppInfo) {
        ImsSuppInfoUtils.addCallExtraForApp(context, suppInfo, profile);
    }

    public static void updateCallProfileOnSessionProgressing(ICallContext context,
            ImsCallProfile profile, final CallInfo callInfo, final SuppInfo suppInfo) {
        int callType = getProfileCallTypeFromCallInfo(callInfo);
        profile.updateCallType(new ImsCallProfile(profile.getServiceType(), callType));

        // If MTC enabler sends necessary supp. services on session progressing,
        // then use updateCallProfileFromSuppInfo(...).
        for (SuppInfo.SuppService ss : suppInfo.objSuppService) {
            if (ss.type == SuppInfo.TYPE_VRBT) {
                profile.setCallExtraBoolean(ImsCallProfileEx.EXTRA_VRBT, ss.boolValue);
            }
        }
    }

    public static boolean isEmergencyPdnUsedForEmergencyCallViaWfc(ICallContext context) {
        int callViaEPdn = com.android.imsstack.util.DBUtils.CP.getInt(
                context.getSlotId(),
                context.getContext().getContentResolver(),
                ProviderInterface.UCEmergency.CONTENT_URI,
                ProviderInterface.UCEmergency.USINGEPDN_WIFI, 1);

        return (callViaEPdn == 1);
    }

    private static String getCallExtra(ImsCallProfile profile, Bundle oemExtras,
            String key, String defaultValue) {
        if (CALL_PROFILE_OEM_EXTRA_PREFERRED && (oemExtras != null)) {
            return oemExtras.getString(key, defaultValue);
        }

        return profile.getCallExtra(key, defaultValue);
    }

    private static boolean getCallExtraBoolean(ImsCallProfile profile, Bundle oemExtras,
            String key, boolean defaultValue) {
        if (CALL_PROFILE_OEM_EXTRA_PREFERRED && (oemExtras != null)) {
            return oemExtras.getBoolean(key, defaultValue);
        }

        return profile.getCallExtraBoolean(key, defaultValue);
    }

    private static int getCallExtraInt(ImsCallProfile profile, Bundle oemExtras,
            String key, int defaultValue) {
        if (CALL_PROFILE_OEM_EXTRA_PREFERRED && (oemExtras != null)) {
            return oemExtras.getInt(key, defaultValue);
        }

        return profile.getCallExtraInt(key, defaultValue);
    }

    private static String getCallExtraNameForBoolean(ICallContext context, int suppInfo) {
        switch (suppInfo) {
        case SuppInfo.TYPE_MMC:
            // no-op
            break;
        case SuppInfo.TYPE_GTT:
            return ImsCallProfileEx.EXTRA_GTT;
        default:
            // no-op
            break;
        }

        return ImsSuppInfoUtils.getCallExtraNameForBoolean(context, suppInfo);
    }

    private static String getCallExtraNameForInt(ICallContext context, int suppInfo) {
        switch (suppInfo) {
        case SuppInfo.TYPE_CDIV_CAUSE:
            return ImsCallProfileEx.EXTRA_CDIV_CAUSE;
        case SuppInfo.TYPE_CALLING_NUM_VERIFICATION:
            return ImsCallProfileEx.EXTRA_OI_VER_STATUS;
        default:
            // no-op
            break;
        }

        return ImsSuppInfoUtils.getCallExtraNameForInt(context, suppInfo);
    }

    private static String getCallExtraNameForString(ICallContext context, int suppInfo) {
        switch (suppInfo) {
        case SuppInfo.TYPE_CNAP:
            return ImsCallProfile.EXTRA_CNA;
        case SuppInfo.TYPE_CNAPEX:
            return ImsCallProfileEx.EXTRA_CNA_EXT;
        case SuppInfo.TYPE_CDIV_HISTORY:
            return ImsCallProfileEx.EXTRA_CDIV_HISTORY;
        case SuppInfo.TYPE_MCID:
            return ImsCallProfileEx.EXTRA_MCID;
        case SuppInfo.TYPE_USSD:
            return ImsCallProfile.EXTRA_USSD;
        default:
            // no-op
            break;
        }

        return ImsSuppInfoUtils.getCallExtraNameForString(context, suppInfo);
    }

    private static int getCallExtraValueFromSuppInfo(int type, int value) {
        switch (type) {
        case SuppInfo.TYPE_CALLING_NUM_VERIFICATION:
            return getOIVerStatusFromCallingNumVerificationType(value);
        default:
            return value;
        }
    }

    private static int getDialogStateFromDialogsInfo(int state) {
        switch (state) {
        case DialogsInfo.DIALOG_STATE_TRYING:
            return ImsDialog.State.STATE_TRYING;
        case DialogsInfo.DIALOG_STATE_PROCEEDING:
            return ImsDialog.State.STATE_PROCEEDING;
        case DialogsInfo.DIALOG_STATE_EARLY:
            return ImsDialog.State.STATE_EARLY;
        case DialogsInfo.DIALOG_STATE_CONFIRMED:
            return ImsDialog.State.STATE_CONFIRMED;
        case DialogsInfo.DIALOG_STATE_TERMINATED:
            return ImsDialog.State.STATE_TERMINATED;
        case DialogsInfo.DIALOG_STATE_ONHOLD:
            return ImsDialog.State.STATE_ON_HOLD;
        case DialogsInfo.DIALOG_STATE_IDLE: // FALL-THROUGH
        default:
            return ImsDialog.State.STATE_IDLE;
        }
    }

    private static int getEmergencyServiceCode(int code) {
        switch (code) {
        case IUMtcCall.Fail_Reason.CODE_EMERGENCYSERVICE_GENERIC:
            return ImsReasonInfoEx.EXTRA_CODE_ECALL_RETRY_GENERIC;
        case IUMtcCall.Fail_Reason.CODE_EMERGENCYSERVICE_POLICE:
            return ImsReasonInfoEx.EXTRA_CODE_ECALL_RETRY_POLICE;
        case IUMtcCall.Fail_Reason.CODE_EMERGENCYSERVICE_AMBULANCE:
            return ImsReasonInfoEx.EXTRA_CODE_ECALL_RETRY_AMBULANCE;
        case IUMtcCall.Fail_Reason.CODE_EMERGENCYSERVICE_FIRE:
            return ImsReasonInfoEx.EXTRA_CODE_ECALL_RETRY_FIRE_BRIGADE;
        case IUMtcCall.Fail_Reason.CODE_EMERGENCYSERVICE_MARINE:
            return ImsReasonInfoEx.EXTRA_CODE_ECALL_RETRY_MARINE_GUARD;
        case IUMtcCall.Fail_Reason.CODE_EMERGENCYSERVICE_MOUNTAIN:
            return ImsReasonInfoEx.EXTRA_CODE_ECALL_RETRY_MOUNTAIN_RESCUE;
        default:
            return ImsReasonInfoEx.EXTRA_CODE_CS_CALL_RETRY;
        }
    }

    private static int getOIRTypeFromTIPType(int tip) {
        switch (tip) {
        case SuppInfo.TIP_RESTRICTED:
            return ImsCallProfile.OIR_PRESENTATION_RESTRICTED;
        case SuppInfo.TIP_IDENTITY:
            return ImsCallProfile.OIR_PRESENTATION_NOT_RESTRICTED;
        default:
            return ImsCallProfile.OIR_DEFAULT;
        }
    }

    private static int getOIVerStatusFromCallingNumVerificationType(int cnv) {
        switch (cnv) {
        case SuppInfo.CALLING_NUM_VERSTAT_VERIFIED:
            return ImsCallProfileEx.OI_VER_STATUS_TN_VALIDATION_PASSED;
        case SuppInfo.CALLING_NUM_VERSTAT_NOT_VERIFIED:
            return ImsCallProfileEx.OI_VER_STATUS_TN_VALIDATION_FAILED;
        case SuppInfo.CALLING_NUM_VERSTAT_NONE:
            return ImsCallProfileEx.OI_VER_STATUS_NO_TN_VALIDATION;
        default:
            return (-1);
        }
    }

    private static int getSuppInfoTypeForOIR(int oir) {
        switch (oir) {
        case ImsCallProfile.OIR_DEFAULT:
            return SuppInfo.CALLERID_NETWORK;
        case ImsCallProfile.OIR_PRESENTATION_RESTRICTED:
            return SuppInfo.CALLERID_RESTRICTED;
        case ImsCallProfile.OIR_PRESENTATION_NOT_RESTRICTED:
            return SuppInfo.CALLERID_IDENTITY;
        default:
            return (-1);
        }
    }

    private static int getSuppInfoTypeForDialString(int dialstring) {
        switch (dialstring) {
        case ImsCallProfile.DIALSTRING_NORMAL:
            return 0;
        case ImsCallProfile.DIALSTRING_SS_CONF:
            return 1;
        case ImsCallProfile.DIALSTRING_USSD:
            return 2;
        default:
            return (-1);
        }
    }

    private static int getVerificationStatusFromOIVerStatus(int status) {
        switch (status) {
        case ImsCallProfileEx.OI_VER_STATUS_TN_VALIDATION_PASSED:
            return ImsCallProfile.VERIFICATION_STATUS_PASSED;
        case ImsCallProfileEx.OI_VER_STATUS_TN_VALIDATION_FAILED:
            return ImsCallProfile.VERIFICATION_STATUS_FAILED;
        case ImsCallProfileEx.OI_VER_STATUS_NO_TN_VALIDATION:
            return ImsCallProfile.VERIFICATION_STATUS_NOT_VERIFIED;
        default:
            return (-1);
        }
    }



    static {
        // Reason codes: from MtcCall to ImsCall
        sMtcReasonToImsReason = new Hashtable<Integer, Integer>();
        sMtcReasonToImsReason.put(IUMtcCall.Fail_Reason.FAIL_REASON_SERVICE_UNAVAILABLE,
                ImsReasonInfo.CODE_LOCAL_NOT_REGISTERED);
        sMtcReasonToImsReason.put(IUMtcCall.Fail_Reason.FAIL_REASON_SERVICE_NOTREGISTERED,
                ImsReasonInfo.CODE_LOCAL_NOT_REGISTERED);

        // In this moment, it is not used...
        sMtcReasonToImsReason.put(IUMtcCall.Fail_Reason.FAIL_REASON_SERVICE_OUT,
                ImsReasonInfo.CODE_LOCAL_SERVICE_UNAVAILABLE);
        sMtcReasonToImsReason.put(IUMtcCall.Fail_Reason.FAIL_REASON_SERVICE_NOTSUPPORTCALL,
                ImsReasonInfo.CODE_UNSPECIFIED);
        sMtcReasonToImsReason.put(IUMtcCall.Fail_Reason.FAIL_REASON_SERVICE_INCSCALL,
                ImsReasonInfo.CODE_LOCAL_CALL_BUSY);
        sMtcReasonToImsReason.put(IUMtcCall.Fail_Reason.FAIL_REASON_SERVICE_INVTCALL,
                ImsReasonInfo.CODE_LOCAL_CALL_BUSY);
        sMtcReasonToImsReason.put(IUMtcCall.Fail_Reason.FAIL_REASON_SERVICE_INOTHERSCALL,
                ImsReasonInfo.CODE_LOCAL_CALL_BUSY);

        sMtcReasonToImsReason.put(IUMtcCall.Fail_Reason.FAIL_REASON_SERVICE_MAXCALL,
                ImsReasonInfo.CODE_LOCAL_CALL_EXCEEDED);
        sMtcReasonToImsReason.put(IUMtcCall.Fail_Reason.FAIL_REASON_SERVICE_POWEROFF,
                ImsReasonInfo.CODE_LOCAL_POWER_OFF);
        sMtcReasonToImsReason.put(IUMtcCall.Fail_Reason.FAIL_REASON_SERVICE_LOWBATTERY,
                ImsReasonInfo.CODE_LOCAL_LOW_BATTERY);

        sMtcReasonToImsReason.put(IUMtcCall.Fail_Reason.FAIL_REASON_NETWORK_OUTOFCOVERAGE,
                ImsReasonInfo.CODE_LOCAL_NETWORK_NO_SERVICE);
        sMtcReasonToImsReason.put(IUMtcCall.Fail_Reason.FAIL_REASON_NETWORK_NO_LTE_COVERAGE,
                ImsReasonInfo.CODE_LOCAL_NETWORK_NO_LTE_COVERAGE);
        sMtcReasonToImsReason.put(IUMtcCall.Fail_Reason.FAIL_REASON_NETWORK_NO_WIFI_COVERAGE,
                ImsReasonInfoEx.CODE_LOCAL_NETWORK_NO_WIFI_COVERAGE);
        sMtcReasonToImsReason.put(IUMtcCall.Fail_Reason.FAIL_REASON_NETWORK_NO_VOLTE,
                ImsReasonInfo.CODE_LOCAL_NETWORK_NO_LTE_COVERAGE);
        sMtcReasonToImsReason.put(IUMtcCall.Fail_Reason.FAIL_REASON_NETWORK_ROAMING,
                ImsReasonInfo.CODE_LOCAL_NETWORK_ROAMING);
        sMtcReasonToImsReason.put(IUMtcCall.Fail_Reason.FAIL_REASON_NETWORK_IPCHANGE,
                ImsReasonInfo.CODE_LOCAL_NETWORK_IP_CHANGED);
        sMtcReasonToImsReason.put(IUMtcCall.Fail_Reason.FAIL_REASON_NETWORK_BARRING,
                ImsReasonInfoEx.CODE_LOCAL_NETWORK_BARRED);
        sMtcReasonToImsReason.put(IUMtcCall.Fail_Reason.FAIL_REASON_NETWORK_SSAC,
                ImsReasonInfoEx.CODE_LOCAL_NETWORK_BARRED);

        sMtcReasonToImsReason.put(IUMtcCall.Fail_Reason.FAIL_REASON_SESSION_SERVERERROR,
                ImsReasonInfo.CODE_SIP_SERVER_ERROR);
        sMtcReasonToImsReason.put(IUMtcCall.Fail_Reason.FAIL_REASON_SESSION_FORBIDDEN,
                ImsReasonInfo.CODE_SIP_FORBIDDEN);
        sMtcReasonToImsReason.put(IUMtcCall.Fail_Reason.FAIL_REASON_SESSION_NOTFOUND,
                ImsReasonInfo.CODE_SIP_NOT_FOUND);
        sMtcReasonToImsReason.put(IUMtcCall.Fail_Reason.FAIL_REASON_SESSION_NOTSUPPORTED,
                ImsReasonInfo.CODE_SIP_NOT_SUPPORTED);
        sMtcReasonToImsReason.put(IUMtcCall.Fail_Reason.FAIL_REASON_SESSION_TEMPUNAVAILABLE,
                ImsReasonInfo.CODE_SIP_TEMPRARILY_UNAVAILABLE);
        sMtcReasonToImsReason.put(IUMtcCall.Fail_Reason.FAIL_REASON_SESSION_BADADDRESS,
                ImsReasonInfo.CODE_SIP_BAD_ADDRESS);
        sMtcReasonToImsReason.put(IUMtcCall.Fail_Reason.FAIL_REASON_SESSION_BUSY,
                ImsReasonInfo.CODE_SIP_BUSY);
        sMtcReasonToImsReason.put(IUMtcCall.Fail_Reason.FAIL_REASON_SESSION_IN_SETUP,
                ImsReasonInfoEx.CODE_LOCAL_CALL_BUSY_BY_MT_CALL);
        sMtcReasonToImsReason.put(IUMtcCall.Fail_Reason.FAIL_REASON_SESSION_REJECTED,
                ImsReasonInfo.CODE_SIP_USER_REJECTED);
        sMtcReasonToImsReason.put(IUMtcCall.Fail_Reason.FAIL_REASON_SESSION_CANCELED,
                ImsReasonInfo.CODE_SIP_REQUEST_CANCELLED);
        sMtcReasonToImsReason.put(IUMtcCall.Fail_Reason.FAIL_REASON_SESSION_USERTERMINATE,
                ImsReasonInfo.CODE_USER_TERMINATED);
        sMtcReasonToImsReason.put(IUMtcCall.Fail_Reason.FAIL_REASON_SESSION_TERMINATED,
                ImsReasonInfo.CODE_USER_TERMINATED_BY_REMOTE);
        // 313 : IUMtcCall.Fail_Reason.FAIL_REASON_SESSION_UPDATE_FAILED
        sMtcReasonToImsReason.put(IUMtcCall.Fail_Reason.FAIL_REASON_SESSION_BADRESPONSE,
                ImsReasonInfo.CODE_SIP_BAD_REQUEST);
        sMtcReasonToImsReason.put(IUMtcCall.Fail_Reason.FAIL_REASON_SESSION_NOTACCEPTABLE,
                ImsReasonInfo.CODE_SIP_NOT_ACCEPTABLE);
        sMtcReasonToImsReason.put(IUMtcCall.Fail_Reason.FAIL_REASON_SESSION_NOTACCEPTABLEHERE,
                ImsReasonInfo.CODE_SIP_NOT_ACCEPTABLE);
        sMtcReasonToImsReason.put(IUMtcCall.Fail_Reason.FAIL_REASON_SESSION_TIMEOUT,
                ImsReasonInfo.CODE_SIP_REQUEST_TIMEOUT);
        sMtcReasonToImsReason.put(IUMtcCall.Fail_Reason.FAIL_REASON_SESSION_REFRESH_OUT,
                ImsReasonInfo.CODE_SIP_REQUEST_TIMEOUT);
        sMtcReasonToImsReason.put(IUMtcCall.Fail_Reason.FAIL_REASON_SESSION_NOTREACHABLE,
                ImsReasonInfo.CODE_SIP_NOT_REACHABLE);
        sMtcReasonToImsReason.put(IUMtcCall.Fail_Reason.FAIL_REASON_SESSION_REDIRECTION,
                ImsReasonInfo.CODE_SIP_REDIRECTED);
        // 320 : IUMtcCall.Fail_Reason.FAIL_REASON_SESSION_EARLYDIALOG

        sMtcReasonToImsReason.put(IUMtcCall.Fail_Reason.FAIL_REASON_SESSION_SETUPFAILED,
                ImsReasonInfo.CODE_LOCAL_INTERNAL_ERROR);

        sMtcReasonToImsReason.put(IUMtcCall.Fail_Reason.FAIL_REASON_SESSION_RES_TIMEOUT,
                ImsReasonInfo.CODE_LOCAL_INTERNAL_ERROR);

        sMtcReasonToImsReason.put(IUMtcCall.Fail_Reason.FAIL_REASON_SESSION_SERVER_AUTH,
                ImsReasonInfo.CODE_SIP_CLIENT_ERROR);
        sMtcReasonToImsReason.put(
                IUMtcCall.Fail_Reason.FAIL_REASON_SESSION_SERVER_REQUEST_TERMINATED,
                ImsReasonInfo.CODE_SIP_REQUEST_CANCELLED);
        sMtcReasonToImsReason.put(
                IUMtcCall.Fail_Reason.FAIL_REASON_SESSION_SERVER_INTERNAL_ERROR,
                ImsReasonInfo.CODE_SIP_SERVER_ERROR);
        sMtcReasonToImsReason.put(
                IUMtcCall.Fail_Reason.FAIL_REASON_SESSION_SERVER_SERVICE_UNAVAILABLE,
                ImsReasonInfo.CODE_SIP_SERVICE_UNAVAILABLE);
        sMtcReasonToImsReason.put(
                IUMtcCall.Fail_Reason.FAIL_REASON_SESSION_SERVER_DECLINE,
                ImsReasonInfo.CODE_SIP_USER_REJECTED);

        sMtcReasonToImsReason.put(IUMtcCall.Fail_Reason.FAIL_REASON_SESSION_RETRY,
                ImsReasonInfo.CODE_LOCAL_CALL_CS_RETRY_REQUIRED);
        sMtcReasonToImsReason.put(IUMtcCall.Fail_Reason.FAIL_REASON_SESSION_RETRY1X,
                ImsReasonInfo.CODE_LOCAL_CALL_CS_RETRY_REQUIRED);
        sMtcReasonToImsReason.put(IUMtcCall.Fail_Reason.FAIL_REASON_SESSION_RETRYVOLTE,
                ImsReasonInfo.CODE_LOCAL_CALL_VOLTE_RETRY_REQUIRED);
        sMtcReasonToImsReason.put(IUMtcCall.Fail_Reason.FAIL_REASON_SESSION_RETRY_RAT,
                ImsReasonInfoEx.CODE_LOCAL_ECALL_RETRY_REQUIRED);
        sMtcReasonToImsReason.put(IUMtcCall.Fail_Reason.FAIL_REASON_SESSION_RETRY_E_1X,
                ImsReasonInfoEx.CODE_LOCAL_IMS_CALL_TO_CS_ECALL_RETRY_REQUIRED);
        sMtcReasonToImsReason.put(IUMtcCall.Fail_Reason.FAIL_REASON_SESSION_RETRY_E_VOLTE,
                ImsReasonInfoEx.CODE_LOCAL_IMS_CALL_TO_ECALL_RETRY_REQUIRED);
        sMtcReasonToImsReason.put(IUMtcCall.Fail_Reason.FAIL_REASON_SESSION_RETRY_R_RAT,
                ImsReasonInfoEx.CODE_LOCAL_IMS_CALL_RETRY_BY_RAT_SELECTION);
        // IUMtcCall.Fail_Reason.FAIL_REASON_SESSION_RETRY_E_RAT
        sMtcReasonToImsReason.put(IUMtcCall.Fail_Reason.FAIL_REASON_SESSION_RETRY1X_FORCE,
                ImsReasonInfoEx.CODE_LOCAL_CALL_CS_RETRY_REQUIRED_FORCE);

        sMtcReasonToImsReason.put(IUMtcCall.Fail_Reason.FAIL_REASON_SESSION_MULTIDEVICE_ACCEPTED,
                ImsReasonInfoEx.CODE_SERVICE_MULTI_DEVICE_ACCEPTED);
        sMtcReasonToImsReason.put(IUMtcCall.Fail_Reason.FAIL_REASON_SESSION_MULTIDEVICE_REJECTED,
                ImsReasonInfoEx.CODE_SERVICE_MULTI_DEVICE_REJECTED);
        sMtcReasonToImsReason.put(IUMtcCall.Fail_Reason.FAIL_REASON_SESSION_MULTIDEVICE_LIMITED,
                ImsReasonInfoEx.CODE_SERVICE_MULTI_DEVICE_LIMITED);
        sMtcReasonToImsReason.put(IUMtcCall.Fail_Reason.FAIL_REASON_SESSION_MULTIDEVICE_PULLED,
                ImsReasonInfoEx.CODE_SERVICE_MULTI_DEVICE_PULLED);

        // ImsReasonInfo.CODE_LOCAL_CALL_VCC_ON_PROGRESSING;
        sMtcReasonToImsReason.put(IUMtcCall.Fail_Reason.FAIL_REASON_SESSION_SRVCC,
                ImsReasonInfoEx.CODE_LOCAL_CALL_TERMINATED_BY_SRVCC);
        sMtcReasonToImsReason.put(IUMtcCall.Fail_Reason.FAIL_REASON_SESSION_PRECONDITION,
                ImsReasonInfo.CODE_LOCAL_CALL_RESOURCE_RESERVATION_FAILED);

        sMtcReasonToImsReason.put(IUMtcCall.Fail_Reason.FAIL_REASON_MEDIA_INITFAIL,
                ImsReasonInfo.CODE_MEDIA_INIT_FAILED);
        sMtcReasonToImsReason.put(IUMtcCall.Fail_Reason.FAIL_REASON_MEDIA_NEGOFAIL,
                ImsReasonInfo.CODE_MEDIA_NOT_ACCEPTABLE);
        sMtcReasonToImsReason.put(IUMtcCall.Fail_Reason.FAIL_REASON_MEDIA_NOMATCH,
                ImsReasonInfo.CODE_MEDIA_NOT_ACCEPTABLE);
        sMtcReasonToImsReason.put(IUMtcCall.Fail_Reason.FAIL_REASON_MEDIA_NODATA,
                ImsReasonInfo.CODE_MEDIA_NO_DATA);
        sMtcReasonToImsReason.put(IUMtcCall.Fail_Reason.FAIL_REASON_MEDIA_UNKNOWN,
                ImsReasonInfo.CODE_MEDIA_UNSPECIFIED);

        sMtcReasonToImsReason.put(IUMtcCall.Fail_Reason.FAIL_REASON_CONF_JOINED,
                ImsReasonInfoEx.CODE_LOCAL_CALL_TERMINATED_BY_CONFERENCE_JOINED);
        sMtcReasonToImsReason.put(IUMtcCall.Fail_Reason.FAIL_REASON_CONF_NOT_ACCEPTABLE,
                ImsReasonInfo.CODE_SIP_NOT_ACCEPTABLE);
        sMtcReasonToImsReason.put(IUMtcCall.Fail_Reason.FAIL_REASON_CONF_BUSY,
                ImsReasonInfo.CODE_SIP_BUSY);
        sMtcReasonToImsReason.put(IUMtcCall.Fail_Reason.FAIL_REASON_CONF_NOT_ACCEPTABLE_HERE,
                ImsReasonInfo.CODE_SIP_NOT_ACCEPTABLE);
        sMtcReasonToImsReason.put(IUMtcCall.Fail_Reason.FAIL_REASON_CONF_TIMEOUT,
                ImsReasonInfo.CODE_SIP_REQUEST_TIMEOUT);

        // ImsReasonInfo.CODE_LOCAL_INTERNAL_ERROR;
        sMtcReasonToImsReason.put(IUMtcCall.Fail_Reason.FAIL_REASON_CONF_INTERNAL_ERROR,
                ImsReasonInfo.CODE_UNSPECIFIED);
        sMtcReasonToImsReason.put(IUMtcCall.Fail_Reason.FAIL_REASON_CONF_ALONE,
                ImsReasonInfo.CODE_UNSPECIFIED);
        sMtcReasonToImsReason.put(IUMtcCall.Fail_Reason.FAIL_REASON_CONF_UNKNOWN,
                ImsReasonInfo.CODE_UNSPECIFIED);

        sMtcReasonToImsReason.put(IUMtcCall.Fail_Reason.FAIL_REASON_TO_MO_PROGRESSING,
                ImsReasonInfo.CODE_TIMEOUT_1XX_WAITING);
        sMtcReasonToImsReason.put(IUMtcCall.Fail_Reason.FAIL_REASON_TO_MO_STARTED,
                ImsReasonInfo.CODE_TIMEOUT_NO_ANSWER);
        sMtcReasonToImsReason.put(IUMtcCall.Fail_Reason.FAIL_REASON_TO_MO_UPDATE,
                ImsReasonInfo.CODE_TIMEOUT_NO_ANSWER_CALL_UPDATE);
        sMtcReasonToImsReason.put(IUMtcCall.Fail_Reason.FAIL_REASON_TO_MT_NOANSWER,
                ImsReasonInfo.CODE_TIMEOUT_NO_ANSWER);
        sMtcReasonToImsReason.put(IUMtcCall.Fail_Reason.FAIL_REASON_TO_MT_UPDATE,
                ImsReasonInfo.CODE_TIMEOUT_NO_ANSWER_CALL_UPDATE);
        // 526: IUMtcCall.Fail_Reason.FAIL_REASON_TO_MT_PRACK

        sMtcReasonToImsReason.put(IUMtcCall.Fail_Reason.FAIL_REASON_FORBIDDEN_BARRING,
                ImsReasonInfo.CODE_SIP_FORBIDDEN);
        sMtcReasonToImsReason.put(IUMtcCall.Fail_Reason.FAIL_REASON_FORBIDDEN_NOPROFILE,
                ImsReasonInfo.CODE_SIP_FORBIDDEN);
        sMtcReasonToImsReason.put(IUMtcCall.Fail_Reason.FAIL_REASON_FORBIDDEN_EXPIRATION,
                ImsReasonInfo.CODE_SIP_FORBIDDEN);
        sMtcReasonToImsReason.put(IUMtcCall.Fail_Reason.FAIL_REASON_FORBIDDEN_NOSUBSCRIBER,
                ImsReasonInfo.CODE_SIP_FORBIDDEN);

        sMtcReasonToImsReason.put(IUMtcCall.Reject_Reason.REJECT_REASON_BUSY_ISOTHERSCALL,
                ImsReasonInfo.CODE_REJECT_CALL_ON_OTHER_SUB);
        sMtcReasonToImsReason.put(IUMtcCall.Reject_Reason.REJECT_REASON_BUSY_ISEMERGENCY,
                ImsReasonInfo.CODE_REJECT_ONGOING_E911_CALL);
        sMtcReasonToImsReason.put(IUMtcCall.Reject_Reason.REJECT_REASON_TO_MO_PROGRESSING,
                ImsReasonInfo.CODE_REJECT_ONGOING_CALL_SETUP);
        sMtcReasonToImsReason.put(IUMtcCall.Reject_Reason.REJECT_REASON_BUSY_MAXCALL,
                ImsReasonInfo.CODE_REJECT_MAX_CALL_LIMIT_REACHED);
        sMtcReasonToImsReason.put(IUMtcCall.Reject_Reason.REJECT_REASON_BUSY_ESTABLISHING,
                ImsReasonInfo.CODE_REJECT_ONGOING_CALL_SETUP);

        sMtcReasonToImsReason.put(IUMtcCall.Fail_Reason.FAIL_REASON_SESSION_NONE,
                ImsReasonInfo.CODE_UNSPECIFIED);
        sMtcReasonToImsReason.put(IUMtcCall.Fail_Reason.FAIL_REASON_SESSION_UNKNOWN,
                ImsReasonInfo.CODE_UNSPECIFIED);

        // Reason codes: from reason code to SIP status code
        sReasonToSIPStatusCode = new Hashtable<Integer, Integer>();
        sReasonToSIPStatusCode.put(
                IUMtcCall.Fail_Reason.FAIL_REASON_FORBIDDEN_BARRING, 403);
        sReasonToSIPStatusCode.put(
                IUMtcCall.Fail_Reason.FAIL_REASON_FORBIDDEN_NOPROFILE, 403);
        sReasonToSIPStatusCode.put(
                IUMtcCall.Fail_Reason.FAIL_REASON_FORBIDDEN_EXPIRATION, 403);
        sReasonToSIPStatusCode.put(
                IUMtcCall.Fail_Reason.FAIL_REASON_FORBIDDEN_NOSUBSCRIBER, 403);
        sReasonToSIPStatusCode.put(
                IUMtcCall.Fail_Reason.FAIL_REASON_SESSION_FORBIDDEN, 403);
        sReasonToSIPStatusCode.put(
                IUMtcCall.Fail_Reason.FAIL_REASON_SESSION_SERVERERROR, 500);
        sReasonToSIPStatusCode.put(
                IUMtcCall.Fail_Reason.FAIL_REASON_SESSION_NOTFOUND, 404);
        sReasonToSIPStatusCode.put(
                IUMtcCall.Fail_Reason.FAIL_REASON_SESSION_NOTSUPPORTED, 415);
        sReasonToSIPStatusCode.put(
                IUMtcCall.Fail_Reason.FAIL_REASON_SESSION_TEMPUNAVAILABLE, 480);
        sReasonToSIPStatusCode.put(
                IUMtcCall.Fail_Reason.FAIL_REASON_SESSION_BADADDRESS, 484);
        sReasonToSIPStatusCode.put(
                IUMtcCall.Fail_Reason.FAIL_REASON_SESSION_BUSY, 486);
        sReasonToSIPStatusCode.put(
                IUMtcCall.Fail_Reason.FAIL_REASON_SESSION_REJECTED, 603);
        sReasonToSIPStatusCode.put(
                IUMtcCall.Fail_Reason.FAIL_REASON_SESSION_CANCELED, 487);
        sReasonToSIPStatusCode.put(
                IUMtcCall.Fail_Reason.FAIL_REASON_SESSION_BADRESPONSE, 400);
        sReasonToSIPStatusCode.put(
                IUMtcCall.Fail_Reason.FAIL_REASON_SESSION_NOTACCEPTABLE, 406);
        sReasonToSIPStatusCode.put(
                IUMtcCall.Fail_Reason.FAIL_REASON_SESSION_NOTACCEPTABLEHERE, 488);
        sReasonToSIPStatusCode.put(
                IUMtcCall.Fail_Reason.FAIL_REASON_SESSION_TIMEOUT, 408);
        sReasonToSIPStatusCode.put(
                IUMtcCall.Fail_Reason.FAIL_REASON_SESSION_NOTREACHABLE, 410);
        sReasonToSIPStatusCode.put(
                IUMtcCall.Fail_Reason.FAIL_REASON_SESSION_REDIRECTION, 302);
        sReasonToSIPStatusCode.put(
                IUMtcCall.Fail_Reason.FAIL_REASON_CONF_NOT_ACCEPTABLE, 406);
        sReasonToSIPStatusCode.put(
                IUMtcCall.Fail_Reason.FAIL_REASON_CONF_BUSY, 486);
        sReasonToSIPStatusCode.put(
                IUMtcCall.Fail_Reason.FAIL_REASON_CONF_NOT_ACCEPTABLE_HERE, 488);
        sReasonToSIPStatusCode.put(
                IUMtcCall.Fail_Reason.FAIL_REASON_CONF_TIMEOUT, 408);

        sReasonToSIPStatusCode.put(
                IUMtcCall.Fail_Reason.FAIL_REASON_SESSION_SERVER_AUTH, 401);
        sReasonToSIPStatusCode.put(
                IUMtcCall.Fail_Reason.FAIL_REASON_SESSION_SERVER_REQUEST_TERMINATED, 487);
        sReasonToSIPStatusCode.put(
                IUMtcCall.Fail_Reason.FAIL_REASON_SESSION_SERVER_INTERNAL_ERROR, 500);
        sReasonToSIPStatusCode.put(
                IUMtcCall.Fail_Reason.FAIL_REASON_SESSION_SERVER_SERVICE_UNAVAILABLE, 503);
        sReasonToSIPStatusCode.put(
                IUMtcCall.Fail_Reason.FAIL_REASON_SESSION_SERVER_DECLINE, 603);

        // User's status: from user status (int) to user status (string)
        sUserStatusToString = new Hashtable<Integer, String>();
        // "progressing" / "connect-fail" will be handled by the application
        // comparing to the previous status,
        // Ims just passes "disconnected" status in both cases.
        sUserStatusToString.put(UsersInfo.USER_STATUS_PROGRESSING,
                ImsConferenceState.STATUS_PENDING);
        sUserStatusToString.put(UsersInfo.USER_STATUS_CONNECTED,
                ImsConferenceState.STATUS_CONNECTED);
        sUserStatusToString.put(UsersInfo.USER_STATUS_DISCONNECTED,
                ImsConferenceState.STATUS_DISCONNECTED);
        sUserStatusToString.put(UsersInfo.USER_STATUS_ONHOLD,
                ImsConferenceState.STATUS_ON_HOLD);
        sUserStatusToString.put(UsersInfo.USER_STATUS_MUTEDVIAFOCUS,
                ImsConferenceState.STATUS_MUTED_VIA_FOCUS);
        sUserStatusToString.put(UsersInfo.USER_STATUS_PENDING,
                ImsConferenceState.STATUS_PENDING);
        sUserStatusToString.put(UsersInfo.USER_STATUS_ALERTING,
                ImsConferenceState.STATUS_ALERTING);
        sUserStatusToString.put(UsersInfo.USER_STATUS_DIALING_IN,
                ImsConferenceState.STATUS_DIALING_IN);
        sUserStatusToString.put(UsersInfo.USER_STATUS_DIALING_OUT,
                ImsConferenceState.STATUS_DIALING_OUT);
        sUserStatusToString.put(UsersInfo.USER_STATUS_DISCONNECTING,
                ImsConferenceState.STATUS_DISCONNECTING);
        sUserStatusToString.put(UsersInfo.USER_STATUS_FAIL,
                ImsConferenceState.STATUS_DISCONNECTED);
        sUserStatusToString.put(UsersInfo.USER_STATUS_REJECT,
                ImsConferenceState.STATUS_DISCONNECTED);
        sUserStatusToString.put(UsersInfo.USER_STATUS_BUSY,
                ImsConferenceState.STATUS_DISCONNECTED);
        sUserStatusToString.put(UsersInfo.USER_STATUS_SERVERERROR,
                ImsConferenceState.STATUS_DISCONNECTED);
        sUserStatusToString.put(UsersInfo.USER_STATUS_NOTSUPPORTED,
                ImsConferenceState.STATUS_DISCONNECTED);
        sUserStatusToString.put(UsersInfo.USER_STATUS_NOTACCEPTABLE,
                ImsConferenceState.STATUS_DISCONNECTED);
        sUserStatusToString.put(UsersInfo.USER_STATUS_NOANSWER,
                ImsConferenceState.STATUS_DISCONNECTED);
        sUserStatusToString.put(UsersInfo.USER_STATUS_NOTREACHABLE,
                ImsConferenceState.STATUS_DISCONNECTED);
        sUserStatusToString.put(UsersInfo.USER_STATUS_LOWBATTERY,
                ImsConferenceState.STATUS_DISCONNECTED);
        sUserStatusToString.put(UsersInfo.USER_STATUS_FORBIDDEN,
                ImsConferenceState.STATUS_DISCONNECTED);
        sUserStatusToString.put(UsersInfo.USER_STATUS_INTSERVERERROR,
                ImsConferenceState.STATUS_DISCONNECTED);
    }
}
