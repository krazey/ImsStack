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

import android.net.Network;
import android.text.TextUtils;

import com.android.imsstack.core.agents.AgentFactory;
import com.android.imsstack.core.agents.WifiInterface;
import com.android.imsstack.core.agents.dcm.DcFactory;
import com.android.imsstack.core.agents.dcmif.EApnType;
import com.android.imsstack.core.agents.dcmif.IDcApn;
import com.android.imsstack.util.ImsLog;
import com.android.internal.annotations.VisibleForTesting;

import org.w3c.dom.Document;

import java.io.IOException;
import java.io.InputStream;
import java.io.OutputStream;
import java.net.HttpURLConnection;
import java.net.SocketTimeoutException;
import java.net.URL;
import java.net.UnknownHostException;
import java.util.List;
import java.util.Map;
import java.util.Set;

import javax.xml.parsers.DocumentBuilder;
import javax.xml.parsers.DocumentBuilderFactory;

public class SscHttpConnection implements ISscHttpConnection {
    private static final int HTTP_CONNECTION_TIMEOUT = 30 * 1000;
    private static final int HTTP_READ_TIMEOUT = 20 * 1000;

    protected final int mSlotId;
    private final EApnType mApnType;
    protected HttpURLConnection mConnection = null;
    @VisibleForTesting
    protected Document mDoc = null;

    public SscHttpConnection(int slotId, EApnType apnType) {
        ImsLog.d(slotId, "apnType : " + apnType);

        mSlotId = slotId;
        mApnType = apnType;
    }

    @Override
    public void close() {
        ImsLog.w(mSlotId, "HttpConnection - close()");
    }

    @Override
    public int sendRequest(@HttpRequestType int requestType, String requestUri, String xui,
            String body) {
        ImsLog.d(mSlotId, "requestType : " + requestType + ", body : \n" + body);

        if (TextUtils.isEmpty(requestUri)) {
            ImsLog.e("requestUri or xui is invalid");
            return HTTP_REQUEST_FAILED_BY_INVALID_URI;
        }

        if (TextUtils.isEmpty(xui)) {
            ImsLog.e(mSlotId, "requestUri or xui is invalid");
            return HTTP_REQUEST_FAILED_BY_INVALID_XUI;
        }

        URL connectionUrl = getSscUrl().getConnectionUrl(mSlotId, requestUri);
        if (connectionUrl == null) {
            ImsLog.e(mSlotId, "URL is null");
            return HTTP_REQUEST_FAILED_BY_INVALID_XCAP_ROOT_URI;
        }

        int responseCode = HTTP_REQUEST_FAILED_UNSPECIFIED;
        try {
            Network nw = null;
            if (mApnType.getType() == EApnType.WIFI.getType()) {
                WifiInterface wifi = AgentFactory.getInstance().getAgent(WifiInterface.class);
                nw = ((wifi != null) ? wifi.getNetwork() : null);
            } else {
                IDcApn dcApn = DcFactory.getDcAgent(IDcApn.class, mSlotId);
                if (dcApn != null) {
                    nw = dcApn.getNetworkByCapability(mApnType.getType());
                }
            }

            if (nw == null) {
                ImsLog.e(mSlotId, "Network is null; apnType=" + mApnType);
                return HTTP_REQUEST_FAILED_BY_NO_NETWORK;
            }

            mConnection = (HttpURLConnection) nw.openConnection(connectionUrl);
            if (mConnection == null) {
                ImsLog.e(mSlotId, "HTTP URL Connection is null");
                return HTTP_REQUEST_FAILED_BY_CONNECTION;
            }

            setSocketFactory();
            setHostnameVerifier();

            // Sets timer values
            mConnection.setConnectTimeout(HTTP_CONNECTION_TIMEOUT);
            mConnection.setReadTimeout(HTTP_READ_TIMEOUT);

            setAuthorizationHeader(requestType, requestUri, body);

            int bodyLength = 0;
            if (!TextUtils.isEmpty(body)) {
                bodyLength = body.length();
            }

            setExtraHeaders(connectionUrl, xui, bodyLength);

            ISscAuthAgent authAgent = getSscAuthAgent();
            if (requestType == HTTP_REQUEST_GET) {
                mConnection.setRequestMethod("GET");
                mConnection.setRequestProperty("Connection", "Keep-Alive");
                mConnection.setDoInput(true);
                displayHeaders(connectionUrl, true, body);
                mConnection.connect();
            } else { // HTTP_REQUEST_PUT
                mConnection.setRequestMethod("PUT");
                mConnection.setDoOutput(true);
                if (!TextUtils.isEmpty(authAgent.getETag())) {
                    mConnection.setRequestProperty("If-Match", authAgent.getETag());
                }
                displayHeaders(connectionUrl, true, body);
                writeContentAndConnect(body);
            }

            ImsLog.i(mSlotId, "Connect");

            responseCode = mConnection.getResponseCode();
            ImsLog.i(mSlotId, "Response Code : " + responseCode);

            if (!TextUtils.isEmpty(mConnection.getHeaderField("ETag"))) {
                authAgent.setETag(mConnection.getHeaderField("ETag"));
            }

            authAgent.setCipherSuite(getCipherSuiteFromConn());

            String authHeader = mConnection.getHeaderField("WWW-Authenticate");
            if (authHeader != null) { // 401 Case
                authAgent.parse(authHeader);
            } else { // authHeader is null
                authHeader = mConnection.getHeaderField("Authentication-Info");
                if (authHeader != null) { // 2XX case
                    authAgent.parse(authHeader);
                }
            }

            if (mConnection.getContentLength() > 0) {
                ImsLog.w(mSlotId, "getContentLength > 0");
                if ((responseCode >= 200) && (responseCode < 300)) {
                    mDoc = readInputStream(mConnection.getInputStream());
                } else {
                    mDoc = readInputStream(mConnection.getErrorStream());
                }
            } else if (("chunked").equalsIgnoreCase(
                    mConnection.getHeaderField("Transfer-Encoding"))) {
                ImsLog.w(mSlotId, "Transfer Encoding : chunked");
                if ((responseCode >= 200) && (responseCode < 300)) {
                    mDoc = readInputStream(mConnection.getInputStream());
                } else {
                    mDoc = readInputStream(mConnection.getErrorStream());
                }
            } else {
                mDoc = null;
            }

            displayHeaders(connectionUrl, false, "");
        } catch (SocketTimeoutException e) {
            ImsLog.e(mSlotId, e.toString());
            return HTTP_REQUEST_FAILED_BY_TIMEOUT;
        } catch (UnknownHostException e) {
            ImsLog.e(mSlotId, e.toString());
            return HTTP_REQUEST_FAILED_BY_DNS;
        } catch (Exception e) {
            ImsLog.e(mSlotId, e.toString(), e);
            return HTTP_REQUEST_FAILED_UNSPECIFIED;
        } finally {
            if (mConnection != null) {
                mConnection.disconnect();
                mConnection = null;
            }
        }

        return responseCode;
    }

    @Override
    public Document getInputStream() {
        return mDoc;
    }

    @VisibleForTesting
    protected SscUrl getSscUrl() {
        return SscUrl.getInstance();
    }

    @VisibleForTesting
    protected ISscAuthAgent getSscAuthAgent() {
        return SscAuthAgent.getInstance(mSlotId);
    }

    /**
     * This method MUST be called after all the connection information setting is complete.
     */
    private void displayHeaders(URL connectionUrl, boolean isRequest, String body) {
        ImsLog.d(mSlotId, "TEXT_HTTP_START\n\n");

        if (isRequest) {
            ImsLog.d(mConnection.getRequestMethod() + " "
                    + (mConnection.getURL().toString().replace("http://" + connectionUrl.getHost()
                    + ":" + connectionUrl.getPort(), "")) + " HTTP/1.1");

            Map<String, List<String>> properties = mConnection.getRequestProperties();
            Set<String> names = properties.keySet();
            for (String name : names) {
                List<String> values = properties.get(name);
                if (values == null) {
                    continue;
                }

                for (String value : values) {
                    ImsLog.d(name + ": " + value);
                }
            }
        } else {
            try {
                ImsLog.d("HTTP/1.1 " + mConnection.getResponseCode() + " "
                        + mConnection.getResponseMessage());
            } catch (IOException e) {
                ImsLog.e(mSlotId, e.toString());
                return;
            }

            String name;
            for (int i = 0; (name = mConnection.getHeaderFieldKey(i)) != null; ++i) {
                String value = mConnection.getHeaderField(i);
                ImsLog.d(name + ": " + (TextUtils.isEmpty(value) ? "" : value));
            }
        }

        if (!TextUtils.isEmpty(body)) {
            ImsLog.d(mSlotId, "\n" + body);
        }

        ImsLog.d(mSlotId, "\nTEXT_HTTP_END\n\n");
    }

    private void writeContentAndConnect(String body) throws IOException {
        ImsLog.d(mSlotId, "");

        if (!TextUtils.isEmpty(body)) {
            OutputStream os = mConnection.getOutputStream();
            os.write(body.getBytes(java.nio.charset.StandardCharsets.UTF_8));
            os.flush();
            os.close();
        }
    }

    private void setAuthorizationHeader(int requestType, String requestUri, String body) {
        ISscAuthAgent authAgent = getSscAuthAgent();
        if (!authAgent.isCredentialInfoUpdated()) {
            return;
        }

        String method = (requestType == HTTP_REQUEST_GET) ? "GET" : "PUT";
        if (!authAgent.calculateResponse(method, requestUri, body)) {
            return;
        }

        ImsLog.d(mSlotId, "Credentials :: " + authAgent.getCredentialInfoString());
        mConnection.setRequestProperty("Authorization", authAgent.getCredentialInfoString());
    }

    private void setExtraHeaders(URL connectionUrl, String xui, int bodyLength) {
        ImsLog.d(mSlotId, "");

        mConnection.setRequestProperty("X-3GPP-Intended-Identity", "\"" + xui + "\"");

        String host = connectionUrl.getHost();
        mConnection.setRequestProperty("Host", host);

        String userAgent = SscUtils.getInstance().getSscUserAgent(mSlotId);
        if (!TextUtils.isEmpty(userAgent)) {
            mConnection.setRequestProperty("User-Agent", userAgent);
        }

        mConnection.setRequestProperty("Accept-Encoding", "");

        if (bodyLength > 0) {
            mConnection.setRequestProperty("Content-Type", "application/xcap-el+xml");
            mConnection.setRequestProperty("Accept-Charset", "utf-8");
        }

        mConnection.setRequestProperty("Content-Length", Integer.toString(bodyLength));
    }

    private Document readInputStream(InputStream in) {
        if (in == null) {
            ImsLog.d(mSlotId, "InputStream is null");
            return null;
        }

        Document document;
        try {
            DocumentBuilderFactory factory = DocumentBuilderFactory.newInstance();
            DocumentBuilder builder = factory.newDocumentBuilder();
            document = builder.parse(in);
        } catch (Exception e) {
            ImsLog.e(mSlotId, e.toString(), e);
            return null;
        } finally {
            try {
                in.close();
            } catch (IOException e) {
                ImsLog.e(mSlotId, e.toString());
            }
        }

        return document;
    }

    protected void setSocketFactory() {
        // no OP - only for HTTPS
    }

    protected void setHostnameVerifier() {
        // no OP - only for HTTPS
    }

    protected String getCipherSuiteFromConn() {
        // no OP - only for HTTPS
        return null;
    }
}
