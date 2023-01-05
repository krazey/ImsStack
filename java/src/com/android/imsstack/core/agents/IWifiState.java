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

import android.os.Handler;

/**
 * This provides an interface to access and monitor the Wi-Fi states.
 */
public interface IWifiState extends IAgent {
    /**
     * Register listener to receive event for Wifi state change.
     */
    void registerForWifiStateChanged(Handler h, int what, Object obj);

    /**
     * Un-register listener to receive event for Wifi state change.
     */
    void unregisterForWifiStateChanged(Handler h);

    /**
     * Returns if wifi is connected or not.
     */
    boolean isWifiConnected();

    /**
     * Returns local address of device via WifiManager.
     */
    String getLocalAddress();

    /**
     * Returns detail status of current wifi connection.
     */
    int getWifiDetailedStatus();

    /**
     * Returns Wifi SSID value.
     */
    String getWifiSSID();

    /**
     * Set 'wifi supported' flag value. This change effect to behavior of this object
     */
    void setWifiSupported(boolean input);

    // Methods belows are used like private method, so remove them from this interface class.
    //int getWifiState();
    //String getWifiBSSID();

    // There is no place to use this method, so made it comment.
    // Please remove this code if this method will not be used.
    //int convertNetworkInfoDetailedStateToInt(NetworkInfo.DetailedState state);

}
