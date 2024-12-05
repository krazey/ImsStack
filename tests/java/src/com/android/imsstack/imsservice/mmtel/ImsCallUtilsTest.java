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

import static org.junit.Assert.assertEquals;
import static org.junit.Assert.assertFalse;
import static org.junit.Assert.assertNotNull;
import static org.junit.Assert.assertTrue;
import static org.mockito.Mockito.when;

import android.content.Context;
import android.os.Bundle;
import android.telephony.CarrierConfigManager;
import android.telephony.TelephonyManager;
import android.telephony.emergency.EmergencyNumber;
import android.telephony.ims.ImsCallProfile;
import android.telephony.ims.ImsConferenceState;
import android.telephony.ims.ImsReasonInfo;
import android.telephony.ims.ImsStreamMediaProfile;


import com.android.imsstack.core.agents.AgentFactory;
import com.android.imsstack.core.agents.ConfigInterface;
import com.android.imsstack.core.config.CarrierConfig;
import com.android.imsstack.enabler.mtc.CallInfo;
import com.android.imsstack.enabler.mtc.CallReasonInfo;
import com.android.imsstack.enabler.mtc.IUMtcCall;
import com.android.imsstack.enabler.mtc.MediaInfo;
import com.android.imsstack.enabler.mtc.MtcCallInfo;
import com.android.imsstack.enabler.mtc.SuppInfo;
import com.android.imsstack.enabler.mtc.conf.UsersInfo;
import com.android.imsstack.imsservice.mmtel.base.ICallContext;

import org.junit.After;
import org.junit.Assert;
import org.junit.Before;
import org.junit.Test;
import org.mockito.Mock;
import org.mockito.Mockito;
import org.mockito.MockitoAnnotations;

import java.util.ArrayList;
import java.util.List;

public class ImsCallUtilsTest {

    /** "sos" URN for IMS emergency call */
    private static final String SOS_SERVICE_URN_POLICE = "urn:service:sos.police";
    private static final String SOS_SERVICE_URN_AMBULANCE = "urn:service:sos.ambulance";
    private static final String SOS_SERVICE_URN_FIRE = "urn:service:sos.fire";
    private static final String SOS_SERVICE_URN_MARINE = "urn:service:sos.marine";
    private static final String SOS_SERVICE_URN_MOUNTAIN = "urn:service:sos.mountain";
    private static final String SOS_SERVICE_URN_GENERIC = "urn:service:sos";
    public static final String EXTRA_CONFERENCE_USER_ID = "conference_user_id";
    public static final String ANONYMOUS = "anonymous";
    private static final int SLOT_ID = 0;

    //Mocked classes
    @Mock CarrierConfig mMockCarrierConfig;
    @Mock ConfigInterface mMockConfigInterface;
    @Mock ICallContext mContext;
    @Mock Context mMockContext;

    @Before
    public void setUp() {
        MockitoAnnotations.initMocks(this);
        when(mMockConfigInterface.getCarrierConfig()).thenReturn(mMockCarrierConfig);
        AgentFactory.getInstance().setAgent(ConfigInterface.class, mMockConfigInterface, SLOT_ID);
    }

    @After
    public void tearDown() throws Exception {
        AgentFactory.getInstance().setAgent(ConfigInterface.class, null, SLOT_ID);
    }

    @Test
    public void testCreateCallProfileFromCallInfo() {
        CallInfo callInfo = Mockito.mock(CallInfo.class);
        MediaInfo mediaInfo = Mockito.mock(MediaInfo.class);

        callInfo.videoCapable = true;
        when(mMockCarrierConfig.getBoolean(CarrierConfigManager.KEY_RTT_SUPPORTED_BOOL, false))
                .thenReturn(true);
        ImsCallProfile profile = ImsCallUtils.createCallProfileFromCallInfo(mContext,
                callInfo, mediaInfo);
        assertEquals(ImsCallProfile.CALL_RESTRICT_CAUSE_HD, profile.getRestrictCause());
        assertEquals(true, profile.getCallExtraBoolean(ImsCallProfile.EXTRA_CALL_MODE_CHANGEABLE));
        assertEquals(false, profile.getCallExtraBoolean(ImsCallUtils.EXTRA_RTT_AVAIL));

        callInfo.isConf = true;
        mediaInfo.AQuality = MediaInfo.AUDIO_QUALITY_EVS_SWB;
        profile = ImsCallUtils.createCallProfileFromCallInfo(mContext, callInfo, mediaInfo);
        assertEquals(ImsCallProfile.CALL_RESTRICT_CAUSE_NONE, profile.getRestrictCause());
        assertEquals(true, profile.getCallExtraBoolean(ImsCallProfile.EXTRA_CONFERENCE));

        callInfo.serviceType = ImsCallProfile.SERVICE_TYPE_NORMAL;
        profile = ImsCallUtils.createCallProfileFromCallInfo(mContext,
            callInfo, mediaInfo);
        assertEquals(ImsCallProfile.SERVICE_TYPE_NORMAL, profile.mServiceType);

        callInfo.callType = ImsCallProfile.CALL_TYPE_VT;
        callInfo.serviceType = ImsCallProfile.SERVICE_TYPE_EMERGENCY;
        profile = ImsCallUtils.createCallProfileFromCallInfo(mContext, callInfo, mediaInfo);
        assertEquals(ImsCallProfile.CALL_TYPE_VT, profile.mCallType);
        assertEquals(ImsCallProfile.SERVICE_TYPE_EMERGENCY, profile.mServiceType);

        mediaInfo.VDir = MediaInfo.DIRECTION_INACTIVE;
        profile = ImsCallUtils.createCallProfileFromCallInfo(mContext, callInfo, mediaInfo);
        assertEquals(ImsStreamMediaProfile.DIRECTION_INVALID,
                profile.getMediaProfile().mVideoDirection);
    }

    @Test
    public void testCloneCallProfile() {
        int serviceType = IUMtcCall.SERVICETYPE_NORMAL;
        int callType = ImsCallProfile.CALL_TYPE_VOICE;
        int audioQuality = ImsStreamMediaProfile.AUDIO_QUALITY_QCELP13K;
        int audioDirection = ImsStreamMediaProfile.DIRECTION_SEND_RECEIVE;
        int videoQuality = ImsStreamMediaProfile.VIDEO_QUALITY_QCIF;
        int videoDirection =  ImsStreamMediaProfile.DIRECTION_SEND_RECEIVE;

        ImsCallProfile profile = ImsCallUtils.createCallProfile(
                serviceType, callType, audioQuality, audioDirection, videoQuality, videoDirection);
        assertNotNull(profile);

        ImsCallProfile clone = ImsCallUtils.cloneCallProfile(profile);
        assertNotNull(clone);
    }

    @Test
    public void testCreateCallProfile() {
        int serviceType = IUMtcCall.SERVICETYPE_NORMAL;
        int callType = ImsCallProfile.CALL_TYPE_VOICE;
        int audioQuality = ImsStreamMediaProfile.AUDIO_QUALITY_NONE;
        int audioDirection = ImsStreamMediaProfile.DIRECTION_SEND_RECEIVE;
        int videoQuality = ImsStreamMediaProfile.VIDEO_QUALITY_NONE;
        int videoDirection =  ImsStreamMediaProfile.DIRECTION_INVALID;
        int rttMode = ImsStreamMediaProfile.RTT_MODE_DISABLED;
        ImsCallProfile actualProfile = new ImsCallProfile(serviceType, callType);
        ImsCallProfile expectedprofile = ImsCallUtils.createCallProfile(
                serviceType, callType, audioQuality, audioDirection, videoQuality, videoDirection,
                    rttMode);
        assertEquals(actualProfile.mServiceType, expectedprofile.mServiceType);
        assertEquals(actualProfile.mCallType, expectedprofile.mCallType);
    }

    @Test
    public void testCreateConferenceParticipant() {
        String user = "sip:anonymousX@example.com";
        String endpoint = "full";
        String displayText = "display text";
        String status = "connected";
        int sipStatusCode = 200;
        int disconnectedCause = 0;

        Bundle bundle = ImsCallUtils.createConferenceParticipant(user, endpoint, displayText,
                status, sipStatusCode, disconnectedCause);

        assertEquals(bundle.getString(ImsConferenceState.STATUS), status);
        assertEquals(bundle.getString(ImsConferenceState.USER), user);
        assertEquals(bundle.getString(ImsConferenceState.ENDPOINT), endpoint);
        assertEquals(bundle.getString(ImsConferenceState.DISPLAY_TEXT), displayText);
        assertEquals(bundle.getInt(ImsConferenceState.SIP_STATUS_CODE), sipStatusCode);
    }

    @Test
    public void testCreateReasonInfo() {
        CallReasonInfo info = new CallReasonInfo();
        ImsReasonInfo reasonInfoExtra = ImsCallUtils.createReasonInfo(info,
                ImsCallUtils.FLAG_REASON_INFO_EXTRA_CODE);
        assertNotNull(reasonInfoExtra);
    }

    @Test
    public void testCreateSuppInfoFromCallProfile() {
        ImsCallProfile profile = new ImsCallProfile(ImsCallProfile.SERVICE_TYPE_EMERGENCY,
                ImsCallProfile.CALL_TYPE_VOICE);
        List<String> urns = new ArrayList<String>();
        /*
        * verify SOS URN for IMS emergency call
        * Case 1:- urns is empty and ESCV is EMERGENCY_SERVICE_CATEGORY_POLICE
        */
        profile.setEmergencyUrns(urns);
        profile.setEmergencyServiceCategories(1);
        SuppInfo suppInfo = ImsCallUtils.createSuppInfoFromCallProfile(mContext, profile);
        assertEquals(1, suppInfo.getServiceSize());
        assertNotNull(suppInfo.getService(SuppInfo.TYPE_TARGET_URI));
        assertEquals(SOS_SERVICE_URN_POLICE,
                suppInfo.getService(SuppInfo.TYPE_TARGET_URI).strValue);

        //Case 2:- urns is empty and ESCV is UNSPECIFIED
        profile.setEmergencyServiceCategories(0);
        suppInfo = ImsCallUtils.createSuppInfoFromCallProfile(mContext, profile);
        assertEquals(1, suppInfo.getServiceSize());
        assertNotNull(suppInfo.getService(SuppInfo.TYPE_TARGET_URI));
        assertEquals(SOS_SERVICE_URN_GENERIC,
                suppInfo.getService(SuppInfo.TYPE_TARGET_URI).strValue);

        //Case 3:- urns is not empty
        urns.add(SOS_SERVICE_URN_AMBULANCE);
        profile.setEmergencyUrns(urns);
        suppInfo = ImsCallUtils.createSuppInfoFromCallProfile(mContext, profile);
        assertEquals(1, suppInfo.getServiceSize());
        assertNotNull(suppInfo.getService(SuppInfo.TYPE_TARGET_URI));
        assertEquals(SOS_SERVICE_URN_AMBULANCE,
                suppInfo.getService(SuppInfo.TYPE_TARGET_URI).strValue);

        //verify SuppInfo for TYPE_CALLERID, TYPE_CNAP and TYPE_CALL_PULL
        profile = new ImsCallProfile();
        profile.setCallExtraInt(ImsCallProfile.EXTRA_OIR, 1);
        profile.setCallExtraInt(ImsCallProfile.EXTRA_OIR, ImsCallProfile.OIR_DEFAULT);
        profile.setCallExtraInt(ImsCallProfile.EXTRA_CNAP,
                ImsCallProfile.OIR_PRESENTATION_NOT_RESTRICTED);
        profile.setCallExtra(ImsCallProfile.EXTRA_CNA, "UNKNOWN");
        profile.setCallExtraBoolean(ImsCallProfile.EXTRA_IS_CALL_PULL, true);
        suppInfo = ImsCallUtils.createSuppInfoFromCallProfile(mContext, profile);
        assertEquals(3, suppInfo.getServiceSize());
        assertNotNull(suppInfo.getService(SuppInfo.TYPE_CALLERID));
        assertNotNull(suppInfo.getService(SuppInfo.TYPE_CNAP));
        assertNotNull(suppInfo.getService(SuppInfo.TYPE_CALL_PULL));
    }

    @Test
    public void testGetCallExtraFromOemExtras() {
        int serviceType = IUMtcCall.SERVICETYPE_NORMAL;
        int callType = ImsCallProfile.CALL_TYPE_VOICE;
        int audioQuality = ImsStreamMediaProfile.AUDIO_QUALITY_QCELP13K;
        int audioDirection = ImsStreamMediaProfile.DIRECTION_SEND_RECEIVE;
        int videoQuality = ImsStreamMediaProfile.VIDEO_QUALITY_QCIF;
        int videoDirection =  ImsStreamMediaProfile.DIRECTION_SEND_RECEIVE;

        ImsCallProfile profile = ImsCallUtils.createCallProfile(
                serviceType, callType, audioQuality, audioDirection, videoQuality, videoDirection);
        assertNotNull(profile);

        profile.setCallExtraBoolean(ImsCallProfile.EXTRA_EXTENDING_TO_CONFERENCE_SUPPORTED, true);
        profile.setCallExtraInt(ImsCallProfile.EXTRA_OIR, ImsCallProfile.OIR_DEFAULT);
        profile.setCallExtraInt(ImsCallProfile.EXTRA_CNAP, ImsCallProfile.OIR_DEFAULT);
        profile.setCallExtraInt(ImsCallProfile.EXTRA_DIALSTRING, ImsCallProfile.DIALSTRING_NORMAL);
        profile.setCallExtraInt(ImsCallProfile.EXTRA_RETRY_CALL_FAIL_REASON,
                ImsReasonInfo.CODE_SIP_BUSY);
        profile.setCallExtraInt(ImsCallProfile.EXTRA_RETRY_CALL_FAIL_NETWORKTYPE,
                TelephonyManager.NETWORK_TYPE_LTE);
        profile.setCallExtraInt(ImsCallProfile.EXTRA_PRIORITY, ImsCallProfile.PRIORITY_URGENT);
        profile.setCallExtra(ImsCallProfile.EXTRA_CALL_SUBJECT, "CALL SUBJECT1");

        String key = "EXTRA_CALL_SUBJECT";
        String defaultValue = "CALL SUBJECT";
        String extraValue = ImsCallUtils.getCallExtraFromOemExtras(profile, key, defaultValue);
        assertEquals(extraValue, defaultValue);

        extraValue = ImsCallUtils.getCallExtraFromOemExtras(profile, key, defaultValue);
        assertEquals(extraValue, defaultValue);

        key = ImsCallProfile.EXTRA_RETRY_CALL_FAIL_NETWORKTYPE;
        extraValue = ImsCallUtils.getCallExtraFromOemExtras(profile, key, defaultValue);
        assertEquals(extraValue, defaultValue);

        key = ImsCallProfile.EXTRA_CNA;
        extraValue = ImsCallUtils.getCallExtraFromOemExtras(profile, key, defaultValue);
        assertEquals(extraValue, defaultValue);

        key = ImsCallProfile.EXTRA_RETRY_CALL_FAIL_REASON;
        extraValue = ImsCallUtils.getCallExtraFromOemExtras(profile, key, defaultValue);
        assertEquals(extraValue, defaultValue);

        key = ImsCallProfile.EXTRA_EXTENDING_TO_CONFERENCE_SUPPORTED;
        extraValue = ImsCallUtils.getCallExtraFromOemExtras(profile, key, defaultValue);
        assertEquals(extraValue, defaultValue);

        key = ImsCallProfile.EXTRA_OIR;
        extraValue = ImsCallUtils.getCallExtraFromOemExtras(profile, key, defaultValue);
        assertEquals(extraValue, defaultValue);

        key = ImsCallProfile.EXTRA_DIALSTRING;
        extraValue = ImsCallUtils.getCallExtraFromOemExtras(profile, key, defaultValue);
        assertEquals(extraValue, defaultValue);
    }

    @Test
    public void testGetCallExtraBooleanFromOemExtras() {
        ImsCallProfile profile = new ImsCallProfile();

        String key = "EXTRA_CALL_BOOL";
        boolean defaultValue = false;
        Bundle extras = new Bundle();
        extras.putBoolean(key, true);
        profile.mCallExtras.putBundle(ImsCallProfile.EXTRA_OEM_EXTRAS, extras);
        assertTrue(ImsCallUtils.getCallExtraBooleanFromOemExtras(profile, key, defaultValue));

        defaultValue = true;
        extras.putBoolean(key, false);
        profile.mCallExtras.putBundle(ImsCallProfile.EXTRA_OEM_EXTRAS, extras);
        assertFalse(ImsCallUtils.getCallExtraBooleanFromOemExtras(profile, key, defaultValue));
    }

    @Test
    public void testGetCallTypeFromProfile() {
        int ret = ImsCallUtils.getCallTypeFromProfile(ImsCallProfile.CALL_TYPE_VOICE, true);
        assertEquals(IUMtcCall.CALLTYPE_RTT, ret);

        ret = ImsCallUtils.getCallTypeFromProfile(ImsCallProfile.CALL_TYPE_VOICE, false);
        assertEquals(IUMtcCall.CALLTYPE_VOIP, ret);

        ret = ImsCallUtils.getCallTypeFromProfile(ImsCallProfile.CALL_TYPE_VT, true);
        assertEquals(IUMtcCall.CALLTYPE_VIDEO_RTT, ret);

        ret = ImsCallUtils.getCallTypeFromProfile(ImsCallProfile.CALL_TYPE_VT, false);
        assertEquals(IUMtcCall.CALLTYPE_VT, ret);

        final int callTypeInvalid = 100;
        ret = ImsCallUtils.getCallTypeFromProfile(callTypeInvalid, false);
        assertEquals(IUMtcCall.CALLTYPE_VOIP, ret);
    }

    @Test
    public void testGetSosUrnFromECallServiceCategory() {
        int[] policies = {CarrierConfig.ImsEmergency.NOT_USE_SERVICE_CATEGORY};
        String ret = ImsCallUtils.getSosUrnFromECallServiceCategory(
                EmergencyNumber.EMERGENCY_SERVICE_CATEGORY_POLICE, policies);
        assertEquals(ret, SOS_SERVICE_URN_GENERIC);

        policies = new int[] {CarrierConfig.ImsEmergency.USE_POLICE_FOR_UNSPECIFIED};
        ret = ImsCallUtils.getSosUrnFromECallServiceCategory(
                EmergencyNumber.EMERGENCY_SERVICE_CATEGORY_UNSPECIFIED, policies);
        assertEquals(ret, SOS_SERVICE_URN_POLICE);

        ret = ImsCallUtils.getSosUrnFromECallServiceCategory(
                EmergencyNumber.EMERGENCY_SERVICE_CATEGORY_UNSPECIFIED, null);
        assertEquals(ret, SOS_SERVICE_URN_GENERIC);

        policies = new int[] {CarrierConfig.ImsEmergency.USE_GENERIC_FOR_MULTIPLE_CATEGORIES};
        ret = ImsCallUtils.getSosUrnFromECallServiceCategory(
                EmergencyNumber.EMERGENCY_SERVICE_CATEGORY_POLICE
                | EmergencyNumber.EMERGENCY_SERVICE_CATEGORY_AMBULANCE, policies);
        assertEquals(ret, SOS_SERVICE_URN_GENERIC);

        ret = ImsCallUtils.getSosUrnFromECallServiceCategory(
                EmergencyNumber.EMERGENCY_SERVICE_CATEGORY_POLICE
                | EmergencyNumber.EMERGENCY_SERVICE_CATEGORY_AMBULANCE, null);
        assertEquals(ret, SOS_SERVICE_URN_POLICE);

        ret = ImsCallUtils.getSosUrnFromECallServiceCategory(
                EmergencyNumber.EMERGENCY_SERVICE_CATEGORY_POLICE, policies);
        assertEquals(ret, SOS_SERVICE_URN_POLICE);

        ret = ImsCallUtils.getSosUrnFromECallServiceCategory(
                EmergencyNumber.EMERGENCY_SERVICE_CATEGORY_AMBULANCE, policies);
        assertEquals(ret, SOS_SERVICE_URN_AMBULANCE);

        ret = ImsCallUtils.getSosUrnFromECallServiceCategory(
                EmergencyNumber.EMERGENCY_SERVICE_CATEGORY_FIRE_BRIGADE, policies);
        assertEquals(ret, SOS_SERVICE_URN_FIRE);

        ret = ImsCallUtils.getSosUrnFromECallServiceCategory(
                EmergencyNumber.EMERGENCY_SERVICE_CATEGORY_MARINE_GUARD, policies);
        assertEquals(ret, SOS_SERVICE_URN_MARINE);

        ret = ImsCallUtils.getSosUrnFromECallServiceCategory(
                EmergencyNumber.EMERGENCY_SERVICE_CATEGORY_MOUNTAIN_RESCUE, policies);
        assertEquals(ret, SOS_SERVICE_URN_MOUNTAIN);
    }

    @Test
    public void testGetDisconnectedCauseFromUserStatus() {
        assertEquals(UsersInfo.USER_STATUS_NOTSUPPORTED,
                ImsCallUtils.getDisconnectedCauseFromUserStatus(
                    UsersInfo.USER_STATUS_NOTSUPPORTED));
        assertEquals(0, ImsCallUtils.getDisconnectedCauseFromUserStatus(
                UsersInfo.USER_STATUS_FAIL));
    }

    @Test
    public void testGetExtraCodeFromMtc() {
        int reason = CallReasonInfo.CODE_LOCAL_CALL_CS_RETRY_REQUIRED;
        int extraCode = -1;
        int result = ImsCallUtils.getExtraCodeFromMtc(reason, extraCode);
        int resultCode = ImsReasonInfo.CODE_UNSPECIFIED;
        assertEquals(resultCode, result);

        reason = ImsReasonInfo.CODE_LOCAL_CALL_TERMINATED;
        result = ImsCallUtils.getExtraCodeFromMtc(reason, extraCode);
        assertEquals(resultCode, result);

        reason = ImsReasonInfo.CODE_LOCAL_CALL_TERMINATED;
        extraCode = CallReasonInfo.EXTRA_CODE_CALL_RETRY_SILENT_REDIAL;
        result = ImsCallUtils.getExtraCodeFromMtc(reason, extraCode);
        assertEquals(CallReasonInfo.EXTRA_CODE_CALL_RETRY_SILENT_REDIAL, result);

        reason = CallReasonInfo.CODE_LOCAL_CALL_CS_RETRY_REQUIRED;
        result = ImsCallUtils.getExtraCodeFromMtc(reason, extraCode);
        assertEquals(ImsReasonInfo.EXTRA_CODE_CALL_RETRY_SILENT_REDIAL, result);
    }

    @Test
    public void testGetProfileCallTypeFromCallInfo() {
        int result;
        boolean videoCapable = true;
        int profileCallType = ImsCallProfile.CALL_TYPE_VOICE;

        int callType = IUMtcCall.CALLTYPE_VOIP;
        result = ImsCallUtils.getProfileCallTypeFromCallInfo(videoCapable, callType);
        assertEquals(result, profileCallType);

        callType = IUMtcCall.CALLTYPE_RTT;
        result = ImsCallUtils.getProfileCallTypeFromCallInfo(videoCapable, callType);
        assertEquals(result, profileCallType);

        callType = IUMtcCall.CALLTYPE_VT;
        profileCallType = ImsCallProfile.CALL_TYPE_VT;
        result = ImsCallUtils.getProfileCallTypeFromCallInfo(videoCapable, callType);
        assertEquals(result, profileCallType);

        videoCapable = false;
        callType = IUMtcCall.CALLTYPE_VOIP;
        profileCallType = ImsCallProfile.CALL_TYPE_VOICE;
        result = ImsCallUtils.getProfileCallTypeFromCallInfo(videoCapable, callType);
        assertEquals(result, profileCallType);

        callType = IUMtcCall.CALLTYPE_RTT;
        result = ImsCallUtils.getProfileCallTypeFromCallInfo(videoCapable, callType);
        assertEquals(result, profileCallType);

        callType = IUMtcCall.CALLTYPE_VT;
        profileCallType = ImsCallProfile.CALL_TYPE_VT;
        result = ImsCallUtils.getProfileCallTypeFromCallInfo(videoCapable, callType);
        assertEquals(result, profileCallType);
    }

    @Test
    public void testGetTerminateCallReasonInfoCodeFromImsReasonInfo() {
        int result = ImsCallUtils.getTerminateCallReasonInfoCodeFromImsReasonInfo(
                ImsReasonInfo.CODE_USER_TERMINATED);
        assertEquals(result, CallReasonInfo.CODE_USER_TERMINATED);

        result = ImsCallUtils.getTerminateCallReasonInfoCodeFromImsReasonInfo(
                ImsReasonInfo.CODE_LOCAL_POWER_OFF);
        assertEquals(result, CallReasonInfo.CODE_LOCAL_POWER_OFF);

        result = ImsCallUtils.getTerminateCallReasonInfoCodeFromImsReasonInfo(
                ImsReasonInfo.CODE_LOW_BATTERY);
        assertEquals(result, CallReasonInfo.CODE_LOCAL_LOW_BATTERY);

        result = ImsCallUtils.getTerminateCallReasonInfoCodeFromImsReasonInfo(
                ImsReasonInfo.CODE_LOCAL_ILLEGAL_ARGUMENT);
        assertEquals(result, CallReasonInfo.CODE_UNSPECIFIED);
    }

    @Test
    public void testIsCsSilentRedialRequired() {
        ImsReasonInfo imsReasonInfo = new ImsReasonInfo(
                ImsReasonInfo.CODE_LOCAL_CALL_CS_RETRY_REQUIRED,
                ImsReasonInfo.EXTRA_CODE_CALL_RETRY_SILENT_REDIAL);
        assertTrue(ImsCallUtils.isCsSilentRedialRequired(imsReasonInfo));
    }

    @Test
    public void testIsEmergencyCall() {
        ImsCallProfile callProfile = new ImsCallProfile(ImsCallProfile.SERVICE_TYPE_EMERGENCY,
                ImsCallProfile.CALL_TYPE_VOICE);
        boolean ret = ImsCallUtils.isEmergencyCall(callProfile);
        assertTrue(ret);

        callProfile.mServiceType = ImsCallProfile.SERVICE_TYPE_NORMAL;
        ret = ImsCallUtils.isEmergencyCall(callProfile);
        assertFalse(ret);
    }

    @Test
    public void testIsAlternateEmergencyCall() {
        CallReasonInfo info = Mockito.mock(CallReasonInfo.class);
        info.mCode = ImsReasonInfo.CODE_SIP_ALTERNATE_EMERGENCY_CALL;
        boolean ret = ImsCallUtils.isAlternateEmergencyCall(info);
        assertTrue(ret);

        info.mCode = ImsReasonInfo.CODE_UNSPECIFIED;
        ret = ImsCallUtils.isAlternateEmergencyCall(info);
        assertFalse(ret);
    }

    @Test
    public void testIsHoldMediaOnVideoCall() {
        boolean ret = ImsCallUtils.isHoldMediaOnVideoCall(mContext, null, null, null);
        assertFalse(ret);

        ret = ImsCallUtils.isHoldMediaOnVideoCall(mContext, new ImsCallProfile(), new CallInfo(),
                new MediaInfo());
        assertFalse(ret);
    }

    @Test
    public void testIsUnholdMediaOnVideoCall() {
        boolean ret = ImsCallUtils.isUnholdMediaOnVideoCall(mContext, null, null, null);
        assertFalse(ret);

        ret = ImsCallUtils.isUnholdMediaOnVideoCall(mContext, new ImsCallProfile(), new CallInfo(),
                new MediaInfo());
        assertFalse(ret);
    }

    @Test
    public void testIsVideoCall() {
        boolean ret = ImsCallUtils.isVideoCall(ImsCallProfile.CALL_TYPE_VIDEO_N_VOICE);
        assertTrue(ret);

        ret = ImsCallUtils.isVideoCall(ImsCallProfile.CALL_TYPE_VT);
        assertTrue(ret);

        ret = ImsCallUtils.isVideoCall(ImsCallProfile.CALL_TYPE_VT_TX);
        assertTrue(ret);

        ret = ImsCallUtils.isVideoCall(ImsCallProfile.CALL_TYPE_VT_RX);
        assertTrue(ret);

        ret = ImsCallUtils.isVideoCall(ImsCallProfile.CALL_TYPE_VT_NODIR);
        assertTrue(ret);
    }

    @Test
    public void testIsVoiceCall() {
        boolean ret = ImsCallUtils.isVoiceCall(ImsCallProfile.CALL_TYPE_VOICE_N_VIDEO);
        assertTrue(ret);

        ret = ImsCallUtils.isVoiceCall(ImsCallProfile.CALL_TYPE_VOICE);
        assertTrue(ret);
    }

    @Test
    public void testIsCallTypeChanged() {
        int callType = ImsCallProfile.CALL_TYPE_VOICE;
        int otherCallTyp = ImsCallProfile.CALL_TYPE_VT;
        boolean ret = ImsCallUtils.isCallTypeChanged(callType, otherCallTyp);
        assertTrue(ret);

        ret = ImsCallUtils.isCallTypeChanged(null, null);
        assertFalse(ret);

        ImsCallProfile profile = new ImsCallProfile();
        profile.mCallType = ImsCallProfile.CALL_TYPE_VOICE;
        CallInfo ci = new CallInfo();
        ci.callType = IUMtcCall.CALLTYPE_VT;
        ret = ImsCallUtils.isCallTypeChanged(profile, ci);
        assertTrue(ret);
    }

    @Test
    public void testIsVideoProfileChanged() {
        boolean ret = ImsCallUtils.isVideoProfileChanged(null, null, null);
        assertFalse(ret);

        ret = ImsCallUtils.isVideoProfileChanged(new ImsCallProfile(), new CallInfo(),
                new MediaInfo());
        assertFalse(ret);

        ImsCallProfile outProfile = new ImsCallProfile();
        outProfile.mCallType = ImsCallProfile.CALL_TYPE_VIDEO_N_VOICE;

        CallInfo ci = new CallInfo();
        ci.serviceType = IUMtcCall.SERVICETYPE_EMERGENCY;
        ci.isConf = true;
        ci.videoCapable = true;
        ci.confSub = true;
        ci.rttCapable = true;

        MediaInfo mMediaProfile = new MediaInfo();
        mMediaProfile.ADir = 3;
        mMediaProfile.VQuality = MediaInfo.VIDEO_QUALITY_QCIF;

        outProfile.mMediaProfile = new ImsStreamMediaProfile();
        outProfile.mMediaProfile.mVideoQuality = MediaInfo.VIDEO_QUALITY_QCIF;
        outProfile.mMediaProfile.mVideoDirection = 3;

        ret = ImsCallUtils.isVideoProfileChanged(outProfile, ci, mMediaProfile);
        assertFalse(ret);
    }

    @Test
    public void testUpdateCallProfileForEmergency() {
        ImsCallProfile profile = new ImsCallProfile();
        profile.mCallType = ImsCallProfile.CALL_TYPE_VOICE;
        CallInfo ci = new CallInfo();
        ImsCallUtils.updateCallProfileForEmergency(profile, ci);
        assertFalse(profile.getCallExtraBoolean(ImsCallProfile.EXTRA_EMERGENCY_CALL));

        ci.serviceType = IUMtcCall.SERVICETYPE_EMERGENCY;
        ImsCallUtils.updateCallProfileForEmergency(profile, ci);
        assertTrue(profile.getCallExtraBoolean(ImsCallProfile.EXTRA_EMERGENCY_CALL));
    }

    @Test
    public void testUpdateCallProfileFromCallInfo() {
        ImsCallProfile profile = new ImsCallProfile();
        CallInfo ci = new CallInfo();
        ImsCallUtils.updateCallProfileFromCallInfo(mContext, profile, ci);

        when(mMockCarrierConfig.getBoolean(CarrierConfigManager.KEY_RTT_SUPPORTED_BOOL, false))
                .thenReturn(true);
        assertFalse(profile.getCallExtraBoolean(ImsCallProfile.EXTRA_CONFERENCE));
        assertFalse(profile.getCallExtraBoolean(ImsCallProfile.EXTRA_CALL_MODE_CHANGEABLE));
        assertEquals("false", profile.getCallExtra(ImsCallProfile.EXTRA_USSD));
        assertFalse(profile.getCallExtraBoolean(ImsCallUtils.EXTRA_RTT_AVAIL));

        MtcCallInfo.setConference(ci, true);
        MtcCallInfo.setVideoCapable(ci, true);
        MtcCallInfo.setUssi(ci, true);
        MtcCallInfo.setRttCapable(ci, true);
        ImsCallUtils.updateCallProfileFromCallInfo(mContext, profile, ci);
        assertTrue(profile.getCallExtraBoolean(ImsCallProfile.EXTRA_CONFERENCE));
        assertTrue(profile.getCallExtraBoolean(ImsCallProfile.EXTRA_CALL_MODE_CHANGEABLE));
        assertEquals("true", profile.getCallExtra(ImsCallProfile.EXTRA_USSD));
        assertTrue(profile.getCallExtraBoolean(ImsCallUtils.EXTRA_RTT_AVAIL));
    }

    @Test
    public void testSetCallExtraIfPresent() {
        ImsCallProfile outProfile = new ImsCallProfile();
        int serviceType = IUMtcCall.SERVICETYPE_NORMAL;
        int callType = ImsCallProfile.CALL_TYPE_VOICE;
        int audioQuality = ImsStreamMediaProfile.AUDIO_QUALITY_QCELP13K;
        int audioDirection = ImsStreamMediaProfile.DIRECTION_SEND_RECEIVE;
        int videoQuality = ImsStreamMediaProfile.VIDEO_QUALITY_QCIF;
        int videoDirection =  ImsStreamMediaProfile.DIRECTION_SEND_RECEIVE;
        ImsCallProfile profile = ImsCallUtils.createCallProfile(
                serviceType, callType, audioQuality, audioDirection, videoQuality, videoDirection);
        assertNotNull(profile);

        profile.setCallExtraInt(ImsCallProfile.EXTRA_OIR, ImsCallProfile.OIR_DEFAULT);
        profile.setCallExtraInt(ImsCallProfile.EXTRA_CNAP, ImsCallProfile.OIR_DEFAULT);
        profile.setCallExtraInt(ImsCallProfile.EXTRA_DIALSTRING, ImsCallProfile.DIALSTRING_NORMAL);
        profile.setCallExtraInt(ImsCallProfile.EXTRA_RETRY_CALL_FAIL_REASON,
                ImsReasonInfo.CODE_SIP_BUSY);
        profile.setCallExtraInt(ImsCallProfile.EXTRA_RETRY_CALL_FAIL_NETWORKTYPE,
                TelephonyManager.NETWORK_TYPE_LTE);
        profile.setCallExtraInt(ImsCallProfile.EXTRA_PRIORITY, ImsCallProfile.PRIORITY_URGENT);
        profile.setCallExtra(ImsCallProfile.EXTRA_CALL_SUBJECT, "CALL SUBJECT1");
        profile.setCallExtraInt(ImsCallProfile.EXTRA_CNA, ImsCallProfile.OIR_PRESENTATION_UNKNOWN);

        profile.setCallExtraInt(ImsCallProfile.EXTRA_OIR, ImsCallProfile.OIR_DEFAULT);
        ImsCallUtils.setCallExtraIfPresent(profile, ImsCallProfile.EXTRA_OIR,
                ImsCallUtils.VAR_TYPE_INT, outProfile);

        assertTrue(outProfile.getCallExtraInt(ImsCallProfile.EXTRA_OIR) == 0);

        profile.setCallExtraBoolean(ImsCallProfile.EXTRA_CALL_MODE_CHANGEABLE, true);
        ImsCallUtils.setCallExtraIfPresent(profile, ImsCallProfile.EXTRA_CALL_MODE_CHANGEABLE,
                ImsCallUtils.VAR_TYPE_BOOLEAN, outProfile);

        assertTrue(outProfile.getCallExtraBoolean(ImsCallProfile.EXTRA_CALL_MODE_CHANGEABLE));

        profile.setCallExtra(EXTRA_CONFERENCE_USER_ID, ANONYMOUS);
        ImsCallUtils.setCallExtraIfPresent(profile, ImsCallProfile.EXTRA_CNA,
                ImsCallUtils.VAR_TYPE_STRING, outProfile);

        assertNotNull(outProfile.getCallExtra(EXTRA_CONFERENCE_USER_ID, ANONYMOUS));
    }

    @Test
    public void testUpdateCallProfileFromSuppInfoExtension() {
        ImsCallProfile profile = new ImsCallProfile();
        SuppInfo suppInfo = new SuppInfo();
        suppInfo.addService_int(SuppInfo.TYPE_CDIV_CAUSE, 1);
        when(mMockCarrierConfig.getBoolean(
                CarrierConfig.ImsVoice.KEY_SUPPINFO_CDIV_CAUSE_REQUIRED_BOOL)).thenReturn(true);
        ImsCallUtils.updateCallProfileFromSuppInfoExtension(mContext, profile, suppInfo);
        assertEquals(1, profile.getCallExtraInt(ImsCallUtils.EXTRA_CDIV_CAUSE));
    }

    @Test
    public void testUpdateCallProfileOnSessionProgressing() {
        ImsCallProfile profile = new ImsCallProfile();
        profile.mCallType = ImsCallProfile.CALL_TYPE_VOICE;
        SuppInfo suppInfo = new SuppInfo();
        SuppInfo suppInfo2 = new SuppInfo(suppInfo);
        CallInfo ci = new CallInfo();
        ci.serviceType = IUMtcCall.SERVICETYPE_EMERGENCY;
        ci.isConf = true;
        ci.videoCapable = true;
        ci.confSub = true;
        ci.rttCapable = true;
        suppInfo2.addService_bool(SuppInfo.TYPE_MMC, true);
        ImsCallUtils.updateCallProfileOnSessionProgressing(mContext, profile, ci, suppInfo2);
        Assert.assertEquals(ImsCallProfile.CALL_TYPE_VOICE, profile.getCallType());

        ci.callType = IUMtcCall.CALLTYPE_VT;
        suppInfo.addService(SuppInfo.TYPE_VRBT, false, SuppInfo.TIP_IDENTITY,
                "Testing Demo,sip:+12345678902");
        ImsCallUtils.updateCallProfileOnSessionProgressing(mContext, profile, ci, suppInfo);
        assertEquals(ImsCallProfile.CALL_TYPE_VT, profile.getCallType());
    }

    @Test
    public void testGetStringFromUserStatus() {
        String str = ImsCallUtils.getStringFromUserStatus(UsersInfo.USER_STATUS_PROGRESSING);
        assertNotNull(str);
    }

    @Test
    public void testGetReasonFromMTC() {
        int ret = ImsCallUtils.getReasonFromMTC(
                CallReasonInfo.CODE_USER_TERMINATED_BY_REMOTE);
        Assert.assertEquals(ImsReasonInfo.CODE_USER_TERMINATED_BY_REMOTE, ret);
    }

    @Test
    public void testGetRejectCallReasonInfoCodeFromImsReasonInfo() {
        int result = ImsCallUtils.getRejectCallReasonInfoCodeFromImsReasonInfo(
                ImsReasonInfo.CODE_LOCAL_CALL_EXCEEDED);
        assertEquals(result, CallReasonInfo.CODE_LOCAL_CALL_EXCEEDED);

        result = ImsCallUtils.getRejectCallReasonInfoCodeFromImsReasonInfo(
                ImsReasonInfo.CODE_LOCAL_CALL_BUSY);
        assertEquals(result, CallReasonInfo.CODE_LOCAL_CALL_BUSY);

        result = ImsCallUtils.getRejectCallReasonInfoCodeFromImsReasonInfo(
                ImsReasonInfo.CODE_USER_NOANSWER);
        assertEquals(result, CallReasonInfo.CODE_USER_NOANSWER);

        result = ImsCallUtils.getRejectCallReasonInfoCodeFromImsReasonInfo(
                ImsReasonInfo.CODE_USER_DECLINE);
        assertEquals(result, CallReasonInfo.CODE_USER_DECLINE);

        result = ImsCallUtils.getRejectCallReasonInfoCodeFromImsReasonInfo(
                ImsReasonInfo.CODE_LOW_BATTERY);
        assertEquals(result, CallReasonInfo.CODE_LOW_BATTERY);

        result = ImsCallUtils.getRejectCallReasonInfoCodeFromImsReasonInfo(
                ImsReasonInfo.CODE_USER_IGNORE);
        assertEquals(result, CallReasonInfo.CODE_USER_IGNORE);

        result = ImsCallUtils.getRejectCallReasonInfoCodeFromImsReasonInfo(
                ImsReasonInfo.CODE_USER_REJECTED_SESSION_MODIFICATION);
        assertEquals(result, CallReasonInfo.CODE_USER_REJECTED_SESSION_MODIFICATION);

        result = ImsCallUtils.getRejectCallReasonInfoCodeFromImsReasonInfo(
                ImsReasonInfo.CODE_LOCAL_ILLEGAL_ARGUMENT);
        assertEquals(result, CallReasonInfo.CODE_UNSPECIFIED);
    }

    @Test
    public void testIsEmergencyPdnUsedForEmergencyCallViaWfc() {
        Mockito.when(mContext.getContext()).thenReturn(mMockContext);
        boolean callViaEPdn = ImsCallUtils.isEmergencyPdnUsedForEmergencyCallViaWfc(mContext);
        assertFalse(callViaEPdn);
        // TODO: need to add a carrier configuration. method is not implemented
    }

    @Test
    public void testUpdateCallProfileOnSessionStarted() {
        ImsCallProfile profile = new ImsCallProfile();
        SuppInfo suppInfo = new SuppInfo();
        // added api for setting all the values for junit
        suppInfo.addService(SuppInfo.TYPE_TIP, false, SuppInfo.TIP_IDENTITY,
                "Testing Demo,sip:+12345678902");

        ImsCallUtils.updateCallProfileOnSessionStarted(profile, suppInfo);
        assertNotNull(suppInfo.getService(SuppInfo.TYPE_TIP));
        assertTrue(suppInfo.getService(SuppInfo.TYPE_TIP).intValue == 1);
        assertNotNull(suppInfo.getService(SuppInfo.TYPE_TIP));
        assertNotNull(profile.getCallExtra(ImsCallProfile.EXTRA_OI));
        assertNotNull(profile.getCallExtra(ImsCallProfile.EXTRA_CNA));
        assertTrue(profile.getCallExtraInt(ImsCallProfile.EXTRA_OIR)
                == ImsCallProfile.OIR_PRESENTATION_NOT_RESTRICTED);
        assertTrue(profile.getCallExtraInt(ImsCallProfile.EXTRA_CNAP)
                == ImsCallProfile.OIR_PRESENTATION_NOT_RESTRICTED);
        assertEquals("Testing Demo", profile.getCallExtra(ImsCallProfile.EXTRA_OI));
        assertEquals("sip:+12345678902", profile.getCallExtra(ImsCallProfile.EXTRA_CNA));
    }

    @Test
    public void testGetEmergencyRoutingFromCallProfile() {
        ImsCallProfile callProfile = new ImsCallProfile();
        callProfile.setEmergencyCallRouting(EmergencyNumber.EMERGENCY_CALL_ROUTING_UNKNOWN);
        assertEquals(EmergencyNumber.EMERGENCY_CALL_ROUTING_UNKNOWN,
                ImsCallUtils.getEmergencyRoutingFromCallProfile(callProfile));

        callProfile.setEmergencyCallRouting(EmergencyNumber.EMERGENCY_CALL_ROUTING_NORMAL);
        assertEquals(EmergencyNumber.EMERGENCY_CALL_ROUTING_NORMAL,
                ImsCallUtils.getEmergencyRoutingFromCallProfile(callProfile));
    }

    @Test
    public void testSetSosUrnFromCallReasonInfo() {
        List emergencyUrn;
        ImsCallProfile callProfile = new ImsCallProfile();
        ImsCallUtils.setSosUrnFromCallReasonInfo(CallReasonInfo.EXTRA_CODE_EMERGENCYSERVICE_POLICE,
                callProfile);
        assertEquals(EmergencyNumber.EMERGENCY_SERVICE_CATEGORY_POLICE,
                callProfile.getEmergencyServiceCategories());
        emergencyUrn = callProfile.getEmergencyUrns();
        assertEquals(SOS_SERVICE_URN_POLICE, emergencyUrn.get(0));

        ImsCallUtils.setSosUrnFromCallReasonInfo(
                CallReasonInfo.EXTRA_CODE_EMERGENCYSERVICE_AMBULANCE, callProfile);
        assertEquals(EmergencyNumber.EMERGENCY_SERVICE_CATEGORY_AMBULANCE,
                callProfile.getEmergencyServiceCategories());
        emergencyUrn = callProfile.getEmergencyUrns();
        assertEquals(SOS_SERVICE_URN_AMBULANCE, emergencyUrn.get(0));

        ImsCallUtils.setSosUrnFromCallReasonInfo(CallReasonInfo.EXTRA_CODE_EMERGENCYSERVICE_FIRE,
                callProfile);
        assertEquals(EmergencyNumber.EMERGENCY_SERVICE_CATEGORY_FIRE_BRIGADE,
                callProfile.getEmergencyServiceCategories());
        emergencyUrn = callProfile.getEmergencyUrns();
        assertEquals(SOS_SERVICE_URN_FIRE, emergencyUrn.get(0));

        ImsCallUtils.setSosUrnFromCallReasonInfo(CallReasonInfo.EXTRA_CODE_EMERGENCYSERVICE_MARINE,
                callProfile);
        assertEquals(EmergencyNumber.EMERGENCY_SERVICE_CATEGORY_MARINE_GUARD,
                callProfile.getEmergencyServiceCategories());
        emergencyUrn = callProfile.getEmergencyUrns();
        assertEquals(SOS_SERVICE_URN_MARINE, emergencyUrn.get(0));

        ImsCallUtils.setSosUrnFromCallReasonInfo(
                CallReasonInfo.EXTRA_CODE_EMERGENCYSERVICE_MOUNTAIN, callProfile);
        assertEquals(EmergencyNumber.EMERGENCY_SERVICE_CATEGORY_MOUNTAIN_RESCUE,
                callProfile.getEmergencyServiceCategories());
        emergencyUrn = callProfile.getEmergencyUrns();
        assertEquals(SOS_SERVICE_URN_MOUNTAIN, emergencyUrn.get(0));

        ImsCallUtils.setSosUrnFromCallReasonInfo(CallReasonInfo.EXTRA_CODE_EMERGENCYSERVICE_INVALID,
                callProfile);
        assertEquals(EmergencyNumber.EMERGENCY_SERVICE_CATEGORY_UNSPECIFIED,
                callProfile.getEmergencyServiceCategories());
        emergencyUrn = callProfile.getEmergencyUrns();
        assertEquals(SOS_SERVICE_URN_GENERIC, emergencyUrn.get(0));
    }
}
