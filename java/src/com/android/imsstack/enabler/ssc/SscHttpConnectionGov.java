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
    private static SscHttpConnectionGov sSscHttpConnectionGov = null;

    private HashMap<Integer, SscHttpConnection> mSscHttpConnection = new HashMap<>();

    public static ISscHttpConnectionGov getInstance() {
        if (sSscHttpConnectionGov == null) {
            sSscHttpConnectionGov = new SscHttpConnectionGov();
        }

        return sSscHttpConnectionGov;
    }

    @Override
    public void open(int slotId, EApnType apntype) {
        SscHttpConnection httpConnection = null;
        if (SscConfig.isTls(slotId)) {
            httpConnection = new SscHttpsConnection(slotId, apntype);
        } else {
            httpConnection = new SscHttpConnection(slotId, apntype);
        }

        mSscHttpConnection.put(slotId, httpConnection);
    }

    @Override
    public void close(int slotId) {
        ISscHttpConnection httpConnection = get(slotId);
        if (httpConnection == null) {
            ImsLog.i("close()");
            return;
        }
        httpConnection.close();
    }

    @Override
    public int sendRequest(int slotId, int requestType, String requestUri, String xui,
            String body) {
        ISscHttpConnection httpConnection = get(slotId);
        if (httpConnection == null) {
            ImsLog.i("sendRequest()");
            return -1;
        }
        return httpConnection.sendRequest(requestType, requestUri, xui, body);
    }

    @Override
    public Document getInputStream(int slotId) {
        ISscHttpConnection httpConnection = get(slotId);
        if (httpConnection == null) {
            ImsLog.i("getInputStream()");
            return null;
        }
        return httpConnection.getInputStream();
    }

    private ISscHttpConnection get(int slotId) {
        if (!mSscHttpConnection.containsKey(slotId)) {
            ImsLog.w("");
            return null;
        }

        return mSscHttpConnection.get(slotId);
    }
}
