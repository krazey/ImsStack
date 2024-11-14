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
import static org.junit.Assert.assertFalse;
import static org.junit.Assert.assertNull;
import static org.junit.Assert.assertTrue;
import static org.mockito.Mockito.when;

import com.android.imsstack.core.agents.ConfigInterface;
import com.android.imsstack.core.config.CarrierConfig;

import org.junit.After;
import org.junit.Before;
import org.junit.Test;
import org.junit.runner.RunWith;
import org.junit.runners.JUnit4;
import org.mockito.Mock;
import org.mockito.MockitoAnnotations;

@RunWith(JUnit4.class)
public class SscAuthAgentTest {
    private static final int SLOT_0 = 0;

    private ISscAuthAgent mSscAuthAgent;

    @Mock private CarrierConfig mMockCarrierConfig;
    @Mock private ConfigInterface mMockConfigInterface;

    @Before
    public void setup() {
        MockitoAnnotations.initMocks(this);

        SscConfig.setConfigInterface(SLOT_0, mMockConfigInterface);
        when(mMockConfigInterface.getCarrierConfig()).thenReturn(mMockCarrierConfig);
        when(mMockCarrierConfig.getString(CarrierConfig.ImsSs.KEY_UT_NAF_FQDN_STRING))
                .thenReturn(null);

        mSscAuthAgent = SscAuthAgent.getInstance(SLOT_0);
    }

    @After
    public void tearDown() {
        mSscAuthAgent.setIsCredentialInfoUpdated(false);
        SscConfig.clear(SLOT_0);
    }

    @Test
    public void calculateResponse_normal() {
        String wwwAuthentication = "Digest realm=\"3GPP-bootstrapping@test.3gpp.com\","
                + " algorithm=\"MD5-sess\", qop=\"auth-int\","
                + " nonce=\"o94MbTY+MMNkpAePG/jVd24yOzbEbJERo98ObjI7p9U=\","
                + " opaque=\"5ccc069c403ebaf9f0171e9517f40e41\"";
        String uri = "/simservs.ngn.etsi.org/users/"
                + "sip:001010123456789@ims.mnc001.mcc001.3gppnetwork.org/simservs.xml";
        mSscAuthAgent.setGbaKeys("o94MbTY+MMNkpAePG/jVdw==@bsf.test.3gpp.com",
                "Jg+hMfl8v8hNj/Lqwx3ujR9HlGl4MFPWaBX518+j9Aw=");
        mSscAuthAgent.parse(wwwAuthentication);

        boolean result = mSscAuthAgent.calculateResponse("GET", uri, null);
        assertTrue(result);

        String credentials = mSscAuthAgent.getCredentialInfoString();
        assertTrue(credentials.contains("Digest"));
        assertTrue(credentials.contains("username=\"o94MbTY+MMNkpAePG/jVdw==@bsf.test.3gpp.com\""));
        assertTrue(credentials.contains("nonce=\"o94MbTY+MMNkpAePG/jVd24yOzbEbJERo98ObjI7p9U=\""));
        assertTrue(credentials.contains("realm=\"3GPP-bootstrapping@test.3gpp.com\""));
        assertTrue(credentials.contains("uri=\"" + uri + "\""));
        assertTrue(credentials.contains("qop=auth"));
        assertTrue(credentials.contains("opaque=\"5ccc069c403ebaf9f0171e9517f40e41\""));
        assertTrue(credentials.contains("algorithm=MD5"));

    }

    @Test
    public void calculateResponse_whenUserNameIsEmpty() {
        assertFalse(mSscAuthAgent.calculateResponse("GET", "xcap.3gpp.com", ""));
    }

    @Test
    public void parse_wwwAuthenticateHeader() {
        String wwwAuthentication = "Digest realm=\"3GPP-bootstrapping@test.3gpp.com\","
                + " algorithm=\"MD5\", qop=\"auth\","
                + " nonce=\"o94MbTY+MMNkpAePG/jVd24yOzbEbJERo98ObjI7p9U=\","
                + " opaque=\"5ccc069c403ebaf9f0171e9517f40e41\"";

        mSscAuthAgent.parse(wwwAuthentication);

        assertTrue(mSscAuthAgent.isCredentialInfoUpdated());
        assertEquals("3GPP-bootstrapping@test.3gpp.com" , mSscAuthAgent.getRealm());
        assertEquals("test.3gpp.com" , mSscAuthAgent.getNafFqdn());
    }

    @Test
    public void parse_wwwAuthenticateHeader_whenNafFqdnExistInConfig() {
        String wwwAuthentication = "Digest realm=\"3GPP-bootstrapping@test.3gpp.com\","
                + " algorithm=\"MD5\", qop=\"auth\","
                + " nonce=\"o94MbTY+MMNkpAePG/jVd24yOzbEbJERo98ObjI7p9U=\","
                + " opaque=\"5ccc069c403ebaf9f0171e9517f40e41\"";
        String nafFqdn = "naf.xcap.3gpp.com";
        when(mMockCarrierConfig.getString(CarrierConfig.ImsSs.KEY_UT_NAF_FQDN_STRING))
                .thenReturn(nafFqdn);

        mSscAuthAgent.parse(wwwAuthentication);

        assertTrue(mSscAuthAgent.isCredentialInfoUpdated());
        assertEquals("3GPP-bootstrapping@test.3gpp.com" , mSscAuthAgent.getRealm());
        assertEquals("naf.xcap.3gpp.com" , mSscAuthAgent.getNafFqdn());
    }

    @Test
    public void parse_authenticationInfoHeader() {
        String authenticationInfo = "authentication-info:"
                + " nextnonce =\"V2VkIEp1bCAyNyAwNDo1MDowMiAyMDIyCg==\", qop=\"auth\","
                + " rspauth=\"e058b9eaeaeef90865825e17bcee82e8\","
                + " cnonce=\"a27a9e90fa22a22b5d85796a3168b35a\", nc=00000001 \n";

        mSscAuthAgent.parse(authenticationInfo);

        String credentials = mSscAuthAgent.getCredentialInfoString();
        assertTrue(credentials.contains("nonce=\"V2VkIEp1bCAyNyAwNDo1MDowMiAyMDIyCg==\""));
    }

    @Test
    public void parse_emptyParam() {
        mSscAuthAgent.parse("");

        assertFalse(mSscAuthAgent.isCredentialInfoUpdated());
    }

    @Test
    public void setCipherSuite_normal() {
        mSscAuthAgent.setCipherSuite("TLS_NULL_WITH_NULL_NULL");

        assertEquals("TLS_NULL_WITH_NULL_NULL", mSscAuthAgent.getCipherSuite());
    }

    @Test
    public void setETag_normal() {
        mSscAuthAgent.setETag("ETag123");

        assertEquals("ETag123", mSscAuthAgent.getETag());
    }

    @Test
    public void getNafFqdn_whenNoRealm() {
        assertNull(mSscAuthAgent.getNafFqdn());
    }
}
