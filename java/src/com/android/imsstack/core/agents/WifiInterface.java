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

package com.android.imsstack.core.agents;

import android.net.Network;

import java.io.FileDescriptor;

/**
 * This interface provides the Wi-Fi related information such as Wi-Fi enabled/disabled
 * and Wi-Fi connection states.
 */
public interface WifiInterface extends IAgent {
    /** Wi-Fi setting states */
    /** Indicates that Wi-Fi setting is turned off. */
    int STATE_DISABLED = 0;
    /** Indicates that Wi-Fi setting is turned on. */
    int STATE_ENABLED = 1;

    /** Wi-Fi connection states */
    /** Indicates that Wi-Fi network is not connected. */
    int CONNECTION_STATE_DISCONNECTED = 0;
    /** Indicates that Wi-Fi network is connected. */
    int CONNECTION_STATE_CONNECTED = 1;

    /**
     * Listener interface to receive the change notification of Wi-Fi states.
     */
    public interface Listener {
        /**
         * Notifies the application that Wi-Fi state is changed.
         */
        default void onWifiStateChanged() {
        }

        /**
         * Notifies the application that Wi-Fi connection state is changed.
         */
        default void onWifiConnectionStateChanged() {
        }
    }

    /**
     * Checks whether the Wi-Fi setting is enabled or not.
     */
    boolean isWifiEnabled();

    /**
     * Checks whether the Wi-Fi netweork is connected or not.
     */
    boolean isWifiConnected();

    /**
     * Returns the {@link Network} object for Wi-Fi network.
     */
    Network getNetwork();

    /**
     * Returns the network interface identifier of Wi-Fi network.
     */
    int getIfaceId();

    /**
     * Returns the network interface name of Wi-Fi network.
     */
    String getIfaceName();

    /**
     * Returns the MTU size used by the current Wi-Fi network.
     */
    int getMtu();

    /**
     * Returns a local address of Wi-Fi network.
     *
     * @param ipVersion Specifying the preferred IP version of Wi-Fi netework.
     *                  {@link EIpVersion#IPV4},
     *                  {@link EIpVersion#IPV6},
     *                  {@link EIpVersion#IPV4V6},
     *                  {@link EIpVersion#IPV6V4}
     * @return A string representation of numeric IP address.
     */
    String getLocalAddress(int ipVersion);

    /**
     * Returns the numeric IP addresses from the specified host name.
     *
     * @param ipVersion Specifying the IP version of the resolved address.
     *                  {@link EIpVersion#IPV4},
     *                  {@link EIpVersion#IPV6}
     * @param host A host name to be resolved.
     * @return A string array of resolved numeric IP address.
     */
    String[] getHostByName(int ipVersion, String host);

    /**
     * Returns BSSID(Basic Service Set IDentifier - AP MAC address) of Wi-Fi network.
     */
    String getBssId();

    /**
     * Returns SSID(Service Set IDentifier - name of network) of Wi-Fi network.
     */
    String getSsId();

    /**
     * Adds a listener to monitor the Wi-Fi state change.
     *
     * @param listener The listener to be set.
     */
    void addListener(Listener listener);

    /**
     * Removes the listener that was previously set.
     *
     * @param listener The listener to be removed.
     */
    void removeListener(Listener listener);

    /**
     * Binds the specified socket fd with the Wi-Fi network.
     *
     * @param sockFd A socket fd to be bound.
     * @return 1 if binding socket is successfully done, 0 otherwise.
     */
    int bindSocket(FileDescriptor sockFd);

    /**
     * Requests the Wi-Fi service to monitor the Wi-Fi setting/connection state
     * from the native logic.
     *
     * @param serviceRequested {@code true} if Wi-Fi service is requested, {@code false} otherwise.
     */
    void requestWifiService(boolean serviceRequested);
}
