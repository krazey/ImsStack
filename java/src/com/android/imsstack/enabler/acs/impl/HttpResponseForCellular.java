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

package com.android.imsstack.enabler.acs.impl;

import android.os.Handler;

import com.android.imsstack.util.ImsLog;

/**
 * This class handles the http response for Cellular ACS flow.
 * Send the ACS result via message to HttpTransaction.
 */
public class HttpResponseForCellular extends HttpResponseHandler{
    private final int mSlotId;

    /**
     * Creator to handle http response
     *
     * @param handler Handler of HttpTransaction
     * @param slotId SIM slot ID
     */
    public HttpResponseForCellular(Handler handler, int slotId) {
        super(handler, slotId);
        mSlotId = slotId;
    }

    /**
     * Handling ResponseCode of HttpURLConnection
     *
     * @param httpResponse HttpURLConnection
     */
    public void handle(HttpResponse httpResponse) {
        if (httpResponse == null) {
            ImsLog.e(mSlotId, "HttpResponse is null");
            sendAcsResultMsg(HttpTransaction.RESULT_TYPE_INTERNAL_ERROR, null);
            return;
        }

        int responseCode = httpResponse.getResponseCode();
        ImsLog.d(mSlotId, "http : " + responseCode);

        switch (responseCode) {
            case 200:
            case 201:
            case 202:
                handle200Response(httpResponse);
                break;
            case 401:
                handle401Response(httpResponse);
                break;
            case 503:
                handle503Response(httpResponse);
                break;
            case 511:
                handle511response(httpResponse);
                break;
            default:
                handleSendResponseWithoutAction(httpResponse);
        }
    }

    private void handle401Response(HttpResponse httpResponse) {
        ImsLog.d(mSlotId, "Http " + httpResponse.toString());
        // TODO : handle 401 case
        // 0. this device didn't support authentication ?
        // 1. consider WWW-Authenticate header , -> GBA or EAP-AKA
        // HttpResponse.getHeader("WWW-Authenticate");
        // 2. carrier config wzw : EAP-AKA ...
        sendNextProgressMsg(HttpTransaction.REQUEST_AUTHENTICATION, 0, 0, null, 0);
    }
}
