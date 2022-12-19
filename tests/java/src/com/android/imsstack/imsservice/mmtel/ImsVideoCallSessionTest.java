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
import static org.mockito.Mockito.verify;
import static org.mockito.Mockito.when;

import android.telecom.VideoProfile;
import android.telephony.ims.ImsCallProfile;
import android.telephony.ims.ImsReasonInfo;
import android.telephony.ims.ImsStreamMediaProfile;
import android.telephony.ims.ImsVideoCallProvider;

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
import org.mockito.Mock;
import org.mockito.MockitoAnnotations;

@RunWith(JUnit4.class)
public class ImsVideoCallSessionTest {
    private static final int UPDATE_STATE_IDLE = 0;
    private static final int UPDATE_STATE_SENT = 1;
    private static final int UPDATE_STATE_FINALIZING = 3;
    private static final int SLOT_ID = 0;
    private ImsVideoCallSession mVideoSession;

    @Mock private ImsCallContext mCallContext;
    @Mock private ImsCallSessionImpl mImsCallSession;
    @Mock private ImsVideoCallProvider mVideoCallProvider;
    @Mock private CarrierConfig mMockCarrierConfig;
    @Mock private ConfigInterface mMockConfigInterface;

    @Before
    public void setUp() throws Exception {
        MockitoAnnotations.initMocks(this);
        mVideoSession = new ImsVideoCallSession(mCallContext, mImsCallSession, true);
        when(mMockConfigInterface.getCarrierConfig()).thenReturn(mMockCarrierConfig);
        AgentFactory.getInstance().setAgent(ConfigInterface.class, mMockConfigInterface, SLOT_ID);
    }

    @After
    public void tearDown() throws Exception {
        mVideoSession = null;
        AgentFactory.getInstance().setAgent(ConfigInterface.class, null, SLOT_ID);
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
        VideoProfile fromProfile = new VideoProfile(VideoProfile.QUALITY_HIGH,
                VideoProfile.STATE_BIDIRECTIONAL);
        VideoProfile toProfile = new VideoProfile(VideoProfile.QUALITY_HIGH,
                VideoProfile.STATE_BIDIRECTIONAL);
        mVideoSession.sendSessionModifyRequest(fromProfile, toProfile);
        verify(mVideoCallProvider).receiveSessionModifyResponse(anyInt(), eq(null),
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

        toProfile = new VideoProfile(VideoProfile.QUALITY_HIGH,
                VideoProfile.STATE_RX_ENABLED);
        mVideoSession.sendSessionModifyRequest(fromProfile, toProfile);
        verify(mImsCallSession).update(anyInt(), any(ImsStreamMediaProfile.class));
        clearInvocations(mImsCallSession);
        mVideoSession.setStateAndType(UPDATE_STATE_IDLE, IVideoCallSession.MODIFICATION_NONE);

        callProfile = null;
        callProfile = new ImsCallProfile(ImsCallProfile.SERVICE_TYPE_NORMAL,
                ImsCallProfile.CALL_TYPE_VT);
        doReturn(callProfile).when(mImsCallSession).getCallProfile();
        toProfile = null;
        toProfile = new VideoProfile(VideoProfile.QUALITY_HIGH,
                VideoProfile.STATE_TX_ENABLED);
        mVideoSession.sendSessionModifyRequest(fromProfile, toProfile);
        verify(mImsCallSession).update(anyInt(), any(ImsStreamMediaProfile.class));
        mVideoSession.setStateAndType(UPDATE_STATE_IDLE, IVideoCallSession.MODIFICATION_NONE);
        mVideoSession.setVideoCallProvider(null);
    }

    @Test
    public void testSendSessionModifyResponse() {
        mVideoSession.setStateAndType(UPDATE_STATE_IDLE, IVideoCallSession.MODIFICATION_CALL_TYPE);
        mVideoSession.setVideoCallProvider(mVideoCallProvider);

        VideoProfile videoProfile = new VideoProfile(VideoProfile.QUALITY_HIGH,
                VideoProfile.STATE_BIDIRECTIONAL);
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
        videoProfile = new VideoProfile(VideoProfile.QUALITY_HIGH,
                VideoProfile.STATE_TX_ENABLED);
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
        callProfile = new ImsCallProfile(ImsCallProfile.SERVICE_TYPE_NORMAL,
                ImsCallProfile.CALL_TYPE_VIDEO_N_VOICE);
        mVideoSession.setStateAndType(UPDATE_STATE_IDLE,
                IVideoCallSession.MODIFICATION_VIDEO_PROFILE);
        doReturn(callProfile).when(mImsCallSession).getCallProfile();
        mVideoSession.sendSessionModifyResponse(videoProfile);
        verify(mImsCallSession).accept(anyInt(), any(ImsStreamMediaProfile.class));

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
    }

    @Test
    public void testReceiveSessionModifyResponse() {
        mVideoSession.setStateAndType(UPDATE_STATE_IDLE, IVideoCallSession.MODIFICATION_NONE);
        mVideoSession.setVideoCallProvider(mVideoCallProvider);
        MediaInfo mediaInfo = new MediaInfo(MediaInfo.AUDIO_QUALITY_AMR_NB,
                MediaInfo.VIDEO_QUALITY_QCIF, MediaInfo.DIRECTION_SEND_RECEIVE,
                MediaInfo.DIRECTION_SEND_RECEIVE, MediaInfo.DIRECTION_INACTIVE,
                MediaInfo.GTTMODE_FULL);
        mVideoSession.receiveSessionModifyResponse(ImsReasonInfo.CODE_SIP_USER_REJECTED, mediaInfo);
        verify(mVideoCallProvider).receiveSessionModifyResponse(anyInt(), eq(null),
                any(VideoProfile.class));
        clearInvocations(mVideoCallProvider);

        mVideoSession.receiveSessionModifyResponse(ImsReasonInfo.CODE_TIMEOUT_NO_ANSWER_CALL_UPDATE,
                mediaInfo);
        verify(mVideoCallProvider).receiveSessionModifyResponse(anyInt(), eq(null),
                any(VideoProfile.class));
        clearInvocations(mVideoCallProvider);

        mVideoSession.receiveSessionModifyResponse(ImsReasonInfo.CODE_LOCAL_ILLEGAL_ARGUMENT,
                mediaInfo);
        verify(mVideoCallProvider).receiveSessionModifyResponse(anyInt(), eq(null),
                any(VideoProfile.class));
        clearInvocations(mVideoCallProvider);

        // To test default case with invalid value
        mVideoSession.setStateAndType(-1, IVideoCallSession.MODIFICATION_CALL_TYPE);
        mVideoSession.receiveSessionModifyResponse(ImsReasonInfo.CODE_UNSPECIFIED, mediaInfo);
        verify(mVideoCallProvider).receiveSessionModifyResponse(anyInt(), eq(null),
                any(VideoProfile.class));

    }
}
