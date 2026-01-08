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

import com.android.imsstack.core.agents.dcmif.EApnType;
import com.android.imsstack.util.ImsLog;
import com.android.internal.annotations.VisibleForTesting;

import java.io.IOException;
import java.net.InetAddress;
import java.net.Socket;

import javax.net.ssl.HostnameVerifier;
import javax.net.ssl.HttpsURLConnection;
import javax.net.ssl.SSLContext;
import javax.net.ssl.SSLSocket;
import javax.net.ssl.SSLSocketFactory;
import javax.net.ssl.TrustManager;
import javax.net.ssl.X509TrustManager;

public class SscHttpsConnection extends SscHttpConnection {
    // Android 15 restricts the usage of TLS versions 1.0 and 1.1. These versions had
    // previously been deprecated in Android, but are now disallowed for apps targeting
    // Android 15.
    public static final String[] PROTOCOLS = { "TLSv1.2", "TLSv1.3" };

    @VisibleForTesting
    final SscSslSocketFactory mSscSocketFactory;

    public SscHttpsConnection(int slotId, EApnType apnType) {
        super(slotId, apnType);

        mSscSocketFactory = new SscSslSocketFactory();
    }

    @Override
    protected void setSocketFactory() {
        ((HttpsURLConnection) mConnection).setSSLSocketFactory(mSscSocketFactory);
    }

    @Override
    protected void setHostnameVerifier() {
        ((HttpsURLConnection) mConnection).setHostnameVerifier(DO_NOT_VERIFY);
    }

    @Override
    protected String getCipherSuiteFromConn() {
        String cipherSuite = ((HttpsURLConnection) mConnection).getCipherSuite();
        ImsLog.d("cipherSuite = " + cipherSuite);

        return cipherSuite;
    }

    @VisibleForTesting
    static final HostnameVerifier DO_NOT_VERIFY = (hostname, session) -> true;

    @VisibleForTesting
    static final class SscSslSocketFactory extends SSLSocketFactory {
        @VisibleForTesting
        SSLSocketFactory mSocketFactory = null;

        private SscSslSocketFactory() {
            init();
        }

        @Override
        public String[] getDefaultCipherSuites() {
            return mSocketFactory.getDefaultCipherSuites();
        }

        @Override
        public String[] getSupportedCipherSuites() {
            return mSocketFactory.getSupportedCipherSuites();
        }

        @Override
        public Socket createSocket(String host, int port) throws IOException {
            SSLSocket sslSocket = (SSLSocket) mSocketFactory.createSocket(host, port);

            return configureSocket(sslSocket);
        }

        @Override
        public Socket createSocket(InetAddress address, int port) throws IOException {
            SSLSocket sslSocket = (SSLSocket) mSocketFactory.createSocket(address, port);

            return configureSocket(sslSocket);
        }

        @Override
        public Socket createSocket(String host, int port, InetAddress clientAddress, int clientPort)
                throws IOException {
            SSLSocket sslSocket =
                    (SSLSocket) mSocketFactory.createSocket(host, port, clientAddress, clientPort);

            return configureSocket(sslSocket);
        }

        @Override
        public Socket createSocket(InetAddress address, int port, InetAddress clientAddress,
                int clientPort) throws IOException {
            SSLSocket sslSocket = (SSLSocket) mSocketFactory
                    .createSocket(address, port, clientAddress, clientPort);

            return configureSocket(sslSocket);
        }

        @Override
        public Socket createSocket(Socket s, String host, int port, boolean autoClose)
                throws IOException {
            SSLSocket sslSocket = (SSLSocket) mSocketFactory.createSocket(s, host, port, autoClose);

            return configureSocket(sslSocket);
        }

        private void init() {
            TrustManager[] trustAllCerts = new TrustManager[] {
                    new X509TrustManager() {
                        public java.security.cert.X509Certificate[] getAcceptedIssuers() {
                            return new java.security.cert.X509Certificate[] {};
                        }

                        public void checkClientTrusted(java.security.cert.X509Certificate[] chain,
                                String authType) throws java.security.cert.CertificateException {
                        }

                        public void checkServerTrusted(java.security.cert.X509Certificate[] chain,
                                String authType) throws java.security.cert.CertificateException {
                        }
                    }
            };

            try {
                SSLContext sc = SSLContext.getInstance("TLS");
                sc.init(null, trustAllCerts, new java.security.SecureRandom());

                mSocketFactory = sc.getSocketFactory();
            } catch (Exception e) {
                ImsLog.e(e.toString(), e);
            }
        }

        private Socket configureSocket(SSLSocket sslSocket) throws IOException {
            sslSocket.setEnabledProtocols(PROTOCOLS);
            return sslSocket;
        }
    }
}