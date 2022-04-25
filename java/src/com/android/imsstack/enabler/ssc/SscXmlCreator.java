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

import com.android.imsstack.enabler.ssc.data.CfServiceUpdateData;
import com.android.imsstack.enabler.ssc.data.SscServiceData;
import com.android.imsstack.util.ImsLog;

import java.util.Hashtable;

import org.w3c.dom.Document;
import org.w3c.dom.Element;
import org.w3c.dom.NodeList;

public class SscXmlCreator {
    protected Hashtable<ESsType, IXmlCreator> mXMLCreatorTable = null;

    public SscXmlCreator() {
        ImsLog.d("");

        if (mXMLCreatorTable == null) {
            mXMLCreatorTable = new Hashtable<ESsType, IXmlCreator>();
        }

        mXMLCreatorTable.put(ESsType.OIP,   new XmlCreatorOip());
        mXMLCreatorTable.put(ESsType.OIR,   new XmlCreatorOir());
        mXMLCreatorTable.put(ESsType.TIP,   new XmlCreatorTip());
        mXMLCreatorTable.put(ESsType.TIR,   new XmlCreatorTir());

        mXMLCreatorTable.put(ESsType.CF,    new XmlCreatorCf());
        mXMLCreatorTable.put(ESsType.OCB,   new XmlCreatorCb());
        mXMLCreatorTable.put(ESsType.ICB,   new XmlCreatorCb());
        mXMLCreatorTable.put(ESsType.CW,    new XmlCreatorCw());

        //mXMLCreatorTable.put(ESsType.ICBA,   new XMLCreatorICBA());
        mXMLCreatorTable.put(ESsType.ICBA,   new XmlCreatorCb());
    }

    public Element createXML(Document doc, SscServiceData data) {
        if (doc == null) {
            ImsLog.e("doc is null");
            return null;
        }

        if (data == null) {
            ImsLog.e("input value is null");
            return null;
        }

        IXmlCreator xmlCreator = mXMLCreatorTable.get(data.getSsType());
        if (xmlCreator == null) {
            ImsLog.e("XML creator is null");
            return null;
        }

        return xmlCreator.createXMLElement(doc, data);
    }

    private Element getElementByTagName(Element rootElement, String tagName) {
        NodeList elementList = rootElement.getElementsByTagName(tagName);
        if (elementList.getLength() == 0) {
            return null;
        }

        return (Element) elementList.item(0);
    }

    private Element getElementByTagNameWithId(Element rootElement, String tagName, String id) {
        NodeList elementList = rootElement.getElementsByTagName(tagName);
        if (elementList.getLength() == 0) {
            return null;
        }

        for (int i = 0; i < elementList.getLength(); i++) {
            Element rule = (Element) elementList.item(i);
            if (rule.getAttribute(SscXmlFormat.ID).equals(id)) {
                return rule;
            }
        }

        return null;
    }

    private Element updateServiceElement(Document doc, int slotId, String serviceName, int state) {
        Element rootElement = doc.getDocumentElement();
        if (rootElement == null) {
            return null;
        }

        String serviceTag = SscXmlFormat.getSsElement(slotId, serviceName);
        Element serviceElement = getElementByTagName(rootElement, serviceTag);
        if (serviceElement == null) {
            ImsLog.d(serviceTag + " is null");
            return null;
        }

        String active = (state == SscConstant.STATUS_DISABLE) ? "false" : "true";
        serviceElement.setAttribute(SscXmlFormat.ACTIVE, active);

        return serviceElement;
    }

    private Element updateRuleElement(Document doc, int slotId, String ruleId, int state) {
        Element rootElement = doc.getDocumentElement();
        if (rootElement == null) {
            return null;
        }

        String ruleTag = SscXmlFormat.getCpElement(slotId, SscXmlFormat.RULE);
        Element ruleElement = getElementByTagNameWithId(rootElement, ruleTag, ruleId);
        if (ruleElement == null) {
            ImsLog.d(ruleId + " is null");
            return null;
        }

        String conditionTag = SscXmlFormat.getCpElement(slotId, SscXmlFormat.CONDITIONS);
        Element conditionElement = getElementByTagName(ruleElement, conditionTag);
        if (conditionElement == null) {
            ImsLog.d("condition is null");
            return null;
        }

        String deactivationTag =
                SscXmlFormat.getSsElement(slotId, SscXmlFormat.RULE_DEACTIVATED);
        NodeList ruleDeactivatedList = conditionElement.getElementsByTagName(deactivationTag);

        if (state == SscConstant.STATUS_ENABLE && ruleDeactivatedList.getLength() > 0) {
            Element ruleDeactivatedElement = (Element) ruleDeactivatedList.item(0);
            conditionElement.removeChild(ruleDeactivatedElement);
        } else if (state == SscConstant.STATUS_DISABLE && ruleDeactivatedList.getLength() == 0) {
            Element ruleDeactivatedElement = doc.createElement(deactivationTag);
            conditionElement.appendChild(ruleDeactivatedElement);
        }

        return ruleElement;
    }

    public interface IXmlCreator {
        Element createXMLElement(Document doc, SscServiceData data);
    }

    protected class XmlCreatorOip implements IXmlCreator {
        @Override
        public Element createXMLElement(Document doc, SscServiceData data) {
            int slotId = data.getSlotId();
            ImsLog.d(slotId, "");
            return updateServiceElement(doc, slotId, data.getSsType().getSSName(), data.getState());
        }
    }

    protected class XmlCreatorOir implements IXmlCreator {
        @Override
        public Element createXMLElement(Document doc, SscServiceData data) {
            int slotId = data.getSlotId();
            ImsLog.d(slotId, "");

            int state = SscConstant.STATUS_ENABLE;
            int operation = data.getState();
            if (operation == SscConstant.OIR_DEFAULT) {
                int defaultOperation = SscConfig.getOirNetworkDefaultOperation(slotId);
                if (defaultOperation == SscConfig.OIR_NOT_PROVISIONED) {
                    state = SscConstant.STATUS_DISABLE;
                } else if (defaultOperation == SscConfig.OIR_TEMP_MODE_RESTRICTED) {
                    operation = SscConstant.OIR_INVOCATION;
                } else if (defaultOperation == SscConfig.OIR_TEMP_MODE_ALLOWED) {
                    operation = SscConstant.OIR_SUPPRESSION;
                } else if (defaultOperation == SscConfig.OIR_TEMP_MODE_WITHOUT_DEFAULT_BEHAVIOUR) {
                    // do nothing
                }
            } else if (operation == SscConstant.OIR_SUPPRESSION) {
                if (SscConfig.isOirTirAlwaysTemporaryMode(slotId)) {
                    state = SscConstant.STATUS_ENABLE;
                } else {
                    state = SscConstant.STATUS_DISABLE;
                }
            }

            Element oirServiceElement =
                    updateServiceElement(doc, slotId, data.getSsType().getSSName(), state);
            if (oirServiceElement == null) {
                return null;
            }

            String defaultBehaviourValue = null;
            if (operation == SscConstant.OIR_INVOCATION) {
                defaultBehaviourValue = SscXmlFormat.PRESENTATION_RESTRICTED;
            } else if (operation == SscConstant.OIR_SUPPRESSION) {
                defaultBehaviourValue = SscXmlFormat.PRESENTATION_NOT_RESTRICTED;
            }

            if (defaultBehaviourValue != null) {
                String dbTag = SscXmlFormat.getSsElement(slotId, SscXmlFormat.DEFAULT_BEHAVIOUR);
                Element defaultBehaviour = getElementByTagName(oirServiceElement, dbTag);
                if (defaultBehaviour != null) {
                    defaultBehaviour.setTextContent(defaultBehaviourValue);
                } else {
                    defaultBehaviour = doc.createElement(dbTag);
                    defaultBehaviour.setTextContent(defaultBehaviourValue);
                    oirServiceElement.appendChild(defaultBehaviour);
                }
            }

            return oirServiceElement;
        }
    }

    protected class XmlCreatorTip implements IXmlCreator {
        @Override
        public Element createXMLElement(Document doc, SscServiceData data) {
            int slotId = data.getSlotId();
            ImsLog.d(slotId, "");
            return updateServiceElement(doc, slotId, data.getSsType().getSSName(), data.getState());
        }
    }

    protected class XmlCreatorTir implements IXmlCreator {
        @Override
        public Element createXMLElement(Document doc, SscServiceData data) {
            int slotId = data.getSlotId();
            ImsLog.d(slotId, "");

            int state = SscConstant.STATUS_ENABLE;
            if (data.getState() == SscConstant.TIR_NOT_PROVISIONED) {
                if (SscConfig.isOirTirAlwaysTemporaryMode(slotId)) {
                    state = SscConstant.STATUS_ENABLE;
                } else {
                    state = SscConstant.STATUS_DISABLE;
                }
            }

            Element tirServiceElement = updateServiceElement(doc, slotId,
                    data.getSsType().getSSName(), state);
            if (tirServiceElement == null) {
                return null;
            }

            String defaultBehaviourValue = null;
            if (data.getState() == SscConstant.TIR_PROVISIONED) {
                defaultBehaviourValue = SscXmlFormat.PRESENTATION_RESTRICTED;
            } else {
                defaultBehaviourValue = SscXmlFormat.PRESENTATION_NOT_RESTRICTED;
            }

            String dbTag = SscXmlFormat.getSsElement(slotId, SscXmlFormat.DEFAULT_BEHAVIOUR);
            Element defaultBehaviour = getElementByTagName(tirServiceElement, dbTag);
            if (defaultBehaviour != null) {
                defaultBehaviour.setTextContent(defaultBehaviourValue);
            } else {
                defaultBehaviour = doc.createElement(dbTag);
                defaultBehaviour.setTextContent(defaultBehaviourValue);
                tirServiceElement.appendChild(defaultBehaviour);
            }

            return tirServiceElement;
        }
    }

    protected class XmlCreatorCf implements IXmlCreator {
        @Override
        public Element createXMLElement(Document doc, SscServiceData data) {
            if (data.getCondition() == SscConstant.CONDITION_CFNR_TIMER) {
                return updateNoReplyTimer(doc, data);
            }

            if (data.getState() == SscConstant.ACTION_ACTIVATION ||
                    data.getState() == SscConstant.ACTION_DEACTIVATION) {
                return updateXmlCfRuleCondition(doc, data);
            }

            return updateXmlCfRule(doc, data);
        }
/*
        private Element updateXmlCfService(Document doc, SscServiceData data) {
            int slotId = data.getSlotId();
            ImsLog.d(slotId, "");

            Element cfServiceElement = updateServiceElement(
                doc, slotId, data.getSsType().getSSName(), data.getState());
            if (cfServiceElement == null) {
                return null;
            }

            CfServiceUpdateData cfData = (CfServiceUpdateData) data;
            for (int i = SscConstant.CONDITION_CFB; i <= SscConstant.CONDITION_CFNRC; i++) {
                CfServiceUpdateData tempCfData = new CfServiceUpdateData(slotId, data.getSsType(),
                        0, 0, data.getState(), i, cfData.getForwardToNumber(), -1,
                        data.getServiceClass());
                updateXmlCfRule(doc, tempCfData);
            }

            if (data.getCondition() == SscConstant.CONDITION_CFA) {
                CfServiceUpdateData tempCfData = new CfServiceUpdateData(slotId, data.getSsType(),
                        0, 0, data.getState(), SscConstant.CONDITION_CFU,
                        cfData.getForwardToNumber(), -1, data.getServiceClass());
                updateXmlCfRule(doc, tempCfData);
            }

            if (SscXmlFormat.getIsCfnlProvisioned(slotId)) {
                CfServiceUpdateData tempCfData = new CfServiceUpdateData(slotId, data.getSsType(),
                        0, 0, data.getState(), SscConstant.CONDITION_CFNL,
                        cfData.getForwardToNumber(), -1, data.getServiceClass());
                updateXmlCfRule(doc, tempCfData);
            }

            if (cfData.getReplyTimer() > 0) {
                updateNoReplyTimer(doc, data);
            }

            return cfServiceElement;
        }
*/
        private Element updateXmlCfRule(Document doc, SscServiceData data) {
            int slotId = data.getSlotId();
            ImsLog.d(slotId, "");

            int mediaType = (data.getServiceClass() == SscServiceClassUtil.SERVICE_CLASS_DATA) ?
                    SscXmlFormat.MEDIA_VIDEO : SscXmlFormat.MEDIA_AUDIO;
            String ruleId = SscXmlFormat.getRuleId(
                    slotId, mediaType, data.getSsType().getSSName(), data.getCondition());

            int state = SscConstant.STATUS_DISABLE;
            if(data.getState() == SscConstant.ACTION_ACTIVATION ||
                    data.getState() == SscConstant.ACTION_REGISTRATION) {
                state = SscConstant.STATUS_ENABLE;
            }

            Element cfRuleElement = updateRuleElement(doc, slotId, ruleId, state);
            if (cfRuleElement == null) {
                return null;
            }

            // Set target
            CfServiceUpdateData cfData = (CfServiceUpdateData) data;
            if (data.getState() == SscConstant.ACTION_REGISTRATION ||
                    data.getState() == SscConstant.ACTION_ERASURE) {
                String targetTag = SscXmlFormat.getSsElement(slotId, SscXmlFormat.TARGET);
                Element targetELement = getElementByTagName(cfRuleElement, targetTag);
                if (targetELement != null) {
                    targetELement.setTextContent(
                            makeValueInTargetTo(slotId, cfData.getForwardToNumber()));
                }
            }

            // Set Timer only when CFNR timer is in rule
            if (cfData.getReplyTimer() > 0 && cfData.getCondition() == SscConstant.CONDITION_CFNR) {
                if (SscXmlFormat.getIsNoReplyTimerInRule(slotId)) {
                    updateNoReplyTimer(doc, data);
                }
            }

            return cfRuleElement;
        }

        private Element updateXmlCfRuleCondition(Document doc, SscServiceData data) {
            int slotId = data.getSlotId();
            ImsLog.d(slotId, "");

            Element cfRuleElement = updateXmlCfRule(doc, data);
            if (cfRuleElement == null) {
                return null;
            }

            return getElementByTagName(cfRuleElement, SscXmlFormat.getCpElement(slotId,
                    SscXmlFormat.CONDITIONS));
        }

        private Element updateNoReplyTimer(Document doc, SscServiceData data) {
            int slotId = data.getSlotId();
            ImsLog.d(slotId, "");

            Element rootElement = doc.getDocumentElement();
            if (rootElement == null) {
                return null;
            }

            String noReplyTimerTag = SscXmlFormat.getSsElement(slotId, SscXmlFormat.NOREPLYTIMER);
            Element noReplyTimerElement = getElementByTagName(rootElement, noReplyTimerTag);
            if (noReplyTimerElement == null) {
                noReplyTimerElement = doc.createElement(noReplyTimerTag);
                String serviceTag = SscXmlFormat.getSsElement(slotId, SscXmlFormat.CD);
                Element serviceElement = getElementByTagName(rootElement, serviceTag);
                if (serviceElement == null) {
                    ImsLog.d(serviceTag + " is null");
                    return null;
                }
                serviceElement.appendChild(noReplyTimerElement);
            }

            CfServiceUpdateData cfData = (CfServiceUpdateData) data;
            noReplyTimerElement.setTextContent(Integer.toString(cfData.getReplyTimer()));

            return noReplyTimerElement;
        }

        private String makeValueInTargetTo(int slotId, String number) {
            if (TextUtils.isEmpty(number)) {
                ImsLog.d("Number is empty !!!");
                return null;
            }

            String domain = null;
            String phoneContext = SscConfig.getPhoneContextForTargetAddress(slotId);
            if (!TextUtils.isEmpty(phoneContext)) {
                domain = phoneContext;
            } else {
                domain = SscUtils.getDomain(slotId);
            }

            if (domain == null) {
                ImsLog.w("Domain is null !!!");
                return null;
            }

            final String ZERO = "0";
            final String ccToAdd = SscConfig.getCountryCodeToReplaceZeroWithCountryCode(slotId);
            if (!TextUtils.isEmpty(ccToAdd) && !number.startsWith("+")) {
                if (number.startsWith(ZERO)) {
                    number = number.substring(1);
                }
                number = ccToAdd + number;
            }

            final String ccToRemove = SscConfig.getCountryCodeToReplaceCountryCodeWithZero(slotId);
            if (!TextUtils.isEmpty(ccToRemove) && number.startsWith(ccToRemove)) {
                number = number.replaceFirst(ccToRemove, ZERO);
            }

            final String format = SscConfig.getTargetAddrScheme(slotId);
            ImsLog.d("number : " + number + ", format : " + format + ", domain : " + domain);

            // IR92 2.2.3 Addressing
            String addressing = null;
            if ("sip".equalsIgnoreCase(format)) {
                addressing = "sip:" + number;
                // local numbering
                if (!number.startsWith("+")) {
                    addressing += ";phone-context=" + domain;
                }
                addressing += "@" + domain + ";user=phone";
            } else if ("tel".equalsIgnoreCase(format)) {
                addressing = "tel:" + number;
                // local numbering
                if (!number.startsWith("+")) {
                    addressing += ";phone-context=" + domain;
                }
            } else {
                addressing = number;
            }

            return addressing;
        }
    }

    protected class XmlCreatorCb implements IXmlCreator {
        @Override
        public Element createXMLElement(Document doc, SscServiceData data) {
            int slotId = data.getSlotId();
            ImsLog.d(slotId, "");

            int mediaType = (data.getServiceClass() == SscServiceClassUtil.SERVICE_CLASS_DATA) ?
                    SscXmlFormat.MEDIA_VIDEO : SscXmlFormat.MEDIA_AUDIO;
            String ruleId = SscXmlFormat.getRuleId(
                    slotId, mediaType, data.getSsType().getSSName(), data.getCondition());

            return updateRuleElement(doc, slotId, ruleId, data.getState());
        }
    }

    protected class XmlCreatorCw implements IXmlCreator {
        @Override
        public Element createXMLElement(Document doc, SscServiceData data) {
            int slotId = data.getSlotId();
            ImsLog.d(slotId, "");
            return updateServiceElement(doc, slotId, data.getSsType().getSSName(), data.getState());
        }
    }
/*
    protected class XMLCreatorICBA implements IXmlCreator {
        @Override
        public Document createXMLElement(Document doc, SscServiceData data) {
            ImsLog.d("");

            if (!(inputData instanceof ICBAServiceData)) {
                ImsLog.e("data type is not valid");
                return null;
            }

            ICBAServiceData data = (ICBAServiceData)inputData;

            Document doc = null;
            Element ruleSet = null;

            try {
                DocumentBuilderFactory docFactory = DocumentBuilderFactory.newInstance();
                DocumentBuilder docBuilder = docFactory.newDocumentBuilder();

                doc = docBuilder.newDocument();
                doc.setXmlStandalone(true);

                int enableValue = data.getState();

                if (enableValue == -1) {
                    ImsLog.d("enableValue is \" -1 \" : ");
                    return doc;
                }

                Element enable = doc.createElement(
                        SscXmlFormat.getXMLSS(data.getSlotId()) +
                        "incoming-communication-barring");
                enable.setAttribute(SscXmlFormat.ACTIVE, ((enableValue == 1) ? "true" : "false"));
                doc.appendChild(enable);

                ruleSet = doc.createElement(SscXmlFormat.getRuleSet(data.getSlotId()));
                enable.appendChild(ruleSet);

                //anonymous element
                for (SscRuleDataICB ruleData : data.getRuleSet()) {
                    Element rule = doc.createElement(SscXmlFormat.getRule(data.getSlotId()));
                    ruleSet.appendChild(rule);

                    Attr ruleid = doc.createAttribute(SscXmlFormat.ID);
                    ruleid.setValue(ruleData.getRuleId());
                    rule.setAttributeNode(ruleid);

                    // condition
                    Element conditions = doc.createElement(
                            SscXmlFormat.getConditions(data.getSlotId()));
                    rule.appendChild(conditions);

                    for (SscRuleElement element : ruleData.getConditionList()) {
                        if (ruleData.getRuleState() == 0) {
                            Element ruleDeactivated = doc.createElement(
                                    SscXmlFormat.getRuleDeactivated(data.getSlotId()));
                            conditions.appendChild(ruleDeactivated);
                        }

                        if (ruleData.getICBType() == SscRuleDataICB.TYPE_ANONYMOUS) {
                            //ss:anonymous
                            Element condElement = doc.createElement(
                                    SscXmlFormat.getXMLSS(data.getSlotId()) + element.getKey());
                            ImsLog.d("condElement : " + element.getKey());
                            conditions.appendChild(condElement);
                        }
                        else {
                            //cp:identity
                            Element condElement = doc.createElement(
                                    SscXmlFormat.getXMLCP(data.getSlotId()) + element.getKey());
                            ImsLog.d("condElement : " + element.getKey());
                            conditions.appendChild(condElement);

                            // one id
                            Element one = doc.createElement(
                                    SscXmlFormat.getXMLCP(data.getSlotId()) + SscXmlFormat.ONE);
                            condElement.appendChild(one);
                            Attr oneid = doc.createAttribute(SscXmlFormat.ID);
                            oneid.setValue(ruleData.getOneId());
                            one.setAttributeNode(oneid);
                        }
                    }

                    // action
                    Element actions = doc.createElement(SscXmlFormat.getActions(data.getSlotId()));
                    rule.appendChild(actions);

                    for (SscRuleElement element : ruleData.getActionList()) {
                        Element actionElement = doc.createElement(
                                SscXmlFormat.getXMLSS(data.getSlotId()) + element.getKey());
                        ImsLog.d("actionElement : " + element.getKey());

                        actionElement.appendChild(doc.createTextNode(element.getValue()));
                        ImsLog.d("actionValue: " + element.getValue());
                        actions.appendChild(actionElement);
                    }
                }
            }
            catch (ParserConfigurationException pce) {
                ImsLog.d(pce.toString());
                pce.printStackTrace();
            }

            return doc;
        }
    }
 */
}
