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
package com.android.imsstack.enabler.aos;

import com.android.imsstack.base.DeviceConfig;
import com.android.imsstack.base.ImsPrivateProperties;
import com.android.imsstack.enabler.aos.service.AosService;
import com.android.imsstack.util.ImsLog;
import com.android.internal.annotations.VisibleForTesting;

import java.util.HashMap;
import java.util.Map;

/**
 * Factory class to instantiate AoS components.
 */
public class AosFactory {

    private static AosFactory sFactory = null;

    @VisibleForTesting
    public final Map<Integer, AosService> mAosServices =
            new HashMap<Integer, AosService>(DeviceConfig.getSupportedSimCount());
    @VisibleForTesting
    final Map<Integer, AosSettingService> mAosSettingServices =
            new HashMap<Integer, AosSettingService>(DeviceConfig.getSupportedSimCount());
    @VisibleForTesting
    final Map<Integer, AosDebug> mAosDebugs =
            new HashMap<Integer, AosDebug>(DeviceConfig.getSupportedSimCount());

    public static AosFactory getInstance() {
        if (sFactory == null) {
            sFactory = new AosFactory();
        }

        return sFactory;
    }

    public synchronized void init(int slotId) {
        ImsLog.d(slotId, "");

        AosService aosService = new AosService();
        aosService.init(slotId);
        mAosServices.put(slotId, aosService);

        AosSettingService aosSettingService = new AosSettingService(slotId);
        aosSettingService.init();
        mAosSettingServices.put(slotId, aosSettingService);

        if (isDebugScreenEnabled(slotId)) {
            AosDebug aosDebug = new AosDebug(slotId);
            aosDebug.init();
            mAosDebugs.put(slotId, aosDebug);
        }
    }

    public synchronized void cleanup(int slotId) {
        ImsLog.d(slotId, "");

        AosDebug aosDebug = mAosDebugs.get(slotId);
        if (aosDebug != null) {
            aosDebug.cleanup();
            mAosDebugs.remove(slotId);
        }

        AosSettingService aosSettingService = mAosSettingServices.get(slotId);
        if (aosSettingService != null) {
            aosSettingService.cleanup();
            mAosSettingServices.remove(slotId);
        }

        AosService aosService = mAosServices.get(slotId);
        if (aosService != null) {
            aosService.cleanup();
            mAosServices.remove(slotId);
        }
    }

    /**
     * Start the AoS service.
     *
     * @param slotId The slot-id to be started.
     */
    public synchronized void start(int slotId) {
        AosService aosService = mAosServices.get(slotId);
        if (aosService != null) {
            aosService.start();
        }
    }

    /**
     * Stop the AoS service.
     *
     * @param slotId The slot-id to be stopped.
     */
    public synchronized void stop(int slotId) {
        AosService aosService = mAosServices.get(slotId);
        if (aosService != null) {
            aosService.stop();
        }
    }

    /**
     * Returns IAosRegistration.
     *
     * @param slotId The slot-id
     * @return Returns the interface of AosService.
     */
    public synchronized IAosRegistration getAosRegistration(int slotId) {
        return mAosServices.get(slotId);
    }

    /**
     * Returns IAosInfo.
     *
     * @param slotId The slot-id
     * @return Returns the interface of AosService.
     */
    public synchronized IAosInfo getAosInfo(int slotId) {
        return mAosServices.get(slotId);
    }

    /**
     * Returns IAosDebug.
     *
     * @param slotId The slot-id
     * @return Returns the interface of AosDebug.
     */
    public synchronized IAosDebug getAosDebug(int slotId) {
        return mAosDebugs.get(slotId);
    }

    /**
     * Checks whether the debug screen is enabled.
     *
     * @param slotId The slot-id
     * @return {@code true} if the debug screen is enabled, {@code false} otherwise.
     */
    private static boolean isDebugScreenEnabled(int slotId) {
        return ImsPrivateProperties.Persistent.getBoolean(
                ImsPrivateProperties.Persistent.KEY_TEST_DEBUG_SCREEN_ENABLED, false, slotId);
    }
}
