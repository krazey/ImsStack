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

import android.annotation.NonNull;
import android.text.TextUtils;

import com.android.imsstack.util.ImsLog;
import com.android.internal.util.ArrayUtils;

import java.io.IOException;
import java.io.InputStream;
import java.net.HttpURLConnection;
import java.util.ArrayDeque;
import java.util.Iterator;
import java.util.List;

/**
 * This class handles HTTP responses defined for ACS operations.
 */
public class HttpResponse {
    /**
     * Defines the response to be processed for ACS operation
     */

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
     * Sets the response code instead of receiving an HTTP response
     *
     * @param responseCode HTTP response code
     */
    public void setResponseCode(int responseCode) {
        mResponse = responseCode;
    }

    /**
     * Gets the response code from an HTTP response message.
     *
     * @return the HTTP response code
     */
    public int getResponseCode() {
        mResponse = HttpTransaction.RESULT_TYPE_INTERNAL_ERROR;

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
     * @return the HTTP body or null
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
     * Gets the cookie header value from an HTTP response.
     *
     * @return the cookie header's value or empty list
     */
    public ArrayDeque getCookies() {
        ArrayDeque cookies = new ArrayDeque<String>();
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
    public void setCookies(ArrayDeque cookies) {
        if (cookies != null && cookies.size() > 0) {
            StringBuilder sb = new StringBuilder();

            Iterator iterator = cookies.iterator();
            sb.append(iterator.next());
            while (iterator.hasNext()) {
                sb.append(";" + iterator.next());
            }

            if (!TextUtils.isEmpty(sb)) {
                mHttpURLConnection.setRequestProperty("Cookie", sb.toString());
            }

            ImsLog.d(mSlotId, "Cookie: " + sb);
        } else {
            ImsLog.d(mSlotId, "no Cookies");
        }
    }

    /**
     * Get value from Retry-After header of 503 response.
     *
     * @return retryAfter value for sec
     */
    public int getRetryAfter() {
        int retryAfter = 0;
        String header = getHeader("Retry-After");

        if (header != null) {
            String[] tokens = header.split(";");

            if (tokens.length > 0) {
                try {
                    retryAfter = Integer.parseInt(tokens[0]);
                } catch (NumberFormatException e) {
                    ImsLog.e(mSlotId, e.getMessage());
                }
            }
        }

        ImsLog.d(mSlotId, "retry after " + retryAfter + " header " + header);
        return retryAfter;
    }

    /**
     * Set the header using HTTP message
     * @param headerName header name in HTTP message
     * @param value using value in HTTP message
     */
    public void setHeader(@NonNull String headerName, @NonNull String value) {
        if (mHttpURLConnection == null) {
            ImsLog.d(mSlotId, "mHttpURLConnection is null");
            return;
        }
        mHttpURLConnection.setRequestProperty(headerName, value);
    }

    private String getHeader(@NonNull String headerName) {
        if (mHttpURLConnection == null) {
            ImsLog.d(mSlotId, "mHttpURLConnection is null");
            return null;
        }
        return mHttpURLConnection.getHeaderField(headerName);
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
