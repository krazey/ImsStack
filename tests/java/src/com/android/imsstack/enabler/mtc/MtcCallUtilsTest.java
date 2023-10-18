/*
 * Copyright (C) 2023 The Android Open Source Project
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

import static org.junit.Assert.assertEquals;
import static org.junit.Assert.assertFalse;
import static org.junit.Assert.assertTrue;

import com.android.imsstack.enabler.mtc.conf.UsersInfo;

import org.junit.Test;
import org.junit.runner.RunWith;
import org.junit.runners.JUnit4;
import org.mockito.Mockito;

@RunWith(JUnit4.class)
public class MtcCallUtilsTest {
    @Test
    public void testAddUserWithEntity() {
        UsersInfo userInfo = new UsersInfo();
        MtcCallUtils.addUser(userInfo, 100, null, null, null);

        assertEquals(100, userInfo.getUser(0).callID);
    }

    @Test
    public void testCreateHoldMedia() {
        CallInfo callInfo = new CallInfo();
        callInfo.callType = IUMtcCall.CALLTYPE_VIDEO_RTT;
        callInfo.isConf = true;
        MediaInfo mediaInfo = new MediaInfo();
        mediaInfo.ADir = MediaInfo.DIRECTION_RECEIVE;
        mediaInfo.TDir = MediaInfo.DIRECTION_SEND;

        MediaInfo createdMediaInfo = MtcCallUtils.createHoldMedia(callInfo, mediaInfo, true, true);

        assertEquals(MediaInfo.DIRECTION_INACTIVE, createdMediaInfo.ADir);
        assertEquals(MediaInfo.DIRECTION_INACTIVE, createdMediaInfo.VDir);
        assertEquals(MediaInfo.DIRECTION_INVALID, createdMediaInfo.TDir);
        assertEquals(MediaInfo.DIRECTION_INVALID, createdMediaInfo.GTTMode);

        mediaInfo.VDir = MediaInfo.DIRECTION_SEND_RECEIVE;
        callInfo.isConf = false;
        mediaInfo.GTTMode = MediaInfo.GTTMODE_VCO;

        createdMediaInfo = MtcCallUtils.createHoldMedia(callInfo, mediaInfo, false, true);

        assertEquals(MediaInfo.DIRECTION_SEND, createdMediaInfo.VDir);
        assertEquals(MediaInfo.DIRECTION_INACTIVE, createdMediaInfo.TDir);

        createdMediaInfo = MtcCallUtils.createHoldMedia(callInfo, mediaInfo, false, false);

        assertEquals(MediaInfo.DIRECTION_SEND, createdMediaInfo.TDir);
    }

    @Test
    public void testCreateUnholdMedia() {
        CallInfo callInfo = new CallInfo();
        callInfo.callType = IUMtcCall.CALLTYPE_VIDEO_RTT;
        callInfo.isConf = true;
        MediaInfo mediaInfo = new MediaInfo();
        mediaInfo.ADir = MediaInfo.DIRECTION_INACTIVE;
        mediaInfo.TDir = MediaInfo.DIRECTION_SEND;

        MediaInfo createdMediaInfo = MtcCallUtils.createUnholdMedia(callInfo, mediaInfo, true);

        assertEquals(MediaInfo.DIRECTION_RECEIVE, createdMediaInfo.ADir);
        assertEquals(MediaInfo.DIRECTION_SEND_RECEIVE, createdMediaInfo.VDir);
        assertEquals(MediaInfo.DIRECTION_INVALID, createdMediaInfo.TDir);
        assertEquals(MediaInfo.DIRECTION_INVALID, createdMediaInfo.GTTMode);

        mediaInfo.VDir = MediaInfo.DIRECTION_INACTIVE;
        callInfo.isConf = false;
        mediaInfo.GTTMode = MediaInfo.GTTMODE_VCO;

        createdMediaInfo = MtcCallUtils.createUnholdMedia(callInfo, mediaInfo, false);

        assertEquals(MediaInfo.DIRECTION_RECEIVE, createdMediaInfo.VDir);
        assertEquals(MediaInfo.DIRECTION_SEND_RECEIVE, createdMediaInfo.TDir);
    }

    @Test
    public void testGetSIPStatusCodeFromUserStatusCode() {
        assertEquals(603, MtcCallUtils.getSIPStatusCodeFromUserStatusCode(
                UsersInfo.USER_STATUS_REJECT));
        assertEquals(1000, MtcCallUtils.getSIPStatusCodeFromUserStatusCode(1000));
    }

    @Test
    public void testHasVideoQuality() {
        MediaInfo mediaInfo = new MediaInfo();

        assertFalse(MtcCallUtils.hasVideoQuality(mediaInfo));

        mediaInfo.VQuality = MediaInfo.VIDEO_QUALITY_QCIF;

        assertTrue(MtcCallUtils.hasVideoQuality(mediaInfo));
    }

    @Test
    public void testIs1WayVideo() {
        MediaInfo mediaInfo = new MediaInfo();

        assertFalse(MtcCallUtils.is1WayVideo(mediaInfo));

        mediaInfo.ADir = MediaInfo.DIRECTION_SEND_RECEIVE;
        mediaInfo.VDir  = MediaInfo.DIRECTION_SEND;

        assertTrue(MtcCallUtils.is1WayVideo(mediaInfo));
    }

    @Test
    public void testIs1WayVideoByRemoteEnd() {
        MediaInfo mediaInfo = new MediaInfo();

        assertFalse(MtcCallUtils.is1WayVideoByRemoteEnd(mediaInfo));

        mediaInfo.ADir = MediaInfo.DIRECTION_SEND_RECEIVE;
        mediaInfo.VDir  = MediaInfo.DIRECTION_RECEIVE;

        assertTrue(MtcCallUtils.is1WayVideoByRemoteEnd(mediaInfo));
    }

    @Test
    public void testIsAudioEvsCategory() {
        assertFalse(MtcCallUtils.isAudioEvsCategory(MediaInfo.AUDIO_QUALITY_MAX));
        assertTrue(MtcCallUtils.isAudioEvsCategory(MediaInfo.AUDIO_QUALITY_EVS_FB));
    }

    @Test
    public void testIsAudioHDQuality() {
        assertFalse(MtcCallUtils.isAudioHDQuality(MediaInfo.AUDIO_QUALITY_MAX));
        assertTrue(MtcCallUtils.isAudioHDQuality(MediaInfo.AUDIO_QUALITY_EVS_WB));
    }

    @Test
    public void testIsAudioUHDQuality() {
        assertFalse(MtcCallUtils.isAudioUHDQuality(MediaInfo.AUDIO_QUALITY_MAX));
        assertTrue(MtcCallUtils.isAudioUHDQuality(MediaInfo.AUDIO_QUALITY_EVS_FB));
    }

    @Test
    public void testIsCallTerminatedByCallForward() {
        CallReasonInfo callReasonInfo = new CallReasonInfo();

        assertFalse(MtcCallUtils.isCallTerminatedByCallForward(callReasonInfo));

        callReasonInfo.mCode = CallReasonInfo.CODE_USER_TERMINATED;
        callReasonInfo.mExtraCode = CallReasonInfo.EXTRA_USER_TERMINATED_ECT;

        assertTrue(MtcCallUtils.isCallTerminatedByCallForward(callReasonInfo));
    }

    @Test
    public void testIsCallTerminatedByJoiningConference() {
        assertFalse(MtcCallUtils.isCallTerminatedByJoiningConference(
                CallReasonInfo.CODE_LOCAL_ILLEGAL_ARGUMENT));
        assertTrue(MtcCallUtils.isCallTerminatedByJoiningConference(
                CallReasonInfo.CODE_LOCAL_ENDED_BY_CONFERENCE_MERGE));
    }

    @Test
    public void testIsCallWaitingEnabled() {
        assertFalse(MtcCallUtils.isCallWaitingEnabled(null));

        SuppInfo suppInfo = new SuppInfo();

        assertFalse(MtcCallUtils.isCallWaitingEnabled(suppInfo));

        suppInfo.addService_bool(SuppInfo.TYPE_CW, true);

        assertTrue(MtcCallUtils.isCallWaitingEnabled(suppInfo));
    }

    @Test
    public void testIsHoldMediaOnVideoCall() {
        MediaInfo mediaInfo = new MediaInfo();

        assertFalse(MtcCallUtils.isHoldMediaOnVideoCall(mediaInfo,  true));

        mediaInfo.ADir = MediaInfo.DIRECTION_SEND;
        mediaInfo.VDir = MediaInfo.DIRECTION_SEND;

        assertFalse(MtcCallUtils.isHoldMediaOnVideoCall(mediaInfo, true));
        assertTrue(MtcCallUtils.isHoldMediaOnVideoCall(mediaInfo, false));

        mediaInfo.VDir = MediaInfo.DIRECTION_INACTIVE;

        assertTrue(MtcCallUtils.isHoldMediaOnVideoCall(mediaInfo, true));
    }

    @Test
    public void testIsHoldMediaOnVideoCallByRemoteEnd() {
        MediaInfo mediaInfo = new MediaInfo();

        assertFalse(MtcCallUtils.isHoldMediaOnVideoCallByRemoteEnd(mediaInfo,  true));

        mediaInfo.ADir = MediaInfo.DIRECTION_RECEIVE;
        mediaInfo.VDir = MediaInfo.DIRECTION_RECEIVE;

        assertFalse(MtcCallUtils.isHoldMediaOnVideoCallByRemoteEnd(mediaInfo, true));
        assertTrue(MtcCallUtils.isHoldMediaOnVideoCallByRemoteEnd(mediaInfo, false));

        mediaInfo.VDir = MediaInfo.DIRECTION_INACTIVE;

        assertTrue(MtcCallUtils.isHoldMediaOnVideoCallByRemoteEnd(mediaInfo, true));
    }

    @Test
    public void testIsUnholdMediaOnVideoCall() {
        MediaInfo mediaInfo = new MediaInfo();

        assertFalse(MtcCallUtils.isUnholdMediaOnVideoCall(mediaInfo,  true));

        mediaInfo.ADir = MediaInfo.DIRECTION_SEND_RECEIVE;
        mediaInfo.VDir = MediaInfo.DIRECTION_SEND_RECEIVE;

        assertTrue(MtcCallUtils.isUnholdMediaOnVideoCall(mediaInfo, true));
        assertTrue(MtcCallUtils.isUnholdMediaOnVideoCall(mediaInfo, false));

        mediaInfo.VDir = MediaInfo.DIRECTION_INACTIVE;

        assertTrue(MtcCallUtils.isUnholdMediaOnVideoCall(mediaInfo, true));
    }

    @Test
    public void testIsUnholdMediaOnVideoCallByRemoteEnd() {
        MediaInfo mediaInfo = new MediaInfo();

        assertFalse(MtcCallUtils.isUnholdMediaOnVideoCallByRemoteEnd(mediaInfo));

        mediaInfo.ADir = MediaInfo.DIRECTION_SEND;
        mediaInfo.VDir = MediaInfo.DIRECTION_SEND;

        assertTrue(MtcCallUtils.isUnholdMediaOnVideoCallByRemoteEnd(mediaInfo));
    }

    @Test
    public void testIsLocalHoldToneEnforced() {
        SuppInfo suppInfo = new SuppInfo();
        suppInfo.addService_bool(SuppInfo.TYPE_VM, true);

        assertFalse(MtcCallUtils.isLocalHoldToneEnforced(suppInfo));

        suppInfo.addService_bool(SuppInfo.TYPE_ENFORCE_LT, true);

        assertTrue(MtcCallUtils.isLocalHoldToneEnforced(suppInfo));
    }

    @Test
    public void testIsOutgoingCallsBarred() {
        CallReasonInfo callReasonInfo = new CallReasonInfo();

        assertFalse(MtcCallUtils.isOutgoingCallsBarred(callReasonInfo));

        callReasonInfo.mCode = CallReasonInfo.CODE_SIP_USER_REJECTED;
        callReasonInfo.mExtraCode = 53;

        assertTrue(MtcCallUtils.isOutgoingCallsBarred(callReasonInfo));
    }

    @Test
    public void testisSuppInfoBoolean() {
        MtcCall call = Mockito.mock(MtcCall.class);

        assertFalse(MtcCallUtils.isSuppInfoBoolean(SuppInfo.TYPE_DUALNUMBER));
        assertTrue(MtcCallUtils.isSuppInfoBoolean(SuppInfo.TYPE_ENFORCE_LT));
    }

    @Test
    public void isSuppInfoInt() {
        MtcCall call = Mockito.mock(MtcCall.class);

        assertFalse(MtcCallUtils.isSuppInfoInt(SuppInfo.TYPE_DUALNUMBER));
        assertTrue(MtcCallUtils.isSuppInfoInt(SuppInfo.TYPE_CALLING_NUM_VERIFICATION));
    }

    @Test
    public void isSuppInfoString() {
        MtcCall call = Mockito.mock(MtcCall.class);

        assertFalse(MtcCallUtils.isSuppInfoString(SuppInfo.TYPE_GEOLOCATION));
        assertTrue(MtcCallUtils.isSuppInfoString(SuppInfo.TYPE_DUALNUMBER));
    }

    @Test
    public void testReverseMediaDirection() {
        MediaInfo mediaInfo = new MediaInfo();
        mediaInfo.ADir = MediaInfo.DIRECTION_SEND_RECEIVE;
        mediaInfo.VDir = MediaInfo.DIRECTION_SEND_RECEIVE;

        MtcCallUtils.reverseMediaDirection(mediaInfo);

        assertEquals(MediaInfo.DIRECTION_SEND_RECEIVE, mediaInfo.ADir);
        assertEquals(MediaInfo.DIRECTION_SEND_RECEIVE, mediaInfo.VDir);

        mediaInfo.ADir = MediaInfo.DIRECTION_RECEIVE;
        mediaInfo.VDir = MediaInfo.DIRECTION_RECEIVE;

        MtcCallUtils.reverseMediaDirection(mediaInfo);

        assertEquals(MediaInfo.DIRECTION_SEND, mediaInfo.ADir);
        assertEquals(MediaInfo.DIRECTION_SEND, mediaInfo.VDir);

        mediaInfo.ADir = MediaInfo.DIRECTION_SEND;
        mediaInfo.VDir = MediaInfo.DIRECTION_SEND;

        MtcCallUtils.reverseMediaDirection(mediaInfo);

        assertEquals(MediaInfo.DIRECTION_RECEIVE, mediaInfo.ADir);
        assertEquals(MediaInfo.DIRECTION_RECEIVE, mediaInfo.VDir);
    }
}
