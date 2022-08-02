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

import android.text.TextUtils;

import com.android.imsstack.util.ImsLog;
import com.android.internal.util.ArrayUtils;

import java.io.IOException;
import java.io.InputStream;
import java.net.HttpURLConnection;
import java.util.LinkedList;
import java.util.List;

/**
 * This class handles HTTP responses defined for ACS operations.
 */
public class HttpResponse {
    /**
     * Defines the response to be processed for ACS operation
     */
    public enum HttpResponseType {
        CODE_UNDEFINED(0, "UnDefined"),
        CODE_INTERNAL_ERROR(1, "Internal Error"),
        CODE_UNREACHABLE_ERROR(2, "Unreachable Error"),
        CODE_200_OK(200, "OK"),
        CODE_401_UNAUTHORIZED(401, "Unauthorized"),
        CODE_403_FORBIDDEN(403, "Forbidden"),
        CODE_409_CONFLICT(403, "Conflict"),
        CODE_500_INTERNAL_SERVER_ERROR(500, "Internal Server Error"),
        CODE_503_RETRY_AFTER(503, "Retry after"),
        CODE_511_NETWORK_AUTHENTICATION_REQUIRED(511, "Network Authentication Required");

        private int mResponseCode;
        private String mReasonPhrase;

        HttpResponseType(int responseCode, String reasonPhrase) {
            mResponseCode = responseCode;
            mReasonPhrase = reasonPhrase;
        }

        /**
         * Change the defined response code to handle the Http response code for ACS
         *
         * @param slotId SIM slot ID
         * @param responseCode the HTTP response code
         * @return the defined HTTP response code
         */
        public static HttpResponseType getEnum(int slotId, int responseCode) {
            for (HttpResponseType response : HttpResponseType.values()) {
                if (response.getResponseCode() == responseCode) {
                    return response;
                }
            }
            ImsLog.d(slotId, "UnDefined " + responseCode);
            return CODE_UNDEFINED;
        }

        /**
         * Gets the response code from an HTTP response message.
         *
         * @return the HTTP response code
         */
        public int getResponseCode() {
            return mResponseCode;
        }

        /**
         * Gets the reason phrase from an HTTP response message.
         *
         * @return the HTTP reason phrase
         */
        public String getReasonPhrase() {
            return mReasonPhrase;
        }
    }

    private final int mSlotId;
    private final HttpURLConnection mHttpURLConnection;

    private int mResponse;
    private InputStream mInputStream = null;

    /**
     * Creator to handle defined http response behavior
     *
     * @param slotId SIM slot ID
     * @param httpURLConnection HttpURLConnection for ACS
     */
    public HttpResponse(int slotId, HttpURLConnection httpURLConnection) {
        mSlotId = slotId;
        mHttpURLConnection = httpURLConnection;
    }

    /**
     * Gets the response code from an HTTP response message.
     *
     * @return the HTTP response code
     */
    public int getResponseCode() {
        // need to check
        if (mResponse != HttpResponseType.CODE_UNDEFINED.getResponseCode()) {
            return mResponse;
        }

        mResponse = HttpResponseType.CODE_INTERNAL_ERROR.getResponseCode();
        if (mHttpURLConnection != null) {
            try {
                mResponse = mHttpURLConnection.getResponseCode();
            } catch (IOException e) {
                ImsLog.e(mSlotId, e.toString());
            }
        }

        ImsLog.i(mSlotId, "Response " + mResponse);

        return mResponse;
    }

    /**
     * Gets the body from an HTTP response.
     *
     * @return the HTTP body
     */
    public byte[] getBody() {
        if (mHttpURLConnection == null) {
            ImsLog.e(mSlotId, "HttpURLConnection is null");
            return null;
        }

        byte[] body = null;
        try {
            mInputStream = mHttpURLConnection.getInputStream();
            if (hasXML(mInputStream)) {
                body = mInputStream.readAllBytes();
            }
        } catch (IOException e) {
            ImsLog.e(mSlotId, e.toString());
        }
        return body;
    }

    /**
     * Gets the header value from an HTTP response.
     *
     * @param headerName header field
     * @return the value of the header or null
     */
    public String getHeader(String headerName) {
        if (mHttpURLConnection == null) {
            return null;
        }
        return mHttpURLConnection.getHeaderField(headerName);
    }

    /**
     * Gets the cookie header value from an HTTP response.
     *
     * @return the cookie header's value or empty list
     */
    public List<String> getCookies() {
        List<String> cookies = new LinkedList<String>();
        if (mHttpURLConnection == null) {
            ImsLog.e(mSlotId, "HttpURLConnection");
            return cookies;
        }
        List<String> cookieHeaders = mHttpURLConnection.getHeaderFields().get(
                "Set-Cookie");

        if (!ArrayUtils.isEmpty(cookieHeaders)) {
            ImsLog.d(mSlotId, "size " + cookieHeaders.size());

            for (int i = 0; i < cookieHeaders.size(); i++) {
                String[] tokens = cookieHeaders.get(i).split(";");
                if (!TextUtils.isEmpty(tokens[0])) {
                    cookies.add(tokens[0]);
                }
            }
        }
        return cookies;
    }

    /**
     * Sets the cookie header in an HTTP request.
     *
     * @param cookies the cookie header value
     */
    public void setCookies(List<String> cookies) {
        if (cookies != null && cookies.size() > 0) {

            StringBuilder sb = new StringBuilder();
            sb.append(cookies.get(0));
            for (int i = 1; i < cookies.size(); i++) {
                if (!TextUtils.isEmpty(cookies.get(i))) {
                    sb.append(";" + cookies.get(i));
                }
            }

            if (!TextUtils.isEmpty(sb)) {
                mHttpURLConnection.setRequestProperty("Cookie", sb.toString());
            }

            ImsLog.d(mSlotId, "Cookie: " + sb.toString());
        } else {
            ImsLog.d(mSlotId, "no Cookies");
        }
    }

    private boolean hasXML(InputStream inputStream) {
        if (inputStream == null) {
            ImsLog.d(mSlotId, "didn't received any xml");
            return false;
        }
        if (inputStream.markSupported()) {
            inputStream.mark(0);
            try {
                if (inputStream.read() != -1) {
                    inputStream.reset();
                    return true;
                }
            } catch (IOException e) {
                ImsLog.e(mSlotId, e.getMessage());
            }
        }
        return false;
    }
}
