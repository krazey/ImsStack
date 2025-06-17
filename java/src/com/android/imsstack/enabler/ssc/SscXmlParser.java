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

import android.text.TextUtils;

import com.android.imsstack.enabler.ssc.data.CbServiceData;
import com.android.imsstack.enabler.ssc.data.CfServiceData;
import com.android.imsstack.enabler.ssc.data.CwServiceData;
import com.android.imsstack.enabler.ssc.data.ErrorResponseData;
import com.android.imsstack.enabler.ssc.data.OipServiceData;
import com.android.imsstack.enabler.ssc.data.OirServiceData;
import com.android.imsstack.enabler.ssc.data.SscRuleData;
import com.android.imsstack.enabler.ssc.data.SscRuleElement;
import com.android.imsstack.enabler.ssc.data.SscServiceData;
import com.android.imsstack.enabler.ssc.data.SscServiceQueryData;
import com.android.imsstack.enabler.ssc.data.TipServiceData;
import com.android.imsstack.enabler.ssc.data.TirServiceData;
import com.android.imsstack.util.ImsLog;

import org.w3c.dom.Document;
import org.w3c.dom.Element;
import org.w3c.dom.Node;
import org.w3c.dom.NodeList;

import java.util.ArrayList;

public class SscXmlParser {
    public SscXmlParser() {
        ImsLog.d("");
    }

    /**
     * This method returns supplementary service data getting from XML of XCAP server
     * @param queryData queried data that is used when sending HTTP GET request
     * @param doc XML data received from XCAP server
     * @param cachedDoc XML data that is cached from previous response from XCAP server
     * @return parsed and optimized data according to queryData
     */
    public SscServiceData getSscServiceFromDoc(SscServiceQueryData queryData, Document doc,
            Document cachedDoc) {
        int slotId = queryData.getSlotId();
        ImsLog.d(slotId, "");

        if (doc == null) {
            ImsLog.e(slotId, "document is null");
            return null;
        }

        int responseCode = queryData.getResponseCode();
        if (responseCode < 200 || responseCode >= 300) {
            return getErrorData(queryData, doc);
        }

        SscServiceData data = null;
        switch(queryData.getSsType()) {
            case OIP:
                data = getOipServiceData(queryData, doc);
                break;
            case OIR:
                data = getOirServiceData(queryData, doc);
                break;
            case TIP:
                data = getTipServiceData(queryData, doc);
                break;
            case TIR:
                data = getTirServiceData(queryData, doc);
                break;
            case CF:
                data = getCfServiceData(queryData, doc);
                break;
            case OCB:
            case ICB:
                data = getCbServiceData(queryData, doc);
                break;
            case CW:
                data = getCwServiceData(queryData, doc);
                break;
            case NONE:
                updateTagsAndRules(slotId, doc);
                if (SscConfig.insertNewRule(slotId)) {
                    updateCfProvisionStatusFromServiceCapability(slotId, doc);
                    updateCbProvisionStatusFromServiceCapability(slotId, doc);
                }
                data = new SscServiceData(slotId, queryData.getSsType(), queryData.getEventNumber(),
                        queryData.getTransactionId(), 1);
                break;
            default:
                ImsLog.e("Invalid SS Type");
                return null;
        }

        if (data != null && queryData.getSsType() != ESsType.NONE) {
            updateCachedDoc(queryData, doc, cachedDoc);
        }

        return data;
    }

    protected void updateTagsAndRules(int slotId, Document doc) {
        Element rootElement = doc.getDocumentElement();
        updateTags(slotId, rootElement);
        updateRuleIds(slotId, rootElement);
        checkCfnrTimerPosition(slotId, rootElement);
        checkCfnlRuleExist(slotId, rootElement);
        SscXmlFormat.displayTags(slotId);
    }

    private void updateCachedDoc(SscServiceQueryData queryData, Document doc, Document cachedDoc) {
        if (cachedDoc == null) {
            return;
        }

        int slotId = queryData.getSlotId();
        NodeList serviceElementList = cachedDoc.getElementsByTagName(
                SscXmlFormat.getSsElement(slotId, queryData.getSsType().getSsName()));
        if (serviceElementList.getLength() == 0) {
            return;
        }

        // Always get service elements when query
        Node serviceElement = serviceElementList.item(0);
        if (serviceElement == null) {
            return;
        }

        Node newElement = doc.getDocumentElement();
        serviceElement.getParentNode()
                .replaceChild(cachedDoc.importNode(newElement, true), serviceElement);
    }

    private void updateTags(int slotId, Node element) {
        if (element == null) {
            return;
        }

        if (element.getNodeType() != Node.ELEMENT_NODE) {
            return;
        }

        String tagName = element.getNodeName();
        if (TextUtils.isEmpty(tagName)) {
            return;
        }

        String[] tokens = tagName.split(":");
        if (tokens.length > 1) {
            if (isNamespaceValid(tokens[0])) {
                SscXmlFormat.setTag(slotId, tokens[1], tokens[0]);
            } else {
                ImsLog.d(slotId, "Invalid namespace : " + tokens[0]);
            }
        } else {
            SscXmlFormat.setTag(slotId, tokens[0], "");
        }

        if (element.hasChildNodes()) {
            NodeList childNodeList = element.getChildNodes();
            for (int i = 0; i < childNodeList.getLength(); i++) {
                updateTags(slotId, childNodeList.item(i));
            }
        }
    }

    private void updateRuleIdForService(int slotId, ESsType ssType, Element rootElement) {
        NodeList serviceElement = rootElement.getElementsByTagName(
                SscXmlFormat.getSsElement(slotId, ssType.getSsName()));
        if (serviceElement.getLength() == 0) {
            ImsLog.d(slotId, ssType.getSsName() + " is null");
            return;
        }

        SscServiceQueryData queryData = new SscServiceQueryData(slotId, ssType,
                SscConstant.EVENT_SSC_QUERY_DOCUMENT, 0, SscServiceClassUtil.SERVICE_CLASS_CALL);
        ArrayList<SscRuleData> ruleSet = getRuleSetData(queryData, (Element)serviceElement.item(0));
        if (ruleSet == null) {
            return;
        }

        for(SscRuleData ruleData : ruleSet) {
            if (ruleData.getSsCondition() == SscConstant.CONDITION_INVALID) {
                continue;
            }

            if (SscServiceClassUtil.hasVideo(ruleData.getServiceClass())) {
                SscXmlFormat.setRuleId(slotId, SscXmlFormat.MEDIA_TYPE_VIDEO, ssType.getSsName(),
                        ruleData.getSsCondition(), ruleData.getRuleId());
            } else {
                SscXmlFormat.setRuleId(slotId, SscXmlFormat.MEDIA_TYPE_AUDIO, ssType.getSsName(),
                        ruleData.getSsCondition(), ruleData.getRuleId());
            }
        }
    }

    private void updateRuleIds(int slotId, Element rootElement) {
        ImsLog.d(slotId, "");
        if (rootElement == null) {
            return;
        }

        updateRuleIdForService(slotId, ESsType.CF, rootElement);
        updateRuleIdForService(slotId, ESsType.ICB, rootElement);
        updateRuleIdForService(slotId, ESsType.OCB, rootElement);
    }

    private void checkCfnrTimerPosition(int slotId, Element rootElement) {
        NodeList timerList = rootElement.getElementsByTagName(
                SscXmlFormat.getSsElement(slotId, SscXmlFormat.NOREPLYTIMER));
        if (timerList.getLength() == 0) {
            SscXmlFormat.setIsNoReplyTimerOmitted(slotId, true);

            if (SscConfig.getCfnrTimerUpdateMethod(slotId)
                    == SscConfig.CFNR_TIMER_UPDATE_METHOD_CDIV_NODE) {
                SscXmlFormat.setIsNoReplyTimerInRule(slotId, false);
            } else if (SscConfig.getCfnrTimerUpdateMethod(slotId)
                    == SscConfig.CFNR_TIMER_UPDATE_METHOD_ACTIONS_NODE) {
                SscXmlFormat.setIsNoReplyTimerInRule(slotId, true);
            }
            return;
        }

        Element timerElement = (Element) timerList.item(0);
        if (!timerElement.getParentNode().getParentNode().isEqualNode(rootElement)) {
            SscXmlFormat.setIsNoReplyTimerInRule(slotId, true);
        } else {
            SscXmlFormat.setIsNoReplyTimerInRule(slotId, false);
        }
    }

    private void checkCfnlRuleExist(int slotId, Element rootElement) {
        NodeList elementList = rootElement.getElementsByTagName(
                SscXmlFormat.getSsElement(slotId, SscXmlFormat.CFNL));
        if (elementList.getLength() > 0) {
            SscXmlFormat.setCfnlExist(slotId, true);
        } else {
            SscXmlFormat.setCfnlExist(slotId, false);
        }
    }

    private SscServiceData getErrorData(SscServiceQueryData queryData, Document doc) {
        int slotId = queryData.getSlotId();

        Element rootElement = doc.getDocumentElement();
        String errorPhrase = getErrorPhrase(rootElement);
        ErrorResponseData data = new ErrorResponseData(slotId,
                queryData.getSsType(), queryData.getEventNumber(),
                queryData.getTransactionId(), SscConstant.STATUS_DISABLE,
                queryData.getResponseCode(), errorPhrase);

        ImsLog.d(data.toString());

        return data;
    }

    private OipServiceData getOipServiceData(SscServiceQueryData queryData, Document doc) {
        int slotId = queryData.getSlotId();
        NodeList oipNodeList =
                doc.getElementsByTagName(SscXmlFormat.getSsElement(slotId, SscXmlFormat.OIP));
        if (oipNodeList.getLength() == 0) {
            ImsLog.e(slotId, "target node is null");
            return null;
        }

        Element oipElement = (Element) oipNodeList.item(0);
        int active = getActiveAttribute(oipElement);

        OipServiceData data = new OipServiceData(slotId, queryData.getSsType(),
                queryData.getEventNumber(), queryData.getTransactionId(), active);

        ImsLog.d(slotId, data.toString());

        return data;
    }

    private OirServiceData getOirServiceData(SscServiceQueryData queryData, Document doc) {
        int slotId = queryData.getSlotId();
        NodeList oirNodeList =
                doc.getElementsByTagName(SscXmlFormat.getSsElement(slotId, SscXmlFormat.OIR));
        if (oirNodeList.getLength() == 0) {
            ImsLog.e(slotId, "target node is null");
            return null;
        }

        /*
         * 3GPP 24.607 4.10.1
         * The OIR service can be activated/deactivated using the active attribute of the
         * <originating-identity-presentation-restriction> service element.
         * Activating the OIR service this way activates the temporary mode OIR service.
         * When deactivated and not overruled by operator settings,
         * basic communication procedures apply.
         */
        Element oirElement = (Element) oirNodeList.item(0);
        int active = getActiveAttribute(oirElement);
        int provisionStatus = SscConstant.OIR_NOT_PROVISIONED; //3GPP 27.007 7.7 m
        int outgoingState = SscConstant.OIR_SUPPRESSION; // 3GPP 27.007 7.7 n
        if (active == SscConstant.STATUS_ENABLE) {
            provisionStatus = SscConstant.OIR_TEMPORARY_MODE_PRESENTATION_RESTRICTED;
            outgoingState = SscConstant.OIR_INVOCATION;

            NodeList dbNodeList = doc.getElementsByTagName(
                    SscXmlFormat.getSsElement(slotId, SscXmlFormat.DEFAULT_BEHAVIOUR));
            if (dbNodeList.getLength() > 0) {
                Element dbElement = (Element) dbNodeList.item(0);
                String defaultBehaviour = dbElement.getTextContent().trim();
                if (SscXmlFormat.PRESENTATION_NOT_RESTRICTED.equalsIgnoreCase(defaultBehaviour)) {
                    provisionStatus = SscConstant.OIR_TEMPORARY_MODE_PRESENTATION_ALLOWED;
                    outgoingState = SscConstant.OIR_SUPPRESSION;
                }
            }
        }

        int state = (provisionStatus == SscConstant.OIR_TEMPORARY_MODE_PRESENTATION_RESTRICTED) ?
                SscConstant.STATUS_ENABLE : SscConstant.STATUS_DISABLE;

        OirServiceData data = new OirServiceData(slotId, queryData.getSsType(),
                queryData.getEventNumber(), queryData.getTransactionId(), state, provisionStatus,
                outgoingState);

        ImsLog.d(slotId, data.toString());

        return data;
    }

    private TipServiceData getTipServiceData(SscServiceQueryData queryData, Document doc) {
        int slotId = queryData.getSlotId();
        NodeList tipNodeList =
                doc.getElementsByTagName(SscXmlFormat.getSsElement(slotId, SscXmlFormat.TIP));
        if (tipNodeList.getLength() == 0) {
            ImsLog.e(slotId, "target node is null");
            return null;
        }

        Element tipElement = (Element) tipNodeList.item(0);
        int active = getActiveAttribute(tipElement);

        TipServiceData data = new TipServiceData(slotId, queryData.getSsType(),
                queryData.getEventNumber(), queryData.getTransactionId(), active);

        ImsLog.d(slotId, data.toString());

        return data;
    }

    private TirServiceData getTirServiceData(SscServiceQueryData queryData, Document doc) {
        int slotId = queryData.getSlotId();
        NodeList tirNodeList =
                doc.getElementsByTagName(SscXmlFormat.getSsElement(slotId, SscXmlFormat.TIR));
        if (tirNodeList.getLength() == 0) {
            ImsLog.e(slotId, "target node is null");
            return null;
        }

        Element tirElement = (Element) tirNodeList.item(0);
        int active = getActiveAttribute(tirElement);
        int provisionStatus = SscConstant.TIR_NOT_PROVISIONED; // 3GPP 27.007 7.31 m
        if (active == SscConstant.STATUS_ENABLE) {
            provisionStatus = SscConstant.TIR_PROVISIONED;

            NodeList dbNodeList = doc.getElementsByTagName(
                    SscXmlFormat.getSsElement(slotId, SscXmlFormat.DEFAULT_BEHAVIOUR));
            if (dbNodeList.getLength() > 0) {
                Element dbElement = (Element) dbNodeList.item(0);
                String defaultBehaviour = dbElement.getTextContent().trim();
                if (SscXmlFormat.PRESENTATION_NOT_RESTRICTED.equalsIgnoreCase(defaultBehaviour)) {
                    provisionStatus = SscConstant.TIR_NOT_PROVISIONED;
                }
            }
        }

        int state = (provisionStatus == SscConstant.TIR_PROVISIONED) ?
                SscConstant.STATUS_ENABLE : SscConstant.STATUS_DISABLE;

        TirServiceData data = new TirServiceData(slotId, queryData.getSsType(),
                queryData.getEventNumber(), queryData.getTransactionId(), state, provisionStatus);

        ImsLog.d(slotId, data.toString());

        return data;
    }

    private SscServiceData getCfServiceData(SscServiceQueryData queryData, Document doc) {
        int slotId = queryData.getSlotId();

        NodeList cdNodeList =
                doc.getElementsByTagName(SscXmlFormat.getSsElement(slotId, SscXmlFormat.CD));
        if (cdNodeList.getLength() == 0) {
            ImsLog.e(slotId, "target node is null");
            return null;
        }

        Element cdElement = (Element) cdNodeList.item(0);
        int active = getActiveAttribute(cdElement);
        int noReplyTimer = getNoPeplyTimer(slotId, cdElement);

        ArrayList<SscRuleData> ruleSet = getRuleSetData(queryData, cdElement);

        CfServiceData data = new CfServiceData(slotId, queryData.getSsType(),
                queryData.getEventNumber(), queryData.getTransactionId(), active,
                queryData.getCondition(), noReplyTimer, ruleSet);

        ImsLog.d(slotId, data.toString());

        return data;
    }

    private void updateCfProvisionStatusFromServiceCapability(int slotId, Document doc) {
        NodeList scCdNodeList = doc.getElementsByTagName(SscXmlFormat.getSsElement(slotId,
                SscXmlFormat.SC_CD));
        if (scCdNodeList.getLength() == 0) {
            ImsLog.e(slotId, "target node is null");
            return;
        }

        Element scCdElement = (Element) scCdNodeList.item(0);
        int active = getActiveAttribute(scCdElement);
        if (active == SscConstant.STATUS_DISABLE) {
            return;
        }

        NodeList conditionNodeList = scCdElement.getElementsByTagName(
                SscXmlFormat.getSsElement(slotId, SscXmlFormat.SC_CONDITIONS));
        if (conditionNodeList.getLength() == 0) {
            ImsLog.e(slotId, "target node is null");
            return;
        }

        Element conditionElement = (Element) conditionNodeList.item(0);
        NodeList conditionCfuNodeList = conditionElement.getElementsByTagName(
                SscXmlFormat.getSsElement(slotId, SscXmlFormat.SC_CFU));
        if (conditionCfuNodeList.getLength() > 0) {
            SscXmlFormat.setProvisionStatus(slotId, SscXmlFormat.SC_CD, SscConstant.CONDITION_CFU,
                    getProvisionedAttribute((Element) conditionCfuNodeList.item(0)));
        }
        NodeList conditionCfbNodeList = conditionElement.getElementsByTagName(
                SscXmlFormat.getSsElement(slotId, SscXmlFormat.SC_CFB));
        if (conditionCfbNodeList.getLength() > 0) {
            SscXmlFormat.setProvisionStatus(slotId, SscXmlFormat.SC_CD, SscConstant.CONDITION_CFB,
                    getProvisionedAttribute((Element) conditionCfbNodeList.item(0)));
        }
        NodeList conditionCfnrNodeList = conditionElement.getElementsByTagName(
                SscXmlFormat.getSsElement(slotId, SscXmlFormat.SC_CFNR));
        if (conditionCfnrNodeList.getLength() > 0) {
            SscXmlFormat.setProvisionStatus(slotId, SscXmlFormat.SC_CD, SscConstant.CONDITION_CFNR,
                    getProvisionedAttribute((Element) conditionCfnrNodeList.item(0)));
        }
        NodeList conditionCfnrcNodeList = conditionElement.getElementsByTagName(
                SscXmlFormat.getSsElement(slotId, SscXmlFormat.SC_CFNRC));
        if (conditionCfnrcNodeList.getLength() > 0) {
            SscXmlFormat.setProvisionStatus(slotId, SscXmlFormat.SC_CD, SscConstant.CONDITION_CFNRC,
                    getProvisionedAttribute((Element) conditionCfnrcNodeList.item(0)));
        }
        NodeList conditionCfnlNodeList = conditionElement.getElementsByTagName(
                SscXmlFormat.getSsElement(slotId, SscXmlFormat.SC_CFNL));
        if (conditionCfnlNodeList.getLength() > 0) {
            SscXmlFormat.setProvisionStatus(slotId, SscXmlFormat.SC_CD, SscConstant.CONDITION_CFNL,
                    getProvisionedAttribute((Element) conditionCfnlNodeList.item(0)));
        }

        NodeList scMediaNodeList = conditionElement.getElementsByTagName(
                SscXmlFormat.getSsElement(slotId, SscXmlFormat.SC_MEDIA));
        if (scMediaNodeList.getLength() > 0) {
            Element scMediaElement = (Element) scMediaNodeList.item(0);
            NodeList mediaNodeList = scMediaElement.getElementsByTagName(
                    SscXmlFormat.getSsElement(slotId, SscXmlFormat.MEDIA));
            for (int i = 0; i < mediaNodeList.getLength(); i++) {
                String mediaValue = mediaNodeList.item(i).getTextContent().trim();
                if (SscXmlFormat.AUDIO.equalsIgnoreCase(mediaValue)) {
                    SscXmlFormat.setMediaCapability(slotId, SscXmlFormat.SC_CD,
                            SscXmlFormat.MEDIA_TYPE_AUDIO, true);
                } else if (SscXmlFormat.VIDEO.equalsIgnoreCase(mediaValue)) {
                    SscXmlFormat.setMediaCapability(slotId, SscXmlFormat.SC_CD,
                            SscXmlFormat.MEDIA_TYPE_VIDEO, true);
                }
            }
        }
    }

    private SscServiceData getCbServiceData(SscServiceQueryData queryData, Document doc) {
        int slotId = queryData.getSlotId();

        NodeList cbNodeList;
        if (queryData.getSsType() == ESsType.ICB) {
            cbNodeList =
                    doc.getElementsByTagName(SscXmlFormat.getSsElement(slotId, SscXmlFormat.ICB));
        } else { // ESsType.OCB
            cbNodeList =
                    doc.getElementsByTagName(SscXmlFormat.getSsElement(slotId, SscXmlFormat.OCB));
        }

        if (cbNodeList.getLength() == 0) {
            ImsLog.e(slotId, "target node is null");
            return null;
        }

        Element cbElement = (Element) cbNodeList.item(0);
        int active = getActiveAttribute(cbElement);

        ArrayList<SscRuleData> ruleSet = getRuleSetData(queryData, cbElement);

        CbServiceData data = new CbServiceData(slotId, queryData.getSsType(),
                queryData.getEventNumber(), queryData.getTransactionId(), active,
                queryData.getCondition(), ruleSet);

        ImsLog.d(slotId, data.toString());

        return data;
    }

    private void updateCbProvisionStatusFromServiceCapability(int slotId, Document doc) {
        NodeList scCbNodeList = doc.getElementsByTagName(
                SscXmlFormat.getSsElement(slotId, SscXmlFormat.SC_CB));
        if (scCbNodeList.getLength() == 0) {
            ImsLog.e(slotId, "target node is null");
            return;
        }

        Element scCbElement = (Element) scCbNodeList.item(0);
        int active = getActiveAttribute(scCbElement);
        if (active == SscConstant.STATUS_DISABLE) {
            return;
        }

        NodeList conditionNodeList = scCbElement.getElementsByTagName(
                SscXmlFormat.getSsElement(slotId, SscXmlFormat.SC_CONDITIONS));
        if (conditionNodeList.getLength() == 0) {
            ImsLog.e(slotId, "target node is null");
            return;
        }

        Element conditionElement = (Element) conditionNodeList.item(0);
        NodeList conditionCbuNodeList = conditionElement.getElementsByTagName(
                SscXmlFormat.getSsElement(slotId, SscXmlFormat.SC_CBU));
        if (conditionCbuNodeList.getLength() > 0) {
            SscXmlFormat.setProvisionStatus(slotId, SscXmlFormat.SC_CB, SscConstant.CONDITION_BAIC,
                    getProvisionedAttribute((Element) conditionCbuNodeList.item(0)));
            SscXmlFormat.setProvisionStatus(slotId, SscXmlFormat.SC_CB, SscConstant.CONDITION_BAOC,
                    getProvisionedAttribute((Element) conditionCbuNodeList.item(0)));
        }
        NodeList conditionBoicNodeList = conditionElement.getElementsByTagName(
                SscXmlFormat.getSsElement(slotId, SscXmlFormat.SC_BOIC));
        if (conditionBoicNodeList.getLength() > 0) {
            SscXmlFormat.setProvisionStatus(slotId, SscXmlFormat.SC_CB, SscConstant.CONDITION_BOIC,
                    getProvisionedAttribute((Element) conditionBoicNodeList.item(0)));
        }
        NodeList conditionBoicExhcNodeList = conditionElement.getElementsByTagName(
                SscXmlFormat.getSsElement(slotId, SscXmlFormat.SC_BOIC_EXHC));
        if (conditionBoicExhcNodeList.getLength() > 0) {
            SscXmlFormat.setProvisionStatus(slotId, SscXmlFormat.SC_CB,
                    SscConstant.CONDITION_BOIC_EXHC,
                    getProvisionedAttribute((Element) conditionBoicExhcNodeList.item(0)));
        }
        NodeList conditionBicWrcNodeList = conditionElement.getElementsByTagName(
                SscXmlFormat.getSsElement(slotId, SscXmlFormat.SC_BIC_WR));
        if (conditionBicWrcNodeList.getLength() > 0) {
            SscXmlFormat.setProvisionStatus(slotId, SscXmlFormat.SC_CB,
                    SscConstant.CONDITION_BIC_WR,
                    getProvisionedAttribute((Element) conditionBicWrcNodeList.item(0)));
        }
        NodeList conditionAcrNodeList = conditionElement.getElementsByTagName(
                SscXmlFormat.getSsElement(slotId, SscXmlFormat.SC_ACR));
        if (conditionAcrNodeList.getLength() > 0) {
            SscXmlFormat.setProvisionStatus(slotId, SscXmlFormat.SC_CB, SscConstant.CONDITION_ACR,
                    getProvisionedAttribute((Element) conditionAcrNodeList.item(0)));
        }

        NodeList scMediaNodeList = conditionElement.getElementsByTagName(
                SscXmlFormat.getSsElement(slotId, SscXmlFormat.SC_MEDIA));
        if (scMediaNodeList.getLength() > 0) {
            Element scMediaElement = (Element) scMediaNodeList.item(0);
            NodeList mediaNodeList = scMediaElement.getElementsByTagName(
                    SscXmlFormat.getSsElement(slotId, SscXmlFormat.MEDIA));
            for (int i = 0; i < mediaNodeList.getLength(); i++) {
                String mediaValue = mediaNodeList.item(i).getTextContent().trim();
                if (SscXmlFormat.AUDIO.equalsIgnoreCase(mediaValue)) {
                    SscXmlFormat.setMediaCapability(slotId, SscXmlFormat.SC_CB,
                            SscXmlFormat.MEDIA_TYPE_AUDIO, true);
                } else if (SscXmlFormat.VIDEO.equalsIgnoreCase(mediaValue)) {
                    SscXmlFormat.setMediaCapability(slotId, SscXmlFormat.SC_CB,
                            SscXmlFormat.MEDIA_TYPE_VIDEO, true);
                }
            }
        }
    }

    private SscServiceData getCwServiceData(SscServiceQueryData queryData, Document doc) {
        int slotId = queryData.getSlotId();
        NodeList cwNodeList =
                doc.getElementsByTagName(SscXmlFormat.getSsElement(slotId, SscXmlFormat.CW));
        if (cwNodeList.getLength() == 0) {
            ImsLog.e(slotId, "target node is null");
            return null;
        }

        Element cwElement = (Element) cwNodeList.item(0);
        int active = getActiveAttribute(cwElement);

        CwServiceData data = new CwServiceData(slotId, queryData.getSsType(),
                queryData.getEventNumber(), queryData.getTransactionId(), active);

        ImsLog.d(slotId, data.toString());

        return data;
    }

    private String getErrorPhrase(Node node) {
        if (node.getNodeType() == Node.ELEMENT_NODE) {
            Element element = (Element) node;
            String errorPhrase = element.getAttribute(SscXmlFormat.PHRASE);
            if (!TextUtils.isEmpty(errorPhrase)) {
                return errorPhrase;
            }

            if (element.hasChildNodes()) {
                for (int i = 0; i < element.getChildNodes().getLength(); i++) {
                    errorPhrase = getErrorPhrase(element.getChildNodes().item(i));
                    if (!TextUtils.isEmpty(errorPhrase)) {
                        return errorPhrase;
                    }
                }
            }
        }

        return null;
    }

    private ArrayList<SscRuleData> getRuleSetData(SscServiceQueryData queryData, Element element) {
        int slotId = queryData.getSlotId();
        NodeList ruleList =
                element.getElementsByTagName(SscXmlFormat.getCpElement(slotId, SscXmlFormat.RULE));
        if (ruleList.getLength() == 0) {
            ImsLog.d(slotId, "No rules");
            return null;
        }

        ArrayList<SscRuleData> ruleSet = null;
        for (int i = 0; i < ruleList.getLength(); i++) {
            Element ruleElement = (Element) ruleList.item(i);
            String id = ruleElement.getAttribute(SscXmlFormat.ID);
            ImsLog.d(slotId, "RuleId : " + id);
            if (TextUtils.isEmpty(id)) {
                continue;
            }

            SscRuleData ruleData = getRuleData(queryData, ruleElement);
            if (ruleData == null || ruleData.getSsCondition() == SscConstant.CONDITION_INVALID) {
                continue;
            }

            if (!isRequestedData(queryData, ruleData)) {
                continue;
            }

            ruleData.setRuleId(id);

            /*
            if (ruleData.getServiceClass() == SscServiceClassUtil.SERVICE_CLASS_NONE) {
                ruleData.setServiceClass(SscServiceClassUtil.SERVICE_CLASS_CALL);
            }
             */

            if (ruleSet == null) {
                ruleSet = new ArrayList<SscRuleData>();
            }

            ruleSet.add(ruleData);
        }

        return ruleSet;
    }

    private SscRuleData getRuleData(SscServiceQueryData queryData, Element element) {
        int slotId = queryData.getSlotId();
        NodeList conditionList = element.getElementsByTagName(
                SscXmlFormat.getCpElement(slotId, SscXmlFormat.CONDITIONS));
        if (conditionList.getLength() == 0) {
            ImsLog.d(slotId, "No conditions");
            return null;
        }

        SscRuleData ruleData = new SscRuleData();

        // Set Conditions
        setConditionData(slotId, queryData.getSsType(), (Element) conditionList.item(0), ruleData);

        // Set Actions
        NodeList actionList = element.getElementsByTagName(SscXmlFormat.getCpElement(slotId,
                SscXmlFormat.ACTIONS));
        if (actionList.getLength() > 0) {
            setActionData(slotId, (Element) actionList.item(0), ruleData);
        }

        return ruleData;
    }

    private void setConditionData(int slotId, ESsType requestType, Element conditionElement,
            SscRuleData ruleData) {
        String strCondition = null;
        int state = SscConstant.STATUS_ENABLE;
        ArrayList<SscRuleElement> conditions = new ArrayList<SscRuleElement>();
        if (conditionElement.hasChildNodes()) {
            NodeList childNoeList = conditionElement.getChildNodes();
            for (int i = 0; i < childNoeList.getLength(); i++) {
                Node childNode = childNoeList.item(i);
                if (childNode == null || childNode.getNodeType() != Node.ELEMENT_NODE) {
                    continue;
                }

                String nodeName = childNode.getNodeName();
                if (TextUtils.isEmpty(nodeName)) {
                    continue;
                }

                ImsLog.d(slotId, "conditionName : " + nodeName);

                if (isRuleCondition(nodeName)) {
                    strCondition = nodeName;
                    conditions.add(new SscRuleElement(SscXmlFormat.CONDITIONS, nodeName));
                } else if (nodeName.endsWith(SscXmlFormat.RULE_DEACTIVATED)) {
                    state = SscConstant.STATUS_DISABLE;
                } else if (SscXmlFormat.getSsElement(slotId, SscXmlFormat.MEDIA).equals(nodeName)) {
                    String mediaValue = childNode.getTextContent();
                    ImsLog.d(slotId, "Media Tag : " + mediaValue);
                    if (SscXmlFormat.AUDIO.equalsIgnoreCase(mediaValue)) {
                        ruleData.addServiceClass(SscServiceClassUtil.SERVICE_CLASS_VOICE);
                    } else if (SscXmlFormat.VIDEO.equalsIgnoreCase(mediaValue)) {
                        ruleData.addServiceClass(SscServiceClassUtil.SERVICE_CLASS_VIDEO);
                    }
                }
                /* TODO: This is for call-barring specific number that is not listed in IR92
                 else if (nodeName.endsWith(SscXmlFormat.IDENTITY)) {
                    String identity = getIdentityInCondition(childNode);
                    if (!TextUtils.isEmpty(identity)) {
                        conditions.add(new SscRuleElement(SscXmlFormat.IDENTITY, identity));
                    }
                } else if (childNode.getTextContent() != null) {
                    conditions.add(new SscRuleElement(nodeName, childNode.getTextContent()));
                }
                 */
            }
        }

        int nCondition = getIntCondition(requestType, strCondition);
        ruleData.setSsCondition(nCondition);
        ruleData.setState(state);
        ruleData.setConditionList(conditions);
    }

    private void setActionData(int slotId, Element actionElement, SscRuleData ruleData) {
        if (actionElement.hasChildNodes()) {
            ArrayList<SscRuleElement> actions = new ArrayList<SscRuleElement>();
            NodeList childNoeList = actionElement.getChildNodes();
            for (int i = 0; i < childNoeList.getLength(); i++) {
                Node childNode = childNoeList.item(i);
                if (childNode == null || childNode.getNodeType() != Node.ELEMENT_NODE) {
                    continue;
                }

                String nodeName = childNode.getNodeName();
                if (TextUtils.isEmpty(nodeName)) {
                    continue;
                }

                ImsLog.d(slotId, "actionName : " + nodeName);

                if (nodeName.endsWith(SscXmlFormat.FORWARD_TO)) {
                    String targetNumber = getTargetNumberInForwardTo(slotId, (Element) childNode);
                    if (!TextUtils.isEmpty(targetNumber)) {
                        ruleData.setForwardToNumber(targetNumber);
                    }
                } else if (childNode.getTextContent() != null) {
                    actions.add(new SscRuleElement(nodeName, childNode.getTextContent()));
                }
            }

            ruleData.setActionList(actions);
        }
    }

    private int getIntCondition(ESsType requestType, String strCondition) {
        if (requestType == ESsType.CF) {
            if (strCondition == null) {
                return SscConstant.CONDITION_CFU;
            }

            if (strCondition.endsWith(SscXmlFormat.CFB)) {
                return SscConstant.CONDITION_CFB;
            } else if (strCondition.endsWith(SscXmlFormat.CFNR)) {
                return SscConstant.CONDITION_CFNR;
            } else if (strCondition.endsWith(SscXmlFormat.CFNRC)) {
                return SscConstant.CONDITION_CFNRC;
            } else if (strCondition.endsWith(SscXmlFormat.CFNL)) {
                return SscConstant.CONDITION_CFNL;
            }

            return SscConstant.CONDITION_CFU;
        } else if (requestType == ESsType.OCB) {
            if (strCondition == null) {
                return SscConstant.CONDITION_BAOC;
            }

            if (strCondition.endsWith(SscXmlFormat.BOIC)) {
                return SscConstant.CONDITION_BOIC;
            } else if (strCondition.endsWith(SscXmlFormat.BOIC_EXHC)) {
                return SscConstant.CONDITION_BOIC_EXHC;
            }

            return SscConstant.CONDITION_BAOC;
        } else if (requestType == ESsType.ICB) {
            if (strCondition == null) {
                return SscConstant.CONDITION_BAIC;
            }

            if (strCondition.endsWith(SscXmlFormat.BIC_WR)) {
                return SscConstant.CONDITION_BIC_WR;
            } else if (strCondition.endsWith(SscXmlFormat.ACR)) {
                return SscConstant.CONDITION_ACR;
            }

            return SscConstant.CONDITION_BAIC;
        }

        return SscConstant.CONDITION_INVALID;
    }

    private boolean isRuleCondition(String condition) {
        if (condition.endsWith(SscXmlFormat.CFB) ||
                condition.endsWith(SscXmlFormat.CFNR) ||
                condition.endsWith(SscXmlFormat.CFNRC) ||
                condition.endsWith(SscXmlFormat.CFNL) ||
                condition.endsWith(SscXmlFormat.BOIC) ||
                condition.endsWith(SscXmlFormat.BOIC_EXHC) ||
                condition.endsWith(SscXmlFormat.BIC_WR) ||
                condition.endsWith(SscXmlFormat.ACR)) {
            return true;
        }

        return false;
    }

    private boolean isRequestedData(SscServiceQueryData queryData, SscRuleData responseData) {
        if (queryData == null || responseData == null) {
            return false;
        }

        if (queryData.getEventNumber() == SscConstant.EVENT_SSC_QUERY_DOCUMENT) {
            return true;
        }

        if (queryData.getServiceClass() == SscServiceClassUtil.SERVICE_CLASS_VOICE
                && SscServiceClassUtil.hasVideo(responseData.getServiceClass())) {
            return false;
        }

        if (queryData.getServiceClass() == SscServiceClassUtil.SERVICE_CLASS_VIDEO
                && !SscServiceClassUtil.hasVideo(responseData.getServiceClass())) {
            return false;
        }

        int queryCondition = queryData.getCondition();
        int responseCondition = responseData.getSsCondition();
        if (queryData.getSsType() == ESsType.CF) {
            if (queryCondition == SscConstant.CONDITION_CFA) {
                return true;
            }

            if (queryCondition == SscConstant.CONDITION_CFAC
                    && responseCondition != SscConstant.CONDITION_CFU) {
                return true;
            }
        }

        if (queryCondition == responseCondition) {
            return true;
        }

        return false;
    }

    private String getTargetNumberInForwardTo(int slotId, Element forwardToElement) {
        NodeList targetNodes = forwardToElement.getElementsByTagName(
                SscXmlFormat.getSsElement(slotId, SscXmlFormat.TARGET));
        if (targetNodes.getLength() > 0) {
            return SscUtils.getInstance().getNumberFromUri(slotId,
                    targetNodes.item(0).getTextContent().trim());
        }

        return null;
    }

    /* TODO: This is for call-barring specific number that is not listed in IR92
    private String getIdentityInCondition(Node identityElement) {
        if (identityElement.hasChildNodes() == false) {
            return null;
        }

        NodeList childNoeList = identityElement.getChildNodes();
        for (int i = 0; i < childNoeList.getLength(); i++) {
            Node childNode = childNoeList.item(i);
            if (childNode == null || childNode.getNodeType() != Node.ELEMENT_NODE) {
                continue;
            }

            String nodeName = childNode.getNodeName();
            if (TextUtils.isEmpty(nodeName)) {
                continue;
            }

            Element one = (Element)childNode;
            if (nodeName.endsWith(SscXmlFormat.ONE)) {
                if (one.getAttribute(SscXmlFormat.ID).length() > 0) {
                    // For RFC 4745
                    return one.getAttribute(SscXmlFormat.ID);
                } else {
                    if (one.getTextContent() != null) {
                        // For TS 24.604
                        return one.getTextContent().replace("id=", "");
                    }
                }
            }
        }

        return null;
    }
     */

    private int getActiveAttribute(Element element) {
        String active = element.getAttribute(SscXmlFormat.ACTIVE);
        // 3GPP 24.623 6.1 When the "active" attribute is absent on a service element,
        // it indicates that the service is activated.
        return "false".equalsIgnoreCase(active) ?
                SscConstant.STATUS_DISABLE : SscConstant.STATUS_ENABLE;
    }

    private boolean getProvisionedAttribute(Element element) {
        String provisioned = element.getAttribute(SscXmlFormat.PROVISIONED);
        return !"false".equalsIgnoreCase(provisioned);
    }

    private int getNoPeplyTimer(int slotId, Element element) {
        int noReplyTimer = -1;
        NodeList timerList = element.getElementsByTagName(
                SscXmlFormat.getSsElement(slotId, SscXmlFormat.NOREPLYTIMER));
        if (timerList.getLength() > 0) {
            noReplyTimer = Integer.parseInt(timerList.item(0).getTextContent().trim());
        }
        ImsLog.d("noReplyTimer is " + noReplyTimer);

        return noReplyTimer;
    }

    private static boolean isNamespaceValid(String namespace) {
        return SscXmlFormat.NS_SS_PREFIX.equals(namespace + ":")
                || SscXmlFormat.NS_CP_PREFIX.equals(namespace + ":")
                || SscXmlFormat.NS_XE_PREFIX.equals(namespace + ":");
    }

    // KDDI - getting preferred URI format(SIP) from one element.
    // TODO_JS: Adding carrier configuration or just remove
    /*
    private String getURIFromData(String preferredURI, String element, NodeList nodeList ) {
        String result = null;
        Node oneNode = null;
        String content = null;

        if (nodeList.getLength() == 0) {
            ImsLog.d("node list is null");
            return result;
        }

        for (int conIdx = 0; conIdx < nodeList.getLength(); conIdx++) {
            oneNode = (Node)nodeList.item(conIdx);
            content = oneNode.getAttributes().getNamedItem(element).getTextContent();
            if (content.startsWith(preferredURI)) {
                ImsLog.d("Preferred URI (" + preferredURI + "), idx : " + conIdx);
                result = content;
                break;
            }
        }

        if (result == null) {
            oneNode = (Node)nodeList.item(0);
            result = oneNode.getAttributes().getNamedItem(element).getTextContent();
        }
        return result;
    }
     */
}
