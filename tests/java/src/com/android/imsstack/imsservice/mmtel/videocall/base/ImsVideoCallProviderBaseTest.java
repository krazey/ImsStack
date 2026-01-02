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

package com.android.imsstack.imsservice.mmtel.videocall.base;

import static com.google.common.truth.Truth.assertThat;

import static org.mockito.ArgumentMatchers.any;
import static org.mockito.ArgumentMatchers.anyInt;
import static org.mockito.Mockito.mock;
import static org.mockito.Mockito.never;
import static org.mockito.Mockito.verify;
import static org.mockito.Mockito.when;

import android.net.Uri;
import android.os.RemoteException;
import android.telecom.VideoProfile;
import android.telephony.ims.ImsStreamMediaProfile;
import android.testing.AndroidTestingRunner;
import android.testing.TestableLooper;
import android.view.Surface;

import androidx.test.filters.SmallTest;

import com.android.imsstack.enabler.mtc.MtcMediaSession;
import com.android.imsstack.imsservice.mmtel.call.IVideoCallSession;
import com.android.imsstack.imsservice.mmtel.util.VideoDimension;

import org.junit.After;
import org.junit.Before;
import org.junit.Test;
import org.junit.runner.RunWith;
import org.mockito.ArgumentCaptor;
import org.mockito.Mockito;

@RunWith(AndroidTestingRunner.class)
@TestableLooper.RunWithLooper
public class ImsVideoCallProviderBaseTest extends ImsVideoCallProviderTestBase {
    @Override
    protected ImsVideoCallProviderBase createProvider() {
        return new ImsVideoCallProviderBase(mMockCallSession, mMockMediaSession);
    }

    @Before
    @Override
    public void setUp() throws Exception {
        super.setUp();
    }

    @After
    @Override
    public void tearDown() throws Exception {
        super.tearDown();
    }

    @SmallTest
    @Test
    public void testConstructor() {
        verify(mMockMediaSession).setVideoListener(any(MtcMediaSession.VideoListener.class));
        verify(mMockCallSession).setVideoCallProvider(mProvider);
        verify(mMockCallSession).setEventListener(mProvider);
    }

    @SmallTest
    @Test
    public void testFinalMethodsUseExecutor() {
        mProvider.onSetCamera("0");
        verify(mExecutor).execute(any(Runnable.class));
        Mockito.clearInvocations(mExecutor);

        Surface surface = mock(Surface.class);
        mProvider.onSetPreviewSurface(surface);
        verify(mExecutor).execute(any(Runnable.class));
        Mockito.clearInvocations(mExecutor);

        Surface surface2 = mock(Surface.class);
        mProvider.onSetDisplaySurface(surface2);
        verify(mExecutor).execute(any(Runnable.class));
        Mockito.clearInvocations(mExecutor);

        mProvider.onSetDeviceOrientation(0);
        verify(mExecutor).execute(any(Runnable.class));
        Mockito.clearInvocations(mExecutor);

        mProvider.onSetZoom(1.0f);
        verify(mExecutor).execute(any(Runnable.class));
        Mockito.clearInvocations(mExecutor);

        VideoProfile from = mock(VideoProfile.class);
        VideoProfile to = mock(VideoProfile.class);
        mProvider.onSendSessionModifyRequest(from, to);
        verify(mExecutor).execute(any(Runnable.class));
        Mockito.clearInvocations(mExecutor);

        VideoProfile response = mock(VideoProfile.class);
        mProvider.onSendSessionModifyResponse(response);
        verify(mExecutor).execute(any(Runnable.class));
        Mockito.clearInvocations(mExecutor);

        mProvider.onRequestCameraCapabilities();
        verify(mExecutor).execute(any(Runnable.class));
        Mockito.clearInvocations(mExecutor);

        mProvider.onRequestCallDataUsage();
        verify(mExecutor).execute(any(Runnable.class));
        Mockito.clearInvocations(mExecutor);

        Uri uri = mock(Uri.class);
        mProvider.onSetPauseImage(uri);
        verify(mExecutor).execute(any(Runnable.class));
        Mockito.clearInvocations(mExecutor);
    }

    @SmallTest
    @Test
    public void testSetCamera() {
        mProvider.setCamera("0");
        // Base implementation is empty, so no interactions are expected with mocks.
        verify(mMockCallSession, never()).onSetCamera(any());
    }

    @SmallTest
    @Test
    public void testSetPreviewSurface() {
        Surface mockSurface = mock(Surface.class);
        mProvider.setPreviewSurface(mockSurface);
        verify(mMockMediaSession).setPreviewSurface(mockSurface);
    }

    @SmallTest
    @Test
    public void testSetPreviewSurface_nullSession() {
        mProvider.updateMediaSession(null);
        Surface mockSurface = mock(Surface.class);
        mProvider.setPreviewSurface(mockSurface);
        verify(mMockMediaSession, never()).setPreviewSurface(mockSurface);
    }

    @SmallTest
    @Test
    public void testSetDisplaySurface() {
        Surface mockSurface = mock(Surface.class);
        mProvider.setDisplaySurface(mockSurface);
        verify(mMockMediaSession).setDisplaySurface(mockSurface);
    }

    @SmallTest
    @Test
    public void testSetDisplaySurface_nullSession() {
        mProvider.updateMediaSession(null);
        Surface mockSurface = mock(Surface.class);
        mProvider.setDisplaySurface(mockSurface);
        verify(mMockMediaSession, never()).setDisplaySurface(mockSurface);
    }

    @SmallTest
    @Test
    public void testSetDeviceOrientation() {
        mProvider.setDeviceOrientation(180);
        verify(mMockMediaSession).setDeviceOrientation(180);
    }

    @SmallTest
    @Test
    public void testSetDeviceOrientation_nullSession() {
        mProvider.updateMediaSession(null);
        mProvider.setDeviceOrientation(180);
        verify(mMockMediaSession, never()).setDeviceOrientation(180);
    }

    @SmallTest
    @Test
    public void testSetZoom() {
        mProvider.setZoom(3.0f);
        verify(mMockMediaSession).setCameraZoom(3);
    }

    @SmallTest
    @Test
    public void testSetZoom_nullSession() {
        mProvider.updateMediaSession(null);
        mProvider.setZoom(3.0f);
        verify(mMockMediaSession, never()).setCameraZoom(3);
    }

    @SmallTest
    @Test
    public void testSendSessionModifyRequest() {
        VideoProfile fromProfile = new VideoProfile(VideoProfile.STATE_AUDIO_ONLY);
        VideoProfile toProfile = new VideoProfile(VideoProfile.STATE_BIDIRECTIONAL);
        mProvider.sendSessionModifyRequest(fromProfile, toProfile);
        verify(mMockCallSession).sendSessionModifyRequest(fromProfile, toProfile);
    }

    @SmallTest
    @Test
    public void testSendSessionModifyResponse() {
        VideoProfile responseProfile = new VideoProfile(VideoProfile.STATE_BIDIRECTIONAL);
        mProvider.sendSessionModifyResponse(responseProfile);
        verify(mMockCallSession).sendSessionModifyResponse(responseProfile);
    }

    @SmallTest
    @Test
    public void testRequestCameraCapabilities() throws RemoteException {
        mProvider.requestCameraCapabilities();
        // Base implementation is empty.
        verify(mMockCallback, never()).changeCameraCapabilities(any());
    }

    @SmallTest
    @Test
    public void testRequestCallDataUsage() {
        mProvider.requestCallDataUsage();
        verify(mMockMediaSession).requestCallDataUsage();
    }

    @SmallTest
    @Test
    public void testRequestCallDataUsage_nullSession() {
        // Make the internal media session null
        mProvider.updateMediaSession(null);
        mProvider.requestCallDataUsage();
        // Verify the mock was NOT called, preventing a crash
        verify(mMockMediaSession, never()).requestCallDataUsage();
    }

    @SmallTest
    @Test
    public void testSetPauseImage() {
        mProvider.setPauseImage(mock(Uri.class));
        // Base implementation is empty. No crash is a pass.
    }

    @SmallTest
    @Test
    public void testOnSessionModificationAbortedByCameraOff() {
        mProvider.onSessionModificationAbortedByCameraOff();
        // Base implementation is empty. No crash is a pass.
    }

    @SmallTest
    @Test
    public void testUpdateMediaSession() {
        MtcMediaSession newMockMediaSession = mock(MtcMediaSession.class);
        mProvider.updateMediaSession(newMockMediaSession);

        verify(mMockMediaSession).setVideoListener(null);
        verify(newMockMediaSession).setVideoListener(any(MtcMediaSession.VideoListener.class));
        assertThat(mProvider.getMediaSession()).isEqualTo(newMockMediaSession);
    }

    @SmallTest
    @Test
    public void testClose() {
        mProvider.close();
        verify(mMockCallSession).setEventListener(null);
        verify(mMockMediaSession).setVideoListener(null);
        assertThat(mProvider.getMediaSession()).isNull();
    }

    @SmallTest
    @Test
    public void testGetVideoCallSession() {
        assertThat(mProvider.getVideoCallSession()).isEqualTo(mMockCallSession);
    }

    @SmallTest
    @Test
    public void testHandleCallEvent() {
        mProvider.handleCallEvent(IVideoCallSession.EVENT_CALL_ESTABLISHED);
        assertThat(mProvider.getCallState())
                .isEqualTo(ImsVideoCallProviderBase.CALL_STATE_ESTABLISHED);

        mProvider.handleCallEvent(IVideoCallSession.EVENT_CALL_TERMINATED);
        assertThat(mProvider.getCallState())
                .isEqualTo(ImsVideoCallProviderBase.CALL_STATE_TERMINATED);
    }

    @SmallTest
    @Test
    public void testHandleMediaSessionStarted() throws RemoteException {
        mProvider.handleMediaSessionStarted();
        // Base implementation is empty.
        verify(mMockCallback, never()).changePeerDimensions(anyInt(), anyInt());
    }

    @SmallTest
    @Test
    public void testMediaSessionListener_onDataUsageChanged() throws RemoteException {
        ArgumentCaptor<MtcMediaSession.VideoListener> listenerCaptor =
                ArgumentCaptor.forClass(MtcMediaSession.VideoListener.class);
        verify(mMockMediaSession).setVideoListener(listenerCaptor.capture());
        MtcMediaSession.VideoListener listener = listenerCaptor.getValue();

        listener.onMediaSessionDataUsageChanged(mMockMediaSession, 1024);

        verify(mMockCallback).changeCallDataUsage(1024);
    }

    @SmallTest
    @Test
    public void testHandleMediaSessionMediaInfoChanged() throws RemoteException {
        mProvider.handleMediaSessionMediaInfoChanged(0, 0, null);
        // Base implementation is empty.
        verify(mMockCallback, never()).handleCallSessionEvent(anyInt());
    }

    @SmallTest
    @Test
    public void testHandleMediaSessionPeerFirstVideoReceived() throws RemoteException {
        mProvider.handleMediaSessionPeerFirstVideoReceived();
        // Base implementation is empty.
        verify(mMockCallback, never()).handleCallSessionEvent(anyInt());
    }

    @SmallTest
    @Test
    public void testHandleMediaSessionPeerDisplayOrientationChanged() throws RemoteException {
        mProvider.handleMediaSessionPeerDisplayOrientationChanged(0);
        // Base implementation is empty.
        verify(mMockCallback, never()).changePeerDimensions(anyInt(), anyInt());
    }

    @SmallTest
    @Test
    public void testMediaSessionListener_onPeerDimensionsChanged() throws RemoteException {
        ArgumentCaptor<MtcMediaSession.VideoListener> listenerCaptor =
                ArgumentCaptor.forClass(MtcMediaSession.VideoListener.class);
        verify(mMockMediaSession).setVideoListener(listenerCaptor.capture());
        MtcMediaSession.VideoListener listener = listenerCaptor.getValue();

        listener.onMediaSessionPeerDimensionsChanged(mMockMediaSession, 1920, 1080);

        verify(mMockCallback).changePeerDimensions(1920, 1080);
    }

    @SmallTest
    @Test
    public void testSetAndGetCurrentVideoDimension() {
        assertThat(mProvider.getCurrentVideoDimension()).isNull();
        mProvider.setCurrentVideoDimension(10, 20);
        VideoDimension vd = mProvider.getCurrentVideoDimension();
        assertThat(vd).isNotNull();
        assertThat(vd.getWidth()).isEqualTo(10);
        assertThat(vd.getHeight()).isEqualTo(20);
    }

    @SmallTest
    @Test
    public void testUpdateReversedPeerDimensionFromMediaProfile_toPortrait()
            throws RemoteException {
        when(mMockCallSession.getStreamMediaProfile()).thenReturn(mMockMediaProfile);
        when(mMockMediaProfile.getVideoQuality())
                .thenReturn(ImsStreamMediaProfile.VIDEO_QUALITY_VGA_LANDSCAPE);

        mProvider.updateReversedPeerDimensionFromMediaProfile(
                VideoCallUtils.ORIENTATION_PORTRAIT, false);

        verify(mMockCallback).changePeerDimensions(480, 640);
    }

    @SmallTest
    @Test
    public void testUpdateReversedPeerDimensionFromMediaProfile_toLandscape()
            throws RemoteException {
        when(mMockCallSession.getStreamMediaProfile()).thenReturn(mMockMediaProfile);
        when(mMockMediaProfile.getVideoQuality())
                .thenReturn(ImsStreamMediaProfile.VIDEO_QUALITY_VGA_PORTRAIT);

        mProvider.updateReversedPeerDimensionFromMediaProfile(
                VideoCallUtils.ORIENTATION_LANDSCAPE, false);

        verify(mMockCallback).changePeerDimensions(640, 480);
    }

    @SmallTest
    @Test
    public void testUpdateReversedPeerDimensionFromMediaProfile_noChange() throws RemoteException {
        when(mMockCallSession.getStreamMediaProfile()).thenReturn(mMockMediaProfile);
        when(mMockMediaProfile.getVideoQuality())
                .thenReturn(ImsStreamMediaProfile.VIDEO_QUALITY_VGA_LANDSCAPE);

        mProvider.updateReversedPeerDimensionFromMediaProfile(
                VideoCallUtils.ORIENTATION_LANDSCAPE, false);

        verify(mMockCallback, never()).changePeerDimensions(anyInt(), anyInt());
    }

    @SmallTest
    @Test
    public void testUpdateReversedPeerDimensionFromMediaProfile_enforceUpdate()
            throws RemoteException {
        when(mMockCallSession.getStreamMediaProfile()).thenReturn(mMockMediaProfile);
        when(mMockMediaProfile.getVideoQuality())
                .thenReturn(ImsStreamMediaProfile.VIDEO_QUALITY_VGA_LANDSCAPE);

        mProvider.updateReversedPeerDimensionFromMediaProfile(
                VideoCallUtils.ORIENTATION_LANDSCAPE, true);

        verify(mMockCallback).changePeerDimensions(640, 480);
    }

    @SmallTest
    @Test
    public void testUpdateReversedPeerDimensionFromMediaProfile_nullProfile()
            throws RemoteException {
        when(mMockCallSession.getStreamMediaProfile()).thenReturn(null);
        mProvider.updateReversedPeerDimensionFromMediaProfile(
                VideoCallUtils.ORIENTATION_LANDSCAPE, false);
        verify(mMockCallback, never()).changePeerDimensions(anyInt(), anyInt());
    }

    @SmallTest
    @Test
    public void testUpdateReversedPeerDimensionFromVideoDimension_toPortrait()
            throws RemoteException {
        mProvider.setCurrentVideoDimension(640, 480); // landscape
        mProvider.updateReversedPeerDimensionFromVideoDimension(
                VideoCallUtils.ORIENTATION_PORTRAIT, false);
        verify(mMockCallback).changePeerDimensions(480, 640);
        VideoDimension vd = mProvider.getCurrentVideoDimension();
        assertThat(vd.getWidth()).isEqualTo(480);
        assertThat(vd.getHeight()).isEqualTo(640);
    }

    @SmallTest
    @Test
    public void testUpdateReversedPeerDimensionFromVideoDimension_toLandscape()
            throws RemoteException {
        mProvider.setCurrentVideoDimension(480, 640); // portrait
        mProvider.updateReversedPeerDimensionFromVideoDimension(
                VideoCallUtils.ORIENTATION_LANDSCAPE, false);
        verify(mMockCallback).changePeerDimensions(640, 480);
        VideoDimension vd = mProvider.getCurrentVideoDimension();
        assertThat(vd.getWidth()).isEqualTo(640);
        assertThat(vd.getHeight()).isEqualTo(480);
    }

    @SmallTest
    @Test
    public void testUpdateReversedPeerDimensionFromVideoDimension_noChange()
            throws RemoteException {
        mProvider.setCurrentVideoDimension(640, 480); // landscape
        mProvider.updateReversedPeerDimensionFromVideoDimension(
                VideoCallUtils.ORIENTATION_LANDSCAPE, false);
        verify(mMockCallback, never()).changePeerDimensions(anyInt(), anyInt());
    }

    @SmallTest
    @Test
    public void testUpdateReversedPeerDimensionFromVideoDimension_enforceUpdate()
            throws RemoteException {
        mProvider.setCurrentVideoDimension(640, 480); // landscape
        mProvider.updateReversedPeerDimensionFromVideoDimension(
                VideoCallUtils.ORIENTATION_LANDSCAPE, true);
        verify(mMockCallback).changePeerDimensions(640, 480);
    }

    @SmallTest
    @Test
    public void testCheckRadius() {
        assertThat(ImsVideoCallProviderBase.checkRadius(90, 90, 45)).isTrue();
        assertThat(ImsVideoCallProviderBase.checkRadius(45, 90, 45)).isTrue();
        assertThat(ImsVideoCallProviderBase.checkRadius(134, 90, 45)).isTrue();
        assertThat(ImsVideoCallProviderBase.checkRadius(44, 90, 45)).isFalse();
        assertThat(ImsVideoCallProviderBase.checkRadius(135, 90, 45)).isFalse();
    }

    @SmallTest
    @Test
    public void testGetQuartile() {
        assertThat(ImsVideoCallProviderBase.getQuartile(0)).isEqualTo(0);
        assertThat(ImsVideoCallProviderBase.getQuartile(44)).isEqualTo(0);
        assertThat(ImsVideoCallProviderBase.getQuartile(45)).isEqualTo(90);
        assertThat(ImsVideoCallProviderBase.getQuartile(90)).isEqualTo(90);
        assertThat(ImsVideoCallProviderBase.getQuartile(134)).isEqualTo(90);
        assertThat(ImsVideoCallProviderBase.getQuartile(135)).isEqualTo(180);
        assertThat(ImsVideoCallProviderBase.getQuartile(180)).isEqualTo(180);
        assertThat(ImsVideoCallProviderBase.getQuartile(224)).isEqualTo(180);
        assertThat(ImsVideoCallProviderBase.getQuartile(225)).isEqualTo(270);
        assertThat(ImsVideoCallProviderBase.getQuartile(270)).isEqualTo(270);
        assertThat(ImsVideoCallProviderBase.getQuartile(314)).isEqualTo(270);
        assertThat(ImsVideoCallProviderBase.getQuartile(315)).isEqualTo(0);
    }
}
