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
import static org.mockito.Mockito.any;
import static org.mockito.Mockito.anyInt;
import static org.mockito.Mockito.doReturn;
import static org.mockito.Mockito.eq;
import static org.mockito.Mockito.times;
import static org.mockito.Mockito.verify;

import android.content.Context;
import android.telecom.VideoProfile;
import android.telephony.ims.ImsCallProfile;
import android.telephony.ims.ImsReasonInfo;
import android.telephony.ims.ImsStreamMediaProfile;
import android.telephony.ims.ImsVideoCallProvider;

import com.android.imsstack.enabler.mtc.MediaInfo;
import com.android.imsstack.imsservice.mmtel.call.IVideoCallSession;
import com.android.imsstack.imsservice.mmtel.videocall.base.ImsVideoCallProviderBase;

import org.junit.After;
import org.junit.Before;
import org.junit.Test;
import org.junit.runner.RunWith;
import org.junit.runners.JUnit4;
import org.mockito.Mockito;

@RunWith(JUnit4.class)
public class ImsVideoCallSessionTest {
    private Context mMockContext;
    private ImsCallContext mCallContext;
    private ImsCallSessionImpl mImsCallSession;
    private ImsVideoCallProvider mVideoCallProvider;
    private ImsVideoCallProviderBase mImsVideoCallProviderBase;
    private ImsVideoCallSession mVideoSession;

    @Before
    public void setUp() throws Exception {
        mMockContext = Mockito.mock(Context.class);
        mCallContext  = Mockito.mock(ImsCallContext.class);
        mImsCallSession = Mockito.mock(ImsCallSessionImpl.class);
        mVideoCallProvider = Mockito.mock(ImsVideoCallProvider.class);
        mImsVideoCallProviderBase = Mockito.mock(ImsVideoCallProviderBase.class);
        mVideoSession = new ImsVideoCallSession(mCallContext, mImsCallSession, true);
    }

    @After
    public void tearDown() throws Exception {
        mVideoSession = null;
    }

    @Test
    public void testGetCallContext() {
        assertEquals(mVideoSession.getCallContext(), mCallContext);
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
        mVideoSession.setStateAndType(0, IVideoCallSession.MODIFICATION_CALL_TYPE);
        assertEquals(mVideoSession.getSessionModificationType(), 1);
    }

    @Test
    public void testGetStreamMediaProfile() {
        ImsCallProfile callProfile = new ImsCallProfile();
        doReturn(callProfile).when(mImsCallSession).getCallProfile();
        assertNotNull(mVideoSession.getStreamMediaProfile());
    }

    @Test
    public void testIsMoCall() {
        assertTrue(mVideoSession.isMoCall());
    }

    @Test
    public void testSendSessionModifyRequest() {
        mVideoSession.setStateAndType(1, IVideoCallSession.MODIFICATION_NONE);
        mVideoSession.setVideoCallProvider(mVideoCallProvider);
        VideoProfile fromProfile = new VideoProfile(VideoProfile.QUALITY_HIGH,
                VideoProfile.STATE_BIDIRECTIONAL);
        VideoProfile toProfile = new VideoProfile(VideoProfile.QUALITY_HIGH,
                VideoProfile.STATE_BIDIRECTIONAL);
        mVideoSession.sendSessionModifyRequest(fromProfile, toProfile);
        verify(mVideoCallProvider, times(1)).receiveSessionModifyResponse(anyInt(), eq(null),
                eq(fromProfile));
        mVideoSession.setStateAndType(0, IVideoCallSession.MODIFICATION_NONE);

        toProfile = null;
        mVideoSession.sendSessionModifyRequest(fromProfile, toProfile);
        verify(mVideoCallProvider, times(2)).receiveSessionModifyResponse(anyInt(), eq(null),
                eq(fromProfile));
        mVideoSession.setStateAndType(0, IVideoCallSession.MODIFICATION_NONE);
        ImsCallProfile callProfile = new ImsCallProfile();
        doReturn(callProfile).when(mImsCallSession).getCallProfile();

        toProfile = new VideoProfile(VideoProfile.QUALITY_HIGH,
                VideoProfile.STATE_RX_ENABLED);
        mVideoSession.sendSessionModifyRequest(fromProfile, toProfile);
        verify(mImsCallSession, times(1)).update(anyInt(), any(ImsStreamMediaProfile.class));
        mVideoSession.setStateAndType(0, IVideoCallSession.MODIFICATION_NONE);

        callProfile = new ImsCallProfile(ImsCallProfile.SERVICE_TYPE_NORMAL,
                ImsCallProfile.CALL_TYPE_VT);
        doReturn(callProfile).when(mImsCallSession).getCallProfile();
        toProfile = new VideoProfile(VideoProfile.QUALITY_HIGH,
                VideoProfile.STATE_TX_ENABLED);
        mVideoSession.sendSessionModifyRequest(fromProfile, toProfile);
        verify(mImsCallSession, times(2)).update(anyInt(), any(ImsStreamMediaProfile.class));
        mVideoSession.setStateAndType(0, IVideoCallSession.MODIFICATION_NONE);
        mVideoSession.setVideoCallProvider(null);
    }

    @Test
    public void testSendSessionModifyResponse() {
        mVideoSession.setStateAndType(0, IVideoCallSession.MODIFICATION_CALL_TYPE);
        mVideoSession.setVideoCallProvider(mVideoCallProvider);

        VideoProfile videoProfile = new VideoProfile(VideoProfile.QUALITY_HIGH,
                VideoProfile.STATE_BIDIRECTIONAL);
        ImsCallProfile callProfile = null;
        doReturn(callProfile).when(mImsCallSession).getCallProfile();
        mVideoSession.sendSessionModifyResponse(videoProfile);
        verify(mVideoCallProvider, times(1)).receiveSessionModifyResponse(anyInt(), eq(null),
                any(VideoProfile.class));
        mVideoSession.setStateAndType(0, IVideoCallSession.MODIFICATION_CALL_TYPE);

        videoProfile = null;
        callProfile = new ImsCallProfile();
        doReturn(callProfile).when(mImsCallSession).getCallProfile();
        mVideoSession.sendSessionModifyResponse(videoProfile);
        verify(mVideoCallProvider, times(2)).receiveSessionModifyResponse(anyInt(), eq(null),
                any(VideoProfile.class));

        mVideoSession.setStateAndType(0, IVideoCallSession.MODIFICATION_VIDEO_PROFILE);
        videoProfile = new VideoProfile(VideoProfile.QUALITY_HIGH,
                VideoProfile.STATE_TX_ENABLED);
        callProfile = new ImsCallProfile(ImsCallProfile.SERVICE_TYPE_NORMAL,
                ImsCallProfile.CALL_TYPE_VOICE);
        doReturn(callProfile).when(mImsCallSession).getCallProfile();
        mVideoSession.sendSessionModifyResponse(videoProfile);
        verify(mImsCallSession, times(1)).accept(anyInt(), any(ImsStreamMediaProfile.class));

        mVideoSession.setStateAndType(0, IVideoCallSession.MODIFICATION_NONE);
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
    public void testReceiveSessionModifyRequest() {
        mVideoSession.setStateAndType(0, IVideoCallSession.MODIFICATION_NONE);
        mVideoSession.setVideoCallProvider(mVideoCallProvider);
        MediaInfo mediaInfo = new MediaInfo(MediaInfo.AUDIO_QUALITY_AMR_NB,
                MediaInfo.VIDEO_QUALITY_QCIF, MediaInfo.DIRECTION_SEND_RECEIVE,
                MediaInfo.DIRECTION_SEND_RECEIVE, MediaInfo.DIRECTION_INACTIVE,
                MediaInfo.GTTMODE_FULL);
        mVideoSession.receiveSessionModifyRequest(IVideoCallSession.MODIFICATION_CALL_TYPE,
                mediaInfo);
        verify(mVideoCallProvider, times(1)).receiveSessionModifyRequest(any(VideoProfile.class));
    }

    @Test
    public void testReceiveSessionModifyResponse() {
        mVideoSession.setStateAndType(0, IVideoCallSession.MODIFICATION_NONE);
        mVideoSession.setVideoCallProvider(mVideoCallProvider);
        MediaInfo mediaInfo = new MediaInfo(MediaInfo.AUDIO_QUALITY_AMR_NB,
                MediaInfo.VIDEO_QUALITY_QCIF, MediaInfo.DIRECTION_SEND_RECEIVE,
                MediaInfo.DIRECTION_SEND_RECEIVE, MediaInfo.DIRECTION_INACTIVE,
                MediaInfo.GTTMODE_FULL);
        mVideoSession.receiveSessionModifyResponse(ImsReasonInfo.CODE_SIP_USER_REJECTED, mediaInfo);
        verify(mVideoCallProvider, times(1)).receiveSessionModifyResponse(anyInt(), eq(null),
                any(VideoProfile.class));
    }
}
