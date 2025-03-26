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

import androidx.annotation.NonNull;
import androidx.annotation.Nullable;

import com.android.imsstack.base.DeviceConfig;
import com.android.imsstack.base.ImsPrivateProperties;
import com.android.imsstack.enabler.aos.service.AosService;
import com.android.imsstack.util.ImsLog;
import com.android.imsstack.util.IndentingPrintWriter;
import com.android.internal.annotations.VisibleForTesting;

import java.util.Map;
import java.util.concurrent.ConcurrentHashMap;

/**
 * Factory class for creating and managing AoS (Always On Service).
 * This class provides a singleton instance for accessing various AoS services
 * such as {@link AosService}, {@link AosSettingService}, {@link AosEmergencyCallbackModeTracker}
 * and {@link AosDebug}.
 */
public class AosFactory {

    private static volatile AosFactory sFactory;

    private final Map<Integer, AosService> mAosServices = new ConcurrentHashMap<>(
            DeviceConfig.getSupportedSimCount());
    private final Map<Integer, AosSettingService> mAosSettingServices = new ConcurrentHashMap<>(
            DeviceConfig.getSupportedSimCount());
    private final Map<Integer, AosEmergencyCallbackModeTracker> mAosEmergencyTracker =
            new ConcurrentHashMap<>(DeviceConfig.getSupportedSimCount());
    private final Map<Integer, AosDebug> mAosDebugs =  new ConcurrentHashMap<>(
            DeviceConfig.getSupportedSimCount());

    private AosFactory() { }

    /**
     * Returns the singleton instance of AosFactory.
     *
     * @return The singleton instance.
     */
    public static AosFactory getInstance() {
        if (sFactory == null) {
            synchronized (AosFactory.class) {
                if (sFactory == null) {
                    sFactory = new AosFactory();
                }
            }
        }
        return sFactory;
    }

    /**
     * Initializes AoS services for the given slot ID.
     *
     * @param slotId The ID of the SIM slot to initialize services for.
     */
    public void init(int slotId) {
        ImsLog.d(slotId, "");

        AosService aosService = new AosService();
        aosService.init(slotId);
        mAosServices.put(slotId, aosService);

        AosSettingService aosSettingService = new AosSettingService(slotId);
        aosSettingService.init();
        mAosSettingServices.put(slotId, aosSettingService);

        AosEmergencyCallbackModeTracker aosEmergencyTracker =
                new AosEmergencyCallbackModeTracker(slotId);
        aosEmergencyTracker.init();
        mAosEmergencyTracker.put(slotId, aosEmergencyTracker);

        if (isDebugScreenEnabled(slotId)) {
            AosDebug aosDebug = new AosDebug(slotId);
            aosDebug.init();
            mAosDebugs.put(slotId, aosDebug);
        }
    }

    /**
     * Cleans up AoS services for the given slot ID.
     *
     * @param slotId The ID of the SIM slot to clean up services for.
     */
    public void cleanup(int slotId) {
        ImsLog.d(slotId, "");

        AosDebug aosDebug = mAosDebugs.remove(slotId);
        if (aosDebug != null) {
            aosDebug.cleanup();
        }

        AosEmergencyCallbackModeTracker aosEmergencyTracker = mAosEmergencyTracker.remove(slotId);
        if (aosEmergencyTracker != null) {
            aosEmergencyTracker.cleanup();
        }

        AosSettingService aosSettingService = mAosSettingServices.remove(slotId);
        if (aosSettingService != null) {
            aosSettingService.cleanup();
        }

        AosService aosService = mAosServices.remove(slotId);
        if (aosService != null) {
            aosService.cleanup();
        }
    }

    /**
     * Starts the AoS service for the specified slot ID.
     *
     * @param slotId The slot ID to start the service for.
     * @throws IllegalStateException If the AosService for the given slot ID is not found.
     */
    public void start(int slotId) {
        AosService aosService = mAosServices.get(slotId);
        if (aosService == null) {
            throw new IllegalStateException("AosService not found for slotId: " + slotId);
        }
        aosService.start();
    }

    /**
     * Stops the AoS service for the specified slot ID.
     *
     * @param slotId The slot ID to stop the service for.
     */
    public void stop(int slotId) {
        AosService aosService = mAosServices.get(slotId);
        if (aosService != null) {
            aosService.stop();
        } else {
            ImsLog.d(slotId, "AosService not found, cannot stop.");
        }
    }

    /**
     * Returns the {@link IAosRegistration} interface for the specified slot ID.
     *
     * @param slotId The slot ID to get the registration interface for.
     * @return The {@link IAosRegistration} interface associated with the slot ID, or
     *         {@code null} if not found.
     */
    @Nullable
    public IAosRegistration getAosRegistration(int slotId) {
        return mAosServices.get(slotId);
    }

    /**
     * Returns the {@link IAosInfo} interface for the specified slot ID.
     *
     * @param slotId The slot ID to get the AoS information for.
     * @return The {@link IAosInfo} interface associated with the slot ID, or
     *         {@code null} if not found.
     */
    @Nullable
    public IAosInfo getAosInfo(int slotId) {
        return mAosServices.get(slotId);
    }

    /**
     * Returns the {@link AosSettingService} instance for the specified slot ID.
     *
     * @param slotId The slot ID to get the setting service for.
     * @return The {@link AosSettingService} instance associated with the slot ID, or
     *         {@code null} if not found.
     */
    @Nullable
    public AosSettingService getAosSettingService(int slotId) {
        return mAosSettingServices.get(slotId);
    }

    /**
     * Returns the {@link IAosDebug} interface for the specified slot ID.
     *
     * @param slotId The slot ID to get the debug interface for.
     * @return The {@link IAosDebug} interface associated with the slot ID, or
     *         {@code null} if not found.
     */
    @Nullable
    public IAosDebug getAosDebug(int slotId) {
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

    /**
     * Replaces or removes the {@link AosService} associated with the given slot ID.
     *
     * @param slotId The slot ID to update.
     * @param service The new {@link AosService} instance, or null to remove the existing service.
     */
    @VisibleForTesting
    public void replaceService(int slotId, AosService service) {
        if (service != null) {
            mAosServices.put(slotId, service);
        } else {
            mAosServices.remove(slotId);
        }
    }

    /**
     * Replaces or removes the {@link AosSettingService} associated with the given slot ID.
     *
     * @param slotId The slot ID to update.
     * @param settingService The new {@link AosSettingService} instance, or null to remove
     *                       the existing service.
     */
    @VisibleForTesting
    public void replaceSettingService(int slotId, AosSettingService settingService) {
        if (settingService != null) {
            mAosSettingServices.put(slotId, settingService);
        } else {
            mAosSettingServices.remove(slotId);
        }
    }

    /**
     * Replaces or removes the {@link AosDebug} associated with the given slot ID.
     *
     * @param slotId The slot ID to update.
     * @param debug The new {@link AosDebug} instance, or null to remove the existing debug.
     */
    @VisibleForTesting
    public void replaceDebug(int slotId, AosDebug debug) {
        if (debug != null) {
            mAosDebugs.put(slotId, debug);
        } else {
            mAosDebugs.remove(slotId);
        }
    }

    /**
     * Dumps this instance into a readable format for dumpsys usage.
     *
     * @param slotId The slot ID
     * @param pw A {@link PrintWriter} object used to write the formatted logs.
     */
    public void dump(int slotId, @NonNull IndentingPrintWriter pw) {
        AosService aosService = mAosServices.get(slotId);
        if (aosService != null) {
            aosService.dump(pw);
        }
    }
}
