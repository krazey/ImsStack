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

import android.annotation.IntDef;

import org.w3c.dom.Document;

import java.lang.annotation.Retention;
import java.lang.annotation.RetentionPolicy;

/**
 * Provide the interface to manage HTTP/HTTPS request and TCP socket with specific network type.
 */
public interface ISscHttpConnection {

    /**
     * HTTP Request types
     */
    int HTTP_REQUEST_GET = 10000;
    int HTTP_REQUEST_PUT = 10001;

    @IntDef(prefix = {"HTTP_REQUEST_"}, value = {
            HTTP_REQUEST_GET,
            HTTP_REQUEST_PUT
    })
    @Retention(RetentionPolicy.SOURCE)
    public @interface HttpRequestType {}

    /**
     * Specific types of internal HTTP request failure other than error codes from a server.
     */
    int HTTP_REQUEST_FAILED_UNSPECIFIED = 0; // error occurs internally but not specified
    int HTTP_REQUEST_FAILED_BY_INVALID_URI = 1;
    int HTTP_REQUEST_FAILED_BY_INVALID_XUI = 2;
    int HTTP_REQUEST_FAILED_BY_INVALID_XCAP_ROOT_URI = 3;
    int HTTP_REQUEST_FAILED_BY_NO_NETWORK = 4;
    int HTTP_REQUEST_FAILED_BY_CONNECTION = 5;
    int HTTP_REQUEST_FAILED_BY_TIMEOUT = 6;
    int HTTP_REQUEST_FAILED_BY_DNS = 7;
    int HTTP_REQUEST_FAILED_MAX = 8;

    @IntDef(prefix = {"HTTP_REQUEST_FAILED_"}, value = {
            HTTP_REQUEST_FAILED_UNSPECIFIED,
            HTTP_REQUEST_FAILED_BY_INVALID_URI,
            HTTP_REQUEST_FAILED_BY_INVALID_XUI,
            HTTP_REQUEST_FAILED_BY_INVALID_XCAP_ROOT_URI,
            HTTP_REQUEST_FAILED_BY_NO_NETWORK,
            HTTP_REQUEST_FAILED_BY_CONNECTION,
            HTTP_REQUEST_FAILED_BY_TIMEOUT,
            HTTP_REQUEST_FAILED_BY_DNS,
            HTTP_REQUEST_FAILED_MAX
    })
    @Retention(RetentionPolicy.SOURCE)
    public @interface HttpRequestFailureReason {}

    /**
     * Disconnect current socket connection
     */
    void close();

    /**
     * Sending HTTP/HTTPS request to requestUri with body.
     *
     * @param requestType The request type, one of {@link HttpRequestType}.
     * @param requestUri The URI used for HTTP request.
     * @param xui The URI getting from P-Associated-Uri of IMS registration.
     * @param body The XML body that is set in HTTP PUT request.
     * @param timeoutMs The timer to wait for completion of the HTTP request.
     *
     * @return The HTTP response code from HTTP server or one of {@link HttpRequestFailureReason}.
     */
    int sendRequest(@HttpRequestType int requestType, String requestUri, String xui, String body,
            int timeoutMs);

    /**
     * Returns XML data received from HTTP server.
     */
    Document getInputStream();
}
