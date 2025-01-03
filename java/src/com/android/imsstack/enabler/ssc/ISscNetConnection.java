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

package com.android.imsstack.enabler.ssc;

import android.os.Handler;

import com.android.imsstack.core.agents.dcmif.EApnType;

/**
 * Provide the interface to manage a connection with apn type
 */
public interface ISscNetConnection {

    /**
     * Initialize SscNetConnection with a given type
     *
     * @param apnType which means network capability of connection
     */
    void init(EApnType apnType);

    /**
     * Remove all handler registered to ohter class related to connection,
     * and disconnect current connection if it's requested
     */
    void cleanup();

    /**
     * Return current connection state
     *
     * @return true if network of apnType is connected, otherwise false
     */
    boolean isConnected();

    /**
     * Request network connnection
     *
     * @param timeoutMs The timer to wait for connection.
     *
     * @return true if connection is requested, otherwise false
     */
    boolean connect(long timeoutMs);

    /**
     * Returns network type of APN used for XCAP operation
     *
     * @return network type. See {@link TelephonyManager#NETWORK_TYPE_XXX}
     */
    int getNetworkType();

    /**
     * Request network disconnection
     */
    void disconnect();

    /**
     * Set handler for connection state event meesages
     *
     * @param handler The handler to send the message to
     */
    void setCallbackHandler(Handler handler);

    /**
     * Extend timer to maintain current connection
     */
    void refreshConnectionTimer();
}
