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
package com.android.imsstack.core.agents;

import android.content.Context;
import android.content.pm.PackageInfo;
import android.content.pm.PackageManager;
import android.content.pm.PackageManager.NameNotFoundException;
import android.os.Environment;
import android.provider.Settings;
import android.text.TextUtils;

import com.android.imsstack.system.ISystemAPIDevice;
import com.android.imsstack.system.SystemInterface;
import com.android.imsstack.util.ImsLog;

public class DeviceAgent implements IDevice, ISystemAPIDevice {
    // Constants--------------------------------------------------

    // Variables--------------------------------------------------
    private static IDevice sDeviceAgent;
    private Context mContext;

    // Public methods --------------------------------------------
    public DeviceAgent() {
    }

    public static IDevice getInstance() {
        if (sDeviceAgent == null) {
            sDeviceAgent = new DeviceAgent();
        }

        return sDeviceAgent;
    }

    // Interface implementation methods --------------------------
    @Override
    public void init(Context context) {
        ImsLog.d("");
        mContext = context;

        SystemInterface.getInstance().setISystemAPIDevice(this);
    }

    @Override
    public void cleanup() {
        SystemInterface.getInstance().setISystemAPIDevice(null);
    }

    @Override
    public String getApplicationPath(String packageName) {
        PackageManager pm = (mContext != null) ? mContext.getPackageManager() : null;

        if (pm == null) {
            return null;
        }

        String evalPackageName = TextUtils.isEmpty(packageName) ?
                mContext.getPackageName() : packageName;
        String path;

        try {
            PackageInfo packageInfo = pm.getPackageInfo(evalPackageName, 0);
            path = packageInfo.applicationInfo.dataDir;
        } catch (NameNotFoundException e) {
            ImsLog.d("Package not found; name=" + evalPackageName);
            return null;
        }

        ImsLog.d("Application path: " + path);

        return path;
    }

    @Override
    public String getDeviceName() {
        if (mContext == null) {
            return "";
        }

        String strDeviceName = Settings.System.getString(
                mContext.getContentResolver(), "xxx_device_name");

        if (strDeviceName == null) {
            ImsLog.w("device name DB empty");
            strDeviceName = "";
        }

        return strDeviceName;
    }

    @Override
    public String getExternalStoragePath() {
        String state = Environment.getExternalStorageState();

        if (!Environment.MEDIA_MOUNTED.equalsIgnoreCase(state)) {
            return "";
        }

        return Environment.getExternalStorageDirectory().getAbsolutePath();
    }

    @Override
    public String getDeviceName4Sys() {
        return getDeviceName();
    }

    @Override
    public String getExternalStoragePath4Sys() {
        return getExternalStoragePath();
    }
}
