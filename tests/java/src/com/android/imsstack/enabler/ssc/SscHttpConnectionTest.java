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

import static org.junit.Assert.assertEquals;
import static org.junit.Assert.assertNotNull;
import static org.junit.Assert.assertNull;
import static org.junit.Assert.fail;
import static org.mockito.ArgumentMatchers.any;
import static org.mockito.ArgumentMatchers.anyString;
import static org.mockito.ArgumentMatchers.eq;
import static org.mockito.Mockito.doThrow;
import static org.mockito.Mockito.mock;
import static org.mockito.Mockito.times;
import static org.mockito.Mockito.verify;
import static org.mockito.Mockito.when;

import android.net.Network;
import android.telephony.CarrierConfigManager;

import com.android.imsstack.core.agents.AgentFactory;
import com.android.imsstack.core.agents.ConfigAgent;
import com.android.imsstack.core.agents.WifiInterface;
import com.android.imsstack.core.agents.dcm.DcFactory;
import com.android.imsstack.core.agents.dcmif.EApnType;
import com.android.imsstack.core.agents.dcmif.IDcApn;
import com.android.imsstack.core.config.CarrierConfig;

import org.junit.After;
import org.junit.Before;
import org.junit.Test;
import org.junit.runner.RunWith;
import org.junit.runners.JUnit4;
import org.mockito.Mock;
import org.mockito.MockitoAnnotations;
import org.w3c.dom.Document;

import java.io.ByteArrayInputStream;
import java.io.IOException;
import java.io.InputStream;
import java.io.OutputStream;
import java.net.HttpURLConnection;
import java.net.MalformedURLException;
import java.net.SocketTimeoutException;
import java.net.URL;
import java.net.UnknownHostException;
import java.util.ArrayList;
import java.util.HashMap;
import java.util.List;
import java.util.Map;

@RunWith(JUnit4.class)
public class SscHttpConnectionTest {
    private static final int SLOT_0 = 0;
    private EApnType mApnType = EApnType.XCAP;
    private String mXui = "tel:+1234567890";
    private String mRequestUri = "/simservs.ngn.etsi.org/users/" + mXui + "/simservs.xml";
    private String mFqdn = "xcap.pub.3gppnetwork.org";
    private URL mUrl;

    private FakeSscHttpConnection mSscHttpConnection;

    @Mock private CarrierConfig mMockCarrierConfig;
    @Mock private ConfigAgent mMockConfigAgent;
    @Mock private IDcApn mMockDcApn;
    @Mock private Network mMockNetwork;
    @Mock private SscAuthAgent mMockSscAuthAgent;
    @Mock private SscUrl mMockSscUrl;
    @Mock private HttpURLConnection mMockConnection;
    @Mock private OutputStream mMockOutputStream;
    @Mock private WifiInterface mMockWifiInterface;

    @Before
    public void setup() throws MalformedURLException, IOException {
        MockitoAnnotations.initMocks(this);

        mUrl = new URL("http://" + mFqdn + mRequestUri);

        when(mMockSscUrl.getConnectionUrl(SLOT_0, mRequestUri)).thenReturn(mUrl);
        when(mMockConnection.getURL()).thenReturn(mUrl);
        when(mMockDcApn.getNetworkByCapability(mApnType.getType())).thenReturn(mMockNetwork);
        when(mMockNetwork.openConnection(mUrl)).thenReturn(mMockConnection);
        when(mMockSscAuthAgent.isCredentialInfoUpdated()).thenReturn(false);

        AgentFactory.getInstance().setAgent(WifiInterface.class, mMockWifiInterface);
        DcFactory.setDcAgent(IDcApn.class, mMockDcApn, SLOT_0);

        when(mMockConfigAgent.getCarrierConfig()).thenReturn(mMockCarrierConfig);
        SscConfig.setConfigAgent(SLOT_0, mMockConfigAgent);

        mSscHttpConnection = new FakeSscHttpConnection(SLOT_0, mApnType);
    }

    @After
    public void tearDown() {
        SscConfig.clear(SLOT_0);
        DcFactory.setDcAgent(IDcApn.class, null, SLOT_0);
    }

    @Test
    public void close_shouldDisconnectConnection() {
        int result = mSscHttpConnection.sendRequest(ISscHttpConnection.HTTP_REQUEST_GET,
                mRequestUri, mXui, "");
        mSscHttpConnection.close();

        verify(mMockConnection).disconnect();
        assertEquals(ISscHttpConnection.HTTP_REQUEST_FAILED_UNSPECIFIED, result);
    }

    @Test
    public void sendRequest_invalidRequestUri() {
        String invalidRequestUri = null;
        int result = mSscHttpConnection.sendRequest(ISscHttpConnection.HTTP_REQUEST_GET,
                invalidRequestUri, mXui, "");

        assertEquals(ISscHttpConnection.HTTP_REQUEST_FAILED_BY_INVALID_URI, result);
    }

    @Test
    public void sendRequest_invalidXui() {
        String invalidXui = null;
        int result = mSscHttpConnection.sendRequest(ISscHttpConnection.HTTP_REQUEST_GET,
                mRequestUri, invalidXui, "");

        assertEquals(ISscHttpConnection.HTTP_REQUEST_FAILED_BY_INVALID_XUI, result);
    }

    @Test
    public void sendRequest_failToGetUrl() {
        when(mMockSscUrl.getConnectionUrl(SLOT_0, mRequestUri)).thenReturn(null);

        int result = mSscHttpConnection.sendRequest(ISscHttpConnection.HTTP_REQUEST_GET,
                mRequestUri, mXui, "");

        verify(mMockSscUrl).getConnectionUrl(SLOT_0, mRequestUri);
        assertEquals(ISscHttpConnection.HTTP_REQUEST_FAILED_BY_INVALID_XCAP_ROOT_URI, result);
    }

    @Test
    public void sendRequest_networkIsNull() {
        when(mMockDcApn.getNetworkByCapability(mApnType.getType())).thenReturn(null);

        int result = mSscHttpConnection.sendRequest(ISscHttpConnection.HTTP_REQUEST_GET,
                mRequestUri, mXui, "");

        verify(mMockDcApn).getNetworkByCapability(mApnType.getType());
        assertEquals(ISscHttpConnection.HTTP_REQUEST_FAILED_BY_NO_NETWORK, result);
    }

    @Test
    public void sendRequest_wifiInterfaceIsNullForWifi() {
        mApnType = EApnType.WIFI;
        mSscHttpConnection = new FakeSscHttpConnection(SLOT_0, mApnType);
        AgentFactory.getInstance().setAgent(WifiInterface.class, null);

        int result = mSscHttpConnection.sendRequest(ISscHttpConnection.HTTP_REQUEST_GET,
                mRequestUri, mXui, "");

        assertEquals(ISscHttpConnection.HTTP_REQUEST_FAILED_BY_NO_NETWORK, result);
    }

    @Test
    public void sendRequest_networkIsNullForWifi() {
        mApnType = EApnType.WIFI;
        mSscHttpConnection = new FakeSscHttpConnection(SLOT_0, mApnType);
        when(mMockWifiInterface.getNetwork()).thenReturn(null);

        int result = mSscHttpConnection.sendRequest(ISscHttpConnection.HTTP_REQUEST_GET,
                mRequestUri, mXui, "");

        assertEquals(ISscHttpConnection.HTTP_REQUEST_FAILED_BY_NO_NETWORK, result);
    }

    @Test
    public void sendRequest_failToOpenConnection() throws IOException {
        when(mMockNetwork.openConnection(any())).thenReturn(null);

        int result = mSscHttpConnection.sendRequest(ISscHttpConnection.HTTP_REQUEST_GET,
                mRequestUri, mXui, "");

        verify(mMockNetwork).openConnection(any());
        assertEquals(ISscHttpConnection.HTTP_REQUEST_FAILED_BY_CONNECTION, result);
    }

    @Test
    public void sendRequest_setAuthHeaderWhenCredentialNotUpdated() {
        mSscHttpConnection.sendRequest(ISscHttpConnection.HTTP_REQUEST_GET, mRequestUri, mXui, "");

        verify(mMockConnection, times(0)).setRequestProperty(eq("Authorization"), any());
    }

    @Test
    public void sendRequest_setAuthHeaderWhenCalculateResponseFailed() {
        when(mMockSscAuthAgent.isCredentialInfoUpdated()).thenReturn(true);
        when(mMockSscAuthAgent.calculateResponse(anyString(), anyString(), anyString()))
                .thenReturn(false);

        mSscHttpConnection.sendRequest(ISscHttpConnection.HTTP_REQUEST_GET, mRequestUri, mXui, "");

        verify(mMockConnection, times(0)).setRequestProperty(eq("Authorization"), any());
    }

    @Test
    public void sendRequest_setAuthHeaderWhenCalculateResponseSucceeded() {
        when(mMockSscAuthAgent.isCredentialInfoUpdated()).thenReturn(true);
        when(mMockSscAuthAgent.calculateResponse(anyString(), anyString(), anyString()))
                .thenReturn(true);
        when(mMockSscAuthAgent.getCredentialInfoString()).thenReturn("credentialInfo");

        mSscHttpConnection.sendRequest(ISscHttpConnection.HTTP_REQUEST_GET, mRequestUri, mXui, "");

        verify(mMockConnection).setRequestProperty("Authorization", "credentialInfo");
    }

    @Test
    public void sendRequest_setExtraHeadersWithoutBody() {
        String identity = "\"" + mXui + "\"";
        String xmlBody = "";
        when(mMockCarrierConfig.getString(CarrierConfigManager.Ims.KEY_IMS_USER_AGENT_STRING))
                .thenReturn("ImsClient");

        mSscHttpConnection.sendRequest(ISscHttpConnection.HTTP_REQUEST_GET, mRequestUri, mXui,
                xmlBody);

        verify(mMockConnection).setRequestProperty("X-3GPP-Intended-Identity", identity);
        verify(mMockConnection).setRequestProperty("Host", mFqdn);
        verify(mMockConnection).setRequestProperty("Accept-Encoding", "");
        verify(mMockConnection).setRequestProperty("User-Agent", "ImsClient 3gpp-gba");
        verify(mMockConnection, times(0)).setRequestProperty(eq("Content-Type"), any());
        verify(mMockConnection, times(0)).setRequestProperty(eq("Accept-Charset"), any());
        verify(mMockConnection).setRequestProperty("Content-Length", "0");
    }

    @Test
    public void sendRequest_setExtraHeadersWithBody() throws Exception {
        String identity = "\"" + mXui + "\"";
        String xmlBody = "This is XML body";
        when(mMockConnection.getOutputStream()).thenReturn(mMockOutputStream);
        when(mMockCarrierConfig.getString(CarrierConfigManager.Ims.KEY_IMS_USER_AGENT_STRING))
                .thenReturn("ImsClient");

        mSscHttpConnection.sendRequest(ISscHttpConnection.HTTP_REQUEST_PUT, mRequestUri, mXui,
                xmlBody);

        verify(mMockConnection).setRequestProperty("X-3GPP-Intended-Identity", identity);
        verify(mMockConnection).setRequestProperty("Host", mFqdn);
        verify(mMockConnection).setRequestProperty("Accept-Encoding", "");
        verify(mMockConnection).setRequestProperty("User-Agent", "ImsClient 3gpp-gba");
        verify(mMockConnection).setRequestProperty("Content-Type", "application/xcap-el+xml");
        verify(mMockConnection).setRequestProperty("Accept-Charset", "utf-8");
        verify(mMockConnection).setRequestProperty("Content-Length",
                Integer.toString(xmlBody.length()));
    }

    @Test
    public void sendRequest_dnsQueryFailure() throws Exception {
        doThrow(new UnknownHostException()).when(mMockConnection).connect();

        int result = mSscHttpConnection.sendRequest(ISscHttpConnection.HTTP_REQUEST_GET,
                mRequestUri, mXui, "");

        verify(mMockConnection).connect();
        verify(mMockConnection).disconnect();
        assertEquals(ISscHttpConnection.HTTP_REQUEST_FAILED_BY_DNS, result);
        assertNull(mSscHttpConnection.getInputStream());
    }

    @Test
    public void sendRequest_socketTimeout() throws Exception {
        doThrow(new SocketTimeoutException()).when(mMockConnection).connect();

        int result = mSscHttpConnection.sendRequest(ISscHttpConnection.HTTP_REQUEST_GET,
                mRequestUri, mXui, "");

        verify(mMockConnection).connect();
        verify(mMockConnection).disconnect();
        assertEquals(ISscHttpConnection.HTTP_REQUEST_FAILED_BY_TIMEOUT, result);
        assertNull(mSscHttpConnection.getInputStream());
    }

    @Test
    public void sendRequest_ioexception() throws Exception {
        doThrow(new IOException()).when(mMockConnection).getResponseCode();

        int result = mSscHttpConnection.sendRequest(ISscHttpConnection.HTTP_REQUEST_GET,
                mRequestUri, mXui, "");

        verify(mMockConnection).connect();
        verify(mMockConnection).disconnect();
        assertEquals(ISscHttpConnection.HTTP_REQUEST_FAILED_UNSPECIFIED, result);
        assertNull(mSscHttpConnection.getInputStream());
    }

    @Test
    public void sendRequest_errorResponseForGetRequest() throws Exception {
        String xml = "<xcap-error />";

        when(mMockConnection.getResponseCode()).thenReturn(SscConstant.HTTP_CONFLICT);
        when(mMockConnection.getContentLength()).thenReturn(xml.length());
        when(mMockConnection.getErrorStream()).thenReturn(createInputStream(xml));

        int result = mSscHttpConnection.sendRequest(ISscHttpConnection.HTTP_REQUEST_GET,
                mRequestUri, mXui, "");

        verify(mMockConnection).connect();
        verify(mMockConnection).getErrorStream();
        assertEquals(SscConstant.HTTP_CONFLICT, result);
        assertNotNull(mSscHttpConnection.getInputStream());
    }

    @Test
    public void sendRequest_errorResponseForGetRequestWithChunkedData() throws Exception {
        String xml = "<xcap-error />";

        when(mMockConnection.getResponseCode()).thenReturn(SscConstant.HTTP_CONFLICT);
        when(mMockConnection.getContentLength()).thenReturn(0);
        when(mMockConnection.getHeaderField("Transfer-Encoding")).thenReturn("chunked");
        when(mMockConnection.getErrorStream()).thenReturn(createInputStream(xml));

        int result = mSscHttpConnection.sendRequest(ISscHttpConnection.HTTP_REQUEST_GET,
                mRequestUri, mXui, "");

        verify(mMockConnection).connect();
        verify(mMockConnection).getErrorStream();
        assertEquals(SscConstant.HTTP_CONFLICT, result);
        assertNotNull(mSscHttpConnection.getInputStream());
    }

    @Test
    public void sendRequest_authChallengeForGetRequest() throws Exception {
        String wwwAuthenticate = "Digest Algorithm=\"MD5\", "
                + "nonce=\"ABCDEFGHIJKLMNOPQRSTUVWXYZ123456==\", "
                + "realm=\"3GPP-bootstrapping@xcap.msg.pc.t-mobile.com;\", "
                + "qop=\"auth\", opaque=\"a1b2c3d4e5f6g7h8i9j0kl==\"";

        when(mMockConnection.getResponseCode()).thenReturn(SscConstant.HTTP_UNAUTHORIZED);
        when(mMockConnection.getHeaderField("WWW-Authenticate")).thenReturn(wwwAuthenticate);

        int result = mSscHttpConnection.sendRequest(ISscHttpConnection.HTTP_REQUEST_GET,
                mRequestUri, mXui, "");

        verify(mMockConnection).connect();
        verify(mMockSscAuthAgent).parse(wwwAuthenticate);
        assertEquals(SscConstant.HTTP_UNAUTHORIZED, result);
        assertNull(mSscHttpConnection.getInputStream());
    }

    @Test
    public void sendRequest_inputStreamIsNull() throws Exception {
        String xml = "<simservs />";
        mSscHttpConnection.mDoc = createDocumentFromString(xml);
        when(mMockConnection.getResponseCode()).thenReturn(SscConstant.HTTP_OK);
        when(mMockConnection.getInputStream()).thenReturn(null);
        when(mMockConnection.getContentLength()).thenReturn(xml.length());

        int result = mSscHttpConnection.sendRequest(ISscHttpConnection.HTTP_REQUEST_GET,
                mRequestUri, mXui, "");

        verify(mMockConnection).connect();
        assertEquals(SscConstant.HTTP_OK, result);
        assertNull(mSscHttpConnection.getInputStream());
    }

    @Test
    public void sendRequest_ioexceptionDuringParsingAndClosingInputStream() throws Exception {
        InputStream inputStream = mock(InputStream.class);
        String xml = "<simservs />";
        mSscHttpConnection.mDoc = createDocumentFromString(xml);
        when(mMockConnection.getResponseCode()).thenReturn(SscConstant.HTTP_OK);
        when(mMockConnection.getContentLength()).thenReturn(xml.length());
        when(mMockConnection.getInputStream()).thenReturn(inputStream);
        doThrow(new IOException()).when(inputStream).close();

        int result = mSscHttpConnection.sendRequest(ISscHttpConnection.HTTP_REQUEST_GET,
                mRequestUri, mXui, "");

        verify(mMockConnection).connect();
        assertEquals(SscConstant.HTTP_OK, result);
        assertNull(mSscHttpConnection.getInputStream());
    }

    @Test
    public void sendRequest_ioexceptionDuringDisplayingResponseMessage() throws Exception {
        String xml = "<simservs />";
        when(mMockConnection.getResponseCode()).thenReturn(SscConstant.HTTP_OK);
        when(mMockConnection.getContentLength()).thenReturn(xml.length());
        when(mMockConnection.getInputStream()).thenReturn(createInputStream(xml));
        doThrow(new IOException()).when(mMockConnection).getResponseMessage();

        int result = mSscHttpConnection.sendRequest(ISscHttpConnection.HTTP_REQUEST_GET,
                mRequestUri, mXui, "");

        verify(mMockConnection).connect();
        verify(mMockConnection).getResponseMessage();
        verify(mMockConnection).disconnect();
        assertEquals(SscConstant.HTTP_OK, result);
        assertNotNull(mSscHttpConnection.getInputStream());
    }

    @Test
    public void sendRequest_successForGetRequest() throws Exception {
        String authenticationInfo = "authentication-info: "
                + "nextnonce =\"ABc1DE2fgHIjKLMnOPq3RstuVWXyZ45678==\", qop=\"auth\", "
                + "rspauth=abc7d41e0fgh8ijk97997lmnop367q9, "
                + "cnonce=\"a993b0369c887d982031efg5hij704k4\", nc=00000002";
        String xml = "<simservs />";
        String eTag = "ETag-ABCD";
        Map<String, List<String>> properties = new HashMap<>();
        List<String> values = new ArrayList<>();
        values.add(eTag);
        properties.put("ETag", values);

        when(mMockConnection.getResponseCode()).thenReturn(SscConstant.HTTP_OK);
        when(mMockConnection.getHeaderField("ETag")).thenReturn(eTag);
        when(mMockConnection.getHeaderField("Authentication-Info")).thenReturn(authenticationInfo);
        when(mMockConnection.getContentLength()).thenReturn(xml.length());
        when(mMockConnection.getInputStream()).thenReturn(createInputStream(xml));
        when(mMockConnection.getRequestProperties()).thenReturn(properties);
        when(mMockConnection.getHeaderFieldKey(0)).thenReturn("ETag");
        when(mMockConnection.getHeaderField(0)).thenReturn("ETag-1234");

        int result = mSscHttpConnection.sendRequest(ISscHttpConnection.HTTP_REQUEST_GET,
                mRequestUri, mXui, "");

        verify(mMockConnection).setRequestMethod("GET");
        verify(mMockConnection).setRequestProperty("Connection", "Keep-Alive");
        verify(mMockConnection).setDoInput(true);
        verify(mMockConnection).connect();
        verify(mMockSscAuthAgent).setETag(eTag);
        verify(mMockSscAuthAgent).parse(authenticationInfo);
        verify(mMockConnection).getInputStream();
        verify(mMockConnection).disconnect();
        assertEquals(SscConstant.HTTP_OK, result);
        assertNotNull(mSscHttpConnection.getInputStream());
    }

    @Test
    public void sendRequest_successForGetRequestWithChunkedData() throws Exception {
        String authenticationInfo = "authentication-info: "
                + "nextnonce =\"ABc1DE2fgHIjKLMnOPq3RstuVWXyZ45678==\", qop=\"auth\", "
                + "rspauth=abc7d41e0fgh8ijk97997lmnop367q9, "
                + "cnonce=\"a993b0369c887d982031efg5hij704k4\", nc=00000002";
        String xml = "<simservs />";
        String eTag = "ETag-ABCD";
        Map<String, List<String>> properties = new HashMap<>();
        properties.put("ETag", null);

        when(mMockConnection.getResponseCode()).thenReturn(SscConstant.HTTP_OK);
        when(mMockConnection.getHeaderField("ETag")).thenReturn(eTag);
        when(mMockConnection.getHeaderField("Authentication-Info")).thenReturn(authenticationInfo);
        when(mMockConnection.getContentLength()).thenReturn(0);
        when(mMockConnection.getHeaderField("Transfer-Encoding")).thenReturn("chunked");
        when(mMockConnection.getInputStream()).thenReturn(createInputStream(xml));
        when(mMockConnection.getRequestProperties()).thenReturn(properties);
        when(mMockConnection.getHeaderFieldKey(0)).thenReturn("ETag");
        when(mMockConnection.getHeaderField(0)).thenReturn("");

        int result = mSscHttpConnection.sendRequest(ISscHttpConnection.HTTP_REQUEST_GET,
                mRequestUri, mXui, "");

        verify(mMockConnection).setRequestMethod("GET");
        verify(mMockConnection).setRequestProperty("Connection", "Keep-Alive");
        verify(mMockConnection).setDoInput(true);
        verify(mMockConnection).connect();
        verify(mMockSscAuthAgent).setETag(eTag);
        verify(mMockSscAuthAgent).parse(authenticationInfo);
        verify(mMockConnection).getInputStream();
        verify(mMockConnection).disconnect();
        assertEquals(SscConstant.HTTP_OK, result);
        assertNotNull(mSscHttpConnection.getInputStream());
    }

    @Test
    public void sendRequest_successForPutRequest() throws Exception {
        String authenticationInfo = "authentication-info: "
                + "nextnonce =\"ABc1DE2fgHIjKLMnOPq3RstuVWXyZ45678==\", qop=\"auth\", "
                + "rspauth=abc7d41e0fgh8ijk97997lmnop367q9, "
                + "cnonce=\"a993b0369c887d982031efg5hij704k4\", nc=00000002";
        String xml = "<simservs />";
        String eTag = "ETag-ABCD";

        when(mMockSscAuthAgent.getETag()).thenReturn(eTag);
        when(mMockConnection.getResponseCode()).thenReturn(SscConstant.HTTP_OK);
        when(mMockConnection.getHeaderField("Authentication-Info")).thenReturn(authenticationInfo);
        when(mMockConnection.getOutputStream()).thenReturn(mMockOutputStream);

        int result = mSscHttpConnection.sendRequest(ISscHttpConnection.HTTP_REQUEST_PUT,
                mRequestUri, mXui, xml);

        verify(mMockConnection).setRequestMethod("PUT");
        verify(mMockConnection).setDoOutput(true);
        verify(mMockConnection).setRequestProperty("If-Match", eTag);

        verify(mMockOutputStream).write(any());
        verify(mMockOutputStream).flush();
        verify(mMockOutputStream).close();
        verify(mMockSscAuthAgent).parse(authenticationInfo);
        assertEquals(SscConstant.HTTP_OK, result);
        assertNull(mSscHttpConnection.getInputStream());
    }

    private Document createDocumentFromString(String xml) {
        return SscXmlGovTest.createDocumentFromString(xml);
    }

    private InputStream createInputStream(String data) {
        return new ByteArrayInputStream(data.getBytes());
    }

    private class FakeSscHttpConnection extends SscHttpConnection {
        private FakeSscHttpConnection(int slotId, EApnType apnType) {
            super(slotId, apnType);
        }

        @Override
        protected SscUrl getSscUrl() {
            if (super.getSscUrl() == null) {
                fail();
            }
            return mMockSscUrl;
        }

        @Override
        protected ISscAuthAgent getSscAuthAgent() {
            if (super.getSscAuthAgent() == null) {
                fail();
            }
            return mMockSscAuthAgent;
        }
    }
}
