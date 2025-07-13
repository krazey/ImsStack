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
import static org.mockito.ArgumentMatchers.any;
import static org.mockito.ArgumentMatchers.anyInt;
import static org.mockito.ArgumentMatchers.anyString;
import static org.mockito.ArgumentMatchers.eq;
import static org.mockito.Mockito.never;
import static org.mockito.Mockito.times;
import static org.mockito.Mockito.verify;
import static org.mockito.Mockito.when;

import android.os.Looper;
import android.os.Parcel;
import android.os.Parcelable;
import android.telephony.imsmedia.ImsMediaSession;
import android.telephony.imsmedia.ImsTextSession;
import android.telephony.imsmedia.MediaQualityThreshold;
import android.telephony.imsmedia.TextConfig;
import android.telephony.imsmedia.TextSessionCallback;
import android.testing.AndroidTestingRunner;
import android.testing.TestableLooper;

import org.junit.After;
import org.junit.Before;
import org.junit.Test;
import org.junit.runner.RunWith;
import org.mockito.Mock;

@RunWith(AndroidTestingRunner.class)
@TestableLooper.RunWithLooper
public class TextSessionHandlerTest extends MediaSessionHandlerTest {
    // open Session Parameters
    private static final String LOCAL_RTP_ADDRESS = "123.12.12.98";
    private static final int LOCAL_RTP_PORT = 50040;
    private static final String REMOTE_RTP_ADDRESS = "gf01:abab:dcef:6fee::1";
    private static final int REMOTE_RTP_PORT = 1000;
    private static final int SESSION_ID = 532;

    // Rtt settings
    private static final String RTT_MESSAGE = "Test Message";

    // Mock Objects
    @Mock TextSessionCallbackHandler mMockTextSessionCallbackHandler;
    @Mock ImsTextSession mMockTextSession;

    private TextSessionHandler mTextSessionHandler;
    private TextSessionCallback mTextSessionCallback;

    @Before
    public void setUp() throws Exception {
        super.setUp(getClass().getSimpleName());

        // create the instance to test
        mTextSessionHandler = new TextSessionHandler(mMockBaseContext, mMediaManager,
                mMockTextSessionCallbackHandler, mMockTextSession, Looper.myLooper());
        mMediaSession.setTextSessionHandler(mTextSessionHandler);
        mTextSessionCallback = mTextSessionHandler.getTextSessionCallback();
    }

    @After
    public void tearDown() throws Exception {
        super.tearDown();
    }

    @Test
    public void testOpenSessionSuccess() {

        Parcel testParcel = Parcel.obtain();
        testParcel.writeInt(MediaConstants.REQUEST_OPEN_SESSION);
        testParcel.writeInt(ImsMediaSession.SESSION_TYPE_RTT);
        testParcel.writeString(LOCAL_RTP_ADDRESS);
        testParcel.writeInt(LOCAL_RTP_PORT);
        testParcel.setDataPosition(0);
        // OpenSession Received
        mTextSessionHandler.setTextSession(null);
        mMediaManager.setImsMediaConnected(true);
        mTextSessionHandler.setTextQosAgent(mMockQosAgent);
        mMediaListener.onMediaMessage(testParcel);
        processAllMessages();
        verify(mMockImsMediaManager).openSession(eq(mMockRtpSocket), eq(mMockRtpSocket),
                eq(ImsMediaSession.SESSION_TYPE_RTT), eq(null), eq(mMockExecutor),
                eq(mTextSessionCallback));
        testParcel.recycle();
    }

    @Test
    public void testOpenSessionFailures() {

        Parcel testParcel = Parcel.obtain();
        testParcel.writeInt(MediaConstants.REQUEST_OPEN_SESSION);
        testParcel.writeInt(ImsMediaSession.SESSION_TYPE_RTT);
        testParcel.writeString(LOCAL_RTP_ADDRESS);
        testParcel.writeInt(LOCAL_RTP_PORT);
        testParcel.setDataPosition(0);

        // OpenSession Received, but QosConnection returned null
        mTextSessionHandler.setTextSession(null);
        mMediaManager.setImsMediaConnected(true);
        when(mMockQosAgent.createQosConnection(anyString(), anyInt()))
                .thenReturn(null);
        mTextSessionHandler.setTextQosAgent(mMockQosAgent);
        mMediaListener.onMediaMessage(testParcel);
        processAllMessages();
        verify(mMockTextSessionCallbackHandler).openSessionResponse(
                eq(ImsMediaSession.RESULT_PORT_UNAVAILABLE));

        // ImsMediaManager is not connected
        mTextSessionHandler.setTextSession(null);
        mMediaManager.setImsMediaConnected(false);
        testParcel.setDataPosition(0);
        mMediaListener.onMediaMessage(testParcel);
        processAllMessages();
        verify(mMockTextSessionCallbackHandler).openSessionResponse(
                eq(ImsMediaSession.RESULT_NOT_READY));

        /**
         * TextSession was opened already, but OpenTextSession requested again
         * for same session
         */
        mTextSessionHandler.setTextSession(mMockTextSession);
        testParcel.setDataPosition(0);
        mMediaListener.onMediaMessage(testParcel);
        processAllMessages();
        verify(mMockTextSessionCallbackHandler).openSessionResponse(
                eq(ImsMediaSession.RESULT_NOT_SUPPORTED));
        testParcel.recycle();
    }

    @Test
    public void testOpenSessionResponses() {
        // OpenSession Success Received.
        when(mMockTextSession.getSessionId()).thenReturn(SESSION_ID);
        mTextSessionCallback.onOpenSessionSuccess(mMockTextSession);
        processAllMessages();
        verify(mMockTextSessionCallbackHandler).openSessionResponse(
                eq(ImsMediaSession.RESULT_SUCCESS));
        assertEquals(mTextSessionHandler.getTextSessionId(), SESSION_ID);

        // TextSession null object received in OpenSession Success.
        mTextSessionCallback.onOpenSessionSuccess(null);
        processAllMessages();
        verify(mMockTextSessionCallbackHandler).openSessionResponse(
                eq(ImsMediaSession.RESULT_NO_MEMORY));

        // OpenSession Failure Received
        mTextSessionCallback.onOpenSessionFailure(ImsMediaSession.RESULT_NO_RESOURCES);
        processAllMessages();
        verify(mMockTextSessionCallbackHandler).openSessionResponse(
                eq(ImsMediaSession.RESULT_NO_RESOURCES));
    }

    @Test
    public void testCloseSession() {
        Parcel testParcel = Parcel.obtain();
        testParcel.writeInt(MediaConstants.REQUEST_CLOSE_SESSION);
        testParcel.writeInt(ImsMediaSession.SESSION_TYPE_RTT);
        testParcel.setDataPosition(0);
        mTextSessionHandler.setTextQosAgent(mMockQosAgent);
        mTextSessionHandler.setRtpSocket(mRtpSocketPair);
        mTextSessionHandler.setMediaState(MediaState.MEDIA_STATE_LIVE);
        mMediaListener.onMediaMessage(testParcel);
        processAllMessages();

        mTextSessionHandler.setMediaState(MediaState.MEDIA_STATE_CLOSED);
        testParcel.setDataPosition(0);
        mMediaListener.onMediaMessage(testParcel);
        processAllMessages();
        verify(mMockImsMediaManager, times(1)).closeSession(eq(mMockTextSession));
        testParcel.recycle();
    }

    @Test
    public void testRequestIgnoredWhenClosing() throws Exception {
        Parcel testParcel = Parcel.obtain();
        testParcel.setDataPosition(0);
        mTextSessionHandler.setRtpSocket(mRtpSocketPair);
        mTextSessionHandler.setMediaState(MediaState.MEDIA_STATE_LIVE);
        // Close session
        mTextSessionHandler.onImsMediaTextMessage(
                MediaConstants.REQUEST_CLOSE_SESSION, testParcel);
        processAllMessages();

        verify(mMockImsMediaManager, times(1)).closeSession(eq(mMockTextSession));
        assertEquals(MediaState.MEDIA_STATE_CLOSED, mTextSessionHandler.getMediaState());

        // Try to modify session, should be discarded
        TextConfig textConfig = MediaTestUtils.createTextConfig();
        Parcel testParcel2 = Parcel.obtain();
        textConfig.writeToParcel(testParcel2, Parcelable.PARCELABLE_WRITE_RETURN_VALUE);
        testParcel2.setDataPosition(0);
        mTextSessionHandler.onImsMediaTextMessage(
                MediaConstants.REQUEST_MODIFY_SESSION, testParcel2);
        processAllMessages();

        // Verify modifySession was not called
        verify(mMockTextSession, never()).modifySession(any());
        testParcel.recycle();
        testParcel2.recycle();
    }

    @Test
    public void testModifySession() {
        // Modify Session Request
        TextConfig textConfig = MediaTestUtils.createTextConfig();
        Parcel testParcel = Parcel.obtain();
        textConfig.writeToParcel(testParcel, Parcelable.PARCELABLE_WRITE_RETURN_VALUE);
        testParcel.setDataPosition(0);
        mTextSessionHandler.setMediaState(MediaState.MEDIA_STATE_LIVE);
        mTextSessionHandler.onImsMediaTextMessage(
                MediaConstants.REQUEST_MODIFY_SESSION, testParcel);
        processAllMessages();
        verify(mMockTextSession, times(1)).modifySession(eq(textConfig));
        assertEquals(MediaState.MEDIA_STATE_LIVE, mTextSessionHandler.getMediaState());

        // Modify Session Response - SUCCESS
        mTextSessionCallback.onModifySessionResponse(textConfig, RESULT_SUCCESS);
        processAllMessages();
        verify(mMockTextSessionCallbackHandler).modifySessionResponse(eq(textConfig),
                eq(RESULT_SUCCESS));

        // Modify Session Response - FAILURE
        mTextSessionCallback.onModifySessionResponse(textConfig, RESULT_FAILURE);
        processAllMessages();
        verify(mMockTextSessionCallbackHandler).modifySessionResponse(eq(textConfig),
                eq(RESULT_FAILURE));
        testParcel.recycle();
    }

    @Test
    public void testSendRtt() {
        // Send Rtt
        Parcel testParcel = Parcel.obtain();
        testParcel.writeInt(MediaConstants.REQUEST_SEND_RTT);
        testParcel.writeInt(ImsMediaSession.SESSION_TYPE_RTT);
        testParcel.writeString(RTT_MESSAGE);
        testParcel.setDataPosition(0);
        mTextSessionHandler.setMediaState(MediaState.MEDIA_STATE_LIVE);
        mMediaListener.onMediaMessage(testParcel);
        processAllMessages();

        testParcel.setDataPosition(0);
        mTextSessionHandler.setMediaState(MediaState.MEDIA_STATE_CLOSED);
        mMediaListener.onMediaMessage(testParcel);
        processAllMessages();
        verify(mMockTextSession, times(1)).sendRtt(eq(RTT_MESSAGE));
        testParcel.recycle();
    }

    @Test
    public void testRequestQoS() {
        // QoS Request
        Parcel testParcel = Parcel.obtain();
        testParcel.writeInt(MediaConstants.REQUEST_QOS);
        testParcel.writeInt(ImsMediaSession.SESSION_TYPE_RTT);
        testParcel.writeString(REMOTE_RTP_ADDRESS);
        testParcel.writeInt(REMOTE_RTP_PORT);
        testParcel.setDataPosition(0);
        mTextSessionHandler.setTextQosAgent(mMockQosAgent);
        mTextSessionHandler.setMediaState(MediaState.MEDIA_STATE_LIVE);
        mTextSessionHandler.setRtpSocket(mRtpSocketPair);
        mMediaListener.onMediaMessage(testParcel);
        processAllMessages();
        verify(mMockQosAgent).updateQosConnection(eq(mMockRtpSocket), eq(mMockRtpSocket),
                eq(REMOTE_RTP_ADDRESS), eq(REMOTE_RTP_PORT), eq(false));
        testParcel.recycle();
    }

    @Test
    public void testSetMediaQualityThreshold() {
        // Set Media Quality Threshold
        MediaQualityThreshold threshold = MediaTestUtils.createMediaQualityThreshold();
        Parcel testParcel = Parcel.obtain();
        testParcel.writeInt(MediaConstants.REQUEST_SET_MEDIA_QUALITY);
        testParcel.writeInt(ImsMediaSession.SESSION_TYPE_RTT);
        threshold.writeToParcel(testParcel, Parcelable.PARCELABLE_WRITE_RETURN_VALUE);
        testParcel.setDataPosition(0);
        mTextSessionHandler.setMediaState(MediaState.MEDIA_STATE_LIVE);
        mMediaListener.onMediaMessage(testParcel);
        processAllMessages();
        verify(mMockTextSession).setMediaQualityThreshold(eq(threshold));
        testParcel.recycle();
    }

    @Test
    public void testRttReceived() {
        // Receive Rtt Notification
        mTextSessionCallback.onRttReceived(RTT_MESSAGE);
        processAllMessages();
        verify(mMockTextSessionCallbackHandler).onRttReceived(eq(RTT_MESSAGE));
    }

    @Test
    public void testMediaInactivityNotifications() {
        // Receive RTP Inactivity Notification
        mTextSessionCallback.notifyMediaInactivity(RTP);
        processAllMessages();
        verify(mMockTextSessionCallbackHandler).onNotifyMediaInactivity(eq(RTP));

        // Receive RTCP Inactivity Notification
        mTextSessionCallback.notifyMediaInactivity(RTCP);
        processAllMessages();
        verify(mMockTextSessionCallbackHandler).onNotifyMediaInactivity(eq(RTCP));
    }

    @Test
    public void testMediaDetach() {
        Parcel testParcel = Parcel.obtain();
        testParcel.writeInt(MediaConstants.NOTIFY_MEDIA_DETACH);
        testParcel.writeInt(ImsMediaSession.SESSION_TYPE_RTT);
        testParcel.setDataPosition(0);
        mTextSessionHandler.setTextQosAgent(mMockQosAgent);
        mTextSessionHandler.setRtpSocket(mRtpSocketPair);
        mTextSessionHandler.setMediaState(MediaState.MEDIA_STATE_LIVE);
        mMediaListener.onMediaMessage(testParcel);
        processAllMessages();

        verify(mMockQosAgent,
                times(1)).destroyQosConnection(eq(mMockRtpSocket), eq(mMockRtpSocket));
        assertEquals(mTextSessionHandler.getMediaState(), MediaState.MEDIA_STATE_IDLE);
        testParcel.recycle();
    }
}
