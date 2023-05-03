/*
 * Copyright (C) 2023 The Android Open Source Project
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
import static org.mockito.ArgumentMatchers.any;
import static org.mockito.Mockito.verify;
import static org.mockito.Mockito.when;

import android.test.suitebuilder.annotation.SmallTest;

import com.android.imsstack.core.agents.dcmif.EApnType;

import org.junit.After;
import org.junit.Before;
import org.junit.Test;
import org.junit.runner.RunWith;
import org.junit.runners.JUnit4;
import org.mockito.Mock;
import org.mockito.MockitoAnnotations;

import java.io.IOException;
import java.net.InetAddress;
import java.net.Socket;

import javax.net.ssl.HttpsURLConnection;
import javax.net.ssl.SSLSocket;
import javax.net.ssl.SSLSocketFactory;

@RunWith(JUnit4.class)
public class SscHttpsConnectionTest {

    private static final int SLOT_0 = 0;

    private SscHttpsConnection mSscHttpsConnection;

    @Mock private HttpsURLConnection mMockConnection;
    @Mock private SSLSocket mMockSslSocket;
    @Mock private SSLSocketFactory mMockSslSocketFactory;
    @Mock private InetAddress mMockHostAddr;
    @Mock private InetAddress mMockClientAddr;

    @Before
    public void setup() {
        MockitoAnnotations.initMocks(this);

        mSscHttpsConnection = new SscHttpsConnection(SLOT_0, EApnType.XCAP);
        mSscHttpsConnection.mConnection = mMockConnection;
    }

    @After
    public void tearDown() {
    }

    @Test
    @SmallTest
    public void setSocketFactory() {
        mSscHttpsConnection.setSocketFactory();

        verify(mMockConnection).setSSLSocketFactory(mSscHttpsConnection.mSscSocketFactory);
    }

    @Test
    @SmallTest
    public void setHostnameVerifier() {
        mSscHttpsConnection.setHostnameVerifier();

        verify(mMockConnection).setHostnameVerifier(SscHttpsConnection.DO_NOT_VERIFY);
    }

    @Test
    @SmallTest
    public void getCipherSuiteFromConn() {
        String tlsCipherSuite = "TLS_RSA_WITH_AES_128_CBC_SHA";
        when(mMockConnection.getCipherSuite()).thenReturn(tlsCipherSuite);

        String cipherSuite = mSscHttpsConnection.getCipherSuiteFromConn();

        assertEquals(tlsCipherSuite, cipherSuite);
    }

    @Test
    @SmallTest
    public void sscSslSocketFactory_getDefaultCipherSuites() {
        String[] defaultCipherSuites = new String[] { "TLS_RSA_WITH_AES_128_CBC_SHA" };
        when(mMockSslSocketFactory.getDefaultCipherSuites()).thenReturn(defaultCipherSuites);
        mSscHttpsConnection.mSscSocketFactory.mSocketFactory = mMockSslSocketFactory;

        String[] cipherSuites = mSscHttpsConnection.mSscSocketFactory.getDefaultCipherSuites();

        verify(mMockSslSocketFactory).getDefaultCipherSuites();
        assertEquals(defaultCipherSuites, cipherSuites);
    }

    @Test
    @SmallTest
    public void sscSslSocketFactory_getSupportedCipherSuites() {
        String[] supportedCipherSuites = new String[] { "TLS_RSA_WITH_AES_128_CBC_SHA" };
        when(mMockSslSocketFactory.getSupportedCipherSuites()).thenReturn(supportedCipherSuites);
        mSscHttpsConnection.mSscSocketFactory.mSocketFactory = mMockSslSocketFactory;

        String[] cipherSuites = mSscHttpsConnection.mSscSocketFactory.getSupportedCipherSuites();

        verify(mMockSslSocketFactory).getSupportedCipherSuites();
        assertEquals(supportedCipherSuites, cipherSuites);
    }

    @Test
    @SmallTest
    public void sscSslSocketFactory_createSocketWithHost() throws IOException {
        String host = "host.com";
        int port  = 80;
        when(mMockSslSocketFactory.createSocket(host, port)).thenReturn(mMockSslSocket);
        mSscHttpsConnection.mSscSocketFactory.mSocketFactory = mMockSslSocketFactory;

        Socket socket = mSscHttpsConnection.mSscSocketFactory.createSocket(host, port);

        verify(mMockSslSocketFactory).createSocket(host, port);
        verify(mMockSslSocket).setEnabledProtocols(any());
        assertEquals(mMockSslSocket, socket);
    }

    @Test
    @SmallTest
    public void sscSslSocketFactory_createSocketWithAddress() throws IOException {
        int port  = 80;
        when(mMockSslSocketFactory.createSocket(mMockHostAddr, port)).thenReturn(mMockSslSocket);
        mSscHttpsConnection.mSscSocketFactory.mSocketFactory = mMockSslSocketFactory;

        Socket socket = mSscHttpsConnection.mSscSocketFactory.createSocket(mMockHostAddr, port);

        verify(mMockSslSocketFactory).createSocket(mMockHostAddr, port);
        verify(mMockSslSocket).setEnabledProtocols(any());
        assertEquals(mMockSslSocket, socket);
    }

    @Test
    @SmallTest
    public void sscSslSocketFactory_createSocketWithHostAndClientAddress() throws IOException {
        String host = "host.com";
        int port  = 80;
        int clientPort = 123;
        when(mMockSslSocketFactory.createSocket(host, port, mMockClientAddr, clientPort))
                .thenReturn(mMockSslSocket);
        mSscHttpsConnection.mSscSocketFactory.mSocketFactory = mMockSslSocketFactory;

        Socket socket = mSscHttpsConnection.mSscSocketFactory
                .createSocket(host, port, mMockClientAddr, clientPort);

        verify(mMockSslSocketFactory).createSocket(host, port, mMockClientAddr, clientPort);
        verify(mMockSslSocket).setEnabledProtocols(any());
        assertEquals(mMockSslSocket, socket);
    }

    @Test
    @SmallTest
    public void sscSslSocketFactory_createSocketWithHostAddressAndClientAddress()
            throws IOException {
        int port  = 80;
        int clientPort = 123;
        when(mMockSslSocketFactory.createSocket(mMockHostAddr, port, mMockClientAddr, clientPort))
                .thenReturn(mMockSslSocket);
        mSscHttpsConnection.mSscSocketFactory.mSocketFactory = mMockSslSocketFactory;

        Socket socket = mSscHttpsConnection.mSscSocketFactory
                .createSocket(mMockHostAddr, port, mMockClientAddr, clientPort);

        verify(mMockSslSocketFactory)
                .createSocket(mMockHostAddr, port, mMockClientAddr, clientPort);
        verify(mMockSslSocket).setEnabledProtocols(any());
        assertEquals(mMockSslSocket, socket);
    }

    @Test
    @SmallTest
    public void sscSslSocketFactory_createSocketWithSocketAndHost()
            throws IOException {
        Socket oldSocket = new Socket();
        String host = "host.com";
        int port  = 80;
        when(mMockSslSocketFactory.createSocket(oldSocket, host, port, true))
                .thenReturn(mMockSslSocket);
        mSscHttpsConnection.mSscSocketFactory.mSocketFactory = mMockSslSocketFactory;

        Socket socket = mSscHttpsConnection.mSscSocketFactory
                .createSocket(oldSocket, host, port, true);

        verify(mMockSslSocketFactory).createSocket(oldSocket, host, port, true);
        verify(mMockSslSocket).setEnabledProtocols(any());
        assertEquals(mMockSslSocket, socket);
    }
}
