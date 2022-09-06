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
import java.net.URL;
import java.net.UnknownHostException;
import java.util.Iterator;
import java.util.List;
import java.util.Map;
import java.util.Set;

import javax.xml.parsers.DocumentBuilder;
import javax.xml.parsers.DocumentBuilderFactory;

public class SscHttpConnection implements ISscHttpConnection {
    protected static final int REQUEST_FAILED = -1;
    protected static final int REQUEST_FAILED_BY_DNS = -2;

    protected static final int HTTP_GET_REQUEST = 10000;
    protected static final int HTTP_PUT_REQUEST = 10001;
    protected static final int HTTP_DELETE_REQUEST = 10002;

    private static final int HTTP_CONNECTION_TIMEOUT = 30 * 1000;
    private static final int HTTP_READ_TIMEOUT = 20 * 1000;

    protected HttpURLConnection mConnection = null;
    protected EApnType mApnType = null;
    protected String mHost = null;
    protected Document mDoc = null;

    protected int mSlotId = -1;

    public SscHttpConnection(int slotId, EApnType apntype) {
        ImsLog.d("slotId/context/apntype : " + slotId + "/" + apntype);

        mSlotId = slotId;
        mApnType = apntype;
    }

    @Override
    public void close() {
        ImsLog.w("HttpConnection - close()");
        if (mConnection != null) {
            mConnection.disconnect();
            mConnection = null;
        }
    }

    @Override
    public int sendRequest(int requestType, String requestUri, String xui, String body) {
        ImsLog.d("requestType : " + requestType + ", body : \n" + body);

        if (TextUtils.isEmpty(requestUri) || TextUtils.isEmpty(xui)) {
            ImsLog.e("requestUri or xui is invalid");
            return REQUEST_FAILED;
        }

        URL connectionUrl = getSscUrl().getConnectionUrl(mSlotId, requestUri);
        if (connectionUrl == null) {
            ImsLog.e("URL is null");
            return REQUEST_FAILED;
        }

        int responseCode = REQUEST_FAILED;
        try {
            Network nw = null;
            IDcApn dcGovApnCtrl = (IDcApn) DcFactory.getDc(DcFactory.APN, mSlotId);
            if (dcGovApnCtrl != null) {
                nw = dcGovApnCtrl.getNetworkByCapability(mApnType.getType());
            }

            if (nw == null) {
                ImsLog.e("Network is null; apnType=" + mApnType);
                return REQUEST_FAILED;
            }

            mConnection = (HttpURLConnection) nw.openConnection(connectionUrl);
            if (mConnection == null) {
                ImsLog.e("HTTP URL Connection is null");
                return REQUEST_FAILED;
            }

            setSocketFactory();
            setHostnameVerifier();

            // Sets timer values
            mConnection.setConnectTimeout(HTTP_CONNECTION_TIMEOUT);
            mConnection.setReadTimeout(HTTP_READ_TIMEOUT);

            setAuthorizationHeader(requestType, requestUri, body);

            int bodyLength = 0;
            if (!TextUtils.isEmpty(body))  {
                bodyLength = body.length();
            }

            setExtraHeaders(connectionUrl, xui, bodyLength);

            ISscAuthAgent authAgent = getSscAuthAgent();
            if (requestType == HTTP_GET_REQUEST) {
                mConnection.setRequestMethod("GET");
                mConnection.setRequestProperty("Connection", "Keep-Alive");
                mConnection.setDoInput(true);
                displayHeaders(connectionUrl, true, body);
                mConnection.connect();
            } else if (requestType == HTTP_PUT_REQUEST) {
                mConnection.setRequestMethod("PUT");
                mConnection.setDoOutput(true);
                if (!TextUtils.isEmpty(authAgent.getETag())) {
                    mConnection.setRequestProperty("If-Match", authAgent.getETag());
                }
                displayHeaders(connectionUrl, true, body);
                writeContentAndConnect(body);
            }

            ImsLog.i("Connect");

            responseCode = mConnection.getResponseCode();
            ImsLog.i("Response Code : " + responseCode);

            String strResponse = mConnection.getResponseMessage();
            ImsLog.d("Response Message : " + strResponse);

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
                ImsLog.w("getContentLength > 0");
                if ((responseCode >= 200) && (responseCode < 300)) {
                    readInputStream(mConnection.getInputStream());
                } else {
                    readInputStream(mConnection.getErrorStream());
                }
            } else if (("chunked").equalsIgnoreCase(
                    mConnection.getHeaderField("Transfer-Encoding"))) {
                ImsLog.w("Transfer Encoding : chunked");
                if ((responseCode >= 200) && (responseCode < 300)) {
                    readInputStream(mConnection.getInputStream());
                } else {
                    readInputStream(mConnection.getErrorStream());
                }
            } else {
                mDoc = null;
            }

            displayHeaders(connectionUrl, false, "");
        } catch (UnknownHostException e) {
            ImsLog.e(e.toString());
            return REQUEST_FAILED_BY_DNS;
        } catch (Exception e) {
            ImsLog.e(e.toString());
            return REQUEST_FAILED;
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
        ImsLog.d("\nTEXT_HTTP_START\n\n");

        if (isRequest) {
            ImsLog.d(mConnection.getRequestMethod() + " "
                    + (mConnection.getURL().toString().replace("http://" + connectionUrl.getHost()
                    + ":" + connectionUrl.getPort(), "")) + " HTTP/1.1");

            Map<String, List<String>> properties = mConnection.getRequestProperties();
            if (properties == null) {
                return;
            }

            Set<String> names = properties.keySet();
            if (names == null) {
                return;
            }

            Iterator<String> iterator = names.iterator();
            while (iterator.hasNext()) {
                String name = iterator.next();
                List<String> values = properties.get(name);
                if (values == null) {
                    continue;
                }

                ImsLog.d(name + ": " + values.get(0));
            }

            ImsLog.d("\n");
        } else {
            try {
                ImsLog.d("HTTP/1.1 " + mConnection.getResponseCode() + " " +
                        mConnection.getResponseMessage());
            } catch (IOException e) {
                ImsLog.e("Connnection is null");
                e.printStackTrace();
                return;
            }

            int pos = 0;
            String name = null;
            String value = null;

            while (((name = mConnection.getHeaderFieldKey(pos)) != null)
                    && ((value = mConnection.getHeaderField(pos)) != null)) {

                ImsLog.d(name + ": " + value);
                ++pos;
            }

            ImsLog.d("\n");
        }

        if (!TextUtils.isEmpty(body)) {
            ImsLog.d(body);
        }

        ImsLog.d("\nTEXT_HTTP_END\n\n");
    }

    private void writeContentAndConnect(String body) throws IOException {
        ImsLog.d("");

        if (!TextUtils.isEmpty(body)) {
            OutputStream os = mConnection.getOutputStream();
            os.write(body.getBytes("utf-8"));
            os.flush();
            os.close();
        }
    }

    private void setAuthorizationHeader(int requestType, String requestUri, String body) {
        ISscAuthAgent authAgent = getSscAuthAgent();
        if (authAgent.isCredentialInfoUpdated() == false) {
            return;
        }

        String mothod = (requestType == HTTP_GET_REQUEST) ? "GET" : "PUT";
        if (authAgent.calculateResponse(mothod, requestUri, body) == false) {
            return;
        }

        ImsLog.d("Credentials :: " + authAgent.getCredentialInfoString());
        mConnection.setRequestProperty("Authorization", authAgent.getCredentialInfoString());
    }

    private void setExtraHeaders(URL connectionUrl, String xui, int bodyLength) {
        ImsLog.d("");

        mConnection.setRequestProperty("X-3GPP-Intended-Identity", "\"" + xui + "\"");

        String host = connectionUrl.getHost();
        /* TODO_JS : Need test in production network w/o port number in host header
        if (SscConfig.isSetPortInHostHeader(mSlotId)) {
            if (connectionUrl.getPort() >= 0) {
                host += ":" + String.valueOf(connectionUrl.getPort());
            }
        }
         */

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

    private void readInputStream(InputStream in) {
        if (in == null) {
            ImsLog.d("InputStream is null");
            mDoc = null;
            return;
        }

        try {
            DocumentBuilderFactory factory = DocumentBuilderFactory.newInstance();
            DocumentBuilder builder = factory.newDocumentBuilder();
            mDoc = builder.parse(in);
        } catch (Exception e) {
            ImsLog.e(e.toString());
            e.printStackTrace();
            mDoc = null;
        } finally {
            try {
                in.close();
            } catch (IOException e) {
                ImsLog.e(e.toString());
                e.printStackTrace();
            }
        }
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
