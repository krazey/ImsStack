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

import static org.mockito.Matchers.eq;
import static org.mockito.Mockito.never;
import static org.mockito.Mockito.verify;

import android.os.Parcel;
import android.os.Parcelable;
import android.telephony.imsmedia.ImsMediaSession;
import android.telephony.imsmedia.TextConfig;

import com.android.imsstack.enabler.mtc.MtcMediaSession;

import org.junit.Before;
import org.junit.Test;
import org.junit.runner.RunWith;
import org.junit.runners.JUnit4;
import org.mockito.ArgumentCaptor;
import org.mockito.Captor;
import org.mockito.Mock;
import org.mockito.MockitoAnnotations;

@RunWith(JUnit4.class)
public class TextSessionCallbackHandlerTest {

    @Mock MtcMediaSession mMockMtcMediaSession;
    @Captor ArgumentCaptor<Parcel> mCaptorParcel;

    private TextSessionCallbackHandler mTextSessionCallbackHandler;

    @Before
    public void setUp() throws Exception {
        MockitoAnnotations.initMocks(this);
        // create the instance to test
        mTextSessionCallbackHandler = new TextSessionCallbackHandler(mMockMtcMediaSession);
    }

    @Test
    public void testOpenSessionSuccess() {

        Parcel testParcel = Parcel.obtain();
        testParcel.writeInt(MediaConstants.RESPONSE_OPEN_SESSION);
        testParcel.writeInt(ImsMediaSession.SESSION_TYPE_RTT);
        testParcel.writeInt(ImsMediaSession.RESULT_SUCCESS);

        mTextSessionCallbackHandler.openSessionResponse(ImsMediaSession.RESULT_SUCCESS);

        verify(mMockMtcMediaSession).sendRequest(mCaptorParcel.capture());
        MediaTestUtils.assertParcelEquals(testParcel, mCaptorParcel.getValue());
    }

    @Test
    public void testOpenSessionFailure() {

        Parcel testParcel = Parcel.obtain();
        testParcel.writeInt(MediaConstants.RESPONSE_OPEN_SESSION);
        testParcel.writeInt(ImsMediaSession.SESSION_TYPE_RTT);
        testParcel.writeInt(ImsMediaSession.RESULT_NO_RESOURCES);

        mTextSessionCallbackHandler.openSessionResponse(ImsMediaSession.RESULT_NO_RESOURCES);

        verify(mMockMtcMediaSession).sendRequest(mCaptorParcel.capture());
        MediaTestUtils.assertParcelEquals(testParcel, mCaptorParcel.getValue());
    }

    @Test
    public void testModifySessionResponse() {

        TextConfig textConfig = MediaTestUtils.createTextConfig();
        Parcel testParcel = Parcel.obtain();

        testParcel.writeInt(MediaConstants.RESPONSE_MODIFY_SESSION);
        testParcel.writeInt(ImsMediaSession.SESSION_TYPE_RTT);
        textConfig.writeToParcel(testParcel, Parcelable.PARCELABLE_WRITE_RETURN_VALUE);
        testParcel.writeInt(ImsMediaSession.RESULT_SUCCESS);

        mTextSessionCallbackHandler.modifySessionResponse(textConfig,
                ImsMediaSession.RESULT_SUCCESS);

        verify(mMockMtcMediaSession).sendRequest(mCaptorParcel.capture());
        MediaTestUtils.assertParcelEquals(testParcel, mCaptorParcel.getValue());
    }

    @Test
    public void testRttReceived() {
        mTextSessionCallbackHandler.onRttReceived(MediaTestUtils.RTT_MESSAGE);
        verify(mMockMtcMediaSession).rttMessageReceived(eq(MediaTestUtils.RTT_MESSAGE));
    }

    @Test
    public void testRttReceivedWithNoText() {
        mTextSessionCallbackHandler.onRttReceived("");
        verify(mMockMtcMediaSession).rttMessageReceived(eq(""));
    }

    @Test
    public void testRttReceivedWithTextNull() {
        mTextSessionCallbackHandler.onRttReceived(null);
        verify(mMockMtcMediaSession, never()).rttMessageReceived(null);
    }

    @Test
    public void testNotifyMediaInactivity() {

        Parcel testParcel = Parcel.obtain();

        testParcel.writeInt(MediaConstants.NOTIFY_MEDIA_INACTIVITY);
        testParcel.writeInt(ImsMediaSession.SESSION_TYPE_RTT);
        testParcel.writeInt(ImsMediaSession.PACKET_TYPE_RTP);

        mTextSessionCallbackHandler.onNotifyMediaInactivity(ImsMediaSession.PACKET_TYPE_RTP);

        verify(mMockMtcMediaSession).sendRequest(mCaptorParcel.capture());
        MediaTestUtils.assertParcelEquals(testParcel, mCaptorParcel.getValue());
    }

    @Test
    public void testNotifyQosInfo() {

        Parcel testParcel = Parcel.obtain();

        testParcel.writeInt(MediaConstants.NOTIFY_QOS_INFO);
        testParcel.writeInt(ImsMediaSession.SESSION_TYPE_RTT);
        testParcel.writeString(MediaTestUtils.REMOTE_RTP_ADDRESS);
        testParcel.writeInt(MediaTestUtils.REMOTE_RTP_PORT);
        testParcel.writeBoolean(true);

        mTextSessionCallbackHandler.onNotifyQosInfo(MediaTestUtils.REMOTE_RTP_ADDRESS,
                MediaTestUtils.REMOTE_RTP_PORT, true);

        verify(mMockMtcMediaSession).sendRequest(mCaptorParcel.capture());
        MediaTestUtils.assertParcelEquals(testParcel, mCaptorParcel.getValue());
    }
}
