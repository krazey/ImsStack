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
import static org.junit.Assert.assertNull;
import static org.mockito.Mockito.verify;
import static org.mockito.Mockito.when;

import android.telephony.CarrierConfigManager;

import com.android.imsstack.core.agents.ConfigAgent;
import com.android.imsstack.core.agents.SimInterface;
import com.android.imsstack.core.agents.SubsInfoInterface;
import com.android.imsstack.core.agents.agentif.ITelephonySubscriber;
import com.android.imsstack.core.config.CarrierConfig;

import org.junit.After;
import org.junit.Before;
import org.junit.Test;
import org.junit.runner.RunWith;
import org.junit.runners.JUnit4;
import org.mockito.Mock;
import org.mockito.MockitoAnnotations;

import java.util.ArrayList;

@RunWith(JUnit4.class)
public class SscUtilsTest {
    private static final int SLOT_0 = 0;

    private FakeSscUtils mSscUtils;

    @Mock private CarrierConfig mMockCarrierConfig;
    @Mock private ConfigAgent mMockConfigAgent;
    @Mock private SimInterface mMockSimInterface;
    @Mock private SubsInfoInterface mMockSubsInfoInterface;
    @Mock private ITelephonySubscriber mMockTelephonySubscriber;

    @Before
    public void setup() {
        MockitoAnnotations.initMocks(this);

        mSscUtils = new FakeSscUtils();
    }

    @After
    public void tearDown() {
        SscConfig.clear(SLOT_0);
    }

    @Test
    public void getTelephonySimType_subsInfoIsNull() {
        mMockSubsInfoInterface = null;

        int simType = mSscUtils.getTelephonySimType(SLOT_0);

        assertEquals(SscConstant.APPTYPE_ISIM, simType);
    }

    @Test
    public void getTelephonySimType_isim() {
        when(mMockSubsInfoInterface.isIsimEnabled()).thenReturn(true);

        int simType = mSscUtils.getTelephonySimType(SLOT_0);

        verify(mMockSubsInfoInterface).isIsimEnabled();
        assertEquals(SscConstant.APPTYPE_ISIM, simType);
    }

    @Test
    public void getTelephonySimType_usim() {
        when(mMockSubsInfoInterface.isIsimEnabled()).thenReturn(false);

        int simType = mSscUtils.getTelephonySimType(SLOT_0);

        verify(mMockSubsInfoInterface).isIsimEnabled();
        assertEquals(SscConstant.APPTYPE_USIM, simType);
    }

    @Test
    public void getImpi_simInterfaceIsNull() {
        mMockSimInterface = null;

        String impi = mSscUtils.getImpi(SLOT_0);

        assertNull(impi);
    }

    @Test
    public void getImpi_impiIsNull() {
        when(mMockSimInterface.getIsimImpi()).thenReturn(null);

        String impi = mSscUtils.getImpi(SLOT_0);

        assertNull(impi);
    }

    @Test
    public void getImpi_normalBehavior() {
        when(mMockSimInterface.getIsimImpi()).thenReturn("impi@operator.com");

        String impi = mSscUtils.getImpi(SLOT_0);

        verify(mMockSimInterface).getIsimImpi();
        assertEquals("impi@operator.com", impi);
    }

    @Test
    public void getImpu_simInterfaceIsNull() {
        mMockSimInterface = null;

        String impu = mSscUtils.getImpu(SLOT_0);

        assertNull(impu);
    }

    @Test
    public void getImpu_impuIsEmpty() {
        when(mMockSimInterface.getIsimImpu()).thenReturn(new ArrayList<String>());

        String impu = mSscUtils.getImpu(SLOT_0);

        assertNull(impu);
    }

    @Test
    public void getImpu_normalBehavior() {
        ArrayList<String> impuList = new ArrayList<>();
        impuList.add("impu@operator.com");
        when(mMockSimInterface.getIsimImpu()).thenReturn(impuList);

        String impu = mSscUtils.getImpu(SLOT_0);

        assertEquals("impu@operator.com", impu);
    }

    @Test
    public void getDomain_subsInfoInterfaceIsNull() {
        mMockSubsInfoInterface = null;

        String domain = mSscUtils.getDomain(SLOT_0, false);

        assertNull(domain);
    }

    @Test
    public void getDomain_fromIsimWhenImpiIsNull() {
        when(mMockSubsInfoInterface.isIsimEnabled()).thenReturn(true);
        when(mMockSimInterface.getIsimImpi()).thenReturn(null);
        when(mMockTelephonySubscriber.getMnc(true)).thenReturn("01");
        when(mMockTelephonySubscriber.getMcc(true)).thenReturn("001");

        String domain = mSscUtils.getDomain(SLOT_0, false);

        assertEquals("ims.mnc001.mcc001.3gppnetwork.org", domain);
    }

    @Test
    public void getDomain_fromUsimWhenTelephonySubscriberIsNull() {
        mMockTelephonySubscriber = null;
        when(mMockSubsInfoInterface.isIsimEnabled()).thenReturn(false);

        String domain = mSscUtils.getDomain(SLOT_0, false);

        assertNull(domain);
    }

    @Test
    public void getDomain_fromUsimWhenMncIsNull() {
        when(mMockSubsInfoInterface.isIsimEnabled()).thenReturn(false);
        when(mMockTelephonySubscriber.getMnc(true)).thenReturn(null);
        when(mMockTelephonySubscriber.getMcc(true)).thenReturn("001");

        String domain = mSscUtils.getDomain(SLOT_0, false);

        assertNull(domain);
    }

    @Test
    public void getDomain_fromUsimWhenMccIsNull() {
        when(mMockSubsInfoInterface.isIsimEnabled()).thenReturn(false);
        when(mMockTelephonySubscriber.getMnc(true)).thenReturn("001");
        when(mMockTelephonySubscriber.getMcc(true)).thenReturn(null);

        String domain = mSscUtils.getDomain(SLOT_0, false);

        assertNull(domain);
    }

    @Test
    public void getDomain_fromUsimWhenNumberFormatException() {
        when(mMockSubsInfoInterface.isIsimEnabled()).thenReturn(false);
        when(mMockTelephonySubscriber.getMnc(true)).thenReturn("abc");
        when(mMockTelephonySubscriber.getMcc(true)).thenReturn("def");

        String domain = mSscUtils.getDomain(SLOT_0, false);

        assertNull(domain);
    }

    @Test
    public void getDomain_fromIsim() {
        when(mMockSubsInfoInterface.isIsimEnabled()).thenReturn(true);
        when(mMockSimInterface.getIsimImpi()).thenReturn("impi@operator.com");

        String domain = mSscUtils.getDomain(SLOT_0, false);

        assertEquals("operator.com", domain);
    }

    @Test
    public void getDomain_fromIsimForXcapRootUri() {
        when(mMockSubsInfoInterface.isIsimEnabled()).thenReturn(true);
        when(mMockSimInterface.getIsimImpi()).thenReturn("impi@operator.com");

        String domain = mSscUtils.getDomain(SLOT_0, true);

        assertEquals("xcap.operator.com", domain);
    }

    @Test
    public void getDomain_fromIsimForXcapRootUriWhenImpiContainsSpecificString() {
        when(mMockSubsInfoInterface.isIsimEnabled()).thenReturn(true);
        when(mMockSimInterface.getIsimImpi()).thenReturn("impi@3gppnetwork.org");

        String domain = mSscUtils.getDomain(SLOT_0, true);

        assertEquals("xcap.pub.3gppnetwork.org", domain);
    }

    @Test
    public void getDomain_fromUsim() {
        when(mMockSubsInfoInterface.isIsimEnabled()).thenReturn(false);
        when(mMockTelephonySubscriber.getMnc(true)).thenReturn("01");
        when(mMockTelephonySubscriber.getMcc(true)).thenReturn("01");

        String domain = mSscUtils.getDomain(SLOT_0, false);

        assertEquals("ims.mnc001.mcc001.3gppnetwork.org", domain);
    }

    @Test
    public void getDomain_fromUsimForXcapRootUri() {
        when(mMockSubsInfoInterface.isIsimEnabled()).thenReturn(false);
        when(mMockTelephonySubscriber.getMnc(true)).thenReturn("02");
        when(mMockTelephonySubscriber.getMcc(true)).thenReturn("02");

        String domain = mSscUtils.getDomain(SLOT_0, true);

        assertEquals("xcap.ims.mnc002.mcc002.pub.3gppnetwork.org", domain);
    }

    @Test
    public void getSscUserAgent_emptyUserAgent() {
        when(mMockConfigAgent.getCarrierConfig()).thenReturn(mMockCarrierConfig);
        when(mMockCarrierConfig.getString(CarrierConfigManager.Ims.KEY_IMS_USER_AGENT_STRING))
            .thenReturn(null);
        SscConfig.setConfigAgent(SLOT_0, mMockConfigAgent);

        String userAgent = mSscUtils.getSscUserAgent(SLOT_0);

        assertEquals("3gpp-gba", userAgent);
    }

    @Test
    public void getSscUserAgent_includingImsUserAgent() {
        when(mMockConfigAgent.getCarrierConfig()).thenReturn(mMockCarrierConfig);
        when(mMockCarrierConfig.getString(CarrierConfigManager.Ims.KEY_IMS_USER_AGENT_STRING))
                .thenReturn("IMS client");
        SscConfig.setConfigAgent(SLOT_0, mMockConfigAgent);

        String userAgent = mSscUtils.getSscUserAgent(SLOT_0);

        assertEquals("IMS client 3gpp-gba", userAgent);
    }

    @Test
    public void getNumberFromUri_targetUriIsNull() {
        String number = mSscUtils.getNumberFromUri(null);

        assertNull(number);
    }

    @Test
    public void getNumberFromUri_fromNumber() {
        String number = mSscUtils.getNumberFromUri("+1234567890");

        assertEquals("+1234567890", number);
    }

    @Test
    public void getNumberFromUri_fromTelUri() {
        String number = mSscUtils.getNumberFromUri("tel:+1234567890");

        assertEquals("+1234567890", number);
    }

    @Test
    public void getNumberFromUri_fromTelUriIncludingPhoneContext() {
        String number = mSscUtils.getNumberFromUri("tel:+1234567890;phone-context:oprator.com");

        assertEquals("+1234567890", number);
    }

    @Test
    public void getNumberFromUri_fromSipUri() {
        String number = mSscUtils.getNumberFromUri("sip:+1234567890@operator.com");

        assertEquals("+1234567890", number);
    }

    @Test
    public void getNumberFromUri_fromSipUriWithoutDomain() {
        String number = mSscUtils.getNumberFromUri("sip:+1234567890");

        assertEquals("+1234567890", number);
    }

    @Test
    public void getNumberFromUri_fromSipUriIncludingPhoneContext() {
        String number = mSscUtils.getNumberFromUri("sip:+1234567890;phone-context:oprator.com");

        assertEquals("+1234567890", number);
    }

    @Test
    public void getUriFromNumer_numberIsNull() {
        String uri = mSscUtils.getUriFromNumber(SLOT_0, null);

        assertNull(uri);
    }

    @Test
    public void getUriFromNumer_domainIsNull() {
        mMockSubsInfoInterface = null; // This is to make domain null

        String uri = mSscUtils.getUriFromNumber(SLOT_0, "1234567890");

        assertEquals("1234567890", uri);
    }

    @Test
    public void getUriFromNumer_telWithLocalNumber() {
        when(mMockSubsInfoInterface.isIsimEnabled()).thenReturn(true);
        when(mMockSimInterface.getIsimImpi()).thenReturn("impi@operator.com");

        String uri = mSscUtils.getUriFromNumber(SLOT_0, "1234567890");

        assertEquals("tel:1234567890;phone-context=operator.com", uri);
    }

    @Test
    public void getUriFromNumer_telWithInternationalNumber() {
        when(mMockSubsInfoInterface.isIsimEnabled()).thenReturn(true);
        when(mMockSimInterface.getIsimImpi()).thenReturn("impi@operator.com");

        String uri = mSscUtils.getUriFromNumber(SLOT_0, "+1234567890");

        assertEquals("tel:+1234567890", uri);
    }

    @Test
    public void getUriFromNumer_telWithContextFromConfig() {
        when(mMockConfigAgent.getCarrierConfig()).thenReturn(mMockCarrierConfig);
        when(mMockCarrierConfig.getString(
                CarrierConfig.Assets.KEY_UT_TARGET_ADDRESS_PHONE_CONTEXT_STRING))
                .thenReturn("contextFromConfig.com");
        SscConfig.setConfigAgent(SLOT_0, mMockConfigAgent);

        String uri = mSscUtils.getUriFromNumber(SLOT_0, "1234567890");

        assertEquals("tel:1234567890;phone-context=contextFromConfig.com", uri);
    }

    @Test
    public void getUriFromNumer_replaceZeroWithCountrycode() {
        when(mMockSubsInfoInterface.isIsimEnabled()).thenReturn(true);
        when(mMockSimInterface.getIsimImpi()).thenReturn("impi@operator.com");
        when(mMockConfigAgent.getCarrierConfig()).thenReturn(mMockCarrierConfig);
        when(mMockCarrierConfig.getString(
                CarrierConfig.Assets.KEY_UT_TARGET_ADDRESS_ZERO_REPLACE_TO_COUNTRY_CODE_STRING))
                .thenReturn("+81");
        SscConfig.setConfigAgent(SLOT_0, mMockConfigAgent);

        String uri = mSscUtils.getUriFromNumber(SLOT_0, "01234567890");

        assertEquals("tel:+811234567890", uri);
    }

    @Test
    public void getUriFromNumer_replaceCountryCodeWithZero() {
        when(mMockSubsInfoInterface.isIsimEnabled()).thenReturn(true);
        when(mMockSimInterface.getIsimImpi()).thenReturn("impi@operator.com");
        when(mMockConfigAgent.getCarrierConfig()).thenReturn(mMockCarrierConfig);
        when(mMockCarrierConfig.getString(
                CarrierConfig.Assets.KEY_UT_TARGET_ADDRESS_COUNTRY_CODE_REPLACE_TO_ZERO_STRING))
                .thenReturn("+81");
        SscConfig.setConfigAgent(SLOT_0, mMockConfigAgent);

        String uri = mSscUtils.getUriFromNumber(SLOT_0, "+81234567890");

        assertEquals("tel:0234567890;phone-context=operator.com", uri);
    }

    private class FakeSscUtils extends SscUtils {
        @Override
        protected SimInterface getSimInterface(int slotId) {
            super.getSimInterface(slotId);
            return mMockSimInterface;
        }

        @Override
        protected SubsInfoInterface getSubsInfoInterface(int slotId) {
            super.getSubsInfoInterface(slotId);
            return mMockSubsInfoInterface;
        }

        @Override
        protected ITelephonySubscriber getTelephonySubscriber(int slotId) {
            super.getTelephonySubscriber(slotId);
            return mMockTelephonySubscriber;
        }
    }
}
