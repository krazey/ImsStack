/*
 * Copyright (C) 2023 The Android Open Source Project
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
package com.android.imsstack.core.config;

import android.annotation.NonNull;
import android.content.Context;
import android.os.PersistableBundle;
import android.os.SystemProperties;
import android.telephony.CarrierConfigManager;
import android.util.SparseArray;

import com.android.imsstack.base.AppContext;
import com.android.imsstack.base.SystemServiceProxy.CarrierConfigManagerProxy;
import com.android.imsstack.util.ImsLog;
import com.android.internal.annotations.VisibleForTesting;

/**
 * A class for providing the IMS service capabilities.
 */
public class ServiceCaps {
    /** Carrier configuration keys for service capability. */
    public static final String[] CARRIER_CONFIG_KEYS = {
        CarrierConfigManager.KEY_CARRIER_VOLTE_AVAILABLE_BOOL,
        CarrierConfigManager.KEY_CARRIER_VT_AVAILABLE_BOOL,
        CarrierConfigManager.KEY_CARRIER_WFC_IMS_AVAILABLE_BOOL
    };
    private static final String KEY_VOLTE_AVAIL_OVR = "persist.dbg.volte_avail_ovr";
    private static final String KEY_VT_AVAIL_OVR = "persist.dbg.vt_avail_ovr";
    private static final String KEY_WFC_AVAIL_OVR = "persist.dbg.wfc_avail_ovr";

    /** Service capabilities for each slot. */
    private static SparseArray<ServiceCaps> sServiceCaps = new SparseArray<>();
    private boolean mVoLteEnabled;
    private boolean mVtEnabled;
    private boolean mWfcEnabled;

    /**
     * Checks whether VoLTE is enabled by platform.
     *
     * @return {@code true} if VoLTE is enabled, {@code false} otherwise.
     */
    public boolean isVoLteEnabled() {
        return mVoLteEnabled;
    }

    /**
     * Checks whether VT is enabled by platform.
     *
     * @return {@code true} if VT is enabled, {@code false} otherwise.
     */
    public boolean isVtEnabled() {
        return mVtEnabled;
    }

    /**
     * Checks whether Wi-Fi calling is enabled by platform.
     *
     * @return {@code true} if Wi-Fi calling is enabled, {@code false} otherwise.
     */
    public boolean isWfcEnabled() {
        return mWfcEnabled;
    }

    @Override
    public String toString() {
        StringBuilder sb = new StringBuilder();

        sb.append("[ ServiceCaps :: VoLteEnabled=");
        sb.append(isVoLteEnabled());
        sb.append(", VtEnabled=");
        sb.append(isVtEnabled());
        sb.append(", WfcEnabled=");
        sb.append(isWfcEnabled());
        sb.append(" ]");

        return sb.toString();
    }

    private void setCapabilities(boolean voLteEnabled, boolean vtEnabled, boolean wfcEnabled) {
        mVoLteEnabled = voLteEnabled;
        mVtEnabled = vtEnabled;
        mWfcEnabled = wfcEnabled;
    }

    /**
     * Returns the {@link ServiceCaps} instance for the specified slot.
     *
     * @param slotId The slot id.
     * @return A {@link ServiceCaps} instance.
     *         If it does not exist, it will be newly created and returned.
     */
    @NonNull
    public static ServiceCaps getServiceCaps(int slotId) {
        ServiceCaps sc = sServiceCaps.get(slotId);

        if (sc == null) {
            sc = new ServiceCaps();
            sServiceCaps.put(slotId, sc);
        }

        return sc;
    }

    /**
     * Updates the service capabilities with the specified slot and subscription.
     *
     * @param c The {@link Context} object.
     * @param slotId The slot id to be updated.
     * @param subId The subscription id to be updated.
     */
    public static void updateServiceCapabilities(Context c, int slotId, int subId) {
        CarrierConfigManagerProxy ccmp =
                AppContext.getInstance().getSystemServiceProxy(CarrierConfigManagerProxy.class);
        PersistableBundle b = ccmp.getConfigForSubId(subId, CARRIER_CONFIG_KEYS);
        boolean voLteEnabled = isVoLteEnabledByDevice(c, slotId)
                && b.getBoolean(CarrierConfigManager.KEY_CARRIER_VOLTE_AVAILABLE_BOOL);
        boolean vtEnabled = isVtEnabledByDevice(c, slotId)
                && b.getBoolean(CarrierConfigManager.KEY_CARRIER_VT_AVAILABLE_BOOL);
        boolean wfcEnabled = isWfcEnabledByDevice(c, slotId)
                && b.getBoolean(CarrierConfigManager.KEY_CARRIER_WFC_IMS_AVAILABLE_BOOL);

        setServiceCapabilities(slotId, voLteEnabled, vtEnabled, wfcEnabled);
    }

    /**
     * Sets the service capabilities for testing.
     */
    @VisibleForTesting
    public static void setServiceCapabilities(int slotId, boolean voLteEnabled, boolean vtEnabled,
            boolean wfcEnabled) {
        ServiceCaps sc = getServiceCaps(slotId);
        sc.setCapabilities(voLteEnabled, vtEnabled, wfcEnabled);
        ImsLog.i(slotId, sc.toString());
    }

    /**
     * Clears the service capabilities for testing.
     */
    @VisibleForTesting
    public static void clear() {
        sServiceCaps.clear();
    }

    /**
     * Checks whether VoLTE is enabled by platform.
     * This checks the service capability using the device and carrier configuration.
     *
     * @param slotId The slot id.
     * @return {@code true} if VoLTE is enabled, {@code false} otherwise.
     */
    public static boolean isVoLteEnabledByPlatform(int slotId) {
        return getServiceCaps(slotId).isVoLteEnabled();
    }

    /**
     * Checks whether VT(Video Telephony) is enabled by platform.
     * This checks the service capability using the device and carrier configuration.
     *
     * @param slotId The slot id.
     * @return {@code true} if VT is enabled, {@code false} otherwise.
     */
    public static boolean isVtEnabledByPlatform(int slotId) {
        return getServiceCaps(slotId).isVtEnabled();
    }

    /**
     * Checks whether Wi-Fi calling is enabled by platform.
     * This checks the service capability using the device and carrier configuration.
     *
     * @param slotId The slot id.
     * @return {@code true} if Wi-Fi calling is enabled, {@code false} otherwise.
     */
    public static boolean isWfcEnabledByPlatform(int slotId) {
        return getServiceCaps(slotId).isWfcEnabled();
    }

    /**
     * Checks whether VoLTE is enabled by devic configuration.
     *
     * @param c The {@link Context} object.
     * @param slotId The slot id.
     * @return {@code true} if VoLTE is enabled, {@code false} otherwise.
     */
    public static boolean isVoLteEnabledByDevice(Context c, int slotId) {
        boolean voLteEnabled = c.getResources().getBoolean(
                com.android.internal.R.bool.config_device_volte_available);

        if (!voLteEnabled) {
            return SystemProperties.getInt(KEY_VOLTE_AVAIL_OVR, -1) == 1
                    || SystemProperties.getInt(
                            KEY_VOLTE_AVAIL_OVR + Integer.toString(slotId),
                            -1) == 1;
        }

        return true;
    }

    /**
     * Checks whether VT(Video Telephony) is enabled by devic configuration.
     *
     * @param c The {@link Context} object.
     * @param slotId The slot id.
     * @return {@code true} if VT is enabled, {@code false} otherwise.
     */
    public static boolean isVtEnabledByDevice(Context c, int slotId) {
        boolean vtEnabled = c.getResources().getBoolean(
                com.android.internal.R.bool.config_device_vt_available);

        if (!vtEnabled) {
            return SystemProperties.getInt(KEY_VT_AVAIL_OVR, -1) == 1
                    || SystemProperties.getInt(
                            KEY_VT_AVAIL_OVR + Integer.toString(slotId),
                            -1) == 1;
        }

        return true;
    }

    /**
     * Checks whether Wi-Fi calling is enabled by devic configuration.
     *
     * @param c The {@link Context} object.
     * @param slotId The slot id.
     * @return {@code true} if Wi-Fi calling is enabled, {@code false} otherwise.
     */
    public static boolean isWfcEnabledByDevice(Context c, int slotId) {
        boolean wfcEnabled = c.getResources().getBoolean(
                com.android.internal.R.bool.config_device_wfc_ims_available);

        if (!wfcEnabled) {
            return SystemProperties.getInt(KEY_WFC_AVAIL_OVR, -1) == 1
                    || SystemProperties.getInt(
                            KEY_WFC_AVAIL_OVR + Integer.toString(slotId),
                            -1) == 1;
        }

        return true;
    }
}
