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

import android.annotation.IntDef;
import android.annotation.NonNull;
import android.content.Context;
import android.net.Network;
import android.os.Handler;
import android.os.HandlerThread;
import android.os.Looper;
import android.os.Message;
import android.text.TextUtils;

import com.android.imsstack.core.agents.AgentFactory;
import com.android.imsstack.core.agents.WifiInterface;
import com.android.imsstack.core.agents.dcm.DcFactory;
import com.android.imsstack.core.agents.dcmif.EApnType;
import com.android.imsstack.core.agents.dcmif.IDcApn;
import com.android.imsstack.util.ImsLog;

import java.lang.annotation.Retention;
import java.lang.annotation.RetentionPolicy;
import java.net.HttpURLConnection;
import java.net.URL;
import java.util.HashMap;
import java.util.Locale;

/**
 * This class provides to send an HTTP GET request to the server and receives an HTTP response.
 * To receive provisioning data from the AC server by service provider, the client should set
 * required headers or set to send all transactions to a specific network before sending HTTP
 * request.
 * HTTP response include error will be passed through Callback.
 */
public class HttpTransaction {
    /** @hide */
    @IntDef(prefix = {"RESULT_TYPE_"}, value = {
            RESULT_TYPE_INTERNAL_ERROR,
            RESULT_TYPE_HTTP_UNREACHABLE,
    })
    @Retention(RetentionPolicy.SOURCE)
    public @interface ErrorResult {}

    // Internal error occurred during ACS request.
    public static final int RESULT_TYPE_INTERNAL_ERROR = 1;
    // Unreachable error occurred during ACS request.
    public static final int RESULT_TYPE_HTTP_UNREACHABLE = 2;

    /** @hide */
    @IntDef(prefix = {"REQUEST_"}, value = {
            REQUEST_HTTP,
            REQUEST_HTTPS,
            REQUEST_NON_CELLULAR,
            REQUEST_GBA,
            REQUEST_EAP_AKA,
            REQUEST_AUTHENTICATION,
            REQUEST_DONE,
    })
    @Retention(RetentionPolicy.SOURCE)
    public @interface RequestType {}

    // minimum msg
    public static final int MSG_MIN = 0;
    // start msg
    public static final int REQUEST_START = 1;
    // send HTTP request
    public static final int REQUEST_HTTP = 2;
    // send HTTPS request
    public static final int REQUEST_HTTPS = 3;
    // send non-cellular HTTP request using OTP authentication
    public static final int REQUEST_NON_CELLULAR = 4;
    // send GBA request.
    public static final int REQUEST_GBA = 5;
    // send EAP_AKA request.
    public static final int REQUEST_EAP_AKA = 6;
    // send HTTPS request using authentication
    public static final int REQUEST_AUTHENTICATION = 7;
    // ACS is done.
    public static final int REQUEST_DONE = 8;
    // maximum msg
    public static final int MSG_MAX = 9;

    /**
     * Callback interface for receiving result from the AC HTTP request.
     */
    public interface Callback {

        /**
         * Receive HTTP response to AC HTTP Request.
         *
         * @param responseCode the status code in HTTP response
         * @param responseString the reason phrase in HTTP response
         * @param xmlData provisioning data or null
         */
        void receivedHttpResponse(int responseCode, String responseString, byte[] xmlData);

        /**
         * Receive non HTTP response to AC HTTP Request.
         *
         * @param result the result is {@link ErrorResult} for meaning of values.
         */
        void receivedNonHttpResponse(@ErrorResult int result);
    }

    private interface MessageFunction {
        // return 0 means handled message complete
        int handleMessage(Message msg);
    }

    private final class MessageHandler extends Handler {
        MessageHandler(Looper looper) {
            super(looper);
        }

        @Override
        public void handleMessage(Message msg) {
            int what = msg.what;
            if (MSG_MIN < what && what < MSG_MAX) {
                mMessageFunctionMap.get(what).handleMessage(msg);
            } else {
                ImsLog.e(mSlotId, "unknown message : " + what);
            }
        }
    }

    private final HashMap<Integer, MessageFunction> mMessageFunctionMap =
            new HashMap<Integer, MessageFunction>() {
                {
                    put(REQUEST_START, mMsgFuncRequestStart);
                    put(REQUEST_HTTP, mMsgFuncRequestHttp);
                    put(REQUEST_HTTPS, mMsgFuncRequestHttps);
                    put(REQUEST_NON_CELLULAR, mMsgFuncRequestNonCellular);
                    put(REQUEST_GBA, mMsgFuncRequestAuthentication);
                    put(REQUEST_EAP_AKA, mMsgFuncRequestAuthentication);
                    put(REQUEST_AUTHENTICATION, mMsgFuncRequestAuthentication);
                    put(REQUEST_DONE, mMsgFuncRequestDone);
                }
            };

    private final MessageFunction mMsgFuncRequestStart = new MessageFunction() {
        @Override
        public int handleMessage(Message msg) {

            // TODO : initial request Type
            sendMessage(REQUEST_HTTP);
            return 0;
        }
    };

    private final MessageFunction mMsgFuncRequestAuthentication = new MessageFunction() {
        @Override
        public int handleMessage(Message msg) {
            // TODO: update for GBA and EAP_AKA
            mUrl = mRequestInfo.getHttpsUrl();

            // TODO : change apn type
            String ACSApnType = "mobile_internet";
            if (!getURLConnection(ACSApnType)) {
                ImsLog.e(mSlotId, "open UrlConnection is failed");
                return 0;
            }

            setConnectionProperties();
            setAuthorizationHeader();
            HttpResponse httpResponse = new HttpResponse(mSlotId, mHttpURLConnection);
            if (!connect()) {
                ImsLog.e(mSlotId, "HTTP connection is failed");
                httpResponse.setResponseCode(RESULT_TYPE_HTTP_UNREACHABLE);
            }

            // handle response
            HttpResponseForAuthentication httpResponseForAuthentication =
                    new HttpResponseForAuthentication(mHandler, mSlotId);
            httpResponseForAuthentication.handle(httpResponse);
            return 0;
        }
    };

    private final MessageFunction mMsgFuncRequestHttp = new MessageFunction() {
        @Override
        public int handleMessage(Message msg) {
            mUrl = mRequestInfo.getHttpUrl();
            // TODO : change apn type
            String ACSApnType = "mobile_internet";
            if (!getURLConnection(ACSApnType) || mHttpURLConnection == null) {
                ImsLog.e(mSlotId, "open UrlConnection is failed");
                return 0;
            }

            setConnectionProperties();
            HttpResponse httpResponse = new HttpResponse(mSlotId, mHttpURLConnection);
            if (!connect()) {
                ImsLog.e(mSlotId, "HTTP connection is failed");
                httpResponse.setResponseCode(RESULT_TYPE_HTTP_UNREACHABLE);
            }

            // handle response
            HttpResponseForCellular httpResponseForCellular = new HttpResponseForCellular(mHandler,
                    mSlotId);
            httpResponseForCellular.handle(httpResponse);
            return 0;
        }
    };

    private final MessageFunction mMsgFuncRequestHttps = new MessageFunction() {
        @Override
        public int handleMessage(Message msg) {
            mUrl = mRequestInfo.getHttpsUrl();
            // TODO : change apn type
            String ACSApnType = "mobile_internet";
            if (!getURLConnection(ACSApnType) || mHttpURLConnection == null) {
                ImsLog.e(mSlotId, "open UrlConnection is failed");
                return 0;
            }

            // TODO : consider TLS?
            setConnectionProperties();
            HttpResponse httpResponse = new HttpResponse(mSlotId, mHttpURLConnection);
            if (!connect()) {
                ImsLog.e(mSlotId, "HTTP connection is failed");
                httpResponse.setResponseCode(RESULT_TYPE_HTTP_UNREACHABLE);
            }

            // handle response
            HttpResponseForCellular httpResponseForCellular = new HttpResponseForCellular(mHandler,
                    mSlotId);
            httpResponseForCellular.handle(httpResponse);

            return 0;
        }
    };

    private final MessageFunction mMsgFuncRequestNonCellular = new MessageFunction() {
        @Override
        public int handleMessage(Message msg) {
            mUrl = mRequestInfo.getHttpsUrl();
            // TODO : change apn type
            String ACSApnType = "mobile_internet";
            if (!getURLConnection(ACSApnType) || mHttpURLConnection == null) {
                ImsLog.e(mSlotId, "open UrlConnection is failed");
                return 0;
            }

            // TODO : consider TLS?
            setConnectionProperties();
            HttpResponse httpResponse = new HttpResponse(mSlotId, mHttpURLConnection);
            if (!connect()) {
                ImsLog.e(mSlotId, "HTTP connection is failed");
                httpResponse.setResponseCode(RESULT_TYPE_HTTP_UNREACHABLE);
            }

            // handle response
            HttpResponseForNonCellular httpResponseNonCellular = new HttpResponseForNonCellular(
                    mHandler, mSlotId);
            httpResponseNonCellular.handle(httpResponse);

            return 0;
        }
    };

    private final MessageFunction mMsgFuncRequestDone = new MessageFunction() {
        @Override
        public int handleMessage(Message msg) {
            Callback cb = mCallback;

            // TODO : need to update
            int responseCode = msg.arg1;
            byte[] acData = msg.obj != null ? (byte[]) msg.obj : null;
            // retryAfter value also include callback param.
            int retryAfter = msg.arg2;
            if (100 <= responseCode && responseCode <= 600) {
                // received http response
                cb.receivedHttpResponse(responseCode, null, acData);
            } else {
                // internal error or unreachable error
                cb.receivedNonHttpResponse(responseCode);
            }

            return 0;
        }
    };

    private final Handler mHandler;
    private final int mSlotId;
    private final int mSubId;
    private final Context mContext;
    private final RequestInfo mRequestInfo;

    private Callback mCallback = null;
    private HttpURLConnection mHttpURLConnection = null;
    private URL mUrl = null;

    public HttpTransaction(Context context, int slotId, int subId, RequestInfo requestInfo) {
        mContext = context;
        mSlotId = slotId;
        mSubId = subId;
        mRequestInfo = requestInfo;
        HandlerThread handlerThread = new HandlerThread(HttpTransaction.class.getName());
        handlerThread.start();

        mHandler = new MessageHandler(handlerThread.getLooper());

        init();
    }

    /**
     * Set the {@link Callback} that can be used to listen to result of ACS request
     *
     * @param callback The callback to receive result.
     */
    public void setCallback(@NonNull Callback callback) {
        mCallback = callback;
    }

    /**
     * Remove the {@link Callback} that can be used to listen to result of ACS request
     */
    public void removeCallback() {
        mCallback = null;
    }

    /**
     * Open a transaction to receive provisioning data from the service provider.
     */
    public void open() {
        sendMessage(REQUEST_START);
    }

    /**
     * Close a transaction, since the client no longer needs to send.
     */
    public void close() {
        if (null != mHttpURLConnection) {
            mHttpURLConnection.disconnect();
            mHttpURLConnection = null;
        }
        mHandler.getLooper().quit();
    }

    private void init() {
        ImsLog.d(mSlotId, "");

        close();
    }

    private boolean getURLConnection(String acsApnType) {
        if (null == mUrl) {
            ImsLog.e(mSlotId, "URL is null");
            return false;
        }

        try {
            Network nw = null;
            IDcApn dcApn = DcFactory.getDcAgent(IDcApn.class, mSlotId);

            if (dcApn == null) {
                ImsLog.e("DCApn is null");
                return false;
            }

            nw = dcApn.getNetworkByCapability(getEApnType(acsApnType));

            if (nw == null) {
                ImsLog.e(mSlotId, "Network is null");
                return false;
            }
            mHttpURLConnection = (HttpURLConnection) nw.openConnection(mUrl);

        } catch (Exception e) {
            ImsLog.e(mSlotId, e.getMessage());
            mHttpURLConnection = null;
            return false;
        }

        return true;
    }

    private int getEApnType(String acsApnType) {
        WifiInterface wifi = AgentFactory.getInstance().getAgent(WifiInterface.class);
        boolean wifiConnected = (wifi != null && !wifi.isWifiConnected());

        int netType = -1;
        if (TextUtils.isEmpty(acsApnType)) {
            ImsLog.e(mSlotId, "ApnType is null");
            return netType;
        }
        if (acsApnType.contains("wifi") && wifiConnected) {
            netType = EApnType.WIFI.getType();
        } else if (acsApnType.contains("mobile_internet")) {
            netType = EApnType.INTERNET.getType();
        } else if (acsApnType.contains("mobile_ims")) {
            netType = EApnType.IMS.getType();
        }
        /* TODO : update vzw app apn
        else if (AcsApnType.contains("mobile_vzwapp")) {
            nNetworkType = EApnType.APP.getType();
        }
         */
        ImsLog.d(mSlotId, "Apn " + acsApnType + ", WiFi " + wifiConnected + ", NetType " + netType);
        return netType;
    }

    private boolean connect() {
        boolean result = true;

        int ConnectionTimeoutMillis = 5 * 1000;
        int TransactionTimeoutMillis = 60 * 1000;

        mHttpURLConnection.setConnectTimeout(ConnectionTimeoutMillis);
        mHttpURLConnection.setReadTimeout(TransactionTimeoutMillis);

        try {
            mHttpURLConnection.connect();
        } catch (Exception e) {
            ImsLog.e(mSlotId, e.getMessage());
            result = false;
        }
        return result;
    }

    private boolean setConnectionProperties() {
        try {
            // TODO : consider requestType
            mHttpURLConnection.setRequestMethod("GET");
            setUserAgent();
            setLanguageProperty();
            setAcceptHeader();
        } catch (Exception e) {
            ImsLog.e(mSlotId, e.getMessage());
            return false;
        }
        return true;
    }

    private void setUserAgent() {
        if (mRequestInfo == null) {
            ImsLog.e(mSlotId, "RequestInfo is null");
            return;
        }

        mHttpURLConnection.setRequestProperty("User-Agent", mRequestInfo.generateUserAgent());
    }

    private void setLanguageProperty() {
        Locale curLocale = Locale.getDefault();
        String curLanguage = "en";
        if (curLocale != null) {
            curLanguage = curLocale.getLanguage();
        }

        mHttpURLConnection.setRequestProperty("Accept-Language", curLanguage);
    }

    private void setAcceptHeader() {
        mHttpURLConnection.setRequestProperty("Accept", "*/*");
    }

    private void setAuthorizationHeader() {
        String Auth = "";
        mHttpURLConnection.setRequestProperty("Authorization", Auth);
    }

    private void sendMessage(int what) {
        if (mHandler == null) {
            ImsLog.e(mSlotId, "handle is null");
            return;
        }
        Message msg = Message.obtain();
        msg.what = what;
        msg.arg1 = 0;
        msg.arg2 = 0;
        msg.obj = null;

        mHandler.sendMessage(msg);
    }
}
