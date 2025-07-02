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

package com.android.imsstack.core.agents.dcmif;

import android.content.Context;
import android.net.Network;
import android.os.Message;

public interface IApn {
    int IPCAN_CATEGORY_MOBILE = 0;
    int IPCAN_CATEGORY_WLAN = 1;

    int HANDOVER_START = 11;
    int HANDOVER_SUCCESS = 12;
    int HANDOVER_FAILURE = 13;

    /**
     * Listener interface to receive the change of data network status.
     */
    interface Listener {
        /**
         * Invoked when the IPCAN(IP Connectivity Access Network) category is changed.
         *
         * @param apnType The APN type.
         *                {@link EApnType#IMS},
         *                {@link EApnType#INTERNET},
         *                {@link EApnType#EMERGENCY}
         * @param ipcanCategory The IPCAN type.
         *                      {@link IApn#IPCAN_CATEGORY_MOBILE},
         *                      {@link IApn#IPCAN_CATEGORY_WLAN}
         */
        default void onIpcanCategoryChanged(int apnType, int ipcanCategory) {
        }

        /**
         * Invoked when the state of handover between WWAN and WLAN is changed.
         *
         * @param handoverState The state of handover
         *                      {@link IApn#HANDOVER_START}
         *                      {@link IApn#HANDOVER_SUCCESS}
         *                      {@link IApn#HANDOVER_FAILURE}
         * @param networkType The network type
         * @param failCause The data connection failure causes code
         */
        default void onHandoverStateChanged(int handoverState, int networkType, int failCause) {
        }

        /**
         * Invoked when the connection status through Cross SIM is changed.
         *
         * @param connectedOverCrossSim {@code true} if connected over CrossSim,
         *                              {@code false} otherwise.
         */
        default void onCrossSimStatusChanged(boolean connectedOverCrossSim) {
        }

        /**
         * Notifies the state of PreciseDataConnectionState by APN type.
         *
         * @param apnType The APN type.
         *                {@link EApnType#IMS},
         *                {@link EApnType#INTERNET},
         *                {@link EApnType#EMERGENCY}
         * @param state The data connection state.
         * @param failCause The data connection failure causes code.
         *                  {@link android.telephony.DataFailCause}
         * @param networkType The network type
         */
        default void onPreciseDataConnectionStateChanged(int apnType, int state, int failCause,
                int networkType) {
        }
    }

    /**
     * Adds a listener to monitor the change of data network status.
     */
    void addListener(Listener listener);

    /**
     * Removes the listener that was previously set.
     */
    void removeListener(Listener listener);

    /**
     * Cleans up the Apn object.
     */
    void cleanup();

    /**
     * Requests a network that satisfies a set of network capabilities.
     */
    boolean connect();

    /**
     * Requests to release a network that was requested through {@link #connect}.
     */
    boolean disconnect();

    /**
     * Returns the name of the APN.
     */
    String getApn();

    /**
     * Returns whether the network of the APN is in a connected state.
     */
    boolean isConnected();

    /**
     * Returns data connection state of the APN.
     */
    int getDataState();

    /**
     * Returns current IPCAN(IP Connectivity Access Network) category.
     * Default value is {@link #IPCAN_CATEGORY_MOBILE}
     */
    int getIpcanCategory();

    /**
     * Returns the IP version setting for the APN.
     */
    int getIpVersion();

    /**
     * Returns the slot id of the APN.
     */
    int getSlotId();

    /**
     * Returns Context object that stored in the APN.
     */
    Context getContext();

    /**
     * Sends message object to the APN.
     * Each APN extends the Handler class to process its messages.
     */
    boolean sendMessage(Message msg);

    /**
     * Returns cached network of APN.
     */
    Network getCachedNetwork();
}
