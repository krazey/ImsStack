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

import android.content.Context;
import android.os.Bundle;
import android.telephony.TelephonyManager;
import android.telephony.emergency.EmergencyNumber;
import android.telephony.ims.ImsCallProfile;
import android.telephony.ims.ImsConferenceState;
import android.telephony.ims.ImsReasonInfo;
import android.telephony.ims.ImsStreamMediaProfile;

import com.android.imsstack.core.ImsGlobal;
import com.android.imsstack.enabler.mtc.CallInfo;
import com.android.imsstack.enabler.mtc.CallReasonInfo;
import com.android.imsstack.enabler.mtc.IUMtcCall;
import com.android.imsstack.enabler.mtc.MediaInfo;
import com.android.imsstack.enabler.mtc.SuppInfo;
import com.android.imsstack.enabler.mtc.conf.UsersInfo;
import com.android.imsstack.imsservice.mmtel.base.ICallContext;
import com.android.imsstack.util.ImsConstants;

import org.junit.After;
import org.junit.Assert;
import org.junit.Before;
import org.junit.Test;
import org.mockito.Mockito;

import java.util.Hashtable;

public class ImsCallUtilsTest {

    /** "sos" URN for IMS emergency call */
    private static final String SOS_SERVICE_URN_POLICE = "urn:service:sos.police";
    private static final String SOS_SERVICE_URN_AMBULANCE = "urn:service:sos.ambulance";
    private static final String SOS_SERVICE_URN_FIRE = "urn:service:sos.fire";
    private static final String SOS_SERVICE_URN_MARINE = "urn:service:sos.marine";
    private static final String SOS_SERVICE_URN_MOUNTAIN = "urn:service:sos.mountain";
    public static final String EXTRA_CONFERENCE_USER_ID = "conference_user_id";
    public static final String ANONYMOUS = "anonymous";

    private ICallContext mContext;
    private Context mMockContext;

    @Before
    public void setUp() {
        mContext = Mockito.mock(ICallContext.class);
        mMockContext = Mockito.mock(Context.class);
    }

    @After
    public void tearDown() {
        mContext = null;
        mMockContext = null;
    }

    @Test
    public void testProfileCreationClone() {
        int serviceType = IUMtcCall.SERVICETYPE_NORMAL;
        int callType = ImsCallProfile.CALL_TYPE_VOICE;
        int audioQuality = ImsStreamMediaProfile.AUDIO_QUALITY_QCELP13K;
        int audioDirection = ImsStreamMediaProfile.DIRECTION_SEND_RECEIVE;
        int videoQuality = ImsStreamMediaProfile.VIDEO_QUALITY_QCIF;
        int videoDirection =  ImsStreamMediaProfile.DIRECTION_SEND_RECEIVE;
        ImsCallProfile profile = null, clone = null;

        profile = ImsCallUtils.createCallProfile(
                serviceType, callType, audioQuality, audioDirection, videoQuality, videoDirection);
        assertNotNull(profile);

        clone = ImsCallUtils.cloneCallProfile(profile);
        assertNotNull(clone);
    }

    @Test
    public void testConfParticipantsBundle() {
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
    public void testReasonInfo() {
        CallReasonInfo info = new CallReasonInfo();
        ImsReasonInfo reasonInfoExtra = ImsCallUtils.createReasonInfo(info,
                ImsCallUtils.FLAG_REASON_INFO_EXTRA_CODE);
        assertNotNull(reasonInfoExtra);
    }

    @Test
    public void testCreateSuppInfoFromCallProfile() {
        ImsCallProfile profile = new ImsCallProfile();
        profile.setCallExtraInt(ImsCallProfile.EXTRA_OIR, 1);
        Bundle extras = new Bundle();
        extras.putBoolean(ImsCallProfile.EXTRA_IS_CALL_PULL, true);
        profile.mCallExtras.putBundle(ImsCallProfile.EXTRA_OEM_EXTRAS, extras);
        profile.setCallExtraInt(ImsCallProfile.EXTRA_DIALSTRING, ImsCallProfile.DIALSTRING_NORMAL);
        profile.setCallExtraInt(ImsCallProfile.EXTRA_OIR, ImsCallProfile.OIR_DEFAULT);
        profile.setCallExtraInt(ImsCallProfile.EXTRA_CNAP,
                ImsCallProfile.OIR_PRESENTATION_NOT_RESTRICTED);
        SuppInfo ret = ImsCallUtils.createSuppInfoFromCallProfile(mContext, profile);
        assertNotNull(ret);
    }

    @Test
    public void testGetCallExtraFromOemExtras() {
        int serviceType = IUMtcCall.SERVICETYPE_NORMAL;
        int callType = ImsCallProfile.CALL_TYPE_VOICE;
        int audioQuality = ImsStreamMediaProfile.AUDIO_QUALITY_QCELP13K;
        int audioDirection = ImsStreamMediaProfile.DIRECTION_SEND_RECEIVE;
        int videoQuality = ImsStreamMediaProfile.VIDEO_QUALITY_QCIF;
        int videoDirection =  ImsStreamMediaProfile.DIRECTION_SEND_RECEIVE;
        ImsCallProfile profile = null, clone = null;

        profile = ImsCallUtils.createCallProfile(
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
    public void testGetCalltypeFromProfile() {
        int ret = 0;
        ret = ImsCallUtils.getCallTypeFromProfile(ImsCallProfile.CALL_TYPE_VOICE, true);
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
    public void testSosUrn() {
        String ret = ImsCallUtils.getSosUrnFromECallServiceCategory(
                EmergencyNumber.EMERGENCY_SERVICE_CATEGORY_POLICE);
        assertEquals(ret, SOS_SERVICE_URN_POLICE);

        ret = ImsCallUtils.getSosUrnFromECallServiceCategory(
                EmergencyNumber.EMERGENCY_SERVICE_CATEGORY_AMBULANCE);
        assertEquals(ret, SOS_SERVICE_URN_AMBULANCE);

        ret = ImsCallUtils.getSosUrnFromECallServiceCategory(
                EmergencyNumber.EMERGENCY_SERVICE_CATEGORY_FIRE_BRIGADE);
        assertEquals(ret, SOS_SERVICE_URN_FIRE);

        ret = ImsCallUtils.getSosUrnFromECallServiceCategory(
                EmergencyNumber.EMERGENCY_SERVICE_CATEGORY_MARINE_GUARD);
        assertEquals(ret, SOS_SERVICE_URN_MARINE);

        ret = ImsCallUtils.getSosUrnFromECallServiceCategory(
                EmergencyNumber.EMERGENCY_SERVICE_CATEGORY_MOUNTAIN_RESCUE);
        assertEquals(ret, SOS_SERVICE_URN_MOUNTAIN);
    }

    @Test
    public void testGetExtraCodeFromMtc() {

        Hashtable<Integer, Integer> sMtcReasonToImsReason = new Hashtable<Integer, Integer>();
        int reason = CallReasonInfo.CODE_LOCAL_CALL_CS_RETRY_REQUIRED;
        int extraCode = 0;

        int resultCode = ImsReasonInfo.EXTRA_CODE_CALL_RETRY_SILENT_REDIAL;
        int result;

        extraCode = -1;
        result = ImsCallUtils.getExtraCodeFromMtc(reason, extraCode);
        resultCode = ImsReasonInfo.CODE_UNSPECIFIED;
        assertEquals(resultCode, result);

        reason = ImsReasonInfo.CODE_LOCAL_CALL_TERMINATED;
        result = ImsCallUtils.getExtraCodeFromMtc(reason, extraCode);
        assertEquals(resultCode, result);
    }

    @Test
    public void testGetProfileCallTypeFromCallInfo() {
        int result;
        boolean videoCapable = true;
        int callType = 1;
        int profileCallType = ImsCallProfile.CALL_TYPE_VOICE;

        callType = IUMtcCall.CALLTYPE_VOIP;
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
    public void testIsGoogleNativeCompliant() {
        boolean ret = ImsCallUtils.isGoogleNativeCompliant(mContext);
        assertEquals(ret, ImsConstants.USE_GOOGLE_NATIVE_APPS);
    }

    @Test
    public void testIsCallOnNativeAppsAndCountryKR() {
        ICallContext context = Mockito.mock(ICallContext.class);
        boolean testvalue = ImsConstants.USE_GOOGLE_NATIVE_APPS && ImsGlobal.isCountry(
                context.getSlotId(), "KR");
        boolean ret = ImsCallUtils.isCallOnNativeAppsAndCountryKR(context);
        assertEquals(ret, testvalue);
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
    public void updateCallProfileForEmergency() {
        ImsCallProfile profile = new ImsCallProfile();
        profile.mCallType = ImsCallProfile.CALL_TYPE_VOICE;
        CallInfo ci = new CallInfo();
        ci.serviceType = IUMtcCall.SERVICETYPE_EMERGENCY;
        ImsCallUtils.updateCallProfileForEmergency(profile, ci);
        assertTrue(profile.getCallExtraBoolean(ImsCallProfile.EXTRA_EMERGENCY_CALL));
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
    public void updateCallProfileOnSessionProgressing() {
        ImsCallProfile profile = new ImsCallProfile();
        profile.mCallType = ImsCallProfile.CALL_TYPE_VOICE;
        SuppInfo suppInfo = new SuppInfo();
        SuppInfo.SuppService ss = suppInfo.getService(SuppInfo.TYPE_TIP);

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
    public void isEmergencyPdnUsedForEmergencyCallViaWfc() {
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
        suppInfo.addService_int_str(SuppInfo.TYPE_TIP, SuppInfo.TIP_IDENTITY,
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
    public void testUpdateCallProfileOnSessionProgressing() {
        CallInfo ci = new CallInfo();
        ci.serviceType = IUMtcCall.SERVICETYPE_EMERGENCY;
        ci.callType = IUMtcCall.CALLTYPE_VT;
        ci.isConf = true;
        ci.videoCapable = true;
        ci.confSub = true;
        ci.rttCapable = true;
        ImsCallProfile profile = new ImsCallProfile();
        SuppInfo suppInfo = new SuppInfo();
        suppInfo.addService_int_str(SuppInfo.TYPE_VRBT, SuppInfo.TIP_IDENTITY,
                "Testing Demo,sip:+12345678902");
        ImsCallUtils.updateCallProfileOnSessionProgressing(mContext, profile, ci, suppInfo);
        assertEquals(ImsCallProfile.CALL_TYPE_VT, profile.getCallType());
    }

}
