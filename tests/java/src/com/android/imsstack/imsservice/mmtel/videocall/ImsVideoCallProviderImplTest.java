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

package com.android.imsstack.imsservice.mmtel.videocall;

import static com.google.common.truth.Truth.assertThat;

import static org.mockito.ArgumentMatchers.any;
import static org.mockito.ArgumentMatchers.anyBoolean;
import static org.mockito.ArgumentMatchers.argThat;
import static org.mockito.ArgumentMatchers.eq;
import static org.mockito.Mockito.mock;
import static org.mockito.Mockito.never;
import static org.mockito.Mockito.times;
import static org.mockito.Mockito.verify;
import static org.mockito.Mockito.when;

import android.graphics.SurfaceTexture;
import android.hardware.camera2.CameraCharacteristics;
import android.hardware.camera2.CameraManager;
import android.hardware.camera2.params.StreamConfigurationMap;
import android.os.Handler;
import android.os.Looper;
import android.os.RemoteException;
import android.telecom.Connection;
import android.telecom.VideoProfile;
import android.telephony.ims.ImsStreamMediaProfile;
import android.testing.AndroidTestingRunner;
import android.testing.TestableLooper;
import android.util.Size;

import androidx.test.filters.SmallTest;

import com.android.imsstack.base.TestAppContext;
import com.android.imsstack.core.agents.AgentFactory;
import com.android.imsstack.core.agents.ConfigInterface;
import com.android.imsstack.core.config.CarrierConfig;
import com.android.imsstack.enabler.mtc.MtcCallUtils;
import com.android.imsstack.imsservice.mmtel.call.IVideoCallSession;
import com.android.imsstack.imsservice.mmtel.videocall.base.ImsVideoCallProviderBase;
import com.android.imsstack.imsservice.mmtel.videocall.base.ImsVideoCallProviderTestBase;

import org.junit.After;
import org.junit.Before;
import org.junit.Test;
import org.junit.runner.RunWith;
import org.mockito.ArgumentCaptor;
import org.mockito.Mock;

@RunWith(AndroidTestingRunner.class)
@TestableLooper.RunWithLooper
public class ImsVideoCallProviderImplTest extends ImsVideoCallProviderTestBase {

    private static final int SLOT_ID = 0;

    @Mock private ConfigInterface mMockConfigInterface;
    @Mock private CarrierConfig mMockCarrierConfig;

    private TestAppContext mTestAppContext;

    @Override
    protected ImsVideoCallProviderBase createProvider() {
        return new ImsVideoCallProviderImpl(mMockCallSession, mMockMediaSession);
    }

    @Before
    @Override
    public void setUp() throws Exception {
        super.setUp();
        mTestAppContext = new TestAppContext(mContext);
        mTestAppContext.setUp();

        when(mMockConfigInterface.getCarrierConfig()).thenReturn(mMockCarrierConfig);
        when(mMockCallContext.getSlotId()).thenReturn(SLOT_ID);
        when(mMockCallContext.getCallHandler()).thenReturn(new Handler(Looper.myLooper()));
        AgentFactory.getInstance().setAgent(ConfigInterface.class, mMockConfigInterface, SLOT_ID);
    }

    @After
    @Override
    public void tearDown() throws Exception {
        if (mTestAppContext != null) {
            mTestAppContext.tearDown();
        }
        AgentFactory.getInstance().setAgent(ConfigInterface.class, null, SLOT_ID);
        super.tearDown();
    }

    private ImsVideoCallProviderImpl getProviderImpl() {
        return (ImsVideoCallProviderImpl) mProvider;
    }

    @SmallTest
    @Test
    public void testReceiveSessionModifyRequest_audioDowngrade() {
        when(mMockCallSession.getSessionModificationType())
                .thenReturn(IVideoCallSession.MODIFICATION_CALL_TYPE);
        VideoProfile audioOnlyProfile = new VideoProfile(VideoProfile.STATE_AUDIO_ONLY);

        getProviderImpl().receiveSessionModifyRequest(audioOnlyProfile);

        verify(mMockCallSession).sendSessionModifyResponse(
                argThat(p -> p != null && p.getVideoState() == VideoProfile.STATE_AUDIO_ONLY));
    }

    @SmallTest
    @Test
    public void testReceiveSessionModifyRequest_videoUpgrade_dynamicQualitySupported()
            throws RemoteException {
        when(mMockCarrierConfig.getBoolean(
                eq(CarrierConfig.ImsVt.KEY_DYNAMIC_VIDEO_QUALITY_SUPPORTED_BOOL)))
                .thenReturn(true);
        when(mMockCallSession.getSessionModificationType())
                .thenReturn(IVideoCallSession.MODIFICATION_CALL_TYPE);
        when(mMockCallSession.getProposedStreamMediaProfile()).thenReturn(mMockMediaProfile);
        when(mMockMediaProfile.getVideoQuality()).thenReturn(
                ImsStreamMediaProfile.VIDEO_QUALITY_QCIF);
        VideoProfile videoProfile = new VideoProfile(VideoProfile.STATE_BIDIRECTIONAL);

        getProviderImpl().receiveSessionModifyRequest(videoProfile);

        verify(mMockCallSession).getProposedStreamMediaProfile();
        verify(mMockCallback).changePeerDimensions(any(int.class), any(int.class));
        verify(mMockCallback).receiveSessionModifyRequest(videoProfile);
    }

    @SmallTest
    @Test
    public void testReceiveSessionModifyRequest_videoUpgrade_dynamicQualityNotSupported()
            throws RemoteException {
        when(mMockCarrierConfig.getBoolean(
                eq(CarrierConfig.ImsVt.KEY_DYNAMIC_VIDEO_QUALITY_SUPPORTED_BOOL)))
                .thenReturn(false);
        when(mMockCallSession.getSessionModificationType())
                .thenReturn(IVideoCallSession.MODIFICATION_CALL_TYPE);
        VideoProfile videoProfile = new VideoProfile(VideoProfile.STATE_BIDIRECTIONAL);

        getProviderImpl().receiveSessionModifyRequest(videoProfile);

        verify(mMockCallback, never()).changePeerDimensions(any(int.class), any(int.class));
        verify(mMockCallSession, never()).getProposedStreamMediaProfile();
        verify(mMockCallback).receiveSessionModifyRequest(videoProfile);
    }

    @SmallTest
    @Test
    public void testReceiveSessionModifyResponse_dynamicQualitySupported() throws RemoteException {
        when(mMockCarrierConfig.getBoolean(
                eq(CarrierConfig.ImsVt.KEY_DYNAMIC_VIDEO_QUALITY_SUPPORTED_BOOL)))
                .thenReturn(true);

        VideoProfile fromProfile = new VideoProfile(VideoProfile.STATE_AUDIO_ONLY);
        VideoProfile toProfile = new VideoProfile(VideoProfile.STATE_BIDIRECTIONAL);
        when(mMockCallSession.getSessionModificationType())
                .thenReturn(IVideoCallSession.MODIFICATION_CALL_TYPE);
        getProviderImpl().sendSessionModifyRequest(fromProfile, toProfile);

        when(mMockCallSession.getStreamMediaProfile()).thenReturn(mMockMediaProfile);
        when(mMockMediaProfile.getVideoQuality()).thenReturn(
                ImsStreamMediaProfile.VIDEO_QUALITY_QCIF);
        VideoProfile requestedProfile = new VideoProfile(VideoProfile.STATE_AUDIO_ONLY);
        VideoProfile responseProfile = new VideoProfile(VideoProfile.STATE_BIDIRECTIONAL);

        getProviderImpl().receiveSessionModifyResponse(
                Connection.VideoProvider.SESSION_MODIFY_REQUEST_SUCCESS,
                requestedProfile,
                responseProfile);

        verify(mMockCallSession).getStreamMediaProfile();
        verify(mMockCallback).changePeerDimensions(any(int.class), any(int.class));
        verify(mMockCallback).receiveSessionModifyResponse(
                Connection.VideoProvider.SESSION_MODIFY_REQUEST_SUCCESS,
                requestedProfile,
                responseProfile);
        assertThat(mProvider.getCallState()).isEqualTo(
                ImsVideoCallProviderBase.CALL_STATE_ESTABLISHED);
    }

    @SmallTest
    @Test
    public void testReceiveSessionModifyResponse_dynamicQualityNotSupported()
            throws RemoteException {
        when(mMockCarrierConfig.getBoolean(
                eq(CarrierConfig.ImsVt.KEY_DYNAMIC_VIDEO_QUALITY_SUPPORTED_BOOL), anyBoolean()))
                .thenReturn(false);

        VideoProfile fromProfile = new VideoProfile(VideoProfile.STATE_AUDIO_ONLY);
        VideoProfile toProfile = new VideoProfile(VideoProfile.STATE_BIDIRECTIONAL);
        when(mMockCallSession.getSessionModificationType())
                .thenReturn(IVideoCallSession.MODIFICATION_CALL_TYPE);
        getProviderImpl().sendSessionModifyRequest(fromProfile, toProfile);

        VideoProfile requestedProfile = new VideoProfile(VideoProfile.STATE_AUDIO_ONLY);
        VideoProfile responseProfile = new VideoProfile(VideoProfile.STATE_BIDIRECTIONAL);

        getProviderImpl().receiveSessionModifyResponse(
                Connection.VideoProvider.SESSION_MODIFY_REQUEST_SUCCESS,
                requestedProfile,
                responseProfile);

        verify(mMockCallSession, never()).getStreamMediaProfile();
        verify(mMockCallback, never()).changePeerDimensions(any(int.class), any(int.class));
        verify(mMockCallback).receiveSessionModifyResponse(
                Connection.VideoProvider.SESSION_MODIFY_REQUEST_SUCCESS,
                requestedProfile,
                responseProfile);
        assertThat(mProvider.getCallState()).isEqualTo(
                ImsVideoCallProviderBase.CALL_STATE_ESTABLISHED);
    }

    @SmallTest
    @Test
    public void testChangeCallDataUsage() throws RemoteException {
        getProviderImpl().changeCallDataUsage(1024);

        verify(mMockCallback).changeCallDataUsage(1024);
    }

    @SmallTest
    @Test
    public void testSetCamera_validId() {
        String cameraId = "1";
        getProviderImpl().setCamera(cameraId);
        verify(mMockMediaSession).selectCamera(1);
        verify(mMockCallSession).onSetCamera(cameraId);
    }

    @SmallTest
    @Test
    public void testSetCamera_nullSession() {
        getProviderImpl().updateMediaSession(null);
        String cameraId = "1";
        getProviderImpl().setCamera(cameraId);
        verify(mMockMediaSession, never()).selectCamera(1);
        // mCallSession.onSetCamera is still called.
        verify(mMockCallSession, never()).onSetCamera(cameraId);
    }

    @SmallTest
    @Test
    public void testSendSessionModifyRequest_videoUpgrade() {
        VideoProfile fromProfile = new VideoProfile(VideoProfile.STATE_AUDIO_ONLY);
        VideoProfile toProfile = new VideoProfile(VideoProfile.STATE_BIDIRECTIONAL);
        when(mMockCallSession.getSessionModificationType())
                .thenReturn(IVideoCallSession.MODIFICATION_CALL_TYPE);

        getProviderImpl().sendSessionModifyRequest(fromProfile, toProfile);

        verify(mMockCallSession).sendSessionModifyRequest(fromProfile, toProfile);
        assertThat(mProvider.getCallState()).isEqualTo(
                ImsVideoCallProviderBase.CALL_STATE_VIDEO_UPGRADE_REQUESTED);
    }

    @SmallTest
    @Test
    public void testSendSessionModifyResponse() {
        VideoProfile responseProfile = new VideoProfile(VideoProfile.STATE_BIDIRECTIONAL);
        getProviderImpl().sendSessionModifyResponse(responseProfile);

        verify(mMockCallSession).sendSessionModifyResponse(responseProfile);
        assertThat(mProvider.getCallState()).isEqualTo(
                ImsVideoCallProviderBase.CALL_STATE_ESTABLISHED);
    }

    @SmallTest
    @Test
    public void testRequestCameraCapabilities_noCamera() throws Exception {
        // mCameraId is null by default
        getProviderImpl().requestCameraCapabilities();
        verify(mMockCallback, never()).changeCameraCapabilities(any());
    }

    @SmallTest
    @Test
    public void testRequestCameraCapabilities() throws Exception {
        // Mock CameraManager and related classes
        CameraManager mCameraManager = mContext.getSystemService(CameraManager.class);
        CameraCharacteristics mockCharacteristics = mock(CameraCharacteristics.class);
        StreamConfigurationMap mockStreamMap = mock(StreamConfigurationMap.class);

        String cameraId = "0";
        int width = 640;
        int height = 480;
        float maxZoom = 2.0f;

        // Setup CameraManager to return Characteristics
        when(mCameraManager.getCameraCharacteristics(cameraId)).thenReturn(mockCharacteristics);

        // Setup Characteristics to return StreamConfigurationMap and Max Zoom
        when(mockCharacteristics.get(CameraCharacteristics.SCALER_STREAM_CONFIGURATION_MAP))
                .thenReturn(mockStreamMap);
        when(mockCharacteristics.get(CameraCharacteristics.SCALER_AVAILABLE_MAX_DIGITAL_ZOOM))
                .thenReturn(maxZoom);
        when(mockCharacteristics.get(CameraCharacteristics.SENSOR_ORIENTATION))
                .thenReturn(90);

        // Setup StreamConfigurationMap to return output sizes
        Size[] sizes = new Size[] { new Size(width, height) };
        when(mockStreamMap.getOutputSizes(SurfaceTexture.class)).thenReturn(sizes);

        // Set Camera ID on the provider
        getProviderImpl().setCamera(cameraId);

        getProviderImpl().requestCameraCapabilities();

        ArgumentCaptor<VideoProfile.CameraCapabilities> captor =
                ArgumentCaptor.forClass(VideoProfile.CameraCapabilities.class);
        verify(mMockCallback).changeCameraCapabilities(captor.capture());

        VideoProfile.CameraCapabilities cc = captor.getValue();
        assertThat(cc.getWidth()).isEqualTo(width);
        assertThat(cc.getHeight()).isEqualTo(height);
        assertThat(cc.getMaxZoom()).isEqualTo(maxZoom);
        assertThat(cc.isZoomSupported()).isTrue();

        width = 480;
        height = 640;

        sizes = new Size[] { new Size(width, height) };
        when(mockStreamMap.getOutputSizes(SurfaceTexture.class)).thenReturn(sizes);
        getProviderImpl().requestCameraCapabilities();

        verify(mMockCallback, times(2)).changeCameraCapabilities(captor.capture());

        cc = captor.getValue();
        assertThat(cc.getWidth()).isEqualTo(height);
        assertThat(cc.getHeight()).isEqualTo(width);
        assertThat(cc.getMaxZoom()).isEqualTo(maxZoom);
        assertThat(cc.isZoomSupported()).isTrue();
    }

    @SmallTest
    @Test
    public void testHandleMediaSessionStarted() throws Exception {
        when(mMockCallSession.getStreamMediaProfile()).thenReturn(mMockMediaProfile);
        when(mMockMediaProfile.getVideoQuality()).thenReturn(
                ImsStreamMediaProfile.VIDEO_QUALITY_QCIF);

        getProviderImpl().handleMediaSessionStarted();

        verify(mMockCallSession).getStreamMediaProfile();
        verify(mMockCallback).changePeerDimensions(any(int.class), any(int.class));
    }

    @SmallTest
    @Test
    public void testHandleMediaSessionMediaInfoChanged() throws RemoteException {
        getProviderImpl().handleMediaSessionMediaInfoChanged(
                MtcCallUtils.INFO_TYPE_MEDIA_VIDEO_NO_DATA, 0, null);
        verify(mMockCallback).handleCallSessionEvent(
                Connection.VideoProvider.SESSION_EVENT_RX_PAUSE);

        getProviderImpl().handleMediaSessionMediaInfoChanged(
                MtcCallUtils.INFO_TYPE_MEDIA_VIDEO_DATA_RECEIVED, 0, null);
        verify(mMockCallback).handleCallSessionEvent(
                Connection.VideoProvider.SESSION_EVENT_RX_RESUME);
    }

    @SmallTest
    @Test
    public void testHandleMediaSessionPeerFirstVideoReceived() throws RemoteException {
        getProviderImpl().handleMediaSessionPeerFirstVideoReceived();
        verify(mMockCallback).handleCallSessionEvent(
                Connection.VideoProvider.SESSION_EVENT_RX_RESUME);
    }

    @SmallTest
    @Test
    public void testHandleMediaSessionPeerDisplayOrientationChanged() throws RemoteException {
        getProviderImpl().handleMediaSessionPeerDisplayOrientationChanged(0);
        verify(mMockCallback).handleCallSessionEvent(
                Connection.VideoProvider.SESSION_EVENT_RX_RESUME);

        getProviderImpl().handleMediaSessionPeerDisplayOrientationChanged(0);
        verify(mMockCallback, times(1)).handleCallSessionEvent(
                Connection.VideoProvider.SESSION_EVENT_RX_RESUME);
    }

    @SmallTest
    @Test
    public void testHandleMediaSessionPeerDimensionsChanged() throws RemoteException {
        getProviderImpl().handleMediaSessionPeerDimensionsChanged(1280, 720);
        verify(mMockCallback).changePeerDimensions(1280, 720);

        getProviderImpl().handleMediaSessionPeerDimensionsChanged(0, 0);
        verify(mMockCallback, times(1)).changePeerDimensions(any(int.class), any(int.class));
    }
}
