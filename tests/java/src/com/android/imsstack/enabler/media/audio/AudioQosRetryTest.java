/*
 * Copyright (C) 2026 The Android Open Source Project
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

import static org.mockito.ArgumentMatchers.any;
import static org.mockito.ArgumentMatchers.anyBoolean;
import static org.mockito.ArgumentMatchers.anyInt;
import static org.mockito.ArgumentMatchers.anyString;
import static org.mockito.ArgumentMatchers.eq;
import static org.mockito.Mockito.mock;
import static org.mockito.Mockito.never;
import static org.mockito.Mockito.times;
import static org.mockito.Mockito.verify;
import static org.mockito.Mockito.when;

import android.os.Looper;
import android.os.Parcel;
import android.telephony.imsmedia.ImsAudioSession;
import android.telephony.imsmedia.ImsMediaSession;
import android.testing.AndroidTestingRunner;
import android.testing.TestableLooper;
import android.util.Pair;

import com.android.imsstack.core.agents.QosAgent.ImsQosCallback;

import org.junit.After;
import org.junit.Before;
import org.junit.Test;
import org.junit.runner.RunWith;
import org.mockito.Mock;

import java.net.DatagramSocket;
import java.net.InetSocketAddress;

@RunWith(AndroidTestingRunner.class)
@TestableLooper.RunWithLooper
public class AudioQosRetryTest extends MediaSessionHandlerTest {
    private static final String REMOTE = "127.0.0.2";
    private static final int PORT = 1240;
    @Mock private AudioSessionCallbackHandler mCallbackHandler;
    @Mock private ImsAudioSession mAudioSession;
    @Mock private DtmfToneGenerator mDtmf;
    private AudioSessionHandler mHandler;
    private ImsQosCallback mCallback;

    @Before
    public void setUp() throws Exception {
        super.setUp(getClass().getSimpleName());
        mHandler = new AudioSessionHandler(mMockBaseContext, mMediaManager,
                mCallbackHandler, mAudioSession, mMockMediaConfig, Looper.myLooper(),
                mDtmf, mMockQosAgent);
        mMediaSession.setAudioSessionHandler(mHandler);
        mHandler.setRtpSocket(mRtpSocketPair);
        mHandler.setLocalAddress("127.0.0.1", 50010);
        mHandler.setQosUpdateRequired(true);
        mHandler.setMediaState(MediaState.MEDIA_STATE_OPENING);
        mCallback = mHandler.getAudioImsQosCallback();
    }

    @After
    public void tearDown() throws Exception {
        sendClose();
        super.tearDown();
    }

    private void sendQos(String address, int port) {
        Parcel parcel = Parcel.obtain();
        parcel.writeInt(MediaConstants.REQUEST_QOS);
        parcel.writeInt(ImsMediaSession.SESSION_TYPE_AUDIO);
        parcel.writeString(address);
        parcel.writeInt(port);
        parcel.setDataPosition(0);
        mMediaListener.onMediaMessage(parcel);
        processAllMessages();
        parcel.recycle();
    }

    private void sendClose() {
        Parcel parcel = Parcel.obtain();
        parcel.writeInt(MediaConstants.REQUEST_CLOSE_SESSION);
        parcel.writeInt(ImsMediaSession.SESSION_TYPE_AUDIO);
        parcel.setDataPosition(0);
        mMediaListener.onMediaMessage(parcel);
        processAllMessages();
        parcel.recycle();
    }

    private void advanceRetry() {
        mTestableLooper.moveTimeForward(AudioSessionHandler.QOS_CALLBACK_RETRY_DELAY_MILLIS);
        processAllMessages();
    }

    private void verifyAttempts(int count) {
        verify(mMockQosAgent, times(count)).updateQosConnection(
                any(), any(), anyString(), anyInt(), anyBoolean());
    }

    @Test
    public void failedRegistrationRetriesAndStopsAfterSuccess() {
        when(mMockQosAgent.updateQosConnection(any(), any(), anyString(), anyInt(), anyBoolean()))
                .thenReturn(false, true);
        sendQos(REMOTE, PORT);
        verifyAttempts(1);
        advanceRetry();
        verifyAttempts(2);
        advanceRetry();
        verifyAttempts(2);
        verify(mCallbackHandler, never()).onNotifyQosInfo(anyString(), anyInt(), eq(true));
    }

    @Test
    public void registrationSuccessDoesNotRetryWhileWaitingForBearer() {
        sendQos(REMOTE, PORT);
        for (int i = 0; i <= AudioSessionHandler.QOS_CALLBACK_MAX_RETRIES; ++i) {
            advanceRetry();
        }
        verifyAttempts(1);
        verify(mCallbackHandler, never()).onNotifyQosInfo(anyString(), anyInt(), eq(true));
    }

    @Test
    public void automaticRetriesAreBounded() {
        when(mMockQosAgent.updateQosConnection(any(), any(), anyString(), anyInt(), anyBoolean()))
                .thenReturn(false);
        sendQos(REMOTE, PORT);
        for (int i = 0; i <= AudioSessionHandler.QOS_CALLBACK_MAX_RETRIES; ++i) {
            advanceRetry();
        }
        verifyAttempts(1 + AudioSessionHandler.QOS_CALLBACK_MAX_RETRIES);
    }

    @Test
    public void asynchronousUnregistrationRetriesWithoutReportingBearerLoss() {
        sendQos(REMOTE, PORT);
        mCallback.onNotifyQosCallbackError(mMockRtpSocket, new InetSocketAddress(REMOTE, PORT));
        processAllMessages();
        advanceRetry();
        verifyAttempts(2);
        verify(mCallbackHandler, never()).onNotifyQosInfo(anyString(), anyInt(), eq(false));
    }

    @Test
    public void oldSocketAndOldPeerErrorsDoNotRetryCurrentRegistration() {
        sendQos(REMOTE, PORT);
        mCallback.onNotifyQosCallbackError(mock(DatagramSocket.class),
                new InetSocketAddress(REMOTE, PORT));
        mCallback.onNotifyQosCallbackError(mMockRtpSocket,
                new InetSocketAddress("127.0.0.3", PORT));
        processAllMessages();
        advanceRetry();
        verifyAttempts(1);
    }

    @Test
    public void closeCancelsPendingRegistrationRetry() {
        when(mMockQosAgent.updateQosConnection(any(), any(), anyString(), anyInt(), anyBoolean()))
                .thenReturn(false);
        sendQos(REMOTE, PORT);
        sendClose();
        advanceRetry();
        verifyAttempts(1);
    }

    @Test
    public void releasedMediaCancelsPendingRegistrationRetry() {
        when(mMockQosAgent.updateQosConnection(any(), any(), anyString(), anyInt(), anyBoolean()))
                .thenReturn(false);
        sendQos(REMOTE, PORT);
        sendQos(null, 0);
        advanceRetry();
        verifyAttempts(1);
    }

    @Test
    public void newRemoteSupersedesPendingRetry() {
        when(mMockQosAgent.updateQosConnection(any(), any(), anyString(), anyInt(), anyBoolean()))
                .thenReturn(false, true);
        sendQos(REMOTE, PORT);
        sendQos("127.0.0.3", PORT + 2);
        advanceRetry();
        verifyAttempts(2);
        verify(mMockQosAgent).updateQosConnection(any(), any(), eq("127.0.0.3"),
                eq(PORT + 2), anyBoolean());
    }

    @Test
    public void replacedSocketsDoNotReceiveOldRemoteFromPendingRetry() {
        when(mMockQosAgent.updateQosConnection(any(), any(), anyString(), anyInt(), anyBoolean()))
                .thenReturn(false);
        sendQos(REMOTE, PORT);
        DatagramSocket replacement = mock(DatagramSocket.class);
        mHandler.setRtpSocket(new Pair<>(replacement, replacement));
        advanceRetry();
        verifyAttempts(1);
    }
}
