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
import android.telephony.AccessNetworkConstants.AccessNetworkType;
import android.telephony.CallQuality;
import android.telephony.ims.RtpHeaderExtension;
import android.telephony.imsmedia.AudioConfig;
import android.telephony.imsmedia.AudioSessionCallback;
import android.telephony.imsmedia.ImsAudioSession;
import android.telephony.imsmedia.ImsMediaSession;
import android.telephony.imsmedia.MediaQualityStatus;
import android.telephony.imsmedia.MediaQualityThreshold;
import android.testing.AndroidTestingRunner;
import android.testing.TestableLooper;

import org.junit.After;
import org.junit.Before;
import org.junit.Test;
import org.junit.runner.RunWith;
import org.mockito.Mock;

import java.util.List;

@RunWith(AndroidTestingRunner.class)
@TestableLooper.RunWithLooper
public class AudioSessionHandlerTest extends MediaSessionHandlerTest {
    // open Session Parameters
    private static final String LOCAL_RTP_ADDRESS = "111.11.11.11";
    private static final int LOCAL_RTP_PORT = 50010;
    private static final String REMOTE_RTP_ADDRESS = "fg01:abab:cdef:6fee::1";
    private static final int REMOTE_RTP_PORT = 1236;
    private static final int SESSION_ID = 439;

    // Dtmf settings
    private static final int DTMF_DURATION = 200;
    private static final char DTMF_DIGIT = '9';

    // Mock Objects
    @Mock AudioSessionCallbackHandler mMockAudioSessionCallbackHandler;
    @Mock ImsAudioSession mMockAudioSession;

    private AudioSessionHandler mAudioSessionHandler;
    private AudioSessionCallback mAudioSessionCallback;

    @Before
    public void setUp() throws Exception {
        super.setUp(getClass().getSimpleName());

        // create the instance to test
        mAudioSessionHandler = new AudioSessionHandler(mMockBaseContext, mMediaManager,
                mMockAudioSessionCallbackHandler, mMockAudioSession, mMockMediaConfig,
                Looper.myLooper());
        mMediaSession.setAudioSessionHandler(mAudioSessionHandler);
        mAudioSessionCallback = mAudioSessionHandler.getAudioSessionCallback();
    }

    @After
    public void tearDown() throws Exception {
        mAudioSessionHandler = null;
        super.tearDown();
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
    public void testSetMediaQualityThresholdwithFwkTimer() {
        // Set Media Quality Threshold
        MediaQualityThreshold threshold = MediaTestUtils.createMediaQualityThreshold();
        Parcel testParcel = Parcel.obtain();
        testParcel.writeInt(MediaConstants.REQUEST_SET_MEDIA_QUALITY);
        testParcel.writeInt(ImsMediaSession.SESSION_TYPE_AUDIO);
        testParcel.writeBoolean(true);
        threshold.writeToParcel(testParcel, Parcelable.PARCELABLE_WRITE_RETURN_VALUE);
        testParcel.setDataPosition(0);
        mAudioSessionHandler.setMediaState(MediaState.MEDIA_STATE_LIVE);
        mMediaListener.onMediaMessage(testParcel);
        processAllMessages();
        verify(mMockAudioSession).setMediaQualityThreshold(eq(threshold));
        testParcel.recycle();
    }

    @Test
    public void testSetMediaQualityThresholdwithoutFwkTimer() {
        // Set Media Quality Threshold
        MediaQualityThreshold threshold = MediaTestUtils.createMediaQualityThreshold();
        Parcel testParcel = Parcel.obtain();
        testParcel.writeInt(MediaConstants.REQUEST_SET_MEDIA_QUALITY);
        testParcel.writeInt(ImsMediaSession.SESSION_TYPE_AUDIO);
        testParcel.writeInt(MediaConstants.REQUEST_SET_MEDIA_QUALITY);
        testParcel.writeBoolean(false);
        threshold.writeToParcel(testParcel, Parcelable.PARCELABLE_WRITE_RETURN_VALUE);
        testParcel.setDataPosition(0);
        mAudioSessionHandler.setMediaState(MediaState.MEDIA_STATE_LIVE);
        mMediaListener.onMediaMessage(testParcel);
        processAllMessages();
        verify(mMockAudioSession).setMediaQualityThreshold(eq(threshold));
        testParcel.recycle();
    }

    @Test
    public void testSetMediaQualityThresholdTwice() {
        // Set Media Quality Threshold
        MediaQualityThreshold threshold = MediaTestUtils.createMediaQualityThreshold();
        Parcel testParcel = Parcel.obtain();
        testParcel.writeInt(MediaConstants.REQUEST_SET_MEDIA_QUALITY);
        testParcel.writeInt(ImsMediaSession.SESSION_TYPE_AUDIO);
        testParcel.writeInt(MediaConstants.REQUEST_SET_MEDIA_QUALITY);
        testParcel.writeBoolean(false);
        threshold.writeToParcel(testParcel, Parcelable.PARCELABLE_WRITE_RETURN_VALUE);
        testParcel.setDataPosition(0);
        Parcel testParcel2 = Parcel.obtain();
        testParcel2.writeInt(MediaConstants.REQUEST_SET_MEDIA_QUALITY);
        testParcel2.writeInt(ImsMediaSession.SESSION_TYPE_AUDIO);
        testParcel2.writeInt(MediaConstants.REQUEST_SET_MEDIA_QUALITY);
        testParcel2.writeBoolean(true);
        threshold.writeToParcel(testParcel2, Parcelable.PARCELABLE_WRITE_RETURN_VALUE);
        testParcel2.setDataPosition(0);
        mAudioSessionHandler.setMediaState(MediaState.MEDIA_STATE_LIVE);
        mMediaListener.onMediaMessage(testParcel);
        processAllMessages();

        mMediaListener.onMediaMessage(testParcel2);
        processAllMessages();
        verify(mMockAudioSession, times(2)).setMediaQualityThreshold(eq(threshold));
        testParcel.recycle();
        testParcel2.recycle();
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
    public void testNotifyAnbrEnabed() {
        // Send Anbr Enabled
        Parcel testParcel = Parcel.obtain();
        testParcel.writeInt(MediaConstants.REQUEST_UPDATE_ANBR_ENABLED_CONFIG);
        testParcel.writeInt(ImsMediaSession.SESSION_TYPE_AUDIO);
        testParcel.writeBoolean(true);

        testParcel.setDataPosition(0);
        mAudioSessionHandler.setMediaState(MediaState.MEDIA_STATE_LIVE);
        mMediaListener.onMediaMessage(testParcel);
        processAllMessages();
        assertEquals(mAudioSessionHandler.getAudioAnbrEnabled(), true);
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
    public void testMediaQualityStatusNotification() {
        // Receive Media Quality Status Notification
        MediaQualityStatus qualityStatus = MediaTestUtils.createMediaQualityStatus();
        mAudioSessionCallback.notifyMediaQualityStatus(qualityStatus);
        processAllMessages();
        verify(mMockAudioSessionCallbackHandler).onNotifyMediaQualityStatus(
                eq(AccessNetworkType.EUTRAN), eq(qualityStatus));
    }

    @Test
    public void testCallQualityChanged() {
        // Receive Call Quality Changed Notification
        CallQuality callQuality = MediaTestUtils.createCallQuality();
        mAudioSessionCallback.onCallQualityChanged(callQuality);
        processAllMessages();
        verify(mMockAudioSessionCallbackHandler).callQualityChanged(eq(callQuality));
    }

    @Test
    public void testMediaDetach() {
        Parcel testParcel = Parcel.obtain();
        testParcel.writeInt(MediaConstants.NOTIFY_MEDIA_DETACH);
        testParcel.writeInt(ImsMediaSession.SESSION_TYPE_AUDIO);
        testParcel.setDataPosition(0);
        mAudioSessionHandler.setAudioQosAgent(mMockQosAgent);
        mAudioSessionHandler.setRtpSocket(mRtpSocketPair);
        mAudioSessionHandler.setMediaState(MediaState.MEDIA_STATE_LIVE);
        mMediaListener.onMediaMessage(testParcel);
        processAllMessages();

        verify(mMockAudioSessionCallbackHandler, times(1)).nofityMediaDetach();
        verify(mMockAudioSessionCallbackHandler, times(1)).closeSessionResponse();
        verify(mMockQosAgent,
                times(1)).destroyQosConnection(eq(mMockRtpSocket), eq(mMockRtpSocket));
        assertEquals(mAudioSessionHandler.getMediaState(), MediaState.MEDIA_STATE_IDLE);
        testParcel.recycle();
    }

    @Test
    public void testOnNotifyIncomingDtmfReceived() {
        mAudioSessionCallback.onDtmfReceived(DTMF_DIGIT, DTMF_DURATION);
        processAllMessages();
        verify(mMockAudioSessionCallbackHandler).onNotifyIncomingDtmfReceived(
                eq((int) DTMF_DIGIT), eq(DTMF_DURATION));
    }
}
