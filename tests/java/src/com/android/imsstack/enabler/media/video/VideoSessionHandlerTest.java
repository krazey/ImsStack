/**
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
package com.android.imsstack.enabler.media;

import static org.junit.Assert.assertEquals;
import static org.mockito.Matchers.eq;
import static org.mockito.Mockito.verify;
import static org.mockito.Mockito.when;

import android.content.Context;
import android.os.Parcel;
import android.os.Parcelable;
import android.telephony.imsmedia.ImsMediaManager;
import android.telephony.imsmedia.ImsMediaSession;
import android.telephony.imsmedia.ImsVideoSession;
import android.telephony.imsmedia.MediaQualityThreshold;
import android.telephony.imsmedia.VideoConfig;
import android.telephony.imsmedia.VideoSessionCallback;

import com.android.imsstack.enabler.IBaseContext;
import com.android.imsstack.enabler.mtc.MtcMediaSession;

import org.junit.Before;
import org.junit.Test;
import org.junit.runner.RunWith;
import org.junit.runners.JUnit4;
import org.mockito.Mock;
import org.mockito.MockitoAnnotations;

import java.util.concurrent.Executor;

@RunWith(JUnit4.class)
public class VideoSessionHandlerTest {
    // open Session Parameters
    private static final String LOCAL_RTP_ADDRESS = "222.22.22.22";
    private static final int LOCAL_RTP_PORT = 50050;
    private static final int SESSION_ID = 12;

    // Session Operator Results
    private static final int RESULT_FAILURE = ImsMediaSession.RESULT_NO_RESOURCES;
    private static final int RESULT_SUCCESS = ImsMediaSession.RESULT_SUCCESS;

    // Packet Types
    private static final int RTP = ImsMediaSession.PACKET_TYPE_RTP;
    private static final int RTCP = ImsMediaSession.PACKET_TYPE_RTCP;

    // Mock Objects
    @Mock Context mContext;
    @Mock IBaseContext mMockContext;
    @Mock MtcMediaSession mMockMtcMediaSession;
    @Mock VideoSessionCallbackHandler mMockVideoSessionCallbackHandler;
    @Mock ImsVideoSession mMockVideoSession;
    @Mock ImsMediaManager mMockImsMediaManager;
    @Mock Executor mMockExecutor;

    private VideoSessionHandler mVideoSessionHandler;
    private VideoSessionCallback mVideoSessionCallback;
    private MediaSession mMediaSession;
    private MediaManagerHelper mMediaManager;
    private IMediaListener mMediaListener;

    @Before
    public void setUp() throws Exception {
        // Initialize Mock Objects
        MockitoAnnotations.initMocks(this);
        when(mMockContext.getContext()).thenReturn(mContext);

        // create the instance to test
        mMediaSession =
                new MediaSession(
                        mMockContext, mMockMtcMediaSession, mMockImsMediaManager, mMockExecutor);
        mMediaManager = mMediaSession.getMediaManager();
        mMediaListener = mMediaSession.getMediaListenerProxy();
        mVideoSessionHandler =
                new VideoSessionHandler(
                        mMediaManager, mMockVideoSessionCallbackHandler, mMockVideoSession);
        mMediaSession.setVideoSessionHandler(mVideoSessionHandler);
        mVideoSessionCallback = mVideoSessionHandler.getVideoSessionCallback();
    }

    @Test
    public void testOpenSession() {

        Parcel testParcel = Parcel.obtain();
        testParcel.writeInt(MediaConstants.REQUEST_OPEN_SESSION);
        testParcel.writeInt(ImsMediaSession.SESSION_TYPE_VIDEO);
        testParcel.writeString(LOCAL_RTP_ADDRESS);
        testParcel.writeInt(LOCAL_RTP_PORT);
        testParcel.setDataPosition(0);
        // OpenSession Received
        mVideoSessionHandler.setVideoSession(null);
        mMediaManager.setImsMediaConnected(true);
        mMediaListener.onMediaMessage(testParcel);
        verify(mMockVideoSessionCallbackHandler)
                .openSessionResponse(eq(ImsMediaSession.RESULT_PORT_UNAVAILABLE));

        // TODO_MEDIA : Socket creation success scenario to be verified
        // VideoSession was opened already, but OpenVideoSession requested again for same session
        mVideoSessionHandler.setVideoSession(mMockVideoSession);
        testParcel.setDataPosition(0);
        mMediaListener.onMediaMessage(testParcel);
        verify(mMockVideoSessionCallbackHandler)
                .openSessionResponse(eq(ImsMediaSession.RESULT_NOT_SUPPORTED));

        // ImsMediaManager is not connected
        mVideoSessionHandler.setVideoSession(null);
        mMediaManager.setImsMediaConnected(false);
        testParcel.setDataPosition(0);
        mMediaListener.onMediaMessage(testParcel);
        verify(mMockVideoSessionCallbackHandler)
                .openSessionResponse(eq(ImsMediaSession.RESULT_NOT_READY));

        // OpenSession Success Received.
        when(mMockVideoSession.getSessionId()).thenReturn(SESSION_ID);
        mVideoSessionCallback.onOpenSessionSuccess(mMockVideoSession);
        verify(mMockVideoSessionCallbackHandler)
                .openSessionResponse(eq(ImsMediaSession.RESULT_SUCCESS));
        assertEquals(mVideoSessionHandler.getVideoSessionId(), SESSION_ID);

        // VideoSession null object received in OpenSession Success.
        mVideoSessionCallback.onOpenSessionSuccess(null);
        verify(mMockVideoSessionCallbackHandler)
                .openSessionResponse(eq(ImsMediaSession.RESULT_NO_MEMORY));

        // OpenSession Failure Received
        mVideoSessionCallback.onOpenSessionFailure(ImsMediaSession.RESULT_NO_RESOURCES);
        verify(mMockVideoSessionCallbackHandler)
                .openSessionResponse(eq(ImsMediaSession.RESULT_NO_RESOURCES));
    }

    @Test
    public void testCloseSession() {
        Parcel testParcel = Parcel.obtain();
        testParcel.writeInt(MediaConstants.REQUEST_CLOSE_SESSION);
        testParcel.writeInt(ImsMediaSession.SESSION_TYPE_VIDEO);
        testParcel.setDataPosition(0);
        mMediaListener.onMediaMessage(testParcel);
        verify(mMockImsMediaManager).closeSession(eq(mMockVideoSession));
    }

    @Test
    public void testSessionChanged() {
        mVideoSessionCallback.onSessionChanged(ImsMediaSession.SESSION_STATE_ACTIVE);
        verify(mMockVideoSessionCallbackHandler)
                .sessionChanged(eq(ImsMediaSession.SESSION_STATE_ACTIVE));
    }

    @Test
    public void testModifySession() {
        // Modify Session Request
        VideoConfig videoConfig = MediaTestUtils.createVideoConfig();
        Parcel testParcel = Parcel.obtain();
        testParcel.writeInt(MediaConstants.REQUEST_MODIFY_SESSION);
        testParcel.writeInt(ImsMediaSession.SESSION_TYPE_VIDEO);
        videoConfig.writeToParcel(testParcel, Parcelable.PARCELABLE_WRITE_RETURN_VALUE);
        testParcel.setDataPosition(0);
        mMediaListener.onMediaMessage(testParcel);
        verify(mMockVideoSession).modifySession(eq(videoConfig));

        // Modify Session Response - SUCCESS
        mVideoSessionCallback.onModifySessionResponse(videoConfig, RESULT_SUCCESS);
        verify(mMockVideoSessionCallbackHandler)
                .modifySessionResponse(eq(videoConfig), eq(RESULT_SUCCESS));

        // Modify Session Response - FAILURE
        mVideoSessionCallback.onModifySessionResponse(videoConfig, RESULT_FAILURE);
        verify(mMockVideoSessionCallbackHandler)
                .modifySessionResponse(eq(videoConfig), eq(RESULT_FAILURE));
    }

    @Test
    public void testFirstMediaPacketReceived() {
        VideoConfig videoConfig = MediaTestUtils.createVideoConfig();
        mVideoSessionCallback.onFirstMediaPacketReceived(videoConfig);
        verify(mMockVideoSessionCallbackHandler).firstMediaPacketReceived(eq(videoConfig));
    }

    @Test
    public void testPeerDimensionChanged() {
        mVideoSessionCallback.onPeerDimensionChanged(
                MediaTestUtils.RESOLUTION_WIDTH, MediaTestUtils.RESOLUTION_HEIGHT);
        verify(mMockVideoSessionCallbackHandler)
                .peerDimensionChanged(
                        eq(MediaTestUtils.RESOLUTION_WIDTH), eq(MediaTestUtils.RESOLUTION_HEIGHT));
    }

    @Test
    public void testSetMediaQualityThreshold() {
        // Set Media Quality Threshold
        MediaQualityThreshold threshold = MediaTestUtils.createMediaQualityThreshold();
        Parcel testParcel = Parcel.obtain();
        testParcel.writeInt(MediaConstants.REQUEST_SET_MEDIA_QUALITY);
        testParcel.writeInt(ImsMediaSession.SESSION_TYPE_VIDEO);
        threshold.writeToParcel(testParcel, Parcelable.PARCELABLE_WRITE_RETURN_VALUE);
        testParcel.setDataPosition(0);
        mMediaListener.onMediaMessage(testParcel);
        verify(mMockVideoSession).setMediaQualityThreshold(eq(threshold));
    }

    @Test
    public void testMediaInactivityNotifications() {
        // Receive RTP Inactivity Notification
        mVideoSessionCallback.notifyMediaInactivity(RTP);
        verify(mMockVideoSessionCallbackHandler).onNotifyMediaInactivity(eq(RTP));

        // Receive RTCP Inactivity Notification
        mVideoSessionCallback.notifyMediaInactivity(RTCP);
        verify(mMockVideoSessionCallbackHandler).onNotifyMediaInactivity(eq(RTCP));
    }

    @Test
    public void testMediaQualityNotifications() {
        // Receive Packet Loss Notification
        mVideoSessionCallback.notifyPacketLoss(MediaTestUtils.PACKET_LOSS_PERCENT);
        verify(mMockVideoSessionCallbackHandler)
                .onNotifyPacketLoss(eq(MediaTestUtils.PACKET_LOSS_PERCENT));

        // Receive Video Data Usage Notification
        mVideoSessionCallback.notifyVideoDataUsage(MediaTestUtils.DATA_BYTES);
        verify(mMockVideoSessionCallbackHandler)
                .onNotifyVideoDataUsage(eq(MediaTestUtils.DATA_BYTES));
    }
}
