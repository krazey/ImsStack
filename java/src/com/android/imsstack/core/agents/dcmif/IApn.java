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
    public static final int IPCAN_CATEGORY_MOBILE = 0;
    public static final int IPCAN_CATEGORY_WLAN = 1;

    public static final int HANDOVER_UNKNOWN = 10;
    public static final int HANDOVER_START = 11;
    public static final int HANDOVER_SUCCESS = 12;
    public static final int HANDOVER_FAILURE = 13;

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
         *                      {@link IApn#HANDOVER_UNKNOWN}
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
         * @param failCause The data failcause.
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
     * request apn connection to use target apn.
     * There are many conditions before requesting to use target apn to connectivity manager
     * This condition can be different based on operator requirement and apn type
     */
    boolean connect();

    /**
     * request apn disconnection to stop use of target apn.
     */
    boolean disconnect();

    /**
     * Return Apn name value of target apn
     */
    String getApn();

    /**
     * Return if target apn is connected state or not
     */
    boolean isConnected();

    /**
     * Return data connection state of target apn
     */
    int getDataState();

    /**
     * Return Ipcan category of target apn
     * In default, return value is "IPCAN_CATEGORY_MOBILE"
     */
    int getIpcanCategory();

    /**
     * Return Ip version setting of target apn.
     */
    int getIpVersion();

    /**
     * Return slot id of target apn.
     */
    int getSlotId();

    /**
     * Return Context object that stored in target apn.
     * Reference of context object delivered to child operator apn classes.
     */
    Context getContext();

    /**
     * Send message object to target Apn.
     * Apn extends Handler class,so each apn can handle those message
     */
    boolean sendMessage(Message msg);

    /**
     * Return cached network of APN
     */
    Network getCachedNetwork();
}
