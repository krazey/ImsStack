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
import static org.junit.Assert.fail;
import static org.mockito.Mockito.when;

import android.telephony.CarrierConfigManager;

import com.android.imsstack.core.agents.ConfigAgent;
import com.android.imsstack.core.config.CarrierConfig;
import com.android.imsstack.enabler.ssc.data.SscServiceData;
import com.android.imsstack.enabler.ssc.data.SscServiceQueryData;

import org.junit.After;
import org.junit.Before;
import org.junit.Test;
import org.junit.runner.RunWith;
import org.junit.runners.JUnit4;
import org.mockito.Mock;
import org.mockito.MockitoAnnotations;

import java.net.URL;

@RunWith(JUnit4.class)
public class SscUrlTest {
    private static final int SLOT_0 = 0;

    private FakeSscUrl mSscUrl;
    private String mXui = "tel:+1234567890";
    private String mRequestUri = "/simservs.ngn.etsi.org/users/" + mXui + "/simservs.xml";

    @Mock private CarrierConfig mMockCarrierConfig;
    @Mock private ConfigAgent mMockConfigAgent;
    @Mock private SscUtils mMockSscUtils;

    @Before
    public void setup() {
        MockitoAnnotations.initMocks(this);

        when(mMockConfigAgent.getCarrierConfig()).thenReturn(mMockCarrierConfig);
        SscConfig.setConfigAgent(SLOT_0, mMockConfigAgent);

        SscXmlFormat.init(SLOT_0);
        mSscUrl = new FakeSscUrl();
    }

    @After
    public void tearDown() {
        SscXmlFormat.reset(SLOT_0);
        SscConfig.clear(SLOT_0);
    }

    @Test
    public void getConnectionUrl_generatedFromConfig() {
        final String nafFqdn = "xcap.root.uri";
        final int nafPort = 123;

        when(mMockCarrierConfig.getString(CarrierConfigManager.ImsSs.KEY_UT_AS_SERVER_FQDN_STRING))
            .thenReturn(nafFqdn);
        when(mMockCarrierConfig.getInt(CarrierConfigManager.ImsSs.KEY_UT_AS_SERVER_PORT_INT))
            .thenReturn(nafPort);
        when(mMockCarrierConfig.getInt(CarrierConfigManager.ImsSs.KEY_UT_TRANSPORT_TYPE_INT))
            .thenReturn(CarrierConfigManager.Ims.PREFERRED_TRANSPORT_TLS);

        URL url = mSscUrl.getConnectionUrl(SLOT_0, mRequestUri);

        assertNotNull(url);
        assertEquals("https", url.getProtocol());
        assertEquals(nafFqdn, url.getHost());
        assertEquals(nafPort, url.getPort());
        assertEquals(mRequestUri, url.getPath());
    }

    @Test
    public void getConnectionUrl_generatedFromUicc() {
        when(mMockCarrierConfig.getString(CarrierConfigManager.ImsSs.KEY_UT_AS_SERVER_FQDN_STRING))
            .thenReturn(null);
        when(mMockCarrierConfig.getInt(CarrierConfigManager.ImsSs.KEY_UT_TRANSPORT_TYPE_INT))
            .thenReturn(CarrierConfigManager.Ims.PREFERRED_TRANSPORT_TLS);
        when(mMockSscUtils.getDomain(SLOT_0, true)).thenReturn("xcap.impi.operator.com");

        URL url = mSscUrl.getConnectionUrl(SLOT_0, mRequestUri);

        assertNotNull(url);
        assertEquals("xcap.impi.operator.com", url.getHost());
        assertEquals(443, url.getPort()); // It's because transport type is TLS
        assertEquals(mRequestUri, url.getPath());
    }

    @Test
    public void getConnectionUrl_generatedFromUiccIncludingPostfix() {
        when(mMockCarrierConfig.getString(CarrierConfigManager.ImsSs.KEY_UT_AS_SERVER_FQDN_STRING))
            .thenReturn(null);
        when(mMockCarrierConfig.getInt(CarrierConfigManager.ImsSs.KEY_UT_TRANSPORT_TYPE_INT))
            .thenReturn(CarrierConfigManager.Ims.PREFERRED_TRANSPORT_TCP);
        when(mMockSscUtils.getDomain(SLOT_0, true)).thenReturn("xcap.impi.pub.3gppnetwork.org");

        URL url = mSscUrl.getConnectionUrl(SLOT_0, mRequestUri);

        assertNotNull(url);
        assertEquals("xcap.impi.pub.3gppnetwork.org", url.getHost());
        assertEquals(80, url.getPort()); // It's because transport type isn't TLS
        assertEquals(mRequestUri, url.getPath());
    }

    @Test
    public void getQueryUri_entireXml() {
        String expectedUri = "/simservs.ngn.etsi.org/users/" + mXui + "/simservs.xml";

        SscServiceQueryData queryData = getQueryData(ESsType.NONE);
        String queryUri = mSscUrl.getQueryUri(queryData, mXui);

        assertEquals(expectedUri, queryUri);
    }

    @Test
    public void getQueryUri_cw() {
        String expectedUri = "/simservs.ngn.etsi.org/users/" + mXui + "/simservs.xml"
                + "/~~/simservs/communication-waiting";

        SscServiceQueryData queryData = getQueryData(ESsType.CW);
        String queryUri = mSscUrl.getQueryUri(queryData, mXui);

        assertEquals(expectedUri, queryUri);
    }

    @Test
    public void getQueryUri_cf() {
        String expectedUri = "/simservs.ngn.etsi.org/users/" + mXui + "/simservs.xml"
                + "/~~/simservs/communication-diversion";

        SscServiceQueryData queryData = getQueryData(ESsType.CF);
        String queryUri = mSscUrl.getQueryUri(queryData, mXui);

        assertEquals(expectedUri, queryUri);
    }

    @Test
    public void getQueryUri_icb() {
        String expectedUri = "/simservs.ngn.etsi.org/users/" + mXui + "/simservs.xml"
                + "/~~/simservs/incoming-communication-barring";

        SscServiceQueryData queryData = getQueryData(ESsType.ICB);
        String queryUri = mSscUrl.getQueryUri(queryData, mXui);

        assertEquals(expectedUri, queryUri);
    }

    @Test
    public void getQueryUri_ocb() {
        String expectedUri = "/simservs.ngn.etsi.org/users/" + mXui + "/simservs.xml"
                + "/~~/simservs/outgoing-communication-barring";

        SscServiceQueryData queryData = getQueryData(ESsType.OCB);
        String queryUri = mSscUrl.getQueryUri(queryData, mXui);

        assertEquals(expectedUri, queryUri);
    }

    @Test
    public void getQueryUri_oir() {
        String expectedUri = "/simservs.ngn.etsi.org/users/" + mXui + "/simservs.xml"
                + "/~~/simservs/originating-identity-presentation-restriction";

        SscServiceQueryData queryData = getQueryData(ESsType.OIR);
        String queryUri = mSscUrl.getQueryUri(queryData, mXui);

        assertEquals(expectedUri, queryUri);
    }

    @Test
    public void getQueryUri_oip() {
        String expectedUri = "/simservs.ngn.etsi.org/users/" + mXui + "/simservs.xml"
                + "/~~/simservs/originating-identity-presentation";

        SscServiceQueryData queryData = getQueryData(ESsType.OIP);
        String queryUri = mSscUrl.getQueryUri(queryData, mXui);

        assertEquals(expectedUri, queryUri);
    }

    @Test
    public void getQueryUri_tir() {
        String expectedUri = "/simservs.ngn.etsi.org/users/" + mXui + "/simservs.xml"
                + "/~~/simservs/terminating-identity-presentation-restriction";

        SscServiceQueryData queryData = getQueryData(ESsType.TIR);
        String queryUri = mSscUrl.getQueryUri(queryData, mXui);

        assertEquals(expectedUri, queryUri);
    }

    @Test
    public void getQueryUri_tip() {
        String expectedUri = "/simservs.ngn.etsi.org/users/" + mXui + "/simservs.xml"
                + "/~~/simservs/terminating-identity-presentation";

        SscServiceQueryData queryData = getQueryData(ESsType.TIP);
        String queryUri = mSscUrl.getQueryUri(queryData, mXui);

        assertEquals(expectedUri, queryUri);
    }

    @Test
    public void getQueryUri_includingNamespace() {
        String expectedUri = "/simservs.ngn.etsi.org/users/" + mXui + "/simservs.xml"
                + "/~~/ss:simservs/ss:communication-diversion"
                + "?xmlns(ss=http://uri.etsi.org/ngn/params/xml/simservs/xcap)"
                + "xmlns(cp=urn:ietf:params:xml:ns:common-policy)";

        SscXmlFormat.setTag(SLOT_0, SscXmlFormat.SIMSERVS, "ss");
        SscXmlFormat.setTag(SLOT_0, SscXmlFormat.RULESET, "cp");

        SscServiceQueryData queryData = getQueryData(ESsType.CF);
        String queryUri = mSscUrl.getQueryUri(queryData, mXui);

        assertEquals(expectedUri, queryUri);
    }

    @Test
    public void getUpdateUri_cfRuleForAudio() {
        String expectedUri = "/simservs.ngn.etsi.org/users/" + mXui + "/simservs.xml"
                + "/~~/simservs/communication-diversion/ruleset/rule%5B@id=%22cfb-audio%22%5D";

        SscXmlFormat.setRuleId(SLOT_0, SscXmlFormat.MEDIA_TYPE_AUDIO, SscXmlFormat.CD,
                SscConstant.CONDITION_CFB, "cfb-audio");

        SscServiceData updateData = getUpdateData(ESsType.CF, SscConstant.ACTION_REGISTRATION,
                SscConstant.CONDITION_CFB, SscServiceClassUtil.SERVICE_CLASS_VOICE);
        String updateUri = mSscUrl.getUpdateUri(updateData, mXui);

        assertEquals(expectedUri, updateUri);
    }

    @Test
    public void getUpdateUri_cfRuleForVideo() {
        String expectedUri = "/simservs.ngn.etsi.org/users/" + mXui + "/simservs.xml"
                + "/~~/simservs/communication-diversion/ruleset/rule%5B@id=%22cfb-video%22%5D";

        SscXmlFormat.setRuleId(SLOT_0, SscXmlFormat.MEDIA_TYPE_VIDEO, SscXmlFormat.CD,
                SscConstant.CONDITION_CFB, "cfb-video");

        SscServiceData updateData = getUpdateData(ESsType.CF, SscConstant.ACTION_REGISTRATION,
                SscConstant.CONDITION_CFB, SscServiceClassUtil.SERVICE_CLASS_VIDEO);
        String updateUri = mSscUrl.getUpdateUri(updateData, mXui);

        assertEquals(expectedUri, updateUri);
    }

    @Test
    public void getUpdateUri_cfRulAndCondition() {
        String expectedUri = "/simservs.ngn.etsi.org/users/" + mXui + "/simservs.xml"
                + "/~~/simservs/communication-diversion/ruleset/rule%5B@id=%22cfu%22%5D"
                + "/conditions";

        SscXmlFormat.setRuleId(SLOT_0, SscXmlFormat.MEDIA_TYPE_AUDIO, SscXmlFormat.CD,
                SscConstant.CONDITION_CFU, "cfu");

        SscServiceData updateData = getUpdateData(ESsType.CF, SscConstant.ACTION_ACTIVATION,
                SscConstant.CONDITION_CFU, SscServiceClassUtil.SERVICE_CLASS_NONE);
        String updateUri = mSscUrl.getUpdateUri(updateData, mXui);

        assertEquals(expectedUri, updateUri);
    }

    @Test
    public void getUpdateUri_cfnrTimer() {
        String expectedUri = "/simservs.ngn.etsi.org/users/" + mXui + "/simservs.xml"
                + "/~~/simservs/communication-diversion/NoReplyTimer";

        SscServiceData updateData = getUpdateData(ESsType.CF, SscConstant.ACTION_ACTIVATION,
                SscConstant.CONDITION_CFNR_TIMER, SscServiceClassUtil.SERVICE_CLASS_NONE);
        String updateUri = mSscUrl.getUpdateUri(updateData, mXui);

        assertEquals(expectedUri, updateUri);
    }

    @Test
    public void getUpdateUri_cfnrTimerWhenOmitted() {
        String expectedUri = "/simservs.ngn.etsi.org/users/" + mXui + "/simservs.xml"
                + "/~~/simservs/communication-diversion";

        SscXmlFormat.setIsNoReplyTimerOmitted(SLOT_0, true);

        SscServiceData updateData = getUpdateData(ESsType.CF, SscConstant.ACTION_ACTIVATION,
                SscConstant.CONDITION_CFNR_TIMER, SscServiceClassUtil.SERVICE_CLASS_NONE);
        String updateUri = mSscUrl.getUpdateUri(updateData, mXui);

        assertEquals(expectedUri, updateUri);
    }

    @Test
    public void getUpdateUri_icb() {
        String expectedUri = "/simservs.ngn.etsi.org/users/" + mXui + "/simservs.xml"
                + "/~~/simservs/incoming-communication-barring/ruleset"
                + "/rule%5B@id=%22all-incoming%22%5D";

        SscXmlFormat.setRuleId(SLOT_0, SscXmlFormat.MEDIA_TYPE_AUDIO, SscXmlFormat.ICB,
                SscConstant.CONDITION_BAIC, "all-incoming");

        SscServiceData updateData = getUpdateData(ESsType.ICB, SscConstant.ACTION_REGISTRATION,
                SscConstant.CONDITION_BAIC, SscServiceClassUtil.SERVICE_CLASS_NONE);
        String updateUri = mSscUrl.getUpdateUri(updateData, mXui);

        assertEquals(expectedUri, updateUri);
    }

    @Test
    public void getUpdateUri_ocb() {
        String expectedUri = "/simservs.ngn.etsi.org/users/" + mXui + "/simservs.xml"
                + "/~~/simservs/outgoing-communication-barring/ruleset"
                + "/rule%5B@id=%22all-outgoing%22%5D";

        SscXmlFormat.setRuleId(SLOT_0, SscXmlFormat.MEDIA_TYPE_AUDIO, SscXmlFormat.OCB,
                SscConstant.CONDITION_BAOC, "all-outgoing");

        SscServiceData updateData = getUpdateData(ESsType.OCB, SscConstant.ACTION_REGISTRATION,
                SscConstant.CONDITION_BAOC, SscServiceClassUtil.SERVICE_CLASS_NONE);
        String updateUri = mSscUrl.getUpdateUri(updateData, mXui);

        assertEquals(expectedUri, updateUri);
    }

    @Test
    public void getUpdateUri_cw() {
        String expectedUri = "/simservs.ngn.etsi.org/users/" + mXui + "/simservs.xml"
                + "/~~/simservs/communication-waiting";

        SscServiceData updateData = getUpdateData(ESsType.CW, SscConstant.ACTION_ACTIVATION,
                SscConstant.CONDITION_INVALID, SscServiceClassUtil.SERVICE_CLASS_NONE);
        String updateUri = mSscUrl.getUpdateUri(updateData, mXui);

        assertEquals(expectedUri, updateUri);
    }

    @Test
    public void getUpdateUri_oir() {
        String expectedUri = "/simservs.ngn.etsi.org/users/" + mXui + "/simservs.xml"
                + "/~~/simservs/originating-identity-presentation-restriction";

        SscServiceData updateData = getUpdateData(ESsType.OIR, SscConstant.ACTION_ACTIVATION,
                SscConstant.CONDITION_INVALID, SscServiceClassUtil.SERVICE_CLASS_NONE);
        String updateUri = mSscUrl.getUpdateUri(updateData, mXui);

        assertEquals(expectedUri, updateUri);
    }

    @Test
    public void getUpdateUri_oip() {
        String expectedUri = "/simservs.ngn.etsi.org/users/" + mXui + "/simservs.xml"
                + "/~~/simservs/originating-identity-presentation";

        SscServiceData updateData = getUpdateData(ESsType.OIP, SscConstant.ACTION_ACTIVATION,
                SscConstant.CONDITION_INVALID, SscServiceClassUtil.SERVICE_CLASS_NONE);
        String updateUri = mSscUrl.getUpdateUri(updateData, mXui);

        assertEquals(expectedUri, updateUri);
    }

    @Test
    public void getUpdateUri_tir() {
        String expectedUri = "/simservs.ngn.etsi.org/users/" + mXui + "/simservs.xml"
                + "/~~/simservs/terminating-identity-presentation-restriction";

        SscServiceData updateData = getUpdateData(ESsType.TIR, SscConstant.ACTION_ACTIVATION,
                SscConstant.CONDITION_INVALID, SscServiceClassUtil.SERVICE_CLASS_NONE);
        String updateUri = mSscUrl.getUpdateUri(updateData, mXui);

        assertEquals(expectedUri, updateUri);
    }

    @Test
    public void getUpdateUri_tip() {
        String expectedUri = "/simservs.ngn.etsi.org/users/" + mXui + "/simservs.xml"
                + "/~~/simservs/terminating-identity-presentation";

        SscServiceData updateData = getUpdateData(ESsType.TIP, SscConstant.ACTION_ACTIVATION,
                SscConstant.CONDITION_INVALID, SscServiceClassUtil.SERVICE_CLASS_NONE);
        String updateUri = mSscUrl.getUpdateUri(updateData, mXui);

        assertEquals(expectedUri, updateUri);
    }

    @Test
    public void getUpdateUri_includingNamespace() {
        String expectedUri = "/simservs.ngn.etsi.org/users/" + mXui + "/simservs.xml"
                + "/~~/ss:simservs/ss:communication-diversion/cp:ruleset/cp:rule%5B@id=%22cfu%22%5D"
                + "/cp:conditions?xmlns(ss=http://uri.etsi.org/ngn/params/xml/simservs/xcap)"
                + "xmlns(cp=urn:ietf:params:xml:ns:common-policy)";

        SscXmlFormat.setTag(SLOT_0, SscXmlFormat.SIMSERVS, "ss");
        SscXmlFormat.setTag(SLOT_0, SscXmlFormat.RULESET, "cp");
        SscXmlFormat.setRuleId(SLOT_0, SscXmlFormat.MEDIA_TYPE_AUDIO, SscXmlFormat.CD,
                SscConstant.CONDITION_CFU, "cfu");

        SscServiceData updateData = getUpdateData(ESsType.CF, SscConstant.ACTION_DEACTIVATION,
                SscConstant.CONDITION_CFU, SscServiceClassUtil.SERVICE_CLASS_NONE);
        String updateUri = mSscUrl.getUpdateUri(updateData, mXui);

        assertEquals(expectedUri, updateUri);
    }

    @Test
    public void getUpdateUri_cwWithPrefix() {
        String expectedUri = "/prefxiAuid/simservs.ngn.etsi.org/users/" + mXui + "/simservs.xml"
                + "/~~/simservs/communication-waiting";
        when(mMockCarrierConfig.getString(CarrierConfig.ImsSs.KEY_XCAP_AUID_PREFIX_STRING))
            .thenReturn("/prefxiAuid");

        SscServiceData updateData = getUpdateData(ESsType.CW, SscConstant.ACTION_ACTIVATION,
                SscConstant.CONDITION_INVALID, SscServiceClassUtil.SERVICE_CLASS_NONE);
        String updateUri = mSscUrl.getUpdateUri(updateData, mXui);

        assertEquals(expectedUri, updateUri);
    }

    @Test
    public void getUpdateUri_insertCf() {
        String expectedUri = "/simservs.ngn.etsi.org/users/" + mXui + "/simservs.xml"
                + "/~~/simservs/communication-diversion";

        SscServiceData insertData = getInsertData(ESsType.CF, SscConstant.ACTION_ACTIVATION,
                SscConstant.CONDITION_CFB);
        String insertUri = mSscUrl.getUpdateUri(insertData, mXui);

        assertEquals(expectedUri, insertUri);
    }

    @Test
    public void getUpdateUri_insertIcb() {
        String expectedUri = "/simservs.ngn.etsi.org/users/" + mXui + "/simservs.xml"
                + "/~~/simservs/incoming-communication-barring";

        SscServiceData insertData = getInsertData(ESsType.ICB, SscConstant.ACTION_ACTIVATION,
                SscConstant.CONDITION_BAIC);
        String insertUri = mSscUrl.getUpdateUri(insertData, mXui);

        assertEquals(expectedUri, insertUri);
    }

    @Test
    public void getUpdateUri_insertOcb() {
        String expectedUri = "/simservs.ngn.etsi.org/users/" + mXui + "/simservs.xml"
                + "/~~/simservs/outgoing-communication-barring";

        SscServiceData insertData = getInsertData(ESsType.OCB, SscConstant.ACTION_ACTIVATION,
                SscConstant.CONDITION_BAOC);
        String insertUri = mSscUrl.getUpdateUri(insertData, mXui);

        assertEquals(expectedUri, insertUri);
    }

    private SscServiceQueryData getQueryData(ESsType ssType) {
        return SscXmlGovTest.createQueryData(ssType, 0, 0, SscServiceClassUtil.SERVICE_CLASS_NONE);
    }

    private SscServiceData getUpdateData(ESsType ssType, int action, int condition,
            int serviceClass) {
        return SscXmlGovTest.createUpdateData(ssType, 0, action, condition, serviceClass);
    }

    private SscServiceData getInsertData(ESsType ssType, int action, int condition) {
        return SscXmlGovTest.createInsertData(ssType, 0, action, condition, null);
    }

    private class FakeSscUrl extends SscUrl {
        @Override
        protected SscUtils getSscUtils() {
            if (super.getSscUtils() == null) {
                fail();
            }

            return mMockSscUtils;
        }
    }
}
