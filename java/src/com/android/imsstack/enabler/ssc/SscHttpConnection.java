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

import android.content.Context;
import android.net.Network;
import android.net.NetworkCapabilities;
import android.os.Handler;
import android.text.TextUtils;

import com.android.imsstack.core.agents.dcm.DCFactory;
import com.android.imsstack.core.agents.dcmif.EApnType;
import com.android.imsstack.core.agents.dcmif.IDCApn;
import com.android.imsstack.util.ImsLog;

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
    // Constants--------------------------------------------------
    protected static final int REQUEST_FAILED = -1;

    protected static final int HTTP_GET_REQUEST = 10000;
    protected static final int HTTP_PUT_REQUEST = 10001;
    protected static final int HTTP_DELETE_REQUEST = 10002;

    private final int HTTP_CONNECTION_TIMEOUT = 25 * 1000;
    private final int HTTP_READ_TIMEOUT = 10 * 1000;

    // Variables--------------------------------------------------
    protected Context mContext = null;
    protected Handler mTransactionHandler = null;

    protected HttpURLConnection mConnection = null;
    protected URL mConnectionUrl = null;

    protected EApnType mApnType = null;
    protected String mXui = null;
    protected String mHost = null;
    protected int mBodyLength = 0;
    protected Document mDoc = null;

    protected int mSlotId = -1;

    // Static loading materials ----------------------------------
    // Public methods --------------------------------------------
    public SscHttpConnection(int slotId, Context context, EApnType apntype) {
        ImsLog.d("slotId/context/apntype : " + slotId + "/" + context + "/" + apntype);

        mSlotId = slotId;
        mContext = context;
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
    public int sendRequest(int requestType, String requestUri, String body) {
        ImsLog.d("requestType : " + requestType + ", requestUri : " + requestUri);

        if (requestUri == null) {
            ImsLog.e("Request URI is null");
            return REQUEST_FAILED;
        }

        mConnectionUrl = SscUrl.getInstance().getConnectionUrl(mSlotId, requestUri);
        if (mConnectionUrl == null) {
            ImsLog.e("URL is null");
            return REQUEST_FAILED;
        }

        int responseCode = REQUEST_FAILED;
        try {
            Network nw = null;
            IDCApn dcGovApnCtrl = (IDCApn)DCFactory.getDC(DCFactory.APN, mSlotId);
            if (dcGovApnCtrl != null) {
                if (mApnType.equals(EApnType.INTERNET)) {
                    nw = dcGovApnCtrl.getNetworkByCapabilityWithTransportType(mApnType.getType(),
                        NetworkCapabilities.TRANSPORT_CELLULAR);
                } else {
                    nw = dcGovApnCtrl.getNetworkByCapability(mApnType.getType());
                }
            }

            if (nw == null) {
                ImsLog.e("Network is null; apnType=" + mApnType);
                SscServiceStateAgent.getInstance().setDnsQueryFailed(mSlotId, true);
                return REQUEST_FAILED;
            }

            mConnection = (HttpURLConnection)nw.openConnection(mConnectionUrl);
            if (mConnection == null) {
                ImsLog.e("HTTP URL Connection is null");
                return REQUEST_FAILED;
            }

            setSocketFactory();
            setHostnameVerifier();

            // Sets timer values
            mConnection.setConnectTimeout(HTTP_CONNECTION_TIMEOUT);
            mConnection.setReadTimeout(HTTP_READ_TIMEOUT);

            ImsLog.d("body : \n" + body);

            if (body != null && body.length() > 0)  {
                mBodyLength = body.length();
                ImsLog.d("mBodyLength : " + mBodyLength);
            } else {
                mBodyLength = 0;
            }

            setAuthorizationHeader(requestType, requestUri, body);
            setExtraHeaders();

            if (requestType == HTTP_GET_REQUEST) {
                mConnection.setRequestMethod("GET");
                mConnection.setRequestProperty("Connection", "Keep-Alive");
                mConnection.setDoInput(true);
                displayHeaders(true, body);
                mConnection.connect();
            } else if (requestType == HTTP_PUT_REQUEST) {
                mConnection.setRequestMethod("PUT");
                mConnection.setDoOutput(true);
                if (!TextUtils.isEmpty(SscAuthAgent.getInstance(mSlotId).getETag())) {
                    mConnection.setRequestProperty(
                            "If-Match", SscAuthAgent.getInstance(mSlotId).getETag());
                }
                displayHeaders(true, body);
                writeContentAndConnect(body);
            }

            ImsLog.i("Connect");

            responseCode = mConnection.getResponseCode();
            ImsLog.i("Response Code : " + responseCode);
/*
            Message msg = Message.obtain();
            msg.what = SscTransaction.EVENT_GOT_HTTP_REQUEST_RESPONSE;
            String strResponse = mConnection.getResponseMessage();
            msg.obj = strResponse;

            if (mTransactionHandler != null) {
                mTransactionHandler.sendMessage(msg);
            } else {
                ImsLog.e("mTransactionHandler is null");
                return REQUEST_FAILED;
            }
*/
            String strResponse = mConnection.getResponseMessage();
            ImsLog.i("Response Message : " + strResponse);

            ISscAuthAgent authAgent = SscAuthAgent.getInstance(mSlotId);
            if (TextUtils.isEmpty(mConnection.getHeaderField("ETag")) != true) {
               authAgent.setETag(mConnection.getHeaderField("ETag"));
            }

            authAgent.setCipherSuite(getCipherSuiteFromConn());

            String authnticateHeader = mConnection.getHeaderField("WWW-Authenticate");
            if (authnticateHeader != null) { // 401 Case
                authAgent.parse(authnticateHeader);
            } else { // authnticateHeader is null
                authnticateHeader = mConnection.getHeaderField("Authentication-Info");
                if (authnticateHeader != null) { // 2XX case
                    authAgent.parse(authnticateHeader);
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

            displayHeaders(false, "");
        } catch (UnknownHostException e) {
            ImsLog.e(e.toString());
            SscServiceStateAgent.getInstance().setDnsQueryFailed(mSlotId, true);
            return REQUEST_FAILED;
        } catch (Exception e) {
            ImsLog.e(e.toString());
            e.printStackTrace();
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
    public void setCredentialOnChallenge(String body) {
        ImsLog.i("");
/* TODO_JS : http digest?
        ISscAuthAgent authAgent = SscAuthAgent.getInstance(mSlotId);
        String response = authAgent.calculateResponse(mXui,
                SscConfig.MMTel.getAuthPassword(mSlotId), body);
        if (response == null) {
            ImsLog.e("RESPONSE is null");
        }
 */
    }

    @Override
    public void setTransactionHandler(Handler handler) {
        mTransactionHandler = handler;
    }

    @Override
    public void setXuiValue(String xui) {
        mXui = xui;
    }

    @Override
    public Document getInputStream() {
        return mDoc;
    }

    // Interface implementation methods --------------------------
    // Private/Protected methods ---------------------------------
    /**
     * displayHeaders
     *     This method MUST be called after all the connection information setting is complete.
     *  This method send HTTP request to server if the connection is not established.
     * @param connection
     * @param isRequest
     */
    protected void displayHeaders(boolean isRequest, String body) {
        ImsLog.d("");

        if (mConnection == null) {
            ImsLog.e("Connection is null");
            return;
        }

        ImsLog.d("\nTEXT_HTTP_START\n\n");
        if (isRequest) {
            ImsLog.d("==SEND==>>>\n\n");
            ImsLog.d(mConnection.getRequestMethod() + " "
                    + (mConnection.getURL().toString().replace("http://" + mConnectionUrl.getHost()
                    + ":" + mConnectionUrl.getPort(), "")) + " HTTP/1.1");

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
            ImsLog.d("==RECV==>>>\n\n");
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

    protected void displayHeaders(boolean isRequest, InputStream in) {
        ImsLog.d("InputStream : " + in);

        StringBuffer sb = new StringBuffer();
        byte[] b = new byte[4096];
        try {
            for (int n; (n = in.read(b)) != -1;) {
                sb.append(new String(b, 0, n));
            }
        } catch (IOException e) {
            ImsLog.e(e.toString());
            e.printStackTrace();
        }

        displayHeaders(isRequest, sb.toString());
    }

    protected void writeContentAndConnect(String body) throws IOException {
        ImsLog.d("");

        if (mBodyLength > 0) {
            OutputStream os = mConnection.getOutputStream();
            os.write(body.getBytes("utf-8"));
            os.flush();
            os.close();
        }
    }

    protected void setAuthorizationHeader(int requestType, String requestUri, String body) {
        ISscAuthAgent authAgent = SscAuthAgent.getInstance(mSlotId);
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

    protected void setExtraHeaders() {
        ImsLog.d("");

        if (mConnection == null) {
            ImsLog.e("HTTP Connection is null");
            return;
        }

        mConnection.setRequestProperty("X-3GPP-Intended-Identity", "\"" + mXui + "\"");

        String host = mConnectionUrl.getHost();
        /* TODO_JS : Need test in production network w/o port number in host header
        if (SscConfig.isSetPortInHostHeader(mSlotId)) {
            if (mConnectionUrl.getPort() >= 0) {
                host += ":" + String.valueOf(mConnectionUrl.getPort());
            }
        }
         */

        mConnection.setRequestProperty("Host", host);

        // Date info
        //mConnection.setRequestProperty("Date"
        //            , DateUtils.formatDate(new Date(), DateUtils.PATTERN_RFC1123));

        String userAgent = SscUtils.getUtUserAgent(mSlotId);
        if (!TextUtils.isEmpty(userAgent)) {
            mConnection.setRequestProperty("User-Agent", userAgent);
        }

        mConnection.setRequestProperty("Accept-Encoding", "");

        if (mBodyLength > 0) {
            mConnection.setRequestProperty("Content-Type", "application/xcap-el+xml");
            mConnection.setRequestProperty("Accept-Charset", "utf-8");
        }

        mConnection.setRequestProperty("Content-Length", Integer.toString(mBodyLength));
    }

    protected void readInputStream(InputStream in) {
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
