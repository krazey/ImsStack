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

package com.android.imsstack.core.agents;

import static org.junit.Assert.assertEquals;
import static org.junit.Assert.assertNotNull;
import static org.junit.Assert.fail;
import static org.mockito.ArgumentMatchers.any;
import static org.mockito.ArgumentMatchers.anyBoolean;
import static org.mockito.ArgumentMatchers.anyInt;
import static org.mockito.Mockito.doAnswer;
import static org.mockito.Mockito.verify;
import static org.mockito.Mockito.when;

import android.net.Uri;
import android.telephony.TelephonyManager;
import android.telephony.gba.UaSecurityProtocolIdentifier;

import com.android.imsstack.ContextFixture;
import com.android.imsstack.core.agents.GbaInterface.GbaCredentials;
import com.android.imsstack.enabler.ssc.SscConfig;
import com.android.imsstack.enabler.ssc.SscConstant;
import com.android.imsstack.util.AppContext;

import org.junit.After;
import org.junit.Before;
import org.junit.Test;
import org.junit.runner.RunWith;
import org.junit.runners.JUnit4;
import org.mockito.ArgumentCaptor;
import org.mockito.Captor;
import org.mockito.Mockito;
import org.mockito.MockitoAnnotations;
import org.mockito.stubbing.Answer;

import java.util.concurrent.ExecutorService;

@RunWith(JUnit4.class)
public class GbaAgentTest {
    private static final int SLOT_0 = 0;
    private static final String BOOTSTRAPPING_TRANSACTION_IDENTIFIER =
            "VVVVVVVVVVVVVVVVVVVVVQ==@bsf.test.3gpp.com";
    private static final String KS_NAF = "U4yEyAhUBaFyqDaLxcPWT0nZc+j/5CphIns7D1G6U6M=";
    private static final byte[] KS_NAF_BYTE =
            android.util.Base64.decode(KS_NAF, android.util.Base64.NO_WRAP);

    private GbaAgent mGbaAgent;
    TelephonyManager mTelephonyManager;

    @Captor ArgumentCaptor<Integer> mAppTypeCaptor;
    @Captor ArgumentCaptor<Uri> mNafUriCaptor;
    @Captor ArgumentCaptor<UaSecurityProtocolIdentifier> mUspiCaptor;
    @Captor ArgumentCaptor<Boolean> mForceBootstrappingCaptor;
    @Captor ArgumentCaptor<TelephonyManager.BootstrapAuthenticationCallback> mCallbackCaptor;

    @Before
    public void setup() {
        MockitoAnnotations.initMocks(this);

        android.content.Context context = new ContextFixture().getTestDouble();
        AppContext.init(context);
        mTelephonyManager = AppContext.getTelephonyManager();
        when(mTelephonyManager.createForSubscriptionId(anyInt())).thenReturn(mTelephonyManager);

        doAnswer((Answer<Void>) (invocation) -> {
            TelephonyManager.BootstrapAuthenticationCallback cb = mCallbackCaptor.getValue();
            assertNotNull(cb);
            cb.onKeysAvailable(KS_NAF_BYTE, BOOTSTRAPPING_TRANSACTION_IDENTIFIER);
            return null;
        }).when(mTelephonyManager).bootstrapAuthenticationRequest(anyInt(), any(), any(),
                anyBoolean(), any(), mCallbackCaptor.capture());

        mGbaAgent = new GbaAgent(SLOT_0);
        mGbaAgent.init(context);
    }

    @After
    public void tearDown() {
        mGbaAgent.cleanup();

        AppContext.deinit();
    }

    @Test
    public void cleanup_executorShutdown() {
        ExecutorService mockExecutorService = Mockito.mock(ExecutorService.class);
        replaceInstance(GbaAgent.class, "mExecutorService", mGbaAgent, mockExecutorService);

        mGbaAgent.cleanup();

        verify(mockExecutorService).shutdown();
    }

    @Test
    public void getGbaKey_bootstrapAuthenticationRequestWithIsimAndGbaDefaultAndTcp() {
        int appType = SscConstant.APPTYPE_ISIM;
        int gbaMode = 0;
        boolean tls = false;
        String nafFqdn = "xcap.test.3gpp.com";
        String securityProtocol = null;
        boolean forceBootStrapping = false;

        mGbaAgent.getGbaKey(appType, gbaMode, tls, nafFqdn, securityProtocol, forceBootStrapping);

        verify(mTelephonyManager).bootstrapAuthenticationRequest(mAppTypeCaptor.capture(),
                mNafUriCaptor.capture(), mUspiCaptor.capture(), mForceBootstrappingCaptor.capture(),
                any(ExecutorService.class),
                any(TelephonyManager.BootstrapAuthenticationCallback.class));

        int capturedAppType = mAppTypeCaptor.getValue();
        assertEquals(appType, capturedAppType);

        Uri nafUri = mNafUriCaptor.getValue();
        verifyNafUri(nafUri, gbaMode, tls, nafFqdn);

        UaSecurityProtocolIdentifier uspi = mUspiCaptor.getValue();
        assertNotNull(uspi);
        assertEquals(UaSecurityProtocolIdentifier.ORG_3GPP, uspi.getOrg());
        assertEquals(
                UaSecurityProtocolIdentifier.UA_SECURITY_PROTOCOL_3GPP_HTTP_DIGEST_AUTHENTICATION,
                uspi.getProtocol());
        assertEquals(0, uspi.getTlsCipherSuite());

        assertEquals(forceBootStrapping, mForceBootstrappingCaptor.getValue());
    }

    @Test
    public void getGbaKey_bootstrapAuthenticationRequestWithIsimAndGbaMeAndTcp() {
        int appType = SscConstant.APPTYPE_ISIM;
        int gbaMode = SscConfig.GBA_ME;
        boolean tls = false;
        String nafFqdn = "xcap.test.3gpp.com";
        String securityProtocol = null;
        boolean forceBootStrapping = false;

        mGbaAgent.getGbaKey(appType, gbaMode, tls, nafFqdn, securityProtocol, forceBootStrapping);

        verify(mTelephonyManager).bootstrapAuthenticationRequest(mAppTypeCaptor.capture(),
                mNafUriCaptor.capture(), mUspiCaptor.capture(), mForceBootstrappingCaptor.capture(),
                any(ExecutorService.class),
                any(TelephonyManager.BootstrapAuthenticationCallback.class));

        int capturedAppType = mAppTypeCaptor.getValue();
        assertEquals(appType, capturedAppType);

        Uri nafUri = mNafUriCaptor.getValue();
        verifyNafUri(nafUri, gbaMode, tls, nafFqdn);

        UaSecurityProtocolIdentifier uspi = mUspiCaptor.getValue();
        assertNotNull(uspi);
        assertEquals(UaSecurityProtocolIdentifier.ORG_3GPP, uspi.getOrg());
        assertEquals(
                UaSecurityProtocolIdentifier.UA_SECURITY_PROTOCOL_3GPP_HTTP_DIGEST_AUTHENTICATION,
                uspi.getProtocol());
        assertEquals(0, uspi.getTlsCipherSuite());

        assertEquals(forceBootStrapping, mForceBootstrappingCaptor.getValue());
    }

    @Test
    public void getGbaKey_bootstrapAuthenticationRequestWithUsimAndGbaUAndTlsNullCipherSuite() {
        int appType = SscConstant.APPTYPE_USIM;
        int gbaMode = SscConfig.GBA_U;
        boolean tls = true;
        String nafFqdn = "xcap.test.3gpp.com";
        String securityProtocol = "TLS_WRONG_MD5";
        boolean forceBootStrapping = false;

        mGbaAgent.getGbaKey(appType, gbaMode, tls, nafFqdn, securityProtocol, forceBootStrapping);

        verify(mTelephonyManager).bootstrapAuthenticationRequest(mAppTypeCaptor.capture(),
                mNafUriCaptor.capture(), mUspiCaptor.capture(), mForceBootstrappingCaptor.capture(),
                any(ExecutorService.class),
                any(TelephonyManager.BootstrapAuthenticationCallback.class));

        int capturedAppType = mAppTypeCaptor.getValue();
        assertEquals(appType, capturedAppType);

        Uri nafUri = mNafUriCaptor.getValue();
        verifyNafUri(nafUri, gbaMode, tls, nafFqdn);

        UaSecurityProtocolIdentifier uspi = mUspiCaptor.getValue();
        assertNotNull(uspi);
        assertEquals(UaSecurityProtocolIdentifier.ORG_3GPP, uspi.getOrg());
        assertEquals(UaSecurityProtocolIdentifier.UA_SECURITY_PROTOCOL_3GPP_TLS_DEFAULT,
                uspi.getProtocol());
        assertEquals(0, uspi.getTlsCipherSuite());

        assertEquals(forceBootStrapping, mForceBootstrappingCaptor.getValue());
    }

    @Test
    public void getGbaKey_bootstrapAuthenticationRequestWithUsimAndGbaDigestAndTls() {
        int appType = SscConstant.APPTYPE_USIM;
        int gbaMode = SscConfig.GBA_DIGEST;
        boolean tls = true;
        String nafFqdn = "xcap.test.3gpp.com";
        String securityProtocol = "TLS_ECDHE_PSK_WITH_AES_128_CCM_SHA256";
        boolean forceBootStrapping = true;

        mGbaAgent.getGbaKey(appType, gbaMode, tls, nafFqdn, securityProtocol, forceBootStrapping);

        verify(mTelephonyManager).bootstrapAuthenticationRequest(mAppTypeCaptor.capture(),
                mNafUriCaptor.capture(), mUspiCaptor.capture(), mForceBootstrappingCaptor.capture(),
                any(ExecutorService.class),
                any(TelephonyManager.BootstrapAuthenticationCallback.class));

        int capturedAppType = mAppTypeCaptor.getValue();
        assertEquals(appType, capturedAppType);

        Uri nafUri = mNafUriCaptor.getValue();
        verifyNafUri(nafUri, gbaMode, tls, nafFqdn);

        UaSecurityProtocolIdentifier uspi = mUspiCaptor.getValue();
        assertNotNull(uspi);
        assertEquals(UaSecurityProtocolIdentifier.ORG_3GPP, uspi.getOrg());
        assertEquals(UaSecurityProtocolIdentifier.UA_SECURITY_PROTOCOL_3GPP_TLS_DEFAULT,
                uspi.getProtocol());
        assertEquals(0xD005, uspi.getTlsCipherSuite());

        assertEquals(forceBootStrapping, mForceBootstrappingCaptor.getValue());
    }

    @Test
    public void getGbaKey_notSupportedCipherSuite() {
        int appType = SscConstant.APPTYPE_ISIM;
        int gbaMode = SscConfig.GBA_ME;
        boolean tls = true;
        String nafFqdn = "xcap.test.3gpp.com";
        String securityProtocol = "SIG_ECDSA_BRAINPOOLP512R1TLS13_SHA512";
        boolean forceBootStrapping = true;

        GbaCredentials credentials = mGbaAgent.getGbaKey(appType, gbaMode, tls, nafFqdn,
                securityProtocol, forceBootStrapping);

        assertNotNull(credentials);
        assertEquals(GbaInterface.RESULT_FAILURE, credentials.getResult());
        assertEquals(GbaInterface.GBA_FAILURE_REASON_TLS_CIPHERSUITE_NOT_SUPPORTED,
                credentials.getReason());
    }

    @Test
    public void getGbaKey_cancellationException() {
        int appType = SscConstant.APPTYPE_ISIM;
        int gbaMode = SscConfig.GBA_ME;
        boolean tls = false;
        String nafFqdn = "xcap.test.3gpp.com";
        String securityProtocol = null;
        boolean forceBootStrapping = false;

        doAnswer((Answer<Void>) (invocation) -> {
            TelephonyManager.BootstrapAuthenticationCallback cb = mCallbackCaptor.getValue();
            assertNotNull(cb);
            throw new java.util.concurrent.CancellationException();
        }).when(mTelephonyManager).bootstrapAuthenticationRequest(anyInt(), any(), any(),
                anyBoolean(), any(), mCallbackCaptor.capture());

        GbaCredentials credentials = mGbaAgent.getGbaKey(appType, gbaMode, tls, nafFqdn,
                securityProtocol, forceBootStrapping);

        assertNotNull(credentials);
        assertEquals(GbaInterface.RESULT_FAILURE, credentials.getResult());
        assertEquals(GbaInterface.GBA_FAILURE_REASON_CANCELLATION_EXCEPTION,
                credentials.getReason());
    }

    @Test
    public void getGbaKey_executionException() {
        int appType = SscConstant.APPTYPE_ISIM;
        int gbaMode = SscConfig.GBA_ME;
        boolean tls = false;
        String nafFqdn = "xcap.test.3gpp.com";
        String securityProtocol = null;
        boolean forceBootStrapping = false;

        doAnswer((Answer<Void>) (invocation) -> {
            TelephonyManager.BootstrapAuthenticationCallback cb = mCallbackCaptor.getValue();
            assertNotNull(cb);
            throw new java.util.concurrent.ExecutionException(null);
        }).when(mTelephonyManager).bootstrapAuthenticationRequest(anyInt(), any(), any(),
                anyBoolean(), any(), mCallbackCaptor.capture());

        GbaCredentials credentials = mGbaAgent.getGbaKey(appType, gbaMode, tls, nafFqdn,
                securityProtocol, forceBootStrapping);

        assertNotNull(credentials);
        assertEquals(GbaInterface.RESULT_FAILURE, credentials.getResult());
        assertEquals(GbaInterface.GBA_FAILURE_REASON_EXECUTION_EXCEPTION, credentials.getReason());
    }

    @Test
    public void getGbaKey_interruptedException() {
        int appType = SscConstant.APPTYPE_ISIM;
        int gbaMode = SscConfig.GBA_ME;
        boolean tls = false;
        String nafFqdn = "xcap.test.3gpp.com";
        String securityProtocol = null;
        boolean forceBootStrapping = false;

        doAnswer((Answer<Void>) (invocation) -> {
            TelephonyManager.BootstrapAuthenticationCallback cb = mCallbackCaptor.getValue();
            assertNotNull(cb);
            throw new InterruptedException();
        }).when(mTelephonyManager).bootstrapAuthenticationRequest(anyInt(), any(), any(),
                anyBoolean(), any(), mCallbackCaptor.capture());

        GbaCredentials credentials = mGbaAgent.getGbaKey(appType, gbaMode, tls, nafFqdn,
                securityProtocol, forceBootStrapping);

        assertNotNull(credentials);
        assertEquals(GbaInterface.RESULT_FAILURE, credentials.getResult());
        assertEquals(GbaInterface.GBA_FAILURE_REASON_INTERRUPTED_EXCEPTION,
                credentials.getReason());
    }

    @Test
    public void getGbaKey_timeoutException() {
        int appType = SscConstant.APPTYPE_ISIM;
        int gbaMode = SscConfig.GBA_ME;
        boolean tls = false;
        String nafFqdn = "xcap.test.3gpp.com";
        String securityProtocol = null;
        boolean forceBootStrapping = false;

        doAnswer((Answer<Void>) (invocation) -> {
            TelephonyManager.BootstrapAuthenticationCallback cb = mCallbackCaptor.getValue();
            assertNotNull(cb);
            throw new java.util.concurrent.TimeoutException();
        }).when(mTelephonyManager).bootstrapAuthenticationRequest(anyInt(), any(), any(),
                anyBoolean(), any(), mCallbackCaptor.capture());

        GbaCredentials credentials = mGbaAgent.getGbaKey(appType, gbaMode, tls, nafFqdn,
                securityProtocol, forceBootStrapping);

        assertNotNull(credentials);
        assertEquals(GbaInterface.RESULT_FAILURE, credentials.getResult());
        assertEquals(GbaInterface.GBA_FAILURE_REASON_TIMEOUT, credentials.getReason());
    }

    @Test
    public void getGbaKey_onKeysAvailableWithProperKey() {
        int appType = SscConstant.APPTYPE_ISIM;
        int gbaMode = SscConfig.GBA_ME;
        boolean tls = false;
        String nafFqdn = "xcap.test.3gpp.com";
        String securityProtocol = null;
        boolean forceBootStrapping = false;

        GbaCredentials credentials = mGbaAgent.getGbaKey(appType, gbaMode, tls, nafFqdn,
                securityProtocol, forceBootStrapping);

        assertNotNull(credentials);
        assertEquals(GbaInterface.RESULT_SUCCESS, credentials.getResult());
        assertEquals(BOOTSTRAPPING_TRANSACTION_IDENTIFIER, credentials.getTransactionId());
        assertEquals(KS_NAF, credentials.getKey());
    }

    @Test
    public void getGbaKey_onKeysAvailableWithNullNafKey() {
        int appType = SscConstant.APPTYPE_ISIM;
        int gbaMode = SscConfig.GBA_ME;
        boolean tls = false;
        String nafFqdn = "xcap.test.3gpp.com";
        String securityProtocol = null;
        boolean forceBootStrapping = false;
        doAnswer((Answer<Void>) (invocation) -> {
            TelephonyManager.BootstrapAuthenticationCallback cb = mCallbackCaptor.getValue();
            assertNotNull(cb);
            cb.onKeysAvailable(null, BOOTSTRAPPING_TRANSACTION_IDENTIFIER);
            return null;
        }).when(mTelephonyManager).bootstrapAuthenticationRequest(anyInt(), any(), any(),
                anyBoolean(), any(), mCallbackCaptor.capture());

        GbaCredentials credentials = mGbaAgent.getGbaKey(appType, gbaMode, tls, nafFqdn,
                securityProtocol, forceBootStrapping);

        assertNotNull(credentials);
        assertEquals(GbaInterface.RESULT_FAILURE, credentials.getResult());
        assertEquals(GbaInterface.GBA_FAILURE_REASON_KEY_INVALID, credentials.getReason());
    }

    @Test
    public void getGbaKey_onKeysAvailableWithNullTId() {
        int appType = SscConstant.APPTYPE_ISIM;
        int gbaMode = SscConfig.GBA_ME;
        boolean tls = false;
        String nafFqdn = "xcap.test.3gpp.com";
        String securityProtocol = null;
        boolean forceBootStrapping = false;
        doAnswer((Answer<Void>) (invocation) -> {
            TelephonyManager.BootstrapAuthenticationCallback cb = mCallbackCaptor.getValue();
            assertNotNull(cb);
            cb.onKeysAvailable(KS_NAF_BYTE, null);
            return null;
        }).when(mTelephonyManager).bootstrapAuthenticationRequest(anyInt(), any(), any(),
                anyBoolean(), any(), mCallbackCaptor.capture());

        GbaCredentials credentials = mGbaAgent.getGbaKey(appType, gbaMode, tls, nafFqdn,
                securityProtocol, forceBootStrapping);

        assertNotNull(credentials);
        assertEquals(GbaInterface.RESULT_FAILURE, credentials.getResult());
        assertEquals(GbaInterface.GBA_FAILURE_REASON_KEY_INVALID, credentials.getReason());
    }

    @Test
    public void getGbaKey_onAuthenticationFailure() {
        int appType = SscConstant.APPTYPE_ISIM;
        int gbaMode = SscConfig.GBA_ME;
        boolean tls = false;
        String nafFqdn = "xcap.test.3gpp.com";
        String securityProtocol = null;
        boolean forceBootStrapping = false;
        doAnswer((Answer<Void>) (invocation) -> {
            TelephonyManager.BootstrapAuthenticationCallback cb = mCallbackCaptor.getValue();
            assertNotNull(cb);
            cb.onAuthenticationFailure(TelephonyManager.GBA_FAILURE_REASON_NETWORK_FAILURE);
            return null;
        }).when(mTelephonyManager).bootstrapAuthenticationRequest(anyInt(), any(), any(),
                anyBoolean(), any(), mCallbackCaptor.capture());

        GbaCredentials credentials = mGbaAgent.getGbaKey(appType, gbaMode, tls, nafFqdn,
                securityProtocol, forceBootStrapping);

        assertNotNull(credentials);
        assertEquals(GbaInterface.RESULT_FAILURE, credentials.getResult());
        assertEquals(TelephonyManager.GBA_FAILURE_REASON_NETWORK_FAILURE, credentials.getReason());
    }

    private void verifyNafUri(Uri nafUri, int gbaMode, boolean tls, String nafFqdn) {
        assertNotNull(nafUri);

        String expectedAuthority = "@" + nafFqdn;
        if (gbaMode == SscConfig.GBA_ME) {
            expectedAuthority = "3GPP-bootstrapping" + expectedAuthority;
        } else if (gbaMode == SscConfig.GBA_U) {
            expectedAuthority = "3GPP-bootstrapping-uicc" + expectedAuthority;
        } else if (gbaMode == SscConfig.GBA_DIGEST) {
            expectedAuthority = "3GPP-bootstrapping-digest" + expectedAuthority;
        } else {
            expectedAuthority = "3GPP-bootstrapping" + expectedAuthority;
        }
        assertEquals(expectedAuthority, nafUri.getAuthority());

        if (tls) {
            assertEquals("https", nafUri.getScheme());
        } else {
            assertEquals("http", nafUri.getScheme());
        }
    }

    private synchronized void replaceInstance(final Class c, final String instanceName,
            final Object obj, final Object newValue) {
        try {
            java.lang.reflect.Field field = c.getDeclaredField(instanceName);
            field.setAccessible(true);
            field.set(obj, newValue);
        } catch (Exception e) {
            fail(e.toString());
        }
    }
}
