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
import static org.mockito.ArgumentMatchers.anyInt;
import static org.mockito.ArgumentMatchers.anyString;
import static org.mockito.ArgumentMatchers.eq;
import static org.mockito.Mockito.times;
import static org.mockito.Mockito.verify;
import static org.mockito.Mockito.when;

import android.os.Looper;
import android.os.Parcel;
import android.os.Parcelable;
import android.telephony.imsmedia.ImsMediaSession;
import android.telephony.imsmedia.ImsVideoSession;
import android.telephony.imsmedia.MediaQualityThreshold;
import android.telephony.imsmedia.VideoConfig;
import android.telephony.imsmedia.VideoSessionCallback;
import android.testing.AndroidTestingRunner;
import android.testing.TestableLooper;

import org.junit.After;
import org.junit.Before;
import org.junit.Test;
import org.junit.runner.RunWith;
import org.mockito.Mock;

@RunWith(AndroidTestingRunner.class)
@TestableLooper.RunWithLooper
public class VideoSessionHandlerTest extends MediaSessionHandlerTest {
    // open Session Parameters
    private static final String LOCAL_RTP_ADDRESS = "222.22.22.22";
    private static final int LOCAL_RTP_PORT = 50050;
    private static final String REMOTE_RTP_ADDRESS = "fg01:cdcd:abef:6fee::1";
    private static final int REMOTE_RTP_PORT = 1240;
    private static final int SESSION_ID = 12;


    // Mock Objects
    @Mock VideoSessionCallbackHandler mMockVideoSessionCallbackHandler;
    @Mock ImsVideoSession mMockVideoSession;

    private VideoSessionHandler mVideoSessionHandler;
    private VideoSessionCallback mVideoSessionCallback;

    @Before
    public void setUp() throws Exception {
        super.setUp(getClass().getSimpleName());

        // create the instance to test
        mVideoSessionHandler = new VideoSessionHandler(mMockBaseContext, mMediaManager,
                mMockMtcMediaSession, mMockVideoSessionCallbackHandler,
                mMockVideoSession, Looper.myLooper());
        mMediaSession.setVideoSessionHandler(mVideoSessionHandler);
        mVideoSessionCallback = mVideoSessionHandler.getVideoSessionCallback();
    }

    @After
    public void tearDown() throws Exception {
        mVideoSessionHandler = null;
        super.tearDown();
    }

    @Test
    public void testOpenSessionSuccess() {

        Parcel testParcel = Parcel.obtain();
        testParcel.writeInt(MediaConstants.REQUEST_OPEN_SESSION);
        testParcel.writeInt(ImsMediaSession.SESSION_TYPE_VIDEO);
        testParcel.writeString(LOCAL_RTP_ADDRESS);
        testParcel.writeInt(LOCAL_RTP_PORT);
        testParcel.setDataPosition(0);
        // OpenSession Received
        mVideoSessionHandler.setVideoSession(null);
        mMediaManager.setImsMediaConnected(true);
        mVideoSessionHandler.setVideoQosAgent(mMockQosAgent);
        mMediaListener.onMediaMessage(testParcel);
        processAllMessages();
        verify(mMockImsMediaManager).openSession(eq(mMockRtpSocket), eq(mMockRtpSocket),
                eq(ImsMediaSession.SESSION_TYPE_VIDEO), eq(null), eq(mMockExecutor),
                eq(mVideoSessionCallback));
        testParcel.recycle();
    }

    @Test
    public void testOpenSessionFailures() {

        Parcel testParcel = Parcel.obtain();
        testParcel.writeInt(MediaConstants.REQUEST_OPEN_SESSION);
        testParcel.writeInt(ImsMediaSession.SESSION_TYPE_VIDEO);
        testParcel.writeString(LOCAL_RTP_ADDRESS);
        testParcel.writeInt(LOCAL_RTP_PORT);
        testParcel.setDataPosition(0);

        // OpenSession Received, but QosConnection returned null
        mVideoSessionHandler.setVideoSession(null);
        mMediaManager.setImsMediaConnected(true);
        when(mMockQosAgent.createQosConnection(anyString(), anyInt()))
                .thenReturn(null);
        mVideoSessionHandler.setVideoQosAgent(mMockQosAgent);
        mMediaListener.onMediaMessage(testParcel);
        processAllMessages();
        verify(mMockVideoSessionCallbackHandler).openSessionResponse(
                eq(ImsMediaSession.RESULT_PORT_UNAVAILABLE));

        // ImsMediaManager is not connected
        mVideoSessionHandler.setVideoSession(null);
        mMediaManager.setImsMediaConnected(false);
        testParcel.setDataPosition(0);
        mMediaListener.onMediaMessage(testParcel);
        processAllMessages();
        verify(mMockVideoSessionCallbackHandler)
                .openSessionResponse(eq(ImsMediaSession.RESULT_NOT_READY));

        /**
         * VideoSession was opened already, but OpenVideoSession requested again
         * for same session
         */
        mVideoSessionHandler.setVideoSession(mMockVideoSession);
        testParcel.setDataPosition(0);
        mMediaListener.onMediaMessage(testParcel);
        processAllMessages();
        verify(mMockVideoSessionCallbackHandler)
            .openSessionResponse(eq(ImsMediaSession.RESULT_NOT_SUPPORTED));
        testParcel.recycle();
    }

    @Test
    public void testOpenSessionResponses() {
        // OpenSession Success Received.
        when(mMockVideoSession.getSessionId()).thenReturn(SESSION_ID);
        mVideoSessionCallback.onOpenSessionSuccess(mMockVideoSession);
        processAllMessages();
        verify(mMockVideoSessionCallbackHandler)
                .openSessionResponse(eq(ImsMediaSession.RESULT_SUCCESS));
        assertEquals(mVideoSessionHandler.getVideoSessionId(), SESSION_ID);

        // VideoSession null object received in OpenSession Success.
        mVideoSessionCallback.onOpenSessionSuccess(null);
        processAllMessages();
        verify(mMockVideoSessionCallbackHandler)
                .openSessionResponse(eq(ImsMediaSession.RESULT_NO_MEMORY));

        // OpenSession Failure Received
        mVideoSessionCallback.onOpenSessionFailure(ImsMediaSession.RESULT_NO_RESOURCES);
        processAllMessages();
        verify(mMockVideoSessionCallbackHandler)
                .openSessionResponse(eq(ImsMediaSession.RESULT_NO_RESOURCES));
    }

    @Test
    public void testCloseSession() {
        Parcel testParcel = Parcel.obtain();
        testParcel.writeInt(MediaConstants.REQUEST_CLOSE_SESSION);
        testParcel.writeInt(ImsMediaSession.SESSION_TYPE_VIDEO);
        testParcel.setDataPosition(0);
        mVideoSessionHandler.setVideoQosAgent(mMockQosAgent);
        mVideoSessionHandler.setRtpSocket(mRtpSocketPair);
        mVideoSessionHandler.setMediaState(MediaState.MEDIA_STATE_LIVE);
        mMediaListener.onMediaMessage(testParcel);
        processAllMessages();

        mVideoSessionHandler.setMediaState(MediaState.MEDIA_STATE_CLOSED);
        testParcel.setDataPosition(0);
        mMediaListener.onMediaMessage(testParcel);
        processAllMessages();
        verify(mMockImsMediaManager, times(1)).closeSession(eq(mMockVideoSession));
        testParcel.recycle();
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
        mVideoSessionHandler.setMediaState(MediaState.MEDIA_STATE_LIVE);
        mMediaListener.onMediaMessage(testParcel);
        processAllMessages();

        testParcel.setDataPosition(0);
        mVideoSessionHandler.setMediaState(MediaState.MEDIA_STATE_CLOSED);
        mMediaListener.onMediaMessage(testParcel);
        processAllMessages();
        verify(mMockVideoSession, times(1)).modifySession(eq(videoConfig));

        // Modify Session Response - SUCCESS
        mVideoSessionCallback.onModifySessionResponse(videoConfig, RESULT_SUCCESS);
        processAllMessages();
        verify(mMockVideoSessionCallbackHandler)
                .modifySessionResponse(eq(videoConfig), eq(RESULT_SUCCESS));

        // Modify Session Response - FAILURE
        mVideoSessionCallback.onModifySessionResponse(videoConfig, RESULT_FAILURE);
        processAllMessages();
        verify(mMockVideoSessionCallbackHandler)
                .modifySessionResponse(eq(videoConfig), eq(RESULT_FAILURE));
        testParcel.recycle();
    }

    @Test
    public void testRequestQoS() {
        // QoS Request
        Parcel testParcel = Parcel.obtain();
        testParcel.writeInt(MediaConstants.REQUEST_QOS);
        testParcel.writeInt(ImsMediaSession.SESSION_TYPE_VIDEO);
        testParcel.writeString(REMOTE_RTP_ADDRESS);
        testParcel.writeInt(REMOTE_RTP_PORT);
        testParcel.setDataPosition(0);
        mVideoSessionHandler.setVideoQosAgent(mMockQosAgent);
        mVideoSessionHandler.setMediaState(MediaState.MEDIA_STATE_LIVE);
        mVideoSessionHandler.setRtpSocket(mRtpSocketPair);
        mMediaListener.onMediaMessage(testParcel);
        processAllMessages();
        verify(mMockQosAgent).updateQosConnection(eq(mMockRtpSocket), eq(mMockRtpSocket),
                eq(REMOTE_RTP_ADDRESS), eq(REMOTE_RTP_PORT));
        testParcel.recycle();
    }

    @Test
    public void testFirstMediaPacketReceived() {
        VideoConfig videoConfig = MediaTestUtils.createVideoConfig();
        mVideoSessionCallback.onFirstMediaPacketReceived(videoConfig);
        processAllMessages();
        verify(mMockVideoSessionCallbackHandler).firstMediaPacketReceived(eq(videoConfig));
    }

    @Test
    public void testPeerDimensionChanged() {
        mVideoSessionCallback.onPeerDimensionChanged(
                MediaTestUtils.RESOLUTION_WIDTH, MediaTestUtils.RESOLUTION_HEIGHT);
        processAllMessages();
        verify(mMockMtcMediaSession).peerDimensionChanged(
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
        mVideoSessionHandler.setMediaState(MediaState.MEDIA_STATE_LIVE);
        mMediaListener.onMediaMessage(testParcel);
        processAllMessages();
        verify(mMockVideoSession).setMediaQualityThreshold(eq(threshold));
        testParcel.recycle();
    }

    @Test
    public void testMediaInactivityNotifications() {
        // Receive RTP Inactivity Notification
        mVideoSessionCallback.notifyMediaInactivity(RTP);
        processAllMessages();
        verify(mMockVideoSessionCallbackHandler).onNotifyMediaInactivity(eq(RTP));

        // Receive RTCP Inactivity Notification
        mVideoSessionCallback.notifyMediaInactivity(RTCP);
        processAllMessages();
        verify(mMockVideoSessionCallbackHandler).onNotifyMediaInactivity(eq(RTCP));
    }

    @Test
    public void testMediaQualityNotifications() {
        // Receive Packet Loss Notification
        mVideoSessionCallback.notifyBitrate(MediaTestUtils.VIDEO_BITRATE_BPS);
        processAllMessages();
        verify(mMockVideoSessionCallbackHandler)
                .onNotifyBitrate(eq(MediaTestUtils.VIDEO_BITRATE_BPS));

        // Receive Video Data Usage Notification
        mVideoSessionCallback.notifyVideoDataUsage(MediaTestUtils.DATA_BYTES);
        processAllMessages();
        verify(mMockMtcMediaSession)
                .onNotifyVideoDataUsage(eq(MediaTestUtils.DATA_BYTES));
    }

    @Test
    public void testMediaDetach() {
        Parcel testParcel = Parcel.obtain();
        testParcel.writeInt(MediaConstants.NOTIFY_MEDIA_DETACH);
        testParcel.writeInt(ImsMediaSession.SESSION_TYPE_VIDEO);
        testParcel.setDataPosition(0);
        mVideoSessionHandler.setVideoQosAgent(mMockQosAgent);
        mVideoSessionHandler.setRtpSocket(mRtpSocketPair);
        mVideoSessionHandler.setMediaState(MediaState.MEDIA_STATE_LIVE);
        mMediaListener.onMediaMessage(testParcel);
        processAllMessages();

        verify(mMockQosAgent,
                times(1)).destroyQosConnection(eq(mMockRtpSocket), eq(mMockRtpSocket));
        assertEquals(mVideoSessionHandler.getMediaState(), MediaState.MEDIA_STATE_IDLE);
        testParcel.recycle();
    }
}
