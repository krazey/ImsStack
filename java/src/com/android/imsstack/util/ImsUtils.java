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
package com.android.imsstack.util;

import android.app.admin.DevicePolicyManager;
import android.content.Context;
import android.os.Handler;
import android.os.Process;
import android.os.SystemProperties;

import com.android.ims.ImsManager;

/**
 * This class provides the common utility methods for IMS services.
 */
public final class ImsUtils {
    // getNrUeCapability - NR_UE_CAPABILITY {
    public static final int NR_UE_CAPABILITY_I_NONE = 0x80000000;
    // Bitmask of integer value
    public static final int NR_UE_CAPABILITY_I_SA = 0x00000001;
    public static final int NR_UE_CAPABILITY_I_VONR = 0x00000002;
    // }

    // getUeCapabilityVoNr - 5G_UE_CAPABILITY_VONR {
    public static final int NR_UE_CAPABILITY_EPS_FB = 0;
    public static final int NR_UE_CAPABILITY_VONR = 1;
    // }

    // getNrNetworkMode - 5G_NETWORK_MODE {
    public static final int NR_NETWORK_MODE_SA = 0;
    public static final int NR_NETWORK_MODE_NSA = 1;
    // }

    private static final String KEY_VOLTE_AVAIL_OVR = "persist.dbg.volte_avail_ovr";
    private static final String KEY_VT_AVAIL_OVR = "persist.dbg.vt_avail_ovr";
    private static final String KEY_WFC_AVAIL_OVR = "persist.dbg.wfc_avail_ovr";

    /**
     * CACHE_FOR_SERVICE_CAPS
     * Service capabilities from the previous (SIM's) carrier configuration.
     */
    private static ServiceCaps[] sCacheForServiceCaps = null;
    /**
     * Cache for ImsManager to check any configuration / settings.
     */
    private static ImsManager[] sImsManagers = null;
    /**
     * VoNR: NR_UE_CAPABILITY
     */
    private static int[] sNrUeCapability = null;

    public static class ServiceCaps {
        private boolean mVoLteEnabled;
        private boolean mVtEnabled;
        private boolean mWfcEnabled;

        public ServiceCaps(boolean voLteEnabled, boolean vtEnabled, boolean wfcEnabled) {
            mVoLteEnabled = voLteEnabled;
            mVtEnabled = vtEnabled;
            mWfcEnabled = wfcEnabled;
        }

        public boolean isVoLteEnabled() {
            return mVoLteEnabled;
        }

        public boolean isVtEnabled() {
            return mVtEnabled;
        }

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
    }

    public static synchronized void init() {
        // CACHE_FOR_SERVICE_CAPS
        if (sCacheForServiceCaps == null) {
            sCacheForServiceCaps = new ServiceCaps[MSimUtils.getMaxSimSlot()];

            for (int i = 0; i < sCacheForServiceCaps.length; i++) {
                sCacheForServiceCaps[i] = new ServiceCaps(false, false, false);
            }
        }

        if (sImsManagers == null) {
            sImsManagers = new ImsManager[MSimUtils.getMaxSimSlot()];

            for (int i = 0; i < sImsManagers.length; i++) {
                sImsManagers[i] = ImsManager.getInstance(AppContext.getInstance(), i);
            }
        }

        if (sNrUeCapability == null) {
            sNrUeCapability = new int[MSimUtils.getMaxSimSlot()];

            for (int i = 0; i < sNrUeCapability.length; i++) {
                sNrUeCapability[i] = getDefaultNrUeCapability();
            }
        }
    }

    public static boolean isEmergencyCallEnabledOnServiceRestricted() {
        /*
        if (isDeviceEncryptionModeEnabled()) {
            // If device is in encryption mode, SIM is in LOADED state
            // and IMS emergency call may be initiated.
            // Finally, IMS emergency call can be made based on
            // the emergency call configuration.
            return true;
        }
        */

        // P-OS: VoLte service always runs, VoLte only enabled
        return true;
    }

    public static boolean isEmergencyCallEnabledOnNonVoLteSim() {
        return true;
    }

    public static boolean isDeviceEncryptionModeEnabled() {
        DevicePolicyManager dpm = AppContext.getInstance().getSystemService(
                DevicePolicyManager.class);
        int status = (dpm != null) ? dpm.getStorageEncryptionStatus()
                : DevicePolicyManager.ENCRYPTION_STATUS_INACTIVE;

        if (status == DevicePolicyManager.ENCRYPTION_STATUS_ACTIVE_PER_USER
                || status == DevicePolicyManager.ENCRYPTION_STATUS_ACTIVE) {
            return true;
        }

        return false;
    }

    /** Full-Disk Encryption mode */
    public static boolean isDeviceEncryptionModeEnabledAsFDE() {
        DevicePolicyManager dpm = AppContext.getInstance().getSystemService(
                DevicePolicyManager.class);
        int status = (dpm != null) ? dpm.getStorageEncryptionStatus()
                : DevicePolicyManager.ENCRYPTION_STATUS_INACTIVE;

        return (status == DevicePolicyManager.ENCRYPTION_STATUS_ACTIVE);
    }

    /** APIs for ImsManager - starts */
    public static synchronized ImsManager getImsManager(int phoneId) {
        if (sImsManagers == null) {
            return ImsManager.getInstance(AppContext.getInstance(), phoneId);
        }

        if (phoneId < 0 || phoneId >= sImsManagers.length) {
            return ImsManager.getInstance(AppContext.getInstance(), phoneId);
        }

        if (sImsManagers[phoneId] == null) {
            return ImsManager.getInstance(AppContext.getInstance(), phoneId);
        }

        return sImsManagers[phoneId];
    }

    public static synchronized void updateImsManager() {
        for (int i = 0; i < sImsManagers.length; i++) {
            sImsManagers[i] = ImsManager.getInstance(AppContext.getInstance(), i);
        }
    }

    public static boolean isVoLteEnabledByDevice(Context c, int phoneId) {
        boolean voLteEnabled = c.getResources().getBoolean(
                com.android.internal.R.bool.config_device_volte_available);

        if (!voLteEnabled) {
            return SystemProperties.getInt(KEY_VOLTE_AVAIL_OVR, -1) == 1
                    || SystemProperties.getInt(
                            KEY_VOLTE_AVAIL_OVR + Integer.toString(phoneId),
                            -1) == 1;
        }

        return voLteEnabled;
    }

    public static boolean isVtEnabledByDevice(Context c, int phoneId) {
        boolean vtEnabled = c.getResources().getBoolean(
                com.android.internal.R.bool.config_device_vt_available);

        if (!vtEnabled) {
            return SystemProperties.getInt(KEY_VT_AVAIL_OVR, -1) == 1
                    || SystemProperties.getInt(
                            KEY_VT_AVAIL_OVR + Integer.toString(phoneId),
                            -1) == 1;
        }

        return vtEnabled;
    }

    public static boolean isWfcEnabledByDevice(Context c, int phoneId) {
        boolean wfcEnabled = c.getResources().getBoolean(
                com.android.internal.R.bool.config_device_wfc_ims_available);

        if (!wfcEnabled) {
            return SystemProperties.getInt(KEY_WFC_AVAIL_OVR, -1) == 1
                    || SystemProperties.getInt(
                            KEY_WFC_AVAIL_OVR + Integer.toString(phoneId),
                            -1) == 1;
        }

        return wfcEnabled;
    }

    public static ServiceCaps getServiceCapsByPlatform(Context c, int phoneId) {
        boolean voLteEnabled = false;
        boolean vtEnabled = false;
        boolean wfcEnabled = false;
        ImsManager imsMgr = getImsManager(phoneId);

        if (imsMgr != null) {
            voLteEnabled = imsMgr.isVolteEnabledByPlatform();
            vtEnabled = imsMgr.isVtEnabledByPlatform();
            wfcEnabled = imsMgr.isWfcEnabledByPlatform();
        }

        return new ServiceCaps(voLteEnabled, vtEnabled, wfcEnabled);
    }

    public static boolean isVoLteEnabledByPlatform(Context c, int phoneId) {
        ImsManager imsMgr = getImsManager(phoneId);
        return (imsMgr != null) ? imsMgr.isVolteEnabledByPlatform() : false;
    }

    public static boolean isVtEnabledByPlatform(Context c, int phoneId) {
        ImsManager imsMgr = getImsManager(phoneId);
        return (imsMgr != null) ? imsMgr.isVtEnabledByPlatform() : false;
    }

    public static boolean isWfcEnabledByPlatform(Context c, int phoneId) {
        ImsManager imsMgr = getImsManager(phoneId);
        return (imsMgr != null) ? imsMgr.isWfcEnabledByPlatform() : false;
    }
    /** APIs for ImsManager -- ends */

    // CACHE_FOR_SERVICE_CAPS {
    public static ServiceCaps getServiceCapsFromLocalStorage(int slotId) {
        if (sCacheForServiceCaps == null) {
            return null;
        }

        if (slotId < 0 || slotId >= sCacheForServiceCaps.length) {
            return null;
        }

        return sCacheForServiceCaps[slotId];
    }

    public static void setServiceCapsToLocalStorage(int slotId, ServiceCaps sc) {
        if (sCacheForServiceCaps == null) {
            return;
        }

        if (slotId < 0 || slotId >= sCacheForServiceCaps.length) {
            return;
        }

        if (sc == null) {
            sCacheForServiceCaps[slotId] = new ServiceCaps(false, false, false);
        } else {
            sCacheForServiceCaps[slotId] = sc;
        }
    }
    // CACHE_FOR_SERVICE_CAPS }

    public static int getDefaultNrUeCapability() {
        return NR_UE_CAPABILITY_I_SA;
    }

    public static int getNrUeCapability(int slotId) {
        if (sNrUeCapability == null) {
            return getDefaultNrUeCapability();
        }

        if (slotId < 0 || slotId >= sNrUeCapability.length) {
            return getDefaultNrUeCapability();
        }

        return sNrUeCapability[slotId];
    }

    public static void setNrUeCapability(int slotId, int nrUeCapability) {
        if (sNrUeCapability == null) {
            return;
        }

        if (slotId < 0 || slotId >= sNrUeCapability.length) {
            return;
        }

        sNrUeCapability[slotId] = nrUeCapability;
    }

    public static int getDefaultNrNetworkMode() {
        return NR_NETWORK_MODE_SA;
    }

    public static void setNrNetworkMode(int slotId, int mode) {
        int nrUeCapability = getNrUeCapability(slotId);

        nrUeCapability &= ~(NR_UE_CAPABILITY_I_SA);

        if (mode == NR_NETWORK_MODE_SA) {
            nrUeCapability |= NR_UE_CAPABILITY_I_SA;
        }

        setNrUeCapability(slotId, nrUeCapability);
    }

    public static int getDefaultUeCapabilityVoNr() {
        return NR_UE_CAPABILITY_EPS_FB;
    }

    public static void setUeCapabilityVoNr(int slotId, int capability) {
        int nrUeCapability = getNrUeCapability(slotId);

        nrUeCapability &= ~(NR_UE_CAPABILITY_I_VONR);

        if (capability == NR_UE_CAPABILITY_VONR) {
            nrUeCapability |= NR_UE_CAPABILITY_I_VONR;
        }

        setNrUeCapability(slotId, nrUeCapability);
    }

    public static boolean has(int value, int bitmask) {
        return (value & bitmask) != 0;
    }

    public static void killProcess(long millis) {
        if (millis <= 0) {
            Process.killProcess(Process.myPid());
        } else {
            Handler h = AppContext.getInstance().getMainHandler();

            if (h != null) {
                h.postDelayed(
                    new Runnable() {
                        @Override
                        public void run() {
                            Process.killProcess(Process.myPid());
                        }
                    }, millis);
            } else {
                Process.killProcess(Process.myPid());
            }
        }
    }
}
