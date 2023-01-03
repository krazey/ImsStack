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
import android.os.Message;

import com.android.imsstack.util.ImsLog;
import com.android.internal.util.ArrayUtils;

import java.util.ArrayDeque;

/**
 * This class handles the http response for common ACS flow.
 * Send the ACS result via message to HttpTransaction.
 */
public class HttpResponseHandler {
    private final Handler mHandler;
    private final int mSlotId;

    private ArrayDeque mCookieList = null;

    HttpResponseHandler(Handler handler, int slotId) {
        mHandler = handler;
        mSlotId = slotId;
    }

    /**
     * return cookie header values
     *
     * @return cookie list or null
     */
    public ArrayDeque getCookies() {
        return mCookieList;
    }

    protected boolean checkCookies(HttpResponse httpResponse) {
        ArrayDeque cookies = httpResponse.getCookies();
        if (ArrayUtils.isEmpty(cookies)) {
            ImsLog.d(mSlotId, "no cookie");
            return false;
        }

        if (!ArrayUtils.isEmpty(mCookieList) && mCookieList.containsAll(cookies)) {
            ImsLog.d(mSlotId, "already received cookies");
            return false;
        }

        httpResponse.setCookies(cookies);
        mCookieList = cookies;
        return true;
    }

    protected void sendNextProgressMsg(int what, int arg1, int arg2, Object obj, int delayMillis) {
        Message msg = Message.obtain();
        msg.what = what;
        msg.arg1 = arg1;
        msg.arg2 = arg2;
        if (obj != null) {
            msg.obj = obj;
        }

        mHandler.sendMessageDelayed(msg, delayMillis);
    }

    protected void sendAcsResultMsg(int result, byte[] acData) {
        // arg1 == 200 or xxx ACS result, obj == acData or null
        sendNextProgressMsg(HttpTransaction.REQUEST_DONE, result, 0, acData, 0);
    }

    // just send HttpResponse's responseCode
    protected void handleSendResponseWithoutAction(HttpResponse httpResponse) {
        ImsLog.d(mSlotId, "Http " + httpResponse.toString());
        sendAcsResultMsg(httpResponse.getResponseCode(), null);
    }

    protected void handle200Response(HttpResponse httpResponse) {
        ImsLog.d(mSlotId, "Http " + httpResponse.toString());

        // check xml
        byte[] acsBody = httpResponse.getBody();
        if (!ArrayUtils.isEmpty(acsBody)) {
            ImsLog.d(mSlotId, "XML exist");
            sendAcsResultMsg(httpResponse.getResponseCode(), acsBody);
            return;
        }

        // check cookies
        if (checkCookies(httpResponse)) {
            sendNextProgressMsg(HttpTransaction.REQUEST_HTTPS, 0, 0, null, 0);
        } else {
            ImsLog.d(mSlotId, "cookie or ac data is not exist");
            sendAcsResultMsg(HttpTransaction.RESULT_TYPE_INTERNAL_ERROR, null);
        }
    }

    protected void handle503Response(HttpResponse httpResponse) {
        ImsLog.d(mSlotId, "Http " + httpResponse.toString());
        sendNextProgressMsg(HttpTransaction.REQUEST_DONE, httpResponse.getResponseCode(),
                httpResponse.getRetryAfter(), null, 0);
    }

    protected void handle511response(HttpResponse httpResponse) {
        ImsLog.d(mSlotId, "Http " + httpResponse.toString());
        // set cookie if exist
        checkCookies(httpResponse);
        sendNextProgressMsg(HttpTransaction.REQUEST_NON_CELLULAR, 0, 0, null, 0);
    }
}
