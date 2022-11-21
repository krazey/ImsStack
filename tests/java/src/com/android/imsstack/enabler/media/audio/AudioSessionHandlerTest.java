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
import static org.junit.Assert.fail;
import static org.mockito.ArgumentMatchers.any;
import static org.mockito.Matchers.anyInt;
import static org.mockito.Matchers.anyString;
import static org.mockito.Matchers.eq;
import static org.mockito.Mockito.doNothing;
import static org.mockito.Mockito.times;
import static org.mockito.Mockito.verify;
import static org.mockito.Mockito.when;

import android.content.Context;
import android.os.Parcel;
import android.os.Parcelable;
import android.telephony.CallQuality;
import android.telephony.ims.RtpHeaderExtension;
import android.telephony.imsmedia.AudioConfig;
import android.telephony.imsmedia.AudioSessionCallback;
import android.telephony.imsmedia.ImsAudioSession;
import android.telephony.imsmedia.ImsMediaManager;
import android.telephony.imsmedia.ImsMediaSession;
import android.telephony.imsmedia.MediaQualityThreshold;
import android.testing.TestableLooper;
import android.util.Pair;

import com.android.imsstack.core.agents.QosAgent;
import com.android.imsstack.enabler.IBaseContext;
import com.android.imsstack.enabler.mtc.MtcMediaSession;

import org.junit.After;
import org.junit.Before;
import org.junit.Test;
import org.junit.runner.RunWith;
import org.junit.runners.JUnit4;
import org.mockito.Mock;
import org.mockito.MockitoAnnotations;

import java.net.DatagramSocket;
import java.util.List;
import java.util.concurrent.Executor;

@RunWith(JUnit4.class)
public class AudioSessionHandlerTest {
    // open Session Parameters
    private static final String LOCAL_RTP_ADDRESS = "111.11.11.11";
    private static final int LOCAL_RTP_PORT = 50010;
    private static final String REMOTE_RTP_ADDRESS = "fg01:abab:cdef:6fee::1";
    private static final int REMOTE_RTP_PORT = 1236;
    private static final int SESSION_ID = 439;

    // Session Operator Results
    private static final int RESULT_FAILURE = ImsMediaSession.RESULT_NO_RESOURCES;
    private static final int RESULT_SUCCESS = ImsMediaSession.RESULT_SUCCESS;

    // Packet Types
    private static final int RTP = ImsMediaSession.PACKET_TYPE_RTP;
    private static final int RTCP = ImsMediaSession.PACKET_TYPE_RTCP;

    // Dtmf settings
    private static final int DTMF_DURATION = 20;
    private static final char DTMF_DIGIT = '9';

    // Mock Objects
    @Mock Context mMockContext;
    @Mock IBaseContext mMockBaseContext;
    @Mock MtcMediaSession mMockMtcMediaSession;
    @Mock AudioSessionCallbackHandler mMockAudioSessionCallbackHandler;
    @Mock ImsAudioSession mMockAudioSession;
    @Mock ImsMediaManager mMockImsMediaManager;
    @Mock Executor mMockExecutor;
    @Mock QosAgent mMockQosAgent;
    @Mock DatagramSocket mMockRtpSocket;

    private AudioSessionHandler mAudioSessionHandler;
    private AudioSessionCallback mAudioSessionCallback;
    private MediaSession mMediaSession;
    private MediaManagerHelper mMediaManager;
    private IMediaListener mMediaListener;
    private AudioSessionHandler.AudioMessageHandler mHandler;
    private TestableLooper mLooper;
    private Pair<DatagramSocket, DatagramSocket> mRtpSocketPair;

    @Before
    public void setUp() throws Exception {
        //Initialize Mock Objects
        MockitoAnnotations.initMocks(this);
        when(mMockBaseContext.getContext()).thenReturn(mMockContext);
        mRtpSocketPair = new Pair<>(mMockRtpSocket, mMockRtpSocket);
        when(mMockQosAgent.createQosConnection(anyString(), anyInt()))
                .thenReturn(mRtpSocketPair);
        doNothing().when(mMockQosAgent).destroyQosConnection(any(), any());
        when(mMockQosAgent.updateQosConnection(any(), any(), anyString(), anyInt()))
                .thenReturn(true);

        // create the instance to test
        mMediaSession = new MediaSession(mMockBaseContext, mMockMtcMediaSession,
                mMockImsMediaManager, mMockExecutor);
        mMediaManager = mMediaSession.getMediaManager();
        mMediaListener = mMediaSession.getMediaListenerProxy();
        mAudioSessionHandler = new AudioSessionHandler(mMockBaseContext, mMediaManager,
                mMockAudioSessionCallbackHandler, mMockAudioSession);
        mMediaSession.setAudioSessionHandler(mAudioSessionHandler);
        mAudioSessionCallback = mAudioSessionHandler.getAudioSessionCallback();
        mHandler = mAudioSessionHandler.getAudioMessageHandler();
        try {
            mLooper = new TestableLooper(mHandler.getLooper());
        } catch (Exception e) {
            fail("Failed to create TestableLooper" + e);
        }
    }

    @After
    public void tearDown() throws Exception {
        if (mLooper != null) {
            mLooper.destroy();
            mLooper = null;
        }
    }

    @Test
    public void testOpenSessionSuccess() {

        Parcel testParcel = Parcel.obtain();
        testParcel.writeInt(MediaConstants.REQUEST_OPEN_SESSION);
        testParcel.writeInt(ImsMediaSession.SESSION_TYPE_AUDIO);
        testParcel.writeString(LOCAL_RTP_ADDRESS);
        testParcel.writeInt(LOCAL_RTP_PORT);
        testParcel.setDataPosition(0);
        // OpenSession Received
        mAudioSessionHandler.setAudioSession(null);
        mMediaManager.setImsMediaConnected(true);
        mAudioSessionHandler.setAudioQosAgent(mMockQosAgent);
        mMediaListener.onMediaMessage(testParcel);
        processAllMessages();
        verify(mMockImsMediaManager).openSession(eq(mMockRtpSocket), eq(mMockRtpSocket),
                eq(ImsMediaSession.SESSION_TYPE_AUDIO), eq(null), eq(mMockExecutor),
                eq(mAudioSessionCallback));
        testParcel.recycle();
    }

    @Test
    public void testOpenSessionFailures() {

        Parcel testParcel = Parcel.obtain();
        testParcel.writeInt(MediaConstants.REQUEST_OPEN_SESSION);
        testParcel.writeInt(ImsMediaSession.SESSION_TYPE_AUDIO);
        testParcel.writeString(LOCAL_RTP_ADDRESS);
        testParcel.writeInt(LOCAL_RTP_PORT);
        testParcel.setDataPosition(0);

        // OpenSession Received, but QosConnection returned null
        mAudioSessionHandler.setAudioSession(null);
        mMediaManager.setImsMediaConnected(true);
        when(mMockQosAgent.createQosConnection(anyString(), anyInt()))
                .thenReturn(null);
        mAudioSessionHandler.setAudioQosAgent(mMockQosAgent);
        mMediaListener.onMediaMessage(testParcel);
        processAllMessages();
        verify(mMockAudioSessionCallbackHandler).openSessionResponse(
                eq(ImsMediaSession.RESULT_PORT_UNAVAILABLE));

        // ImsMediaManager is not connected
        mAudioSessionHandler.setAudioSession(null);
        mMediaManager.setImsMediaConnected(false);
        testParcel.setDataPosition(0);
        mMediaListener.onMediaMessage(testParcel);
        processAllMessages();
        verify(mMockAudioSessionCallbackHandler).openSessionResponse(
                eq(ImsMediaSession.RESULT_NOT_READY));

        /**
         * AudioSession was opened already, but OpenAudioSession requested again
         * for same session
         */
        mAudioSessionHandler.setAudioSession(mMockAudioSession);
        testParcel.setDataPosition(0);
        mMediaListener.onMediaMessage(testParcel);
        processAllMessages();
        verify(mMockAudioSessionCallbackHandler).openSessionResponse(
                eq(ImsMediaSession.RESULT_NOT_SUPPORTED));

        testParcel.recycle();
    }

    @Test
    public void testOpenSessionResponses() {
        // OpenSession Success Received.
        when(mMockAudioSession.getSessionId()).thenReturn(SESSION_ID);
        mAudioSessionCallback.onOpenSessionSuccess(mMockAudioSession);
        processAllMessages();
        verify(mMockAudioSessionCallbackHandler).openSessionResponse(
                eq(ImsMediaSession.RESULT_SUCCESS));
        assertEquals(mAudioSessionHandler.getAudioSessionId(),SESSION_ID);

        // AudioSession null object received in OpenSession Success.
        mAudioSessionCallback.onOpenSessionSuccess(null);
        processAllMessages();
        verify(mMockAudioSessionCallbackHandler).openSessionResponse(
                eq(ImsMediaSession.RESULT_NO_MEMORY));

        // OpenSession Failure Received
        mAudioSessionCallback.onOpenSessionFailure(ImsMediaSession.RESULT_NO_RESOURCES);
        processAllMessages();
        verify(mMockAudioSessionCallbackHandler).openSessionResponse(
                eq(ImsMediaSession.RESULT_NO_RESOURCES));
    }

    @Test
    public void testCloseSession() {
        Parcel testParcel = Parcel.obtain();
        testParcel.writeInt(MediaConstants.REQUEST_CLOSE_SESSION);
        testParcel.writeInt(ImsMediaSession.SESSION_TYPE_AUDIO);
        testParcel.setDataPosition(0);
        mAudioSessionHandler.setAudioQosAgent(mMockQosAgent);
        mAudioSessionHandler.setRtpSocket(mRtpSocketPair);
        mAudioSessionHandler.setMediaState(MediaState.MEDIA_STATE_LIVE);
        mMediaListener.onMediaMessage(testParcel);
        processAllMessages();

        mAudioSessionHandler.setMediaState(MediaState.MEDIA_STATE_CLOSED);
        testParcel.setDataPosition(0);
        mMediaListener.onMediaMessage(testParcel);
        processAllMessages();
        verify(mMockImsMediaManager, times(1)).closeSession(eq(mMockAudioSession));
        testParcel.recycle();
    }

    @Test
    public void testModifySession() {
        // Modify Session Request
        AudioConfig audioConfig = MediaTestUtils.createAudioConfig();
        Parcel testParcel = Parcel.obtain();
        testParcel.writeInt(MediaConstants.REQUEST_MODIFY_SESSION);
        testParcel.writeInt(ImsMediaSession.SESSION_TYPE_AUDIO);
        audioConfig.writeToParcel(testParcel, Parcelable.PARCELABLE_WRITE_RETURN_VALUE);
        testParcel.setDataPosition(0);
        mAudioSessionHandler.setMediaState(MediaState.MEDIA_STATE_LIVE);
        mMediaListener.onMediaMessage(testParcel);
        processAllMessages();

        mAudioSessionHandler.setMediaState(MediaState.MEDIA_STATE_CLOSED);
        testParcel.setDataPosition(0);
        mMediaListener.onMediaMessage(testParcel);
        processAllMessages();
        verify(mMockAudioSession, times(1)).modifySession(eq(audioConfig));

        // Modify Session Response - SUCCESS
        mAudioSessionCallback.onModifySessionResponse(audioConfig, RESULT_SUCCESS);
        processAllMessages();
        verify(mMockAudioSessionCallbackHandler).modifySessionResponse(eq(audioConfig),
                eq(RESULT_SUCCESS));

        // Modify Session Response - FAILURE
        mAudioSessionCallback.onModifySessionResponse(audioConfig, RESULT_FAILURE);
        processAllMessages();
        verify(mMockAudioSessionCallbackHandler).modifySessionResponse(eq(audioConfig),
                eq(RESULT_FAILURE));
        testParcel.recycle();
    }

    @Test
    public void testAddConfig() {
        // Add Config Request
        AudioConfig audioConfig = MediaTestUtils.createAudioConfig();
        Parcel testParcel = Parcel.obtain();
        testParcel.writeInt(MediaConstants.REQUEST_ADD_CONFIG);
        testParcel.writeInt(ImsMediaSession.SESSION_TYPE_AUDIO);
        audioConfig.writeToParcel(testParcel, Parcelable.PARCELABLE_WRITE_RETURN_VALUE);
        testParcel.setDataPosition(0);
        mAudioSessionHandler.setMediaState(MediaState.MEDIA_STATE_OPENING);
        mMediaListener.onMediaMessage(testParcel);
        processAllMessages();

        mAudioSessionHandler.setMediaState(MediaState.MEDIA_STATE_IDLE);
        testParcel.setDataPosition(0);
        mMediaListener.onMediaMessage(testParcel);
        processAllMessages();
        verify(mMockAudioSession, times(1)).addConfig(eq(audioConfig));

        // Add Config Response - SUCCESS
        mAudioSessionCallback.onAddConfigResponse(audioConfig,
            RESULT_SUCCESS);
        processAllMessages();
        verify(mMockAudioSessionCallbackHandler).addConfigResponse(eq(audioConfig),
                eq(RESULT_SUCCESS));

        // Add Config Response - FAILURE
        mAudioSessionCallback.onAddConfigResponse(audioConfig,
            RESULT_FAILURE);
        processAllMessages();
        verify(mMockAudioSessionCallbackHandler).addConfigResponse(eq(audioConfig),
                eq(RESULT_FAILURE));
        testParcel.recycle();
    }

    @Test
    public void testRequestQoSForCreateConnection() {
        // QoS Request
        Parcel testParcel = Parcel.obtain();
        testParcel.writeInt(MediaConstants.REQUEST_QOS);
        testParcel.writeInt(ImsMediaSession.SESSION_TYPE_AUDIO);
        testParcel.writeString(REMOTE_RTP_ADDRESS);
        testParcel.writeInt(REMOTE_RTP_PORT);
        testParcel.setDataPosition(0);
        mAudioSessionHandler.setAudioQosAgent(mMockQosAgent);
        mAudioSessionHandler.setLocalAddress(LOCAL_RTP_ADDRESS, LOCAL_RTP_PORT);
        mAudioSessionHandler.setMediaState(MediaState.MEDIA_STATE_LIVE);
        mMediaListener.onMediaMessage(testParcel);
        processAllMessages();
        verify(mMockQosAgent).createQosConnection(eq(LOCAL_RTP_ADDRESS), eq(LOCAL_RTP_PORT),
                eq(REMOTE_RTP_ADDRESS), eq(REMOTE_RTP_PORT));
        testParcel.recycle();
    }

    @Test
    public void testRequestQoSForUpdateConnection() {
        // QoS Request
        Parcel testParcel = Parcel.obtain();
        testParcel.writeInt(MediaConstants.REQUEST_QOS);
        testParcel.writeInt(ImsMediaSession.SESSION_TYPE_AUDIO);
        testParcel.writeString(REMOTE_RTP_ADDRESS);
        testParcel.writeInt(REMOTE_RTP_PORT);
        testParcel.setDataPosition(0);
        mAudioSessionHandler.setAudioQosAgent(mMockQosAgent);
        mAudioSessionHandler.setRtpSocket(mRtpSocketPair);
        mAudioSessionHandler.setQosUpdateRequired(true);
        mAudioSessionHandler.setMediaState(MediaState.MEDIA_STATE_LIVE);
        mMediaListener.onMediaMessage(testParcel);
        processAllMessages();
        verify(mMockQosAgent).updateQosConnection(eq(mMockRtpSocket), eq(mMockRtpSocket),
                eq(REMOTE_RTP_ADDRESS), eq(REMOTE_RTP_PORT));
        testParcel.recycle();
    }

    @Test
    public void testOpenAndQosRequests() {

        Parcel testParcel = Parcel.obtain();
        testParcel.writeInt(MediaConstants.REQUEST_OPEN_SESSION);
        testParcel.writeInt(ImsMediaSession.SESSION_TYPE_AUDIO);
        testParcel.writeString(LOCAL_RTP_ADDRESS);
        testParcel.writeInt(LOCAL_RTP_PORT);
        testParcel.setDataPosition(0);

        Parcel testQosParcel = Parcel.obtain();
        testQosParcel.writeInt(MediaConstants.REQUEST_QOS);
        testQosParcel.writeInt(ImsMediaSession.SESSION_TYPE_AUDIO);
        testQosParcel.writeString(REMOTE_RTP_ADDRESS);
        testQosParcel.writeInt(REMOTE_RTP_PORT);
        testQosParcel.setDataPosition(0);

        // OpenSession Request
        mAudioSessionHandler.setAudioSession(null);
        mMediaManager.setImsMediaConnected(true);
        mAudioSessionHandler.setAudioQosAgent(mMockQosAgent);
        mMediaListener.onMediaMessage(testParcel);

        // QoS Request
        mMediaListener.onMediaMessage(testQosParcel);
        processAllMessages();

        verify(mMockQosAgent).createQosConnection(eq(LOCAL_RTP_ADDRESS), eq(LOCAL_RTP_PORT));
        verify(mMockImsMediaManager, times(1)).openSession(eq(mMockRtpSocket), eq(mMockRtpSocket),
                eq(ImsMediaSession.SESSION_TYPE_AUDIO), eq(null), eq(mMockExecutor),
                eq(mAudioSessionCallback));
        verify(mMockQosAgent).updateQosConnection(eq(mMockRtpSocket), eq(mMockRtpSocket),
                eq(REMOTE_RTP_ADDRESS), eq(REMOTE_RTP_PORT));
        testParcel.recycle();
        testQosParcel.recycle();
    }

    @Test
    public void testDeleteConfig() {
        // Delete Config Request
        AudioConfig audioConfig = MediaTestUtils.createAudioConfig();
        Parcel testParcel = Parcel.obtain();
        testParcel.writeInt(MediaConstants.REQUEST_DELETE_CONFIG);
        testParcel.writeInt(ImsMediaSession.SESSION_TYPE_AUDIO);
        audioConfig.writeToParcel(testParcel, Parcelable.PARCELABLE_WRITE_RETURN_VALUE);
        testParcel.setDataPosition(0);
        mAudioSessionHandler.setMediaState(MediaState.MEDIA_STATE_LIVE);
        mMediaListener.onMediaMessage(testParcel);
        processAllMessages();
        verify(mMockAudioSession).deleteConfig(eq(audioConfig));
        testParcel.recycle();
    }

    @Test
    public void testConfirmConfig() {
        // Confirm Config Request
        AudioConfig audioConfig = MediaTestUtils.createAudioConfig();
        Parcel testParcel = Parcel.obtain();
        testParcel.writeInt(MediaConstants.REQUEST_CONFIRM_CONFIG);
        testParcel.writeInt(ImsMediaSession.SESSION_TYPE_AUDIO);
        audioConfig.writeToParcel(testParcel, Parcelable.PARCELABLE_WRITE_RETURN_VALUE);
        testParcel.setDataPosition(0);
        mAudioSessionHandler.setMediaState(MediaState.MEDIA_STATE_LIVE);
        mMediaListener.onMediaMessage(testParcel);
        processAllMessages();
        verify(mMockAudioSession).confirmConfig(eq(audioConfig));

        // Confirm Config Response - SUCCESS
        mAudioSessionCallback.onConfirmConfigResponse(audioConfig,
            RESULT_SUCCESS);
        processAllMessages();
        verify(mMockAudioSessionCallbackHandler).confirmConfigResponse(eq(audioConfig),
                eq(RESULT_SUCCESS));

        // Confirm Config Response - FAILURE
        mAudioSessionCallback.onConfirmConfigResponse(audioConfig,
            RESULT_FAILURE);
        processAllMessages();
        verify(mMockAudioSessionCallbackHandler).confirmConfigResponse(eq(audioConfig),
                eq(RESULT_FAILURE));
        testParcel.recycle();
    }

    @Test
    public void testDtmf() {
        // Send Dtmf
        Parcel testParcel = Parcel.obtain();
        testParcel.writeInt(MediaConstants.REQUEST_SEND_DTMF);
        testParcel.writeInt(ImsMediaSession.SESSION_TYPE_AUDIO);
        testParcel.writeInt(DTMF_DIGIT);
        testParcel.writeInt(DTMF_DURATION);
        testParcel.setDataPosition(0);
        mAudioSessionHandler.setMediaState(MediaState.MEDIA_STATE_LIVE);
        mMediaListener.onMediaMessage(testParcel);
        processAllMessages();
        verify(mMockAudioSession).sendDtmf(eq(DTMF_DIGIT), eq(DTMF_DURATION));
        testParcel.recycle();
    }

    @Test
    public void testSetMediaQualityThreshold() {
        // Set Media Quality Threshold
        MediaQualityThreshold threshold = MediaTestUtils.createMediaQualityThreshold();
        Parcel testParcel = Parcel.obtain();
        testParcel.writeInt(MediaConstants.REQUEST_SET_MEDIA_QUALITY);
        testParcel.writeInt(ImsMediaSession.SESSION_TYPE_AUDIO);
        threshold.writeToParcel(testParcel, Parcelable.PARCELABLE_WRITE_RETURN_VALUE);
        testParcel.setDataPosition(0);
        mAudioSessionHandler.setMediaState(MediaState.MEDIA_STATE_LIVE);
        mMediaListener.onMediaMessage(testParcel);
        processAllMessages();
        verify(mMockAudioSession).setMediaQualityThreshold(eq(threshold));
        testParcel.recycle();
    }

    @Test
    public void testRtpHeaderExtension() {
        // Send RtpHeaderExtensions
        List<RtpHeaderExtension> rtpExtensions = MediaTestUtils.createRtpExtensions();
        Parcel testParcel = Parcel.obtain();
        testParcel.writeInt(MediaConstants.REQUEST_HEADER_EXTENSION);
        testParcel.writeInt(ImsMediaSession.SESSION_TYPE_AUDIO);
        testParcel.writeInt(rtpExtensions.size());

        if (!rtpExtensions.isEmpty()) {
            for (int i = 0; i < rtpExtensions.size(); ++i) {
                rtpExtensions.get(i).writeToParcel(testParcel,
                    Parcelable.PARCELABLE_WRITE_RETURN_VALUE);
            }
        }
        testParcel.setDataPosition(0);
        mAudioSessionHandler.setMediaState(MediaState.MEDIA_STATE_LIVE);
        mMediaListener.onMediaMessage(testParcel);
        processAllMessages();
        verify(mMockAudioSession).sendHeaderExtension(eq(rtpExtensions));

        // Receive RtpHeaderExtensions
        mAudioSessionCallback.onHeaderExtensionReceived(rtpExtensions);
        processAllMessages();
        verify(mMockAudioSessionCallbackHandler).headerExtensionReceived(eq(rtpExtensions));
        testParcel.recycle();
    }

    @Test
    public void testFirstMediaPacketReceived() {
        // Receive First Packet Notification
        AudioConfig audioConfig = MediaTestUtils.createAudioConfig();
        mAudioSessionCallback.onFirstMediaPacketReceived(audioConfig);
        processAllMessages();
        verify(mMockAudioSessionCallbackHandler).firstMediaPacketReceived(eq(audioConfig));
    }

    @Test
    public void testMediaInactivityNotifications() {
        // Receive RTP Inactivity Notification
        mAudioSessionCallback.notifyMediaInactivity(RTP);
        processAllMessages();
        verify(mMockAudioSessionCallbackHandler).onNotifyMediaInactivity(eq(RTP));

        // Receive RTCP Inactivity Notification
        mAudioSessionCallback.notifyMediaInactivity(RTCP);
        processAllMessages();
        verify(mMockAudioSessionCallbackHandler).onNotifyMediaInactivity(eq(RTCP));
    }

    @Test
    public void testMediaQualityNotifications() {
        // Receive Packet Loss Notification
        mAudioSessionCallback.notifyPacketLoss(MediaTestUtils.PACKET_LOSS_PERCENT);
        processAllMessages();
        verify(mMockAudioSessionCallbackHandler).onNotifyPacketLoss(
            eq(MediaTestUtils.PACKET_LOSS_PERCENT));

        // Receive Packet Loss Notification
        mAudioSessionCallback.notifyJitter(MediaTestUtils.JITTER);
        processAllMessages();
        verify(mMockAudioSessionCallbackHandler).onNotifyJitter(eq(MediaTestUtils.JITTER));

    }

    @Test
    public void testCallQualityChanged() {
        // Receive Call Quality Changed Notification
        CallQuality callQuality = MediaTestUtils.createCallQuality();
        mAudioSessionCallback.onCallQualityChanged(callQuality);
        processAllMessages();
        verify(mMockAudioSessionCallbackHandler).callQualityChanged(eq(callQuality));
    }

    private void processAllMessages() {
        while (!mLooper.getLooper().getQueue().isIdle()) {
            mLooper.processAllMessages();
        }
    }
}
