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

package com.android.imsstack.enabler.media;

import static org.mockito.ArgumentMatchers.any;
import static org.mockito.ArgumentMatchers.anyBoolean;
import static org.mockito.ArgumentMatchers.anyInt;
import static org.mockito.ArgumentMatchers.anyString;
import static org.mockito.Mockito.doNothing;
import static org.mockito.Mockito.when;

import android.content.Context;
import android.telephony.ims.MediaThreshold;
import android.telephony.imsmedia.AudioConfig;
import android.telephony.imsmedia.ImsMediaManager;
import android.telephony.imsmedia.ImsMediaSession;
import android.telephony.imsmedia.MediaQualityThreshold;
import android.util.Pair;

import com.android.imsstack.ImsStackTest;
import com.android.imsstack.core.agents.QosAgent;
import com.android.imsstack.enabler.IBaseContext;
import com.android.imsstack.enabler.mtc.MtcMediaSession;

import org.mockito.Mock;
import org.mockito.MockitoAnnotations;

import java.net.DatagramSocket;
import java.util.concurrent.Executor;

public abstract class MediaSessionHandlerTest extends ImsStackTest {

    // Session Operator Results
    protected static final int RESULT_SUCCESS = ImsMediaSession.RESULT_SUCCESS;
    protected static final int RESULT_FAILURE = ImsMediaSession.RESULT_NO_RESOURCES;

    // Packet Types
    protected static final int RTP = ImsMediaSession.PACKET_TYPE_RTP;
    protected static final int RTCP = ImsMediaSession.PACKET_TYPE_RTCP;

    // Mocked classes
    @Mock protected Context mMockContext;
    @Mock protected IBaseContext mMockBaseContext;
    @Mock protected MtcMediaSession mMockMtcMediaSession;
    @Mock protected ImsMediaManager mMockImsMediaManager;
    @Mock protected Executor mMockExecutor;
    @Mock protected QosAgent mMockQosAgent;
    @Mock protected DatagramSocket mMockRtpSocket;
    @Mock protected MediaConfig mMockMediaConfig;
    // Initialized classes
    protected MediaSession mMediaSession;
    protected MediaManagerHelper mMediaManager;
    protected IMediaListener mMediaListener;
    protected Pair<DatagramSocket, DatagramSocket> mRtpSocketPair;

    protected void setUp(String tag) throws Exception {
        super.setUp(tag);

        //Initialize Mock Objects
        MockitoAnnotations.initMocks(this);
        stubContext();
        stubQosAgent();
        stubMediaConfig();

        // create the instance to test
        mMediaSession = new MediaSession(mMockBaseContext, mMockMtcMediaSession,
                mMockImsMediaManager, mMockExecutor);
        mMediaManager = mMediaSession.getMediaManager();
        mMediaListener = mMediaSession.getMediaListenerProxy();
    }

    protected void tearDown() throws Exception {
        mRtpSocketPair = null;
        mMediaSession = null;
        mMediaManager.close();
        super.tearDown();
    }

    private void stubContext() {
        when(mMockBaseContext.getContext()).thenReturn(mMockContext);
    }

    private void stubQosAgent() {
        mRtpSocketPair = new Pair<>(mMockRtpSocket, mMockRtpSocket);
        when(mMockQosAgent.createQosConnection(anyString(), anyInt()))
                .thenReturn(mRtpSocketPair);
        doNothing().when(mMockQosAgent).destroyQosConnection(any(), any());
        when(mMockQosAgent.updateQosConnection(any(), any(), anyString(), anyInt()))
                .thenReturn(true);
    }

    private void stubMediaConfig() {
        doNothing().when(mMockMediaConfig).updateRtpConfig(any(AudioConfig.class));
        when(mMockMediaConfig.getRtpConfig()).thenReturn(MediaTestUtils.createAudioConfig());
        doNothing().when(mMockMediaConfig).updateMediaQualityThreshold(
                any(MediaQualityThreshold.class), anyBoolean());
        doNothing().when(mMockMediaConfig).updateMediaQualityThreshold(any(MediaThreshold.class));
        when(mMockMediaConfig.getMediaQualityThreshold())
                .thenReturn(MediaTestUtils.createMediaQualityThreshold());
    }
}
