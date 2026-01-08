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
import static org.junit.Assert.assertNotNull;
import static org.junit.Assert.assertNull;
import static org.junit.Assert.assertTrue;
import static org.mockito.ArgumentMatchers.any;
import static org.mockito.ArgumentMatchers.eq;
import static org.mockito.Mockito.never;
import static org.mockito.Mockito.verify;
import static org.mockito.Mockito.when;

import com.android.imsstack.core.agents.ConfigInterface;
import com.android.imsstack.core.config.CarrierConfig;
import com.android.imsstack.enabler.ssc.data.CbServiceQueryData;
import com.android.imsstack.enabler.ssc.data.CbServiceUpdateData;
import com.android.imsstack.enabler.ssc.data.CfServiceQueryData;
import com.android.imsstack.enabler.ssc.data.CfServiceUpdateData;
import com.android.imsstack.enabler.ssc.data.CwServiceData;
import com.android.imsstack.enabler.ssc.data.OipServiceData;
import com.android.imsstack.enabler.ssc.data.OirServiceData;
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
import org.w3c.dom.Element;
import org.w3c.dom.NodeList;
import org.xml.sax.InputSource;

import java.io.StringReader;

import javax.xml.parsers.DocumentBuilder;
import javax.xml.parsers.DocumentBuilderFactory;

@RunWith(JUnit4.class)
public class SscXmlGovTest {
    private static final int SLOT_0 = 0;

    @Mock private CarrierConfig mMockCarrierConfig;
    @Mock private ConfigInterface mMockConfigInterface;
    @Mock private SscXmlCreator mMockXmlCreator;
    @Mock private SscXmlParser mMockXmlParser;

    private SscXmlGov mSscXmlGov;

    @Before
    public void setup() {
        MockitoAnnotations.initMocks(this);

        when(mMockConfigInterface.getCarrierConfig()).thenReturn(mMockCarrierConfig);
        SscConfig.setConfigInterface(SLOT_0, mMockConfigInterface);

        mSscXmlGov = SscXmlGov.getInstance(SLOT_0);
        mSscXmlGov.init();
    }

    @After
    public void tearDown() {
        mSscXmlGov.clear();
        SscConfig.clear(SLOT_0);
    }

    @Test
    public void initAndClear() {
        // init() is called in setup()
        assertEquals(1, mSscXmlGov.sSscXmlGovs.size());
        assertEquals(1, SscXmlFormat.sServerFormats.size());

        mSscXmlGov.mSimservDoc = createEmptyXmlDoc();
        mSscXmlGov.mSimservDocForUpdate = createEmptyXmlDoc();
        mSscXmlGov.clear();

        assertEquals(0, SscXmlFormat.sServerFormats.size());
        assertNull(mSscXmlGov.mSimservDoc);
        assertNull(mSscXmlGov.mSimservDocForUpdate);
    }

    @Test
    public void syncCachedDataWithUpdatedData_responseCode100() {
        Document xmlDoc = createEmptyXmlDoc();
        mSscXmlGov.mSimservDocForUpdate = xmlDoc;

        mSscXmlGov.syncCachedDataWithUpdatedData(100);

        assertNull(mSscXmlGov.mSimservDoc);
        assertNull(mSscXmlGov.mSimservDocForUpdate);
    }

    @Test
    public void syncCachedDataWithUpdatedData_responseCode200() {
        Document xmlDoc = createEmptyXmlDoc();
        mSscXmlGov.mSimservDocForUpdate = xmlDoc;

        mSscXmlGov.syncCachedDataWithUpdatedData(200);

        assertEquals(xmlDoc, mSscXmlGov.mSimservDoc);
        assertNull(mSscXmlGov.mSimservDocForUpdate);
    }

    @Test
    public void syncCachedDataWithUpdatedData_responseCode300() {
        Document xmlDoc = createEmptyXmlDoc();
        mSscXmlGov.mSimservDocForUpdate = xmlDoc;

        mSscXmlGov.syncCachedDataWithUpdatedData(300);

        assertNull(mSscXmlGov.mSimservDoc);
        assertNull(mSscXmlGov.mSimservDocForUpdate);
    }

    @Test
    public void updateTagsAndRules_whenCachedDocIsNull() {
        replaceInstance(SscXmlGov.class, "mSscXmlParser", mSscXmlGov, mMockXmlParser);

        mSscXmlGov.updateTagsAndRules();

        verify(mMockXmlParser, never()).updateTagsAndRules(eq(SLOT_0), any());
    }

    @Test
    public void updateTagsAndRules_whenCachedDocIsNotNull() {
        Document xmlDoc = createEmptyXmlDoc();
        replaceInstance(SscXmlGov.class, "mSscXmlParser", mSscXmlGov, mMockXmlParser);
        mSscXmlGov.mSimservDoc = xmlDoc;

        mSscXmlGov.updateTagsAndRules();

        verify(mMockXmlParser).updateTagsAndRules(SLOT_0, xmlDoc);
    }

    @Test
    public void isXmlDataPresent() {
        mSscXmlGov.mSimservDoc = createEmptyXmlDoc();
        assertTrue(mSscXmlGov.isXmlDataPresent());

        mSscXmlGov.mSimservDoc = null;
        assertFalse(mSscXmlGov.isXmlDataPresent());
    }

    @Test
    public void parserXmlStream_documentQueryDataOmittingNsOfRootElement() {
        Document xmlDoc = createEntireXmlDoc();
        SscServiceQueryData queryData = createDocumentQueryData();
        queryData.setResponseCode(SscConstant.HTTP_OK);
        SscServiceData parsedData = new SscServiceData(SLOT_0, queryData.getSsType(),
                queryData.getEventNumber(), queryData.getTransactionId(), 1);

        replaceInstance(SscXmlGov.class, "mSscXmlParser", mSscXmlGov, mMockXmlParser);
        when(mMockXmlParser.getSscServiceFromDoc(any(), any(), any())).thenReturn(parsedData);

        when(mMockCarrierConfig.getBoolean(CarrierConfig.ImsSs.KEY_UT_OMIT_NAMESPACE_SS_BOOL))
                .thenReturn(false);
        when(mMockCarrierConfig.getBoolean(CarrierConfig.ImsSs.KEY_UT_OMIT_NAMESPACE_CP_BOOL))
                .thenReturn(false);
        when(mMockCarrierConfig
                .getBoolean(CarrierConfig.ImsSs.KEY_UT_OMIT_NAMESPACE_OF_DOCUMENT_ELEMENT_BOOL))
                .thenReturn(true);

        SscServiceData result = mSscXmlGov.parseXmlStream(queryData, xmlDoc);

        assertEquals(parsedData, result);
        assertNotNull(mSscXmlGov.mSimservDoc);
        verify(mMockXmlParser).getSscServiceFromDoc(any(), any(), any());

        NodeList nodeList = mSscXmlGov.mSimservDoc.getElementsByTagName("ss:simservs");
        assertEquals(0, nodeList.getLength());
    }

    @Test
    public void parserXmlStream_documentQueryDataOmittingAllNs() {
        Document xmlDoc = createEntireXmlDoc();
        SscServiceQueryData queryData = createDocumentQueryData();
        queryData.setResponseCode(SscConstant.HTTP_OK);
        SscServiceData parsedData = new SscServiceData(SLOT_0, queryData.getSsType(),
                queryData.getEventNumber(), queryData.getTransactionId(), 1);

        replaceInstance(SscXmlGov.class, "mSscXmlParser", mSscXmlGov, mMockXmlParser);
        when(mMockXmlParser.getSscServiceFromDoc(any(), any(), any())).thenReturn(parsedData);

        when(mMockCarrierConfig.getBoolean(CarrierConfig.ImsSs.KEY_UT_OMIT_NAMESPACE_SS_BOOL))
                .thenReturn(true);
        when(mMockCarrierConfig.getBoolean(CarrierConfig.ImsSs.KEY_UT_OMIT_NAMESPACE_CP_BOOL))
                .thenReturn(true);
        when(mMockCarrierConfig
                .getBoolean(CarrierConfig.ImsSs.KEY_UT_OMIT_NAMESPACE_OF_DOCUMENT_ELEMENT_BOOL))
                .thenReturn(false);

        SscServiceData result = mSscXmlGov.parseXmlStream(queryData, xmlDoc);

        assertEquals(parsedData, result);
        assertNotNull(mSscXmlGov.mSimservDoc);
        verify(mMockXmlParser).getSscServiceFromDoc(any(), any(), any());

        NodeList cwList = mSscXmlGov.mSimservDoc.getElementsByTagName("ss:communication-waiting");
        assertEquals(0, cwList.getLength());
        NodeList ruleList = mSscXmlGov.mSimservDoc.getElementsByTagName("cp:rule");
        assertEquals(0, ruleList.getLength());
    }

    @Test
    public void parserXmlStream_documentQueryDataNotCachedWhenError() {
        Document xmlDoc = createEntireXmlDoc();
        SscServiceQueryData queryData = createDocumentQueryData();
        queryData.setResponseCode(SscConstant.HTTP_CONFLICT);
        SscServiceData parsedData = new SscServiceData(SLOT_0, queryData.getSsType(),
                queryData.getEventNumber(), queryData.getTransactionId(), 1);

        replaceInstance(SscXmlGov.class, "mSscXmlParser", mSscXmlGov, mMockXmlParser);
        when(mMockXmlParser.getSscServiceFromDoc(any(), any(), any())).thenReturn(parsedData);

        SscServiceData result = mSscXmlGov.parseXmlStream(queryData, xmlDoc);

        assertEquals(parsedData, result);
        assertNull(mSscXmlGov.mSimservDoc);
        verify(mMockXmlParser).getSscServiceFromDoc(any(), any(), any());
    }

    @Test
    public void parserXmlStream_serviceQueryDataNotCached() {
        Document xmlDoc = createEntireXmlDoc();
        SscServiceQueryData queryData = createQueryData(ESsType.CF, 0, SscConstant.CONDITION_CFNR,
                SscServiceClassUtil.SERVICE_CLASS_NONE);
        queryData.setResponseCode(SscConstant.HTTP_CONFLICT);
        SscServiceData parsedData = new SscServiceData(SLOT_0, queryData.getSsType(),
                queryData.getEventNumber(), queryData.getTransactionId(), 1);

        replaceInstance(SscXmlGov.class, "mSscXmlParser", mSscXmlGov, mMockXmlParser);
        when(mMockXmlParser.getSscServiceFromDoc(any(), any(), any())).thenReturn(parsedData);

        SscServiceData result = mSscXmlGov.parseXmlStream(queryData, xmlDoc);

        assertEquals(parsedData, result);
        assertNull(mSscXmlGov.mSimservDoc);
        verify(mMockXmlParser).getSscServiceFromDoc(any(), any(), any());
    }

    @Test
    public void parserXmlStream_updateData() {
        Document xmlDoc = createEntireXmlDoc();
        SscServiceData updateData = createUpdateData(ESsType.CF, 0, SscConstant.ACTION_ACTIVATION,
                SscConstant.CONDITION_CFNR, null, SscServiceClassUtil.SERVICE_CLASS_NONE, 0);
        updateData.setResponseCode(SscConstant.HTTP_CONFLICT);
        SscServiceData parsedData = new SscServiceData(SLOT_0, updateData.getSsType(),
                updateData.getEventNumber(), updateData.getTransactionId(), 1);

        replaceInstance(SscXmlGov.class, "mSscXmlParser", mSscXmlGov, mMockXmlParser);
        when(mMockXmlParser.getSscServiceFromDoc(any(), any(), any())).thenReturn(parsedData);

        SscServiceData result = mSscXmlGov.parseXmlStream(updateData, xmlDoc);

        assertEquals(parsedData, result);
        assertNull(mSscXmlGov.mSimservDoc);
        verify(mMockXmlParser).getSscServiceFromDoc(any(), any(), any());
    }

    @Test
    public void createXmlStream_noCachedData() {
        SscServiceData updateData = createUpdateData(ESsType.CF, 0, SscConstant.ACTION_ACTIVATION,
                SscConstant.CONDITION_CFNR, null, SscServiceClassUtil.SERVICE_CLASS_NONE, 0);
        replaceInstance(SscXmlGov.class, "mSscXmlCreator", mSscXmlGov, mMockXmlCreator);
        when(mMockXmlCreator.createXml(any(), any()))
                .thenReturn(createEmptyXmlDoc().getDocumentElement());

        String xml = mSscXmlGov.createXmlStream(updateData);

        assertNull(xml);
        assertNull(mSscXmlGov.mSimservDocForUpdate);
        verify(mMockXmlCreator, never()).createXml(any(), any());
    }

    @Test
    public void createXmlStream() {
        SscServiceData updateData = createUpdateData(ESsType.CF, 0, SscConstant.ACTION_ACTIVATION,
                SscConstant.CONDITION_CFNR, null, SscServiceClassUtil.SERVICE_CLASS_NONE, 0);
        replaceInstance(SscXmlGov.class, "mSscXmlCreator", mSscXmlGov, mMockXmlCreator);
        when(mMockXmlCreator.createXml(any(), any()))
                .thenReturn(createEmptyXmlDoc().getDocumentElement());
        mSscXmlGov.mSimservDoc = createEntireXmlDoc();

        String xml = mSscXmlGov.createXmlStream(updateData);

        assertNotNull(xml);
        assertTrue(xml.contains("ss:simservs"));
        assertNotNull(mSscXmlGov.mSimservDocForUpdate);
        verify(mMockXmlCreator).createXml(any(), eq(updateData));
    }

    protected static SscServiceQueryData createDocumentQueryData() {
        return new SscServiceQueryData(SLOT_0, ESsType.NONE, SscConstant.EVENT_SSC_QUERY_DOCUMENT,
                0, 0);
    }

    protected static SscServiceQueryData createQueryData(ESsType ssType, int transactionId,
            int condition, int serviceClass) {
        if (ssType == ESsType.CF) {
            return new CfServiceQueryData(SLOT_0, ssType, SscConstant.EVENT_SSC_QUERY_CF,
                    transactionId, condition, "", serviceClass);
        } else if (ssType == ESsType.ICB || ssType == ESsType.OCB) {
            return new CbServiceQueryData(SLOT_0, ssType, SscConstant.EVENT_SSC_QUERY_CB,
                    transactionId, condition, serviceClass);
        } else if (ssType == ESsType.CW) {
            return new SscServiceQueryData(SLOT_0, ssType, SscConstant.EVENT_SSC_QUERY_CW,
                    transactionId, -1);
        } else if (ssType == ESsType.OIR) {
            return new SscServiceQueryData(SLOT_0, ssType, SscConstant.EVENT_SSC_QUERY_OIR,
                    transactionId, -1);
        } else if (ssType == ESsType.OIP) {
            return new SscServiceQueryData(SLOT_0, ssType, SscConstant.EVENT_SSC_QUERY_OIP,
                    transactionId, -1);
        } else if (ssType == ESsType.TIR) {
            return new SscServiceQueryData(SLOT_0, ssType, SscConstant.EVENT_SSC_QUERY_TIR,
                    transactionId, -1);
        } else if (ssType == ESsType.TIP) {
            return new SscServiceQueryData(SLOT_0, ssType, SscConstant.EVENT_SSC_QUERY_TIP,
                    transactionId, -1);
        }

        return createDocumentQueryData();
    }

    protected static SscServiceData createUpdateData(ESsType ssType, int transactionId, int action,
            int condition, String targetNumber, int serviceClass, int noReplyTimer) {
        if (ssType == ESsType.CF) {
            return new CfServiceUpdateData(SLOT_0, ssType, SscConstant.EVENT_SSC_UPDATE_CF,
                    transactionId, action, condition, targetNumber, noReplyTimer, serviceClass);
        } else if (ssType == ESsType.ICB || ssType == ESsType.OCB) {
            return new CbServiceUpdateData(SLOT_0, ssType, SscConstant.EVENT_SSC_UPDATE_CB,
                    transactionId, action, condition, null, serviceClass, null);
        } else if (ssType == ESsType.CW) {
            return new CwServiceData(SLOT_0, ssType, SscConstant.EVENT_SSC_UPDATE_CW,
                    transactionId, action);
        } else if (ssType == ESsType.OIR) {
            return new OirServiceData(SLOT_0, ssType, SscConstant.EVENT_SSC_UPDATE_OIR,
                    transactionId, action, 0, 0);
        } else if (ssType == ESsType.OIP) {
            return new OipServiceData(SLOT_0, ssType, SscConstant.EVENT_SSC_UPDATE_OIP,
                    transactionId, action);
        } else if (ssType == ESsType.TIR) {
            return new TirServiceData(SLOT_0, ssType, SscConstant.EVENT_SSC_UPDATE_TIR,
                    transactionId, action, 0);
        } else if (ssType == ESsType.TIP) {
            return new TipServiceData(SLOT_0, ssType, SscConstant.EVENT_SSC_UPDATE_TIP,
                    transactionId, action);
        }

        return null;
    }

    protected static SscServiceData createInsertData(ESsType ssType, int transactionId, int action,
            int condition, String targetNumber, int serviceClass) {
        if (ssType == ESsType.CF) {
            return new CfServiceUpdateData(SLOT_0, ssType, SscConstant.EVENT_SSC_INSERT_CF,
                    transactionId, action, condition, targetNumber, 0, serviceClass);
        }

        return new CbServiceUpdateData(SLOT_0, ssType, SscConstant.EVENT_SSC_INSERT_CB,
                transactionId, action, condition, null, serviceClass, null);
    }

    protected static Document removeRuleSet(Document doc) {
        NodeList ruleSetList = doc.getElementsByTagName("cp:ruleset");
        for (int i = 0; i < ruleSetList.getLength(); i++) {
            ruleSetList.item(i).getParentNode().removeChild(ruleSetList.item(i));
        }

        return doc;
    }

    protected static Document removeRule(Document doc, String ruleId) {
        Element rule = doc.getElementById(ruleId);
        assertNotNull(rule);
        rule.getParentNode().removeChild(rule);

        return doc;
    }

    protected static Document createDocumentFromString(String xml) {
        Document document;
        try {
            DocumentBuilderFactory factory = DocumentBuilderFactory.newInstance();
            factory.setNamespaceAware(false);
            DocumentBuilder builder = factory.newDocumentBuilder();
            InputSource is = new InputSource(new StringReader(xml));
            document = builder.parse(is);
        } catch (Exception e) {
            e.printStackTrace();
            return null;
        }

        return document;
    }

    protected static Document createErrorXmlDoc(boolean includingErrorPhrase) {
        String xml;
        if (includingErrorPhrase) {
            xml = "<xe:xcap-error><xe:constraint-failure "
                    + "phrase=\"Service setting could not be updated.\"></xe:constraint-failure>"
                    + "</xe:xcap-error>";
        } else {
            xml = "<xe:xcap-error></xe:xcap-error>";
        }

        Document document;
        try {
            DocumentBuilderFactory factory = DocumentBuilderFactory.newInstance();
            factory.setNamespaceAware(false);
            DocumentBuilder builder = factory.newDocumentBuilder();
            InputSource is = new InputSource(new StringReader(xml));
            document = builder.parse(is);
        } catch (Exception e) {
            e.printStackTrace();
            return null;
        }

        return document;
    }

    protected static Document createEmptyXmlDoc() {
        String xml = "<ss:simservs />";

        Document document;
        try {
            DocumentBuilderFactory factory = DocumentBuilderFactory.newInstance();
            factory.setNamespaceAware(false);
            DocumentBuilder builder = factory.newDocumentBuilder();
            InputSource is = new InputSource(new StringReader(xml));
            document = builder.parse(is);
        } catch (Exception e) {
            e.printStackTrace();
            return null;
        }

        return document;
    }

    protected static Document createEntireXmlDoc() {
        String xml = "<ss:simservs xmlns:SOAP-ENV=\"http://schemas.xmlsoap.org/soap/envelope/\""
                + "xmlns:SOAP-ENC=\"http://schemas.xmlsoap.org/soap/encoding/\""
                + "xmlns:xsi=\"http://www.w3.org/2001/XMLSchema-instance\""
                + "xmlns:xsd=\"http://www.w3.org/2001/XMLSchema\""
                + "xmlns:cp=\"urn:ietf:params:xml:ns:common-policy\""
                + "xmlns:ss=\"http://uri.etsi.org/ngn/params/xml/simservs/xcap\""
                + "xmlns:ocp=\"urn:oma:xml:xdm:common-policy\""
                + "xmlns:utns=\"urn:com:att:tlv:utx\""
                + "xmlns:xe=\"urn:ietf:params:xml:ns:xcap-error\""
                + "xmlns:data=\"http://com/alu/icm/fs5000dbv5_0/data\""
                + "xmlns:prs=\"http://www.nokia.com/prs/SubscriptionAPI\">"
                // ICB
                + "<ss:incoming-communication-barring active=\"true\">"
                + "<cp:ruleset xmlns:cp=\"urn:ietf:params:xml:ns:common-policy\">"
                + "<cp:rule id=\"call-barring-all-incoming\">"
                + "<cp:conditions>"
                + "<ss:rule-deactivated/>"
                + "</cp:conditions>"
                + "<cp:actions>"
                + "<ss:allow>false</ss:allow>"
                + "</cp:actions>"
                + "</cp:rule>"
                + "<cp:rule id=\"call-barring-incoming-in-roaming\">"
                + "<cp:conditions>"
                + "<ss:rule-deactivated/>"
                + "<ss:roaming/>"
                + "</cp:conditions>"
                + "<cp:actions>"
                + "<ss:allow>false</ss:allow>"
                + "</cp:actions>"
                + "</cp:rule>"
                + "<cp:rule id=\"call-barring-anonymous-incoming\">"
                + "<cp:conditions>"
                + "<ss:rule-deactivated/>"
                + "<ss:anonymous/>"
                + "</cp:conditions>"
                + "<cp:actions>"
                + "<ss:allow>false</ss:allow>"
                + "</cp:actions>"
                + "</cp:rule>"
                // ICB Video
                + "<cp:rule id=\"call-barring-all-incoming-video\">"
                + "<cp:conditions>"
                + "<ss:rule-deactivated/>"
                + "<ss:media>video</ss:media>"
                + "</cp:conditions>"
                + "<cp:actions>"
                + "<ss:allow>false</ss:allow>"
                + "</cp:actions>"
                + "</cp:rule>"
                + "<cp:rule id=\"call-barring-incoming-in-roaming-video\">"
                + "<cp:conditions>"
                + "<ss:rule-deactivated/>"
                + "<ss:roaming/>"
                + "<ss:media>video</ss:media>"
                + "</cp:conditions>"
                + "<cp:actions>"
                + "<ss:allow>false</ss:allow>"
                + "</cp:actions>"
                + "</cp:rule>"
                + "<cp:rule id=\"call-barring-anonymous-incoming-video\">"
                + "<cp:conditions>"
                + "<ss:rule-deactivated/>"
                + "<ss:anonymous/>"
                + "<ss:media>video</ss:media>"
                + "</cp:conditions>"
                + "<cp:actions>"
                + "<ss:allow>false</ss:allow>"
                + "</cp:actions>"
                + "</cp:rule>"
                + "</cp:ruleset>"
                + "</ss:incoming-communication-barring>"
                // OCB
                + "<ss:outgoing-communication-barring active=\"true\">"
                + "<cp:ruleset xmlns:cp=\"urn:ietf:params:xml:ns:common-policy\">"
                // OCB audio media
                + "<cp:rule id=\"call-barring-all-outgoing-audio\">"
                + "<cp:conditions>"
                + "<ss:rule-deactivated/>"
                + "<ss:media>audio</ss:media>"
                + "</cp:conditions>"
                + "<cp:actions>"
                + "<ss:allow>false</ss:allow>"
                + "</cp:actions>"
                + "</cp:rule>"
                // OCB no media
                + "<cp:rule id=\"call-barring-all-outgoing\">"
                + "<cp:conditions>"
                + "<ss:rule-deactivated/>"
                + "</cp:conditions>"
                + "<cp:actions>"
                + "<ss:allow>false</ss:allow>"
                + "</cp:actions>"
                + "</cp:rule>"
                + "<cp:rule id=\"call-barring-outgoing-international\">"
                + "<cp:conditions>"
                + "<ss:rule-deactivated/>"
                + "<ss:international/>"
                + "</cp:conditions>"
                + "<cp:actions>"
                + "<ss:allow>false</ss:allow>"
                + "</cp:actions>"
                + "</cp:rule>"
                + "<cp:rule id=\"call-barring-outgoing-internationalExHC\">"
                + "<cp:conditions>"
                + "<ss:rule-deactivated/>"
                + "<ss:international-exHC/>"
                + "</cp:conditions>"
                + "<cp:actions>"
                + "<ss:allow>false</ss:allow>"
                + "</cp:actions>"
                + "</cp:rule>"
                + "</cp:ruleset>"
                + "</ss:outgoing-communication-barring>"
                // CW
                + "<ss:communication-waiting active=\"true\"/>"
                // CD
                + "<ss:communication-diversion active=\"true\">"
                + "<ss:NoReplyTimer>25</ss:NoReplyTimer>"
                + "<cp:ruleset>"
                // CD - CFB audio
                + "<cp:rule id=\"call-diversion-busy-audio\">"
                + "<cp:conditions>"
                + "<ss:rule-deactivated/>"
                + "<ss:media>audio</ss:media>"
                + "<ss:busy/>"
                + "</cp:conditions>"
                + "<cp:actions>"
                + "<ss:forward-to>"
                + "<ss:target>tel:+1234567890</ss:target>"
                + "<ss:notify-caller>true</ss:notify-caller>"
                + "<ss:reveal-identity-to-caller>true</ss:reveal-identity-to-caller>"
                + "<ss:reveal-served-user-identity-to-caller>false"
                + "</ss:reveal-served-user-identity-to-caller>"
                + "<ss:reveal-identity-to-target>false</ss:reveal-identity-to-target>"
                + "</ss:forward-to>"
                + "</cp:actions>"
                + "</cp:rule>"
                // CD - no media
                + "<cp:rule id=\"call-diversion-unconditional\">"
                + "<cp:conditions>"
                + "<ss:rule-deactivated/>"
                + "</cp:conditions>"
                + "<cp:actions>"
                + "<ss:forward-to>"
                + "<ss:target>tel:+1234567890</ss:target>"
                + "<ss:notify-caller>true</ss:notify-caller>"
                + "<ss:reveal-identity-to-caller>true</ss:reveal-identity-to-caller>"
                + "<ss:reveal-served-user-identity-to-caller>false"
                + "</ss:reveal-served-user-identity-to-caller>"
                + "<ss:notify-served-user>false</ss:notify-served-user>"
                + "<ss:notify-served-user-on-outbound-call>false"
                + "</ss:notify-served-user-on-outbound-call>"
                + "<ss:reveal-identity-to-target>false</ss:reveal-identity-to-target>"
                + "</ss:forward-to>"
                + "</cp:actions>"
                + "</cp:rule>"
                + "<cp:rule id=\"call-diversion-busy\">"
                + "<cp:conditions>"
                + "<ss:rule-deactivated/>"
                + "<ss:busy/>"
                + "</cp:conditions>"
                + "<cp:actions>"
                + "<ss:forward-to>"
                + "<ss:target>tel:+1234567890</ss:target>"
                + "<ss:notify-caller>true</ss:notify-caller>"
                + "<ss:reveal-identity-to-caller>true</ss:reveal-identity-to-caller>"
                + "<ss:reveal-served-user-identity-to-caller>false"
                + "</ss:reveal-served-user-identity-to-caller>"
                + "<ss:reveal-identity-to-target>false</ss:reveal-identity-to-target>"
                + "</ss:forward-to>"
                + "</cp:actions>"
                + "</cp:rule>"
                + "<cp:rule id=\"call-diversion-no-reply\">"
                + "<cp:conditions>"
                + "<ss:rule-deactivated/>"
                + "<ss:no-answer/>"
                + "</cp:conditions>"
                + "<cp:actions>"
                + "<ss:forward-to>"
                + "<ss:target>tel:+1234567890</ss:target>"
                + "<ss:notify-caller>true</ss:notify-caller>"
                + "<ss:reveal-identity-to-caller>true</ss:reveal-identity-to-caller>"
                + "<ss:reveal-served-user-identity-to-caller>false"
                + "</ss:reveal-served-user-identity-to-caller>"
                + "<ss:reveal-identity-to-target>false</ss:reveal-identity-to-target>"
                + "</ss:forward-to>"
                + "</cp:actions>"
                + "</cp:rule>"
                + "<cp:rule id=\"call-diversion-not-reachable\">"
                + "<cp:conditions>"
                + "<ss:rule-deactivated/>"
                + "<ss:not-reachable/>"
                + "</cp:conditions>"
                + "<cp:actions>"
                + "<ss:forward-to>"
                + "<ss:target>tel:+1234567890</ss:target>"
                + "<ss:notify-caller>true</ss:notify-caller>"
                + "<ss:reveal-identity-to-caller>true</ss:reveal-identity-to-caller>"
                + "<ss:reveal-served-user-identity-to-caller>false"
                + "</ss:reveal-served-user-identity-to-caller>"
                + "<ss:reveal-identity-to-target>false</ss:reveal-identity-to-target>"
                + "</ss:forward-to>"
                + "</cp:actions>"
                + "</cp:rule>"
                + "<cp:rule id=\"call-diversion-not-loggedin\">"
                + "<cp:conditions>"
                + "<ss:rule-deactivated/>"
                + "<ss:not-registered/>"
                + "</cp:conditions>"
                + "<cp:actions>"
                + "<ss:forward-to>"
                + "<ss:target>tel:+1234567890</ss:target>"
                + "<ss:notify-caller>true</ss:notify-caller>"
                + "<ss:reveal-identity-to-caller>true</ss:reveal-identity-to-caller>"
                + "<ss:reveal-served-user-identity-to-caller>false"
                + "</ss:reveal-served-user-identity-to-caller>"
                + "<ss:reveal-identity-to-target>false</ss:reveal-identity-to-target>"
                + "</ss:forward-to>"
                + "</cp:actions>"
                + "</cp:rule>"
                // CD - CFU audio
                + "<cp:rule id=\"call-diversion-unconditional-audio\">"
                + "<cp:conditions>"
                + "<ss:rule-deactivated/>"
                + "<ss:media>audio</ss:media>"
                + "</cp:conditions>"
                + "<cp:actions>"
                + "<ss:forward-to>"
                + "<ss:target>tel:+1234567890</ss:target>"
                + "<ss:notify-caller>true</ss:notify-caller>"
                + "<ss:reveal-identity-to-caller>true</ss:reveal-identity-to-caller>"
                + "<ss:reveal-served-user-identity-to-caller>false"
                + "</ss:reveal-served-user-identity-to-caller>"
                + "<ss:notify-served-user>false</ss:notify-served-user>"
                + "<ss:notify-served-user-on-outbound-call>false"
                + "</ss:notify-served-user-on-outbound-call>"
                + "<ss:reveal-identity-to-target>false</ss:reveal-identity-to-target>"
                + "</ss:forward-to>"
                + "</cp:actions>"
                + "</cp:rule>"
                // CD video
                + "<cp:rule id=\"call-diversion-unconditional-video\">"
                + "<cp:conditions>"
                + "<ss:rule-deactivated/>"
                + "<ss:media>video</ss:media>"
                + "</cp:conditions>"
                + "<cp:actions>"
                + "<ss:forward-to>"
                + "<ss:target>tel:+1234567890</ss:target>"
                + "<ss:notify-caller>true</ss:notify-caller>"
                + "<ss:reveal-identity-to-caller>true</ss:reveal-identity-to-caller>"
                + "<ss:reveal-served-user-identity-to-caller>false"
                + "</ss:reveal-served-user-identity-to-caller>"
                + "<ss:notify-served-user>false</ss:notify-served-user>"
                + "<ss:notify-served-user-on-outbound-call>false"
                + "</ss:notify-served-user-on-outbound-call>"
                + "<ss:reveal-identity-to-target>false</ss:reveal-identity-to-target>"
                + "</ss:forward-to>"
                + "</cp:actions>"
                + "</cp:rule>"
                + "<cp:rule id=\"call-diversion-busy-video\">"
                + "<cp:conditions>"
                + "<ss:rule-deactivated/>"
                + "<ss:busy/>"
                + "<ss:media>video</ss:media>"
                + "</cp:conditions>"
                + "<cp:actions>"
                + "<ss:forward-to>"
                + "<ss:target>tel:+1234567890</ss:target>"
                + "<ss:notify-caller>true</ss:notify-caller>"
                + "<ss:reveal-identity-to-caller>true</ss:reveal-identity-to-caller>"
                + "<ss:reveal-served-user-identity-to-caller>false"
                + "</ss:reveal-served-user-identity-to-caller>"
                + "<ss:reveal-identity-to-target>false</ss:reveal-identity-to-target>"
                + "</ss:forward-to>"
                + "</cp:actions>"
                + "</cp:rule>"
                + "<cp:rule id=\"call-diversion-no-reply-video\">"
                + "<cp:conditions>"
                + "<ss:media>video</ss:media>"
                + "<ss:no-answer/>"
                + "</cp:conditions>"
                + "<cp:actions>"
                + "<ss:forward-to>"
                + "<ss:target>tel:+1234567890</ss:target>"
                + "<ss:notify-caller>true</ss:notify-caller>"
                + "<ss:reveal-identity-to-caller>true</ss:reveal-identity-to-caller>"
                + "<ss:reveal-served-user-identity-to-caller>false"
                + "</ss:reveal-served-user-identity-to-caller>"
                + "<ss:reveal-identity-to-target>false</ss:reveal-identity-to-target>"
                + "</ss:forward-to>"
                + "</cp:actions>"
                + "</cp:rule>"
                + "<cp:rule id=\"call-diversion-not-reachable-video\">"
                + "<cp:conditions>"
                + "<ss:rule-deactivated/>"
                + "<ss:not-reachable/>"
                + "<ss:media>video</ss:media>"
                + "</cp:conditions>"
                + "<cp:actions>"
                + "<ss:forward-to>"
                + "<ss:target>tel:+1234567890</ss:target>"
                + "<ss:notify-caller>true</ss:notify-caller>"
                + "<ss:reveal-identity-to-caller>true</ss:reveal-identity-to-caller>"
                + "<ss:reveal-served-user-identity-to-caller>false"
                + "</ss:reveal-served-user-identity-to-caller>"
                + "<ss:reveal-identity-to-target>false</ss:reveal-identity-to-target>"
                + "</ss:forward-to>"
                + "</cp:actions>"
                + "</cp:rule>"
                + "<cp:rule id=\"call-diversion-not-loggedin-video\">"
                + "<cp:conditions>"
                + "<ss:rule-deactivated/>"
                + "<ss:not-registered/>"
                + "<ss:media>video</ss:media>"
                + "</cp:conditions>"
                + "<cp:actions>"
                + "<ss:forward-to>"
                + "<ss:target>tel:+1234567890</ss:target>"
                + "<ss:notify-caller>true</ss:notify-caller>"
                + "<ss:reveal-identity-to-caller>true</ss:reveal-identity-to-caller>"
                + "<ss:reveal-served-user-identity-to-caller>false"
                + "</ss:reveal-served-user-identity-to-caller>"
                + "<ss:reveal-identity-to-target>false</ss:reveal-identity-to-target>"
                + "</ss:forward-to>"
                + "</cp:actions>"
                + "</cp:rule>"
                + "</cp:ruleset>"
                + "</ss:communication-diversion>"
                // CB service capability
                + "<ss:communication-barring-serv-cap active=\"true\">"
                + "<ss:serv-cap-conditions>"
                + "<ss:serv-cap-anonymous provisioned=\"false\"/>"
                + "<ss:serv-cap-communication-diverted provisioned=\"false\"/>"
                + "<ss:serv-cap-external-list provisioned=\"false\"/>"
                + "<ss:serv-cap-identity provisioned=\"false\"/>"
                + "<ss:serv-cap-international provisioned=\"true\"/>"
                + "<ss:serv-cap-international-exHC provisioned=\"true\"/>"
                + "<ss:serv-cap-unconditional provisioned=\"false\"/>"
                + "<ss:serv-cap-media>"
                + "<ss:media>audio</ss:media>"
                + "<ss:media>video</ss:media>"
                + "<ss:media>message</ss:media>"
                + "<ss:media>image</ss:media>"
                + "</ss:serv-cap-media>"
                + "<ss:serv-cap-other-identity provisioned=\"false\"/>"
                + "<ss:serv-cap-presence-status provisioned=\"false\"/>"
                + "<ss:serv-cap-roaming provisioned=\"true\"/>"
                + "<ss:serv-cap-rule-deactivated provisioned=\"true\"/>"
                + "<ss:serv-cap-validity provisioned=\"false\"/>"
                + "</ss:serv-cap-conditions>"
                + "</ss:communication-barring-serv-cap>"
                // CD service capability
                + "<ss:communication-diversion-serv-cap active=\"true\">"
                + "<ss:serv-cap-conditions>"
                + "<ss:serv-cap-anonymous provisioned=\"false\"/>"
                + "<ss:serv-cap-busy provisioned=\"true\"/>"
                + "<ss:serv-cap-external-list provisioned=\"false\"/>"
                + "<ss:serv-cap-identity provisioned=\"false\"/>"
                + "<ss:serv-cap-media>"
                + "<ss:media>audio</ss:media>"
                + "<ss:media>video</ss:media>"
                + "</ss:serv-cap-media>"
                + "<ss:serv-cap-not-registered provisioned=\"false\"/>"
                + "<ss:serv-cap-no-answer provisioned=\"false\"/>"
                + "<ss:serv-cap-not-reachable provisioned=\"true\"/>"
                + "<ss:serv-cap-presence-status provisioned=\"false\"/>"
                + "<ss:serv-cap-rule-deactivated provisioned=\"true\"/>"
                + "<ss:serv-cap-selective provisioned=\"false\"/>"
                + "<ss:serv-cap-validity provisioned=\"false\"/>"
                + "<ss:serv-cap-default provisioned=\"true\"/>"
                + "</ss:serv-cap-conditions>"
                + "<ss:serv-cap-actions>"
                + "<ss:serv-cap-target>"
                + "<ss:telephony-type/>"
                + "</ss:serv-cap-target>"
                + "<ss:serv-cap-notify-caller provisioned=\"false\"/>"
                + "<ss:serv-cap-notify-served-user provisioned=\"false\"/>"
                + "<ss:serv-cap-notify-served-user-on-outbound-call provisioned=\"false\"/>"
                + "<ss:serv-cap-reveal-identity-to-caller provisioned=\"false\"/>"
                + "<ss:serv-cap-reveal-served-user-identity-to-caller provisioned=\"false\"/>"
                + "<ss:serv-cap-reveal-identity-to-target provisioned=\"false\"/>"
                + "</ss:serv-cap-actions>"
                + "</ss:communication-diversion-serv-cap>"
                // OIP
                + "<ss:originating-identity-presentation active=\"true\"/>"
                // OIR
                + "<ss:originating-identity-presentation-restriction active=\"false\">"
                + "<ss:default-behaviour>presentation-not-restricted</ss:default-behaviour>"
                + "</ss:originating-identity-presentation-restriction>"
                // TIP
                + "<ss:terminating-identity-presentation active=\"false\"/>"
                // TIR
                + "<ss:terminating-identity-presentation-restriction>"
                + "<ss:default-behaviour>presentation-not-restricted</ss:default-behaviour>"
                + "</ss:terminating-identity-presentation-restriction>"
                // Etc
                + "<mmt-serv:user-common-data xmlns:mmt-serv=\"http://schemas.ericsson.com/mmtel/services\">"
                + "<mmt-serv:target-list fixed-targets=\"true\">"
                + "<mmt-serv:target id=\"tel:+61482880000\" name=\"Secondary Device 1\"/>"
                + "</mmt-serv:target-list>"
                + "</mmt-serv:user-common-data>"
                + "</ss:simservs>";

        Document document;
        try {
            DocumentBuilderFactory factory = DocumentBuilderFactory.newInstance();
            factory.setNamespaceAware(false);
            DocumentBuilder builder = factory.newDocumentBuilder();
            InputSource is = new InputSource(new StringReader(xml));
            document = builder.parse(is);
        } catch (Exception e) {
            e.printStackTrace();
            return null;
        }

        return document;
    }

    private synchronized void replaceInstance(final Class c, final String instanceName,
            final Object obj, final Object newValue) {
        try {
            java.lang.reflect.Field field = c.getDeclaredField(instanceName);
            field.setAccessible(true);
            field.set(obj, newValue);
        } catch (Exception e) {
            org.junit.Assert.fail(e.toString());
        }
    }
}
