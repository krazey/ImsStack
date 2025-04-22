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

package com.android.imsstack.imsservice.mmtel;

import android.annotation.Nullable;
import android.os.Bundle;
import android.telephony.emergency.EmergencyNumber;
import android.telephony.emergency.EmergencyNumber.EmergencyCallRouting;
import android.telephony.emergency.EmergencyNumber.EmergencyServiceCategories;
import android.telephony.ims.ImsCallProfile;
import android.telephony.ims.ImsConferenceState;
import android.telephony.ims.ImsReasonInfo;
import android.telephony.ims.ImsStreamMediaProfile;
import android.text.TextUtils;

import com.android.imsstack.base.ImsPrivateProperties;
import com.android.imsstack.core.agents.AgentFactory;
import com.android.imsstack.core.agents.ConfigInterface;
import com.android.imsstack.core.agents.TelephonyInterface;
import com.android.imsstack.core.config.CarrierConfig;
import com.android.imsstack.enabler.mtc.CallFeature;
import com.android.imsstack.enabler.mtc.CallInfo;
import com.android.imsstack.enabler.mtc.CallReasonInfo;
import com.android.imsstack.enabler.mtc.IServiceStateTracker;
import com.android.imsstack.enabler.mtc.IUMtcCall;
import com.android.imsstack.enabler.mtc.IUMtcService;
import com.android.imsstack.enabler.mtc.IncomingMtcCall;
import com.android.imsstack.enabler.mtc.MediaInfo;
import com.android.imsstack.enabler.mtc.MtcCallInfo;
import com.android.imsstack.enabler.mtc.MtcCallUtils;
import com.android.imsstack.enabler.mtc.SuppInfo;
import com.android.imsstack.enabler.mtc.conf.UsersInfo;
import com.android.imsstack.imsservice.mmtel.base.ICallContext;
import com.android.imsstack.util.ImsLog;
import com.android.internal.telephony.imsphone.ImsExternalCallTracker;

import java.util.Arrays;
import java.util.LinkedHashMap;
import java.util.LinkedList;
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

    /**
     * Definition of the index as configured in
     * CarrierConfig::ImsEmergency::KEY_DYNAMIC_ROUTING_NUMBER_PER_PLMN_STRING_ARRAY
     */
    private static final int DYNAMIC_ROUTING_NUMBER_CONFIG_INDEX_COUNTRY_ISO = 0;
    private static final int DYNAMIC_ROUTING_NUMBER_CONFIG_INDEX_MNC = 1;
    private static final int DYNAMIC_ROUTING_NUMBER_CONFIG_INDEX_NUMBER = 2;

    /** "sos" URN for IMS emergency call */
    private static final String SOS_SERVICE_URN_POLICE = "urn:service:sos.police";
    private static final String SOS_SERVICE_URN_AMBULANCE = "urn:service:sos.ambulance";
    private static final String SOS_SERVICE_URN_FIRE = "urn:service:sos.fire";
    private static final String SOS_SERVICE_URN_MARINE = "urn:service:sos.marine";
    private static final String SOS_SERVICE_URN_MOUNTAIN = "urn:service:sos.mountain";
    private static final String SOS_SERVICE_URN_GENERIC = "urn:service:sos";
    //To-Do: Need to check AOSP behaviour.
    // for supplementary Service
    public static final String EXTRA_CDIV_CAUSE = "cdiv_cause";
    //  for Conference disconnect cause
    public static final String DISCONNECTED_CAUSE = "disconnected_cause";
    // for supplementary Service
    public static final String EXTRA_CDIV_HISTORY = "cdiv_history";
    //  rtt_avail : Indicates if the session is able to upgrade rtt during voice call
    public static final String EXTRA_RTT_AVAIL = "rtt_avail";
    /*subaddress : Indicates the sub-address when dialing the original number with *XXX.
     *      09012341234*123*456
     *      original-dialed-number: 09012341234
     *      subaddress: 123*456
     */
    public static final String EXTRA_SUBADDRESS = "subaddress";

    private static final LinkedHashMap<Integer, Integer> sMtcReasonToImsReason;
    private static final LinkedHashMap<Integer, String> sUserStatusToString;

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

        ImsCallUtils.setCallExtraIfPresent(profile,
                 ImsCallProfile.EXTRA_CALL_NETWORK_TYPE, ImsCallUtils.VAR_TYPE_INT, newProfile);

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

        boolean isAudioHD = MtcCallUtils.isAudioHDQuality(mi.AQuality);
        boolean isAudioUHD = MtcCallUtils.isAudioUHDQuality(mi.AQuality);

        if (isAudioHD || isAudioUHD) {
            profile.setCallRestrictCause(ImsCallProfile.CALL_RESTRICT_CAUSE_NONE);
        } else {
            profile.setCallRestrictCause(ImsCallProfile.CALL_RESTRICT_CAUSE_HD);
        }

        profile.setCallExtraBoolean(ImsCallUtils.EXTRA_RTT_AVAIL, ci.rttCapable);

        if (profile.getMediaProfile().getVideoDirection()
                == ImsStreamMediaProfile.DIRECTION_INACTIVE) {
            profile.updateMediaProfile(new ImsCallProfile(serviceType, callType, new Bundle(),
                    new ImsStreamMediaProfile(mediaProfile.getAudioQuality(),
                    mediaProfile.getAudioDirection(), mediaProfile.getVideoQuality(),
                    ImsStreamMediaProfile.DIRECTION_INVALID, mediaProfile.getRttMode())));
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
        } else if (incomingCall.OIPType == (IncomingMtcCall.OIPTYPE_PAYPHONE)) {
            oir = ImsCallProfile.OIR_PRESENTATION_PAYPHONE;
        } else if (incomingCall.OIPType == (IncomingMtcCall.OIPTYPE_UNAVAILABLE)) {
            oir = ImsCallProfile.OIR_PRESENTATION_UNAVAILABLE;
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

            if ("LGU".equals(ImsPrivateProperties.getSimOperator(context.getSlotId()))) {
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
        participant.putInt(ImsCallUtils.DISCONNECTED_CAUSE, disconnectedCause);

        return participant;
    }

    /**
    * Helper for creating the object of {@code ImsReasonInfo}
    *
    * @param callReasonInfo the target to be converted
    * @param flags indicates which member of the {@code CallReasonInfo} to be converted
    */
    public static ImsReasonInfo createReasonInfo(final CallReasonInfo callReasonInfo, int flags) {
        return createReasonInfo(callReasonInfo.mCode, callReasonInfo.mExtraCode,
                callReasonInfo.mExtraMessage, flags);
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
            ICallContext context, final ImsCallProfile profile, @Nullable String callee) {
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
        }

        // EXTRA_IS_CALL_PULL (true: Idicates a pull of an external call)
        boolean isCallPull = profile.getCallExtraBoolean(ImsCallProfile.EXTRA_IS_CALL_PULL, false);

        if (isCallPull) {
            int dialogId = profile.getProprietaryCallExtras()
                    .getInt(ImsExternalCallTracker.EXTRA_IMS_EXTERNAL_CALL_ID, -1);
            si.addService(SuppInfo.TYPE_CALL_PULL, isCallPull, dialogId, null);
        }

        // "sos" URN for IMS emergency call
        if (isEmergencyCall(profile)) {
            String urn = null;

            if (profile.isEmergencyCallTesting()) {
                ImsLog.d("Omit TYPE_TARGET_URI");
            } else {
                List<String> urns = profile.getEmergencyUrns();

                if (urns.isEmpty()) {
                    @EmergencyCallRouting
                    int emergencyRouting = getEmergencyRoutingFromCallProfile(profile);
                    if (callee != null && !callee.isEmpty()) {
                        emergencyRouting =
                                maybeUpdateEmergencyRouting(context, emergencyRouting, callee);
                    }

                    if (emergencyRouting != EmergencyNumber.EMERGENCY_CALL_ROUTING_NORMAL) {
                        ConfigInterface config = AgentFactory.getInstance().getAgent(
                                ConfigInterface.class, context.getSlotId());
                        CarrierConfig cc = config != null ? config.getCarrierConfig() : null;
                        int[] policies = cc != null ? cc.getIntArray(
                            CarrierConfig.ImsEmergency.KEY_POLICY_FOR_EMERGENCY_URN_INT_ARRAY) : null;
                        urn = getSosUrnFromECallServiceCategory(
                                profile.getEmergencyServiceCategories(), policies);
                    }
                } else {
                    // The first item has priority ??
                    urn = urns.get(0);
                }
            }

            if (urn != null) {
                si.addService_str(SuppInfo.TYPE_TARGET_URI, urn);
            }
        }

        ImsSuppInfoUtils.addSuppInfoForIms(context, profile, si);
        ImsSuppInfoUtils.addSuppInfoForCallComposer(profile, si);

        return si;
    }

    /**
     * Gets the emergency call routing value from {@link ImsCallProfile}.
     *
     * @param profile The {@link ImsCallProfile} which has an emergency call routing value.
     * @return The emergency call routing value.
     */
    public static @EmergencyCallRouting int getEmergencyRoutingFromCallProfile(
            final ImsCallProfile profile) {
        return profile.getEmergencyCallRouting();
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
            @EmergencyServiceCategories int category, int[] policies) {
        if (containsPolicy(policies, CarrierConfig.ImsEmergency.NOT_USE_SERVICE_CATEGORY)) {
            return SOS_SERVICE_URN_GENERIC;
        }
        if ((category == EmergencyNumber.EMERGENCY_SERVICE_CATEGORY_UNSPECIFIED)
                && containsPolicy(
                policies, CarrierConfig.ImsEmergency.USE_POLICE_FOR_UNSPECIFIED)) {
            return SOS_SERVICE_URN_POLICE;
        }
        if (isMultipleCategories(category) && containsPolicy(policies,
                CarrierConfig.ImsEmergency.USE_GENERIC_FOR_MULTIPLE_CATEGORIES)) {
            return SOS_SERVICE_URN_GENERIC;
        }

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

    /**
     * Sets the emergency URN information to {@link ImsCallProfile}.
     *
     * @param info The {@link CallReasonInfo} which contains the emergency URN information.
     * @param profile The {@link ImsCallProfile} will be updated.
     */
    public static void setSosUrnFromCallReasonInfo(
            final CallReasonInfo info, ImsCallProfile profile) {
        String sosUrn = null;
        int emergencyServiceCategory = 0;

        switch (info.mExtraCode)  {
            case CallReasonInfo.EXTRA_CODE_EMERGENCYSERVICE_POLICE: {
                sosUrn = SOS_SERVICE_URN_POLICE;
                emergencyServiceCategory = EmergencyNumber.EMERGENCY_SERVICE_CATEGORY_POLICE;
                break;
            }
            case CallReasonInfo.EXTRA_CODE_EMERGENCYSERVICE_AMBULANCE: {
                sosUrn = SOS_SERVICE_URN_AMBULANCE;
                emergencyServiceCategory = EmergencyNumber.EMERGENCY_SERVICE_CATEGORY_AMBULANCE;
                break;
            }
            case CallReasonInfo.EXTRA_CODE_EMERGENCYSERVICE_FIRE: {
                sosUrn = SOS_SERVICE_URN_FIRE;
                emergencyServiceCategory = EmergencyNumber.EMERGENCY_SERVICE_CATEGORY_FIRE_BRIGADE;
                break;
            }
            case CallReasonInfo.EXTRA_CODE_EMERGENCYSERVICE_MARINE: {
                sosUrn = SOS_SERVICE_URN_MARINE;
                emergencyServiceCategory = EmergencyNumber.EMERGENCY_SERVICE_CATEGORY_MARINE_GUARD;
                break;
            }
            case CallReasonInfo.EXTRA_CODE_EMERGENCYSERVICE_MOUNTAIN: {
                sosUrn = SOS_SERVICE_URN_MOUNTAIN;
                emergencyServiceCategory =
                        EmergencyNumber.EMERGENCY_SERVICE_CATEGORY_MOUNTAIN_RESCUE;
                break;
            }
            case CallReasonInfo.EXTRA_CODE_EMERGENCYSERVICE_COUNTRY_SPECIFIC: {
                if (info.mExtraMessage.length() > 0) {
                    sosUrn = info.mExtraMessage;
                } else {
                    sosUrn = SOS_SERVICE_URN_GENERIC;
                }
                emergencyServiceCategory = EmergencyNumber.EMERGENCY_SERVICE_CATEGORY_UNSPECIFIED;
                break;
            }
            default: {
                sosUrn = SOS_SERVICE_URN_GENERIC;
                emergencyServiceCategory = EmergencyNumber.EMERGENCY_SERVICE_CATEGORY_UNSPECIFIED;
            }
        }

        List emergencyUrn = new LinkedList<String>();
        emergencyUrn.add(sosUrn);
        profile.setEmergencyUrns(emergencyUrn);
        profile.setEmergencyServiceCategories(emergencyServiceCategory);
    }

   public static @EmergencyCallRouting int maybeUpdateEmergencyRouting(
            ICallContext context, @EmergencyCallRouting int emergencyRouting, String callee) {
        if (emergencyRouting != EmergencyNumber.EMERGENCY_CALL_ROUTING_UNKNOWN) {
            return emergencyRouting;
        }

        IServiceStateTracker serviceStateTracker = context.getServiceStateTracker();
        if (isFromNetworkOrSim(context, callee)) {
            return emergencyRouting;
        }

        if (!isDynamicRoutingNumber(context, callee)) {
            return emergencyRouting;
        }

        if (serviceStateTracker.isServiceRegistered(IUMtcService.SERVICE_VOIP)) {
            ImsLog.d("update to EMERGENCY_CALL_ROUTING_NORMAL");
            emergencyRouting = EmergencyNumber.EMERGENCY_CALL_ROUTING_NORMAL;
        }

        return emergencyRouting;
    }

    public static String getStringFromUserStatus(int status) {
        // "progressing" / "connect-fail" will be handled by the application
        // comparing to the previous status,
        // Ims just passes "disconnected" status in both cases.
        String statusString = sUserStatusToString.get(status);
        return (statusString != null) ? statusString : "";
    }

    public static int getDisconnectedCauseFromUserStatus(int status) {
        if (status >= UsersInfo.USER_STATUS_REJECT
                && status <= UsersInfo.USER_STATUS_INTSERVERERROR) {
            return status;
        } else {
            return 0;
        }
    }

    public static int getExtraCodeFromMtc(int reason, int extraCode) {
        if (MtcCallUtils.isCallTerminatedByCSRetry(reason)) {
            if (extraCode == CallReasonInfo.EXTRA_CODE_CALL_RETRY_SILENT_REDIAL) {
                return ImsReasonInfo.EXTRA_CODE_CALL_RETRY_SILENT_REDIAL;
            } else {
                return (extraCode > 0) ? extraCode : ImsReasonInfo.CODE_UNSPECIFIED;
            }
        } else if (MtcCallUtils.isCallTerminatedByECallRetry(reason)) {
            return (extraCode >= 300) ? extraCode : getEmergencyServiceCode(extraCode);
        } else if (extraCode > 0) {
        // TODO : need to modify this after emergency domain selection policy is decided.
        /*else if (reason == IUMtcCall.Fail_Reason.FAIL_REASON_SESSION_RETRYVOLTE) {
            // SIP status code
            return (extraCode > 0) ? extraCode : ImsReasonInfo.CODE_UNSPECIFIED;
        }*/
            return extraCode;
        } else {
            return ImsReasonInfo.CODE_UNSPECIFIED;
        }
    }

    public static int getReasonFromMTC(int reason) {
        Integer code = sMtcReasonToImsReason.get(reason);
        return (code != null) ? code.intValue() : ImsReasonInfo.CODE_UNSPECIFIED;
    }

    /**
    * converts the code of {@code ImsReasonInfo} to the code of {@code CallReasonInfo}
    * when received rejection.
    *
    * @param reason the code of {@code ImsReasonInfo}
    * @return the code of {@code CallReasonInfo}
    */
    public static int getRejectCallReasonInfoCodeFromImsReasonInfo(int reason) {
        switch (reason) {
            case ImsReasonInfo.CODE_LOCAL_CALL_EXCEEDED:
                return CallReasonInfo.CODE_LOCAL_CALL_EXCEEDED;
            case ImsReasonInfo.CODE_LOCAL_CALL_BUSY:
                return CallReasonInfo.CODE_LOCAL_CALL_BUSY;
            case ImsReasonInfo.CODE_USER_NOANSWER:
                return CallReasonInfo.CODE_USER_NOANSWER;
            case ImsReasonInfo.CODE_LOCAL_CALL_DECLINE: // FALL_THROUGH
            case ImsReasonInfo.CODE_USER_DECLINE:
                return CallReasonInfo.CODE_USER_DECLINE;
            case ImsReasonInfo.CODE_LOCAL_LOW_BATTERY: // FALL-THROUGH
            case ImsReasonInfo.CODE_LOW_BATTERY:
                return CallReasonInfo.CODE_LOW_BATTERY;
            case ImsReasonInfo.CODE_USER_IGNORE:
                return CallReasonInfo.CODE_USER_IGNORE;
            case ImsReasonInfo.CODE_USER_REJECTED_SESSION_MODIFICATION:
                return CallReasonInfo.CODE_USER_REJECTED_SESSION_MODIFICATION;
            default:
                return CallReasonInfo.CODE_UNSPECIFIED;
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

    /**
    * converts the code of {@code ImsReasonInfo} to the code of {@code CallReasonInfo}
    * when received termination.
    *
    * @param reason the code of {@code ImsReasonInfo}
    * @return the code of {@code CallReasonInfo}
    */
    public static int getTerminateCallReasonInfoCodeFromImsReasonInfo(int reason) {
        switch (reason) {
            case ImsReasonInfo.CODE_LOCAL_CALL_DECLINE: // FALL_THROUGH
            case ImsReasonInfo.CODE_USER_DECLINE: // FALL_THROUGH
            case ImsReasonInfo.CODE_USER_TERMINATED:
                return CallReasonInfo.CODE_USER_TERMINATED;
            case ImsReasonInfo.CODE_LOCAL_POWER_OFF:
                return CallReasonInfo.CODE_LOCAL_POWER_OFF;
            case ImsReasonInfo.CODE_LOCAL_LOW_BATTERY: // FALL-THROUGH
            case ImsReasonInfo.CODE_LOW_BATTERY:
                return CallReasonInfo.CODE_LOCAL_LOW_BATTERY;
            default:
                return CallReasonInfo.CODE_UNSPECIFIED;
        }
    }

    public static boolean isCallOnNativeAppsAndCountryKR(ICallContext context) {
        return "KR".equals(ImsPrivateProperties.getSimCountry(context.getSlotId()));
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

    public static boolean isAlternateEmergencyCall(CallReasonInfo info) {
        return (info.mCode == ImsReasonInfo.CODE_SIP_ALTERNATE_EMERGENCY_CALL);
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

    /**
    * converts the {@code mCode} of {@code CallReasonInfo} with specific case.
    *
    * @param profile used for this operation
    * @param callReasonInfo the target to be converted
    */
    public static void refineCallReasonInfoForCode(ICallContext context,
            ImsCallProfile profile, CallReasonInfo callReasonInfo) {
        if ((callReasonInfo.mCode == CallReasonInfo.CODE_SIP_NOT_SUPPORTED)
                && (callReasonInfo.mExtraCode == 415)
                && ImsCallUtils.isVideoCall(profile.getCallType())
                && "LGU".equals(ImsPrivateProperties.getSimOperator(context.getSlotId()))) {
            callReasonInfo.mCode = CallReasonInfo.CODE_LOCAL_CALL_VOLTE_RETRY_REQUIRED;
        }
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
        profile.setCallExtra(ImsCallProfile.EXTRA_USSD,
                MtcCallInfo.isUssi(ci) ? "true" : "false");
        profile.setCallExtraBoolean(ImsCallUtils.EXTRA_RTT_AVAIL, MtcCallInfo.isRttCapable(ci));

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
                if (ss.type == SuppInfo.TYPE_CALLING_NUM_VERIFICATION) {
                    int verStat =  getOIVerStatusFromCallingNumVerificationType(ss.intValue);
                    if (verStat != -1) {
                        profile.setCallerNumberVerificationStatus(verStat);
                    }
                } else {
                    profile.setCallExtraInt(key, ss.intValue);
                }
            } else if (MtcCallUtils.isSuppInfoString(ss.type)) {
                String key = getCallExtraNameForString(context, ss.type);

                if (key != null) {
                    profile.setCallExtra(key, ss.strValue);
                }
            }
        }

        ImsSuppInfoUtils.addCallExtraForCallComposer(si, profile);
    }

    public static void updateCallProfileFromSuppInfoExtension(ICallContext context,
            ImsCallProfile profile, final SuppInfo suppInfo) {
        ImsSuppInfoUtils.addCallExtraForApp(context, suppInfo, profile);
    }

    public static void updateCallProfileOnSessionProgressing(ICallContext context,
            ImsCallProfile profile, final CallInfo callInfo, final SuppInfo suppInfo) {
        int callType = getProfileCallTypeFromCallInfo(callInfo);
        profile.updateCallType(new ImsCallProfile(profile.getServiceType(), callType));
    }

    public static boolean isEmergencyPdnUsedForEmergencyCallViaWfc(ICallContext context) {
        // TODO: need to add a carrier configuration.
        return false;
    }

    /**
     * Convert the Dtmf digit from int to char
     *
     * @param numDtmf int type Dtmf digit
     * @return char type Dtmf digit
     */
    public static char convertIntToDtmfDigit(int numDtmf) {
        if (numDtmf < 10) {
            return (char) (numDtmf + '0');       // convert to '0'~'9'
        } else if (numDtmf == 10) {
            return '*';
        } else if (numDtmf == 11) {
            return '#';
        }
        // TODO : Need to check if DTMF service supports alphabets from A to D

        return (char) numDtmf;
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
        default:
            // no-op
            break;
        }

        return ImsSuppInfoUtils.getCallExtraNameForBoolean(context, suppInfo);
    }

    private static String getCallExtraNameForInt(ICallContext context, int suppInfo) {
        switch (suppInfo) {
            case SuppInfo.TYPE_CDIV_CAUSE:
                return ImsCallUtils.EXTRA_CDIV_CAUSE;
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
            case SuppInfo.TYPE_CDIV_HISTORY:
                return ImsCallUtils.EXTRA_CDIV_HISTORY;
            default:
                // no-op
                break;
        }

        return ImsSuppInfoUtils.getCallExtraNameForString(context, suppInfo);
    }

    private static int getEmergencyServiceCode(int code) {
        switch (code) {
        // TODO : need to modify this after emergency domain selection policy is decided.
        /*case IUMtcCall.Fail_Reason.CODE_EMERGENCYSERVICE_GENERIC:
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
            return ImsReasonInfoEx.EXTRA_CODE_ECALL_RETRY_MOUNTAIN_RESCUE;*/
            default:
                return CallReasonInfo.EXTRA_CODE_CALL_RETRY_NORMAL;
        }
    }

    private static boolean isFromNetworkOrSim(ICallContext context, String callee) {
        TelephonyInterface telephony = AgentFactory.getInstance().getAgent(
                TelephonyInterface.class, context.getSlotId());
        if (telephony == null) {
            return false;
        }
        for (EmergencyNumber number : telephony.getEmergencyNumberList()) {
            if (number.getNumber().equals(callee)) {
                if (number.isFromSources(EmergencyNumber.EMERGENCY_NUMBER_SOURCE_NETWORK_SIGNALING)
                        || number.isFromSources(EmergencyNumber.EMERGENCY_NUMBER_SOURCE_SIM)) {
                    ImsLog.d("SIM or NETWORK emergency number");
                    return true;
                }
            }
        }

        return false;
    }

    private static boolean isDynamicRoutingNumber(ICallContext context, String callee) {
        String[] configs = AgentFactory.getInstance().getAgent(ConfigInterface.class,
                context.getSlotId()).getCarrierConfig().getStringArray(CarrierConfig.ImsEmergency
                        .KEY_DYNAMIC_ROUTING_NUMBER_PER_PLMN_STRING_ARRAY);
        if (configs == null) {
            return false;
        }

        TelephonyInterface telephony = AgentFactory.getInstance().getAgent(
                TelephonyInterface.class, context.getSlotId());
        if (telephony == null) {
            return false;
        }
        String countryIso = telephony.getNetworkCountryIso();
        String mnc = telephony.getNetworkMnc();
        ImsLog.d("isDynamicRoutingNumber :: countryIso=" + countryIso + ", mnc=" + mnc);

        for (String config : configs) {
            String[] fields = config.split(",");
            // Format: "iso,mnc,number1,number2,..."
            if (fields == null || fields.length < 3) {
                continue;
            }
            if (!countryIso.equals(fields[DYNAMIC_ROUTING_NUMBER_CONFIG_INDEX_COUNTRY_ISO])) {
                continue;
            }
            if (!fields[DYNAMIC_ROUTING_NUMBER_CONFIG_INDEX_MNC].isEmpty() && mnc != null
                    && !mnc.equals(fields[DYNAMIC_ROUTING_NUMBER_CONFIG_INDEX_MNC])) {
                continue;
            }
            for (int i = DYNAMIC_ROUTING_NUMBER_CONFIG_INDEX_NUMBER; i < fields.length; i++) {
                if (callee.equals(fields[i])) {
                    return true;
                }
            }
        }

        return false;
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
                return ImsCallProfile.VERIFICATION_STATUS_PASSED;
            case SuppInfo.CALLING_NUM_VERSTAT_NOT_VERIFIED:
                return ImsCallProfile.VERIFICATION_STATUS_FAILED;
            case SuppInfo.CALLING_NUM_VERSTAT_NONE:
                return ImsCallProfile.VERIFICATION_STATUS_NOT_VERIFIED;
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

    private static boolean containsPolicy(int[] policies, int policy) {
        if (policies == null) {
            return false;
        }

        return Arrays.stream(policies).anyMatch(value -> value == policy);
    }

    private static boolean isMultipleCategories(int categories) {
        return Integer.bitCount(categories) > 1;
    }

    static {
        // Reason codes: from MtcCall to ImsCall
        sMtcReasonToImsReason = new LinkedHashMap<Integer, Integer>();
        sMtcReasonToImsReason.put(CallReasonInfo.CODE_UNSPECIFIED,
                ImsReasonInfo.CODE_UNSPECIFIED);
        sMtcReasonToImsReason.put(CallReasonInfo.CODE_LOCAL_ILLEGAL_ARGUMENT,
                ImsReasonInfo.CODE_LOCAL_ILLEGAL_ARGUMENT);
        sMtcReasonToImsReason.put(CallReasonInfo.CODE_LOCAL_ILLEGAL_STATE,
                ImsReasonInfo.CODE_LOCAL_ILLEGAL_STATE);
        sMtcReasonToImsReason.put(CallReasonInfo.CODE_LOCAL_INTERNAL_ERROR,
                ImsReasonInfo.CODE_LOCAL_INTERNAL_ERROR);
        sMtcReasonToImsReason.put(CallReasonInfo.CODE_LOCAL_ENDED_BY_CONFERENCE_MERGE,
                ImsReasonInfo.CODE_LOCAL_ENDED_BY_CONFERENCE_MERGE);
        sMtcReasonToImsReason.put(CallReasonInfo.CODE_LOCAL_POWER_OFF,
                ImsReasonInfo.CODE_LOCAL_POWER_OFF);
        sMtcReasonToImsReason.put(CallReasonInfo.CODE_LOCAL_LOW_BATTERY,
                ImsReasonInfo.CODE_LOCAL_LOW_BATTERY);
        sMtcReasonToImsReason.put(CallReasonInfo.CODE_LOCAL_NETWORK_NO_SERVICE,
                ImsReasonInfo.CODE_LOCAL_NETWORK_NO_SERVICE);
        sMtcReasonToImsReason.put(CallReasonInfo.CODE_LOCAL_NETWORK_NO_LTE_COVERAGE,
                ImsReasonInfo.CODE_LOCAL_NETWORK_NO_LTE_COVERAGE);
        sMtcReasonToImsReason.put(CallReasonInfo.CODE_LOCAL_NETWORK_IP_CHANGED,
                ImsReasonInfo.CODE_LOCAL_NETWORK_IP_CHANGED);
        sMtcReasonToImsReason.put(CallReasonInfo.CODE_LOCAL_SERVICE_UNAVAILABLE,
                ImsReasonInfo.CODE_LOCAL_SERVICE_UNAVAILABLE);
        sMtcReasonToImsReason.put(CallReasonInfo.CODE_LOCAL_NOT_REGISTERED,
                ImsReasonInfo.CODE_LOCAL_NOT_REGISTERED);
        sMtcReasonToImsReason.put(CallReasonInfo.CODE_LOCAL_CALL_EXCEEDED,
                ImsReasonInfo.CODE_LOCAL_CALL_EXCEEDED);
        sMtcReasonToImsReason.put(CallReasonInfo.CODE_LOCAL_CALL_BUSY,
                ImsReasonInfo.CODE_LOCAL_CALL_BUSY);
        sMtcReasonToImsReason.put(CallReasonInfo.CODE_LOCAL_CALL_DECLINE,
                ImsReasonInfo.CODE_LOCAL_CALL_DECLINE);
        sMtcReasonToImsReason.put(CallReasonInfo.CODE_LOCAL_CALL_VCC_ON_PROGRESSING,
                ImsReasonInfo.CODE_LOCAL_CALL_VCC_ON_PROGRESSING);
        sMtcReasonToImsReason.put(CallReasonInfo.CODE_LOCAL_CALL_RESOURCE_RESERVATION_FAILED,
                ImsReasonInfo.CODE_LOCAL_CALL_RESOURCE_RESERVATION_FAILED);
        sMtcReasonToImsReason.put(CallReasonInfo.CODE_LOCAL_CALL_CS_RETRY_REQUIRED,
                ImsReasonInfo.CODE_LOCAL_CALL_CS_RETRY_REQUIRED);
        sMtcReasonToImsReason.put(CallReasonInfo.CODE_LOCAL_CALL_VOLTE_RETRY_REQUIRED,
                ImsReasonInfo.CODE_LOCAL_CALL_VOLTE_RETRY_REQUIRED);
        sMtcReasonToImsReason.put(CallReasonInfo.CODE_LOCAL_CALL_TERMINATED,
                ImsReasonInfo.CODE_LOCAL_CALL_TERMINATED);
        sMtcReasonToImsReason.put(CallReasonInfo.CODE_LOCAL_HO_NOT_FEASIBLE,
                ImsReasonInfo.CODE_LOCAL_HO_NOT_FEASIBLE);
        sMtcReasonToImsReason.put(CallReasonInfo.CODE_TIMEOUT_1XX_WAITING,
                ImsReasonInfo.CODE_TIMEOUT_1XX_WAITING);
        sMtcReasonToImsReason.put(CallReasonInfo.CODE_TIMEOUT_NO_ANSWER,
                ImsReasonInfo.CODE_TIMEOUT_NO_ANSWER);
        sMtcReasonToImsReason.put(CallReasonInfo.CODE_TIMEOUT_NO_ANSWER_CALL_UPDATE,
                ImsReasonInfo.CODE_TIMEOUT_NO_ANSWER_CALL_UPDATE);
        sMtcReasonToImsReason.put(CallReasonInfo.CODE_CALL_BARRED,
                ImsReasonInfo.CODE_CALL_BARRED);
        sMtcReasonToImsReason.put(CallReasonInfo.CODE_SIP_REDIRECTED,
                ImsReasonInfo.CODE_SIP_REDIRECTED);
        sMtcReasonToImsReason.put(CallReasonInfo.CODE_SIP_BAD_REQUEST,
                ImsReasonInfo.CODE_SIP_BAD_REQUEST);
        sMtcReasonToImsReason.put(CallReasonInfo.CODE_SIP_FORBIDDEN,
                ImsReasonInfo.CODE_SIP_FORBIDDEN);
        sMtcReasonToImsReason.put(CallReasonInfo.CODE_SIP_NOT_FOUND,
                ImsReasonInfo.CODE_SIP_NOT_FOUND);
        sMtcReasonToImsReason.put(CallReasonInfo.CODE_SIP_NOT_SUPPORTED,
                ImsReasonInfo.CODE_SIP_NOT_SUPPORTED);
        sMtcReasonToImsReason.put(CallReasonInfo.CODE_SIP_REQUEST_TIMEOUT,
                ImsReasonInfo.CODE_SIP_REQUEST_TIMEOUT);
        sMtcReasonToImsReason.put(CallReasonInfo.CODE_SIP_TEMPRARILY_UNAVAILABLE,
                ImsReasonInfo.CODE_SIP_TEMPRARILY_UNAVAILABLE);
        sMtcReasonToImsReason.put(CallReasonInfo.CODE_SIP_BAD_ADDRESS,
                ImsReasonInfo.CODE_SIP_BAD_ADDRESS);
        sMtcReasonToImsReason.put(CallReasonInfo.CODE_SIP_BUSY,
                ImsReasonInfo.CODE_SIP_BUSY);
        sMtcReasonToImsReason.put(CallReasonInfo.CODE_SIP_REQUEST_CANCELLED,
                ImsReasonInfo.CODE_SIP_REQUEST_CANCELLED);
        sMtcReasonToImsReason.put(CallReasonInfo.CODE_SIP_NOT_ACCEPTABLE,
                ImsReasonInfo.CODE_SIP_NOT_ACCEPTABLE);
        sMtcReasonToImsReason.put(CallReasonInfo.CODE_SIP_NOT_REACHABLE,
                ImsReasonInfo.CODE_SIP_NOT_REACHABLE);
        sMtcReasonToImsReason.put(CallReasonInfo.CODE_SIP_CLIENT_ERROR,
                ImsReasonInfo.CODE_SIP_CLIENT_ERROR);
        sMtcReasonToImsReason.put(CallReasonInfo.CODE_SIP_TRANSACTION_DOES_NOT_EXIST,
                ImsReasonInfo.CODE_SIP_TRANSACTION_DOES_NOT_EXIST);
        sMtcReasonToImsReason.put(CallReasonInfo.CODE_SIP_SERVER_INTERNAL_ERROR,
                ImsReasonInfo.CODE_SIP_SERVER_INTERNAL_ERROR);
        sMtcReasonToImsReason.put(CallReasonInfo.CODE_SIP_SERVICE_UNAVAILABLE,
                ImsReasonInfo.CODE_SIP_SERVICE_UNAVAILABLE);
        sMtcReasonToImsReason.put(CallReasonInfo.CODE_SIP_SERVER_TIMEOUT,
                ImsReasonInfo.CODE_SIP_SERVER_TIMEOUT);
        sMtcReasonToImsReason.put(CallReasonInfo.CODE_SIP_SERVER_ERROR,
                ImsReasonInfo.CODE_SIP_SERVER_ERROR);
        sMtcReasonToImsReason.put(CallReasonInfo.CODE_SIP_USER_REJECTED,
                ImsReasonInfo.CODE_SIP_USER_REJECTED);
        sMtcReasonToImsReason.put(CallReasonInfo.CODE_SIP_GLOBAL_ERROR,
                ImsReasonInfo.CODE_SIP_GLOBAL_ERROR);
        sMtcReasonToImsReason.put(CallReasonInfo.CODE_EMERGENCY_TEMP_FAILURE,
                ImsReasonInfo.CODE_EMERGENCY_TEMP_FAILURE);
        sMtcReasonToImsReason.put(CallReasonInfo.CODE_EMERGENCY_PERM_FAILURE,
                ImsReasonInfo.CODE_EMERGENCY_PERM_FAILURE);
        sMtcReasonToImsReason.put(CallReasonInfo.CODE_SIP_METHOD_NOT_ALLOWED,
                ImsReasonInfo.CODE_SIP_METHOD_NOT_ALLOWED);
        sMtcReasonToImsReason.put(CallReasonInfo.CODE_SIP_PROXY_AUTHENTICATION_REQUIRED,
                ImsReasonInfo.CODE_SIP_PROXY_AUTHENTICATION_REQUIRED);
        sMtcReasonToImsReason.put(CallReasonInfo.CODE_SIP_REQUEST_ENTITY_TOO_LARGE,
                ImsReasonInfo.CODE_SIP_REQUEST_ENTITY_TOO_LARGE);
        sMtcReasonToImsReason.put(CallReasonInfo.CODE_SIP_REQUEST_URI_TOO_LARGE,
                ImsReasonInfo.CODE_SIP_REQUEST_URI_TOO_LARGE);
        sMtcReasonToImsReason.put(CallReasonInfo.CODE_SIP_EXTENSION_REQUIRED,
                ImsReasonInfo.CODE_SIP_EXTENSION_REQUIRED);
        sMtcReasonToImsReason.put(CallReasonInfo.CODE_SIP_INTERVAL_TOO_BRIEF,
                ImsReasonInfo.CODE_SIP_INTERVAL_TOO_BRIEF);
        sMtcReasonToImsReason.put(CallReasonInfo.CODE_SIP_CALL_OR_TRANS_DOES_NOT_EXIST,
                ImsReasonInfo.CODE_SIP_CALL_OR_TRANS_DOES_NOT_EXIST);
        sMtcReasonToImsReason.put(CallReasonInfo.CODE_SIP_LOOP_DETECTED,
                ImsReasonInfo.CODE_SIP_LOOP_DETECTED);
        sMtcReasonToImsReason.put(CallReasonInfo.CODE_SIP_TOO_MANY_HOPS,
                ImsReasonInfo.CODE_SIP_TOO_MANY_HOPS);
        sMtcReasonToImsReason.put(CallReasonInfo.CODE_SIP_AMBIGUOUS,
                ImsReasonInfo.CODE_SIP_AMBIGUOUS);
        sMtcReasonToImsReason.put(CallReasonInfo.CODE_SIP_REQUEST_PENDING,
                ImsReasonInfo.CODE_SIP_REQUEST_PENDING);
        sMtcReasonToImsReason.put(CallReasonInfo.CODE_SIP_UNDECIPHERABLE,
                ImsReasonInfo.CODE_SIP_UNDECIPHERABLE);
        sMtcReasonToImsReason.put(CallReasonInfo.CODE_MEDIA_INIT_FAILED,
                ImsReasonInfo.CODE_MEDIA_INIT_FAILED);
        sMtcReasonToImsReason.put(CallReasonInfo.CODE_MEDIA_NO_DATA,
                ImsReasonInfo.CODE_MEDIA_NO_DATA);
        sMtcReasonToImsReason.put(CallReasonInfo.CODE_MEDIA_NOT_ACCEPTABLE,
                ImsReasonInfo.CODE_MEDIA_NOT_ACCEPTABLE);
        sMtcReasonToImsReason.put(CallReasonInfo.CODE_MEDIA_UNSPECIFIED,
                ImsReasonInfo.CODE_MEDIA_UNSPECIFIED);
        sMtcReasonToImsReason.put(CallReasonInfo.CODE_USER_TERMINATED,
                ImsReasonInfo.CODE_USER_TERMINATED);
        sMtcReasonToImsReason.put(CallReasonInfo.CODE_USER_NOANSWER,
                ImsReasonInfo.CODE_USER_NOANSWER);
        sMtcReasonToImsReason.put(CallReasonInfo.CODE_USER_IGNORE,
                ImsReasonInfo.CODE_USER_IGNORE);
        sMtcReasonToImsReason.put(CallReasonInfo.CODE_USER_DECLINE,
                ImsReasonInfo.CODE_USER_DECLINE);
        sMtcReasonToImsReason.put(CallReasonInfo.CODE_LOW_BATTERY,
                ImsReasonInfo.CODE_LOW_BATTERY);
        sMtcReasonToImsReason.put(CallReasonInfo.CODE_BLACKLISTED_CALL_ID,
                ImsReasonInfo.CODE_BLACKLISTED_CALL_ID);
        sMtcReasonToImsReason.put(CallReasonInfo.CODE_USER_TERMINATED_BY_REMOTE,
                ImsReasonInfo.CODE_USER_TERMINATED_BY_REMOTE);
        sMtcReasonToImsReason.put(CallReasonInfo.CODE_USER_REJECTED_SESSION_MODIFICATION,
                ImsReasonInfo.CODE_USER_REJECTED_SESSION_MODIFICATION);
        sMtcReasonToImsReason.put(CallReasonInfo.CODE_USER_CANCELLED_SESSION_MODIFICATION,
                ImsReasonInfo.CODE_USER_CANCELLED_SESSION_MODIFICATION);
        sMtcReasonToImsReason.put(CallReasonInfo.CODE_SESSION_MODIFICATION_FAILED,
                ImsReasonInfo.CODE_SESSION_MODIFICATION_FAILED);
        sMtcReasonToImsReason.put(CallReasonInfo.CODE_MULTIENDPOINT_NOT_SUPPORTED,
                ImsReasonInfo.CODE_MULTIENDPOINT_NOT_SUPPORTED);
        sMtcReasonToImsReason.put(CallReasonInfo.CODE_ANSWERED_ELSEWHERE,
                ImsReasonInfo.CODE_ANSWERED_ELSEWHERE);
        sMtcReasonToImsReason.put(CallReasonInfo.CODE_CALL_PULL_OUT_OF_SYNC,
                ImsReasonInfo.CODE_CALL_PULL_OUT_OF_SYNC);
        sMtcReasonToImsReason.put(CallReasonInfo.CODE_CALL_END_CAUSE_CALL_PULL,
                ImsReasonInfo.CODE_CALL_END_CAUSE_CALL_PULL);
        sMtcReasonToImsReason.put(CallReasonInfo.CODE_REJECTED_ELSEWHERE,
                ImsReasonInfo.CODE_REJECTED_ELSEWHERE);
        sMtcReasonToImsReason.put(CallReasonInfo.CODE_SUPP_SVC_FAILED,
                ImsReasonInfo.CODE_SUPP_SVC_FAILED);
        sMtcReasonToImsReason.put(CallReasonInfo.CODE_SUPP_SVC_CANCELLED,
                ImsReasonInfo.CODE_SUPP_SVC_CANCELLED);
        sMtcReasonToImsReason.put(CallReasonInfo.CODE_SUPP_SVC_REINVITE_COLLISION,
                ImsReasonInfo.CODE_SUPP_SVC_REINVITE_COLLISION);
        sMtcReasonToImsReason.put(CallReasonInfo.CODE_MAXIMUM_NUMBER_OF_CALLS_REACHED,
                ImsReasonInfo.CODE_MAXIMUM_NUMBER_OF_CALLS_REACHED);
        sMtcReasonToImsReason.put(CallReasonInfo.CODE_REMOTE_CALL_DECLINE,
                ImsReasonInfo.CODE_REMOTE_CALL_DECLINE);
        sMtcReasonToImsReason.put(CallReasonInfo.CODE_WIFI_LOST,
                ImsReasonInfo.CODE_WIFI_LOST);
        sMtcReasonToImsReason.put(CallReasonInfo.CODE_RADIO_OFF,
                ImsReasonInfo.CODE_RADIO_OFF);
        sMtcReasonToImsReason.put(CallReasonInfo.CODE_RADIO_INTERNAL_ERROR,
                ImsReasonInfo.CODE_RADIO_INTERNAL_ERROR);
        sMtcReasonToImsReason.put(CallReasonInfo.CODE_NETWORK_RESP_TIMEOUT,
                ImsReasonInfo.CODE_NETWORK_RESP_TIMEOUT);
        sMtcReasonToImsReason.put(CallReasonInfo.CODE_ACCESS_CLASS_BLOCKED,
                ImsReasonInfo.CODE_ACCESS_CLASS_BLOCKED);
        sMtcReasonToImsReason.put(CallReasonInfo.CODE_SIP_ALTERNATE_EMERGENCY_CALL,
                ImsReasonInfo.CODE_SIP_ALTERNATE_EMERGENCY_CALL);
        sMtcReasonToImsReason.put(CallReasonInfo.CODE_REJECT_UNKNOWN,
                ImsReasonInfo.CODE_REJECT_UNKNOWN);
        sMtcReasonToImsReason.put(CallReasonInfo.CODE_REJECT_ONGOING_CALL_WAITING_DISABLED,
                ImsReasonInfo.CODE_REJECT_ONGOING_CALL_WAITING_DISABLED);
        sMtcReasonToImsReason.put(CallReasonInfo.CODE_REJECT_CALL_ON_OTHER_SUB,
                ImsReasonInfo.CODE_REJECT_CALL_ON_OTHER_SUB);
        sMtcReasonToImsReason.put(CallReasonInfo.CODE_REJECT_SERVICE_NOT_REGISTERED,
                ImsReasonInfo.CODE_REJECT_SERVICE_NOT_REGISTERED);
        sMtcReasonToImsReason.put(CallReasonInfo.CODE_REJECT_CALL_TYPE_NOT_ALLOWED,
                ImsReasonInfo.CODE_REJECT_CALL_TYPE_NOT_ALLOWED);
        sMtcReasonToImsReason.put(CallReasonInfo.CODE_REJECT_ONGOING_E911_CALL,
                ImsReasonInfo.CODE_REJECT_ONGOING_E911_CALL);
        sMtcReasonToImsReason.put(CallReasonInfo.CODE_REJECT_ONGOING_CALL_SETUP,
                ImsReasonInfo.CODE_REJECT_ONGOING_CALL_SETUP);
        sMtcReasonToImsReason.put(CallReasonInfo.CODE_REJECT_MAX_CALL_LIMIT_REACHED,
                ImsReasonInfo.CODE_REJECT_MAX_CALL_LIMIT_REACHED);
        sMtcReasonToImsReason.put(CallReasonInfo.CODE_REJECT_UNSUPPORTED_SIP_HEADERS,
                ImsReasonInfo.CODE_REJECT_UNSUPPORTED_SIP_HEADERS);
        sMtcReasonToImsReason.put(CallReasonInfo.CODE_REJECT_UNSUPPORTED_SDP_HEADERS,
                ImsReasonInfo.CODE_REJECT_UNSUPPORTED_SDP_HEADERS);
        sMtcReasonToImsReason.put(CallReasonInfo.CODE_REJECT_ONGOING_CALL_TRANSFER,
                ImsReasonInfo.CODE_REJECT_ONGOING_CALL_TRANSFER);
        sMtcReasonToImsReason.put(CallReasonInfo.CODE_REJECT_INTERNAL_ERROR,
                ImsReasonInfo.CODE_REJECT_INTERNAL_ERROR);
        sMtcReasonToImsReason.put(CallReasonInfo.CODE_REJECT_QOS_FAILURE,
                ImsReasonInfo.CODE_REJECT_QOS_FAILURE);
        sMtcReasonToImsReason.put(CallReasonInfo.CODE_REJECT_VT_TTY_NOT_ALLOWED,
                ImsReasonInfo.CODE_REJECT_VT_TTY_NOT_ALLOWED);
        sMtcReasonToImsReason.put(CallReasonInfo.CODE_REJECT_ONGOING_CALL_UPGRADE,
                ImsReasonInfo.CODE_REJECT_ONGOING_CALL_UPGRADE);
        sMtcReasonToImsReason.put(CallReasonInfo.CODE_REJECT_ONGOING_CONFERENCE_CALL,
                ImsReasonInfo.CODE_REJECT_ONGOING_CONFERENCE_CALL);
        sMtcReasonToImsReason.put(CallReasonInfo.CODE_REJECT_ONGOING_CS_CALL,
                ImsReasonInfo.CODE_REJECT_ONGOING_CS_CALL);
        sMtcReasonToImsReason.put(CallReasonInfo.CODE_EMERGENCY_CALL_OVER_WFC_NOT_AVAILABLE,
                ImsReasonInfo.CODE_EMERGENCY_CALL_OVER_WFC_NOT_AVAILABLE);
        sMtcReasonToImsReason.put(CallReasonInfo.CODE_OEM_CAUSE_3, ImsReasonInfo.CODE_OEM_CAUSE_3);

        // User's status: from user status (int) to user status (string)
        sUserStatusToString = new LinkedHashMap<Integer, String>();
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
