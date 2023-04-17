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
package com.android.imsstack.core;

import android.content.Context;
import android.os.Parcel;

import com.android.imsstack.core.config.ServiceCaps;
import com.android.imsstack.jni.JniImsProxy;
import com.android.imsstack.util.MSimUtils;

/**
 * This class provides an interface to control and manage the native logics and the configurations.
 */
public class NativeCommands {
    /** Command for setting a device configuration. */
    public static final int CMD_SET_DEVICE_CONFIG = 1;
    /** Command for starting the native enablers. */
    public static final int CMD_START_ENABLER = 2;
    /** Command for stopping the native enablers. */
    public static final int CMD_STOP_ENABLER = 3;

    /** Sets the device configuration. */
    public static void setDeviceConfig(Context c) {
        int supportedSimCount = MSimUtils.getSupportedSimCount();
        int activeSimCount = MSimUtils.getActiveSimCount();
        boolean imsEmergencyEnabled = true;
        boolean voLteEnabled = ServiceCaps.isVoLteEnabledByDevice(c, MSimUtils.DEFAULT_PHONE_ID);
        boolean vtEnabled = ServiceCaps.isVtEnabledByDevice(c, MSimUtils.DEFAULT_PHONE_ID);
        boolean wfcEnabled = ServiceCaps.isWfcEnabledByDevice(c, MSimUtils.DEFAULT_PHONE_ID);

        Parcel p = Parcel.obtain();
        p.writeInt(supportedSimCount);
        p.writeInt(activeSimCount);
        p.writeInt(imsEmergencyEnabled ? 1 : 0);
        p.writeInt(voLteEnabled ? 1 : 0);
        p.writeInt(vtEnabled ? 1 : 0);
        p.writeInt(wfcEnabled ? 1 : 0);

        byte[] data = p.marshall();
        p.recycle();
        p = null;

        JniImsProxy.sendCommand(CMD_SET_DEVICE_CONFIG, -1, data);
    }

    /**
     * Starts the native enablers with the specified slot-id.
     *
     * @param slotId The slot-id to be started.
     */
    public static void startEnabler(int slotId) {
        JniImsProxy.sendCommand(CMD_START_ENABLER, slotId, null);
    }

    /**
     * Stops the native enablers with the specified slot-id.
     *
     * @param slotId The slot-id to be stopped.
     */
    public static void stopEnabler(int slotId) {
        JniImsProxy.sendCommand(CMD_STOP_ENABLER, slotId, null);
    }
}
