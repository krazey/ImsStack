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

import android.os.PersistableBundle;
import android.telephony.CarrierConfigManager.ImsVt;
import android.telephony.imsmedia.ImsMediaManager;
import android.telephony.imsmedia.VideoConfig;

import com.android.imsstack.base.AppContext;
import com.android.imsstack.core.agents.AgentFactory;
import com.android.imsstack.core.agents.ConfigInterface;
import com.android.imsstack.core.config.CarrierConfig;
import com.android.imsstack.core.config.CarrierConfig.Assets;
import com.android.imsstack.util.ImsLog;
import com.android.internal.annotations.VisibleForTesting;

import java.util.ArrayList;
import java.util.Arrays;
import java.util.HashMap;
import java.util.concurrent.Executors;

public class VideoConfigSpropGenerator {
    static final int MAX_VIDEO_PAYLOAD_TYPES = 3;
    /* Holds SIM slot-id and corresponding CarrierConfigChangeListener */
    private static final HashMap<Integer, CarrierConfigChangeListener> sConfigListeners =
            new HashMap<>();

    /** VideoConfigSpropGenerator is a pure static class. Default constructor is made private to
     * restrict instantiation.
     */
    private VideoConfigSpropGenerator() {
        throw new AssertionError("Cannot instantiate VideoConfigSpropGenerator");
    }

    /**
     * Registers carrier config change listener with the slot specific instance of ConfigInterface.
     *
     * @param slotId SIM slot id for which carrier config change should be monitored.
     */
    public static void init(int slotId) {
        ImsLog.i(slotId, "VideoConfigSpropGenerator: init");
        if (!hasConfigChangeListener(slotId)) {
            ConfigInterface configInterface = AgentFactory.getInstance()
                    .getAgent(ConfigInterface.class, slotId);
            if (configInterface == null) {
                ImsLog.e(slotId, "VideoConfigSpropGenerator: ConfigInterface is null. "
                        + "Listener for CarrierConfig change not added");
                return;
            }

            CarrierConfigChangeListener configListener = new CarrierConfigChangeListener();
            configInterface.addListener(configListener);
            ImsLog.i(slotId, "VideoConfigSpropGenerator: ConfigInterface listener added");

            sConfigListeners.put(slotId, configListener);
        }
    }

    /**
     * Removes the listener previously registered for the give slot id and stops listening to
     * carrier config changes.
     *
     * @param slotId SIM slot for which carrier config change monitoring should be stopped.
     */
    public static void cleanup(int slotId) {
        ImsLog.i(slotId, "VideoConfigSpropGenerator: cleanup");

        if (hasConfigChangeListener(slotId)) {
            ConfigInterface configInterface = AgentFactory.getInstance()
                    .getAgent(ConfigInterface.class, slotId);
            if (configInterface != null) {
                configInterface.removeListener(sConfigListeners.get(slotId));
                ImsLog.i(slotId, "VideoConfigSpropGenerator: ConfigInterface listener removed");
            }
            sConfigListeners.remove(slotId);
        }
    }

    private static class CarrierConfigChangeListener implements ConfigInterface.Listener {

        @Override
        public void onCarrierConfigChanged(int slotId, int subId) {
            ImsLog.d(slotId, "onCarrierConfigChanged subId:" + subId);
            // Connect to ImsMedia Service via ImsMediaManager.
            MediaCallback callback = new MediaCallback(slotId);
            ImsMediaManager imsMediaManager = new ImsMediaManager(AppContext.getInstance(),
                    Executors.newSingleThreadExecutor(), callback);
            callback.setMediaManager(imsMediaManager);
        }

        static class MediaCallback implements ImsMediaManager.OnConnectedCallback {
            private ImsMediaManager mImsMediaManager;
            private final int mSlotId;

            MediaCallback(int slotId) {
                mSlotId = slotId;
            }

            public void setMediaManager(ImsMediaManager imsMediaManager) {
                mImsMediaManager = imsMediaManager;
            }

            @Override
            public void onConnected() {
                ImsLog.i(mSlotId, "Service connected.");

                if (mImsMediaManager == null) {
                    ImsLog.e(mSlotId, "ImsMediaManager is null");
                    return;
                }

                // 1. Read VideoConfig params from carrierConfig
                ConfigInterface configInterface = AgentFactory.getInstance()
                        .getAgent(ConfigInterface.class, mSlotId);
                if (configInterface == null) {
                    ImsLog.e(mSlotId, "ConfigInterface is null");
                    return;
                }
                CarrierConfig carrierConfig = configInterface.getCarrierConfig();

                VideoConfig[] videoConfigs = readVideoConfigs(carrierConfig);
                if (videoConfigs == null || videoConfigs.length == 0) {
                    ImsLog.e(mSlotId, "No videoConfigs available in carrier config");
                    return;
                }

                //TODO:
                // 2. Call ImsMedia service to generate SPROP for all VideoConfig
                // 3. Update carrier config with sprop params
                // 4. Notify carrier config change

                mImsMediaManager.release();
                mImsMediaManager = null;
            }

            @Override
            public void onDisconnected() {
                ImsLog.d(mSlotId, "Service disconnected.");
            }

            private VideoConfig[] readVideoConfigs(CarrierConfig carrierConfig) {
                PersistableBundle videoPayloadTypes = carrierConfig.getBundle(
                        ImsVt.KEY_VIDEO_CODEC_CAPABILITY_PAYLOAD_TYPES_BUNDLE);
                if (videoPayloadTypes == null || videoPayloadTypes.isEmpty()) {
                    ImsLog.e(mSlotId, "makeVideoConfig: Unable to read video payload types from"
                            + "carrier config.");
                    return null;
                }

                ImsLog.i(mSlotId, "makeVideoConfig.videoPayloadTypes: " + videoPayloadTypes);

                ArrayList<VideoConfig> videoConfigs = new ArrayList<VideoConfig>();

                int[] hevcPayloadTypes = videoPayloadTypes.getIntArray(
                        CarrierConfig.Assets.KEY_HEVC_PAYLOAD_TYPE_INT_ARRAY);
                if (hevcPayloadTypes != null && hevcPayloadTypes.length > 0) {
                    ImsLog.i(mSlotId, "makeVideoConfig.hevcPayloadTypes: "
                            + Arrays.toString(hevcPayloadTypes));

                    PersistableBundle hevcPayloadDescriptions = carrierConfig.getBundle(
                            Assets.KEY_HEVC_PAYLOAD_DESCRIPTION_BUNDLE);
                    ImsLog.i(mSlotId, "makeVideoConfig.hevcPayloadDesc: "
                            + hevcPayloadDescriptions);

                    for (int payloadTypeIdx = 0;
                            (payloadTypeIdx < hevcPayloadTypes.length)
                                 && (payloadTypeIdx <= MAX_VIDEO_PAYLOAD_TYPES);
                            payloadTypeIdx++) {

                        int payloadType = hevcPayloadTypes[payloadTypeIdx];
                        PersistableBundle hevcPayloadDesc = hevcPayloadDescriptions
                                .getPersistableBundle("" + payloadType);

                        // Check if SPROP already exists.
                        String sprop = hevcPayloadDesc.getString(
                                Assets.KEY_HEVC_SPROP_PARAMETER_SETS_STRING, null);
                        if (sprop != null && !sprop.isEmpty()) {
                            // SPROP already exists in carrier config. No need to generate again.
                            continue;
                        }

                        int[] resolution = hevcPayloadDesc.getIntArray(
                                ImsVt.KEY_VIDEO_CODEC_ATTRIBUTE_RESOLUTION_INT_ARRAY);
                        if (resolution == null || resolution.length != 2) {
                            continue;
                        }

                        VideoConfig videoConfig = new VideoConfig.Builder()
                                .setTxPayloadTypeNumber((byte) payloadType)
                                .setCodecType(VideoConfig.VIDEO_CODEC_HEVC)
                                .setCodecProfile(hevcPayloadDesc
                                        .getInt(Assets.KEY_HEVC_PROFILE_INT))
                                .setCodecLevel(hevcPayloadDesc.getInt(Assets.KEY_HEVC_LEVEL_INT))
                                .setResolutionWidth(resolution[0])
                                .setResolutionHeight(resolution[1])
                                .setFramerate(hevcPayloadDesc.getInt(
                                        ImsVt.KEY_VIDEO_CODEC_ATTRIBUTE_FRAME_RATE_INT))
                                .build();

                        videoConfigs.add(videoConfig);
                    }
                }

                int[] h264PayloadTypes = videoPayloadTypes.getIntArray(
                        ImsVt.KEY_H264_PAYLOAD_TYPE_INT_ARRAY);
                if (h264PayloadTypes != null && h264PayloadTypes.length > 0) {
                    PersistableBundle h264PayloadDescriptions = carrierConfig.getBundle(
                            ImsVt.KEY_H264_PAYLOAD_DESCRIPTION_BUNDLE);
                    ImsLog.i(mSlotId, "makeVideoConfig.h264PayloadDesc: "
                            + h264PayloadDescriptions);

                    for (int payloadTypeIdx = 0;
                            (payloadTypeIdx < h264PayloadTypes.length)
                                 && (payloadTypeIdx <= MAX_VIDEO_PAYLOAD_TYPES);
                            payloadTypeIdx++) {
                        int payloadType = h264PayloadTypes[payloadTypeIdx];
                        PersistableBundle h264PayloadDesc = h264PayloadDescriptions
                                .getPersistableBundle("" + payloadType);

                        // Check if SPROP already exists.
                        String sprop = h264PayloadDesc.getString(
                                CarrierConfig.ImsVt.KEY_H264_VIDEO_CODEC_ATTRIBUTE_SPROP_STRING,
                                null);
                        if (sprop != null && !sprop.isEmpty()) {
                            // SPROP already exists in carrier config. No need to generate again.
                            continue;
                        }

                        int[] resolution = h264PayloadDesc.getIntArray(
                                ImsVt.KEY_VIDEO_CODEC_ATTRIBUTE_RESOLUTION_INT_ARRAY);
                        if (resolution == null || resolution.length != 2) {
                            continue;
                        }

                        VideoConfig videoConfig = new VideoConfig.Builder()
                                .setTxPayloadTypeNumber((byte) payloadType)
                                .setCodecType(VideoConfig.VIDEO_CODEC_AVC)
                                .setCodecProfileLevelString(h264PayloadDesc.getString(
                                        CarrierConfig.ImsVt
                                                .KEY_H264_VIDEO_CODEC_ATTRIBUTE_SPROP_STRING))
                                .setResolutionWidth(resolution[0])
                                .setResolutionHeight(resolution[1])
                                .setFramerate(h264PayloadDesc.getInt(
                                        ImsVt.KEY_VIDEO_CODEC_ATTRIBUTE_FRAME_RATE_INT))
                                .build();

                        videoConfigs.add(videoConfig);
                    }
                }

                if (videoConfigs.isEmpty()) {
                    ImsLog.e(mSlotId, "makeVideoConfig: videoConfigs is empty");
                    return null;
                }

                return videoConfigs.toArray(new VideoConfig[videoConfigs.size()]);
            }
        }
    }

    @VisibleForTesting
    protected static boolean hasConfigChangeListener(int slotId) {
        return sConfigListeners.containsKey(slotId);
    }
}
