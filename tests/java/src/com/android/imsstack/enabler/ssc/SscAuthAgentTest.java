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

import android.telephony.CarrierConfigManager;

import com.android.imsstack.core.agents.AgentFactory;
import com.android.imsstack.core.agents.ConfigInterface;
import com.android.imsstack.core.agents.SimInterface;
import com.android.imsstack.core.config.CarrierConfig;
import com.android.imsstack.util.ImsUtils;

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
    private static final String UST = "86EF112C27FE01744200FF040100001E01";
    private static final byte[] UST_BYTES = ImsUtils.hexStringToBytes(UST);
    private static final String IST = "E300";
    private static final byte[] IST_BYTES = ImsUtils.hexStringToBytes(IST);

    private ISscAuthAgent mSscAuthAgent;

    @Mock private CarrierConfig mMockCarrierConfig;
    @Mock private ConfigInterface mMockConfigInterface;
    @Mock private SimInterface mMockSimInterface;

    @Before
    public void setup() {
        MockitoAnnotations.initMocks(this);

        SscConfig.setConfigInterface(SLOT_0, mMockConfigInterface);
        when(mMockConfigInterface.getCarrierConfig()).thenReturn(mMockCarrierConfig);

        AgentFactory.getInstance().setAgent(SimInterface.class, mMockSimInterface, SLOT_0);
        when(mMockSimInterface.getUsimServiceTable()).thenReturn(UST_BYTES);
        when(mMockSimInterface.getIsimServiceTable()).thenReturn(IST_BYTES);

        mSscAuthAgent = SscAuthAgent.getInstance(SLOT_0);
    }

    @After
    public void tearDown() {
        AgentFactory.getInstance().setAgent(SimInterface.class, null, SLOT_0);
        mSscAuthAgent.setIsCredentialInfoUpdated(false);
        mSscAuthAgent.setLastSuccessfulGbaMode(SscConfig.GBA_NONE);
        SscConfig.clear(SLOT_0);
    }

    @Test
    public void calculateResponse_normal() {
        String wwwAuthentication = "Digest realm=\"3GPP-bootstrapping-uicc@test.3gpp.com;"
                + "3GPP-bootstrapping@test.3gpp.com\","
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
    public void parse_wwwAuthenticateHeader_whenNoGbaRealm() {
        String wwwAuthentication = "Digest realm=\"3GPP@test.3gpp.com\","
                + " algorithm=\"MD5\", qop=\"auth\","
                + " nonce=\"o94MbTY+MMNkpAePG/jVd24yOzbEbJERo98ObjI7p9U=\","
                + " opaque=\"5ccc069c403ebaf9f0171e9517f40e41\"";

        mSscAuthAgent.parse(wwwAuthentication);

        assertTrue(mSscAuthAgent.isCredentialInfoUpdated());
        assertEquals("3GPP@test.3gpp.com" , mSscAuthAgent.getRealm());
        assertEquals("test.3gpp.com" , mSscAuthAgent.getNafFqdnFromRealm());
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
    public void getNafFqdnFromRealm_whenNoRealm() {
        assertNull(mSscAuthAgent.getNafFqdnFromRealm());
    }

    @Test
    public void getGbaMode_configGbaUButSimInterfaceIsNull() {
        AgentFactory.getInstance().setAgent(SimInterface.class, null, SLOT_0);
        when(mMockCarrierConfig.getInt(CarrierConfigManager.KEY_GBA_MODE_INT))
                .thenReturn(SscConfig.GBA_U);
        when(mMockSimInterface.isGbaAvailable()).thenReturn(true);

        int gbaMode = mSscAuthAgent.getGbaMode(SscConstant.APPTYPE_ISIM);

        assertEquals(SscConfig.GBA_ME, gbaMode);
    }

    @Test
    public void getGbaMode_configGbaUButIstIsNull() {
        when(mMockCarrierConfig.getInt(CarrierConfigManager.KEY_GBA_MODE_INT))
                .thenReturn(SscConfig.GBA_U);
        when(mMockSimInterface.getIsimServiceTable()).thenReturn(new byte[0]);

        int gbaMode = mSscAuthAgent.getGbaMode(SscConstant.APPTYPE_ISIM);

        assertEquals(SscConfig.GBA_U, gbaMode);
    }

    @Test
    public void getGbaMode_configGbaUButNotSupportedByIsim() {
        when(mMockCarrierConfig.getInt(CarrierConfigManager.KEY_GBA_MODE_INT))
                .thenReturn(SscConfig.GBA_U);
        when(mMockSimInterface.isGbaAvailable()).thenReturn(false);

        int gbaMode = mSscAuthAgent.getGbaMode(SscConstant.APPTYPE_ISIM);

        assertEquals(SscConfig.GBA_ME, gbaMode);
    }

    @Test
    public void getGbaMode_configGbaUAndSupportedByIsim() {
        when(mMockCarrierConfig.getInt(CarrierConfigManager.KEY_GBA_MODE_INT))
                .thenReturn(SscConfig.GBA_U);
        when(mMockSimInterface.isGbaAvailable()).thenReturn(true);

        int gbaMode = mSscAuthAgent.getGbaMode(SscConstant.APPTYPE_ISIM);

        assertEquals(SscConfig.GBA_U, gbaMode);
    }

    @Test
    public void getGbaMode_configGbaUButUstIsNull() {
        when(mMockCarrierConfig.getInt(CarrierConfigManager.KEY_GBA_MODE_INT))
                .thenReturn(SscConfig.GBA_U);
        when(mMockSimInterface.getUsimServiceTable()).thenReturn(new byte[0]);

        int gbaMode = mSscAuthAgent.getGbaMode(SscConstant.APPTYPE_USIM);

        assertEquals(SscConfig.GBA_U, gbaMode);
    }

    @Test
    public void getGbaMode_configGbaUAndSupportedByUsim() {
        when(mMockCarrierConfig.getInt(CarrierConfigManager.KEY_GBA_MODE_INT))
                .thenReturn(SscConfig.GBA_U);

        int gbaMode = mSscAuthAgent.getGbaMode(SscConstant.APPTYPE_USIM);

        assertEquals(SscConfig.GBA_U, gbaMode);
    }

    @Test
    public void getGbaMode_configGbaMe() {
        when(mMockCarrierConfig.getInt(CarrierConfigManager.KEY_GBA_MODE_INT))
                .thenReturn(SscConfig.GBA_ME);
        when(mMockSimInterface.isGbaAvailable()).thenReturn(true);

        int gbaMode = mSscAuthAgent.getGbaMode(SscConstant.APPTYPE_ISIM);

        assertEquals(SscConfig.GBA_ME, gbaMode);
    }

    @Test
    public void setAndGetLastSuccessfulGbaMode() {
        mSscAuthAgent.setLastSuccessfulGbaMode(SscConfig.GBA_NONE);
        assertEquals(SscConfig.GBA_NONE, mSscAuthAgent.getLastSuccessfulGbaMode());

        mSscAuthAgent.setLastSuccessfulGbaMode(SscConfig.GBA_ME);
        assertEquals(SscConfig.GBA_ME, mSscAuthAgent.getLastSuccessfulGbaMode());

        mSscAuthAgent.setLastSuccessfulGbaMode(SscConfig.GBA_U);
        assertEquals(SscConfig.GBA_U, mSscAuthAgent.getLastSuccessfulGbaMode());
    }
}
