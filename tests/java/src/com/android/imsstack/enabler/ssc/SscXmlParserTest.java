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
import static org.mockito.ArgumentMatchers.eq;
import static org.mockito.Mockito.when;

import com.android.imsstack.core.agents.ConfigAgent;
import com.android.imsstack.core.config.CarrierConfig;
import com.android.imsstack.enabler.ssc.data.CfServiceData;
import com.android.imsstack.enabler.ssc.data.SscRuleData;
import com.android.imsstack.enabler.ssc.data.SscServiceQueryData;

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

    @Mock private CarrierConfig mMockCarrierConfig;
    @Mock private ConfigAgent mMockConfigAgent;

    @Before
    public void setup() {
        MockitoAnnotations.initMocks(this);

        SscXmlFormat.init(SLOT_0);
        mSscXmlParser = new SscXmlParser();

        SscConfig.setConfigAgent(SLOT_0, mMockConfigAgent);
        when(mMockConfigAgent.getCarrierConfig()).thenReturn(mMockCarrierConfig);
    }

    @After
    public void tearDown() {
    }

    @Test
    public void getSscServiceFromDoc_cfnrData() {
        SscServiceQueryData documentQueryData = getDocumentQueryData();
        documentQueryData.setResponseCode(SscConstant.HTTP_OK);
        mSscXmlParser.getSscServiceFromDoc(documentQueryData, getEntireXmlDoc(), null);

        SscServiceQueryData queryData = getCfQueryData(SscConstant.CONDITION_CFU);
        queryData.setResponseCode(SscConstant.HTTP_OK);
        CfServiceData data = (CfServiceData) mSscXmlParser.getSscServiceFromDoc(queryData,
                getEntireXmlDoc(), null);

        assertEquals(25, data.getNoReplyTimer());
        assertEquals(SscConstant.CONDITION_CFU, data.getCondition());
        assertNotNull(data.getRuleSet());
        assertEquals(1, data.getRuleSet().size());

        SscRuleData ruleData = data.getRuleSet().get(0);
        assertEquals(SscConstant.CONDITION_CFU, ruleData.getSsCondition());
        assertEquals(SscConstant.STATUS_DISABLE , ruleData.getState());
        assertEquals("+1234567890", ruleData.getForwardToNumber());
    }

    @Test
    public void getSscServiceFromDoc_serviceCapability() {
        when(mMockCarrierConfig.getBoolean(eq(CarrierConfig.Assets.KEY_UT_INSERT_NEW_RULE_BOOL)))
                .thenReturn(true);

        SscServiceQueryData documentQueryData = getDocumentQueryData();
        documentQueryData.setResponseCode(SscConstant.HTTP_OK);
        mSscXmlParser.getSscServiceFromDoc(documentQueryData, getEntireXmlDoc(), null);

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
        assertEquals(false, SscXmlFormat.getMediaCapability(SLOT_0, SscXmlFormat.SC_CD,
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
    }

    private SscServiceQueryData getDocumentQueryData() {
        return SscXmlGovTest.createDocumentQueryData();
    }

    private SscServiceQueryData getCfQueryData(int condition) {
        return SscXmlGovTest.createQueryData(ESsType.CF, 0, condition);
    }

    private Document getEntireXmlDoc() {
        return SscXmlGovTest.createEntireXmlDoc(false);
    }
}
