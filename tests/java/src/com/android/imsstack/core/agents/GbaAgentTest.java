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
import static org.junit.Assert.assertNull;
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
import android.util.Pair;

import com.android.imsstack.ContextFixture;
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
    private String mGbaTId = "VVVVVVVVVVVVVVVVVVVVVQ==@bsf.test.3gpp.com";
    private String mGbaKey = "U4yEyAhUBaFyqDaLxcPWT0nZc+j/5CphIns7D1G6U6M=";
    private byte[] mByteGbaKey = android.util.Base64.decode(mGbaKey, android.util.Base64.NO_WRAP);

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
            cb.onKeysAvailable(mByteGbaKey, mGbaTId);
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

        Pair<String, String> credentials = mGbaAgent.getGbaKey(appType, gbaMode, tls, nafFqdn,
                securityProtocol, forceBootStrapping);

        assertNull(credentials);
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

        Pair<String, String> credentials = mGbaAgent.getGbaKey(appType, gbaMode, tls, nafFqdn,
                securityProtocol, forceBootStrapping);

        assertNull(credentials);
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

        Pair<String, String> credentials = mGbaAgent.getGbaKey(appType, gbaMode, tls, nafFqdn,
                securityProtocol, forceBootStrapping);

        assertNull(credentials);
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

        Pair<String, String> credentials = mGbaAgent.getGbaKey(appType, gbaMode, tls, nafFqdn,
                securityProtocol, forceBootStrapping);

        assertNull(credentials);
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

        Pair<String, String> credentials = mGbaAgent.getGbaKey(appType, gbaMode, tls, nafFqdn,
                securityProtocol, forceBootStrapping);

        assertNull(credentials);
    }

    @Test
    public void getGbaKey_onKeysAvailableWithProperKey() {
        int appType = SscConstant.APPTYPE_ISIM;
        int gbaMode = SscConfig.GBA_ME;
        boolean tls = false;
        String nafFqdn = "xcap.test.3gpp.com";
        String securityProtocol = null;
        boolean forceBootStrapping = false;

        Pair<String, String> credentials = mGbaAgent.getGbaKey(appType, gbaMode, tls, nafFqdn,
                securityProtocol, forceBootStrapping);

        assertNotNull(credentials);
        assertEquals(mGbaTId, credentials.first);
        assertEquals(mGbaKey, credentials.second);
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
            cb.onKeysAvailable(null, mGbaTId);
            return null;
        }).when(mTelephonyManager).bootstrapAuthenticationRequest(anyInt(), any(), any(),
                anyBoolean(), any(), mCallbackCaptor.capture());

        Pair<String, String> credentials = mGbaAgent.getGbaKey(appType, gbaMode, tls, nafFqdn,
                securityProtocol, forceBootStrapping);

        assertNull(credentials);
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
            cb.onKeysAvailable(mByteGbaKey, null);
            return null;
        }).when(mTelephonyManager).bootstrapAuthenticationRequest(anyInt(), any(), any(),
                anyBoolean(), any(), mCallbackCaptor.capture());

        Pair<String, String> credentials = mGbaAgent.getGbaKey(appType, gbaMode, tls, nafFqdn,
                securityProtocol, forceBootStrapping);

        assertNull(credentials);
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
            cb.onAuthenticationFailure(0);
            return null;
        }).when(mTelephonyManager).bootstrapAuthenticationRequest(anyInt(), any(), any(),
                anyBoolean(), any(), mCallbackCaptor.capture());

        Pair<String, String> credentials = mGbaAgent.getGbaKey(appType, gbaMode, tls, nafFqdn,
                securityProtocol, forceBootStrapping);

        assertNull(credentials);
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
