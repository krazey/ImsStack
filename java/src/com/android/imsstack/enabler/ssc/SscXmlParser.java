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

    // NOTICE
    // This SscServiceData doesn't have EventNumber value.
    // It must be set with ESsType info or add the value in other way.
    public SscServiceData getSSCServiceFromDoc(SscServiceQueryData queryData, Document doc,
            Document cachedDoc) {
        ImsLog.d(queryData.getSlotId(), "");

        int responseCode = queryData.getResponseCode();
        if (responseCode < 200 || responseCode >= 300) {
            return getErrorPhrase(queryData, doc);
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
            case ICBA:
                data = getCbServiceData(queryData, doc);
                break;
            case CW:
                data = getCwServiceData(queryData, doc);
                break;
            case NONE:
                updateTagsAndRules(queryData, doc);
                data = new SscServiceData(queryData.getSlotId(), queryData.getSsType(),
                        queryData.getEventNumber(), queryData.getTransactionId(), 1);
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

    private void updateCachedDoc(SscServiceQueryData queryData, Document doc, Document cachedDoc) {
        int slotId = queryData.getSlotId();
        Node newElement = doc.getDocumentElement();
        if (newElement == null) {
            return;
        }

        if (cachedDoc == null) {
            return;
        }

        NodeList serviceElementList = cachedDoc.getElementsByTagName(
                SscXmlFormat.getSsElement(slotId, queryData.getSsType().getSSName()));
        if (serviceElementList.getLength() == 0) {
            return;
        }

        Node serviceElement = serviceElementList.item(0);
        if (serviceElement == null) {
            return;
        }

        serviceElement.getParentNode().replaceChild(cachedDoc.importNode(newElement, true),
                serviceElement);
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
            SscXmlFormat.setTag(slotId, tokens[1], tokens[0]);
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
                SscXmlFormat.getSsElement(slotId, ssType.getSSName()));
        if (serviceElement.getLength() == 0) {
            ImsLog.d(slotId, ssType.getSSName() + " is null");
            return;
        }

        SscServiceQueryData queryData = new SscServiceQueryData(slotId, ssType,
                SscConstant.EVENT_SSC_QUERY_ALL, 0, SscServiceClassUtil.SERVICE_CLASS_CALL);
        ArrayList<SscRuleData> ruleSet = getRuleSetData(queryData, (Element)serviceElement.item(0));
        if (ruleSet == null) {
            return;
        }

        for(SscRuleData ruleData : ruleSet) {
            if (ruleData.getSsCondition() == SscConstant.CONDITION_INVALID) {
                continue;
            }

            if (SscServiceClassUtil.isVideo(ruleData.getServiceClass())) {
                SscXmlFormat.setRuleId(slotId, SscXmlFormat.MEDIA_VIDEO, ssType.getSSName(),
                        ruleData.getSsCondition(), ruleData.getRuleId());
            } else {
                SscXmlFormat.setRuleId(slotId, SscXmlFormat.MEDIA_AUDIO, ssType.getSSName(),
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
            SscXmlFormat.setIsNoReplyTimerInRule(slotId, false);
            return;
        }

        Element timerElement = (Element) timerList.item(0);
        if (timerElement.getParentNode().getParentNode().isEqualNode(rootElement) == false) {
            SscXmlFormat.setIsNoReplyTimerInRule(slotId, true);
        } else {
            SscXmlFormat.setIsNoReplyTimerInRule(slotId, false);
        }
    }

    private void checkCfnlRuleExist(int slotId, Element rootElement) {
        NodeList elementList = rootElement.getElementsByTagName(
                SscXmlFormat.getSsElement(slotId, SscXmlFormat.CFNL));
        if (elementList.getLength() > 0) {
            SscXmlFormat.setIsCfnlProvisioned(slotId, true);
        } else {
            SscXmlFormat.setIsCfnlProvisioned(slotId, false);
        }
    }

    private void updateTagsAndRules(SscServiceQueryData queryData, Document doc) {
        int slotId = queryData.getSlotId();
        ImsLog.d(slotId, "");
        Element rootElement = (Element) doc.getDocumentElement();
        updateTags(slotId, rootElement);
        updateRuleIds(slotId, rootElement);
        checkCfnrTimerPosition(slotId, rootElement);
        checkCfnlRuleExist(slotId, rootElement);
        //SscXmlFormat.displayTags(queryData.getSlotId());
    }

    private SscServiceData getErrorPhrase(SscServiceQueryData queryData, Document doc) {
        int slotId = queryData.getSlotId();
        NodeList xeNodeList =
                doc.getElementsByTagName(SscXmlFormat.getSsElement(slotId, SscXmlFormat.XCAPERROR));
        if (xeNodeList.getLength() == 0) {
            String xeTag = SscXmlFormat.NS_XE_PREFIX + SscXmlFormat.XCAPERROR;
            xeNodeList = doc.getElementsByTagName(xeTag);
            if (xeNodeList.getLength() == 0) {
                xeNodeList = doc.getElementsByTagName(SscXmlFormat.XCAPERROR);
                if (xeNodeList.getLength() == 0) {
                    ImsLog.e(slotId, "target node is null");
                    return null;
                }
            }
        }

        if (xeNodeList.item(0).hasChildNodes() == false) {
            ImsLog.e(slotId, "target node is null");
            return null;
        }

        NodeList childNodeList = xeNodeList.item(0).getChildNodes();
        for (int i = 0; i < childNodeList.getLength(); i++) {
            if (childNodeList.item(i).getNodeType() != Node.ELEMENT_NODE) {
                continue;
            }

            Element element = (Element) childNodeList.item(i);
            Node phraseNode = element.getAttributes().getNamedItem(SscXmlFormat.PHRASE);
            String errorPhrase = (phraseNode != null) ? phraseNode.getTextContent() : null;
            if (TextUtils.isEmpty(errorPhrase) == false) {
                ErrorResponseData data = new ErrorResponseData(slotId,
                        queryData.getSsType(), queryData.getEventNumber(),
                        queryData.getTransactionId(), SscConstant.STATUS_DISABLE,
                        queryData.getResponseCode(), errorPhrase);

                ImsLog.d(data.toString());

                return data;
            }
        }

        return null;
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
                if (SscXmlFormat.PRESENTATION_NOT_RESTRICTED.equals(defaultBehaviour)) {
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
                if (SscXmlFormat.PRESENTATION_NOT_RESTRICTED.equals(defaultBehaviour)) {
                    provisionStatus = SscConstant.TIR_NOT_PROVISIONED;
                }
            }
        }

        int state = (provisionStatus == SscConstant.TIR_PROVISIONED) ?
                SscConstant.STATUS_ENABLE : SscConstant.STATUS_DISABLE;

        TirServiceData data = new TirServiceData(slotId, queryData.getSsType(),
                queryData.getEventNumber(), queryData.getTransactionId(), active, provisionStatus);

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

    /*
    // This class is not using now. made for future usage.
    protected SscServiceData getCFServerCapaData(SscServiceQueryData queryData,
            Document doc, XPath xpath) {
        ImsLog.d("");

        String strTopExpression = "//" + queryData.getSsType().getSSName();
        String expression = strTopExpression;

        Node node = getNode(expression, doc, xpath);
        if (node == null) {
            ImsLog.e("target node is null");
            return null;
        }

        int cfCapaActive = "true".equals(getActiveNodeText(node)) ? 1 : 0;
        CFServerCapaData data = new CFServerCapaData(queryData.getSlotId(),
                                                    queryData.getSsType(),
                                                    queryData.getEventNumber(),
                                                    queryData.getTransactionId(),
                                                    cfCapaActive);

        expression += "/" + SscXmlFormat.SERVCAPCONDITIONS + "/*";

        NodeList conditionsNodeList = getNodeList(expression, doc, xpath);
        if (conditionsNodeList == null) {
            ImsLog.e("conditionNodeList is null");
            return data;
        }

        for (int idx = 0; idx < conditionsNodeList.getLength(); idx++) {
            Element condition = (Element)conditionsNodeList.item(idx);

            if (condition == null) {
                ImsLog.e("condition is null");
                break;
            }

            if (SscXmlFormat.SERVCAPMEDIA.equals(condition.getNodeName())) {
                //NodeList mediaNodeList = condition.getChildNodes();
                expression = strTopExpression
                            + "/" + SscXmlFormat.SERVCAPCONDITIONS
                            + "/" + SscXmlFormat.SERVCAPMEDIA + "/*";

                NodeList mediaNodeList = getNodeList(expression, doc, xpath);
                if (mediaNodeList == null) {
                    ImsLog.e("media Node List is null");
                    continue;
                }

                for (int mediaIdx = 0; mediaIdx < mediaNodeList.getLength(); mediaIdx++) {
                    Element media = (Element)mediaNodeList.item(mediaIdx);
                    if (media == null) {
                        ImsLog.e("media target node is null");
                        break;
                    }

                    if (SscXmlFormat.AUDIO.equals(media.getTextContent())) {
                        data.setServCapMediaAudio(SscConstant.STATUS_ENABLE);
                    } else if (SscXmlFormat.VIDEO.equals(media.getTextContent())) {
                        data.setServCapMediaVideo(SscConstant.STATUS_ENABLE);
                    } else {
                        ImsLog.w("");
                    }
                } // End of Media List
            } else {
                int enable = "true".equals(condition.getAttribute("provisioned")) ? 1 : 0;
                String nodeName = condition.getNodeName();

                if ("serv-cap-anonymous".equals(nodeName)) {
                    data.setServCapAnonymous(enable);
                } else if ("serv-cap-busy".equals(nodeName)) {
                    data.setServCapbusy(enable);
                } else if ("serv-cap-external-list".equals(nodeName)) {
                    data.setServCapExternalList(enable);
                } else if ("serv-cap-identity".equals(nodeName)) {
                    data.setServCapIdentity(enable);
                } else if ("serv-cap-not-registered".equals(nodeName)) {
                    data.setServCapNotRegistered(enable);
                } else if ("serv-cap-no-answer".equals(nodeName)) {
                    data.setServCapNoAnswer(enable);
                } else if ("serv-cap-not-reachable".equals(nodeName)) {
                    data.setServCapNotReachable(enable);
                } else if ("serv-cap-presence-status".equals(nodeName)) {
                    data.setServCapPresenceStatus(enable);
                } else if ("serv-cap-rule-deactivated".equals(nodeName)) {
                    data.setServCapRuleDeactivated(enable);
                } else if ("serv-cap-validity".equals(nodeName)) {
                    data.setServCapValidity(enable);
                } else {
                    ImsLog.w("");
                }
            }
        }

        expression = strTopExpression + "/serv-cap-actions/*";

        NodeList actionNodeList = getNodeList(expression, doc, xpath);
        if (actionNodeList == null) { // In case of No action
            ImsLog.w("actionNodeList is null");
            return data;
        }

        for (int idx = 0; idx < actionNodeList.getLength(); idx++) {
            Element action = (Element)actionNodeList.item(idx);

            if (action == null) {
                ImsLog.e("condition is null");
                break;
            }

            if ("serv-cap-target".equals(action.getNodeName())) {
                expression = strTopExpression
                            + "/serv-cap-actions" + "/serv-cap-target" + "/telephony-type";
                Node childNode = getNode(expression, doc, xpath);
                if (childNode != null) {
                    data.setServCapTarget(childNode.getNodeName());
                }
            } else {
                int enable = "true".equals(action.getAttribute("provisioned")) ? 1 : 0;
                String nodeName = action.getNodeName();

                if ("serv-cap-notify-caller".equals(nodeName)) {
                    data.setServCapNotifyCaller(enable);
                } else if ("serv-cap-notify-served-user".equals(nodeName)) {
                    data.setServCapNotifyServedUser(enable);
                } else if ("serv-cap-notify-served-user-on-outbound-call".equals(nodeName)) {
                    data.setServCapNotifyServedUserOnOutboundCall(enable);
                } else if ("serv-cap-reveal-identity-to-caller".equals(nodeName)) {
                    data.setServCapRevealIdentifyToCaller(enable);
                } else if ("serv-cap-reveal-served-user-identity-to-caller".equals(nodeName)) {
                    data.setServCapRevealServedUserIdentityToCaller(enable);
                } else if ("serv-cap-reveal-identity-to-target".equals(nodeName)) {
                    data.setServCapRevealIdentityToTarget(enable);
                } else {
                    ImsLog.w("");
                }
            }
        }

        ImsLog.d(data.toString());

        return data;
    }
     */

    private SscServiceData getCbServiceData(SscServiceQueryData queryData, Document doc) {
        int slotId = queryData.getSlotId();

        NodeList cbNodeList = null;
        if (queryData.getSsType() == ESsType.ICB) {
            cbNodeList =
                    doc.getElementsByTagName(SscXmlFormat.getSsElement(slotId, SscXmlFormat.ICB));
        } else if (queryData.getSsType() == ESsType.OCB) {
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

/*
    // This class is not using now. made for future usage.
    protected SscServiceData getCBServerCapaData(SscServiceQueryData queryData,
            Document doc, XPath xpath) {
        ImsLog.d("");

        String strTopExpression = "//" + queryData.getSsType().getSSName();
        String expression = strTopExpression;

        Node node = getNode(expression, doc, xpath);
        if (node == null) {
            ImsLog.e("target node is null");
            return null;
        }

        String cbActive = getActiveNodeText(node);
        int CBPavaActive = "true".equals(cbActive) ? 1 : 0;

        CBServerCapaData data = new CBServerCapaData(queryData.getSlotId(),
                                                    queryData.getSsType(),
                                                    queryData.getEventNumber(),
                                                    queryData.getTransactionId(),
                                                    CBPavaActive);

        expression += "/" + SscXmlFormat.SERVCAPCONDITIONS + "/*";

        NodeList conditionsNodeList = getNodeList(expression, doc, xpath);
        if (conditionsNodeList == null) {
            ImsLog.e("conditionNodeList is null");
            return data;
        }

        for (int idx = 0; idx < conditionsNodeList.getLength(); idx++) {
            Element condition = (Element)conditionsNodeList.item(idx);

            if (condition == null) {
                ImsLog.e("condition is null");
                break;
            }

            if (SscXmlFormat.SERVCAPMEDIA.equals(condition.getNodeName())) {

                //NodeList mediaNodeList = condition.getChildNodes();
                expression = strTopExpression
                            + "/" + SscXmlFormat.SERVCAPCONDITIONS
                            + "/" + SscXmlFormat.SERVCAPMEDIA + "/*";

                NodeList mediaNodeList = getNodeList(expression, doc, xpath);
                if (mediaNodeList == null) {
                    ImsLog.e("media Node List is null");
                    continue;
                }
                for (int mediaIdx = 0; mediaIdx < mediaNodeList.getLength(); mediaIdx++) {
                    Node mediaNode = mediaNodeList.item(mediaIdx);

                    if (mediaNode == null) {
                        ImsLog.e("media target node is null");
                        continue;
                    }

                    if (SscXmlFormat.AUDIO.equals(mediaNode.getTextContent())) {
                        data.setServCapMediaAudio(SscConstant.STATUS_ENABLE);
                    } else if (SscXmlFormat.VIDEO.equals(mediaNode.getTextContent())) {
                        data.setServCapMediaVideo(SscConstant.STATUS_ENABLE);
                    } else {
                        ImsLog.w("");
                    }
                } // End of Media List
            } else {
                int enable = "true".equals(condition.getAttribute("provisioned")) ? 1 : 0;
                String nodeName = condition.getNodeName();

                if ("serv-cap-anonymous".equals(nodeName)) {
                    data.setServCapAnonymous(enable);
                } else if ("serv-cap-communication-diverted".equals(nodeName)) {
                    data.setServCapCommunicationDiverted(enable);
                } else if ("serv-cap-external-list".equals(nodeName)) {
                    data.setServCapExternalList(enable);
                } else if ("serv-cap-identity".equals(nodeName)) {
                    data.setServCapIdentity(enable);
                } else if ("serv-cap-international".equals(nodeName)) {
                    data.setServCapInternational(enable);
                } else if ("serv-cap-international-exHC".equals(nodeName)) {
                    data.setServCapInternationalExHC(enable);
                } else if ("serv-cap-other-identity".equals(nodeName)) {
                    data.setServCapOtherIdentity(enable);
                } else if ("serv-cap-presence-status".equals(nodeName)) {
                    data.setServCapPresenceStatus(enable);
                } else if ("serv-cap-roaming".equals(nodeName)) {
                    data.setServCapRoaming(enable);
                } else if ("serv-cap-rule-deactivated".equals(nodeName)) {
                    data.setServCapRuleDeactivated(enable);
                } else if ("serv-cap-validity".equals(nodeName)) {
                    data.setServCapValidity(enable);
                } else {
                    ImsLog.w("");
                }
            }
        }

        ImsLog.d(data.toString());

        return data;
    }
*/
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

/*
    protected SscServiceData getICBAServerData(SscServiceQueryData queryData,
            Document doc, XPath xpath) {
        ImsLog.d("");

        int slotId = queryData.getSlotId();

        String strTopExpression = "//";
        strTopExpression += SscXmlFormat.getXMLSS(slotId) + queryData.getSsType().getSSName();

        String expression = strTopExpression;

        Node node = getNode(expression, doc, xpath);
        if (node == null) {
            ImsLog.e("target node is null");
            return null;
        }

        String cbActive = getActiveNodeText(node);

        expression = "//" + SscXmlFormat.getRuleSet(slotId) + "/*";
        ImsLog.d("expression : " + expression);

        NodeList nodeList = getNodeList(expression, doc, xpath);
        if (nodeList == null) {
            ImsLog.e("nodeList is null");
            return null;
        }

        ArrayList<SscRuleData> ruleSet = null;
        if (SscConfig.isGetRuleSetByRuleID(slotId)) {
            ruleSet = getRuleSetDataByRuleID(queryData, nodeList, xpath);
        }
        if (ruleSet == null) {
            ruleSet = getRuleSetData(queryData, nodeList, xpath);
        }

        CbServiceData data = new CbServiceData(slotId,
                                            queryData.getSsType(),
                                            queryData.getEventNumber(),
                                            queryData.getTransactionId(),
                                            (cbActive.equalsIgnoreCase("true") ? 1 : 0),
                                            queryData.getCondition(),
                                            ruleSet);
        ImsLog.d(data.toString());

        return data;

    }
*/
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

            SscRuleData ruleData = getRuleData(queryData, ruleElement);
            if (!TextUtils.isEmpty(id)) {
                ruleData.setRuleId(id);
            }

            if (ruleData == null || ruleData.getSsCondition() == SscConstant.CONDITION_INVALID) {
                continue;
            }

            if (!isRequestedData(queryData, ruleData)) {
                continue;
            }

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
                    if (SscXmlFormat.AUDIO.equals(mediaValue)) {
                        ruleData.addServiceClass(SscServiceClassUtil.SERVICE_CLASS_VOICE);
                    } else if (SscXmlFormat.VIDEO.equals(mediaValue)) {
                        ruleData.addServiceClass(SscServiceClassUtil.SERVICE_CLASS_DATA_SYNC);
                    }
                } else if (nodeName.endsWith(SscXmlFormat.IDENTITY)) {
                    String identity = getIdentityInCondition(childNode);
                    if (TextUtils.isEmpty(identity) == false) {
                        conditions.add(new SscRuleElement(SscXmlFormat.IDENTITY, identity));
                    }
                } else if (childNode.getTextContent() != null) {
                    conditions.add(new SscRuleElement(nodeName, childNode.getTextContent()));
                }
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
                    String targetNumber = getTargetNumberInForwardTo(slotId, (Element)childNode);
                    if (TextUtils.isEmpty(targetNumber) == false) {
                        actions.add(new SscRuleElement(SscXmlFormat.TARGET, targetNumber));
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
        } else { // CB cases
            if (strCondition == null) {
                if (requestType == ESsType.OCB) {
                    return SscConstant.CONDITION_BAOC;
                } else if (requestType == ESsType.ICB || requestType == ESsType.ICBA) {
                    return SscConstant.CONDITION_BAIC;
                }
            }

            if (strCondition.endsWith(SscXmlFormat.BOIC)) {
                return SscConstant.CONDITION_BOIC;
            } else if (strCondition.endsWith(SscXmlFormat.BOIC_EXHC)) {
                return SscConstant.CONDITION_BOIC_EXHC;
            } else if (strCondition.endsWith(SscXmlFormat.BIC_WR)) {
                return SscConstant.CONDITION_BIC_WR;
            } else if (strCondition.endsWith(SscXmlFormat.ACR)) {
                return SscConstant.CONDITION_ACR;
            }
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

        if (queryData.getEventNumber() == SscConstant.EVENT_SSC_QUERY_ALL) {
            return true;
        }

        if (queryData.getServiceClass() == SscServiceClassUtil.SERVICE_CLASS_VOICE
                && SscServiceClassUtil.isVideo(responseData.getServiceClass())) {
            return false;
        }

        if (queryData.getServiceClass() == SscServiceClassUtil.SERVICE_CLASS_DATA_SYNC
                && responseData.getServiceClass() == SscServiceClassUtil.SERVICE_CLASS_VOICE) {
            //&& !SscServiceClassUtil.isVideo(responseData.getServiceClass())) {
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
                SscXmlFormat.getCpElement(slotId, SscXmlFormat.TARGET));
        if (targetNodes.getLength() > 0) {
            return getNumberInTargetTo(targetNodes.item(0).getTextContent().trim());
        }

        return null;
    }

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

    private String getNumberInTargetTo(final String targetTo) {
        if (targetTo == null) {
            return null;
        }

        String targetNumber = "";
        // tel:+4477009900123 -> +4477009900123
        // tel:004477009900123;phone-context=exampl.com -> 004477009900123
        if (targetTo.startsWith("tel:")) {
            int beginIndex = 4;
            int endIndex = targetTo.indexOf(";phone-context");
            if (endIndex == -1 || beginIndex > endIndex || beginIndex >= targetTo.length()) {
                targetNumber = targetTo.substring(beginIndex);
            } else {
                targetNumber = targetTo.substring(beginIndex, endIndex);
            }
        }
        // sip:+4477009900123@example.com;user=phone -> +4477009900123
        // sip:004477009900123;phone-context=example.com@example.com;user=phone -> 004477009900123
        else if (targetTo.startsWith("sip:")) {
            int beginIndex = 4;
            int endIndex = targetTo.indexOf(";phone-context");
            if (endIndex == -1) {
                endIndex = targetTo.indexOf("@");
            }

            if (endIndex == -1 || beginIndex > endIndex || beginIndex >= targetTo.length()) {
                targetNumber = targetTo.substring(beginIndex);
            } else {
                targetNumber = targetTo.substring(beginIndex, endIndex);
            }
        } else {
            targetNumber = targetTo;
        }

        ImsLog.d("forwarded to is " + targetNumber);

        return targetNumber;
    }

    private int getActiveAttribute(Element element) {
        String active = element.getAttribute(SscXmlFormat.ACTIVE);
        // 3GPP 24.623 6.1 When the "active" attribute is absent on a service element,
        // it indicates that the service is activated.
        return "false".equalsIgnoreCase(active) ?
                SscConstant.STATUS_DISABLE : SscConstant.STATUS_ENABLE;
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

    // KDDI - If the number begins with “+81”, the device shall translate it to “0”.
    // TODO_JS: Will be removed or modified according to base operation.
    /*
    protected String getNumberInTargetTo(String targetNumber) {
        String result = null;
        if (targetNumber == null || targetNumber.isEmpty()) {
            ImsLog.d("targetNumber is null");
            return result;
        }

        if (targetNumber.startsWith("sip:")
            || targetNumber.startsWith("tel:")) {
            result = targetNumber.substring(4);
            if (result.startsWith("+81")) {
                result = "0" + result.substring(3);
            }
            if (result.contains("@")) {
                result = result.substring(0, result.indexOf("@"));
            }
            if (result.contains(";")) {
                result = result.substring(0, result.indexOf(";"));
            }
        } else {
            result = targetNumber;
        }

        ImsLog.d("number: " + result );
        return result;
    }
     */

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
