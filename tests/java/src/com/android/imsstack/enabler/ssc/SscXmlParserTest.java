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
import static org.mockito.ArgumentMatchers.eq;
import static org.mockito.Mockito.when;

import com.android.imsstack.core.agents.ConfigInterface;
import com.android.imsstack.core.config.CarrierConfig;
import com.android.imsstack.enabler.ssc.data.CbServiceData;
import com.android.imsstack.enabler.ssc.data.CfServiceData;
import com.android.imsstack.enabler.ssc.data.CwServiceData;
import com.android.imsstack.enabler.ssc.data.ErrorResponseData;
import com.android.imsstack.enabler.ssc.data.OipServiceData;
import com.android.imsstack.enabler.ssc.data.OirServiceData;
import com.android.imsstack.enabler.ssc.data.SscRuleData;
import com.android.imsstack.enabler.ssc.data.SscServiceData;
import com.android.imsstack.enabler.ssc.data.SscServiceQueryData;
import com.android.imsstack.enabler.ssc.data.TipServiceData;
import com.android.imsstack.enabler.ssc.data.TirServiceData;

import org.junit.After;
import org.junit.Before;
import org.junit.Test;
import org.junit.runner.RunWith;
import org.junit.runners.JUnit4;
import org.mockito.Mock;
import org.mockito.MockitoAnnotations;
import org.w3c.dom.Document;

@RunWith(JUnit4.class)
public class SscXmlParserTest {
    private static final int SLOT_0 = 0;

    private SscXmlParser mSscXmlParser;
    private int mTransactionId = 1;

    @Mock private CarrierConfig mMockCarrierConfig;
    @Mock private ConfigInterface mMockConfigInterface;

    @Before
    public void setup() {
        MockitoAnnotations.initMocks(this);

        when(mMockConfigInterface.getCarrierConfig()).thenReturn(mMockCarrierConfig);
        SscConfig.setConfigInterface(SLOT_0, mMockConfigInterface);

        SscXmlFormat.init(SLOT_0);
        mSscXmlParser = new SscXmlParser();
    }

    @After
    public void tearDown() {
        SscXmlFormat.clear(SLOT_0);
    }

    @Test
    public void getSscServicefromDoc_docIsNull() {
        SscServiceQueryData queryData = getQueryData(ESsType.OIP, SscConstant.CONDITION_INVALID,
                SscServiceClassUtil.SERVICE_CLASS_NONE);
        queryData.setResponseCode(SscConstant.HTTP_OK);

        SscServiceData data = mSscXmlParser.getSscServiceFromDoc(queryData, null, null);

        assertNull(data);
    }

    @Test
    public void getSscServiceFromDoc_errorResponse() {
        SscServiceQueryData queryData = getDocumentQueryData();
        queryData.setResponseCode(SscConstant.HTTP_CONFLICT);

        ErrorResponseData data = (ErrorResponseData) mSscXmlParser.getSscServiceFromDoc(queryData,
                getErrorXmlDoc(false), null);

        assertNotNull(data);
        assertEquals(0, data.getTransactionId());
        assertNull(data.getErrorPhrase());
    }

    @Test
    public void getSscServiceFromDoc_errorResponseIncludingErrorPhrase() {
        SscServiceQueryData queryData = getDocumentQueryData();
        queryData.setResponseCode(SscConstant.HTTP_CONFLICT);

        ErrorResponseData data = (ErrorResponseData) mSscXmlParser.getSscServiceFromDoc(queryData,
                getErrorXmlDoc(true), null);

        assertNotNull(data);
        assertEquals(0, data.getTransactionId());
        assertEquals("Service setting could not be updated.", data.getErrorPhrase());
    }

    @Test
    public void getSscServiceFromDoc_entireDocumentQueryForUpdatingXmlFormat() {
        processEntireDocumentQuery();

        assertEquals(false, SscXmlFormat.getIsNoReplyTimerOmitted(SLOT_0));
        assertEquals(false, SscXmlFormat.getIsNoReplyTimerInRule(SLOT_0));
        assertEquals(true, SscXmlFormat.getCfnlExist(SLOT_0));
        assertEquals(true, SscXmlFormat.isNamespaceSsSupported(SLOT_0));
        assertEquals(true, SscXmlFormat.isNamespaceCpSupported(SLOT_0));
        assertEquals("ss:communication-waiting",
                SscXmlFormat.getSsElement(SLOT_0, SscXmlFormat.CW));
        assertEquals("ss:communication-diversion",
                SscXmlFormat.getSsElement(SLOT_0, SscXmlFormat.CD));
        assertEquals("ss:incoming-communication-barring",
                SscXmlFormat.getSsElement(SLOT_0, SscXmlFormat.ICB));
        assertEquals("ss:outgoing-communication-barring",
                SscXmlFormat.getSsElement(SLOT_0, SscXmlFormat.OCB));
        assertEquals("ss:originating-identity-presentation",
                SscXmlFormat.getSsElement(SLOT_0, SscXmlFormat.OIP));
        assertEquals("ss:originating-identity-presentation-restriction",
                SscXmlFormat.getSsElement(SLOT_0, SscXmlFormat.OIR));
        assertEquals("ss:terminating-identity-presentation",
                SscXmlFormat.getSsElement(SLOT_0, SscXmlFormat.TIP));
        assertEquals("ss:terminating-identity-presentation-restriction",
                SscXmlFormat.getSsElement(SLOT_0, SscXmlFormat.TIR));
        assertEquals("ss:busy", SscXmlFormat.getSsElement(SLOT_0, SscXmlFormat.CFB));
        assertEquals("ss:no-answer", SscXmlFormat.getSsElement(SLOT_0, SscXmlFormat.CFNR));
        assertEquals("ss:not-reachable", SscXmlFormat.getSsElement(SLOT_0, SscXmlFormat.CFNRC));
        assertEquals("ss:not-registered", SscXmlFormat.getSsElement(SLOT_0, SscXmlFormat.CFNL));
        assertEquals("cp:ruleset", SscXmlFormat.getCpElement(SLOT_0, SscXmlFormat.RULESET));
        assertEquals("cp:rule", SscXmlFormat.getCpElement(SLOT_0, SscXmlFormat.RULE));
        assertEquals("cp:conditions", SscXmlFormat.getCpElement(SLOT_0, SscXmlFormat.CONDITIONS));
        assertEquals("cp:actions", SscXmlFormat.getCpElement(SLOT_0, SscXmlFormat.ACTIONS));
        assertEquals("call-diversion-unconditional", SscXmlFormat.getRuleId(SLOT_0,
                SscXmlFormat.MEDIA_TYPE_AUDIO, SscXmlFormat.CD, SscConstant.CONDITION_CFU));
        assertEquals("call-diversion-busy", SscXmlFormat.getRuleId(SLOT_0,
                SscXmlFormat.MEDIA_TYPE_AUDIO, SscXmlFormat.CD, SscConstant.CONDITION_CFB));
        assertEquals("call-diversion-no-reply", SscXmlFormat.getRuleId(SLOT_0,
                SscXmlFormat.MEDIA_TYPE_AUDIO, SscXmlFormat.CD, SscConstant.CONDITION_CFNR));
        assertEquals("call-diversion-not-reachable", SscXmlFormat.getRuleId(SLOT_0,
                SscXmlFormat.MEDIA_TYPE_AUDIO, SscXmlFormat.CD, SscConstant.CONDITION_CFNRC));
        assertEquals("call-diversion-not-loggedin", SscXmlFormat.getRuleId(SLOT_0,
                SscXmlFormat.MEDIA_TYPE_AUDIO, SscXmlFormat.CD, SscConstant.CONDITION_CFNL));
        assertEquals("call-diversion-unconditional-video", SscXmlFormat.getRuleId(SLOT_0,
                SscXmlFormat.MEDIA_TYPE_VIDEO, SscXmlFormat.CD, SscConstant.CONDITION_CFU));
        assertEquals("call-diversion-busy-video", SscXmlFormat.getRuleId(SLOT_0,
                SscXmlFormat.MEDIA_TYPE_VIDEO, SscXmlFormat.CD, SscConstant.CONDITION_CFB));
        assertEquals("call-diversion-no-reply-video", SscXmlFormat.getRuleId(SLOT_0,
                SscXmlFormat.MEDIA_TYPE_VIDEO, SscXmlFormat.CD, SscConstant.CONDITION_CFNR));
        assertEquals("call-diversion-not-reachable-video", SscXmlFormat.getRuleId(SLOT_0,
                SscXmlFormat.MEDIA_TYPE_VIDEO, SscXmlFormat.CD, SscConstant.CONDITION_CFNRC));
        assertEquals("call-diversion-not-loggedin-video", SscXmlFormat.getRuleId(SLOT_0,
                SscXmlFormat.MEDIA_TYPE_VIDEO, SscXmlFormat.CD, SscConstant.CONDITION_CFNL));
        assertEquals("call-barring-all-incoming", SscXmlFormat.getRuleId(SLOT_0,
                SscXmlFormat.MEDIA_TYPE_AUDIO, SscXmlFormat.ICB, SscConstant.CONDITION_BAIC));
        assertEquals("call-barring-incoming-in-roaming", SscXmlFormat.getRuleId(SLOT_0,
                SscXmlFormat.MEDIA_TYPE_AUDIO, SscXmlFormat.ICB, SscConstant.CONDITION_BIC_WR));
        assertEquals("call-barring-all-outgoing", SscXmlFormat.getRuleId(SLOT_0,
                SscXmlFormat.MEDIA_TYPE_AUDIO, SscXmlFormat.OCB, SscConstant.CONDITION_BAOC));
        assertEquals("call-barring-outgoing-international", SscXmlFormat.getRuleId(SLOT_0,
                SscXmlFormat.MEDIA_TYPE_AUDIO, SscXmlFormat.OCB, SscConstant.CONDITION_BOIC));
        assertEquals("call-barring-outgoing-internationalExHC", SscXmlFormat.getRuleId(SLOT_0,
                SscXmlFormat.MEDIA_TYPE_AUDIO, SscXmlFormat.OCB, SscConstant.CONDITION_BOIC_EXHC));
    }

    @Test
    public void getSscServiceFromDoc_oip() {
        String xml = "<ss:originating-identity-presentation active=\"true\"/>";

        processEntireDocumentQuery();

        SscServiceQueryData queryData = getQueryData(ESsType.OIP, SscConstant.CONDITION_INVALID,
                SscServiceClassUtil.SERVICE_CLASS_NONE);
        queryData.setResponseCode(SscConstant.HTTP_OK);
        OipServiceData data = (OipServiceData) mSscXmlParser.getSscServiceFromDoc(queryData,
                getDocumentFromString(xml), null);

        assertNotNull(data);
        assertEquals(mTransactionId, data.getTransactionId());
        assertEquals(ESsType.OIP, data.getSsType());
        assertEquals(SscConstant.EVENT_SSC_QUERY_OIP, data.getEventNumber());
        assertEquals(SscConstant.STATUS_ENABLE, data.getState());
    }

    @Test
    public void getSscServiceFromDoc_oipWhenNoData() {
        processEntireDocumentQuery();

        SscServiceQueryData queryData = getQueryData(ESsType.OIP, SscConstant.CONDITION_INVALID,
                SscServiceClassUtil.SERVICE_CLASS_NONE);
        queryData.setResponseCode(SscConstant.HTTP_OK);
        OipServiceData data = (OipServiceData) mSscXmlParser.getSscServiceFromDoc(queryData,
                getEmptyXmlDoc(), null);

        assertNull(data);
    }

    @Test
    public void getSscServiceFromDoc_oirNotProvisioned() {
        String xml = "<ss:originating-identity-presentation-restriction active=\"false\">"
                + "<ss:default-behaviour>PRESENTATION-NOT-RESTRICTED</ss:default-behaviour>"
                + "</ss:originating-identity-presentation-restriction>";

        processEntireDocumentQuery();

        SscServiceQueryData queryData = getQueryData(ESsType.OIR, SscConstant.CONDITION_INVALID,
                SscServiceClassUtil.SERVICE_CLASS_NONE);
        queryData.setResponseCode(SscConstant.HTTP_OK);
        OirServiceData data = (OirServiceData) mSscXmlParser.getSscServiceFromDoc(queryData,
                getDocumentFromString(xml), null);

        assertNotNull(data);
        assertEquals(mTransactionId, data.getTransactionId());
        assertEquals(ESsType.OIR, data.getSsType());
        assertEquals(SscConstant.EVENT_SSC_QUERY_OIR, data.getEventNumber());
        assertEquals(SscConstant.STATUS_DISABLE, data.getState());
        assertEquals(SscConstant.OIR_NOT_PROVISIONED, data.getProvisionStatus());
        assertEquals(SscConstant.OIR_SUPPRESSION, data.getOutgoingState());
    }

    @Test
    public void getSscServiceFromDoc_oirEnabled() {
        String xml = "<ss:originating-identity-presentation-restriction active=\"true\">"
                + "<ss:default-behaviour>presentation-restricted</ss:default-behaviour>"
                + "</ss:originating-identity-presentation-restriction>";

        processEntireDocumentQuery();

        SscServiceQueryData queryData = getQueryData(ESsType.OIR, SscConstant.CONDITION_INVALID,
                SscServiceClassUtil.SERVICE_CLASS_NONE);
        queryData.setResponseCode(SscConstant.HTTP_OK);
        OirServiceData data = (OirServiceData) mSscXmlParser.getSscServiceFromDoc(queryData,
                getDocumentFromString(xml), null);

        assertNotNull(data);
        assertEquals(mTransactionId, data.getTransactionId());
        assertEquals(ESsType.OIR, data.getSsType());
        assertEquals(SscConstant.EVENT_SSC_QUERY_OIR, data.getEventNumber());
        assertEquals(SscConstant.STATUS_ENABLE, data.getState());
        assertEquals(SscConstant.OIR_TEMPORARY_MODE_PRESENTATION_RESTRICTED,
                data.getProvisionStatus());
        assertEquals(SscConstant.OIR_INVOCATION, data.getOutgoingState());
    }

    @Test
    public void getSscServiceFromDoc_oirDisabled() {
        String xml = "<ss:originating-identity-presentation-restriction active=\"true\">"
                + "<ss:default-behaviour>presentation-not-restricted</ss:default-behaviour>"
                + "</ss:originating-identity-presentation-restriction>";

        processEntireDocumentQuery();

        SscServiceQueryData queryData = getQueryData(ESsType.OIR, SscConstant.CONDITION_INVALID,
                SscServiceClassUtil.SERVICE_CLASS_NONE);
        queryData.setResponseCode(SscConstant.HTTP_OK);
        OirServiceData data = (OirServiceData) mSscXmlParser.getSscServiceFromDoc(queryData,
                getDocumentFromString(xml), null);

        assertNotNull(data);
        assertEquals(mTransactionId, data.getTransactionId());
        assertEquals(ESsType.OIR, data.getSsType());
        assertEquals(SscConstant.EVENT_SSC_QUERY_OIR, data.getEventNumber());
        assertEquals(SscConstant.STATUS_DISABLE, data.getState());
        assertEquals(SscConstant.OIR_TEMPORARY_MODE_PRESENTATION_ALLOWED,
                data.getProvisionStatus());
        assertEquals(SscConstant.OIR_SUPPRESSION, data.getOutgoingState());
    }

    @Test
    public void getSscServiceFromDoc_oirWithoutDefaultBehaviour() {
        String xml = "<ss:originating-identity-presentation-restriction active=\"true\">"
                + "</ss:originating-identity-presentation-restriction>";

        processEntireDocumentQuery();

        SscServiceQueryData queryData = getQueryData(ESsType.OIR, SscConstant.CONDITION_INVALID,
                SscServiceClassUtil.SERVICE_CLASS_NONE);
        queryData.setResponseCode(SscConstant.HTTP_OK);
        OirServiceData data = (OirServiceData) mSscXmlParser.getSscServiceFromDoc(queryData,
                getDocumentFromString(xml), null);

        assertNotNull(data);
        assertEquals(mTransactionId, data.getTransactionId());
        assertEquals(ESsType.OIR, data.getSsType());
        assertEquals(SscConstant.EVENT_SSC_QUERY_OIR, data.getEventNumber());
        assertEquals(SscConstant.STATUS_ENABLE, data.getState());
        assertEquals(SscConstant.OIR_TEMPORARY_MODE_PRESENTATION_RESTRICTED,
                data.getProvisionStatus());
        assertEquals(SscConstant.OIR_INVOCATION, data.getOutgoingState());
    }

    @Test
    public void getSscServiceFromDoc_oirWhenNoData() {
        processEntireDocumentQuery();

        SscServiceQueryData queryData = getQueryData(ESsType.OIR, SscConstant.CONDITION_INVALID,
                SscServiceClassUtil.SERVICE_CLASS_NONE);
        queryData.setResponseCode(SscConstant.HTTP_OK);
        OirServiceData data = (OirServiceData) mSscXmlParser.getSscServiceFromDoc(queryData,
                getEmptyXmlDoc(), null);

        assertNull(data);
    }

    @Test
    public void getSscServiceFromDoc_tip() {
        String xml = "<ss:terminating-identity-presentation />";

        processEntireDocumentQuery();

        SscServiceQueryData queryData = getQueryData(ESsType.TIP, SscConstant.CONDITION_INVALID,
                SscServiceClassUtil.SERVICE_CLASS_NONE);
        queryData.setResponseCode(SscConstant.HTTP_OK);
        TipServiceData data = (TipServiceData) mSscXmlParser.getSscServiceFromDoc(queryData,
                getDocumentFromString(xml), null);

        assertNotNull(data);
        assertEquals(mTransactionId, data.getTransactionId());
        assertEquals(ESsType.TIP, data.getSsType());
        assertEquals(SscConstant.EVENT_SSC_QUERY_TIP, data.getEventNumber());
        assertEquals(SscConstant.STATUS_ENABLE, data.getState());
    }

    @Test
    public void getSscServiceFromDoc_tipWhenNoData() {
        processEntireDocumentQuery();

        SscServiceQueryData queryData = getQueryData(ESsType.TIP, SscConstant.CONDITION_INVALID,
                SscServiceClassUtil.SERVICE_CLASS_NONE);
        queryData.setResponseCode(SscConstant.HTTP_OK);
        TipServiceData data = (TipServiceData) mSscXmlParser.getSscServiceFromDoc(queryData,
                getEmptyXmlDoc(), null);

        assertNull(data);
    }

    @Test
    public void getSscServiceFromDoc_tirNotProvisioned() {
        String xml = "<ss:terminating-identity-presentation-restriction active=\"false\">"
                + "<ss:default-behaviour>presentation-not-restricted</ss:default-behaviour>"
                + "</ss:terminating-identity-presentation-restriction>";

        processEntireDocumentQuery();

        SscServiceQueryData queryData = getQueryData(ESsType.TIR, SscConstant.CONDITION_INVALID,
                SscServiceClassUtil.SERVICE_CLASS_NONE);
        queryData.setResponseCode(SscConstant.HTTP_OK);
        TirServiceData data = (TirServiceData) mSscXmlParser.getSscServiceFromDoc(queryData,
                getDocumentFromString(xml), null);

        assertNotNull(data);
        assertEquals(mTransactionId, data.getTransactionId());
        assertEquals(ESsType.TIR, data.getSsType());
        assertEquals(SscConstant.EVENT_SSC_QUERY_TIR, data.getEventNumber());
        assertEquals(SscConstant.STATUS_DISABLE, data.getState());
        assertEquals(SscConstant.TIR_NOT_PROVISIONED, data.getProvisionStatus());
    }

    @Test
    public void getSscServiceFromDoc_tirEnabled() {
        String xml = "<ss:terminating-identity-presentation-restriction active=\"true\">"
                + "<ss:default-behaviour>presentation-restricted</ss:default-behaviour>"
                + "</ss:terminating-identity-presentation-restriction>";

        processEntireDocumentQuery();

        SscServiceQueryData queryData = getQueryData(ESsType.TIR, SscConstant.CONDITION_INVALID,
                SscServiceClassUtil.SERVICE_CLASS_NONE);
        queryData.setResponseCode(SscConstant.HTTP_OK);
        TirServiceData data = (TirServiceData) mSscXmlParser.getSscServiceFromDoc(queryData,
                getDocumentFromString(xml), null);

        assertNotNull(data);
        assertEquals(mTransactionId, data.getTransactionId());
        assertEquals(ESsType.TIR, data.getSsType());
        assertEquals(SscConstant.EVENT_SSC_QUERY_TIR, data.getEventNumber());
        assertEquals(SscConstant.STATUS_ENABLE, data.getState());
        assertEquals(SscConstant.TIR_PROVISIONED, data.getProvisionStatus());
    }

    @Test
    public void getSscServiceFromDoc_tirDisabled() {
        String xml = "<ss:terminating-identity-presentation-restriction active=\"true\">"
                + "<ss:default-behaviour>presentation-not-restricted</ss:default-behaviour>"
                + "</ss:terminating-identity-presentation-restriction>";

        processEntireDocumentQuery();

        SscServiceQueryData queryData = getQueryData(ESsType.TIR, SscConstant.CONDITION_INVALID,
                SscServiceClassUtil.SERVICE_CLASS_NONE);
        queryData.setResponseCode(SscConstant.HTTP_OK);
        TirServiceData data = (TirServiceData) mSscXmlParser.getSscServiceFromDoc(queryData,
                getDocumentFromString(xml), null);

        assertNotNull(data);
        assertEquals(mTransactionId, data.getTransactionId());
        assertEquals(ESsType.TIR, data.getSsType());
        assertEquals(SscConstant.EVENT_SSC_QUERY_TIR, data.getEventNumber());
        assertEquals(SscConstant.STATUS_DISABLE, data.getState());
        assertEquals(SscConstant.TIR_NOT_PROVISIONED, data.getProvisionStatus());
    }

    @Test
    public void getSscServiceFromDoc_tirWithoutDefaultBehaviour() {
        String xml = "<ss:terminating-identity-presentation-restriction active=\"true\">"
                + "</ss:terminating-identity-presentation-restriction>";

        processEntireDocumentQuery();

        SscServiceQueryData queryData = getQueryData(ESsType.TIR, SscConstant.CONDITION_INVALID,
                SscServiceClassUtil.SERVICE_CLASS_NONE);
        queryData.setResponseCode(SscConstant.HTTP_OK);
        TirServiceData data = (TirServiceData) mSscXmlParser.getSscServiceFromDoc(queryData,
                getDocumentFromString(xml), null);

        assertNotNull(data);
        assertEquals(mTransactionId, data.getTransactionId());
        assertEquals(ESsType.TIR, data.getSsType());
        assertEquals(SscConstant.EVENT_SSC_QUERY_TIR, data.getEventNumber());
        assertEquals(SscConstant.STATUS_ENABLE, data.getState());
        assertEquals(SscConstant.TIR_PROVISIONED, data.getProvisionStatus());
    }

    @Test
    public void getSscServiceFromDoc_tirWhenNoData() {
        processEntireDocumentQuery();

        SscServiceQueryData queryData = getQueryData(ESsType.TIR, SscConstant.CONDITION_INVALID,
                SscServiceClassUtil.SERVICE_CLASS_NONE);
        queryData.setResponseCode(SscConstant.HTTP_OK);
        TirServiceData data = (TirServiceData) mSscXmlParser.getSscServiceFromDoc(queryData,
                getEmptyXmlDoc(), null);

        assertNull(data);
    }

    @Test
    public void getSscServiceFromDoc_cfWhenNoMediaInXml() {
        String xml = "<ss:communication-diversion active=\"true\">"
                + "<cp:ruleset>"
                + "<cp:rule id=\"call-diversion-unconditional\">"
                + "<cp:conditions>"
                + "<ss:rule-deactivated/>"
                + "</cp:conditions>"
                + "<cp:actions>"
                + "<ss:forward-to>"
                + "<ss:target>tel:+1234567890</ss:target>"
                + "</ss:forward-to>"
                + "</cp:actions>"
                + "</cp:rule>"
                + "</cp:ruleset>"
                + "</ss:communication-diversion>";

        processEntireDocumentQuery();

        SscServiceQueryData queryData = getQueryData(ESsType.CF, SscConstant.CONDITION_CFU,
                SscServiceClassUtil.SERVICE_CLASS_NONE);
        queryData.setResponseCode(SscConstant.HTTP_OK);
        CfServiceData data = (CfServiceData) mSscXmlParser.getSscServiceFromDoc(queryData,
                getDocumentFromString(xml), null);

        assertNotNull(data);
        assertEquals(SscConstant.CONDITION_CFU, data.getCondition());
        assertEquals(mTransactionId, data.getTransactionId());
        assertEquals(ESsType.CF, data.getSsType());
        assertEquals(SscConstant.EVENT_SSC_QUERY_CF, data.getEventNumber());
        assertNotNull(data.getRuleSet());
        assertEquals(1, data.getRuleSet().size());
        SscRuleData ruleData = data.getRuleSet().get(0);
        assertEquals(SscConstant.CONDITION_CFU, ruleData.getSsCondition());
        assertEquals(SscServiceClassUtil.SERVICE_CLASS_NONE, ruleData.getServiceClass());
        assertEquals(SscConstant.STATUS_DISABLE, ruleData.getState());
    }

    @Test
    public void getSscServiceFromDoc_cfWhenXmlHasAudio() {
        String xml = "<ss:communication-diversion active=\"true\">"
                + "<cp:ruleset>"
                + "<cp:rule id=\"call-diversion-unconditional\">"
                + "<cp:conditions>"
                + "<ss:media>audio</ss:media>"
                + "</cp:conditions>"
                + "<cp:actions>"
                + "<ss:forward-to>"
                + "<ss:target>tel:+1234567890</ss:target>"
                + "</ss:forward-to>"
                + "</cp:actions>"
                + "</cp:rule>"
                + "</cp:ruleset>"
                + "</ss:communication-diversion>";

        processEntireDocumentQuery();

        SscServiceQueryData queryData = getQueryData(ESsType.CF, SscConstant.CONDITION_CFU,
                SscServiceClassUtil.SERVICE_CLASS_NONE);
        queryData.setResponseCode(SscConstant.HTTP_OK);
        CfServiceData data = (CfServiceData) mSscXmlParser.getSscServiceFromDoc(queryData,
                getDocumentFromString(xml), null);

        assertNotNull(data);
        assertEquals(SscConstant.CONDITION_CFU, data.getCondition());
        assertNotNull(data.getRuleSet());
        assertEquals(1, data.getRuleSet().size());
        SscRuleData ruleData = data.getRuleSet().get(0);
        assertEquals(SscConstant.CONDITION_CFU, ruleData.getSsCondition());
        assertEquals(SscServiceClassUtil.SERVICE_CLASS_VOICE, ruleData.getServiceClass());
        assertEquals(SscConstant.STATUS_ENABLE, ruleData.getState());
    }

    @Test
    public void getSscServiceFromDoc_cfWhenXmlHasVideo() {
        String xml = "<ss:communication-diversion active=\"true\">"
                + "<cp:ruleset>"
                + "<cp:rule id=\"call-diversion-unconditional-video\">"
                + "<cp:conditions>"
                + "<ss:rule-deactivated/>"
                + "<ss:media>video</ss:media>"
                + "</cp:conditions>"
                + "<cp:actions>"
                + "<ss:forward-to>"
                + "<ss:target>tel:+1234567890</ss:target>"
                + "</ss:forward-to>"
                + "</cp:actions>"
                + "</cp:rule>"
                + "</cp:ruleset>"
                + "</ss:communication-diversion>";

        processEntireDocumentQuery();

        SscServiceQueryData queryData = getQueryData(ESsType.CF, SscConstant.CONDITION_CFU,
                SscServiceClassUtil.SERVICE_CLASS_NONE);
        queryData.setResponseCode(SscConstant.HTTP_OK);
        CfServiceData data = (CfServiceData) mSscXmlParser.getSscServiceFromDoc(queryData,
                getDocumentFromString(xml), null);

        assertNotNull(data);
        assertEquals(SscConstant.CONDITION_CFU, data.getCondition());
        assertNotNull(data.getRuleSet());
        assertEquals(1, data.getRuleSet().size());
        SscRuleData ruleData = data.getRuleSet().get(0);
        assertEquals(SscConstant.CONDITION_CFU, ruleData.getSsCondition());
        assertEquals(SscServiceClassUtil.SERVICE_CLASS_VIDEO, ruleData.getServiceClass());
        assertEquals(SscConstant.STATUS_DISABLE, ruleData.getState());
    }

    @Test
    public void getSscServiceFromDoc_cfWhenXmlHasBothAudioAndVideo() {
        String xml = "<ss:communication-diversion active=\"true\">"
                + "<cp:ruleset>"
                + "<cp:rule id=\"call-diversion-unconditional\">"
                + "<cp:conditions>"
                + "<ss:rule-deactivated/>"
                + "<ss:media>audio</ss:media>"
                + "</cp:conditions>"
                + "<cp:actions>"
                + "<ss:forward-to>"
                + "<ss:target></ss:target>"
                + "</ss:forward-to>"
                + "</cp:actions>"
                + "</cp:rule>"
                + "<cp:rule id=\"call-diversion-unconditional-video\">"
                + "<cp:conditions>"
                + "<ss:media>video</ss:media>"
                + "</cp:conditions>"
                + "<cp:actions>"
                + "<ss:forward-to>"
                + "<ss:target>tel:+1234567890</ss:target>"
                + "</ss:forward-to>"
                + "</cp:actions>"
                + "</cp:rule>"
                + "</cp:ruleset>"
                + "</ss:communication-diversion>";

        processEntireDocumentQuery();

        SscServiceQueryData queryData = getQueryData(ESsType.CF, SscConstant.CONDITION_CFU,
                SscServiceClassUtil.SERVICE_CLASS_NONE);
        queryData.setResponseCode(SscConstant.HTTP_OK);
        CfServiceData data = (CfServiceData) mSscXmlParser.getSscServiceFromDoc(queryData,
                getDocumentFromString(xml), null);

        assertNotNull(data);
        assertNotNull(data.getRuleSet());
        assertEquals(2, data.getRuleSet().size());

        for (int i = 0; i < data.getRuleSet().size(); i++) {
            SscRuleData ruleData = data.getRuleSet().get(i);
            assertEquals(SscConstant.CONDITION_CFU, ruleData.getSsCondition());
            if (ruleData.getServiceClass() == SscServiceClassUtil.SERVICE_CLASS_VIDEO) {
                assertEquals(SscConstant.STATUS_ENABLE, ruleData.getState());
                assertEquals("+1234567890", ruleData.getForwardToNumber());
            } else if (ruleData.getServiceClass() == SscServiceClassUtil.SERVICE_CLASS_VOICE) {
                assertEquals(SscConstant.STATUS_DISABLE, ruleData.getState());
                assertNull(ruleData.getForwardToNumber());
            }
        }
    }

    @Test
    public void getSscServiceFromDoc_cfAudioWhenNoMediaInXml() {
        String xml = "<ss:communication-diversion active=\"true\">"
                + "<cp:ruleset>"
                + "<cp:rule id=\"call-diversion-unconditional\">"
                + "<cp:conditions>"
                + "<ss:rule-deactivated/>"
                + "</cp:conditions>"
                + "<cp:actions>"
                + "<ss:forward-to>"
                + "<ss:target>tel:+1234567890</ss:target>"
                + "</ss:forward-to>"
                + "</cp:actions>"
                + "</cp:rule>"
                + "</cp:ruleset>"
                + "</ss:communication-diversion>";

        processEntireDocumentQuery();

        SscServiceQueryData queryData = getQueryData(ESsType.CF, SscConstant.CONDITION_CFU,
                SscServiceClassUtil.SERVICE_CLASS_VOICE);
        queryData.setResponseCode(SscConstant.HTTP_OK);
        CfServiceData data = (CfServiceData) mSscXmlParser.getSscServiceFromDoc(queryData,
                getDocumentFromString(xml), null);

        assertNotNull(data);
        assertEquals(SscConstant.CONDITION_CFU, data.getCondition());
        assertNotNull(data.getRuleSet());
        assertEquals(1, data.getRuleSet().size());
        SscRuleData ruleData = data.getRuleSet().get(0);
        assertEquals(SscConstant.CONDITION_CFU, ruleData.getSsCondition());
        assertEquals(SscServiceClassUtil.SERVICE_CLASS_NONE, ruleData.getServiceClass());
        assertEquals(SscConstant.STATUS_DISABLE, ruleData.getState());
    }

    @Test
    public void getSscServiceFromDoc_cfVideoWhenNoMediaInXml() {
        String xml = "<ss:communication-diversion active=\"true\">"
                + "<cp:ruleset>"
                + "<cp:rule id=\"call-diversion-unconditional-video\">"
                + "<cp:conditions>"
                + "<ss:rule-deactivated/>"
                + "</cp:conditions>"
                + "<cp:actions>"
                + "<ss:forward-to>"
                + "<ss:target>tel:+1234567890</ss:target>"
                + "</ss:forward-to>"
                + "</cp:actions>"
                + "</cp:rule>"
                + "</cp:ruleset>"
                + "</ss:communication-diversion>";

        processEntireDocumentQuery();

        SscServiceQueryData queryData = getQueryData(ESsType.CF, SscConstant.CONDITION_CFU,
                SscServiceClassUtil.SERVICE_CLASS_VIDEO);
        queryData.setResponseCode(SscConstant.HTTP_OK);
        CfServiceData data = (CfServiceData) mSscXmlParser.getSscServiceFromDoc(queryData,
                getDocumentFromString(xml), null);

        assertNotNull(data);
        assertEquals(SscConstant.CONDITION_CFU, data.getCondition());
        assertNull(data.getRuleSet());
    }

    @Test
    public void getSscServiceFromDoc_cfVideoWhenXmlHasAudioOnly() {
        String xml = "<ss:communication-diversion active=\"true\">"
                + "<cp:ruleset>"
                + "<cp:rule id=\"call-diversion-unconditional\">"
                + "<cp:conditions>"
                + "<ss:rule-deactivated/>"
                + "<ss:media>audio</ss:media>"
                + "</cp:conditions>"
                + "<cp:actions>"
                + "<ss:forward-to>"
                + "<ss:target>tel:+1234567890</ss:target>"
                + "</ss:forward-to>"
                + "</cp:actions>"
                + "</cp:rule>"
                + "</cp:ruleset>"
                + "</ss:communication-diversion>";

        processEntireDocumentQuery();

        SscServiceQueryData queryData = getQueryData(ESsType.CF, SscConstant.CONDITION_CFU,
                SscServiceClassUtil.SERVICE_CLASS_VIDEO);
        queryData.setResponseCode(SscConstant.HTTP_OK);
        CfServiceData data = (CfServiceData) mSscXmlParser.getSscServiceFromDoc(queryData,
                getDocumentFromString(xml), null);

        assertNotNull(data);
        assertEquals(SscConstant.CONDITION_CFU, data.getCondition());
        assertNull(data.getRuleSet());
    }

    @Test
    public void getSscServiceFromDoc_cfVideoWhenXmlHasVideo() {
        String xml = "<ss:communication-diversion active=\"true\">"
                + "<cp:ruleset>"
                + "<cp:rule id=\"call-diversion-unconditional-video\">"
                + "<cp:conditions>"
                + "<ss:media>VIDEO</ss:media>"
                + "</cp:conditions>"
                + "<cp:actions>"
                + "<ss:forward-to>"
                + "<ss:target>tel:+1234567890</ss:target>"
                + "</ss:forward-to>"
                + "</cp:actions>"
                + "</cp:rule>"
                + "</cp:ruleset>"
                + "</ss:communication-diversion>";

        processEntireDocumentQuery();

        SscServiceQueryData queryData = getQueryData(ESsType.CF, SscConstant.CONDITION_CFU,
                SscServiceClassUtil.SERVICE_CLASS_VIDEO);
        queryData.setResponseCode(SscConstant.HTTP_OK);
        CfServiceData data = (CfServiceData) mSscXmlParser.getSscServiceFromDoc(queryData,
                getDocumentFromString(xml), null);

        assertNotNull(data);
        assertEquals(SscConstant.CONDITION_CFU, data.getCondition());
        assertNotNull(data.getRuleSet());
        assertEquals(1, data.getRuleSet().size());
        SscRuleData ruleData = data.getRuleSet().get(0);
        assertEquals(SscConstant.CONDITION_CFU, ruleData.getSsCondition());
        assertEquals(SscServiceClassUtil.SERVICE_CLASS_VIDEO, ruleData.getServiceClass());
        assertEquals(SscConstant.STATUS_ENABLE, ruleData.getState());
        assertEquals("+1234567890", ruleData.getForwardToNumber());
    }

    @Test
    public void getSscServiceFromDoc_cfnrTimer() {
        String xml = "<ss:communication-diversion active=\"true\">"
                + "<ss:NoReplyTimer>25</ss:NoReplyTimer>"
                + "<cp:ruleset>"
                + "<cp:rule id=\"call-diversion-no-reply\">"
                + "<cp:conditions>"
                + "<ss:rule-deactivated/>"
                + "<ss:no-answer/>"
                + "</cp:conditions>"
                + "<cp:actions>"
                + "<ss:forward-to>"
                + "<ss:target>tel:+1234567890</ss:target>"
                + "</ss:forward-to>"
                + "</cp:actions>"
                + "</cp:rule>"
                + "</cp:ruleset>"
                + "</ss:communication-diversion>";

        processEntireDocumentQuery();

        SscServiceQueryData queryData = getQueryData(ESsType.CF, SscConstant.CONDITION_CFNR_TIMER,
                SscServiceClassUtil.SERVICE_CLASS_NONE);
        queryData.setResponseCode(SscConstant.HTTP_OK);
        CfServiceData data = (CfServiceData) mSscXmlParser.getSscServiceFromDoc(queryData,
                getDocumentFromString(xml), null);

        assertNotNull(data);
        assertEquals(SscConstant.CONDITION_CFNR_TIMER, data.getCondition());
        assertEquals(25, data.getNoReplyTimer());
        assertNull(data.getRuleSet());
    }

    @Test
    public void getSscServiceFromDoc_cfnrWhenTimerIsInRule() {
        String xml = "<ss:communication-diversion active=\"true\">"
                + "<cp:ruleset>"
                + "<cp:rule id=\"call-diversion-no-reply\">"
                + "<cp:conditions>"
                + "<ss:rule-deactivated/>"
                + "<ss:no-answer/>"
                + "</cp:conditions>"
                + "<cp:actions>"
                + "<ss:forward-to>"
                + "<ss:target>tel:+1234567890</ss:target>"
                + "</ss:forward-to>"
                + "<ss:NoReplyTimer>25</ss:NoReplyTimer>"
                + "</cp:actions>"
                + "</cp:rule>"
                + "</cp:ruleset>"
                + "</ss:communication-diversion>";

        processEntireDocumentQuery();

        SscServiceQueryData queryData = getQueryData(ESsType.CF, SscConstant.CONDITION_CFNR,
                SscServiceClassUtil.SERVICE_CLASS_NONE);
        queryData.setResponseCode(SscConstant.HTTP_OK);
        CfServiceData data = (CfServiceData) mSscXmlParser.getSscServiceFromDoc(queryData,
                getDocumentFromString(xml), null);

        assertNotNull(data);
        assertEquals(SscConstant.CONDITION_CFNR, data.getCondition());
        assertEquals(25, data.getNoReplyTimer());
        assertNotNull(data.getRuleSet());
        assertEquals(1, data.getRuleSet().size());
        SscRuleData ruleData = data.getRuleSet().get(0);
        assertEquals(SscConstant.CONDITION_CFNR, ruleData.getSsCondition());
        assertEquals(SscServiceClassUtil.SERVICE_CLASS_NONE, ruleData.getServiceClass());
        assertEquals(SscConstant.STATUS_DISABLE, ruleData.getState());
    }

    @Test
    public void getSscServiceFromDoc_cfWhenNoData() {
        processEntireDocumentQuery();

        SscServiceQueryData queryData = getQueryData(ESsType.CF, SscConstant.CONDITION_CFNR,
                SscServiceClassUtil.SERVICE_CLASS_NONE);
        queryData.setResponseCode(SscConstant.HTTP_OK);
        CfServiceData data = (CfServiceData) mSscXmlParser.getSscServiceFromDoc(queryData,
                getEmptyXmlDoc(), null);

        assertNull(data);
    }

    @Test
    public void getSscServiceFromDoc_icb() {
        String xml = "<ss:incoming-communication-barring>"
                + "<cp:ruleset>"
                + "<cp:rule id=\"call-barring-all-incoming\">"
                + "<cp:conditions>"
                + "</cp:conditions>"
                + "<cp:actions>"
                + "<ss:allow>false</ss:allow>"
                + "</cp:actions>"
                + "</cp:rule>"
                + "</cp:ruleset>"
                + "</ss:incoming-communication-barring>";

        processEntireDocumentQuery();

        SscServiceQueryData queryData = getQueryData(ESsType.ICB, SscConstant.CONDITION_BAIC,
                SscServiceClassUtil.SERVICE_CLASS_NONE);
        queryData.setResponseCode(SscConstant.HTTP_OK);
        CbServiceData data = (CbServiceData) mSscXmlParser.getSscServiceFromDoc(queryData,
                getDocumentFromString(xml), null);

        assertNotNull(data);
        assertEquals(SscConstant.CONDITION_BAIC, data.getCondition());
        assertEquals(mTransactionId, data.getTransactionId());
        assertEquals(ESsType.ICB, data.getSsType());
        assertNotNull(data.getRuleSet());
        assertEquals(1, data.getRuleSet().size());
        SscRuleData ruleData = data.getRuleSet().get(0);
        assertEquals(SscConstant.CONDITION_BAIC, ruleData.getSsCondition());
        assertEquals(SscServiceClassUtil.SERVICE_CLASS_NONE, ruleData.getServiceClass());
        assertEquals(SscConstant.STATUS_ENABLE, ruleData.getState());
    }

    @Test
    public void getSscServiceFromDoc_icbAnonymous() {
        String xml = "<ss:incoming-communication-barring>"
                + "<cp:ruleset>"
                + "<cp:rule id=\"call-barring-anonymous-incoming\">"
                + "<cp:conditions>"
                + "<ss:anonymous/>"
                + "</cp:conditions>"
                + "<cp:actions>"
                + "<ss:allow>false</ss:allow>"
                + "</cp:actions>"
                + "</cp:rule>"
                + "</cp:ruleset>"
                + "</ss:incoming-communication-barring>";

        processEntireDocumentQuery();

        SscServiceQueryData queryData = getQueryData(ESsType.ICB, SscConstant.CONDITION_ACR,
                SscServiceClassUtil.SERVICE_CLASS_NONE);
        queryData.setResponseCode(SscConstant.HTTP_OK);
        CbServiceData data = (CbServiceData) mSscXmlParser.getSscServiceFromDoc(queryData,
                getDocumentFromString(xml), null);

        assertNotNull(data);
        assertEquals(SscConstant.CONDITION_ACR, data.getCondition());
        assertEquals(mTransactionId, data.getTransactionId());
        assertEquals(ESsType.ICB, data.getSsType());
        assertNotNull(data.getRuleSet());
        assertEquals(1, data.getRuleSet().size());
        SscRuleData ruleData = data.getRuleSet().get(0);
        assertEquals(SscConstant.CONDITION_ACR, ruleData.getSsCondition());
        assertEquals(SscServiceClassUtil.SERVICE_CLASS_NONE, ruleData.getServiceClass());
        assertEquals(SscConstant.STATUS_ENABLE, ruleData.getState());
    }

    @Test
    public void getSscServiceFromDoc_ocb() {
        String xml = "<ss:outgoing-communication-barring active=\"true\">"
                + "<cp:ruleset>"
                + "<cp:rule id=\"call-barring-outgoing-international\">"
                + "<cp:conditions>"
                + "<ss:international/>"
                + "<ss:rule-deactivated/>"
                + "</cp:conditions>"
                + "<cp:actions>"
                + "<ss:allow>false</ss:allow>"
                + "</cp:actions>"
                + "</cp:rule>"
                + "</cp:ruleset>"
                + "</ss:outgoing-communication-barring>";

        processEntireDocumentQuery();

        SscServiceQueryData queryData = getQueryData(ESsType.OCB, SscConstant.CONDITION_BOIC,
                SscServiceClassUtil.SERVICE_CLASS_NONE);
        queryData.setResponseCode(SscConstant.HTTP_OK);
        CbServiceData data = (CbServiceData) mSscXmlParser.getSscServiceFromDoc(queryData,
                getDocumentFromString(xml), null);

        assertNotNull(data);
        assertEquals(SscConstant.CONDITION_BOIC, data.getCondition());
        assertEquals(mTransactionId, data.getTransactionId());
        assertEquals(ESsType.OCB, data.getSsType());
        assertNotNull(data.getRuleSet());
        assertEquals(1, data.getRuleSet().size());
        SscRuleData ruleData = data.getRuleSet().get(0);
        assertEquals(SscConstant.CONDITION_BOIC, ruleData.getSsCondition());
        assertEquals(SscServiceClassUtil.SERVICE_CLASS_NONE, ruleData.getServiceClass());
        assertEquals(SscConstant.STATUS_DISABLE, ruleData.getState());
    }

    @Test
    public void getSscServiceFromDoc_cbWhenNoData() {
        processEntireDocumentQuery();

        SscServiceQueryData queryData = getQueryData(ESsType.ICB, SscConstant.CONDITION_BAIC,
                SscServiceClassUtil.SERVICE_CLASS_NONE);
        queryData.setResponseCode(SscConstant.HTTP_OK);
        CbServiceData data = (CbServiceData) mSscXmlParser.getSscServiceFromDoc(queryData,
                getEmptyXmlDoc(), null);

        assertNull(data);
    }

    @Test
    public void getSscServiceFromDoc_cw() {
        String xml = "<ss:communication-waiting active=\"false\"/>";

        processEntireDocumentQuery();

        SscServiceQueryData queryData = getQueryData(ESsType.CW, SscConstant.CONDITION_INVALID,
                SscServiceClassUtil.SERVICE_CLASS_NONE);
        queryData.setResponseCode(SscConstant.HTTP_OK);
        CwServiceData data = (CwServiceData) mSscXmlParser.getSscServiceFromDoc(queryData,
                getDocumentFromString(xml), null);

        assertNotNull(data);
        assertEquals(mTransactionId, data.getTransactionId());
        assertEquals(ESsType.CW, data.getSsType());
        assertEquals(SscConstant.EVENT_SSC_QUERY_CW, data.getEventNumber());
        assertEquals(SscConstant.STATUS_DISABLE, data.getState());
    }

    @Test
    public void getSscServiceFromDoc_cwWhenNoData() {
        processEntireDocumentQuery();

        SscServiceQueryData queryData = getQueryData(ESsType.CW, SscConstant.CONDITION_INVALID,
                SscServiceClassUtil.SERVICE_CLASS_NONE);
        queryData.setResponseCode(SscConstant.HTTP_OK);
        CwServiceData data = (CwServiceData) mSscXmlParser.getSscServiceFromDoc(queryData,
                getEmptyXmlDoc(), null);

        assertNull(data);
    }

    @Test
    public void getSscServiceFromDoc_updateCache() {
        String xml = "<ss:communication-waiting active=\"false\"/>";
        String xmlForCache = "<ss:communication-waiting active=\"true\"/>";
        Document cachedDoc = getDocumentFromString(xmlForCache);

        processEntireDocumentQuery();
        SscServiceQueryData queryData = getQueryData(ESsType.CW, SscConstant.CONDITION_INVALID,
                SscServiceClassUtil.SERVICE_CLASS_NONE);
        queryData.setResponseCode(SscConstant.HTTP_OK);
        mSscXmlParser.getSscServiceFromDoc(queryData, getDocumentFromString(xml), cachedDoc);

        assertEquals("false", cachedDoc.getDocumentElement().getAttribute("active"));
    }

    @Test
    public void getSscServiceFromDoc_serviceCapability() {
        when(mMockCarrierConfig.getBoolean(eq(CarrierConfig.ImsSs.KEY_UT_INSERT_NEW_RULE_BOOL)))
                .thenReturn(true);

        processEntireDocumentQuery();

        assertEquals(true, SscXmlFormat.getProvisionStatus(SLOT_0, SscXmlFormat.SC_CD,
                SscConstant.CONDITION_CFU));
        assertEquals(true, SscXmlFormat.getProvisionStatus(SLOT_0, SscXmlFormat.SC_CD,
                SscConstant.CONDITION_CFB));
        assertEquals(false, SscXmlFormat.getProvisionStatus(SLOT_0, SscXmlFormat.SC_CD,
                SscConstant.CONDITION_CFNR));
        assertEquals(true, SscXmlFormat.getProvisionStatus(SLOT_0, SscXmlFormat.SC_CD,
                SscConstant.CONDITION_CFNRC));
        assertEquals(false, SscXmlFormat.getProvisionStatus(SLOT_0, SscXmlFormat.SC_CD,
                SscConstant.CONDITION_CFNL));

        assertEquals(true, SscXmlFormat.getMediaCapability(SLOT_0, SscXmlFormat.SC_CD,
                SscXmlFormat.MEDIA_TYPE_AUDIO));
        assertEquals(true, SscXmlFormat.getMediaCapability(SLOT_0, SscXmlFormat.SC_CD,
                SscXmlFormat.MEDIA_TYPE_VIDEO));

        assertEquals(false, SscXmlFormat.getProvisionStatus(SLOT_0, SscXmlFormat.SC_CB,
                SscConstant.CONDITION_BAIC));
        assertEquals(false, SscXmlFormat.getProvisionStatus(SLOT_0, SscXmlFormat.SC_CB,
                SscConstant.CONDITION_BAOC));
        assertEquals(true, SscXmlFormat.getProvisionStatus(SLOT_0, SscXmlFormat.SC_CB,
                SscConstant.CONDITION_BOIC));
        assertEquals(true, SscXmlFormat.getProvisionStatus(SLOT_0, SscXmlFormat.SC_CB,
                SscConstant.CONDITION_BOIC_EXHC));
        assertEquals(true, SscXmlFormat.getProvisionStatus(SLOT_0, SscXmlFormat.SC_CB,
                SscConstant.CONDITION_BIC_WR));
        assertEquals(false, SscXmlFormat.getProvisionStatus(SLOT_0, SscXmlFormat.SC_CB,
                SscConstant.CONDITION_ACR));

        assertEquals(true, SscXmlFormat.getMediaCapability(SLOT_0, SscXmlFormat.SC_CB,
                SscXmlFormat.MEDIA_TYPE_AUDIO));
        assertEquals(true, SscXmlFormat.getMediaCapability(SLOT_0, SscXmlFormat.SC_CB,
                SscXmlFormat.MEDIA_TYPE_VIDEO));

        SscConfig.clear(SLOT_0);
    }

    @Test
    public void getSscServiceFromDoc_serviceCapabilityWhenNoData() {
        when(mMockCarrierConfig.getBoolean(eq(CarrierConfig.ImsSs.KEY_UT_INSERT_NEW_RULE_BOOL)))
                .thenReturn(true);

        SscServiceQueryData documentQueryData = getDocumentQueryData();
        documentQueryData.setResponseCode(SscConstant.HTTP_OK);
        mSscXmlParser.getSscServiceFromDoc(documentQueryData, getEmptyXmlDoc(), null);

        assertEquals(true, SscXmlFormat.getProvisionStatus(SLOT_0, SscXmlFormat.SC_CD,
                SscConstant.CONDITION_CFU));
        assertEquals(true, SscXmlFormat.getProvisionStatus(SLOT_0, SscXmlFormat.SC_CD,
                SscConstant.CONDITION_CFB));
        assertEquals(true, SscXmlFormat.getProvisionStatus(SLOT_0, SscXmlFormat.SC_CD,
                SscConstant.CONDITION_CFNR));
        assertEquals(true, SscXmlFormat.getProvisionStatus(SLOT_0, SscXmlFormat.SC_CD,
                SscConstant.CONDITION_CFNRC));
        assertEquals(true, SscXmlFormat.getProvisionStatus(SLOT_0, SscXmlFormat.SC_CD,
                SscConstant.CONDITION_CFNL));

        assertEquals(false, SscXmlFormat.getMediaCapability(SLOT_0, SscXmlFormat.SC_CD,
                SscXmlFormat.MEDIA_TYPE_AUDIO));
        assertEquals(false, SscXmlFormat.getMediaCapability(SLOT_0, SscXmlFormat.SC_CD,
                SscXmlFormat.MEDIA_TYPE_VIDEO));

        assertEquals(true, SscXmlFormat.getProvisionStatus(SLOT_0, SscXmlFormat.SC_CB,
                SscConstant.CONDITION_BAIC));
        assertEquals(true, SscXmlFormat.getProvisionStatus(SLOT_0, SscXmlFormat.SC_CB,
                SscConstant.CONDITION_BAOC));
        assertEquals(true, SscXmlFormat.getProvisionStatus(SLOT_0, SscXmlFormat.SC_CB,
                SscConstant.CONDITION_BOIC));
        assertEquals(true, SscXmlFormat.getProvisionStatus(SLOT_0, SscXmlFormat.SC_CB,
                SscConstant.CONDITION_BOIC_EXHC));
        assertEquals(true, SscXmlFormat.getProvisionStatus(SLOT_0, SscXmlFormat.SC_CB,
                SscConstant.CONDITION_BIC_WR));
        assertEquals(true, SscXmlFormat.getProvisionStatus(SLOT_0, SscXmlFormat.SC_CB,
                SscConstant.CONDITION_ACR));

        assertEquals(false, SscXmlFormat.getMediaCapability(SLOT_0, SscXmlFormat.SC_CB,
                SscXmlFormat.MEDIA_TYPE_AUDIO));
        assertEquals(false, SscXmlFormat.getMediaCapability(SLOT_0, SscXmlFormat.SC_CB,
                SscXmlFormat.MEDIA_TYPE_VIDEO));

        SscConfig.clear(SLOT_0);
    }

    private void processEntireDocumentQuery() {
        SscServiceQueryData documentQueryData = getDocumentQueryData();
        documentQueryData.setResponseCode(SscConstant.HTTP_OK);
        mSscXmlParser.getSscServiceFromDoc(documentQueryData, getEntireXmlDoc(), null);
    }

    private SscServiceQueryData getDocumentQueryData() {
        return SscXmlGovTest.createDocumentQueryData();
    }

    private SscServiceQueryData getQueryData(ESsType ssType, int condition, int serviceClass) {
        return SscXmlGovTest.createQueryData(ssType, mTransactionId, condition, serviceClass);
    }

    private Document getDocumentFromString(String xml) {
        return SscXmlGovTest.createDocumentFromString(xml);
    }

    private Document getEmptyXmlDoc() {
        return SscXmlGovTest.createEmptyXmlDoc();
    }

    private Document getEntireXmlDoc() {
        return SscXmlGovTest.createEntireXmlDoc();
    }

    private Document getErrorXmlDoc(boolean includingErrorPhrase) {
        return SscXmlGovTest.createErrorXmlDoc(includingErrorPhrase);
    }
}
