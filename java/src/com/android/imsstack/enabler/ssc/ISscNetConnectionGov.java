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
 * Provides the interface to manage SscNetConnection for each slot.
 */
public interface ISscNetConnectionGov {

    /**
     * Create a new SscNetConnection object and initialize with APN type for a given slotId
     *
     * @param apnType The type of network used for APN connection
     */
    void init(int slotId, EApnType apnType);

    /**
     * Call cleanup() of SscNetConnection for a given slotId
     */
    void cleanup(int slotId);

    /**
     *  Call isConnected() of SscNetConnection for a given slotId
     */
    boolean isConnected(int slotId);

    /**
     * Call connect() of SscNetConnection for a given slotId
     *
     * @param timeoutMs The timer to wait for connection.
     */
    boolean connect(int slotId, long timeoutMs);

    /**
     * Call disconnect() of SscNetConnection for a given slotId
     */
    void disconnect(int slotId);

    /**
     * Call getNetworkType() of SscNetConnection for a given slotId
     */
    int getNetworkType(int slotId);

    /**
     * Call setCallbackHandler() of SscNetConnection for a given slotId
     */
    void setCallbackHandler(int slotId, Handler handler);

    /**
     * Call refreshConnectionTimer() of SscNetConnection for a given slotId
     */
    void refreshConnectionTimer(int slotId);
}
