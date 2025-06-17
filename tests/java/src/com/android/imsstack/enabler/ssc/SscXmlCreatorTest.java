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
import static org.mockito.Mockito.when;

import com.android.imsstack.core.agents.ConfigInterface;
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

    private SscXmlCreator mSscXmlCreator;
    private Document mCachedDoc;

    @Mock private CarrierConfig mMockCarrierConfig;
    @Mock private ConfigInterface mMockConfigInterface;
    @Mock private SscUtils mMockSscUtils;

    @Before
    public void setup() {
        MockitoAnnotations.initMocks(this);

        SscXmlFormat.init(SLOT_0);
        mSscXmlCreator = new FakeSscXmlCreator();

        SscConfig.setConfigInterface(SLOT_0, mMockConfigInterface);
        when(mMockConfigInterface.getCarrierConfig()).thenReturn(mMockCarrierConfig);

        mCachedDoc = getEntireXmlDoc();
        updateTagsAndRules(mCachedDoc);
    }

    @After
    public void tearDown() {
        SscXmlFormat.clear(SLOT_0);
        SscConfig.clear(SLOT_0);
    }

    @Test
    public void createXml_invalidDocParam() {
        SscServiceData updateData = getUpdateData(ESsType.CW, SscConstant.ACTION_ACTIVATION);

        Element xml = mSscXmlCreator.createXml(null, updateData);

        assertNull(xml);
    }

    @Test
    public void createXml_invalidDataParam() {
        Element xml = mSscXmlCreator.createXml(mCachedDoc, null);

        assertNull(xml);
    }

    @Test
    public void createXml_inavlidEsstypeOfData() {
        SscServiceData updateData = getUpdateData(ESsType.NONE, SscConstant.ACTION_ACTIVATION);

        Element xml = mSscXmlCreator.createXml(mCachedDoc, updateData);

        assertNull(xml);
    }

    @Test
    public void createXml_oip() {
        SscServiceData updateData = getUpdateData(ESsType.OIP, SscConstant.ACTION_ACTIVATION);

        Element xml = mSscXmlCreator.createXml(mCachedDoc, updateData);

        assertNotNull(xml);
        assertEquals(SscXmlFormat.getSsElement(SLOT_0, SscXmlFormat.OIP), xml.getTagName());
        assertEquals("true", xml.getAttribute(SscXmlFormat.ACTIVE));
    }

    @Test
    public void createXml_oirDefaultWhenOperationNotProvisioned() {
        SscServiceData updateData = getUpdateData(ESsType.OIR, SscConstant.OIR_DEFAULT);

        when(mMockCarrierConfig.getInt(
                CarrierConfig.ImsSs.KEY_UT_OIR_NETWORK_DEFAULT_OPERATION_INT))
                .thenReturn(SscConfig.OIR_NOT_PROVISIONED);

        Element xml = mSscXmlCreator.createXml(mCachedDoc, updateData);

        assertNotNull(xml);
        assertEquals(SscXmlFormat.getSsElement(SLOT_0, SscXmlFormat.OIR), xml.getTagName());
        assertEquals("false", xml.getAttribute(SscXmlFormat.ACTIVE));
    }

    @Test
    public void createXml_oirDefaultWhenOperationTempModeRestricted() {
        SscServiceData updateData = getUpdateData(ESsType.OIR, SscConstant.OIR_DEFAULT);

        when(mMockCarrierConfig.getInt(
                CarrierConfig.ImsSs.KEY_UT_OIR_NETWORK_DEFAULT_OPERATION_INT))
                .thenReturn(SscConfig.OIR_TEMP_MODE_RESTRICTED);

        Element xml = mSscXmlCreator.createXml(mCachedDoc, updateData);

        assertNotNull(xml);
        assertEquals(SscXmlFormat.getSsElement(SLOT_0, SscXmlFormat.OIR), xml.getTagName());
        assertEquals("true", xml.getAttribute(SscXmlFormat.ACTIVE));

        Element defaultBehaviour = (Element) xml.getFirstChild();
        assertNotNull(defaultBehaviour);
        assertEquals(SscXmlFormat.getSsElement(SLOT_0, SscXmlFormat.DEFAULT_BEHAVIOUR),
                defaultBehaviour.getNodeName());
        assertEquals(SscXmlFormat.PRESENTATION_RESTRICTED, defaultBehaviour.getTextContent());
    }

    @Test
    public void createXml_oirDefaultWhenOperationTempModeNotRestricted() {
        SscServiceData updateData = getUpdateData(ESsType.OIR, SscConstant.OIR_DEFAULT);

        when(mMockCarrierConfig.getInt(
                CarrierConfig.ImsSs.KEY_UT_OIR_NETWORK_DEFAULT_OPERATION_INT))
                .thenReturn(SscConfig.OIR_TEMP_MODE_ALLOWED);

        Element xml = mSscXmlCreator.createXml(mCachedDoc, updateData);

        assertNotNull(xml);
        assertEquals(SscXmlFormat.getSsElement(SLOT_0, SscXmlFormat.OIR), xml.getTagName());
        assertEquals("true", xml.getAttribute(SscXmlFormat.ACTIVE));

        Element defaultBehaviour = (Element) xml.getFirstChild();
        assertNotNull(defaultBehaviour);
        assertEquals(SscXmlFormat.getSsElement(SLOT_0, SscXmlFormat.DEFAULT_BEHAVIOUR),
                defaultBehaviour.getNodeName());
        assertEquals(SscXmlFormat.PRESENTATION_NOT_RESTRICTED, defaultBehaviour.getTextContent());
    }

    @Test
    public void createXml_oirDefaultWhenOperationTempModeWithoutDefaultBehaviour() {
        SscServiceData updateData = getUpdateData(ESsType.OIR, SscConstant.OIR_DEFAULT);

        when(mMockCarrierConfig.getInt(
                CarrierConfig.ImsSs.KEY_UT_OIR_NETWORK_DEFAULT_OPERATION_INT))
                .thenReturn(SscConfig.OIR_TEMP_MODE_WITHOUT_DEFAULT_BEHAVIOUR);

        Element xml = mSscXmlCreator.createXml(mCachedDoc, updateData);

        assertNotNull(xml);
        assertEquals(SscXmlFormat.getSsElement(SLOT_0, SscXmlFormat.OIR), xml.getTagName());
        assertEquals("true", xml.getAttribute(SscXmlFormat.ACTIVE));
        assertEquals(0, xml.getChildNodes().getLength());
    }

    @Test
    public void createXml_oirInvocation() {
        SscServiceData updateData = getUpdateData(ESsType.OIR, SscConstant.OIR_INVOCATION);

        Element xml = mSscXmlCreator.createXml(mCachedDoc, updateData);

        assertNotNull(xml);
        assertEquals(SscXmlFormat.getSsElement(SLOT_0, SscXmlFormat.OIR), xml.getTagName());
        assertEquals("true", xml.getAttribute(SscXmlFormat.ACTIVE));

        Element defaultBehaviour = (Element) xml.getFirstChild();
        assertNotNull(defaultBehaviour);
        assertEquals(SscXmlFormat.getSsElement(SLOT_0, SscXmlFormat.DEFAULT_BEHAVIOUR),
                defaultBehaviour.getNodeName());
        assertEquals(SscXmlFormat.PRESENTATION_RESTRICTED, defaultBehaviour.getTextContent());
    }

    @Test
    public void createXml_oirSuppression() {
        SscServiceData updateData = getUpdateData(ESsType.OIR, SscConstant.OIR_SUPPRESSION);

        when(mMockCarrierConfig.getBoolean(
                CarrierConfig.ImsSs.KEY_UT_OIR_TIR_ALWAYS_TEMPORARY_MODE_BOOL)).thenReturn(false);

        Element xml = mSscXmlCreator.createXml(mCachedDoc, updateData);

        assertNotNull(xml);
        assertEquals(SscXmlFormat.getSsElement(SLOT_0, SscXmlFormat.OIR), xml.getTagName());
        assertEquals("false", xml.getAttribute(SscXmlFormat.ACTIVE));

        Element defaultBehaviour = (Element) xml.getFirstChild();
        assertNotNull(defaultBehaviour);
        assertEquals(SscXmlFormat.getSsElement(SLOT_0, SscXmlFormat.DEFAULT_BEHAVIOUR),
                defaultBehaviour.getNodeName());
        assertEquals(SscXmlFormat.PRESENTATION_NOT_RESTRICTED, defaultBehaviour.getTextContent());
    }

    @Test
    public void createXml_oirSuppressionWhenAlwaysTemporaryMode() {
        SscServiceData updateData = getUpdateData(ESsType.OIR, SscConstant.OIR_SUPPRESSION);

        when(mMockCarrierConfig.getBoolean(
                CarrierConfig.ImsSs.KEY_UT_OIR_TIR_ALWAYS_TEMPORARY_MODE_BOOL)).thenReturn(true);

        Element xml = mSscXmlCreator.createXml(mCachedDoc, updateData);

        assertNotNull(xml);
        assertEquals(SscXmlFormat.getSsElement(SLOT_0, SscXmlFormat.OIR), xml.getTagName());
        assertEquals("true", xml.getAttribute(SscXmlFormat.ACTIVE));

        Element defaultBehaviour = (Element) xml.getFirstChild();
        assertNotNull(defaultBehaviour);
        assertEquals(SscXmlFormat.getSsElement(SLOT_0, SscXmlFormat.DEFAULT_BEHAVIOUR),
                defaultBehaviour.getNodeName());
        assertEquals(SscXmlFormat.PRESENTATION_NOT_RESTRICTED, defaultBehaviour.getTextContent());
    }

    @Test
    public void createXml_oirSuppressionWhenNoDefaultBehaviourInXml() {
        String xmlData = "<ss:simservs>"
                + "<ss:originating-identity-presentation-restriction active=\"false\">"
                + "</ss:originating-identity-presentation-restriction>"
                + "</ss:simservs>";

        SscServiceData updateData = getUpdateData(ESsType.OIR, SscConstant.OIR_SUPPRESSION);

        when(mMockCarrierConfig.getBoolean(
                CarrierConfig.ImsSs.KEY_UT_OIR_TIR_ALWAYS_TEMPORARY_MODE_BOOL)).thenReturn(true);

        Element xml = mSscXmlCreator.createXml(getDocumentFromString(xmlData), updateData);

        assertNotNull(xml);
        assertEquals(SscXmlFormat.getSsElement(SLOT_0, SscXmlFormat.OIR), xml.getTagName());
        assertEquals("true", xml.getAttribute(SscXmlFormat.ACTIVE));

        Element defaultBehaviour = (Element) xml.getFirstChild();
        assertNotNull(defaultBehaviour);
        assertEquals(SscXmlFormat.getSsElement(SLOT_0, SscXmlFormat.DEFAULT_BEHAVIOUR),
                defaultBehaviour.getNodeName());
        assertEquals(SscXmlFormat.PRESENTATION_NOT_RESTRICTED, defaultBehaviour.getTextContent());
    }

    @Test
    public void createXml_tip() {
        SscServiceData updateData = getUpdateData(ESsType.TIP, SscConstant.ACTION_DEACTIVATION);

        Element xml = mSscXmlCreator.createXml(mCachedDoc, updateData);

        assertNotNull(xml);
        assertEquals(SscXmlFormat.getSsElement(SLOT_0, SscXmlFormat.TIP), xml.getTagName());
        assertEquals("false", xml.getAttribute(SscXmlFormat.ACTIVE));
    }

    @Test
    public void createXml_tirNotProvisioned() {
        SscServiceData updateData = getUpdateData(ESsType.TIR, SscConstant.TIR_NOT_PROVISIONED);

        when(mMockCarrierConfig.getBoolean(
                CarrierConfig.ImsSs.KEY_UT_OIR_TIR_ALWAYS_TEMPORARY_MODE_BOOL)).thenReturn(false);

        Element xml = mSscXmlCreator.createXml(mCachedDoc, updateData);

        assertNotNull(xml);
        assertEquals(SscXmlFormat.getSsElement(SLOT_0, SscXmlFormat.TIR), xml.getTagName());
        assertEquals("false", xml.getAttribute(SscXmlFormat.ACTIVE));

        Element defaultBehaviour = (Element) xml.getFirstChild();
        assertNotNull(defaultBehaviour);
        assertEquals(SscXmlFormat.getSsElement(SLOT_0, SscXmlFormat.DEFAULT_BEHAVIOUR),
                defaultBehaviour.getNodeName());
        assertEquals(SscXmlFormat.PRESENTATION_NOT_RESTRICTED, defaultBehaviour.getTextContent());
    }

    @Test
    public void createXml_tirNotProvisionedWhenAlwaysTemporaryMode() {
        SscServiceData updateData = getUpdateData(ESsType.TIR, SscConstant.TIR_NOT_PROVISIONED);

        when(mMockCarrierConfig.getBoolean(
                CarrierConfig.ImsSs.KEY_UT_OIR_TIR_ALWAYS_TEMPORARY_MODE_BOOL)).thenReturn(true);

        Element xml = mSscXmlCreator.createXml(mCachedDoc, updateData);

        assertNotNull(xml);
        assertEquals(SscXmlFormat.getSsElement(SLOT_0, SscXmlFormat.TIR), xml.getTagName());
        assertEquals("true", xml.getAttribute(SscXmlFormat.ACTIVE));

        Element defaultBehaviour = (Element) xml.getFirstChild();
        assertNotNull(defaultBehaviour);
        assertEquals(SscXmlFormat.getSsElement(SLOT_0, SscXmlFormat.DEFAULT_BEHAVIOUR),
                defaultBehaviour.getNodeName());
        assertEquals(SscXmlFormat.PRESENTATION_NOT_RESTRICTED, defaultBehaviour.getTextContent());
    }

    @Test
    public void createXml_tirProvisioned() {
        SscServiceData updateData = getUpdateData(ESsType.TIR, SscConstant.TIR_PROVISIONED);

        when(mMockCarrierConfig.getBoolean(
                CarrierConfig.ImsSs.KEY_UT_OIR_TIR_ALWAYS_TEMPORARY_MODE_BOOL)).thenReturn(false);

        Element xml = mSscXmlCreator.createXml(mCachedDoc, updateData);

        assertNotNull(xml);
        assertEquals(SscXmlFormat.getSsElement(SLOT_0, SscXmlFormat.TIR), xml.getTagName());
        assertEquals("true", xml.getAttribute(SscXmlFormat.ACTIVE));

        Element defaultBehaviour = (Element) xml.getFirstChild();
        assertNotNull(defaultBehaviour);
        assertEquals(SscXmlFormat.getSsElement(SLOT_0, SscXmlFormat.DEFAULT_BEHAVIOUR),
                defaultBehaviour.getNodeName());
        assertEquals(SscXmlFormat.PRESENTATION_RESTRICTED, defaultBehaviour.getTextContent());
    }

    @Test
    public void createXml_tirProvisionedWhenNoDefaultBehaviourInXml() {
        String xmlData = "<ss:simservs>"
                + "<ss:terminating-identity-presentation-restriction active=\"false\">"
                + "</ss:terminating-identity-presentation-restriction>"
                + "</ss:simservs>";

        SscServiceData updateData = getUpdateData(ESsType.TIR, SscConstant.TIR_PROVISIONED);

        when(mMockCarrierConfig.getBoolean(
                CarrierConfig.ImsSs.KEY_UT_OIR_TIR_ALWAYS_TEMPORARY_MODE_BOOL)).thenReturn(false);

        Element xml = mSscXmlCreator.createXml(getDocumentFromString(xmlData), updateData);

        assertNotNull(xml);
        assertEquals(SscXmlFormat.getSsElement(SLOT_0, SscXmlFormat.TIR), xml.getTagName());
        assertEquals("true", xml.getAttribute(SscXmlFormat.ACTIVE));

        Element defaultBehaviour = (Element) xml.getFirstChild();
        assertNotNull(defaultBehaviour);
        assertEquals(SscXmlFormat.getSsElement(SLOT_0, SscXmlFormat.DEFAULT_BEHAVIOUR),
                defaultBehaviour.getNodeName());
        assertEquals(SscXmlFormat.PRESENTATION_RESTRICTED, defaultBehaviour.getTextContent());
    }

    @Test
    public void createXml_updateCfnrTimer() {
        SscServiceData updateData = getCfUpdateData(ESsType.CF, SscConstant.ACTION_ACTIVATION,
                SscConstant.CONDITION_CFNR_TIMER, null, SscServiceClassUtil.SERVICE_CLASS_NONE, 25);

        Element xml = mSscXmlCreator.createXml(mCachedDoc, updateData);

        assertNotNull(xml);
        assertEquals(SscXmlFormat.getSsElement(SLOT_0, SscXmlFormat.NOREPLYTIMER),
                xml.getTagName());
        assertEquals("25", xml.getTextContent());
    }

    @Test
    public void createXml_updateCfnrTimerWhenNoReplyTimerInXmlButOmittedFlagTrue() {
        String xmlData = "<ss:simservs>"
                + "<ss:communication-diversion active=\"true\">"
                + "<ss:NoReplyTimer>10</ss:NoReplyTimer>"
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
                + "</ss:communication-diversion>"
                + "</ss:simservs>";

        SscServiceData updateData = getCfUpdateData(ESsType.CF, SscConstant.ACTION_ACTIVATION,
                SscConstant.CONDITION_CFNR_TIMER, null, SscServiceClassUtil.SERVICE_CLASS_NONE, 25);

        SscXmlFormat.setIsNoReplyTimerOmitted(SLOT_0, true);

        Element xml = mSscXmlCreator.createXml(getDocumentFromString(xmlData), updateData);

        assertNotNull(xml);
        assertEquals(SscXmlFormat.getSsElement(SLOT_0, SscXmlFormat.NOREPLYTIMER),
                xml.getTagName());
        assertEquals("25", xml.getTextContent());

        assertFalse(SscXmlFormat.getIsNoReplyTimerOmitted(SLOT_0));
    }

    @Test
    public void createXml_updateNoReplyTimerWhenNoNoReplyTimerInXml_insertInCdivNode() {
        String xmlData = "<ss:simservs>"
                + "<ss:communication-diversion active=\"true\">"
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
                + "</ss:communication-diversion>"
                + "</ss:simservs>";

        SscServiceData updateData = getCfUpdateData(ESsType.CF, SscConstant.ACTION_ACTIVATION,
                SscConstant.CONDITION_CFNR_TIMER, null, SscServiceClassUtil.SERVICE_CLASS_NONE, 25);

        SscXmlFormat.setIsNoReplyTimerOmitted(SLOT_0, true);
        SscXmlFormat.setIsNoReplyTimerInRule(SLOT_0, false);

        Element xml = mSscXmlCreator.createXml(getDocumentFromString(xmlData), updateData);

        assertNotNull(xml);
        assertEquals(SscXmlFormat.getSsElement(SLOT_0, SscXmlFormat.CD), xml.getTagName());

        NodeList nodeList = xml.getChildNodes();
        assertEquals(2, nodeList.getLength()); // Ruleset and NoReplyTimer.

        Element noReplyTimer = (Element) nodeList.item(0);
        assertEquals(SscXmlFormat.getSsElement(SLOT_0, SscXmlFormat.NOREPLYTIMER),
                noReplyTimer.getTagName());
        assertEquals("25", noReplyTimer.getTextContent());

        Element ruleSet = (Element) nodeList.item(1);
        assertEquals(SscXmlFormat.getCpElement(SLOT_0, SscXmlFormat.RULESET), ruleSet.getTagName());
    }

    @Test
    public void createXml_updateCfnrAndNoReplyTimerWhenNoNoReplyTimerInXml_insertInActionsNode() {
        String xmlData = "<ss:simservs>"
                + "<ss:communication-diversion active=\"true\">"
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
                + "</ss:communication-diversion>"
                + "</ss:simservs>";

        SscServiceData updateData = getCfUpdateData(ESsType.CF, SscConstant.ACTION_REGISTRATION,
                SscConstant.CONDITION_CFNR, null, SscServiceClassUtil.SERVICE_CLASS_NONE, 25);

        SscXmlFormat.setIsNoReplyTimerOmitted(SLOT_0, true);
        SscXmlFormat.setIsNoReplyTimerInRule(SLOT_0, true);

        Element xml = mSscXmlCreator.createXml(getDocumentFromString(xmlData), updateData);

        assertNotNull(xml);
        assertEquals(SscXmlFormat.getSsElement(SLOT_0, SscXmlFormat.RULE), xml.getTagName());

        NodeList nodeList = xml.getChildNodes();
        assertEquals(2, nodeList.getLength()); // Conditions and Actions.

        Element actions = (Element) nodeList.item(1);
        assertEquals(SscXmlFormat.getCpElement(SLOT_0, SscXmlFormat.ACTIONS), actions.getTagName());

        nodeList = actions.getChildNodes();
        assertEquals(2, nodeList.getLength()); // Forward-to and NoReplyTimer.

        Element noReplyTimer = (Element) nodeList.item(1);
        assertEquals(SscXmlFormat.getSsElement(SLOT_0, SscXmlFormat.NOREPLYTIMER),
                noReplyTimer.getTagName());
        assertEquals("25", noReplyTimer.getTextContent());

        assertTrue(SscXmlFormat.getIsNoReplyTimerOmitted(SLOT_0)); // No change here.
    }

    @Test
    public void createXml_updateCfnrAndNoReplyTimerWhenNoReplyTimerInXml_updateTimer() {
        String xmlData = "<ss:simservs>"
                + "<ss:communication-diversion active=\"true\">"
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
                + "<ss:NoReplyTimer>10</ss:NoReplyTimer>"
                + "</cp:actions>"
                + "</cp:rule>"
                + "</cp:ruleset>"
                + "</ss:communication-diversion>"
                + "</ss:simservs>";

        SscServiceData updateData = getCfUpdateData(ESsType.CF, SscConstant.ACTION_REGISTRATION,
                SscConstant.CONDITION_CFNR, null, SscServiceClassUtil.SERVICE_CLASS_NONE, 25);

        SscXmlFormat.setIsNoReplyTimerOmitted(SLOT_0, true);
        SscXmlFormat.setIsNoReplyTimerInRule(SLOT_0, true);

        Element xml = mSscXmlCreator.createXml(getDocumentFromString(xmlData), updateData);

        assertNotNull(xml);
        assertEquals(SscXmlFormat.getSsElement(SLOT_0, SscXmlFormat.RULE), xml.getTagName());

        NodeList nodeList = xml.getChildNodes();
        assertEquals(2, nodeList.getLength()); // Conditions and Actions.

        Element actions = (Element) nodeList.item(1);
        assertEquals(SscXmlFormat.getCpElement(SLOT_0, SscXmlFormat.ACTIONS), actions.getTagName());

        nodeList = actions.getChildNodes();
        assertEquals(2, nodeList.getLength()); // Forward-to and NoReplyTimer.

        Element noReplyTimer = (Element) nodeList.item(1);
        assertEquals(SscXmlFormat.getSsElement(SLOT_0, SscXmlFormat.NOREPLYTIMER),
                noReplyTimer.getTagName());
        assertEquals("25", noReplyTimer.getTextContent());

        assertFalse(SscXmlFormat.getIsNoReplyTimerOmitted(SLOT_0)); // Shall be changed to false.
    }

    @Test
    public void createXml_updateCfWhenNoMatchingRuleInXml() {
        SscServiceData updateData = getCfUpdateData(ESsType.CF, SscConstant.ACTION_ACTIVATION,
                SscConstant.CONDITION_CFNR, null, SscServiceClassUtil.SERVICE_CLASS_NONE, 0);

        SscXmlFormat.setRuleId(SLOT_0, SscXmlFormat.MEDIA_TYPE_AUDIO, SscXmlFormat.CD,
                SscConstant.CONDITION_CFNR, null);

        Element xml = mSscXmlCreator.createXml(mCachedDoc, updateData);

        assertNull(xml);
    }

    @Test
    public void createXml_updateCfActivation() {
        SscServiceData updateData = getCfUpdateData(ESsType.CF, SscConstant.ACTION_ACTIVATION,
                SscConstant.CONDITION_CFNR, null, SscServiceClassUtil.SERVICE_CLASS_NONE, 0);

        Element xml = mSscXmlCreator.createXml(mCachedDoc, updateData);

        assertNotNull(xml);
        assertEquals(SscXmlFormat.getSsElement(SLOT_0, SscXmlFormat.CONDITIONS), xml.getTagName());

        NodeList nodeList = xml.getElementsByTagName(
                SscXmlFormat.getSsElement(SLOT_0, SscXmlFormat.RULE_DEACTIVATED));
        assertEquals(0, nodeList.getLength());

        NodeList ruleCondition = xml.getElementsByTagName(
                SscXmlFormat.getSsElement(SLOT_0, SscXmlFormat.CFNR));
        assertEquals(1, ruleCondition.getLength());
    }

    @Test
    public void createXml_updateCfActivationForVideo() {
        SscServiceData updateData = getCfUpdateData(ESsType.CF, SscConstant.ACTION_ACTIVATION,
                SscConstant.CONDITION_CFNR, null, SscServiceClassUtil.SERVICE_CLASS_VIDEO, 0);

        Element xml = mSscXmlCreator.createXml(mCachedDoc, updateData);

        assertNotNull(xml);
        assertEquals(SscXmlFormat.getSsElement(SLOT_0, SscXmlFormat.CONDITIONS), xml.getTagName());

        NodeList nodeList = xml.getElementsByTagName(
                SscXmlFormat.getSsElement(SLOT_0, SscXmlFormat.RULE_DEACTIVATED));
        assertEquals(0, nodeList.getLength());

        NodeList mediaList = xml.getElementsByTagName(
                SscXmlFormat.getSsElement(SLOT_0, SscXmlFormat.MEDIA));
        assertEquals(1, mediaList.getLength());
        assertEquals(SscXmlFormat.VIDEO, mediaList.item(0).getTextContent());

        NodeList ruleCondition = xml.getElementsByTagName(
                SscXmlFormat.getSsElement(SLOT_0, SscXmlFormat.CFNR));
        assertEquals(1, ruleCondition.getLength());
    }

    @Test
    public void createXml_updateCfDeactivation() {
        SscServiceData updateData = getCfUpdateData(ESsType.CF, SscConstant.ACTION_DEACTIVATION,
                SscConstant.CONDITION_CFNR, null, SscServiceClassUtil.SERVICE_CLASS_NONE, 0);

        Element xml = mSscXmlCreator.createXml(mCachedDoc, updateData);

        assertNotNull(xml);
        assertEquals(SscXmlFormat.getSsElement(SLOT_0, SscXmlFormat.CONDITIONS), xml.getTagName());

        NodeList nodeList = xml.getElementsByTagName(
                SscXmlFormat.getSsElement(SLOT_0, SscXmlFormat.RULE_DEACTIVATED));
        assertEquals(1, nodeList.getLength());

        NodeList ruleCondition = xml.getElementsByTagName(
                SscXmlFormat.getSsElement(SLOT_0, SscXmlFormat.CFNR));
        assertEquals(1, ruleCondition.getLength());
    }

    @Test
    public void createXml_updateCfRegistrationForAudio() {
        String target = "+1234567890";
        SscServiceData updateData = getCfUpdateData(ESsType.CF, SscConstant.ACTION_REGISTRATION,
                SscConstant.CONDITION_CFNR, target, SscServiceClassUtil.SERVICE_CLASS_VOICE, 0);

        when(mMockSscUtils.getUriFromNumber(SLOT_0, target)).thenReturn(target);

        Element xml = mSscXmlCreator.createXml(mCachedDoc, updateData);

        assertNotNull(xml);
        assertEquals(SscXmlFormat.getSsElement(SLOT_0, SscXmlFormat.RULE), xml.getTagName());
        assertEquals("call-diversion-no-reply", xml.getAttribute(SscXmlFormat.ID));

        NodeList nodeList = xml.getElementsByTagName(
                SscXmlFormat.getSsElement(SLOT_0, SscXmlFormat.RULE_DEACTIVATED));
        assertEquals(0, nodeList.getLength());

        NodeList forwardTotList = xml.getElementsByTagName(
                SscXmlFormat.getSsElement(SLOT_0, SscXmlFormat.FORWARD_TO));
        assertEquals(1, forwardTotList.getLength());

        Element targetElement = (Element) forwardTotList.item(0).getFirstChild();
        assertEquals(SscXmlFormat.getSsElement(SLOT_0, SscXmlFormat.TARGET),
                targetElement.getTagName());
        assertEquals(target, targetElement.getTextContent());

        NodeList ruleCondition = xml.getElementsByTagName(
                SscXmlFormat.getSsElement(SLOT_0, SscXmlFormat.CFNR));
        assertEquals(1, ruleCondition.getLength());
    }

    @Test
    public void createXml_updateCfRegistrationForVideo() {
        String target = "+1234567890";
        SscServiceData updateData = getCfUpdateData(ESsType.CF, SscConstant.ACTION_REGISTRATION,
                SscConstant.CONDITION_CFNR, target, SscServiceClassUtil.SERVICE_CLASS_VIDEO, 0);

        when(mMockSscUtils.getUriFromNumber(SLOT_0, target)).thenReturn(target);

        Element xml = mSscXmlCreator.createXml(mCachedDoc, updateData);

        assertNotNull(xml);
        assertEquals(SscXmlFormat.getSsElement(SLOT_0, SscXmlFormat.RULE), xml.getTagName());
        assertEquals("call-diversion-no-reply-video", xml.getAttribute(SscXmlFormat.ID));

        NodeList nodeList = xml.getElementsByTagName(
                SscXmlFormat.getSsElement(SLOT_0, SscXmlFormat.RULE_DEACTIVATED));
        assertEquals(0, nodeList.getLength());

        NodeList forwardTotList = xml.getElementsByTagName(
                SscXmlFormat.getSsElement(SLOT_0, SscXmlFormat.FORWARD_TO));
        assertEquals(1, forwardTotList.getLength());

        Element targetElement = (Element) forwardTotList.item(0).getFirstChild();
        assertEquals(SscXmlFormat.getSsElement(SLOT_0, SscXmlFormat.TARGET),
                targetElement.getTagName());
        assertEquals(target, targetElement.getTextContent());

        NodeList ruleCondition = xml.getElementsByTagName(
                SscXmlFormat.getSsElement(SLOT_0, SscXmlFormat.CFNR));
        assertEquals(1, ruleCondition.getLength());

        NodeList mediaList = xml.getElementsByTagName(
                SscXmlFormat.getSsElement(SLOT_0, SscXmlFormat.MEDIA));
        assertEquals(1, mediaList.getLength());
        assertEquals(SscXmlFormat.VIDEO, mediaList.item(0).getTextContent());
    }

    @Test
    public void createXml_updateCfRegistrationWhenNoTargetElementInXml() {
        String target = "+1234567890";
        String xmlData = "<ss:simservs>"
                + "<ss:communication-diversion active=\"true\">"
                + "<cp:ruleset>"
                + "<cp:rule id=\"call-diversion-not-reachable\">"
                + "<cp:conditions>"
                + "<ss:rule-deactivated/>"
                + "<ss:not-reachable/>"
                + "</cp:conditions>"
                + "<cp:actions>"
                + "<ss:forward-to>"
                + "</ss:forward-to>"
                + "</cp:actions>"
                + "</cp:rule>"
                + "</cp:ruleset>"
                + "</ss:communication-diversion>"
                + "</ss:simservs>";

        when(mMockSscUtils.getUriFromNumber(SLOT_0, target)).thenReturn(target);

        SscServiceData updateData = getCfUpdateData(ESsType.CF, SscConstant.ACTION_REGISTRATION,
                SscConstant.CONDITION_CFNRC, target, SscServiceClassUtil.SERVICE_CLASS_NONE, 0);

        Element xml = mSscXmlCreator.createXml(getDocumentFromString(xmlData), updateData);

        assertNotNull(xml);
        assertEquals(SscXmlFormat.getSsElement(SLOT_0, SscXmlFormat.RULE), xml.getTagName());
        assertEquals("call-diversion-not-reachable", xml.getAttribute(SscXmlFormat.ID));

        NodeList nodeList = xml.getElementsByTagName(
                SscXmlFormat.getSsElement(SLOT_0, SscXmlFormat.RULE_DEACTIVATED));
        assertEquals(0, nodeList.getLength());

        NodeList forwardTotList = xml.getElementsByTagName(
                SscXmlFormat.getSsElement(SLOT_0, SscXmlFormat.FORWARD_TO));
        assertEquals(1, forwardTotList.getLength());

        Element targetElement = (Element) forwardTotList.item(0).getFirstChild();
        assertEquals(SscXmlFormat.getSsElement(SLOT_0, SscXmlFormat.TARGET),
                targetElement.getTagName());
        assertEquals(target, targetElement.getTextContent());

        NodeList ruleCondition = xml.getElementsByTagName(
                SscXmlFormat.getSsElement(SLOT_0, SscXmlFormat.CFNRC));
        assertEquals(1, ruleCondition.getLength());
    }

    @Test
    public void createXml_updateCfRegistrationWhenNoForwardToElementInXml() {
        String target = "+1234567890";
        String xmlData = "<ss:simservs>"
                + "<ss:communication-diversion active=\"true\">"
                + "<cp:ruleset>"
                + "<cp:rule id=\"call-diversion-not-reachable\">"
                + "<cp:conditions>"
                + "<ss:rule-deactivated/>"
                + "<ss:not-reachable/>"
                + "</cp:conditions>"
                + "<cp:actions>"
                + "</cp:actions>"
                + "</cp:rule>"
                + "</cp:ruleset>"
                + "</ss:communication-diversion>"
                + "</ss:simservs>";

        when(mMockSscUtils.getUriFromNumber(SLOT_0, target)).thenReturn(target);

        SscServiceData updateData = getCfUpdateData(ESsType.CF, SscConstant.ACTION_REGISTRATION,
                SscConstant.CONDITION_CFNRC, target, SscServiceClassUtil.SERVICE_CLASS_NONE, 0);

        Element xml = mSscXmlCreator.createXml(getDocumentFromString(xmlData), updateData);

        assertNotNull(xml);
        assertEquals(SscXmlFormat.getSsElement(SLOT_0, SscXmlFormat.RULE), xml.getTagName());
        assertEquals("call-diversion-not-reachable", xml.getAttribute(SscXmlFormat.ID));

        NodeList nodeList = xml.getElementsByTagName(
                SscXmlFormat.getSsElement(SLOT_0, SscXmlFormat.RULE_DEACTIVATED));
        assertEquals(0, nodeList.getLength());

        NodeList forwardTotList = xml.getElementsByTagName(
                SscXmlFormat.getSsElement(SLOT_0, SscXmlFormat.FORWARD_TO));
        assertEquals(1, forwardTotList.getLength());

        Element targetElement = (Element) forwardTotList.item(0).getFirstChild();
        assertEquals(SscXmlFormat.getSsElement(SLOT_0, SscXmlFormat.TARGET),
                targetElement.getTagName());
        assertEquals(target, targetElement.getTextContent());
    }

    @Test
    public void createXml_updateCfRegistrationWhenNoReplyTimerInRule() {
        String target = "+1234567890";
        String xmlData = "<ss:simservs>"
                + "<ss:communication-diversion active=\"true\">"
                + "<cp:ruleset>"
                + "<cp:rule id=\"call-diversion-no-reply\">"
                + "<cp:conditions>"
                + "<ss:rule-deactivated/>"
                + "<ss:NoReplyTimer>20</ss:NoReplyTimer>"
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
                + "</cp:ruleset>"
                + "</ss:communication-diversion>"
                + "</ss:simservs>";

        SscXmlFormat.setIsNoReplyTimerInRule(SLOT_0, true);
        when(mMockSscUtils.getUriFromNumber(SLOT_0, target)).thenReturn(target);

        SscServiceData updateData = getCfUpdateData(ESsType.CF, SscConstant.ACTION_REGISTRATION,
                SscConstant.CONDITION_CFNR, target, SscServiceClassUtil.SERVICE_CLASS_NONE, 30);

        Element xml = mSscXmlCreator.createXml(getDocumentFromString(xmlData), updateData);

        assertNotNull(xml);
        assertEquals(SscXmlFormat.getSsElement(SLOT_0, SscXmlFormat.RULE), xml.getTagName());
        assertEquals("call-diversion-no-reply", xml.getAttribute(SscXmlFormat.ID));

        NodeList nodeList = xml.getElementsByTagName(
                SscXmlFormat.getSsElement(SLOT_0, SscXmlFormat.RULE_DEACTIVATED));
        assertEquals(0, nodeList.getLength());

        NodeList forwardTotList = xml.getElementsByTagName(
                SscXmlFormat.getSsElement(SLOT_0, SscXmlFormat.FORWARD_TO));
        assertEquals(1, forwardTotList.getLength());

        Element targetElement = (Element) forwardTotList.item(0).getFirstChild();
        assertEquals(SscXmlFormat.getSsElement(SLOT_0, SscXmlFormat.TARGET),
                targetElement.getTagName());
        assertEquals(target, targetElement.getTextContent());

        NodeList ruleCondition = xml.getElementsByTagName(
                SscXmlFormat.getSsElement(SLOT_0, SscXmlFormat.CFNR));
        assertEquals(1, ruleCondition.getLength());

        NodeList noReplyTimerList = xml.getElementsByTagName(
                SscXmlFormat.getSsElement(SLOT_0, SscXmlFormat.NOREPLYTIMER));
        assertEquals(1, noReplyTimerList.getLength());

        Element noReplyTimer = (Element) noReplyTimerList.item(0);
        assertEquals("30", noReplyTimer.getTextContent());
    }

    @Test
    public void createXml_updateCfErasure() {
        SscServiceData updateData = getCfUpdateData(ESsType.CF, SscConstant.ACTION_ERASURE,
                SscConstant.CONDITION_CFB, null, SscServiceClassUtil.SERVICE_CLASS_NONE, 0);

        when(mMockSscUtils.getUriFromNumber(SLOT_0, null)).thenReturn("");

        Element xml = mSscXmlCreator.createXml(mCachedDoc, updateData);

        assertNotNull(xml);
        assertEquals(SscXmlFormat.getSsElement(SLOT_0, SscXmlFormat.RULE), xml.getTagName());
        assertEquals("call-diversion-busy", xml.getAttribute(SscXmlFormat.ID));

        NodeList nodeList = xml.getElementsByTagName(
                SscXmlFormat.getSsElement(SLOT_0, SscXmlFormat.RULE_DEACTIVATED));
        assertEquals(1, nodeList.getLength());

        NodeList forwardTotList = xml.getElementsByTagName(
                SscXmlFormat.getSsElement(SLOT_0, SscXmlFormat.FORWARD_TO));
        assertEquals(1, forwardTotList.getLength());

        Element targetElement = (Element) forwardTotList.item(0).getFirstChild();
        assertEquals(SscXmlFormat.getSsElement(SLOT_0, SscXmlFormat.TARGET),
                targetElement.getTagName());
        assertEquals("", targetElement.getTextContent());

        NodeList ruleCondition = xml.getElementsByTagName(
                SscXmlFormat.getSsElement(SLOT_0, SscXmlFormat.CFB));
        assertEquals(1, ruleCondition.getLength());
    }

    @Test
    public void createXml_updateIcbWhenNoMatchingRuleInXml() {
        SscServiceData updateData = getCbUpdateData(ESsType.ICB, SscConstant.ACTION_ACTIVATION,
                SscConstant.CONDITION_BAIC, SscServiceClassUtil.SERVICE_CLASS_NONE);

        SscXmlFormat.setRuleId(SLOT_0, SscXmlFormat.MEDIA_TYPE_AUDIO, SscXmlFormat.ICB,
                SscConstant.CONDITION_BAIC, null);

        Element xml = mSscXmlCreator.createXml(mCachedDoc, updateData);

        assertNull(xml);
    }

    @Test
    public void createXml_updateIcbEnable() {
        SscServiceData updateData = getCbUpdateData(ESsType.ICB, SscConstant.ACTION_ACTIVATION,
                SscConstant.CONDITION_BAIC, SscServiceClassUtil.SERVICE_CLASS_NONE);

        Element xml = mSscXmlCreator.createXml(mCachedDoc, updateData);

        assertNotNull(xml);
        assertEquals(SscXmlFormat.getSsElement(SLOT_0, SscXmlFormat.RULE), xml.getTagName());
        assertEquals("call-barring-all-incoming", xml.getAttribute(SscXmlFormat.ID));

        NodeList nodeList = xml.getElementsByTagName(
                SscXmlFormat.getSsElement(SLOT_0, SscXmlFormat.RULE_DEACTIVATED));
        assertEquals(0, nodeList.getLength());
    }

    @Test
    public void createXml_updateIcbDisable() {
        SscServiceData updateData = getCbUpdateData(ESsType.ICB, SscConstant.ACTION_DEACTIVATION,
                SscConstant.CONDITION_BIC_WR, SscServiceClassUtil.SERVICE_CLASS_NONE);

        Element xml = mSscXmlCreator.createXml(mCachedDoc, updateData);

        assertNotNull(xml);
        assertEquals(SscXmlFormat.getSsElement(SLOT_0, SscXmlFormat.RULE), xml.getTagName());
        assertEquals("call-barring-incoming-in-roaming", xml.getAttribute(SscXmlFormat.ID));

        NodeList nodeList = xml.getElementsByTagName(
                SscXmlFormat.getSsElement(SLOT_0, SscXmlFormat.RULE_DEACTIVATED));
        assertEquals(1, nodeList.getLength());

        NodeList ruleCondition = xml.getElementsByTagName(
                SscXmlFormat.getSsElement(SLOT_0, SscXmlFormat.BIC_WR));
        assertEquals(1, ruleCondition.getLength());
    }

    @Test
    public void createXml_updateOcbEnable() {
        SscServiceData updateData = getCbUpdateData(ESsType.OCB, SscConstant.ACTION_ACTIVATION,
                SscConstant.CONDITION_BAOC, SscServiceClassUtil.SERVICE_CLASS_NONE);

        Element xml = mSscXmlCreator.createXml(mCachedDoc, updateData);

        assertNotNull(xml);
        assertEquals(SscXmlFormat.getSsElement(SLOT_0, SscXmlFormat.RULE), xml.getTagName());
        assertEquals("call-barring-all-outgoing", xml.getAttribute(SscXmlFormat.ID));

        NodeList nodeList = xml.getElementsByTagName(
                SscXmlFormat.getSsElement(SLOT_0, SscXmlFormat.RULE_DEACTIVATED));
        assertEquals(0, nodeList.getLength());
    }

    @Test
    public void createXml_updateOcbDisable() {
        SscServiceData updateData = getCbUpdateData(ESsType.OCB, SscConstant.ACTION_DEACTIVATION,
                SscConstant.CONDITION_BOIC, SscServiceClassUtil.SERVICE_CLASS_NONE);

        Element xml = mSscXmlCreator.createXml(mCachedDoc, updateData);

        assertNotNull(xml);
        assertEquals(SscXmlFormat.getSsElement(SLOT_0, SscXmlFormat.RULE), xml.getTagName());
        assertEquals("call-barring-outgoing-international", xml.getAttribute(SscXmlFormat.ID));

        NodeList nodeList = xml.getElementsByTagName(
                SscXmlFormat.getSsElement(SLOT_0, SscXmlFormat.RULE_DEACTIVATED));
        assertEquals(1, nodeList.getLength());

        NodeList ruleCondition = xml.getElementsByTagName(
                SscXmlFormat.getSsElement(SLOT_0, SscXmlFormat.BOIC));
        assertEquals(1, ruleCondition.getLength());
    }

    @Test
    public void createXml_updateCbForAudio() {
        SscServiceData updateData = getCbUpdateData(ESsType.ICB, SscConstant.ACTION_ACTIVATION,
                SscConstant.CONDITION_ACR, SscServiceClassUtil.SERVICE_CLASS_VOICE);

        Element xml = mSscXmlCreator.createXml(mCachedDoc, updateData);

        assertNotNull(xml);
        assertEquals(SscXmlFormat.getSsElement(SLOT_0, SscXmlFormat.RULE), xml.getTagName());
        assertEquals("call-barring-anonymous-incoming", xml.getAttribute(SscXmlFormat.ID));

        NodeList nodeList = xml.getElementsByTagName(
                SscXmlFormat.getSsElement(SLOT_0, SscXmlFormat.RULE_DEACTIVATED));
        assertEquals(0, nodeList.getLength());

        NodeList ruleCondition = xml.getElementsByTagName(
                SscXmlFormat.getSsElement(SLOT_0, SscXmlFormat.ACR));
        assertEquals(1, ruleCondition.getLength());
    }

    @Test
    public void createXml_updateCbForVideo() {
        SscServiceData updateData = getCbUpdateData(ESsType.ICB, SscConstant.ACTION_ACTIVATION,
                SscConstant.CONDITION_ACR, SscServiceClassUtil.SERVICE_CLASS_VIDEO);

        Element xml = mSscXmlCreator.createXml(mCachedDoc, updateData);

        assertNotNull(xml);
        assertEquals(SscXmlFormat.getSsElement(SLOT_0, SscXmlFormat.RULE), xml.getTagName());
        assertEquals("call-barring-anonymous-incoming-video", xml.getAttribute(SscXmlFormat.ID));

        NodeList nodeList = xml.getElementsByTagName(
                SscXmlFormat.getSsElement(SLOT_0, SscXmlFormat.RULE_DEACTIVATED));
        assertEquals(0, nodeList.getLength());

        NodeList ruleCondition = xml.getElementsByTagName(
                SscXmlFormat.getSsElement(SLOT_0, SscXmlFormat.ACR));
        assertEquals(1, ruleCondition.getLength());

        NodeList mediaList = xml.getElementsByTagName(
                SscXmlFormat.getSsElement(SLOT_0, SscXmlFormat.MEDIA));
        assertEquals(1, mediaList.getLength());
        assertEquals(SscXmlFormat.VIDEO, mediaList.item(0).getTextContent());
    }

    @Test
    public void createXml_updateCwEnable() {
        SscServiceData updateData = getUpdateData(ESsType.CW, SscConstant.ACTION_ACTIVATION);

        Element xml = mSscXmlCreator.createXml(mCachedDoc, updateData);

        assertNotNull(xml);
        assertEquals(SscXmlFormat.getSsElement(SLOT_0, SscXmlFormat.CW), xml.getTagName());
        assertEquals("true", xml.getAttribute(SscXmlFormat.ACTIVE));
    }

    @Test
    public void createXml_updateCwDisable() {
        SscServiceData updateData = getUpdateData(ESsType.CW, SscConstant.ACTION_DEACTIVATION);

        Element xml = mSscXmlCreator.createXml(mCachedDoc, updateData);

        assertNotNull(xml);
        assertEquals(SscXmlFormat.getSsElement(SLOT_0, SscXmlFormat.CW), xml.getTagName());
        assertEquals("false", xml.getAttribute(SscXmlFormat.ACTIVE));
    }

    @Test
    public void createXml_insertCfWhenNotProvisioned() {
        int condition = SscConstant.CONDITION_CFB;
        String ruleId = SscXmlFormat.getDefaultRuleId(SLOT_0, SscXmlFormat.MEDIA_TYPE_AUDIO,
                SscXmlFormat.CD, condition);
        Document doc = removeRule(getEntireXmlDoc(), ruleId);
        updateTagsAndRules(doc);
        SscXmlFormat.setProvisionStatus(SLOT_0, SscXmlFormat.SC_CD, condition, false);

        SscServiceData insertData = getInsertCfData(ESsType.CF, SscConstant.ACTION_ACTIVATION,
                condition, null, SscServiceClassUtil.SERVICE_CLASS_NONE);

        Element xml = mSscXmlCreator.createXml(doc, insertData);

        assertNull(xml);
    }

    @Test
    public void createXml_insertCfWhenNoRulesetInXml() {
        int condition = SscConstant.CONDITION_CFB;
        String ruleId = SscXmlFormat.getDefaultRuleId(SLOT_0, SscXmlFormat.MEDIA_TYPE_AUDIO,
                SscXmlFormat.CD, condition);
        Document doc = removeRuleSet(getEntireXmlDoc());
        updateTagsAndRules(doc);
        String ruleConditionTag = SscXmlFormat.getRuleConditionTag(SLOT_0, SscXmlFormat.CD,
                condition);

        when(mMockCarrierConfig.getBoolean(CarrierConfig.ImsSs.KEY_UT_INSERT_NEW_RULE_BOOL))
                .thenReturn(true);

        SscServiceData insertData = getInsertCfData(ESsType.CF, SscConstant.ACTION_ACTIVATION,
                condition, null, SscServiceClassUtil.SERVICE_CLASS_NONE);

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
        assertEquals(0, mediaNodeList.getLength());
    }

    @Test
    public void createXml_insertCfbActivationWithoutAudioTag() {
        int condition = SscConstant.CONDITION_CFB;
        String ruleId = SscXmlFormat.getDefaultRuleId(SLOT_0, SscXmlFormat.MEDIA_TYPE_AUDIO,
                SscXmlFormat.CD, condition);
        Document doc = removeRule(getEntireXmlDoc(), ruleId);
        updateTagsAndRules(doc);
        String ruleConditionTag = SscXmlFormat.getRuleConditionTag(SLOT_0, SscXmlFormat.CD,
                condition);
        SscXmlFormat.setProvisionStatus(SLOT_0, SscXmlFormat.SC_CD, condition, true);
        SscXmlFormat.setMediaCapability(SLOT_0, SscXmlFormat.SC_CD, SscXmlFormat.MEDIA_TYPE_AUDIO,
                false);

        when(mMockCarrierConfig.getBoolean(CarrierConfig.ImsSs.KEY_UT_INSERT_NEW_RULE_BOOL))
                .thenReturn(true);

        SscServiceData insertData = getInsertCfData(ESsType.CF, SscConstant.ACTION_ACTIVATION,
                condition, null, SscServiceClassUtil.SERVICE_CLASS_VOICE);
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
        String ruleId = SscXmlFormat.getDefaultRuleId(SLOT_0, SscXmlFormat.MEDIA_TYPE_AUDIO,
                SscXmlFormat.CD, condition);
        Document doc = removeRule(getEntireXmlDoc(), ruleId);
        updateTagsAndRules(doc);
        String ruleConditionTag = SscXmlFormat.getRuleConditionTag(SLOT_0, SscXmlFormat.CD,
                condition);
        SscXmlFormat.setProvisionStatus(SLOT_0, SscXmlFormat.SC_CD, condition, true);
        SscXmlFormat.setMediaCapability(SLOT_0, SscXmlFormat.SC_CD, SscXmlFormat.MEDIA_TYPE_AUDIO,
                true);

        when(mMockCarrierConfig.getBoolean(CarrierConfig.ImsSs.KEY_UT_INSERT_NEW_RULE_BOOL))
                .thenReturn(true);

        SscServiceData insertData = getInsertCfData(ESsType.CF, SscConstant.ACTION_ACTIVATION,
                condition, null, SscServiceClassUtil.SERVICE_CLASS_VOICE);
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
    public void createXml_insertCfbActivationWithVideoTag() {
        int condition = SscConstant.CONDITION_CFB;
        String ruleId = SscXmlFormat.getDefaultRuleId(SLOT_0, SscXmlFormat.MEDIA_TYPE_VIDEO,
                SscXmlFormat.CD, condition);
        Document doc = removeRule(getEntireXmlDoc(), ruleId);
        updateTagsAndRules(doc);
        String ruleConditionTag = SscXmlFormat.getRuleConditionTag(SLOT_0, SscXmlFormat.CD,
                condition);
        SscXmlFormat.setProvisionStatus(SLOT_0, SscXmlFormat.SC_CD, condition, true);
        SscXmlFormat.setMediaCapability(SLOT_0, SscXmlFormat.SC_CD, SscXmlFormat.MEDIA_TYPE_VIDEO,
                true);

        when(mMockCarrierConfig.getBoolean(CarrierConfig.ImsSs.KEY_UT_INSERT_NEW_RULE_BOOL))
                .thenReturn(true);

        SscServiceData insertData = getInsertCfData(ESsType.CF, SscConstant.ACTION_ACTIVATION,
                condition, null, SscServiceClassUtil.SERVICE_CLASS_VIDEO);
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
        assertEquals(SscXmlFormat.VIDEO, mediaNodeList.item(0).getTextContent());
    }

    @Test
    public void createXml_insertCfnrRegistration() {
        int condition = SscConstant.CONDITION_CFNR;
        String targetNumber = "+1234567890";
        String ruleId = SscXmlFormat.getDefaultRuleId(SLOT_0, SscXmlFormat.MEDIA_TYPE_AUDIO,
                SscXmlFormat.CD, condition);
        Document doc = removeRule(getEntireXmlDoc(), ruleId);
        updateTagsAndRules(doc);
        String ruleConditionTag = SscXmlFormat.getRuleConditionTag(SLOT_0, SscXmlFormat.CD,
                condition);
        SscXmlFormat.setProvisionStatus(SLOT_0, SscXmlFormat.SC_CD, condition, true);

        when(mMockSscUtils.getUriFromNumber(SLOT_0, targetNumber)).thenReturn(targetNumber);
        when(mMockCarrierConfig.getBoolean(CarrierConfig.ImsSs.KEY_UT_INSERT_NEW_RULE_BOOL))
                .thenReturn(true);

        SscServiceData insertData = getInsertCfData(ESsType.CF, SscConstant.ACTION_REGISTRATION,
                condition, targetNumber, SscServiceClassUtil.SERVICE_CLASS_NONE);
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
        String ruleId = SscXmlFormat.getDefaultRuleId(SLOT_0, SscXmlFormat.MEDIA_TYPE_AUDIO,
                SscXmlFormat.CD, condition);
        Document doc = removeRule(getEntireXmlDoc(), ruleId);
        updateTagsAndRules(doc);
        SscXmlFormat.setProvisionStatus(SLOT_0, SscXmlFormat.SC_CD, condition, true);

        when(mMockCarrierConfig.getBoolean(CarrierConfig.ImsSs.KEY_UT_INSERT_NEW_RULE_BOOL))
                .thenReturn(true);

        SscServiceData insertData = getInsertCfData(ESsType.CF, SscConstant.ACTION_DEACTIVATION,
                condition, null, SscServiceClassUtil.SERVICE_CLASS_NONE);
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
        String ruleId = SscXmlFormat.getDefaultRuleId(SLOT_0, SscXmlFormat.MEDIA_TYPE_AUDIO,
                SscXmlFormat.CD, condition);
        Document doc = removeRule(getEntireXmlDoc(), ruleId);
        updateTagsAndRules(doc);
        String ruleConditionTag = SscXmlFormat.getRuleConditionTag(SLOT_0, SscXmlFormat.CD,
                condition);
        SscXmlFormat.setProvisionStatus(SLOT_0, SscXmlFormat.SC_CD, condition, true);

        when(mMockCarrierConfig.getBoolean(CarrierConfig.ImsSs.KEY_UT_INSERT_NEW_RULE_BOOL))
                .thenReturn(true);

        SscServiceData insertData = getInsertCfData(ESsType.CF, SscConstant.ACTION_ERASURE,
                condition, null, SscServiceClassUtil.SERVICE_CLASS_NONE);
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
        assertEquals("", targetList.item(0).getTextContent());
    }

    @Test
    public void createXml_insertCfbActivation() {
        int condition = SscConstant.CONDITION_CFB;
        String ruleId = SscXmlFormat.getDefaultRuleId(SLOT_0, SscXmlFormat.MEDIA_TYPE_AUDIO,
                SscXmlFormat.CD, condition);
        Document doc = removeRule(getEntireXmlDoc(), ruleId);
        updateTagsAndRules(doc);
        String ruleConditionTag = SscXmlFormat.getRuleConditionTag(SLOT_0, SscXmlFormat.CD,
                condition);
        SscXmlFormat.setProvisionStatus(SLOT_0, SscXmlFormat.SC_CD, condition, true);

        when(mMockCarrierConfig.getBoolean(CarrierConfig.ImsSs.KEY_UT_INSERT_NEW_RULE_BOOL))
                .thenReturn(true);

        SscServiceData insertData = getInsertCfData(ESsType.CF, SscConstant.ACTION_ACTIVATION,
                condition, null, SscServiceClassUtil.SERVICE_CLASS_NONE);
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

        when(mMockCarrierConfig.getBoolean(CarrierConfig.ImsSs.KEY_UT_INSERT_NEW_RULE_BOOL))
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
        String ruleId = SscXmlFormat.getDefaultRuleId(SLOT_0, SscXmlFormat.MEDIA_TYPE_AUDIO,
                SscXmlFormat.OCB, condition);
        Document doc = removeRule(getEntireXmlDoc(), ruleId);
        updateTagsAndRules(doc);
        String ruleConditionTag = SscXmlFormat.getRuleConditionTag(SLOT_0, SscXmlFormat.OCB,
                condition);
        SscXmlFormat.setProvisionStatus(SLOT_0, SscXmlFormat.SC_CB, condition, true);

        when(mMockCarrierConfig.getBoolean(CarrierConfig.ImsSs.KEY_UT_INSERT_NEW_RULE_BOOL))
                .thenReturn(true);

        SscServiceData insertData = getInsertCbData(ESsType.OCB, SscConstant.STATUS_ENABLE,
                condition);
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

        when(mMockCarrierConfig.getBoolean(CarrierConfig.ImsSs.KEY_UT_INSERT_NEW_RULE_BOOL))
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
        new SscXmlParser().updateTagsAndRules(SLOT_0, doc);
    }

    private Document getDocumentFromString(String xml) {
        return SscXmlGovTest.createDocumentFromString(xml);
    }

    private Document getEntireXmlDoc() {
        return SscXmlGovTest.createEntireXmlDoc();
    }

    private Document removeRule(Document doc, String ruleId) {
        return SscXmlGovTest.removeRule(doc, ruleId);
    }

    private Document removeRuleSet(Document document) {
        return SscXmlGovTest.removeRuleSet(document);
    }

    private SscServiceData getUpdateData(ESsType ssType, int action) {
        return SscXmlGovTest.createUpdateData(ssType, 0, action, SscConstant.CONDITION_INVALID,
                null, SscServiceClassUtil.SERVICE_CLASS_NONE, 0);
    }

    private SscServiceData getCfUpdateData(ESsType ssType, int action, int condition,
            String targetNumber, int serviceClass, int noReplyTimer) {
        return SscXmlGovTest.createUpdateData(ssType, 0, action, condition, targetNumber,
                serviceClass, noReplyTimer);
    }

    private SscServiceData getCbUpdateData(ESsType ssType, int action, int condition,
            int serviceClass) {
        return SscXmlGovTest.createUpdateData(ssType, 0, action, condition, null,
                serviceClass, 0);
    }

    private SscServiceData getInsertCfData(ESsType ssType, int action, int condition,
            String targetNumber, int serviceClass) {
        return SscXmlGovTest.createInsertData(ssType, 0, action, condition, targetNumber,
                serviceClass);
    }

    private SscServiceData getInsertCbData(ESsType ssType, int action, int condition) {
        return SscXmlGovTest.createInsertData(ssType, 0, action, condition, null,
                SscServiceClassUtil.SERVICE_CLASS_NONE);
    }

    private class FakeSscXmlCreator extends SscXmlCreator {
        @Override
        protected SscUtils getSscUtils() {
            return mMockSscUtils;
        }
    }
}
