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
import static org.junit.Assert.assertNull;
import static org.junit.Assert.assertTrue;
import static org.mockito.Mockito.any;
import static org.mockito.Mockito.anyInt;
import static org.mockito.Mockito.clearInvocations;
import static org.mockito.Mockito.doReturn;
import static org.mockito.Mockito.eq;
import static org.mockito.Mockito.times;
import static org.mockito.Mockito.verify;
import static org.mockito.Mockito.verifyNoMoreInteractions;
import static org.mockito.Mockito.when;

import android.content.Context;
import android.os.Bundle;
import android.telecom.Connection;
import android.telecom.VideoProfile;
import android.telephony.ims.ImsCallProfile;
import android.telephony.ims.ImsReasonInfo;
import android.telephony.ims.ImsStreamMediaProfile;
import android.telephony.ims.ImsVideoCallProvider;

import com.android.imsstack.base.AppContext;
import com.android.imsstack.core.agents.AgentFactory;
import com.android.imsstack.core.agents.ConfigInterface;
import com.android.imsstack.core.config.CarrierConfig;
import com.android.imsstack.enabler.mtc.MediaInfo;
import com.android.imsstack.imsservice.mmtel.call.IVideoCallSession;

import org.junit.After;
import org.junit.Before;
import org.junit.Test;
import org.junit.runner.RunWith;
import org.junit.runners.JUnit4;
import org.mockito.ArgumentCaptor;
import org.mockito.Mock;
import org.mockito.MockitoAnnotations;

@RunWith(JUnit4.class)
public class ImsVideoCallSessionTest {
    private static final int UPDATE_STATE_IDLE = 0;
    private static final int UPDATE_STATE_SENT = 1;
    private static final int UPDATE_STATE_FINALIZING = 3;
    private static final int SLOT_ID = 0;
    private ImsVideoCallSession mVideoSession;

    @Mock private Context mMockContext;
    @Mock private ImsCallContext mCallContext;
    @Mock private ImsCallSessionImpl mImsCallSession;
    @Mock private ImsVideoCallProvider mVideoCallProvider;
    @Mock private CarrierConfig mMockCarrierConfig;
    @Mock private ConfigInterface mMockConfigInterface;

    @Before
    public void setUp() throws Exception {
        MockitoAnnotations.initMocks(this);
        AppContext.init(mMockContext);
        mVideoSession = new ImsVideoCallSession(mCallContext, mImsCallSession, true);
        when(mMockConfigInterface.getCarrierConfig()).thenReturn(mMockCarrierConfig);
        AgentFactory.getInstance().setAgent(ConfigInterface.class, mMockConfigInterface, SLOT_ID);
    }

    @After
    public void tearDown() throws Exception {
        mVideoSession = null;
        mMockContext = null;
        AgentFactory.getInstance().setAgent(ConfigInterface.class, null, SLOT_ID);
        AppContext.deinit();
    }

    @Test
    public void testInitialization() {
        assertEquals(mVideoSession.getCallContext(), mCallContext);

        //Default return type is false
        assertFalse(mVideoSession.isVrbtEnabled());
    }

    @Test
    public void testGetCallType() {
        ImsCallProfile callProfile = new ImsCallProfile();
        doReturn(callProfile).when(mImsCallSession).getCallProfile();
        assertEquals(mVideoSession.getCallType(), ImsCallProfile.CALL_TYPE_VOICE_N_VIDEO);

        callProfile = null;
        doReturn(callProfile).when(mImsCallSession).getCallProfile();
        assertEquals(mVideoSession.getCallType(), ImsCallProfile.CALL_TYPE_VOICE);
    }

    @Test
    public void testGetSessionModificationType() {
        mVideoSession.setStateAndType(UPDATE_STATE_IDLE, IVideoCallSession.MODIFICATION_CALL_TYPE);
        assertEquals(mVideoSession.getSessionModificationType(), 1);
    }

    @Test
    public void testGetStreamMediaProfile() {
        ImsCallProfile callProfile = null;
        assertNull(mVideoSession.getStreamMediaProfile());

        callProfile = new ImsCallProfile();
        doReturn(callProfile).when(mImsCallSession).getCallProfile();
        assertNotNull(mVideoSession.getStreamMediaProfile());
        assertEquals(callProfile.getMediaProfile(), mVideoSession.getStreamMediaProfile());
    }

    @Test
    public void testIsMoCall() {
        assertTrue(mVideoSession.isMoCall());
    }

    @Test
    public void testSendSessionModifyRequest() {
        //To set call state and type for testing purpose
        mVideoSession.setStateAndType(UPDATE_STATE_SENT, IVideoCallSession.MODIFICATION_NONE);
        mVideoSession.setVideoCallProvider(mVideoCallProvider);
        VideoProfile fromProfile = new VideoProfile(VideoProfile.STATE_BIDIRECTIONAL,
                VideoProfile.QUALITY_HIGH);
        VideoProfile toProfile = new VideoProfile(VideoProfile.STATE_BIDIRECTIONAL,
                VideoProfile.QUALITY_HIGH);
        mVideoSession.sendSessionModifyRequest(fromProfile, toProfile);
        verify(mVideoCallProvider, times(1)).receiveSessionModifyResponse(anyInt(), eq(null),
                eq(fromProfile));

        toProfile = new VideoProfile(VideoProfile.STATE_BIDIRECTIONAL | VideoProfile.STATE_PAUSED,
                VideoProfile.QUALITY_HIGH);
        mVideoSession.sendSessionModifyRequest(fromProfile, toProfile);
        verify(mVideoCallProvider, times(1)).receiveSessionModifyResponse(anyInt(), eq(null),
                eq(fromProfile));

        clearInvocations(mVideoCallProvider);
        mVideoSession.setStateAndType(UPDATE_STATE_IDLE, IVideoCallSession.MODIFICATION_NONE);

        toProfile = null;
        mVideoSession.sendSessionModifyRequest(fromProfile, toProfile);
        verify(mVideoCallProvider).receiveSessionModifyResponse(anyInt(), eq(null),
                eq(fromProfile));
        mVideoSession.setStateAndType(UPDATE_STATE_IDLE, IVideoCallSession.MODIFICATION_NONE);
        ImsCallProfile callProfile = new ImsCallProfile();
        doReturn(callProfile).when(mImsCallSession).getCallProfile();

        toProfile = new VideoProfile(VideoProfile.STATE_RX_ENABLED, VideoProfile.QUALITY_HIGH);
        mVideoSession.sendSessionModifyRequest(fromProfile, toProfile);
        verify(mImsCallSession).update(anyInt(), any(ImsStreamMediaProfile.class));
        clearInvocations(mImsCallSession);
        mVideoSession.setStateAndType(UPDATE_STATE_IDLE, IVideoCallSession.MODIFICATION_NONE);

        callProfile = null;
        callProfile = new ImsCallProfile(ImsCallProfile.SERVICE_TYPE_NORMAL,
                ImsCallProfile.CALL_TYPE_VT);
        doReturn(callProfile).when(mImsCallSession).getCallProfile();
        toProfile = new VideoProfile(VideoProfile.STATE_TX_ENABLED, VideoProfile.QUALITY_HIGH);
        mVideoSession.sendSessionModifyRequest(fromProfile, toProfile);
        verify(mImsCallSession).update(anyInt(), any(ImsStreamMediaProfile.class));
        mVideoSession.setStateAndType(UPDATE_STATE_IDLE, IVideoCallSession.MODIFICATION_NONE);
        mVideoSession.setVideoCallProvider(null);
    }

    @Test
    public void testHandleVideoProfileUpdate() {
        mVideoSession.setStateAndType(UPDATE_STATE_IDLE, IVideoCallSession.MODIFICATION_NONE);
        ImsStreamMediaProfile mediaProfile = new ImsStreamMediaProfile(
                ImsStreamMediaProfile.AUDIO_QUALITY_NONE, ImsStreamMediaProfile.DIRECTION_RECEIVE,
                ImsStreamMediaProfile.VIDEO_QUALITY_NONE, ImsStreamMediaProfile.DIRECTION_INACTIVE);
        ImsCallProfile callProfile = new ImsCallProfile(ImsCallProfile.SERVICE_TYPE_NORMAL,
                        ImsCallProfile.CALL_TYPE_VIDEO_N_VOICE, new Bundle(), mediaProfile);
        doReturn(callProfile).when(mImsCallSession).getCallProfile();
        mVideoSession.setVideoCallProvider(mVideoCallProvider);
        VideoProfile fromProfile = new VideoProfile(VideoProfile.STATE_RX_ENABLED,
                VideoProfile.QUALITY_HIGH);
        VideoProfile toProfile = new VideoProfile(VideoProfile.STATE_TX_ENABLED,
                VideoProfile.QUALITY_HIGH);

        // Verify {@link IVideoCallSession#MODIFICATION_VIDEO_PROFILE} with camera on when the UE
        // is held by remote.
        mVideoSession.sendSessionModifyRequest(fromProfile, toProfile);
        assertTrue(mVideoSession.isCameraOn());
        verify(mVideoCallProvider, times(1)).receiveSessionModifyResponse(anyInt(),
                any(VideoProfile.class), any(VideoProfile.class));
        assertFalse(mVideoSession.isSessionModificationInProgress());
        verify(mImsCallSession, times(1)).update(anyInt(), any(ImsStreamMediaProfile.class));
        mVideoSession.setStateAndType(UPDATE_STATE_IDLE, IVideoCallSession.MODIFICATION_NONE);

        // Verify {@link IVideoCallSession#MODIFICATION_VIDEO_PROFILE} with camera off when the UE
        // is held by remote.
        fromProfile = new VideoProfile(VideoProfile.STATE_TX_ENABLED,
                VideoProfile.QUALITY_HIGH);
        toProfile = new VideoProfile(VideoProfile.STATE_RX_ENABLED,
                VideoProfile.QUALITY_HIGH);
        mVideoSession.sendSessionModifyRequest(fromProfile, toProfile);
        assertFalse(mVideoSession.isCameraOn());
        verify(mVideoCallProvider, times(2)).receiveSessionModifyResponse(anyInt(),
                any(VideoProfile.class), any(VideoProfile.class));
        assertFalse(mVideoSession.isSessionModificationInProgress());
        mVideoSession.setStateAndType(UPDATE_STATE_IDLE, IVideoCallSession.MODIFICATION_NONE);

        // Verify {@link IVideoCallSession#MODIFICATION_VIDEO_PROFILE} with a resume reuest when
        // the UE is held by remote.
        mVideoSession.setMultitaskingState(IVideoCallSession.MULTITASKING_ACTIVATED);
        fromProfile = new VideoProfile(VideoProfile.STATE_TX_ENABLED | VideoProfile.STATE_PAUSED,
                VideoProfile.QUALITY_HIGH);
        toProfile = new VideoProfile(VideoProfile.STATE_RX_ENABLED,
                VideoProfile.QUALITY_HIGH);
        mVideoSession.sendSessionModifyRequest(fromProfile, toProfile);
        verify(mVideoCallProvider, times(2)).receiveSessionModifyResponse(anyInt(),
                any(VideoProfile.class), any(VideoProfile.class));
        assertFalse(mVideoSession.isMultitaskingState());
        mVideoSession.setStateAndType(UPDATE_STATE_IDLE, IVideoCallSession.MODIFICATION_NONE);

        // Verify resume request when camera is off.
        callProfile = new ImsCallProfile(ImsCallProfile.SERVICE_TYPE_NORMAL,
                ImsCallProfile.CALL_TYPE_VIDEO_N_VOICE, new Bundle(), new ImsStreamMediaProfile());
        doReturn(callProfile).when(mImsCallSession).getCallProfile();
        mVideoSession.sendSessionModifyRequest(fromProfile, toProfile);
        assertTrue(mVideoSession.isSessionModificationInProgress());
        verify(mImsCallSession, times(4)).update(anyInt(), any(ImsStreamMediaProfile.class));
        mVideoSession.setStateAndType(UPDATE_STATE_IDLE, IVideoCallSession.MODIFICATION_NONE);
        mVideoSession.setVideoCallProvider(null);
    }

    @Test
    public void testSendSessionModifyResponse() {
        mVideoSession.setStateAndType(UPDATE_STATE_IDLE, IVideoCallSession.MODIFICATION_CALL_TYPE);
        mVideoSession.setVideoCallProvider(mVideoCallProvider);

        VideoProfile videoProfile = new VideoProfile(VideoProfile.STATE_BIDIRECTIONAL,
                VideoProfile.QUALITY_HIGH);
        ImsCallProfile callProfile = null;
        doReturn(callProfile).when(mImsCallSession).getCallProfile();
        mVideoSession.sendSessionModifyResponse(videoProfile);
        verify(mVideoCallProvider).receiveSessionModifyResponse(anyInt(), eq(null),
                any(VideoProfile.class));
        clearInvocations(mVideoCallProvider);

        mVideoSession.setStateAndType(UPDATE_STATE_IDLE, IVideoCallSession.MODIFICATION_CALL_TYPE);
        videoProfile = null;
        callProfile = new ImsCallProfile();
        doReturn(callProfile).when(mImsCallSession).getCallProfile();
        mVideoSession.sendSessionModifyResponse(videoProfile);
        verify(mVideoCallProvider).receiveSessionModifyResponse(anyInt(), eq(null),
                any(VideoProfile.class));

        mVideoSession.setStateAndType(UPDATE_STATE_IDLE,
                IVideoCallSession.MODIFICATION_VIDEO_PROFILE);
        videoProfile = null;
        videoProfile = new VideoProfile(VideoProfile.STATE_TX_ENABLED, VideoProfile.QUALITY_HIGH);
        callProfile = null;
        callProfile = new ImsCallProfile(ImsCallProfile.SERVICE_TYPE_NORMAL,
                ImsCallProfile.CALL_TYPE_VOICE);
        doReturn(callProfile).when(mImsCallSession).getCallProfile();
        mVideoSession.sendSessionModifyResponse(videoProfile);
        verify(mImsCallSession).accept(anyInt(), any(ImsStreamMediaProfile.class));
        clearInvocations(mImsCallSession);

        mVideoSession.setStateAndType(UPDATE_STATE_IDLE, IVideoCallSession.MODIFICATION_CALL_TYPE);
        doReturn(callProfile).when(mImsCallSession).getCallProfile();
        mVideoSession.sendSessionModifyResponse(videoProfile);
        verify(mImsCallSession).accept(anyInt(), any(ImsStreamMediaProfile.class));
        clearInvocations(mImsCallSession);

        callProfile = null;
        MediaInfo proposalMediaInfo = new MediaInfo(MediaInfo.AUDIO_QUALITY_AMR_NB,
                MediaInfo.VIDEO_QUALITY_QCIF, MediaInfo.DIRECTION_SEND_RECEIVE,
                MediaInfo.DIRECTION_SEND_RECEIVE, MediaInfo.DIRECTION_INACTIVE,
                MediaInfo.GTTMODE_FULL);
        mVideoSession.receiveSessionModifyRequest(IVideoCallSession.MODIFICATION_CALL_TYPE,
                proposalMediaInfo);
        ImsStreamMediaProfile OriginalMediaProfile = new ImsStreamMediaProfile(
                ImsStreamMediaProfile.AUDIO_QUALITY_NONE, ImsStreamMediaProfile.DIRECTION_RECEIVE,
                ImsStreamMediaProfile.VIDEO_QUALITY_NONE, ImsStreamMediaProfile.DIRECTION_INACTIVE);
        callProfile = new ImsCallProfile(ImsCallProfile.SERVICE_TYPE_NORMAL,
                ImsCallProfile.CALL_TYPE_VIDEO_N_VOICE, new Bundle(), OriginalMediaProfile);
        videoProfile = new VideoProfile(VideoProfile.STATE_PAUSED, VideoProfile.QUALITY_HIGH);
        mVideoSession.setStateAndType(UPDATE_STATE_IDLE,
                IVideoCallSession.MODIFICATION_VIDEO_PROFILE);
        doReturn(callProfile).when(mImsCallSession).getCallProfile();
        mVideoSession.sendSessionModifyResponse(videoProfile);
        ArgumentCaptor<ImsStreamMediaProfile> mediaProfileCaptor =
                ArgumentCaptor.forClass(ImsStreamMediaProfile.class);
        verify(mImsCallSession).accept(anyInt(), mediaProfileCaptor.capture());
        assertEquals(ImsStreamMediaProfile.DIRECTION_SEND_RECEIVE,
                mediaProfileCaptor.getValue().mAudioDirection);

        mVideoSession.setStateAndType(UPDATE_STATE_IDLE, IVideoCallSession.MODIFICATION_NONE);
        doReturn(callProfile).when(mImsCallSession).getCallProfile();
        mVideoSession.sendSessionModifyResponse(videoProfile);
        assertTrue(mVideoSession.isClearedSessionModificationInfo());
    }

    @Test
    public void testSetCameraSetting() {
        mVideoSession.setCameraSetting(IVideoCallSession.CAMERA_OFF);
        assertFalse(mVideoSession.isCameraOn());
        mVideoSession.setCameraSetting(IVideoCallSession.CAMERA_ON);
        assertTrue(mVideoSession.isCameraOn());
    }

    @Test
    public void testSetMultitaskingState() {
        mVideoSession.setMultitaskingState(IVideoCallSession.MULTITASKING_ACTIVATED);
        assertTrue(mVideoSession.isMultitaskingState());
        mVideoSession.setMultitaskingState(IVideoCallSession.MULTITASKING_NONE);
        assertFalse(mVideoSession.isMultitaskingState());
    }

    @Test
    public void testIsSessionModificationFinalizing() {
        assertFalse(mVideoSession.isSessionModificationFinalizing());
        mVideoSession.setStateAndType(UPDATE_STATE_FINALIZING, IVideoCallSession.MODIFICATION_NONE);
        assertTrue(mVideoSession.isSessionModificationFinalizing());
    }

    @Test
    public void testReceiveSessionModifyRequest() {
        mVideoSession.setStateAndType(UPDATE_STATE_IDLE, IVideoCallSession.MODIFICATION_NONE);
        mVideoSession.setVideoCallProvider(mVideoCallProvider);
        MediaInfo mediaInfo = new MediaInfo(MediaInfo.AUDIO_QUALITY_AMR_NB,
                MediaInfo.VIDEO_QUALITY_QCIF, MediaInfo.DIRECTION_SEND_RECEIVE,
                MediaInfo.DIRECTION_SEND_RECEIVE, MediaInfo.DIRECTION_INACTIVE,
                MediaInfo.GTTMODE_FULL);
        mVideoSession.receiveSessionModifyRequest(IVideoCallSession.MODIFICATION_CALL_TYPE,
                mediaInfo);
        verify(mVideoCallProvider).receiveSessionModifyRequest(any(VideoProfile.class));
        clearInvocations(mVideoCallProvider);

        mVideoSession.receiveSessionModifyRequest(-1, mediaInfo);
        verify(mVideoCallProvider).receiveSessionModifyRequest(any(VideoProfile.class));
        clearInvocations(mVideoCallProvider);

        mVideoSession.setMultitaskingState(IVideoCallSession.MULTITASKING_ACTIVATED);
        mVideoSession.receiveSessionModifyRequest(IVideoCallSession.MODIFICATION_VIDEO_PROFILE,
                mediaInfo);
        verifyNoMoreInteractions(mVideoCallProvider);
    }

    @Test
    public void testReceiveSessionModifyResponse() {
        mVideoSession.setStateAndType(UPDATE_STATE_IDLE, IVideoCallSession.MODIFICATION_NONE);
        mVideoSession.setVideoCallProvider(mVideoCallProvider);
        MediaInfo mediaInfo = new MediaInfo(MediaInfo.AUDIO_QUALITY_AMR_NB,
                MediaInfo.VIDEO_QUALITY_QCIF, MediaInfo.DIRECTION_SEND_RECEIVE,
                MediaInfo.DIRECTION_SEND_RECEIVE, MediaInfo.DIRECTION_INACTIVE,
                MediaInfo.GTTMODE_FULL);
        mVideoSession.receiveSessionModifyResponse(
                ImsReasonInfo.CODE_USER_REJECTED_SESSION_MODIFICATION, mediaInfo);
        verify(mVideoCallProvider).receiveSessionModifyResponse(
                eq(Connection.VideoProvider.SESSION_MODIFY_REQUEST_REJECTED_BY_REMOTE), eq(null),
                any(VideoProfile.class));
        clearInvocations(mVideoCallProvider);

        mVideoSession.receiveSessionModifyResponse(ImsReasonInfo.CODE_TIMEOUT_NO_ANSWER_CALL_UPDATE,
                mediaInfo);
        verify(mVideoCallProvider).receiveSessionModifyResponse(
                eq(Connection.VideoProvider.SESSION_MODIFY_REQUEST_TIMED_OUT), eq(null),
                any(VideoProfile.class));
        clearInvocations(mVideoCallProvider);

        mVideoSession.receiveSessionModifyResponse(ImsReasonInfo.CODE_LOCAL_ILLEGAL_ARGUMENT,
                mediaInfo);
        verify(mVideoCallProvider).receiveSessionModifyResponse(
                eq(Connection.VideoProvider.SESSION_MODIFY_REQUEST_INVALID), eq(null),
                any(VideoProfile.class));
        clearInvocations(mVideoCallProvider);

        // To test default case with invalid value
        mVideoSession.setStateAndType(-1, IVideoCallSession.MODIFICATION_CALL_TYPE);
        mVideoSession.receiveSessionModifyResponse(ImsReasonInfo.CODE_UNSPECIFIED, mediaInfo);
        verify(mVideoCallProvider).receiveSessionModifyResponse(
                eq(Connection.VideoProvider.SESSION_MODIFY_REQUEST_FAIL), eq(null),
                any(VideoProfile.class));
        clearInvocations(mVideoCallProvider);

        mVideoSession.setMultitaskingState(IVideoCallSession.MULTITASKING_ACTIVATED);
        mVideoSession.receiveSessionModifyResponse(-1, mediaInfo);
        assertFalse(mVideoSession.isMultitaskingState());
        verifyNoMoreInteractions(mVideoCallProvider);

        mediaInfo = new MediaInfo(MediaInfo.AUDIO_QUALITY_AMR_NB,
                MediaInfo.VIDEO_QUALITY_QCIF, MediaInfo.DIRECTION_SEND_RECEIVE,
                MediaInfo.DIRECTION_INACTIVE, MediaInfo.DIRECTION_INACTIVE,
                MediaInfo.GTTMODE_FULL);
        mVideoSession.receiveSessionModifyResponse(-1, mediaInfo);
        assertTrue(mVideoSession.isMultitaskingState());
        verifyNoMoreInteractions(mVideoCallProvider);
    }

    @Test
    public void testSendSessionModifyRequest_isVideoCall() {
        ImsCallProfile callProfile = new ImsCallProfile(ImsCallProfile.SERVICE_TYPE_NORMAL,
                ImsCallProfile.CALL_TYPE_VT);
        doReturn(callProfile).when(mImsCallSession).getCallProfile();
        VideoProfile fromProfile = new VideoProfile(VideoProfile.STATE_BIDIRECTIONAL,
                VideoProfile.QUALITY_HIGH);
        VideoProfile toProfile = new VideoProfile(VideoProfile.STATE_AUDIO_ONLY,
                VideoProfile.QUALITY_HIGH);

        mVideoSession.sendSessionModifyRequest(fromProfile, toProfile);
        verify(mImsCallSession).update(anyInt(), any(ImsStreamMediaProfile.class));
    }

    @Test
    public void testSendSessionModifyResponse_isVideoCall() {
        ImsCallProfile callProfile = new ImsCallProfile(ImsCallProfile.SERVICE_TYPE_NORMAL,
                ImsCallProfile.CALL_TYPE_VT);
        doReturn(callProfile).when(mImsCallSession).getCallProfile();
        VideoProfile fromProfile = new VideoProfile(VideoProfile.STATE_AUDIO_ONLY,
                VideoProfile.QUALITY_HIGH);
        mVideoSession.sendSessionModifyResponse(fromProfile);
        verify(mImsCallSession).accept(anyInt(), any(ImsStreamMediaProfile.class));
    }

    @Test
    public void testOnSetCamera() {
        // Camera ID null case
        doReturn(SLOT_ID).when(mCallContext).getSlotId();
        doReturn(true).when(mMockCarrierConfig).getBoolean(
                CarrierConfig.ImsVt.KEY_REQUIRE_SIP_SIGNALING_ON_MULTITASKING_BOOL);
        mVideoSession.setMultitaskingState(IVideoCallSession.MULTITASKING_NONE);
        mVideoSession.setStateAndType(UPDATE_STATE_IDLE, IVideoCallSession.MODIFICATION_NONE);
        ImsStreamMediaProfile mediaProfile = new ImsStreamMediaProfile(
                ImsStreamMediaProfile.AUDIO_QUALITY_NONE,
                ImsStreamMediaProfile.DIRECTION_SEND_RECEIVE,
                ImsStreamMediaProfile.VIDEO_QUALITY_NONE,
                ImsStreamMediaProfile.DIRECTION_SEND_RECEIVE);
        ImsCallProfile callProfile = new ImsCallProfile(ImsCallProfile.SERVICE_TYPE_NORMAL,
                        ImsCallProfile.CALL_TYPE_VOICE, new Bundle(), mediaProfile);
        doReturn(callProfile).when(mImsCallSession).getCallProfile();
        mVideoSession.onSetCamera(null);
        verify(mImsCallSession, times(0)).update(anyInt(), any(ImsStreamMediaProfile.class));

        callProfile = new ImsCallProfile(ImsCallProfile.SERVICE_TYPE_NORMAL,
                        ImsCallProfile.CALL_TYPE_VT, new Bundle(), mediaProfile);
        doReturn(callProfile).when(mImsCallSession).getCallProfile();
        mVideoSession.onSetCamera(null);
        verify(mImsCallSession, times(1)).update(anyInt(), any(ImsStreamMediaProfile.class));

        doReturn(false).when(mMockCarrierConfig).getBoolean(
                CarrierConfig.ImsVt.KEY_REQUIRE_SIP_SIGNALING_ON_MULTITASKING_BOOL);
        mVideoSession.setMultitaskingState(IVideoCallSession.MULTITASKING_NONE);
        mVideoSession.setStateAndType(UPDATE_STATE_IDLE, IVideoCallSession.MODIFICATION_NONE);
        mVideoSession.onSetCamera(null);
        verify(mImsCallSession, times(1)).update(anyInt(), any(ImsStreamMediaProfile.class));

        doReturn(true).when(mMockCarrierConfig).getBoolean(
                CarrierConfig.ImsVt.KEY_REQUIRE_SIP_SIGNALING_ON_MULTITASKING_BOOL);
        mVideoSession.setMultitaskingState(IVideoCallSession.MULTITASKING_NONE);
        mVideoSession.setStateAndType(UPDATE_STATE_IDLE, IVideoCallSession.MODIFICATION_NONE);
        mediaProfile = new ImsStreamMediaProfile(
                ImsStreamMediaProfile.AUDIO_QUALITY_NONE,
                ImsStreamMediaProfile.DIRECTION_SEND_RECEIVE,
                ImsStreamMediaProfile.VIDEO_QUALITY_NONE,
                ImsStreamMediaProfile.DIRECTION_INACTIVE);
        callProfile = new ImsCallProfile(ImsCallProfile.SERVICE_TYPE_NORMAL,
                        ImsCallProfile.CALL_TYPE_VT, new Bundle(), mediaProfile);
        doReturn(callProfile).when(mImsCallSession).getCallProfile();
        mVideoSession.onSetCamera(null);
        verify(mImsCallSession, times(1)).update(anyInt(), any(ImsStreamMediaProfile.class));
        assertTrue(mVideoSession.isMultitaskingState());

        // Camera ID valid case
        mVideoSession.setMultitaskingState(IVideoCallSession.MULTITASKING_ACTIVATED);
        mVideoSession.setStateAndType(UPDATE_STATE_IDLE, IVideoCallSession.MODIFICATION_NONE);
        mediaProfile = new ImsStreamMediaProfile(
                ImsStreamMediaProfile.AUDIO_QUALITY_NONE,
                ImsStreamMediaProfile.DIRECTION_SEND_RECEIVE,
                ImsStreamMediaProfile.VIDEO_QUALITY_NONE,
                ImsStreamMediaProfile.DIRECTION_SEND_RECEIVE);
        callProfile = new ImsCallProfile(ImsCallProfile.SERVICE_TYPE_NORMAL,
                        ImsCallProfile.CALL_TYPE_VT, new Bundle(), mediaProfile);
        doReturn(callProfile).when(mImsCallSession).getCallProfile();
        mVideoSession.onSetCamera("1");
        verify(mImsCallSession, times(1)).update(anyInt(), any(ImsStreamMediaProfile.class));

        mediaProfile = new ImsStreamMediaProfile(
                ImsStreamMediaProfile.AUDIO_QUALITY_NONE,
                ImsStreamMediaProfile.DIRECTION_SEND_RECEIVE,
                ImsStreamMediaProfile.VIDEO_QUALITY_NONE,
                ImsStreamMediaProfile.DIRECTION_INACTIVE);
        callProfile = new ImsCallProfile(ImsCallProfile.SERVICE_TYPE_NORMAL,
                        ImsCallProfile.CALL_TYPE_VT, new Bundle(), mediaProfile);
        doReturn(callProfile).when(mImsCallSession).getCallProfile();
        mVideoSession.onSetCamera("1");
        verify(mImsCallSession, times(2)).update(anyInt(), any(ImsStreamMediaProfile.class));
    }
}
