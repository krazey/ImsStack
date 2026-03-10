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

import static org.junit.Assert.assertFalse;
import static org.junit.Assert.assertEquals;
import static org.junit.Assert.assertNull;
import static org.junit.Assert.assertTrue;
import static org.mockito.ArgumentMatchers.any;
import static org.mockito.ArgumentMatchers.anyBoolean;
import static org.mockito.ArgumentMatchers.anyInt;
import static org.mockito.ArgumentMatchers.anyString;
import static org.mockito.ArgumentMatchers.eq;
import static org.mockito.Mockito.never;
import static org.mockito.Mockito.times;
import static org.mockito.Mockito.spy;
import static org.mockito.Mockito.timeout;
import static org.mockito.Mockito.verify;
import static org.mockito.Mockito.when;

import android.os.Looper;
import android.os.Parcel;
import android.os.Parcelable;
import android.telephony.imsmedia.ImsMediaSession;
import android.telephony.imsmedia.ImsVideoSession;
import android.telephony.imsmedia.MediaQualityThreshold;
import android.telephony.imsmedia.RtpReceptionStats;
import android.telephony.imsmedia.VideoConfig;
import android.telephony.imsmedia.VideoSessionCallback;
import android.testing.AndroidTestingRunner;
import android.testing.TestableLooper;
import android.util.Pair;
import android.view.Surface;

import com.android.imsstack.core.agents.QosAgent.ImsQosCallback;

import org.junit.After;
import org.junit.Before;
import org.junit.Test;
import org.junit.runner.RunWith;
import org.mockito.Mock;

import java.net.DatagramSocket;
import java.net.InetAddress;
import java.net.InetSocketAddress;

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
    @Mock Surface mMockSurface;

    private VideoSessionHandler mVideoSessionHandler;
    private VideoSessionCallback mVideoSessionCallback;

    @Before
    public void setUp() throws Exception {
        super.setUp(getClass().getSimpleName());

        // create the instance to test
        mVideoSessionHandler = new VideoSessionHandler(mMockBaseContext, mMediaManager,
                mMockMtcMediaSession, mMockVideoSessionCallbackHandler,
                mMockVideoSession, Looper.myLooper(), mMockQosAgent);
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

        // ImsMediaManager is not ready
        mVideoSessionHandler.setVideoSession(null);
        MediaManagerHelper.setImsMediaConnected(false);
        MediaManagerHelper spyMediaManager = spy(mMediaManager);
        mVideoSessionHandler = new VideoSessionHandler(mMockBaseContext, spyMediaManager,
                mMockMtcMediaSession, mMockVideoSessionCallbackHandler,
                null, Looper.myLooper(), mMockQosAgent);
        mMediaSession.setVideoSessionHandler(mVideoSessionHandler);

        testParcel.setDataPosition(0);
        mMediaListener.onMediaMessage(testParcel);
        processAllMessages();
        verify(spyMediaManager).waitForConnection(eq((long) MediaConstants.SERVICE_WAIT_TIMEOUT));
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
    public void testOpenSessionFailureWhenNoRtpsocket() {

        Parcel testParcel = Parcel.obtain();
        testParcel.writeString(LOCAL_RTP_ADDRESS);
        testParcel.writeInt(LOCAL_RTP_PORT);
        testParcel.setDataPosition(0);

        // OpenSession Received, but Rtp socket is null
        mVideoSessionHandler.setVideoSession(null);
        mMediaManager.setImsMediaConnected(true);
        testParcel.setDataPosition(0);
        Pair<DatagramSocket, DatagramSocket> rtpSocketPair = new Pair<>(null, null);
        when(mMockQosAgent.createQosConnection(LOCAL_RTP_ADDRESS, LOCAL_RTP_PORT))
                .thenReturn(rtpSocketPair);
        mVideoSessionHandler.onImsMediaVideoMessage(
                MediaConstants.REQUEST_OPEN_SESSION, testParcel);
        processAllMessages();

        verify(mMockVideoSessionCallbackHandler).openSessionResponse(
                eq(ImsMediaSession.RESULT_PORT_UNAVAILABLE));
        assertTrue(mVideoSessionHandler.isIdle());
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

        verify(mMockImsMediaManager, times(1)).closeSession(eq(mMockVideoSession));
        assertEquals(MediaState.MEDIA_STATE_CLOSED, mVideoSessionHandler.getMediaState());

        // Receive onSessionClosed callback
        mVideoSessionCallback.onSessionClosed();
        processAllMessages();

        // Verify timeout is cancelled and session is closed
        assertFalse(mVideoSessionHandler.getVideoMessageHandler().hasMessages(
                MediaConstants.RESPONSE_SESSION_CLOSED_TIMEOUT));
        verify(mMockQosAgent,
                times(1)).destroyQosConnection(eq(mMockRtpSocket), eq(mMockRtpSocket));
        assertEquals(mVideoSessionHandler.getMediaState(), MediaState.MEDIA_STATE_IDLE);
        testParcel.recycle();
    }

    @Test
    public void testCloseSessionWithTimeout() {
        Parcel testParcel = Parcel.obtain();
        testParcel.writeInt(MediaConstants.REQUEST_CLOSE_SESSION);
        testParcel.writeInt(ImsMediaSession.SESSION_TYPE_VIDEO);
        testParcel.setDataPosition(0);
        mVideoSessionHandler.setVideoQosAgent(mMockQosAgent);
        mVideoSessionHandler.setRtpSocket(mRtpSocketPair);
        mVideoSessionHandler.setMediaState(MediaState.MEDIA_STATE_LIVE);
        mMediaListener.onMediaMessage(testParcel);
        processAllMessages();

        verify(mMockImsMediaManager, times(1)).closeSession(eq(mMockVideoSession));
        assertEquals(MediaState.MEDIA_STATE_CLOSED, mVideoSessionHandler.getMediaState());

        // Do not receive onSessionClosed, and timeout scheduled for 5000ms is fired
        moveTimeForward(MediaConstants.RESPONSE_WAIT_TIMEOUT);
        processAllMessages();
        verify(mMockQosAgent,
                times(1)).destroyQosConnection(eq(mMockRtpSocket), eq(mMockRtpSocket));
        assertEquals(mVideoSessionHandler.getMediaState(), MediaState.MEDIA_STATE_IDLE);
        testParcel.recycle();
    }

    @Test
    public void testOnSessionClosed() {
        mVideoSessionHandler.setVideoQosAgent(mMockQosAgent);
        mVideoSessionHandler.setRtpSocket(mRtpSocketPair);
        mVideoSessionCallback.onSessionClosed();
        processAllMessages();

        verify(mMockQosAgent,
            times(1)).destroyQosConnection(eq(mMockRtpSocket), eq(mMockRtpSocket));
        assertTrue(mVideoSessionHandler.isIdle());
        assertEquals(mVideoSessionHandler.getVideoSessionId(), 0);
        assertNull(mVideoSessionHandler.getVideoSession());
    }

    @Test
    public void testRequestIgnoredWhenClosing() throws Exception {
        Parcel testParcel = Parcel.obtain();
        testParcel.setDataPosition(0);
        mVideoSessionHandler.setRtpSocket(mRtpSocketPair);
        mVideoSessionHandler.setMediaState(MediaState.MEDIA_STATE_LIVE);
        // Close session
        mVideoSessionHandler.onImsMediaVideoMessage(
                MediaConstants.REQUEST_CLOSE_SESSION, testParcel);
        processAllMessages();

        verify(mMockImsMediaManager, times(1)).closeSession(eq(mMockVideoSession));
        assertEquals(MediaState.MEDIA_STATE_CLOSED, mVideoSessionHandler.getMediaState());

        // Try to modify session, should be discarded
        VideoConfig videoConfig = MediaTestUtils.createVideoConfig();
        Parcel testParcel2 = Parcel.obtain();
        videoConfig.writeToParcel(testParcel2, Parcelable.PARCELABLE_WRITE_RETURN_VALUE);
        testParcel2.setDataPosition(0);
        mVideoSessionHandler.onImsMediaVideoMessage(
                MediaConstants.REQUEST_MODIFY_SESSION, testParcel2);
        processAllMessages();

        // Verify modifySession was not called
        verify(mMockVideoSession, never()).modifySession(any());
        testParcel.recycle();
        testParcel2.recycle();
    }

    @Test
    public void testModifySession() {
        // Modify Session Request
        VideoConfig videoConfig = MediaTestUtils.createVideoConfig();
        Parcel testParcel = Parcel.obtain();
        videoConfig.writeToParcel(testParcel, Parcelable.PARCELABLE_WRITE_RETURN_VALUE);
        testParcel.setDataPosition(0);
        mVideoSessionHandler.setMediaState(MediaState.MEDIA_STATE_LIVE);
        mVideoSessionHandler.onImsMediaVideoMessage(
                MediaConstants.REQUEST_MODIFY_SESSION, testParcel);
        processAllMessages();
        verify(mMockVideoSession, times(1)).modifySession(eq(videoConfig));
        assertEquals(MediaState.MEDIA_STATE_LIVE, mVideoSessionHandler.getMediaState());

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
    public void testModifySessionWhenNoVideoSession_shouldWait() throws InterruptedException {
        // Modify Session Request
        VideoConfig videoConfig = MediaTestUtils.createVideoConfig();
        Parcel testParcel = Parcel.obtain();
        videoConfig.writeToParcel(testParcel, Parcelable.PARCELABLE_WRITE_RETURN_VALUE);
        testParcel.setDataPosition(0);
        mVideoSessionHandler.setMediaState(MediaState.MEDIA_STATE_OPENING);
        mVideoSessionHandler.setVideoSession(null);
        mVideoSessionHandler.onImsMediaVideoMessage(
                MediaConstants.REQUEST_MODIFY_SESSION, testParcel);
        processAllMessages();

        verify(mMockVideoSession, timeout(500).times(0)).modifySession(eq(videoConfig));
        verify(mMockVideoSessionCallbackHandler).openSessionResponse(
                eq(ImsMediaSession.RESULT_NOT_READY));
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
                eq(REMOTE_RTP_ADDRESS), eq(REMOTE_RTP_PORT), eq(false));
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

    @Test
    public void testOnNotifyQosConnectionAvailable() throws Exception {
        InetSocketAddress remoteAddress =
                new InetSocketAddress(
                        InetAddress.getByName(MediaTestUtils.REMOTE_RTP_ADDRESS),
                        MediaTestUtils.REMOTE_RTP_PORT);
        ImsQosCallback callback = mVideoSessionHandler.getVideoImsQosCallback();
        callback.onNotifyQosConnectionAvailable(remoteAddress);

        verify(mMockVideoSessionCallbackHandler)
                .onNotifyQosInfo(
                        eq(MediaTestUtils.REMOTE_RTP_ADDRESS),
                        eq(MediaTestUtils.REMOTE_RTP_PORT),
                        eq(true));
    }

    @Test
    public void testOnNotifyQosConnectionLost() throws Exception {
        InetSocketAddress remoteAddress =
                new InetSocketAddress(
                        InetAddress.getByName(MediaTestUtils.REMOTE_RTP_ADDRESS),
                        MediaTestUtils.REMOTE_RTP_PORT);
        ImsQosCallback callback = mVideoSessionHandler.getVideoImsQosCallback();
        callback.onNotifyQosConnectionLost(remoteAddress);

        verify(mMockVideoSessionCallbackHandler)
                .onNotifyQosInfo(
                        eq(MediaTestUtils.REMOTE_RTP_ADDRESS),
                        eq(MediaTestUtils.REMOTE_RTP_PORT),
                        eq(false));
    }

    @Test
    public void testVideoImsQosCallbackWithNullVideoSessionHandler() {
        // clear the instance created in setup
        mVideoSessionHandler = null;
        // create the instance to test
        mVideoSessionHandler = new VideoSessionHandler(mMockBaseContext, mMediaManager,
                mMockMtcMediaSession, null, mMockVideoSession, Looper.myLooper(), mMockQosAgent);
        ImsQosCallback callback = mVideoSessionHandler.getVideoImsQosCallback();
        callback.onNotifyQosConnectionAvailable(null);
        callback.onNotifyQosConnectionLost(null);

        verify(mMockVideoSessionCallbackHandler, never())
                .onNotifyQosInfo(anyString(), anyInt(), anyBoolean());
    }

    @Test
    public void testGetSamplingRateKHz() {
        assertEquals(90, mVideoSessionHandler.getSamplingRateKHz());
    }

    @Test
    public void testNotifyRtpReceptionStats() {
        RtpReceptionStats videoStats =
                new RtpReceptionStats.Builder()
                        .setRtpTimestamp(10)
                        .setRtcpSrTimestamp(20)
                        .setRtcpSrNtpTimestamp(200)
                        .setJitterBufferMs(200)
                        .setRoundTripTimeMs(20)
                        .build();
        mVideoSessionCallback.notifyRtpReceptionStats(videoStats);
        processAllMessages();

        verify(mMockVideoSessionCallbackHandler).onNotifyRtpReceptionStats(eq(videoStats));
    }

    @Test
    public void testVideoDataUsageRequest() {
        // Video Data Usage Request
        mVideoSessionHandler.onImsMediaVideoMessage(MediaConstants.REQUEST_VIDEO_DATA_USAGE, null);
        processAllMessages();

        verify(mMockVideoSession, times(1)).requestVideoDataUsage();
    }

    @Test
    public void testSetPreviewSurface() {
        when(mMockMtcMediaSession.getPreviewSurface()).thenReturn(mMockSurface);
        mVideoSessionHandler.onImsMediaVideoMessage(
                MediaConstants.REQUEST_SET_PREVIEW_SURFACE, null);
        processAllMessages();

        verify(mMockVideoSession, times(1)).setPreviewSurface(any(Surface.class));
        assertTrue(mVideoSessionHandler.isPreviewSurfaceSet());
    }

    @Test
    public void testSetPreviewSurfaceWithNoVideoSession() {
        when(mMockMtcMediaSession.getPreviewSurface()).thenReturn(mMockSurface);
        // clear the instance created in setup
        mVideoSessionHandler = null;
        // create the instance to test
        mVideoSessionHandler = new VideoSessionHandler(mMockBaseContext, mMediaManager,
                null, mMockVideoSessionCallbackHandler, mMockVideoSession,
                Looper.myLooper(), mMockQosAgent);
        mVideoSessionHandler.onImsMediaVideoMessage(
                MediaConstants.REQUEST_SET_PREVIEW_SURFACE, null);
        processAllMessages();

        verify(mMockVideoSession, never()).setPreviewSurface(any(Surface.class));
        assertFalse(mVideoSessionHandler.isPreviewSurfaceSet());
    }

    @Test
    public void testSetDisplaySurface() {
        when(mMockMtcMediaSession.getDisplaySurface()).thenReturn(mMockSurface);
        mVideoSessionHandler.onImsMediaVideoMessage(
                MediaConstants.REQUEST_SET_DISPLAY_SURFACE, null);
        processAllMessages();

        verify(mMockVideoSession, times(1)).setDisplaySurface(any(Surface.class));
        assertTrue(mVideoSessionHandler.isDisplaySurfaceSet());
    }

    @Test
    public void testSetDisplaySurfaceWithNoVideoSession() {
        when(mMockMtcMediaSession.getDisplaySurface()).thenReturn(mMockSurface);
        // clear the instance created in setup
        mVideoSessionHandler = null;
        // create the instance to test
        mVideoSessionHandler = new VideoSessionHandler(mMockBaseContext, mMediaManager,
            null, mMockVideoSessionCallbackHandler, mMockVideoSession,
            Looper.myLooper(), mMockQosAgent);
        mVideoSessionHandler.onImsMediaVideoMessage(
                MediaConstants.REQUEST_SET_DISPLAY_SURFACE, null);
        processAllMessages();

        verify(mMockVideoSession, never()).setDisplaySurface(any(Surface.class));
        assertFalse(mVideoSessionHandler.isDisplaySurfaceSet());
    }

    @Test
    public void testAdjustDelayRequest() {
        // Adjust Delay Request
        Parcel testParcel = Parcel.obtain();
        testParcel.writeInt(5);
        testParcel.setDataPosition(0);
        mVideoSessionHandler.onImsMediaVideoMessage(
                MediaConstants.REQUEST_ADJUST_DELAY, testParcel);
        processAllMessages();

        verify(mMockVideoSession, times(1)).adjustDelay(eq(5));
    }

    @Test
    public void testOnHeaderExtensionReceived() {
        mVideoSessionCallback.onHeaderExtensionReceived(null);
        // This method only logs a message,
        // so there is nothing to verify other than that it doesn't crash.
    }

    @Test
    public void testInvalidRequestType() {

        Parcel testParcel = Parcel.obtain();
        testParcel.writeString(LOCAL_RTP_ADDRESS);
        testParcel.writeInt(LOCAL_RTP_PORT);
        testParcel.setDataPosition(0);
        mVideoSessionHandler.onImsMediaVideoMessage(MediaConstants.IMSMEDIA_MAX, testParcel);
        processAllMessages();

        verify(mMockVideoSessionCallbackHandler, never()).openSessionResponse(anyInt());
    }

    @Test
    public void testRequestRtpReceptionStats() {
        final int testInterval = 3000;

        Parcel testParcel = Parcel.obtain();
        testParcel.writeInt(testInterval);
        testParcel.setDataPosition(0);

        // Set the session state to live
        mVideoSessionHandler.setMediaState(MediaState.MEDIA_STATE_LIVE);

        // Trigger the message handler
        mVideoSessionHandler.onImsMediaVideoMessage(
                MediaConstants.REQUEST_RTP_RECEPTION_STATS, testParcel);
        processAllMessages();

        // Verify that the ImsVideoSession method was called with the correct interval
        verify(mMockVideoSession).requestRtpReceptionStats(eq(testInterval));
    }
}
