/*
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
package com.android.imsstack.its.base;

import android.telephony.ims.ImsMmTelManager;
import android.telephony.ims.ImsMmTelManager.WiFiCallingMode;

import com.android.imsstack.base.SystemServiceProxy.ImsMmTelManagerProxy;

/**
 * An implementation class to access the {@link ImsMmTelManager}.
 */
public class ImsMmTelManagerProxyImpl implements ImsMmTelManagerProxy {
    private boolean mAdvancedCallingSettingEnabled;
    private boolean mVtSettingEnabled;
    private boolean mVoWiFiSettingEnabled;
    private @WiFiCallingMode int mVoWiFiMode = ImsMmTelManager.WIFI_MODE_UNKNOWN;
    private boolean mVoWiFiRoamingSettingEnabled;
    private @WiFiCallingMode int mVoWiFiRoamingMode = ImsMmTelManager.WIFI_MODE_UNKNOWN;
    private boolean mCrossSimCallingEnabled;

    @Override
    public boolean isAdvancedCallingSettingEnabled() {
        return mAdvancedCallingSettingEnabled;
    }

    @Override
    public boolean isVtSettingEnabled() {
        return mVtSettingEnabled;
    }

    @Override
    public boolean isVoWiFiSettingEnabled() {
        return mVoWiFiSettingEnabled;
    }

    @Override
    public @WiFiCallingMode int getVoWiFiModeSetting() {
        return mVoWiFiMode;
    }

    @Override
    public boolean isVoWiFiRoamingSettingEnabled() {
        return mVoWiFiRoamingSettingEnabled;
    }

    @Override
    public @WiFiCallingMode int getVoWiFiRoamingModeSetting() {
        return mVoWiFiRoamingMode;
    }

    @Override
    public boolean isCrossSimCallingEnabled() {
        return mCrossSimCallingEnabled;
    }

    @Override
    public void setCrossSimCallingEnabled(boolean isEnabled) {
        mCrossSimCallingEnabled = isEnabled;
    }

    /**
     * Sets the user's preference for voice over LTE/NR calling setting.
     *
     * @param isEnabled A flag specifying the setting enabled.
     */
    public void setAdvancedCallingSettingEnabled(boolean isEnabled) {
        mAdvancedCallingSettingEnabled = isEnabled;
    }

    /**
     * Sets the user's preference for video calling setting.
     *
     * @param isEnabled A flag specifying the setting enabled.
     */
    public void setVtSettingEnabled(boolean isEnabled) {
        mVtSettingEnabled = isEnabled;
    }

    /**
     * Sets the user's preference for voice over WiFi calling mode.
     *
     * @param isEnabled A flag specifying the setting enabled.
     */
    public void setVoWiFiSettingEnabled(boolean isEnabled) {
        mVoWiFiSettingEnabled = isEnabled;
    }

    /**
     * Sets the user's voice over WiFi mode setting associated with the device.
     *
     * @param mode The VoWiFi mode.
     */
    public void setVoWiFiMode(@WiFiCallingMode int mode) {
        mVoWiFiMode = mode;
    }

    /**
     * Sets the user's preference for voice over WiFi calling mode in the roaming area.
     *
     * @param isEnabled A flag specifying the setting enabled.
     */
    public void setVoWiFiRoamingSettingEnabled(boolean isEnabled) {
        mVoWiFiRoamingSettingEnabled = isEnabled;
    }

    /**
     * Sets the user's voice over WiFi roaming mode setting associated with the device.
     *
     * @param mode The VoWiFi mode.
     */
    public void setVoWiFiRoamingMode(@WiFiCallingMode int mode) {
        mVoWiFiRoamingMode = mode;
    }
}
