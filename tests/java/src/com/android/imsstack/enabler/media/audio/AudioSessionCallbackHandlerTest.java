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

import static org.mockito.Mockito.verify;

import android.net.InetAddresses;
import android.os.Parcel;
import android.os.Parcelable;
import android.telephony.CallQuality;
import android.telephony.ims.RtpHeaderExtension;
import android.telephony.imsmedia.AudioConfig;
import android.telephony.imsmedia.ImsMediaSession;
import com.android.imsstack.enabler.media.MediaConstants;
import com.android.imsstack.enabler.media.MediaSession;
import com.android.imsstack.enabler.media.MediaTestUtils;
import java.util.List;
import org.junit.Before;
import org.junit.Test;
import org.junit.runner.RunWith;
import org.junit.runners.JUnit4;
import org.mockito.ArgumentCaptor;
import org.mockito.Captor;
import org.mockito.Mock;
import org.mockito.MockitoAnnotations;

@RunWith(JUnit4.class)
public class AudioSessionCallbackHandlerTest {

    @Mock MediaSession mMockMediaSession;
    @Captor ArgumentCaptor<Parcel> mCaptorParcel;

    private AudioSessionCallbackHandler mAudioSessionCallbackHandler;

    @Before
    public void setUp() throws Exception {
        MockitoAnnotations.initMocks(this);
        // create the instance to test
        mAudioSessionCallbackHandler = new AudioSessionCallbackHandler(mMockMediaSession);
    }

    @Test
    public void testOpenSessionSuccess() {

        Parcel testParcel = Parcel.obtain();
        testParcel.writeInt(MediaConstants.RESPONSE_OPEN_SESSION);
        testParcel.writeInt(ImsMediaSession.SESSION_TYPE_AUDIO);
        testParcel.writeInt(ImsMediaSession.RESULT_SUCCESS);

        mAudioSessionCallbackHandler.openSessionResponse(ImsMediaSession.RESULT_SUCCESS);

        verify(mMockMediaSession).sendRequest(mCaptorParcel.capture());
        MediaTestUtils.assertParcelEquals(testParcel, mCaptorParcel.getValue());
    }

    @Test
    public void testOpenSessionFailure() {

        Parcel testParcel = Parcel.obtain();
        testParcel.writeInt(MediaConstants.RESPONSE_OPEN_SESSION);
        testParcel.writeInt(ImsMediaSession.SESSION_TYPE_AUDIO);
        testParcel.writeInt(ImsMediaSession.RESULT_NO_RESOURCES);

        mAudioSessionCallbackHandler.openSessionResponse(ImsMediaSession.RESULT_NO_RESOURCES);

        verify(mMockMediaSession).sendRequest(mCaptorParcel.capture());
        MediaTestUtils.assertParcelEquals(testParcel, mCaptorParcel.getValue());
    }

    @Test
    public void testSessionChanged() {

        Parcel testParcel = Parcel.obtain();

        testParcel.writeInt(MediaConstants.RESPONSE_SESSION_CHANGED);
        testParcel.writeInt(ImsMediaSession.SESSION_TYPE_AUDIO);
        testParcel.writeInt(ImsMediaSession.SESSION_STATE_OPEN);

        mAudioSessionCallbackHandler.sessionChanged(ImsMediaSession.SESSION_STATE_OPEN);

        verify(mMockMediaSession).sendRequest(mCaptorParcel.capture());
        MediaTestUtils.assertParcelEquals(testParcel, mCaptorParcel.getValue());
    }

    @Test
    public void testModifySessionResponse() {

        AudioConfig audioConfig = MediaTestUtils.createAudioConfig();
        Parcel testParcel = Parcel.obtain();

        testParcel.writeInt(MediaConstants.RESPONSE_MODIFY_SESSION);
        testParcel.writeInt(ImsMediaSession.SESSION_TYPE_AUDIO);
        audioConfig.writeToParcel(testParcel, Parcelable.PARCELABLE_WRITE_RETURN_VALUE);
        testParcel.writeInt(ImsMediaSession.RESULT_SUCCESS);

        mAudioSessionCallbackHandler.modifySessionResponse(audioConfig,
            ImsMediaSession.RESULT_SUCCESS);

        verify(mMockMediaSession).sendRequest(mCaptorParcel.capture());
        MediaTestUtils.assertParcelEquals(testParcel, mCaptorParcel.getValue());
    }

    @Test
    public void testAddConfigResponse() {

        AudioConfig audioConfig = MediaTestUtils.createAudioConfig();
        Parcel testParcel = Parcel.obtain();

        testParcel.writeInt(MediaConstants.RESPONSE_ADD_CONFIG);
        testParcel.writeInt(ImsMediaSession.SESSION_TYPE_AUDIO);
        audioConfig.writeToParcel(testParcel, Parcelable.PARCELABLE_WRITE_RETURN_VALUE);
        testParcel.writeInt(ImsMediaSession.RESULT_SUCCESS);

        mAudioSessionCallbackHandler.addConfigResponse(audioConfig,
            ImsMediaSession.RESULT_SUCCESS);

        verify(mMockMediaSession).sendRequest(mCaptorParcel.capture());
        MediaTestUtils.assertParcelEquals(testParcel, mCaptorParcel.getValue());
    }

    @Test
    public void testConfirmConfigResponse() {

        AudioConfig audioConfig = MediaTestUtils.createAudioConfig();
        Parcel testParcel = Parcel.obtain();

        testParcel.writeInt(MediaConstants.RESPONSE_CONFIRM_CONFIG);
        testParcel.writeInt(ImsMediaSession.SESSION_TYPE_AUDIO);
        audioConfig.writeToParcel(testParcel, Parcelable.PARCELABLE_WRITE_RETURN_VALUE);
        testParcel.writeInt(ImsMediaSession.RESULT_SUCCESS);

        mAudioSessionCallbackHandler.confirmConfigResponse(audioConfig,
            ImsMediaSession.RESULT_SUCCESS);

        verify(mMockMediaSession).sendRequest(mCaptorParcel.capture());
        MediaTestUtils.assertParcelEquals(testParcel, mCaptorParcel.getValue());
    }

    @Test
    public void testFirstMediaPacketReceived() {

        AudioConfig audioConfig = MediaTestUtils.createAudioConfig();
        Parcel testParcel = Parcel.obtain();

        testParcel.writeInt(MediaConstants.NOTIFY_FIRST_PACKET);
        testParcel.writeInt(ImsMediaSession.SESSION_TYPE_AUDIO);
        audioConfig.writeToParcel(testParcel, Parcelable.PARCELABLE_WRITE_RETURN_VALUE);

        mAudioSessionCallbackHandler.firstMediaPacketReceived(audioConfig);

        verify(mMockMediaSession).sendRequest(mCaptorParcel.capture());
        MediaTestUtils.assertParcelEquals(testParcel, mCaptorParcel.getValue());
    }

    @Test
    public void testHeaderExtensionReceived() {

        List<RtpHeaderExtension> rtpExtensions = MediaTestUtils.createRtpExtensions();
        Parcel testParcel = Parcel.obtain();

        testParcel.writeInt(MediaConstants.NOTIFY_HEADER_EXTENSION);
        testParcel.writeInt(ImsMediaSession.SESSION_TYPE_AUDIO);
        testParcel.writeInt(rtpExtensions.size());

        if (!rtpExtensions.isEmpty()) {
            for (int i = 0; i < rtpExtensions.size(); ++i) {
                rtpExtensions.get(i).writeToParcel(testParcel,
                    Parcelable.PARCELABLE_WRITE_RETURN_VALUE);
            }
        }

        mAudioSessionCallbackHandler.headerExtensionReceived(rtpExtensions);

        verify(mMockMediaSession).sendRequest(mCaptorParcel.capture());
        MediaTestUtils.assertParcelEquals(testParcel, mCaptorParcel.getValue());
    }

    @Test
    public void testNotifyMediaInactivity() {

        Parcel testParcel = Parcel.obtain();

        testParcel.writeInt(MediaConstants.NOTIFY_MEDIA_INACTIVITY);
        testParcel.writeInt(ImsMediaSession.SESSION_TYPE_AUDIO);
        testParcel.writeInt(ImsMediaSession.PACKET_TYPE_RTP);

        mAudioSessionCallbackHandler.onNotifyMediaInactivity(ImsMediaSession.PACKET_TYPE_RTP);

        verify(mMockMediaSession).sendRequest(mCaptorParcel.capture());
        MediaTestUtils.assertParcelEquals(testParcel, mCaptorParcel.getValue());
    }

    @Test
    public void testNotifyPacketLoss() {

        Parcel testParcel = Parcel.obtain();

        testParcel.writeInt(MediaConstants.NOTIFY_PACKET_LOSS);
        testParcel.writeInt(ImsMediaSession.SESSION_TYPE_AUDIO);
        testParcel.writeInt(MediaTestUtils.PACKET_LOSS_PERCENT);

        mAudioSessionCallbackHandler.onNotifyPacketLoss(MediaTestUtils.PACKET_LOSS_PERCENT);

        verify(mMockMediaSession).sendRequest(mCaptorParcel.capture());
        MediaTestUtils.assertParcelEquals(testParcel, mCaptorParcel.getValue());
    }

    @Test
    public void testNotifyJitter() {

        Parcel testParcel = Parcel.obtain();

        testParcel.writeInt(MediaConstants.NOTIFY_JITTER);
        testParcel.writeInt(ImsMediaSession.SESSION_TYPE_AUDIO);
        testParcel.writeInt(MediaTestUtils.JITTER);

        mAudioSessionCallbackHandler.onNotifyJitter(MediaTestUtils.JITTER);

        verify(mMockMediaSession).sendRequest(mCaptorParcel.capture());
        MediaTestUtils.assertParcelEquals(testParcel, mCaptorParcel.getValue());
    }

    @Test
    public void mediaQualityChanged() {

        CallQuality callQuality = MediaTestUtils.createCallQuality();
        Parcel testParcel = Parcel.obtain();

        testParcel.writeInt(MediaConstants.NOTIFY_MEDIA_QUALITY_CHANGE);
        testParcel.writeInt(ImsMediaSession.SESSION_TYPE_AUDIO);
        callQuality.writeToParcel(testParcel, Parcelable.PARCELABLE_WRITE_RETURN_VALUE);
        mAudioSessionCallbackHandler.mediaQualityChanged(callQuality);

        verify(mMockMediaSession).sendRequest(mCaptorParcel.capture());
        MediaTestUtils.assertParcelEquals(testParcel, mCaptorParcel.getValue());
    }
}
