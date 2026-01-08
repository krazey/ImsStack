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
import android.telephony.TelephonyManager;

import androidx.test.filters.SmallTest;

import com.android.imsstack.core.agents.ConfigInterface;
import com.android.imsstack.core.agents.ImsRadioInterface;
import com.android.imsstack.core.agents.SimInterface;
import com.android.imsstack.core.agents.SubsInfoInterface;
import com.android.imsstack.core.agents.TelephonyInterface;
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
    @Mock private ConfigInterface mMockConfigInterface;
    @Mock private SimInterface mMockSimInterface;
    @Mock private SubsInfoInterface mMockSubsInfoInterface;
    @Mock private TelephonyInterface mMockTelephonyInterface;

    @Before
    public void setup() {
        MockitoAnnotations.initMocks(this);

        when(mMockConfigInterface.getCarrierConfig()).thenReturn(mMockCarrierConfig);
        SscConfig.setConfigInterface(SLOT_0, mMockConfigInterface);

        mSscUtils = new FakeSscUtils();
    }

    @After
    public void tearDown() {
        SscConfig.clear(SLOT_0);
    }

    @Test
    @SmallTest
    public void getTelephonySimType_subsInfoIsNull() {
        mMockSubsInfoInterface = null;

        int simType = mSscUtils.getTelephonySimType(SLOT_0);

        assertEquals(SscConstant.APPTYPE_ISIM, simType);
    }

    @Test
    @SmallTest
    public void getTelephonySimType_isim() {
        when(mMockSubsInfoInterface.isIsimEnabled()).thenReturn(true);

        int simType = mSscUtils.getTelephonySimType(SLOT_0);

        verify(mMockSubsInfoInterface).isIsimEnabled();
        assertEquals(SscConstant.APPTYPE_ISIM, simType);
    }

    @Test
    @SmallTest
    public void getTelephonySimType_usim() {
        when(mMockSubsInfoInterface.isIsimEnabled()).thenReturn(false);

        int simType = mSscUtils.getTelephonySimType(SLOT_0);

        verify(mMockSubsInfoInterface).isIsimEnabled();
        assertEquals(SscConstant.APPTYPE_USIM, simType);
    }

    @Test
    @SmallTest
    public void getImpi_simInterfaceIsNull() {
        mMockSimInterface = null;

        String impi = mSscUtils.getImpi(SLOT_0);

        assertNull(impi);
    }

    @Test
    @SmallTest
    public void getImpi_impiIsNull() {
        when(mMockSimInterface.getIsimImpi()).thenReturn(null);

        String impi = mSscUtils.getImpi(SLOT_0);

        assertNull(impi);
    }

    @Test
    @SmallTest
    public void getImpi_normalBehavior() {
        when(mMockSimInterface.getIsimImpi()).thenReturn("impi@operator.com");

        String impi = mSscUtils.getImpi(SLOT_0);

        verify(mMockSimInterface).getIsimImpi();
        assertEquals("impi@operator.com", impi);
    }

    @Test
    @SmallTest
    public void getImpu_simInterfaceIsNull() {
        mMockSimInterface = null;

        String impu = mSscUtils.getImpu(SLOT_0);

        assertNull(impu);
    }

    @Test
    @SmallTest
    public void getImpu_impuIsEmpty() {
        when(mMockSimInterface.getIsimImpu()).thenReturn(new ArrayList<String>());

        String impu = mSscUtils.getImpu(SLOT_0);

        assertNull(impu);
    }

    @Test
    @SmallTest
    public void getImpu_normalBehavior() {
        ArrayList<String> impuList = new ArrayList<>();
        impuList.add("impu@operator.com");
        when(mMockSimInterface.getIsimImpu()).thenReturn(impuList);

        String impu = mSscUtils.getImpu(SLOT_0);

        assertEquals("impu@operator.com", impu);
    }

    @Test
    @SmallTest
    public void getDomain_subsInfoInterfaceIsNull() {
        mMockSubsInfoInterface = null;

        String domain = mSscUtils.getDomain(SLOT_0, false);

        assertNull(domain);
    }

    @Test
    @SmallTest
    public void getDomain_fromIsimWhenImpiIsNull() {
        when(mMockSubsInfoInterface.isIsimEnabled()).thenReturn(true);
        when(mMockSimInterface.getIsimImpi()).thenReturn(null);
        when(mMockTelephonyInterface.getSimMnc()).thenReturn("01");
        when(mMockTelephonyInterface.getSimMcc()).thenReturn("001");

        String domain = mSscUtils.getDomain(SLOT_0, false);

        assertEquals("ims.mnc001.mcc001.3gppnetwork.org", domain);
    }

    @Test
    @SmallTest
    public void getDomain_fromUsimWhenTelephonySubscriberIsNull() {
        mMockTelephonyInterface = null;
        when(mMockSubsInfoInterface.isIsimEnabled()).thenReturn(false);

        String domain = mSscUtils.getDomain(SLOT_0, false);

        assertNull(domain);
    }

    @Test
    @SmallTest
    public void getDomain_fromUsimWhenMncIsNull() {
        when(mMockSubsInfoInterface.isIsimEnabled()).thenReturn(false);
        when(mMockTelephonyInterface.getSimMnc()).thenReturn(null);
        when(mMockTelephonyInterface.getSimMcc()).thenReturn("001");

        String domain = mSscUtils.getDomain(SLOT_0, false);

        assertNull(domain);
    }

    @Test
    @SmallTest
    public void getDomain_fromUsimWhenMccIsNull() {
        when(mMockSubsInfoInterface.isIsimEnabled()).thenReturn(false);
        when(mMockTelephonyInterface.getSimMnc()).thenReturn("001");
        when(mMockTelephonyInterface.getSimMcc()).thenReturn(null);

        String domain = mSscUtils.getDomain(SLOT_0, false);

        assertNull(domain);
    }

    @Test
    @SmallTest
    public void getDomain_fromUsimWhenNumberFormatException() {
        when(mMockSubsInfoInterface.isIsimEnabled()).thenReturn(false);
        when(mMockTelephonyInterface.getSimMnc()).thenReturn("abc");
        when(mMockTelephonyInterface.getSimMcc()).thenReturn("def");

        String domain = mSscUtils.getDomain(SLOT_0, false);

        assertNull(domain);
    }

    @Test
    @SmallTest
    public void getDomain_fromIsim() {
        when(mMockSubsInfoInterface.isIsimEnabled()).thenReturn(true);
        when(mMockSimInterface.getIsimImpi()).thenReturn("impi@operator.com");

        String domain = mSscUtils.getDomain(SLOT_0, false);

        assertEquals("operator.com", domain);
    }

    @Test
    @SmallTest
    public void getDomain_fromIsimForXcapRootUri() {
        when(mMockSubsInfoInterface.isIsimEnabled()).thenReturn(true);
        when(mMockSimInterface.getIsimImpi()).thenReturn("impi@operator.com");

        String domain = mSscUtils.getDomain(SLOT_0, true);

        assertEquals("xcap.operator.com", domain);
    }

    @Test
    @SmallTest
    public void getDomain_fromIsimForXcapRootUriWhenImpiContainsSpecificString() {
        when(mMockSubsInfoInterface.isIsimEnabled()).thenReturn(true);
        when(mMockSimInterface.getIsimImpi()).thenReturn("impi@3gppnetwork.org");

        String domain = mSscUtils.getDomain(SLOT_0, true);

        assertEquals("xcap.pub.3gppnetwork.org", domain);
    }

    @Test
    @SmallTest
    public void getDomain_fromUsim() {
        when(mMockSubsInfoInterface.isIsimEnabled()).thenReturn(false);
        when(mMockTelephonyInterface.getSimMnc()).thenReturn("01");
        when(mMockTelephonyInterface.getSimMcc()).thenReturn("01");

        String domain = mSscUtils.getDomain(SLOT_0, false);

        assertEquals("ims.mnc001.mcc001.3gppnetwork.org", domain);
    }

    @Test
    @SmallTest
    public void getDomain_fromUsimForXcapRootUri() {
        when(mMockSubsInfoInterface.isIsimEnabled()).thenReturn(false);
        when(mMockTelephonyInterface.getSimMnc()).thenReturn("02");
        when(mMockTelephonyInterface.getSimMcc()).thenReturn("02");

        String domain = mSscUtils.getDomain(SLOT_0, true);

        assertEquals("xcap.ims.mnc002.mcc002.pub.3gppnetwork.org", domain);
    }

    @Test
    @SmallTest
    public void getSscUserAgent_emptyUserAgent() {
        when(mMockCarrierConfig.getString(CarrierConfigManager.Ims.KEY_IMS_USER_AGENT_STRING))
            .thenReturn(null);

        String userAgent = mSscUtils.getSscUserAgent(SLOT_0);

        assertEquals("3gpp-gba", userAgent);
    }

    @Test
    @SmallTest
    public void getSscUserAgent_includingImsUserAgent() {
        when(mMockCarrierConfig.getString(CarrierConfigManager.Ims.KEY_IMS_USER_AGENT_STRING))
                .thenReturn("IMS client");

        String userAgent = mSscUtils.getSscUserAgent(SLOT_0);

        assertEquals("IMS client 3gpp-gba", userAgent);
    }

    @Test
    @SmallTest
    public void getNumberFromUri_targetUriIsNull() {
        String number = mSscUtils.getNumberFromUri(SLOT_0, null);

        assertNull(number);
    }

    @Test
    @SmallTest
    public void getNumberFromUri_fromNumber() {
        String number = mSscUtils.getNumberFromUri(SLOT_0, "+1234567890");

        assertEquals("+1234567890", number);
    }

    @Test
    @SmallTest
    public void getNumberFromUri_fromTelUri() {
        String number = mSscUtils.getNumberFromUri(SLOT_0, "tel:+1234567890");

        assertEquals("+1234567890", number);
    }

    @Test
    @SmallTest
    public void getNumberFromUri_fromTelUriIncludingPhoneContext() {
        String number = mSscUtils.getNumberFromUri(SLOT_0,
                "tel:1234567890;phone-context:operator.com");

        assertEquals("1234567890", number);
    }

    @Test
    @SmallTest
    public void getNumberFromUri_fromSipUri() {
        String number = mSscUtils.getNumberFromUri(SLOT_0, "sip:+1234567890@operator.com");

        assertEquals("+1234567890", number);
    }

    @Test
    @SmallTest
    public void getNumberFromUri_fromSipsUri() {
        String number = mSscUtils.getNumberFromUri(SLOT_0,
                "sips:+1234567890;postd=pp22@operator.com;user=phone");

        assertEquals("+1234567890", number);
    }

    @Test
    @SmallTest
    public void getNumberFromUri_fromSipUriWithoutDomain() {
        // this is a wrong URI, just used for testing
        String number = mSscUtils.getNumberFromUri(SLOT_0, "sip:+1234567890");

        assertEquals("+1234567890", number);
    }

    @Test
    @SmallTest
    public void getNumberFromUri_fromSipUriIncludingPhoneContext() {
        String number = mSscUtils.getNumberFromUri(SLOT_0,
                "sip:1234567890;phone-context:operator.com");

        assertEquals("1234567890", number);
    }

    @Test
    @SmallTest
    public void getNumberFromUri_replaceZeroWithCountrycode() {
        when(mMockCarrierConfig.getString(
                CarrierConfig.ImsSs.KEY_UT_TARGET_ADDRESS_ZERO_REPLACE_TO_COUNTRY_CODE_STRING))
                .thenReturn("+81");

        String number = mSscUtils.getNumberFromUri(SLOT_0,
                "sip:0234567890;phone-context:operator.com");

        assertEquals("+81234567890", number);
    }

    @Test
    @SmallTest
    public void getNumberFromUri_replaceCountryCodeWithZero() {
        when(mMockCarrierConfig.getString(
                CarrierConfig.ImsSs.KEY_UT_TARGET_ADDRESS_COUNTRY_CODE_REPLACE_TO_ZERO_STRING))
                .thenReturn("+81");

        String number = mSscUtils.getNumberFromUri(SLOT_0, "tel:+81234567890");

        assertEquals("0234567890", number);
    }

    @Test
    @SmallTest
    public void getUriFromNumer_numberIsNull() {
        String uri = mSscUtils.getUriFromNumber(SLOT_0, null);

        assertEquals("", uri);
    }

    @Test
    @SmallTest
    public void getUriFromNumer_domainIsNull() {
        mMockSubsInfoInterface = null; // This is to make domain null

        String uri = mSscUtils.getUriFromNumber(SLOT_0, "1234567890");

        assertEquals("1234567890", uri);
    }

    @Test
    @SmallTest
    public void getUriFromNumer_invalidCfTargetUriType() {
        final int invalidUriType = -1;
        when(mMockCarrierConfig
                .getInt(CarrierConfig.ImsSs.KEY_UT_URI_TYPE_FOR_CF_TARGET_NUMBER_INT))
                .thenReturn(invalidUriType);
        when(mMockSubsInfoInterface.isIsimEnabled()).thenReturn(true);
        when(mMockSimInterface.getIsimImpi()).thenReturn("impi@operator.com");

        String uri = mSscUtils.getUriFromNumber(SLOT_0, "1234567890");

        assertEquals("1234567890", uri);
    }

    @Test
    @SmallTest
    public void getUriFromNumer_telWithLocalNumber() {
        when(mMockCarrierConfig
                .getInt(CarrierConfig.ImsSs.KEY_UT_URI_TYPE_FOR_CF_TARGET_NUMBER_INT))
                .thenReturn(SscConfig.URI_TYPE_TEL);
        when(mMockSubsInfoInterface.isIsimEnabled()).thenReturn(true);
        when(mMockSimInterface.getIsimImpi()).thenReturn("impi@operator.com");

        String uri = mSscUtils.getUriFromNumber(SLOT_0, "1234567890");

        assertEquals("tel:1234567890;phone-context=operator.com", uri);
    }

    @Test
    @SmallTest
    public void getUriFromNumer_sipWithLocalNumber() {
        when(mMockCarrierConfig
                .getInt(CarrierConfig.ImsSs.KEY_UT_URI_TYPE_FOR_CF_TARGET_NUMBER_INT))
                .thenReturn(SscConfig.URI_TYPE_SIP);
        when(mMockSubsInfoInterface.isIsimEnabled()).thenReturn(true);
        when(mMockSimInterface.getIsimImpi()).thenReturn("impi@operator.com");

        String uri = mSscUtils.getUriFromNumber(SLOT_0, "1234567890");

        assertEquals("sip:1234567890;phone-context=operator.com@operator.com;user=phone", uri);
    }

    @Test
    @SmallTest
    public void getUriFromNumer_telWithInternationalNumber() {
        when(mMockCarrierConfig
                .getInt(CarrierConfig.ImsSs.KEY_UT_URI_TYPE_FOR_CF_TARGET_NUMBER_INT))
                .thenReturn(SscConfig.URI_TYPE_TEL);
        when(mMockSubsInfoInterface.isIsimEnabled()).thenReturn(true);
        when(mMockSimInterface.getIsimImpi()).thenReturn("impi@operator.com");

        String uri = mSscUtils.getUriFromNumber(SLOT_0, "+1234567890");

        assertEquals("tel:+1234567890", uri);
    }

    @Test
    @SmallTest
    public void getUriFromNumer_sipWithInternationalNumber() {
        when(mMockCarrierConfig
                .getInt(CarrierConfig.ImsSs.KEY_UT_URI_TYPE_FOR_CF_TARGET_NUMBER_INT))
                .thenReturn(SscConfig.URI_TYPE_SIP);
        when(mMockSubsInfoInterface.isIsimEnabled()).thenReturn(true);
        when(mMockSimInterface.getIsimImpi()).thenReturn("impi@operator.com");

        String uri = mSscUtils.getUriFromNumber(SLOT_0, "+1234567890");

        assertEquals("sip:+1234567890@operator.com;user=phone", uri);
    }

    @Test
    @SmallTest
    public void getUriFromNumer_telWithContextFromConfig() {
        when(mMockCarrierConfig.getString(
                CarrierConfig.ImsSs.KEY_UT_TARGET_ADDRESS_PHONE_CONTEXT_STRING))
                .thenReturn("contextFromConfig.com");

        String uri = mSscUtils.getUriFromNumber(SLOT_0, "1234567890");

        assertEquals("tel:1234567890;phone-context=contextFromConfig.com", uri);
    }

    @Test
    @SmallTest
    public void getUriFromNumer_replaceZeroWithCountrycode() {
        when(mMockSubsInfoInterface.isIsimEnabled()).thenReturn(true);
        when(mMockSimInterface.getIsimImpi()).thenReturn("impi@operator.com");
        when(mMockCarrierConfig.getString(
                CarrierConfig.ImsSs.KEY_UT_TARGET_ADDRESS_ZERO_REPLACE_TO_COUNTRY_CODE_STRING))
                .thenReturn("+81");

        String uri = mSscUtils.getUriFromNumber(SLOT_0, "01234567890");

        assertEquals("tel:+811234567890", uri);
    }

    @Test
    @SmallTest
    public void getUriFromNumer_replaceCountryCodeWithZero() {
        when(mMockSubsInfoInterface.isIsimEnabled()).thenReturn(true);
        when(mMockSimInterface.getIsimImpi()).thenReturn("impi@operator.com");
        when(mMockCarrierConfig.getString(
                CarrierConfig.ImsSs.KEY_UT_TARGET_ADDRESS_COUNTRY_CODE_REPLACE_TO_ZERO_STRING))
                .thenReturn("+81");

        String uri = mSscUtils.getUriFromNumber(SLOT_0, "+81234567890");

        assertEquals("tel:0234567890;phone-context=operator.com", uri);
    }

    @Test
    @SmallTest
    public void convertToImsRadioNetworkType() {
        assertEquals(ImsRadioInterface.ACCESS_NETWORK_TYPE_IWLAN,
                SscUtils.convertToImsRadioNetworkType(TelephonyManager.NETWORK_TYPE_IWLAN));

        assertEquals(ImsRadioInterface.ACCESS_NETWORK_TYPE_UTRAN,
                SscUtils.convertToImsRadioNetworkType(TelephonyManager.NETWORK_TYPE_UMTS));
        assertEquals(ImsRadioInterface.ACCESS_NETWORK_TYPE_UTRAN,
                SscUtils.convertToImsRadioNetworkType(TelephonyManager.NETWORK_TYPE_HSDPA));
        assertEquals(ImsRadioInterface.ACCESS_NETWORK_TYPE_UTRAN,
                SscUtils.convertToImsRadioNetworkType(TelephonyManager.NETWORK_TYPE_HSUPA));
        assertEquals(ImsRadioInterface.ACCESS_NETWORK_TYPE_UTRAN,
                SscUtils.convertToImsRadioNetworkType(TelephonyManager.NETWORK_TYPE_HSPA));
        assertEquals(ImsRadioInterface.ACCESS_NETWORK_TYPE_UTRAN,
                SscUtils.convertToImsRadioNetworkType(TelephonyManager.NETWORK_TYPE_HSPAP));
        assertEquals(ImsRadioInterface.ACCESS_NETWORK_TYPE_UTRAN,
                SscUtils.convertToImsRadioNetworkType(TelephonyManager.NETWORK_TYPE_TD_SCDMA));

        assertEquals(ImsRadioInterface.ACCESS_NETWORK_TYPE_EUTRAN,
                SscUtils.convertToImsRadioNetworkType(TelephonyManager.NETWORK_TYPE_LTE));

        assertEquals(ImsRadioInterface.ACCESS_NETWORK_TYPE_NGRAN,
                SscUtils.convertToImsRadioNetworkType(TelephonyManager.NETWORK_TYPE_NR));

        assertEquals(ImsRadioInterface.ACCESS_NETWORK_TYPE_UNKNOWN,
                SscUtils.convertToImsRadioNetworkType(TelephonyManager.NETWORK_TYPE_UNKNOWN));
    }

    @Test
    @SmallTest
    public void convertToAccessNetworkType() {
        assertEquals(SscConstant.NETWORK_TYPE_IWLAN,
                SscUtils.convertToAccessNetworkType(TelephonyManager.NETWORK_TYPE_IWLAN));

        assertEquals(SscConstant.NETWORK_TYPE_GERAN,
                SscUtils.convertToAccessNetworkType(TelephonyManager.NETWORK_TYPE_GPRS));
        assertEquals(SscConstant.NETWORK_TYPE_GERAN,
                SscUtils.convertToAccessNetworkType(TelephonyManager.NETWORK_TYPE_GSM));
        assertEquals(SscConstant.NETWORK_TYPE_GERAN,
                SscUtils.convertToAccessNetworkType(TelephonyManager.NETWORK_TYPE_EDGE));

        assertEquals(SscConstant.NETWORK_TYPE_UTRAN,
                SscUtils.convertToAccessNetworkType(TelephonyManager.NETWORK_TYPE_UMTS));
        assertEquals(SscConstant.NETWORK_TYPE_UTRAN,
                SscUtils.convertToAccessNetworkType(TelephonyManager.NETWORK_TYPE_HSDPA));
        assertEquals(SscConstant.NETWORK_TYPE_UTRAN,
                SscUtils.convertToAccessNetworkType(TelephonyManager.NETWORK_TYPE_HSUPA));
        assertEquals(SscConstant.NETWORK_TYPE_UTRAN,
                SscUtils.convertToAccessNetworkType(TelephonyManager.NETWORK_TYPE_HSPA));
        assertEquals(SscConstant.NETWORK_TYPE_UTRAN,
                SscUtils.convertToAccessNetworkType(TelephonyManager.NETWORK_TYPE_HSPAP));
        assertEquals(SscConstant.NETWORK_TYPE_UTRAN,
                SscUtils.convertToAccessNetworkType(TelephonyManager.NETWORK_TYPE_TD_SCDMA));

        assertEquals(SscConstant.NETWORK_TYPE_EUTRAN,
                SscUtils.convertToAccessNetworkType(TelephonyManager.NETWORK_TYPE_LTE));

        assertEquals(SscConstant.NETWORK_TYPE_NGRAN,
                SscUtils.convertToAccessNetworkType(TelephonyManager.NETWORK_TYPE_NR));

        assertEquals(SscConstant.NETWORK_TYPE_UNKNOWN,
                SscUtils.convertToAccessNetworkType(TelephonyManager.NETWORK_TYPE_UNKNOWN));
    }

    @Test
    @SmallTest
    public void getSupplementaryServiceTypeForCarrierConfig() {
        assertEquals(SscConfig.SERVICE_TYPE_OIP,
                SscUtils.getSupplementaryServiceTypeForCarrierConfig(ESsType.OIP,
                        SscConstant.CONDITION_INVALID));
        assertEquals(SscConfig.SERVICE_TYPE_OIR,
                SscUtils.getSupplementaryServiceTypeForCarrierConfig(ESsType.OIR,
                        SscConstant.CONDITION_INVALID));

        assertEquals(SscConfig.SERVICE_TYPE_TIP,
                SscUtils.getSupplementaryServiceTypeForCarrierConfig(ESsType.TIP,
                        SscConstant.CONDITION_INVALID));
        assertEquals(SscConfig.SERVICE_TYPE_TIR,
                SscUtils.getSupplementaryServiceTypeForCarrierConfig(ESsType.TIR,
                        SscConstant.CONDITION_INVALID));

        assertEquals(SscConfig.SERVICE_TYPE_CFU,
                SscUtils.getSupplementaryServiceTypeForCarrierConfig(ESsType.CF,
                        SscConstant.CONDITION_CFU));
        assertEquals(SscConfig.SERVICE_TYPE_CFB,
                SscUtils.getSupplementaryServiceTypeForCarrierConfig(ESsType.CF,
                        SscConstant.CONDITION_CFB));
        assertEquals(SscConfig.SERVICE_TYPE_CFNRY,
                SscUtils.getSupplementaryServiceTypeForCarrierConfig(ESsType.CF,
                        SscConstant.CONDITION_CFNR));
        assertEquals(SscConfig.SERVICE_TYPE_CFNRC,
                SscUtils.getSupplementaryServiceTypeForCarrierConfig(ESsType.CF,
                        SscConstant.CONDITION_CFNRC));
        assertEquals(SscConfig.SERVICE_TYPE_CFA,
                SscUtils.getSupplementaryServiceTypeForCarrierConfig(ESsType.CF,
                        SscConstant.CONDITION_CFA));
        assertEquals(SscConfig.SERVICE_TYPE_CFAC,
                SscUtils.getSupplementaryServiceTypeForCarrierConfig(ESsType.CF,
                        SscConstant.CONDITION_CFAC));
        assertEquals(SscConfig.SERVICE_TYPE_CFNL,
                SscUtils.getSupplementaryServiceTypeForCarrierConfig(ESsType.CF,
                        SscConstant.CONDITION_CFNL));
        assertEquals(SscConfig.SERVICE_TYPE_INVALID,
                SscUtils.getSupplementaryServiceTypeForCarrierConfig(ESsType.CF,
                        SscConstant.CONDITION_INVALID));

        assertEquals(SscConfig.SERVICE_TYPE_BAOC,
                SscUtils.getSupplementaryServiceTypeForCarrierConfig(ESsType.OCB,
                        SscConstant.CONDITION_BAOC));
        assertEquals(SscConfig.SERVICE_TYPE_BOIC,
                SscUtils.getSupplementaryServiceTypeForCarrierConfig(ESsType.OCB,
                        SscConstant.CONDITION_BOIC));
        assertEquals(SscConfig.SERVICE_TYPE_BOIC_EXHC,
                SscUtils.getSupplementaryServiceTypeForCarrierConfig(ESsType.OCB,
                        SscConstant.CONDITION_BOIC_EXHC));
        assertEquals(SscConfig.SERVICE_TYPE_INVALID,
                SscUtils.getSupplementaryServiceTypeForCarrierConfig(ESsType.OCB,
                        SscConstant.CONDITION_INVALID));

        assertEquals(SscConfig.SERVICE_TYPE_BAIC,
                SscUtils.getSupplementaryServiceTypeForCarrierConfig(ESsType.ICB,
                        SscConstant.CONDITION_BAIC));
        assertEquals(SscConfig.SERVICE_TYPE_BIC_ROAM,
                SscUtils.getSupplementaryServiceTypeForCarrierConfig(ESsType.ICB,
                        SscConstant.CONDITION_BIC_WR));
        assertEquals(SscConfig.SERVICE_TYPE_ACR,
                SscUtils.getSupplementaryServiceTypeForCarrierConfig(ESsType.ICB,
                        SscConstant.CONDITION_ACR));
        assertEquals(SscConfig.SERVICE_TYPE_INVALID,
                SscUtils.getSupplementaryServiceTypeForCarrierConfig(ESsType.ICB,
                        SscConstant.CONDITION_INVALID));

        assertEquals(SscConfig.SERVICE_TYPE_CW,
                SscUtils.getSupplementaryServiceTypeForCarrierConfig(ESsType.CW,
                        SscConstant.CONDITION_INVALID));

        assertEquals(SscConfig.SERVICE_TYPE_INVALID,
                SscUtils.getSupplementaryServiceTypeForCarrierConfig(ESsType.NONE,
                        SscConstant.CONDITION_INVALID));
    }

    @Test
    @SmallTest
    public void testGetConditionFromSsType() {
        // Test Call Forwarding service types.
        assertEquals(SscConstant.CONDITION_CFU,
                SscUtils.getConditionFromSsType(SscConfig.SERVICE_TYPE_CFU));
        assertEquals(SscConstant.CONDITION_CFB,
                SscUtils.getConditionFromSsType(SscConfig.SERVICE_TYPE_CFB));
        assertEquals(SscConstant.CONDITION_CFNR,
                SscUtils.getConditionFromSsType(SscConfig.SERVICE_TYPE_CFNRY));
        assertEquals(SscConstant.CONDITION_CFNRC,
                SscUtils.getConditionFromSsType(SscConfig.SERVICE_TYPE_CFNRC));
        assertEquals(SscConstant.CONDITION_CFNL,
                SscUtils.getConditionFromSsType(SscConfig.SERVICE_TYPE_CFNL));

        // Test Call Barring service types.
        assertEquals(SscConstant.CONDITION_BAOC,
                SscUtils.getConditionFromSsType(SscConfig.SERVICE_TYPE_BAOC));
        assertEquals(SscConstant.CONDITION_BOIC,
                SscUtils.getConditionFromSsType(SscConfig.SERVICE_TYPE_BOIC));
        assertEquals(SscConstant.CONDITION_BOIC_EXHC,
                SscUtils.getConditionFromSsType(SscConfig.SERVICE_TYPE_BOIC_EXHC));
        assertEquals(SscConstant.CONDITION_BAIC,
                SscUtils.getConditionFromSsType(SscConfig.SERVICE_TYPE_BAIC));
        assertEquals(SscConstant.CONDITION_BIC_WR,
                SscUtils.getConditionFromSsType(SscConfig.SERVICE_TYPE_BIC_ROAM));
        assertEquals(SscConstant.CONDITION_ACR,
                SscUtils.getConditionFromSsType(SscConfig.SERVICE_TYPE_ACR));

        // Test other service types (should return CONDITION_INVALID).
        assertEquals(SscConstant.CONDITION_INVALID,
                SscUtils.getConditionFromSsType(SscConfig.SERVICE_TYPE_CW));
        assertEquals(SscConstant.CONDITION_INVALID,
                SscUtils.getConditionFromSsType(SscConfig.SERVICE_TYPE_OIP));
        assertEquals(SscConstant.CONDITION_INVALID,
                SscUtils.getConditionFromSsType(SscConfig.SERVICE_TYPE_OIR));
        assertEquals(SscConstant.CONDITION_INVALID,
                SscUtils.getConditionFromSsType(SscConfig.SERVICE_TYPE_TIP));
        assertEquals(SscConstant.CONDITION_INVALID,
                SscUtils.getConditionFromSsType(SscConfig.SERVICE_TYPE_TIR));
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
        protected TelephonyInterface getTelephonyInterface(int slotId) {
            super.getTelephonyInterface(slotId);
            return mMockTelephonyInterface;
        }
    }
}
