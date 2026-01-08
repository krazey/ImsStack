/*
 * Copyright (C) 2025 The Android Open Source Project
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
import static org.junit.Assert.assertNotNull;
import static org.mockito.ArgumentMatchers.any;
import static org.mockito.ArgumentMatchers.anyInt;
import static org.mockito.ArgumentMatchers.eq;
import static org.mockito.Mockito.mock;
import static org.mockito.Mockito.never;
import static org.mockito.Mockito.times;
import static org.mockito.Mockito.verify;
import static org.mockito.Mockito.when;

import android.content.Context;
import android.os.Parcel;
import android.telephony.imsmedia.ImsMediaManager;
import android.telephony.imsmedia.ImsMediaSession;
import android.telephony.imsmedia.RtpReceptionStats;
import android.testing.AndroidTestingRunner;

import com.android.imsstack.ImsStackTest;
import com.android.imsstack.base.AppContext;
import com.android.imsstack.enabler.IBaseContext;
import com.android.imsstack.enabler.mtc.MtcMediaSession;

import org.junit.After;
import org.junit.Before;
import org.junit.Test;
import org.junit.runner.RunWith;
import org.mockito.ArgumentCaptor;
import org.mockito.Mock;
import org.mockito.MockitoAnnotations;

import java.util.concurrent.Executor;

@RunWith(AndroidTestingRunner.class)
public class MediaSessionTest extends ImsStackTest {

    @Mock private IBaseContext mMockBaseContext;
    @Mock private MtcMediaSession mMockMtcMediaSession;
    @Mock private ImsMediaManager mMockImsMediaManager;
    @Mock private Executor mMockExecutor;
    @Mock private AudioSessionHandler mMockAudioSessionHandler;
    @Mock private VideoSessionHandler mMockVideoSessionHandler;
    @Mock private TextSessionHandler mMockTextSessionHandler;
    @Mock private AudioVideoSync mMockAvSync;
    @Mock private Context mMockContext;

    private MediaSession mMediaSession;

    @Before
    public void setUp() throws Exception {
        super.setUp(getClass().getSimpleName());

        // Initialize Mock Objects
        MockitoAnnotations.initMocks(this);
        when(mMockBaseContext.getContext()).thenReturn(mMockContext);
        // create the instance to test
        mMediaSession =
                new MediaSession(
                        mMockBaseContext,
                        mMockMtcMediaSession,
                        mMockImsMediaManager,
                        mMockExecutor);
        mMediaSession.setAudioSessionHandler(mMockAudioSessionHandler);
        mMediaSession.setVideoSessionHandler(mMockVideoSessionHandler);
        mMediaSession.setTextSessionHandler(mMockTextSessionHandler);
        mMediaSession.setAvSync(mMockAvSync);
    }

    @After
    public void tearDown() throws Exception {
        mMediaSession = null;
        super.tearDown();
    }

    @Test
    public void testConstructor_withImsMediaManager() {

        assertNotNull(mMediaSession.getMediaManager());
        assertNotNull(mMediaSession.getMediaListenerProxy());
        verify(mMockMtcMediaSession).setMediaListener(any(IMediaListener.class));
    }

    @Test
    public void testConstructor_withoutImsMediaManager() {
        AppContext.init(mContext);
        MediaSession mediaSession = new MediaSession(mMockBaseContext, mMockMtcMediaSession);

        assertNotNull(mediaSession.getMediaManager());
        assertNotNull(mediaSession.getMediaListenerProxy());
        verify(mMockMtcMediaSession, times(2)).setMediaListener(any(IMediaListener.class));
        AppContext.deinit();
    }

    @Test
    public void testNotifyRtpReceptionStats_audio() {
        when(mMockAudioSessionHandler.getSamplingRateKHz()).thenReturn(8);
        RtpReceptionStats stats = mock(RtpReceptionStats.class);

        mMediaSession.notifyRtpReceptionStats(ImsMediaSession.SESSION_TYPE_AUDIO, stats);

        verify(mMockAvSync).setAudioClockRate(8);
        verify(mMockAvSync).setAudioStats(stats);
    }

    @Test
    public void testNotifyRtpReceptionStats_video() {
        when(mMockVideoSessionHandler.isValidRequest(MediaConstants.REQUEST_ADJUST_DELAY))
                .thenReturn(true);
        when(mMockVideoSessionHandler.getSamplingRateKHz()).thenReturn(16);
        when(mMockAvSync.getVideoDelay()).thenReturn(100);
        RtpReceptionStats stats = mock(RtpReceptionStats.class);
        Parcel testParcel = Parcel.obtain();
        testParcel.writeInt(MediaConstants.REQUEST_ADJUST_DELAY);
        testParcel.writeInt(ImsMediaSession.SESSION_TYPE_VIDEO);
        testParcel.writeInt(100);
        testParcel.setDataPosition(0);

        mMediaSession.notifyRtpReceptionStats(ImsMediaSession.SESSION_TYPE_VIDEO, stats);

        verify(mMockAvSync).setVideoClockRate(16);
        verify(mMockAvSync).setVideoStats(stats);
        ArgumentCaptor<Parcel> parcelCaptor = ArgumentCaptor.forClass(Parcel.class);
        verify(mMockVideoSessionHandler)
                .onImsMediaVideoMessage(
                        eq(MediaConstants.REQUEST_ADJUST_DELAY), parcelCaptor.capture());
        MediaTestUtils.assertParcelEquals(testParcel, parcelCaptor.getValue());
    }

    @Test
    public void testNotifyRtpReceptionStats_text() {

        RtpReceptionStats stats = mock(RtpReceptionStats.class);

        mMediaSession.notifyRtpReceptionStats(ImsMediaSession.SESSION_TYPE_RTT, stats);

        verify(mMockAvSync, never()).setAudioClockRate(anyInt());
        verify(mMockAvSync, never()).setAudioStats(any());
        verify(mMockAvSync, never()).setVideoClockRate(anyInt());
        verify(mMockAvSync, never()).setVideoStats(any());
    }

    @Test
    public void testNotifyRtpReceptionStats_noAvSync() {
        mMediaSession.setAvSync(null);
        RtpReceptionStats stats = mock(RtpReceptionStats.class);

        mMediaSession.notifyRtpReceptionStats(ImsMediaSession.SESSION_TYPE_AUDIO, stats);

        verify(mMockAvSync, never()).setAudioStats(any());
    }

    @Test
    public void testRequestCallDataUsage() {
        when(mMockVideoSessionHandler.isValidRequest(MediaConstants.REQUEST_VIDEO_DATA_USAGE))
                .thenReturn(true);

        Parcel testParcel = Parcel.obtain();
        testParcel.writeInt(MediaConstants.REQUEST_VIDEO_DATA_USAGE);
        testParcel.writeInt(ImsMediaSession.SESSION_TYPE_VIDEO);

        mMediaSession.requestCallDataUsage();

        ArgumentCaptor<Parcel> parcelCaptor = ArgumentCaptor.forClass(Parcel.class);
        verify(mMockVideoSessionHandler)
                .onImsMediaVideoMessage(
                        eq(MediaConstants.REQUEST_VIDEO_DATA_USAGE), parcelCaptor.capture());
        MediaTestUtils.assertParcelEquals(testParcel, parcelCaptor.getValue());
    }

    @Test
    public void testRequestCallDataUsage_noHandler() {
        mMediaSession.setVideoSessionHandler(null);

        mMediaSession.requestCallDataUsage();

        verify(mMockVideoSessionHandler, never()).onImsMediaVideoMessage(anyInt(), any());
    }

    @Test
    public void testOnMediaMessage_UnknownSessionType() {

        Parcel testParcel = Parcel.obtain();
        testParcel.writeInt(MediaConstants.REQUEST_VIDEO_DATA_USAGE);
        testParcel.writeInt(5);
        testParcel.setDataPosition(0);

        mMediaSession.getMediaListenerProxy().onMediaMessage(testParcel);

        verify(mMockAudioSessionHandler, never()).onImsMediaAudioMessage(anyInt(), any());
        verify(mMockVideoSessionHandler, never()).onImsMediaVideoMessage(anyInt(), any());
        verify(mMockTextSessionHandler, never()).onImsMediaTextMessage(anyInt(), any());
    }

    @Test
    public void testCreateAudioSession() {
        when(mMockBaseContext.getSlotId()).thenReturn(1);
        mMediaSession.setAudioSessionHandler(null);

        Parcel testParcel = Parcel.obtain();
        testParcel.writeInt(MediaConstants.REQUEST_SEND_DTMF);
        testParcel.writeInt(ImsMediaSession.SESSION_TYPE_AUDIO);
        testParcel.setDataPosition(0);

        mMediaSession.getMediaListenerProxy().onMediaMessage(testParcel);

        assertNotNull(mMediaSession.getAudioSessionHandler());
    }

    @Test
    public void testCreateVideoSession() {
        when(mMockBaseContext.getSlotId()).thenReturn(1);
        mMediaSession.setVideoSessionHandler(null);

        Parcel testParcel = Parcel.obtain();
        testParcel.writeInt(MediaConstants.REQUEST_OPEN_SESSION);
        testParcel.writeInt(ImsMediaSession.SESSION_TYPE_VIDEO);
        testParcel.setDataPosition(0);

        mMediaSession.getMediaListenerProxy().onMediaMessage(testParcel);

        assertNotNull(mMediaSession.getVideoSessionHandler());
    }

    @Test
    public void testCreateTextSession() {
        when(mMockBaseContext.getSlotId()).thenReturn(1);
        mMediaSession.setTextSessionHandler(null);

        Parcel testParcel = Parcel.obtain();
        testParcel.writeInt(MediaConstants.REQUEST_OPEN_SESSION);
        testParcel.writeInt(ImsMediaSession.SESSION_TYPE_RTT);
        testParcel.setDataPosition(0);

        mMediaSession.getMediaListenerProxy().onMediaMessage(testParcel);

        assertNotNull(mMediaSession.getTextSessionHandler());
    }

    @Test
    public void testSendRtt_noHandler() {
        mMediaSession.setTextSessionHandler(null);

        Parcel testParcel = Parcel.obtain();
        testParcel.writeInt(MediaConstants.REQUEST_SEND_RTT);
        testParcel.writeInt(ImsMediaSession.SESSION_TYPE_RTT);
        testParcel.setDataPosition(0);

        mMediaSession.getMediaListenerProxy().onMediaMessage(testParcel);

        verify(mMockTextSessionHandler, never()).onImsMediaTextMessage(anyInt(), any());
    }

    @Test
    public void testOnMediaConnected() {
        mMediaSession.onMediaConnected();
        // This method only logs a message,
        // so there is nothing to verify other than that it doesn't crash.
    }

    @Test
    public void testOnMediaDisconnected_withHandlers() {
        mMediaSession.onMediaDisconnected();

        verify(mMockAudioSessionHandler)
                .onImsMediaAudioMessage(eq(MediaConstants.NOTIFY_MEDIA_DETACH), eq(null));
        verify(mMockVideoSessionHandler)
                .onImsMediaVideoMessage(eq(MediaConstants.NOTIFY_MEDIA_DETACH), eq(null));
        verify(mMockTextSessionHandler)
                .onImsMediaTextMessage(eq(MediaConstants.NOTIFY_MEDIA_DETACH), eq(null));
    }

    @Test
    public void testOnMediaDisconnected_withoutHandlers() {
        AppContext.init(mContext);
        MediaSession mediaSession = new MediaSession(mMockBaseContext, mMockMtcMediaSession);
        mediaSession.setAudioSessionHandler(null);

        mediaSession.onMediaDisconnected();

        verify(mMockAudioSessionHandler, never())
                .onImsMediaAudioMessage(eq(MediaConstants.NOTIFY_MEDIA_DETACH), eq(null));
        verify(mMockVideoSessionHandler, never())
                .onImsMediaVideoMessage(eq(MediaConstants.NOTIFY_MEDIA_DETACH), eq(null));
        verify(mMockTextSessionHandler, never())
                .onImsMediaTextMessage(eq(MediaConstants.NOTIFY_MEDIA_DETACH), eq(null));
        AppContext.deinit();
    }

    @Test
    public void testSetSessionHandlers() {
        MediaSession mediaSession =
                new MediaSession(mMockBaseContext, null, mMockImsMediaManager,
                    mMockExecutor);
        mediaSession.setAudioSessionHandler(mMockAudioSessionHandler);
        mediaSession.setVideoSessionHandler(mMockVideoSessionHandler);
        mediaSession.setTextSessionHandler(mMockTextSessionHandler);
        mediaSession.setAvSync(mMockAvSync);

        assertEquals(mMockAudioSessionHandler, mediaSession.getAudioSessionHandler());
        assertEquals(mMockVideoSessionHandler, mediaSession.getVideoSessionHandler());
        assertEquals(mMockTextSessionHandler, mediaSession.getTextSessionHandler());
    }
}
