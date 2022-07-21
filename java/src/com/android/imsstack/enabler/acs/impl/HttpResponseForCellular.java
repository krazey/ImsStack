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

import java.net.HttpURLConnection;
import java.util.HashMap;
import java.util.List;

/**
 * This class handles the http response for Cellular ACS flow.
 * Send the ACS result via message to HttpTransaction.
 */
public class HttpResponseForCellular {
    private interface IHttpResponseHandle {
        void handle(HttpResponse httpResponse);
    }

    private final Handler mHandler;
    private final int mSlotId;
    private final HashMap<HttpResponse.HttpResponseType, IHttpResponseHandle> mHttpResultHandlers =
            new HashMap<HttpResponse.HttpResponseType, IHttpResponseHandle>();
    private final HttpURLConnection mHttpURLConnection;

    private List<String> mCookieList = null;

    /**
     * Creator to handle http response
     *
     * @param handler Handler of HttpTransaction
     * @param slotId SIM slot ID
     * @param httpURLConnection HttpURLConnection for ACS
     */
    public HttpResponseForCellular(Handler handler, int slotId,
            HttpURLConnection httpURLConnection) {
        mHandler = handler;
        mSlotId = slotId;
        mHttpURLConnection = httpURLConnection;

        initHandlerMap();
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

        IHttpResponseHandle httpResponseHandler = mHttpResultHandlers.get(
                HttpResponse.HttpResponseType.getEnum(mSlotId, responseCode));

        if (httpResponseHandler == null) {
            httpResponseHandler = new IHttpResponseHandle() {

                @Override
                public void handle(HttpResponse httpResponse) {
                    ImsLog.d(mSlotId, "Http " + httpResponse.getResponseCode());
                    sendAcsResultMsg(HttpTransaction.RESULT_TYPE_INTERNAL_ERROR, null);
                    return;
                }
            };
        }
        httpResponseHandler.handle(httpResponse);
    }

    /**
     * return cookie header values
     *
     * @return cookie list
     */
    public List<String> getCookies() {
        return mCookieList;
    }

    private void initHandlerMap() {
        /*
        httpResultHandlers.put(HttpResponse.HttpResponseType.CODE_UNDEFINED,
                handle_Undefined);
        httpResultHandlers.put(HttpResponse.HttpResponseType.CODE_INTERNAL_ERROR,
                handle_InternalError);
        httpResultHandlers.put(HttpResponse.HttpResponseType.CODE_UNREACHABLE_ERROR,
                handle_UnreachableError);*/
        mHttpResultHandlers.put(HttpResponse.HttpResponseType.CODE_200_OK, mHandle200OK);
        /*
        httpResultHandlers.put(HttpResponse.HttpResponseType.CODE_401_UNAUTHORIZED, handle_401);
        httpResultHandlers.put(HttpResponse.HttpResponseType.CODE_403_FORBIDDEN, handle_403);
        httpResultHandlers.put(HttpResponse.HttpResponseType.CODE_409_CONFLICT, handle_409);
        httpResultHandlers.put(HttpResponse.HttpResponseType.CODE_500_INTERNAL_SERVER_ERROR,
                handle_500);
        httpResultHandlers.put(HttpResponse.HttpResponseType.CODE_503_RETRY_AFTER, handle_503);
        httpResultHandlers.put(
                HttpResponse.HttpResponseType.CODE_511_NETWORK_AUTHENTICATION_REQUIRED,
                handle_511);*/
    }

    private boolean checkCookies(HttpResponse httpResponse) {
        List<String> cookies = httpResponse.getCookies();
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

    private void sendNextProgressMsg(int what, int arg1, int arg2, Object obj, int delayMillis) {
        Message msg = Message.obtain();
        msg.what = what;
        msg.arg1 = arg1;
        msg.arg2 = arg2;
        if (obj != null) {
            msg.obj = obj;
        }

        mHandler.sendMessageDelayed(msg, delayMillis);
    }

    private void sendAcsResultMsg(int result, byte[] acData) {
        // result == 200, obj == acData or null
        sendNextProgressMsg(HttpTransaction.REQUEST_DONE, result, 0, (byte[]) acData, 0);
    }

    /*
        private IHttpResponseHandle handle_Undefined = new IHttpResponseHandle() {

            @Override
            public void handle(HttpResponse httpResponse) {
                sendACSResultMsg(httpResponse.getResponseCode(), null);
            }
        };

        private IHttpResponseHandle handle_InternalError = new IHttpResponseHandle() {

            @Override
            public void handle(HttpResponse httpResponse) {
                sendACSResultMsg(HttpTransaction.RESULT_TYPE_INTERNAL_ERROR, null);
            }
        };

        private IHttpResponseHandle handle_UnreachableError = new IHttpResponseHandle() {

            @Override
            public void handle(HttpResponse httpResponse) {
                sendACSResultMsg(HttpTransaction.RESULT_TYPE_HTTP_UNREACHABLE, null);
            }
        };
    */
    private final IHttpResponseHandle mHandle200OK = new IHttpResponseHandle() {

        @Override
        public void handle(HttpResponse httpResponse) {
            if (httpResponse == null) {
                ImsLog.d(mSlotId, "HttpResponse is null ");
                return;
            }
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
//                httpResponse.setCookies(httpResponse.getCookies());
                sendNextProgressMsg(HttpTransaction.REQUEST_HTTPS, 0, 0, null, 0);
            } else {
                ImsLog.d(mSlotId, "didn't receive cookie");
                // TODO : send internal fail or 200 ok + no data ?
                sendAcsResultMsg(httpResponse.getResponseCode(), null);
            }

        }
    };
/*
    private IHttpResponseHandle handle_401 = new IHttpResponseHandle() {

        @Override
        public void handle(HttpResponse httpResponse) {
            sendACSResultMsg(httpResponse.getResponseCode(), null);
        }
    };

    private IHttpResponseHandle handle_403 = new IHttpResponseHandle() {

        @Override
        public void handle(HttpResponse httpResponse) {
            sendACSResultMsg(httpResponse.getResponseCode(), null);
        }
    };

    private IHttpResponseHandle handle_409 = new IHttpResponseHandle() {

        @Override
        public void handle(HttpResponse httpResponse) {
            sendACSResultMsg(httpResponse.getResponseCode(), null);
        }
    };

    private IHttpResponseHandle handle_500 = new IHttpResponseHandle() {

        @Override
        public void handle(HttpResponse httpResponse) {
            sendACSResultMsg(httpResponse.getResponseCode(), null);
        }
    };

    private IHttpResponseHandle handle_503 = new IHttpResponseHandle() {

        @Override
        public void handle(HttpResponse httpResponse) {
            sendACSResultMsg(httpResponse.getResponseCode(), null);
        }
    };

    private IHttpResponseHandle handle_511 = new IHttpResponseHandle() {

        @Override
        public void handle(HttpResponse httpResponse) {
            sendACSResultMsg(httpResponse.getResponseCode(), null);
        }
    };*/
}
