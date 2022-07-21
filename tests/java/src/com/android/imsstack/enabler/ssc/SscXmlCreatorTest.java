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

import com.android.imsstack.core.agents.ConfigAgent;
import com.android.imsstack.core.config.CarrierConfig;
import com.android.imsstack.enabler.ssc.data.SscServiceData;

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

@RunWith(JUnit4.class)
public class SscXmlCreatorTest {
    private static final int SLOT_0 = 0;

    private String mTargetNumber = "+1234567890";

    private SscXmlCreator mSscXmlCreator;
    private SscXmlParser mSscXmlParser;

    @Mock private CarrierConfig mMockCarrierConfig;
    @Mock private ConfigAgent mMockConfigAgent;
    @Mock private SscUtils mMockSscUtils;

    @Before
    public void setup() {
        MockitoAnnotations.initMocks(this);

        SscXmlFormat.init(SLOT_0);
        mSscXmlCreator = new FakeSscXmlCreator();
        mSscXmlParser = new SscXmlParser();

        SscConfig.setConfigAgent(SLOT_0, mMockConfigAgent);
        when(mMockConfigAgent.getCarrierConfig()).thenReturn(mMockCarrierConfig);
        when(mMockSscUtils.getUriFromNumber(eq(SLOT_0), eq(mTargetNumber)))
                .thenReturn(mTargetNumber);
    }

    @After
    public void tearDown() {
    }

    @Test
    public void createXml_insertCfWhenNotProvisioned() {
        int condition = SscConstant.CONDITION_CFB;
        int action = SscConstant.ACTION_ACTIVATION;
        String ruleId = SscXmlFormat.getDefaultRuleId(SLOT_0, SscXmlFormat.MEDIA_TYPE_AUDIO,
                SscXmlFormat.CD, condition);
        Document doc = removeRule(getEntireXmlDoc(), ruleId);
        updateTagsAndRules(doc);
        SscXmlFormat.setProvisionStatus(SLOT_0, SscXmlFormat.SC_CD, condition, false);

        SscServiceData insertData = getInsertCfData(ESsType.CF, action, condition, null);
        Element serviceElement = mSscXmlCreator.createXml(doc, insertData);
        assertNull(serviceElement);
    }

    @Test
    public void createXml_insertCfbActivationWithoutAudioTag() {
        int condition = SscConstant.CONDITION_CFB;
        int action = SscConstant.ACTION_ACTIVATION;
        String targetNumber = null;
        String ruleId = SscXmlFormat.getDefaultRuleId(SLOT_0, SscXmlFormat.MEDIA_TYPE_AUDIO,
                SscXmlFormat.CD, condition);
        Document doc = removeRule(getEntireXmlDoc(), ruleId);
        updateTagsAndRules(doc);
        String ruleConditionTag = SscXmlFormat.getRuleConditionTag(SLOT_0, SscXmlFormat.CD,
                condition);
        SscXmlFormat.setProvisionStatus(SLOT_0, SscXmlFormat.SC_CD, condition, true);
        SscXmlFormat.setMediaCapability(SLOT_0, SscXmlFormat.SC_CD, SscXmlFormat.MEDIA_TYPE_AUDIO,
                false);

        when(mMockCarrierConfig.getBoolean(eq(CarrierConfig.Assets.KEY_UT_INSERT_NEW_RULE_BOOL)))
                .thenReturn(true);

        SscServiceData insertData = getInsertCfData(ESsType.CF, action, condition, targetNumber);
        Element xml = mSscXmlCreator.createXml(doc, insertData);
        assertNotNull(xml);
        assertEquals(1, xml.getElementsByTagName(
                SscXmlFormat.getCpElement(SLOT_0, SscXmlFormat.RULESET)).getLength());

        NodeList ruleList = xml.getElementsByTagName(SscXmlFormat.getCpElement(SLOT_0,
                SscXmlFormat.RULE));
        assertNotNull(ruleList);
        Element createdRule = null;
        for (int i = 0; i < ruleList.getLength(); i++) {
            Element element = (Element) ruleList.item(i);
            if (ruleId.equals(element.getAttribute(SscXmlFormat.ID))) {
                createdRule = element;
                break;
            }
        }

        assertNotNull(createdRule);
        assertEquals(1, createdRule.getElementsByTagName(
                SscXmlFormat.getCpElement(SLOT_0, SscXmlFormat.CONDITIONS)).getLength());
        assertEquals(1, createdRule.getElementsByTagName(ruleConditionTag).getLength());
        assertEquals(0, createdRule.getElementsByTagName(
                SscXmlFormat.getSsElement(SLOT_0, SscXmlFormat.RULE_DEACTIVATED)).getLength());
        assertEquals(0, createdRule.getElementsByTagName(
                SscXmlFormat.getSsElement(SLOT_0, SscXmlFormat.TARGET)).getLength());
        assertEquals(0, createdRule.getElementsByTagName(
                SscXmlFormat.getSsElement(SLOT_0, SscXmlFormat.MEDIA)).getLength());
    }

    @Test
    public void createXml_insertCfbActivationWithAudioTag() {
        int condition = SscConstant.CONDITION_CFB;
        int action = SscConstant.ACTION_ACTIVATION;
        String targetNumber = null;
        String ruleId = SscXmlFormat.getDefaultRuleId(SLOT_0, SscXmlFormat.MEDIA_TYPE_AUDIO,
                SscXmlFormat.CD, condition);
        Document doc = removeRule(getEntireXmlDoc(), ruleId);
        updateTagsAndRules(doc);
        String ruleConditionTag = SscXmlFormat.getRuleConditionTag(SLOT_0, SscXmlFormat.CD,
                condition);
        SscXmlFormat.setProvisionStatus(SLOT_0, SscXmlFormat.SC_CD, condition, true);
        SscXmlFormat.setMediaCapability(SLOT_0, SscXmlFormat.SC_CD, SscXmlFormat.MEDIA_TYPE_AUDIO,
                true);

        when(mMockCarrierConfig.getBoolean(eq(CarrierConfig.Assets.KEY_UT_INSERT_NEW_RULE_BOOL)))
                .thenReturn(true);

        SscServiceData insertData = getInsertCfData(ESsType.CF, action, condition, targetNumber);
        Element xml = mSscXmlCreator.createXml(doc, insertData);
        assertNotNull(xml);
        assertEquals(1, xml.getElementsByTagName(
                SscXmlFormat.getCpElement(SLOT_0, SscXmlFormat.RULESET)).getLength());

        NodeList ruleList = xml.getElementsByTagName(SscXmlFormat.getCpElement(SLOT_0,
                SscXmlFormat.RULE));
        assertNotNull(ruleList);
        Element createdRule = null;
        for (int i = 0; i < ruleList.getLength(); i++) {
            Element element = (Element) ruleList.item(i);
            if (ruleId.equals(element.getAttribute(SscXmlFormat.ID))) {
                createdRule = element;
                break;
            }
        }

        assertNotNull(createdRule);
        assertEquals(1, createdRule.getElementsByTagName(
                SscXmlFormat.getCpElement(SLOT_0, SscXmlFormat.CONDITIONS)).getLength());
        assertEquals(1, createdRule.getElementsByTagName(ruleConditionTag).getLength());
        assertEquals(0, createdRule.getElementsByTagName(
                SscXmlFormat.getSsElement(SLOT_0, SscXmlFormat.RULE_DEACTIVATED)).getLength());
        assertEquals(0, createdRule.getElementsByTagName(
                SscXmlFormat.getSsElement(SLOT_0, SscXmlFormat.TARGET)).getLength());

        NodeList mediaNodeList = createdRule.getElementsByTagName(
                SscXmlFormat.getSsElement(SLOT_0, SscXmlFormat.MEDIA));
        assertEquals(1, mediaNodeList.getLength());
        assertEquals(SscXmlFormat.AUDIO, mediaNodeList.item(0).getTextContent());
    }

    @Test
    public void createXml_insertCfnrRegistration() {
        int condition = SscConstant.CONDITION_CFNR;
        int action = SscConstant.ACTION_REGISTRATION;
        String targetNumber = mTargetNumber;
        String ruleId = SscXmlFormat.getDefaultRuleId(SLOT_0, SscXmlFormat.MEDIA_TYPE_AUDIO,
                SscXmlFormat.CD, condition);
        Document doc = removeRule(getEntireXmlDoc(), ruleId);
        updateTagsAndRules(doc);
        String ruleConditionTag = SscXmlFormat.getRuleConditionTag(SLOT_0, SscXmlFormat.CD,
                condition);
        SscXmlFormat.setProvisionStatus(SLOT_0, SscXmlFormat.SC_CD, condition, true);

        when(mMockCarrierConfig.getBoolean(eq(CarrierConfig.Assets.KEY_UT_INSERT_NEW_RULE_BOOL)))
                .thenReturn(true);

        SscServiceData insertData = getInsertCfData(ESsType.CF, action, condition, targetNumber);
        Element xml = mSscXmlCreator.createXml(doc, insertData);

        assertNotNull(xml);
        assertEquals(1, xml.getElementsByTagName(
                SscXmlFormat.getCpElement(SLOT_0, SscXmlFormat.RULESET)).getLength());

        NodeList ruleList = xml.getElementsByTagName(SscXmlFormat.getCpElement(SLOT_0,
                SscXmlFormat.RULE));
        assertNotNull(ruleList);
        Element createdRule = null;
        for (int i = 0; i < ruleList.getLength(); i++) {
            Element element = (Element) ruleList.item(i);
            if (ruleId.equals(element.getAttribute(SscXmlFormat.ID))) {
                createdRule = element;
                break;
            }
        }

        assertNotNull(createdRule);
        assertEquals(1, createdRule.getElementsByTagName(
                SscXmlFormat.getCpElement(SLOT_0, SscXmlFormat.CONDITIONS)).getLength());
        assertEquals(1, createdRule.getElementsByTagName(ruleConditionTag).getLength());
        assertEquals(0, createdRule.getElementsByTagName(
                SscXmlFormat.getSsElement(SLOT_0, SscXmlFormat.RULE_DEACTIVATED)).getLength());

        NodeList targetList = createdRule.getElementsByTagName(
                SscXmlFormat.getSsElement(SLOT_0, SscXmlFormat.TARGET));
        assertEquals(1, targetList.getLength());
        assertEquals(targetNumber, targetList.item(0).getTextContent());
    }

    @Test
    public void createXml_insertCfuDeactivation() {
        int condition = SscConstant.CONDITION_CFU;
        int action = SscConstant.ACTION_DEACTIVATION;
        String targetNumber = null;
        String ruleId = SscXmlFormat.getDefaultRuleId(SLOT_0, SscXmlFormat.MEDIA_TYPE_AUDIO,
                SscXmlFormat.CD, condition);
        Document doc = removeRule(getEntireXmlDoc(), ruleId);
        updateTagsAndRules(doc);
        SscXmlFormat.setProvisionStatus(SLOT_0, SscXmlFormat.SC_CD, condition, true);

        when(mMockCarrierConfig.getBoolean(eq(CarrierConfig.Assets.KEY_UT_INSERT_NEW_RULE_BOOL)))
                .thenReturn(true);

        SscServiceData insertData = getInsertCfData(ESsType.CF, action, condition, targetNumber);
        Element xml = mSscXmlCreator.createXml(doc, insertData);

        assertNotNull(xml);
        assertEquals(1, xml.getElementsByTagName(
                SscXmlFormat.getCpElement(SLOT_0, SscXmlFormat.RULESET)).getLength());

        NodeList ruleList = xml.getElementsByTagName(SscXmlFormat.getCpElement(SLOT_0,
                SscXmlFormat.RULE));
        assertNotNull(ruleList);
        Element createdRule = null;
        for (int i = 0; i < ruleList.getLength(); i++) {
            Element element = (Element) ruleList.item(i);
            if (ruleId.equals(element.getAttribute(SscXmlFormat.ID))) {
                createdRule = element;
                break;
            }
        }

        assertNotNull(createdRule);
        NodeList conditionNodeList = createdRule.getElementsByTagName(
                SscXmlFormat.getCpElement(SLOT_0, SscXmlFormat.CONDITIONS));
        assertEquals(1, conditionNodeList.getLength());
        Element conditionElement = (Element) conditionNodeList.item(0);
        assertEquals(1, conditionElement.getChildNodes().getLength());
        assertEquals(1, conditionElement.getElementsByTagName(
                SscXmlFormat.getSsElement(SLOT_0, SscXmlFormat.RULE_DEACTIVATED)).getLength());
        assertEquals(0, createdRule.getElementsByTagName(
                SscXmlFormat.getSsElement(SLOT_0, SscXmlFormat.TARGET)).getLength());
    }

    @Test
    public void createXml_insertCfnrcErasure() {
        int condition = SscConstant.CONDITION_CFNRC;
        int action = SscConstant.ACTION_ERASURE;
        String targetNumber = "";
        String ruleId = SscXmlFormat.getDefaultRuleId(SLOT_0, SscXmlFormat.MEDIA_TYPE_AUDIO,
                SscXmlFormat.CD, condition);
        Document doc = removeRule(getEntireXmlDoc(), ruleId);
        updateTagsAndRules(doc);
        String ruleConditionTag = SscXmlFormat.getRuleConditionTag(SLOT_0, SscXmlFormat.CD,
                condition);
        SscXmlFormat.setProvisionStatus(SLOT_0, SscXmlFormat.SC_CD, condition, true);

        when(mMockCarrierConfig.getBoolean(eq(CarrierConfig.Assets.KEY_UT_INSERT_NEW_RULE_BOOL)))
                .thenReturn(true);

        SscServiceData insertData = getInsertCfData(ESsType.CF, action, condition, targetNumber);
        Element xml = mSscXmlCreator.createXml(doc, insertData);

        assertNotNull(xml);
        assertEquals(1, xml.getElementsByTagName(
                SscXmlFormat.getCpElement(SLOT_0, SscXmlFormat.RULESET)).getLength());

        NodeList ruleList = xml.getElementsByTagName(SscXmlFormat.getCpElement(SLOT_0,
                SscXmlFormat.RULE));
        assertNotNull(ruleList);
        Element createdRule = null;
        for (int i = 0; i < ruleList.getLength(); i++) {
            Element element = (Element) ruleList.item(i);
            if (ruleId.equals(element.getAttribute(SscXmlFormat.ID))) {
                createdRule = element;
                break;
            }
        }

        assertNotNull(createdRule);
        assertEquals(1, createdRule.getElementsByTagName(
                SscXmlFormat.getCpElement(SLOT_0, SscXmlFormat.CONDITIONS)).getLength());
        assertEquals(1, createdRule.getElementsByTagName(ruleConditionTag).getLength());
        assertEquals(1, createdRule.getElementsByTagName(
                SscXmlFormat.getSsElement(SLOT_0, SscXmlFormat.RULE_DEACTIVATED)).getLength());

        NodeList targetList = createdRule.getElementsByTagName(
                SscXmlFormat.getSsElement(SLOT_0, SscXmlFormat.TARGET));
        assertEquals(1, targetList.getLength());
        assertEquals(targetNumber, targetList.item(0).getTextContent());
    }

    @Test
    public void createXml_insertCfbActivation() {
        int condition = SscConstant.CONDITION_CFB;
        int action = SscConstant.ACTION_ACTIVATION;
        String targetNumber = null;
        String ruleId = SscXmlFormat.getDefaultRuleId(SLOT_0, SscXmlFormat.MEDIA_TYPE_AUDIO,
                SscXmlFormat.CD, condition);
        Document doc = removeRule(getEntireXmlDoc(), ruleId);
        updateTagsAndRules(doc);
        String ruleConditionTag = SscXmlFormat.getRuleConditionTag(SLOT_0, SscXmlFormat.CD,
                condition);
        SscXmlFormat.setProvisionStatus(SLOT_0, SscXmlFormat.SC_CD, condition, true);

        when(mMockCarrierConfig.getBoolean(eq(CarrierConfig.Assets.KEY_UT_INSERT_NEW_RULE_BOOL)))
                .thenReturn(true);

        SscServiceData insertData = getInsertCfData(ESsType.CF, action, condition, targetNumber);
        Element xml = mSscXmlCreator.createXml(doc, insertData);
        assertNotNull(xml);
        assertEquals(1, xml.getElementsByTagName(
                SscXmlFormat.getCpElement(SLOT_0, SscXmlFormat.RULESET)).getLength());

        NodeList ruleList = xml.getElementsByTagName(SscXmlFormat.getCpElement(SLOT_0,
                SscXmlFormat.RULE));
        assertNotNull(ruleList);
        Element createdRule = null;
        for (int i = 0; i < ruleList.getLength(); i++) {
            Element element = (Element) ruleList.item(i);
            if (ruleId.equals(element.getAttribute(SscXmlFormat.ID))) {
                createdRule = element;
                break;
            }
        }

        assertNotNull(createdRule);
        assertEquals(1, createdRule.getElementsByTagName(
                SscXmlFormat.getCpElement(SLOT_0, SscXmlFormat.CONDITIONS)).getLength());
        assertEquals(1, createdRule.getElementsByTagName(ruleConditionTag).getLength());
        assertEquals(0, createdRule.getElementsByTagName(
                SscXmlFormat.getSsElement(SLOT_0, SscXmlFormat.RULE_DEACTIVATED)).getLength());
        assertEquals(0, createdRule.getElementsByTagName(
                SscXmlFormat.getSsElement(SLOT_0, SscXmlFormat.TARGET)).getLength());
    }

    @Test
    public void createXml_insertCbWhenNotProvisioned() {
        int condition = SscConstant.CONDITION_BAIC;
        int action = SscConstant.STATUS_ENABLE;
        String ruleId = SscXmlFormat.getDefaultRuleId(SLOT_0, SscXmlFormat.MEDIA_TYPE_AUDIO,
                SscXmlFormat.ICB, condition);
        Document doc = removeRule(getEntireXmlDoc(), ruleId);
        updateTagsAndRules(doc);
        SscXmlFormat.setProvisionStatus(SLOT_0, SscXmlFormat.SC_CB, condition, false);

        SscServiceData insertData = getInsertCbData(ESsType.ICB, action, condition);
        Element serviceElement = mSscXmlCreator.createXml(doc, insertData);
        assertNull(serviceElement);
    }

    @Test
    public void createXml_insertBaicEnable() {
        int condition = SscConstant.CONDITION_BAIC;
        int action = SscConstant.STATUS_ENABLE;
        String ruleId = SscXmlFormat.getDefaultRuleId(SLOT_0, SscXmlFormat.MEDIA_TYPE_AUDIO,
                SscXmlFormat.ICB, condition);
        Document doc = removeRule(getEntireXmlDoc(), ruleId);
        updateTagsAndRules(doc);
        SscXmlFormat.setProvisionStatus(SLOT_0, SscXmlFormat.SC_CB, condition, true);

        when(mMockCarrierConfig.getBoolean(eq(CarrierConfig.Assets.KEY_UT_INSERT_NEW_RULE_BOOL)))
                .thenReturn(true);

        SscServiceData insertData = getInsertCbData(ESsType.ICB, action, condition);
        Element xml = mSscXmlCreator.createXml(doc, insertData);
        assertNotNull(xml);
        assertEquals(1, xml.getElementsByTagName(
                SscXmlFormat.getCpElement(SLOT_0, SscXmlFormat.RULESET)).getLength());

        NodeList ruleList = xml.getElementsByTagName(SscXmlFormat.getCpElement(SLOT_0,
                SscXmlFormat.RULE));
        assertNotNull(ruleList);
        Element createdRule = null;
        for (int i = 0; i < ruleList.getLength(); i++) {
            Element element = (Element) ruleList.item(i);
            if (ruleId.equals(element.getAttribute(SscXmlFormat.ID))) {
                createdRule = element;
                break;
            }
        }

        assertNotNull(createdRule);
        NodeList conditionNodeList = createdRule.getElementsByTagName(
                SscXmlFormat.getCpElement(SLOT_0, SscXmlFormat.CONDITIONS));
        assertEquals(1, conditionNodeList.getLength());
        Element conditionElement = (Element) conditionNodeList.item(0);
        assertEquals(0, conditionElement.getChildNodes().getLength());
        assertEquals(0, createdRule.getElementsByTagName(
                SscXmlFormat.getSsElement(SLOT_0, SscXmlFormat.RULE_DEACTIVATED)).getLength());

        NodeList actionNodeList = createdRule.getElementsByTagName(
                SscXmlFormat.getCpElement(SLOT_0, SscXmlFormat.ACTIONS));
        assertEquals(1, actionNodeList.getLength());
        Element actionElement = (Element) actionNodeList.item(0);
        NodeList allowNodeList = actionElement.getElementsByTagName(
                SscXmlFormat.getSsElement(SLOT_0, SscXmlFormat.ALLOW));
        assertEquals(1, allowNodeList.getLength());
        assertEquals("false", allowNodeList.item(0).getTextContent());
    }

    @Test
    public void createXml_insertBoicEnable() {
        int condition = SscConstant.CONDITION_BOIC;
        int action = SscConstant.STATUS_ENABLE;
        String ruleId = SscXmlFormat.getDefaultRuleId(SLOT_0, SscXmlFormat.MEDIA_TYPE_AUDIO,
                SscXmlFormat.OCB, condition);
        Document doc = removeRule(getEntireXmlDoc(), ruleId);
        updateTagsAndRules(doc);
        String ruleConditionTag = SscXmlFormat.getRuleConditionTag(SLOT_0, SscXmlFormat.OCB,
                condition);
        SscXmlFormat.setProvisionStatus(SLOT_0, SscXmlFormat.SC_CB, condition, true);

        when(mMockCarrierConfig.getBoolean(eq(CarrierConfig.Assets.KEY_UT_INSERT_NEW_RULE_BOOL)))
                .thenReturn(true);

        SscServiceData insertData = getInsertCbData(ESsType.OCB, action, condition);
        Element xml = mSscXmlCreator.createXml(doc, insertData);
        assertNotNull(xml);
        assertEquals(1, xml.getElementsByTagName(
                SscXmlFormat.getCpElement(SLOT_0, SscXmlFormat.RULESET)).getLength());

        NodeList ruleList = xml.getElementsByTagName(SscXmlFormat.getCpElement(SLOT_0,
                SscXmlFormat.RULE));
        assertNotNull(ruleList);
        Element createdRule = null;
        for (int i = 0; i < ruleList.getLength(); i++) {
            Element element = (Element) ruleList.item(i);
            if (ruleId.equals(element.getAttribute(SscXmlFormat.ID))) {
                createdRule = element;
                break;
            }
        }

        assertNotNull(createdRule);
        assertEquals(1, createdRule.getElementsByTagName(
                SscXmlFormat.getCpElement(SLOT_0, SscXmlFormat.CONDITIONS)).getLength());
        assertEquals(1, createdRule.getElementsByTagName(ruleConditionTag).getLength());
        assertEquals(0, createdRule.getElementsByTagName(
                SscXmlFormat.getSsElement(SLOT_0, SscXmlFormat.RULE_DEACTIVATED)).getLength());

        NodeList actionNodeList = createdRule.getElementsByTagName(
                SscXmlFormat.getCpElement(SLOT_0, SscXmlFormat.ACTIONS));
        assertEquals(1, actionNodeList.getLength());
        Element actionElement = (Element) actionNodeList.item(0);
        NodeList allowNodeList = actionElement.getElementsByTagName(
                SscXmlFormat.getSsElement(SLOT_0, SscXmlFormat.ALLOW));
        assertEquals(1, allowNodeList.getLength());
        assertEquals("false", allowNodeList.item(0).getTextContent());
    }

    @Test
    public void createXml_insertBicWrDisable() {
        int condition = SscConstant.CONDITION_BIC_WR;
        int action = SscConstant.STATUS_DISABLE;
        String ruleId = SscXmlFormat.getDefaultRuleId(SLOT_0, SscXmlFormat.MEDIA_TYPE_AUDIO,
                SscXmlFormat.ICB, condition);
        Document doc = removeRule(getEntireXmlDoc(), ruleId);
        updateTagsAndRules(doc);
        String ruleConditionTag = SscXmlFormat.getRuleConditionTag(SLOT_0, SscXmlFormat.ICB,
                condition);
        SscXmlFormat.setProvisionStatus(SLOT_0, SscXmlFormat.SC_CB, condition, true);

        when(mMockCarrierConfig.getBoolean(eq(CarrierConfig.Assets.KEY_UT_INSERT_NEW_RULE_BOOL)))
                .thenReturn(true);

        SscServiceData insertData = getInsertCbData(ESsType.ICB, action, condition);
        Element xml = mSscXmlCreator.createXml(doc, insertData);
        assertNotNull(xml);
        assertEquals(1, xml.getElementsByTagName(
                SscXmlFormat.getCpElement(SLOT_0, SscXmlFormat.RULESET)).getLength());

        NodeList ruleList = xml.getElementsByTagName(SscXmlFormat.getCpElement(SLOT_0,
                SscXmlFormat.RULE));
        assertNotNull(ruleList);
        Element createdRule = null;
        for (int i = 0; i < ruleList.getLength(); i++) {
            Element element = (Element) ruleList.item(i);
            if (ruleId.equals(element.getAttribute(SscXmlFormat.ID))) {
                createdRule = element;
                break;
            }
        }

        assertNotNull(createdRule);
        assertEquals(1, createdRule.getElementsByTagName(
                SscXmlFormat.getCpElement(SLOT_0, SscXmlFormat.CONDITIONS)).getLength());
        assertEquals(1, createdRule.getElementsByTagName(ruleConditionTag).getLength());
        assertEquals(1, createdRule.getElementsByTagName(
                SscXmlFormat.getSsElement(SLOT_0, SscXmlFormat.RULE_DEACTIVATED)).getLength());

        NodeList actionNodeList = createdRule.getElementsByTagName(
                SscXmlFormat.getCpElement(SLOT_0, SscXmlFormat.ACTIONS));
        assertEquals(1, actionNodeList.getLength());
        Element actionElement = (Element) actionNodeList.item(0);
        NodeList allowNodeList = actionElement.getElementsByTagName(
                SscXmlFormat.getSsElement(SLOT_0, SscXmlFormat.ALLOW));
        assertEquals(1, allowNodeList.getLength());
        assertEquals("false", allowNodeList.item(0).getTextContent());
    }

    private void updateTagsAndRules(Document doc) {
        mSscXmlParser.updateTagsAndRules(SLOT_0, doc);
    }

    private Document getEntireXmlDoc() {
        return SscXmlGovTest.createEntireXmlDoc();
    }

    private Document removeRule(Document doc, String ruleId) {
        return SscXmlGovTest.removeRule(doc, ruleId);
    }

    private SscServiceData getUpdateData(ESsType ssType, int action, int condition) {
        return SscXmlGovTest.createUpdateData(ssType, 0, action, condition,
                SscServiceClassUtil.SERVICE_CLASS_NONE);
    }

    private SscServiceData getInsertCfData(ESsType ssType, int action, int condition,
            String targetNumber) {
        return SscXmlGovTest.createInsertData(ssType, 0, action, condition, targetNumber);
    }

    private SscServiceData getInsertCbData(ESsType ssType, int action, int condition) {
        return SscXmlGovTest.createInsertData(ssType, 0, action, condition, null);
    }

    private class FakeSscXmlCreator extends SscXmlCreator {
        private FakeSscXmlCreator() {
        }

        @Override
        protected SscUtils getSscUtils() {
            return mMockSscUtils;
        }
    }
}
