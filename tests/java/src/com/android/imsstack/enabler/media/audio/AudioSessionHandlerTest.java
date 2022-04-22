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
import android.telephony.CallQuality;
import android.telephony.ims.RtpHeaderExtension;
import android.telephony.imsmedia.AudioConfig;
import android.telephony.imsmedia.AudioSessionCallback;
import android.telephony.imsmedia.ImsAudioSession;
import android.telephony.imsmedia.ImsMediaManager;
import android.telephony.imsmedia.ImsMediaSession;
import android.telephony.imsmedia.MediaQualityThreshold;
import com.android.imsstack.enabler.IBaseContext;
import com.android.imsstack.enabler.media.AudioSessionCallbackHandler;
import com.android.imsstack.enabler.media.AudioSessionHandler;
import com.android.imsstack.enabler.media.IMediaListener;
import com.android.imsstack.enabler.media.MediaConstants;
import com.android.imsstack.enabler.media.MediaManagerHelper;
import com.android.imsstack.enabler.media.MediaSession;
import com.android.imsstack.enabler.media.MediaTestUtils;
import com.android.imsstack.enabler.mtc.MtcMediaSession;
import java.net.DatagramSocket;
import java.util.List;
import java.util.concurrent.Executor;
import org.junit.Before;
import org.junit.Test;
import org.junit.runner.RunWith;
import org.junit.runners.JUnit4;
import org.mockito.Mock;
import org.mockito.MockitoAnnotations;

@RunWith(JUnit4.class)
public class AudioSessionHandlerTest {
    // open Session Parameters
    private static final String LOCAL_RTP_ADDRESS = "111.11.11.11";
    private static final int LOCAL_RTP_PORT = 50010;
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
    @Mock Context context;
    @Mock IBaseContext mMockContext;
    @Mock MtcMediaSession mMockMtcMediaSession;
    @Mock AudioSessionCallbackHandler mMockAudioSessionCallbackHandler;
    @Mock ImsAudioSession mMockAudioSession;
    @Mock ImsMediaManager mMockImsMediaManager;
    @Mock Executor mMockExecutor;

    private AudioSessionHandler mAudioSessionHandler;
    private AudioSessionCallback mAudioSessionCallback;
    private MediaSession mMediaSession;
    private MediaManagerHelper mMediaManager;
    private IMediaListener mMediaListener;

    @Before
    public void setUp() throws Exception {
        //Initialize Mock Objects
        MockitoAnnotations.initMocks(this);
        when(mMockContext.getContext()).thenReturn(context);

        //create the instance to test
        mMediaSession = new MediaSession(mMockContext, mMockMtcMediaSession,
            mMockImsMediaManager, mMockExecutor);
        mMediaManager = mMediaSession.getMediaManager();
        mMediaListener = mMediaSession.getMediaListenerProxy();
        mAudioSessionHandler = new AudioSessionHandler(mMediaManager,
            mMockAudioSessionCallbackHandler, mMockAudioSession);
        mMediaSession.setAudioSessionHandler(mAudioSessionHandler);
        mAudioSessionCallback = mAudioSessionHandler.getAudioSessionCallback();
    }

    @Test
    public void testOpenSession() {

        Parcel testParcel = Parcel.obtain();
        testParcel.writeInt(MediaConstants.REQUEST_OPEN_SESSION);
        testParcel.writeInt(ImsMediaSession.SESSION_TYPE_AUDIO);
        testParcel.writeString(LOCAL_RTP_ADDRESS);
        testParcel.writeInt(LOCAL_RTP_PORT);
        testParcel.setDataPosition(0);
        // OpenSession Received
        mAudioSessionHandler.setAudioSession(null);
        mMediaManager.setImsMediaConnected(true);
        mMediaListener.onMediaMessage(testParcel);
        verify(mMockAudioSessionCallbackHandler).openSessionResponse(
            eq(ImsMediaSession.RESULT_PORT_UNAVAILABLE));
        //TODO: Socket creation success scenario to be verified

        /**
         * AudioSession was opened already, but OpenAudioSession requested again
         * for same session
         */
        mAudioSessionHandler.setAudioSession(mMockAudioSession);
        testParcel.setDataPosition(0);
        mMediaListener.onMediaMessage(testParcel);
        verify(mMockAudioSessionCallbackHandler).openSessionResponse(
            eq(ImsMediaSession.RESULT_NOT_SUPPORTED));

        // ImsMediaManager is not connected
        mAudioSessionHandler.setAudioSession(null);
        mMediaManager.setImsMediaConnected(false);
        testParcel.setDataPosition(0);
        mMediaListener.onMediaMessage(testParcel);
        verify(mMockAudioSessionCallbackHandler).openSessionResponse(
            eq(ImsMediaSession.RESULT_NOT_READY));

        // OpenSession Success Received.
        when(mMockAudioSession.getSessionId()).thenReturn(SESSION_ID);
        mAudioSessionCallback.onOpenSessionSuccess(mMockAudioSession);
        verify(mMockAudioSessionCallbackHandler).openSessionResponse(
            eq(ImsMediaSession.RESULT_SUCCESS));
        assertEquals(mAudioSessionHandler.getAudioSessionId(),SESSION_ID);

        // AudioSession null object received in OpenSession Success.
        mAudioSessionCallback.onOpenSessionSuccess(null);
        verify(mMockAudioSessionCallbackHandler).openSessionResponse(
            eq(ImsMediaSession.RESULT_NO_MEMORY));

        // OpenSession Failure Received
        mAudioSessionCallback.onOpenSessionFailure(ImsMediaSession.RESULT_NO_RESOURCES);
        verify(mMockAudioSessionCallbackHandler).openSessionResponse(
            eq(ImsMediaSession.RESULT_NO_RESOURCES));
    }

    @Test
    public void testCloseSession() {
        Parcel testParcel = Parcel.obtain();
        testParcel.writeInt(MediaConstants.REQUEST_CLOSE_SESSION);
        testParcel.writeInt(ImsMediaSession.SESSION_TYPE_AUDIO);
        testParcel.setDataPosition(0);
        mMediaListener.onMediaMessage(testParcel);
        verify(mMockImsMediaManager).closeSession(eq(mMockAudioSession));
    }

    @Test
    public void testSessionChanged() {
        mAudioSessionCallback.onSessionChanged(ImsMediaSession.SESSION_STATE_ACTIVE);
        verify(mMockAudioSessionCallbackHandler).sessionChanged(
                eq(ImsMediaSession.SESSION_STATE_ACTIVE));
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
        mMediaListener.onMediaMessage(testParcel);
        verify(mMockAudioSession).modifySession(eq(audioConfig));

        // Modify Session Response - SUCCESS
        mAudioSessionCallback.onModifySessionResponse(audioConfig,
            RESULT_SUCCESS);
        verify(mMockAudioSessionCallbackHandler).modifySessionResponse(eq(audioConfig),
            eq(RESULT_SUCCESS));

        // Modify Session Response - FAILURE
        mAudioSessionCallback.onModifySessionResponse(audioConfig,
            RESULT_FAILURE);
        verify(mMockAudioSessionCallbackHandler).modifySessionResponse(eq(audioConfig),
            eq(RESULT_FAILURE));
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
        mMediaListener.onMediaMessage(testParcel);
        verify(mMockAudioSession).addConfig(eq(audioConfig));

        // Add Config Response - SUCCESS
        mAudioSessionCallback.onAddConfigResponse(audioConfig,
            RESULT_SUCCESS);
        verify(mMockAudioSessionCallbackHandler).addConfigResponse(eq(audioConfig),
            eq(RESULT_SUCCESS));

        // Add Config Response - FAILURE
        mAudioSessionCallback.onAddConfigResponse(audioConfig,
            RESULT_FAILURE);
        verify(mMockAudioSessionCallbackHandler).addConfigResponse(eq(audioConfig),
            eq(RESULT_FAILURE));
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
        mMediaListener.onMediaMessage(testParcel);
        verify(mMockAudioSession).deleteConfig(eq(audioConfig));
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
        mMediaListener.onMediaMessage(testParcel);
        verify(mMockAudioSession).confirmConfig(eq(audioConfig));

        // Confirm Config Response - SUCCESS
        mAudioSessionCallback.onConfirmConfigResponse(audioConfig,
            RESULT_SUCCESS);
        verify(mMockAudioSessionCallbackHandler).confirmConfigResponse(eq(audioConfig),
            eq(RESULT_SUCCESS));

        // Confirm Config Response - FAILURE
        mAudioSessionCallback.onConfirmConfigResponse(audioConfig,
            RESULT_FAILURE);
        verify(mMockAudioSessionCallbackHandler).confirmConfigResponse(eq(audioConfig),
            eq(RESULT_FAILURE));
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
        mMediaListener.onMediaMessage(testParcel);
        verify(mMockAudioSession).sendDtmf(eq(DTMF_DIGIT), eq(DTMF_DURATION));
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
        mMediaListener.onMediaMessage(testParcel);
        verify(mMockAudioSession).setMediaQualityThreshold(eq(threshold));
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
        mMediaListener.onMediaMessage(testParcel);
        verify(mMockAudioSession).sendHeaderExtension(eq(rtpExtensions));

        // Receive RtpHeaderExtensions
        mAudioSessionCallback.onHeaderExtensionReceived(rtpExtensions);
        verify(mMockAudioSessionCallbackHandler).headerExtensionReceived(eq(rtpExtensions));
    }

    @Test
    public void testMediaInactivityNotifications() {
        // Receive RTP Inactivity Notification
        mAudioSessionCallback.notifyMediaInactivity(RTP);
        verify(mMockAudioSessionCallbackHandler).onNotifyMediaInactivity(eq(RTP));

        // Receive RTCP Inactivity Notification
        mAudioSessionCallback.notifyMediaInactivity(RTCP);
        verify(mMockAudioSessionCallbackHandler).onNotifyMediaInactivity(eq(RTCP));
    }

    @Test
    public void testMediaQualityNotifications() {
        // Receive Packet Loss Notification
        mAudioSessionCallback.notifyPacketLoss(
            MediaTestUtils.PACKET_LOSS_PERCENT);
        verify(mMockAudioSessionCallbackHandler).onNotifyPacketLoss(
            eq(MediaTestUtils.PACKET_LOSS_PERCENT));

        // Receive Packet Loss Notification
        mAudioSessionCallback.notifyJitter(MediaTestUtils.JITTER);
        verify(mMockAudioSessionCallbackHandler).onNotifyJitter(eq(MediaTestUtils.JITTER));

        // Receive Media Quality Changed Notification
        CallQuality callQuality = MediaTestUtils.createCallQuality();
        mAudioSessionCallback.onMediaQualityChanged(callQuality);
        verify(mMockAudioSessionCallbackHandler).mediaQualityChanged(eq(callQuality));
    }

}
