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

import com.android.imsstack.core.agents.dcmif.EApnType;

import org.w3c.dom.Document;

/**
 * Provides the interface to manage SscHttpConnection for each slot.
 */
public interface ISscHttpConnectionGov {

    /**
     * Create a new SscHttpConnection or SscHttpsConnection object according to
     * CarrierConfigManager.ImsSs.KEY_UT_TRANSPORT_TYPE_INT, and initialize with APN type
     * for a given slotId
     *
     * @param apntype The network type that is used for socket connection
     */
    void open(int slotId, EApnType apntype);

    /**
     * Call close() of SscHttpConnection for a given slotId
     */
    void close(int slotId);

    /**
     * Call sendRequest() of SscHttpConnection for a given slotId
     */
    int  sendRequest(int slotId, int requestType, String requestUri, String xui, String body);

    /**
     * Call getInputStream() of SscHttpConnection for a given slotId
     */
    Document getInputStream(int slotId);
}
