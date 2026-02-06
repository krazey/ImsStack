/**
 * Copyright (C) 2024 The Android Open Source Project
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

import static org.junit.Assert.assertFalse;
import static org.junit.Assert.assertTrue;
import static org.mockito.ArgumentMatchers.any;
import static org.mockito.Mockito.doAnswer;
import static org.mockito.Mockito.times;
import static org.mockito.Mockito.verify;
import static org.mockito.Mockito.when;

import android.os.PersistableBundle;
import android.telephony.CarrierConfigManager;
import android.telephony.CarrierConfigManager.ImsVt;
import android.telephony.imsmedia.ImsMediaManager;
import android.telephony.imsmedia.VideoConfig;

import com.android.imsstack.ImsStackTest;
import com.android.imsstack.base.TestAppContext;
import com.android.imsstack.core.agents.AgentFactory;
import com.android.imsstack.core.agents.ConfigInterface;
import com.android.imsstack.core.config.CarrierConfig;

import org.junit.After;
import org.junit.Before;
import org.junit.Test;
import org.junit.runner.RunWith;
import org.junit.runners.JUnit4;
import org.mockito.ArgumentCaptor;
import org.mockito.Captor;
import org.mockito.Mock;
import org.mockito.MockitoAnnotations;

@RunWith(JUnit4.class)
public class VideoConfigSpropGeneratorTest extends ImsStackTest {

    private static final int SLOT0 = TestAppContext.SLOT0;
    private static final int SUB0 = 1;
    private static final int HEVC_PAYLOAD_TYPE = 96;
    private static final int H264_PAYLOAD_TYPE = 97;

    @Mock ConfigInterface mMockConfigInterface;
    @Mock CarrierConfig mMockCarrierConfig;
    @Mock ImsMediaManager mMockImsMediaManager;

    @Captor ArgumentCaptor<ConfigInterface.Listener> mConfigListenerCaptor;
    @Captor ArgumentCaptor<VideoConfig[]> mVideoConfigsCaptor;
    @Captor ArgumentCaptor<ImsMediaManager.ImsMediaManagerCallback> mMediaManagerCallbackCaptor;

    private PersistableBundle mCarrierConfigBundle;

    @Before
    public void setUp() throws Exception {
        super.setUp(getClass().getSimpleName()); // ImsStackTest setup
        MockitoAnnotations.initMocks(this);

        AgentFactory.getInstance().setAgent(ConfigInterface.class, mMockConfigInterface, SLOT0);
        when(mMockConfigInterface.getCarrierConfig()).thenReturn(mMockCarrierConfig);
        mCarrierConfigBundle = new PersistableBundle();
        when(mMockCarrierConfig.getConfig()).thenReturn(mCarrierConfigBundle);
    }

    @After
    public void tearDown() throws Exception {
        super.tearDown(); // ImsStackTest teardown
        VideoConfigSpropGenerator.cleanup(SLOT0);
        AgentFactory.getInstance().setAgent(ConfigInterface.class, null, SLOT0);
    }

    @Test
    public void testInitAndCleanup() {
        VideoConfigSpropGenerator.cleanup(SLOT0); // Ensure clean state
        assertFalse(VideoConfigSpropGenerator.hasConfigChangeListener(SLOT0));

        VideoConfigSpropGenerator.init(SLOT0);
        assertTrue(VideoConfigSpropGenerator.hasConfigChangeListener(SLOT0));
        verify(mMockConfigInterface).addListener(any(ConfigInterface.Listener.class));

        // Init again, should not add listener again
        VideoConfigSpropGenerator.init(SLOT0);
        verify(mMockConfigInterface, times(1)).addListener(any(ConfigInterface.Listener.class));

        VideoConfigSpropGenerator.cleanup(SLOT0);
        assertFalse(VideoConfigSpropGenerator.hasConfigChangeListener(SLOT0));
        verify(mMockConfigInterface).removeListener(any(ConfigInterface.Listener.class));

        // Cleanup again, should not crash
        VideoConfigSpropGenerator.cleanup(SLOT0);
        verify(mMockConfigInterface, times(1)).removeListener(any(ConfigInterface.Listener.class));
    }

    @Test
    public void testInit_nullConfigInterface() {
        AgentFactory.getInstance().setAgent(ConfigInterface.class, null, SLOT0);
        VideoConfigSpropGenerator.init(SLOT0);
        assertFalse(VideoConfigSpropGenerator.hasConfigChangeListener(SLOT0));
    }

    private ConfigInterface.Listener captureListener() {
        VideoConfigSpropGenerator.init(SLOT0);
        verify(mMockConfigInterface).addListener(mConfigListenerCaptor.capture());
        return mConfigListenerCaptor.getValue();
    }

    @Test
    public void testOnCarrierConfigChanged_vtNotAvailable() {
        when(mMockCarrierConfig.getBoolean(CarrierConfigManager.KEY_CARRIER_VT_AVAILABLE_BOOL))
                .thenReturn(false);

        ConfigInterface.Listener listener = captureListener();
        listener.onCarrierConfigChanged(SLOT0, SUB0);
        // No interaction with ImsMediaManager expected
    }

    @Test
    public void testOnCarrierConfigChanged_nullPayloadTypes() {
        when(mMockCarrierConfig.getBoolean(CarrierConfigManager.KEY_CARRIER_VT_AVAILABLE_BOOL))
                .thenReturn(true);
        when(mMockCarrierConfig.getBundle(ImsVt.KEY_VIDEO_CODEC_CAPABILITY_PAYLOAD_TYPES_BUNDLE))
                .thenReturn(null);

        ConfigInterface.Listener listener = captureListener();
        listener.onCarrierConfigChanged(SLOT0, SUB0);
        // No interaction with ImsMediaManager expected
    }

    @Test
    public void testOnCarrierConfigChanged_emptyPayloadTypes() {
        when(mMockCarrierConfig.getBoolean(CarrierConfigManager.KEY_CARRIER_VT_AVAILABLE_BOOL))
                .thenReturn(true);
        when(mMockCarrierConfig.getBundle(ImsVt.KEY_VIDEO_CODEC_CAPABILITY_PAYLOAD_TYPES_BUNDLE))
                .thenReturn(new PersistableBundle());

        ConfigInterface.Listener listener = captureListener();
        listener.onCarrierConfigChanged(SLOT0, SUB0);
        // No interaction with ImsMediaManager expected
    }

    private void setupHevcConfig(boolean withSprop) {
        PersistableBundle payloadTypes = new PersistableBundle();
        payloadTypes.putIntArray(
                CarrierConfig.ImsVt.KEY_HEVC_PAYLOAD_TYPE_INT_ARRAY, new int[] {HEVC_PAYLOAD_TYPE});
        when(mMockCarrierConfig.getBundle(ImsVt.KEY_VIDEO_CODEC_CAPABILITY_PAYLOAD_TYPES_BUNDLE))
                .thenReturn(payloadTypes);

        PersistableBundle hevcDescriptions = new PersistableBundle();
        PersistableBundle hevcDesc = new PersistableBundle();
        hevcDesc.putIntArray(
                ImsVt.KEY_VIDEO_CODEC_ATTRIBUTE_RESOLUTION_INT_ARRAY, new int[] {640, 480});
        hevcDesc.putIntArray(
                CarrierConfig.ImsVt.KEY_VIDEO_CODEC_BITRATE_INT_ARRAY, new int[] {512});
        hevcDesc.putInt(CarrierConfig.ImsVt.KEY_HEVC_PROFILE_INT, 1);
        hevcDesc.putInt(CarrierConfig.ImsVt.KEY_HEVC_LEVEL_INT, 1);
        hevcDesc.putInt(ImsVt.KEY_VIDEO_CODEC_ATTRIBUTE_FRAME_RATE_INT, 30);
        if (withSprop) {
            hevcDesc.putString(
                    CarrierConfig.ImsVt.KEY_HEVC_SPROP_PARAMETER_SETS_STRING, "testSprop");
        }
        hevcDescriptions.putPersistableBundle(String.valueOf(HEVC_PAYLOAD_TYPE), hevcDesc);
        when(mMockCarrierConfig.getBundle(CarrierConfig.ImsVt.KEY_HEVC_PAYLOAD_DESCRIPTION_BUNDLE))
                .thenReturn(hevcDescriptions);
    }

    private void setupH264Config(boolean withSprop) {
        PersistableBundle payloadTypes = new PersistableBundle();
        payloadTypes.putIntArray(
                ImsVt.KEY_H264_PAYLOAD_TYPE_INT_ARRAY, new int[] {H264_PAYLOAD_TYPE});
        when(mMockCarrierConfig.getBundle(ImsVt.KEY_VIDEO_CODEC_CAPABILITY_PAYLOAD_TYPES_BUNDLE))
                .thenReturn(payloadTypes);

        PersistableBundle h264Descriptions = new PersistableBundle();
        PersistableBundle h264Desc = new PersistableBundle();
        h264Desc.putIntArray(
                ImsVt.KEY_VIDEO_CODEC_ATTRIBUTE_RESOLUTION_INT_ARRAY, new int[] {640, 480});
        h264Desc.putIntArray(
                CarrierConfig.ImsVt.KEY_VIDEO_CODEC_BITRATE_INT_ARRAY, new int[] {384});
        h264Desc.putString(
                CarrierConfig.ImsVt.KEY_H264_VIDEO_CODEC_ATTRIBUTE_PROFILE_LEVEL_ID_STRING,
                "42e01f");
        h264Desc.putInt(ImsVt.KEY_VIDEO_CODEC_ATTRIBUTE_FRAME_RATE_INT, 30);
        if (withSprop) {
            h264Desc.putString(
                    CarrierConfig.ImsVt.KEY_AVC_SPROP_PARAMETER_SETS_STRING, "testSprop");
        }
        h264Descriptions.putPersistableBundle(String.valueOf(H264_PAYLOAD_TYPE), h264Desc);
        when(mMockCarrierConfig.getBundle(ImsVt.KEY_H264_PAYLOAD_DESCRIPTION_BUNDLE))
                .thenReturn(h264Descriptions);
    }

    @Test
    public void testOnCarrierConfigChanged_configWithSprop() {
        when(mMockCarrierConfig.getBoolean(CarrierConfigManager.KEY_CARRIER_VT_AVAILABLE_BOOL))
                .thenReturn(true);
        setupHevcConfig(true);
        setupH264Config(true);

        ConfigInterface.Listener listener = captureListener();
        listener.onCarrierConfigChanged(SLOT0, SUB0);
        // No ImsMediaManager interaction expected as sprop is already present
    }

    @Test
    public void testMediaCallback_onConnected_generateSprop() {
        when(mMockCarrierConfig.getBoolean(CarrierConfigManager.KEY_CARRIER_VT_AVAILABLE_BOOL))
                .thenReturn(true);

        // Combined setup for both codecs
        PersistableBundle payloadTypes = new PersistableBundle();
        payloadTypes.putIntArray(
                CarrierConfig.ImsVt.KEY_HEVC_PAYLOAD_TYPE_INT_ARRAY, new int[] {HEVC_PAYLOAD_TYPE});
        payloadTypes.putIntArray(
                ImsVt.KEY_H264_PAYLOAD_TYPE_INT_ARRAY, new int[] {H264_PAYLOAD_TYPE});
        when(mMockCarrierConfig.getBundle(ImsVt.KEY_VIDEO_CODEC_CAPABILITY_PAYLOAD_TYPES_BUNDLE))
                .thenReturn(payloadTypes);

        PersistableBundle hevcDescriptions = new PersistableBundle();
        PersistableBundle hevcDescBundle = new PersistableBundle();
        hevcDescBundle.putIntArray(
                ImsVt.KEY_VIDEO_CODEC_ATTRIBUTE_RESOLUTION_INT_ARRAY, new int[] {640, 480});
        hevcDescBundle.putIntArray(
                CarrierConfig.ImsVt.KEY_VIDEO_CODEC_BITRATE_INT_ARRAY, new int[] {512});
        hevcDescBundle.putInt(CarrierConfig.ImsVt.KEY_HEVC_PROFILE_INT, 1);
        hevcDescBundle.putInt(CarrierConfig.ImsVt.KEY_HEVC_LEVEL_INT, 1);
        hevcDescBundle.putInt(ImsVt.KEY_VIDEO_CODEC_ATTRIBUTE_FRAME_RATE_INT, 30);
        hevcDescriptions.putPersistableBundle(String.valueOf(HEVC_PAYLOAD_TYPE), hevcDescBundle);
        when(mMockCarrierConfig.getBundle(CarrierConfig.ImsVt.KEY_HEVC_PAYLOAD_DESCRIPTION_BUNDLE))
                .thenReturn(hevcDescriptions);

        PersistableBundle h264Descriptions = new PersistableBundle();
        PersistableBundle h264DescBundle = new PersistableBundle();
        h264DescBundle.putIntArray(
                ImsVt.KEY_VIDEO_CODEC_ATTRIBUTE_RESOLUTION_INT_ARRAY, new int[] {640, 480});
        h264DescBundle.putIntArray(
                CarrierConfig.ImsVt.KEY_VIDEO_CODEC_BITRATE_INT_ARRAY, new int[] {384});
        h264DescBundle.putString(
                CarrierConfig.ImsVt.KEY_H264_VIDEO_CODEC_ATTRIBUTE_PROFILE_LEVEL_ID_STRING,
                "42e01f");
        h264DescBundle.putInt(ImsVt.KEY_VIDEO_CODEC_ATTRIBUTE_FRAME_RATE_INT, 30);
        h264Descriptions.putPersistableBundle(String.valueOf(H264_PAYLOAD_TYPE), h264DescBundle);
        when(mMockCarrierConfig.getBundle(ImsVt.KEY_H264_PAYLOAD_DESCRIPTION_BUNDLE))
                .thenReturn(h264Descriptions);

        // Setup for addSpropToCarrierConfig to write to
        mCarrierConfigBundle.putPersistableBundle(
                ImsVt.KEY_VIDEO_CODEC_CAPABILITY_PAYLOAD_TYPES_BUNDLE, payloadTypes);
        mCarrierConfigBundle.putPersistableBundle(
                CarrierConfig.ImsVt.KEY_HEVC_PAYLOAD_DESCRIPTION_BUNDLE, hevcDescriptions);
        mCarrierConfigBundle.putPersistableBundle(
                ImsVt.KEY_H264_PAYLOAD_DESCRIPTION_BUNDLE, h264Descriptions);

        VideoConfigSpropGenerator.MediaCallback mediaCallback =
                new VideoConfigSpropGenerator.MediaCallback(SLOT0);
        mediaCallback.setMediaManager(mMockImsMediaManager);

        doAnswer(
                        invocation -> {
                            ImsMediaManager.ImsMediaManagerCallback callback =
                                    invocation.getArgument(1);
                            callback.onVideoSpropResponse(
                                    new String[] {
                                        HEVC_PAYLOAD_TYPE + ":hevcSprop",
                                        H264_PAYLOAD_TYPE + ":h264Sprop"
                                    });
                            return null;
                        })
                .when(mMockImsMediaManager)
                .generateVideoSprop(any(VideoConfig[].class), any());

        mediaCallback.onConnected();

        verify(mMockImsMediaManager).generateVideoSprop(mVideoConfigsCaptor.capture(), any());
        assertTrue(mVideoConfigsCaptor.getValue().length == 2);

        verify(mMockConfigInterface).notifyCarrierConfigChangedForNative();
        verify(mMockImsMediaManager).release();

        // Verify sprop added to bundle
        PersistableBundle hevcDesc =
                mCarrierConfigBundle
                        .getPersistableBundle(
                                CarrierConfig.ImsVt.KEY_HEVC_PAYLOAD_DESCRIPTION_BUNDLE)
                        .getPersistableBundle(String.valueOf(HEVC_PAYLOAD_TYPE));
        assertTrue(
                "hevcSprop"
                        .equals(
                                hevcDesc.getString(
                                        CarrierConfig.ImsVt.KEY_HEVC_SPROP_PARAMETER_SETS_STRING)));

        PersistableBundle h264Desc =
                mCarrierConfigBundle
                        .getPersistableBundle(ImsVt.KEY_H264_PAYLOAD_DESCRIPTION_BUNDLE)
                        .getPersistableBundle(String.valueOf(H264_PAYLOAD_TYPE));
        assertTrue(
                "h264Sprop"
                        .equals(
                                h264Desc.getString(
                                        CarrierConfig.ImsVt.KEY_AVC_SPROP_PARAMETER_SETS_STRING)));
    }

    @Test
    public void testMediaCallback_onConnected_nullManager() {
        VideoConfigSpropGenerator.MediaCallback mediaCallback =
                new VideoConfigSpropGenerator.MediaCallback(SLOT0);
        mediaCallback.setMediaManager(null);
        mediaCallback.onConnected();
        // Expect no crash
    }

    @Test
    public void testMediaCallback_onDisconnected() {
        VideoConfigSpropGenerator.MediaCallback mediaCallback =
                new VideoConfigSpropGenerator.MediaCallback(SLOT0);
        mediaCallback.onDisconnected();
        // Expect no crash
    }

    @Test
    public void testReadVideoConfigs_missingDetails() {
        when(mMockCarrierConfig.getBoolean(CarrierConfigManager.KEY_CARRIER_VT_AVAILABLE_BOOL))
                .thenReturn(true);

        PersistableBundle payloadTypes = new PersistableBundle();
        payloadTypes.putIntArray(
                CarrierConfig.ImsVt.KEY_HEVC_PAYLOAD_TYPE_INT_ARRAY, new int[] {HEVC_PAYLOAD_TYPE});
        payloadTypes.putIntArray(
                ImsVt.KEY_H264_PAYLOAD_TYPE_INT_ARRAY, new int[] {H264_PAYLOAD_TYPE});
        when(mMockCarrierConfig.getBundle(ImsVt.KEY_VIDEO_CODEC_CAPABILITY_PAYLOAD_TYPES_BUNDLE))
                .thenReturn(payloadTypes);

        // Missing description bundles
        when(mMockCarrierConfig.getBundle(CarrierConfig.ImsVt.KEY_HEVC_PAYLOAD_DESCRIPTION_BUNDLE))
                .thenReturn(null);
        when(mMockCarrierConfig.getBundle(ImsVt.KEY_H264_PAYLOAD_DESCRIPTION_BUNDLE))
                .thenReturn(null);

        ConfigInterface.Listener listener = captureListener();
        listener.onCarrierConfigChanged(SLOT0, SUB0);
        // No ImsMediaManager interaction

        // Missing individual payload description
        PersistableBundle hevcDescriptions = new PersistableBundle();
        when(mMockCarrierConfig.getBundle(CarrierConfig.ImsVt.KEY_HEVC_PAYLOAD_DESCRIPTION_BUNDLE))
                .thenReturn(hevcDescriptions);
        PersistableBundle h264Descriptions = new PersistableBundle();
        when(mMockCarrierConfig.getBundle(ImsVt.KEY_H264_PAYLOAD_DESCRIPTION_BUNDLE))
                .thenReturn(h264Descriptions);
        listener.onCarrierConfigChanged(SLOT0, SUB0);
        // No ImsMediaManager interaction

        // Missing resolution
        PersistableBundle hevcDesc = new PersistableBundle();
        hevcDescriptions.putPersistableBundle(String.valueOf(HEVC_PAYLOAD_TYPE), hevcDesc);
        PersistableBundle h264Desc = new PersistableBundle();
        h264Descriptions.putPersistableBundle(String.valueOf(H264_PAYLOAD_TYPE), h264Desc);
        listener.onCarrierConfigChanged(SLOT0, SUB0);
        // No ImsMediaManager interaction
    }
}
