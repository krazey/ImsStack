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
import com.android.imsstack.util.ImsLog;

import org.w3c.dom.Document;

import java.util.HashMap;

public class SscHttpConnectionGov implements ISscHttpConnectionGov {
    private static final SscHttpConnectionGov sSscHttpConnectionGov = new SscHttpConnectionGov();
    private static final HashMap<Integer, ISscHttpConnection> sSscHttpConnections = new HashMap<>();

    public static ISscHttpConnectionGov getInstance() {
        return sSscHttpConnectionGov;
    }

    @Override
    public void open(int slotId, EApnType apnType) {
        final ISscHttpConnection httpConnection;

        if (SscConfig.isTls(slotId)) {
            httpConnection = new SscHttpsConnection(slotId, apnType);
        } else {
            httpConnection = new SscHttpConnection(slotId, apnType);
        }

        setHttpConnectionForSlot(slotId, httpConnection);
    }

    @Override
    public void close(int slotId) {
        ISscHttpConnection httpConnection = sSscHttpConnections.get(slotId);
        if (httpConnection == null) {
            ImsLog.e(slotId, "close()");
            return;
        }

        httpConnection.close();
        sSscHttpConnections.remove(slotId);
    }

    @Override
    public int sendRequest(int slotId, int requestType, String requestUri, String xui,
            String body) {
        ISscHttpConnection httpConnection = sSscHttpConnections.get(slotId);
        if (httpConnection == null) {
            ImsLog.e(slotId, "sendRequest()");
            return ISscHttpConnection.HTTP_REQUEST_FAILED_UNSPECIFIED;
        }

        return httpConnection.sendRequest(requestType, requestUri, xui, body);
    }

    @Override
    public Document getInputStream(int slotId) {
        ISscHttpConnection httpConnection = sSscHttpConnections.get(slotId);
        if (httpConnection == null) {
            ImsLog.e(slotId, "getInputStream()");
            return null;
        }

        return httpConnection.getInputStream();
    }

    private void setHttpConnectionForSlot(int slotId, ISscHttpConnection sscHttpConnection) {
        ISscHttpConnection httpConnection = sSscHttpConnections.get(slotId);
        if (httpConnection != null) {
            httpConnection.close();
        }

        sSscHttpConnections.put(slotId, sscHttpConnection);
    }
}
