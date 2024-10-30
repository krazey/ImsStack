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

import static org.mockito.ArgumentMatchers.eq;
import static org.mockito.Mockito.verify;

import android.os.Parcel;
import android.os.Parcelable;
import android.telephony.AccessNetworkConstants.AccessNetworkType;
import android.telephony.CallQuality;
import android.telephony.ims.RtpHeaderExtension;
import android.telephony.imsmedia.AudioConfig;
import android.telephony.imsmedia.ImsMediaSession;
import android.telephony.imsmedia.MediaQualityStatus;

import com.android.imsstack.enabler.mtc.MtcMediaSession;

import org.junit.Before;
import org.junit.Test;
import org.junit.runner.RunWith;
import org.junit.runners.JUnit4;
import org.mockito.ArgumentCaptor;
import org.mockito.Captor;
import org.mockito.Mock;
import org.mockito.MockitoAnnotations;

import java.util.List;

@RunWith(JUnit4.class)
public class AudioSessionCallbackHandlerTest {
    private static final int MEDIA_STREAM_TYPE_AUDIO = 1;
    private static final int MEDIA_STREAM_DIRECTION_UPLINK = 1;
    private static final int MEDIA_STREAM_TEST_BITRATE = 10;

    @Mock MtcMediaSession mMockMtcMediaSession;
    @Captor ArgumentCaptor<Parcel> mCaptorParcel;

    private AudioSessionCallbackHandler mAudioSessionCallbackHandler;

    @Before
    public void setUp() throws Exception {
        MockitoAnnotations.initMocks(this);
        // create the instance to test
        mAudioSessionCallbackHandler = new AudioSessionCallbackHandler(mMockMtcMediaSession);
    }

    @Test
    public void testOpenSessionSuccess() {

        Parcel testParcel = Parcel.obtain();
        testParcel.writeInt(MediaConstants.RESPONSE_OPEN_SESSION);
        testParcel.writeInt(ImsMediaSession.SESSION_TYPE_AUDIO);
        testParcel.writeInt(ImsMediaSession.RESULT_SUCCESS);

        mAudioSessionCallbackHandler.openSessionResponse(ImsMediaSession.RESULT_SUCCESS);

        verify(mMockMtcMediaSession).sendRequest(mCaptorParcel.capture());
        MediaTestUtils.assertParcelEquals(testParcel, mCaptorParcel.getValue());
    }

    @Test
    public void testOpenSessionFailure() {

        Parcel testParcel = Parcel.obtain();
        testParcel.writeInt(MediaConstants.RESPONSE_OPEN_SESSION);
        testParcel.writeInt(ImsMediaSession.SESSION_TYPE_AUDIO);
        testParcel.writeInt(ImsMediaSession.RESULT_NO_RESOURCES);

        mAudioSessionCallbackHandler.openSessionResponse(ImsMediaSession.RESULT_NO_RESOURCES);

        verify(mMockMtcMediaSession).sendRequest(mCaptorParcel.capture());
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

        verify(mMockMtcMediaSession).sendRequest(mCaptorParcel.capture());
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

        verify(mMockMtcMediaSession).sendRequest(mCaptorParcel.capture());
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

        verify(mMockMtcMediaSession).sendRequest(mCaptorParcel.capture());
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

        verify(mMockMtcMediaSession).sendRequest(mCaptorParcel.capture());
        MediaTestUtils.assertParcelEquals(testParcel, mCaptorParcel.getValue());
    }

    @Test
    public void testHandleTriggerAnbrQuery() {

        mAudioSessionCallbackHandler.triggerAnbrQuery(MEDIA_STREAM_TYPE_AUDIO,
                MEDIA_STREAM_DIRECTION_UPLINK, MEDIA_STREAM_TEST_BITRATE);

        verify(mMockMtcMediaSession).triggerAnbrQuery(
                eq(MEDIA_STREAM_TYPE_AUDIO), eq(MEDIA_STREAM_DIRECTION_UPLINK),
                eq(MEDIA_STREAM_TEST_BITRATE));
    }

    @Test
    public void testHeaderExtensionReceived() {

        List<RtpHeaderExtension> rtpExtensions = MediaTestUtils.createRtpExtensions();
        mAudioSessionCallbackHandler.headerExtensionReceived(rtpExtensions);

        verify(mMockMtcMediaSession).rtpHeaderExtensionsReceived(
                eq(MediaTestUtils.createRtpExtensionsSet()));
    }

    @Test
    public void testNotifyMediaQualityStatus() {

        MediaQualityStatus qualityStatus = MediaTestUtils.createMediaQualityStatus();
        Parcel testParcel = Parcel.obtain();

        testParcel.writeInt(MediaConstants.NOTIFY_MEDIA_INACTIVITY);
        testParcel.writeInt(ImsMediaSession.SESSION_TYPE_AUDIO);
        testParcel.writeInt(qualityStatus.getRtpInactivityTimeMillis());
        testParcel.writeInt(qualityStatus.getRtcpInactivityTimeMillis());

        mAudioSessionCallbackHandler.onNotifyMediaQualityStatus(AccessNetworkType.EUTRAN,
                qualityStatus);

        verify(mMockMtcMediaSession).sendRequest(mCaptorParcel.capture());
        MediaTestUtils.assertParcelEquals(testParcel, mCaptorParcel.getValue());
    }

    @Test
    public void testCallQualityChanged() {

        CallQuality callQuality = MediaTestUtils.createCallQuality();
        mAudioSessionCallbackHandler.callQualityChanged(callQuality);

        verify(mMockMtcMediaSession).callQualityChanged(eq(callQuality));
    }

    @Test
    public void testMediaDetach() {

        Parcel testParcel = Parcel.obtain();

        testParcel.writeInt(MediaConstants.NOTIFY_MEDIA_DETACH);
        mAudioSessionCallbackHandler.nofityMediaDetach();

        verify(mMockMtcMediaSession).sendRequest(mCaptorParcel.capture());
        MediaTestUtils.assertParcelEquals(testParcel, mCaptorParcel.getValue());
    }

    @Test
    public void testNotifyQosInfo() {

        Parcel testParcel = Parcel.obtain();

        testParcel.writeInt(MediaConstants.NOTIFY_QOS_INFO);
        testParcel.writeInt(ImsMediaSession.SESSION_TYPE_AUDIO);
        testParcel.writeString(MediaTestUtils.REMOTE_RTP_ADDRESS);
        testParcel.writeInt(MediaTestUtils.REMOTE_RTP_PORT);
        testParcel.writeBoolean(true);

        mAudioSessionCallbackHandler.onNotifyQosInfo(MediaTestUtils.REMOTE_RTP_ADDRESS,
                MediaTestUtils.REMOTE_RTP_PORT, true);

        verify(mMockMtcMediaSession).sendRequest(mCaptorParcel.capture());
        MediaTestUtils.assertParcelEquals(testParcel, mCaptorParcel.getValue());
    }

    @Test
    public void testOnNotifyIncomingDtmfReceived() {
        int dtmfDigit = 1;
        int duration = 200;
        mAudioSessionCallbackHandler.onNotifyIncomingDtmfReceived(dtmfDigit, duration);
        verify(mMockMtcMediaSession).onNotifyIncomingDtmfReceived(eq(dtmfDigit), eq(duration));
    }
}
