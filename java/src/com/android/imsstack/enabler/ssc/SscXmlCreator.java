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
import com.android.internal.annotations.VisibleForTesting;

import org.w3c.dom.Attr;
import org.w3c.dom.Document;
import org.w3c.dom.Element;
import org.w3c.dom.NodeList;

import java.util.LinkedHashMap;

public class SscXmlCreator {
    private final LinkedHashMap<ESsType, IXmlCreator> mXmlCreatorTable;

    public SscXmlCreator() {
        ImsLog.d("");

        mXmlCreatorTable = new LinkedHashMap<ESsType, IXmlCreator>();
        mXmlCreatorTable.put(ESsType.OIP, new XmlCreatorOip());
        mXmlCreatorTable.put(ESsType.OIR, new XmlCreatorOir());
        mXmlCreatorTable.put(ESsType.TIP, new XmlCreatorTip());
        mXmlCreatorTable.put(ESsType.TIR, new XmlCreatorTir());
        mXmlCreatorTable.put(ESsType.CF, new XmlCreatorCf());
        mXmlCreatorTable.put(ESsType.OCB, new XmlCreatorCb());
        mXmlCreatorTable.put(ESsType.ICB, new XmlCreatorCb());
        mXmlCreatorTable.put(ESsType.CW, new XmlCreatorCw());
    }

    protected Element createXml(Document doc, SscServiceData data) {
        if (doc == null) {
            ImsLog.e("doc is null");
            return null;
        }

        if (data == null) {
            ImsLog.e("input value is null");
            return null;
        }

        IXmlCreator xmlCreator = mXmlCreatorTable.get(data.getSsType());
        if (xmlCreator == null) {
            ImsLog.e("XML creator is null");
            return null;
        }

        return xmlCreator.createXmlElement(doc, data);
    }

    private Element getElementByTagName(Element rootElement, String tagName) {
        NodeList elementList = rootElement.getElementsByTagName(tagName);
        if (elementList.getLength() == 0) {
            return null;
        }

        return (Element) elementList.item(0);
    }

    private Element updateServiceElement(Document doc, int slotId, String serviceName, int state) {
        Element rootElement = doc.getDocumentElement();
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
        Element ruleElement = doc.getElementById(ruleId);
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

    /**
     * Createing new rule element and ruleset element. This method must be used only when the
     * xcap server doesn't have same rule in XML data.
     */
    private Element createRuleAndRuleSet(Document doc, SscServiceData data) {
        int slotId = data.getSlotId();
        ImsLog.d(slotId, "");

        String serviceName = data.getSsType().getSsName();
        Element serviceElement = updateServiceElement(doc, slotId, serviceName,
                SscConstant.STATUS_ENABLE);
        if (serviceElement == null) {
            return null;
        }

        String ruleSetTag = SscXmlFormat.getCpElement(slotId, SscXmlFormat.RULESET);
        Element ruleSetElement = getElementByTagName(serviceElement, ruleSetTag);
        if (ruleSetElement == null) {
            ruleSetElement = doc.createElement(ruleSetTag);
            serviceElement.appendChild(ruleSetElement);
        }

        String ruleTag = SscXmlFormat.getCpElement(slotId, SscXmlFormat.RULE);
        Element ruleElement = doc.createElement(ruleTag);
        ruleSetElement.appendChild(ruleElement);

        int mediaType = (data.getServiceClass() == SscServiceClassUtil.SERVICE_CLASS_VIDEO)
                ? SscXmlFormat.MEDIA_TYPE_VIDEO : SscXmlFormat.MEDIA_TYPE_AUDIO;
        String defaultRuleId = SscXmlFormat.getDefaultRuleId(slotId, mediaType, serviceName,
                data.getCondition());
        if (TextUtils.isEmpty(defaultRuleId)) {
            return null;
        }

        Attr attrRuleId = doc.createAttribute(SscXmlFormat.ID);
        attrRuleId.setValue(defaultRuleId);
        ruleElement.setAttributeNode(attrRuleId);

        String conditionTag = SscXmlFormat.getCpElement(slotId, SscXmlFormat.CONDITIONS);
        Element conditionElement = doc.createElement(conditionTag);
        ruleElement.appendChild(conditionElement);

        String ruleConditionTag = SscXmlFormat.getRuleConditionTag(slotId, serviceName,
                data.getCondition());
        if (!TextUtils.isEmpty(ruleConditionTag)) {
            Element ruleConditionElement = doc.createElement(ruleConditionTag);
            conditionElement.appendChild(ruleConditionElement);
        }

        String serviceCapabilityName = SscXmlFormat.SC_CD;
        if (SscXmlFormat.ICB.equals(serviceName) || SscXmlFormat.OCB.equals(serviceName)) {
            serviceCapabilityName = SscXmlFormat.SC_CB;
        }

        if (mediaType == SscXmlFormat.MEDIA_TYPE_AUDIO) {
            boolean audioCapability = SscXmlFormat.getMediaCapability(slotId, serviceCapabilityName,
                    SscXmlFormat.MEDIA_TYPE_AUDIO);
            if (audioCapability) {
                String mediaTag = SscXmlFormat.getSsElement(slotId, SscXmlFormat.MEDIA);
                Element mediaElement = doc.createElement(mediaTag);
                mediaElement.setTextContent(SscXmlFormat.AUDIO);
                conditionElement.appendChild(mediaElement);
            }
        } else if (mediaType == SscXmlFormat.MEDIA_TYPE_VIDEO) {
            boolean videoCapability = SscXmlFormat.getMediaCapability(slotId, serviceCapabilityName,
                    SscXmlFormat.MEDIA_TYPE_VIDEO);
            if (videoCapability) {
                String mediaTag = SscXmlFormat.getSsElement(slotId, SscXmlFormat.MEDIA);
                Element mediaElement = doc.createElement(mediaTag);
                mediaElement.setTextContent(SscXmlFormat.VIDEO);
                conditionElement.appendChild(mediaElement);
            }
        }

        return serviceElement;
    }

    protected interface IXmlCreator {
        Element createXmlElement(Document doc, SscServiceData data);
    }

    private class XmlCreatorOip implements IXmlCreator {
        @Override
        public Element createXmlElement(Document doc, SscServiceData data) {
            int slotId = data.getSlotId();
            ImsLog.d(slotId, "");
            return updateServiceElement(doc, slotId, data.getSsType().getSsName(), data.getState());
        }
    }

    private class XmlCreatorOir implements IXmlCreator {
        @Override
        public Element createXmlElement(Document doc, SscServiceData data) {
            int slotId = data.getSlotId();
            ImsLog.d(slotId, "");

            int state = SscConstant.STATUS_DISABLE;
            int operation = data.getState();
            if (operation == SscConstant.OIR_DEFAULT) {
                int defaultOperation = SscConfig.getOirNetworkDefaultOperation(slotId);
                if (defaultOperation == SscConfig.OIR_NOT_PROVISIONED) {
                    operation = SscConstant.OIR_SUPPRESSION;
                } else if (defaultOperation == SscConfig.OIR_TEMP_MODE_RESTRICTED) {
                    state = SscConstant.STATUS_ENABLE;
                    operation = SscConstant.OIR_INVOCATION;
                } else if (defaultOperation == SscConfig.OIR_TEMP_MODE_ALLOWED) {
                    state = SscConstant.STATUS_ENABLE;
                    operation = SscConstant.OIR_SUPPRESSION;
                } else if (defaultOperation == SscConfig.OIR_TEMP_MODE_WITHOUT_DEFAULT_BEHAVIOUR) {
                    state = SscConstant.STATUS_ENABLE;
                }
            } else if (operation == SscConstant.OIR_INVOCATION) {
                state = SscConstant.STATUS_ENABLE;
            } else if (operation == SscConstant.OIR_SUPPRESSION) {
                if (SscConfig.isOirTirAlwaysTemporaryMode(slotId)) {
                    state = SscConstant.STATUS_ENABLE;
                } else {
                    state = SscConstant.STATUS_DISABLE;
                }
            }

            Element oirServiceElement =
                    updateServiceElement(doc, slotId, data.getSsType().getSsName(), state);
            if (oirServiceElement == null) {
                return null;
            }

            String defaultBehaviour = null;
            if (operation == SscConstant.OIR_INVOCATION) {
                defaultBehaviour = SscXmlFormat.PRESENTATION_RESTRICTED;
            } else if (operation == SscConstant.OIR_SUPPRESSION) {
                defaultBehaviour = SscXmlFormat.PRESENTATION_NOT_RESTRICTED;
            }

            String dbTag = SscXmlFormat.getSsElement(slotId, SscXmlFormat.DEFAULT_BEHAVIOUR);
            Element defaultBehaviourElement = getElementByTagName(oirServiceElement, dbTag);
            if (defaultBehaviourElement != null) {
                if (defaultBehaviour == null) {
                    defaultBehaviourElement.getParentNode().removeChild(defaultBehaviourElement);
                } else {
                    defaultBehaviourElement.setTextContent(defaultBehaviour);
                }
            } else {
                if (defaultBehaviour != null) {
                    defaultBehaviourElement = doc.createElement(dbTag);
                    defaultBehaviourElement.setTextContent(defaultBehaviour);
                    oirServiceElement.appendChild(defaultBehaviourElement);
                }
            }

            return oirServiceElement;
        }
    }

    private class XmlCreatorTip implements IXmlCreator {
        @Override
        public Element createXmlElement(Document doc, SscServiceData data) {
            int slotId = data.getSlotId();
            ImsLog.d(slotId, "");
            return updateServiceElement(doc, slotId, data.getSsType().getSsName(), data.getState());
        }
    }

    private class XmlCreatorTir implements IXmlCreator {
        @Override
        public Element createXmlElement(Document doc, SscServiceData data) {
            int slotId = data.getSlotId();
            ImsLog.d(slotId, "");

            int state = SscConstant.STATUS_DISABLE;
            if (data.getState() == SscConstant.TIR_NOT_PROVISIONED) {
                if (SscConfig.isOirTirAlwaysTemporaryMode(slotId)) {
                    state = SscConstant.STATUS_ENABLE;
                } else {
                    state = SscConstant.STATUS_DISABLE;
                }
            } else if (data.getState() == SscConstant.TIR_PROVISIONED) {
                state = SscConstant.STATUS_ENABLE;
            }

            Element tirServiceElement = updateServiceElement(doc, slotId,
                    data.getSsType().getSsName(), state);
            if (tirServiceElement == null) {
                return null;
            }

            String defaultBehaviour = null;
            if (data.getState() == SscConstant.TIR_PROVISIONED) {
                defaultBehaviour = SscXmlFormat.PRESENTATION_RESTRICTED;
            } else {
                defaultBehaviour = SscXmlFormat.PRESENTATION_NOT_RESTRICTED;
            }

            String dbTag = SscXmlFormat.getSsElement(slotId, SscXmlFormat.DEFAULT_BEHAVIOUR);
            Element defaultBehaviourElement = getElementByTagName(tirServiceElement, dbTag);
            if (defaultBehaviourElement != null) {
                defaultBehaviourElement.setTextContent(defaultBehaviour);
            } else {
                defaultBehaviourElement = doc.createElement(dbTag);
                defaultBehaviourElement.setTextContent(defaultBehaviour);
                tirServiceElement.appendChild(defaultBehaviourElement);
            }

            return tirServiceElement;
        }
    }

    private class XmlCreatorCf implements IXmlCreator {
        @Override
        public Element createXmlElement(Document doc, SscServiceData data) {
            if (data.getEventNumber() == SscConstant.EVENT_SSC_INSERT_CF) {
                return createCfRuleAndRuleSet(doc, data);
            }

            if (data.getCondition() == SscConstant.CONDITION_CFNR_TIMER) {
                if (SscXmlFormat.getIsNoReplyTimerOmitted(data.getSlotId())) {
                    return createNoReplyTimer(doc, data);
                }

                return updateNoReplyTimer(doc, data);
            }

            if (data.getState() == SscConstant.ACTION_ACTIVATION ||
                    data.getState() == SscConstant.ACTION_DEACTIVATION) {
                return updateCfCondition(doc, data);
            }

            return updateCfRule(doc, data);
        }

        private Element updateCfRule(Document doc, SscServiceData data) {
            int slotId = data.getSlotId();
            ImsLog.d(slotId, "");

            int mediaType = (data.getServiceClass() == SscServiceClassUtil.SERVICE_CLASS_VIDEO)
                    ? SscXmlFormat.MEDIA_TYPE_VIDEO : SscXmlFormat.MEDIA_TYPE_AUDIO;
            String ruleId = null;
            if (data.getEventNumber() == SscConstant.EVENT_SSC_INSERT_CF) {
                ruleId = SscXmlFormat.getDefaultRuleId(slotId, mediaType,
                        data.getSsType().getSsName(), data.getCondition());
            } else {
                ruleId = SscXmlFormat.getRuleId(slotId, mediaType, data.getSsType().getSsName(),
                        data.getCondition());
            }

            if (ruleId == null) {
                return null;
            }

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
            if (data.getState() == SscConstant.ACTION_REGISTRATION
                    || data.getState() == SscConstant.ACTION_ERASURE) {
                String targetTag = SscXmlFormat.getSsElement(slotId, SscXmlFormat.TARGET);
                Element targetElement = getElementByTagName(cfRuleElement, targetTag);
                if (targetElement == null) {
                    targetElement = doc.createElement(targetTag);
                    String forwardToTag =
                            SscXmlFormat.getSsElement(slotId, SscXmlFormat.FORWARD_TO);
                    Element forwardToElement = getElementByTagName(cfRuleElement, forwardToTag);
                    if (forwardToElement == null) {
                        forwardToElement = doc.createElement(forwardToTag);
                        String actionsTag = SscXmlFormat.getCpElement(slotId, SscXmlFormat.ACTIONS);
                        Element actionsElement = getElementByTagName(cfRuleElement, actionsTag);
                        if (actionsElement == null) {
                            actionsElement = doc.createElement(actionsTag);
                            cfRuleElement.appendChild(actionsElement);
                        }
                        actionsElement.appendChild(forwardToElement);

                    }
                    forwardToElement.appendChild(targetElement);
                }
                targetElement.setTextContent(getSscUtils().getUriFromNumber(slotId,
                        cfData.getForwardToNumber()));
            }

            // Set Timer only when CFNR timer is in rule.
            if (cfData.getReplyTimer() > 0 && cfData.getCondition() == SscConstant.CONDITION_CFNR) {
                if (SscXmlFormat.getIsNoReplyTimerInRule(slotId)) {
                    if (SscXmlFormat.getIsNoReplyTimerOmitted(slotId)) {
                        String noReplyTimerTag = SscXmlFormat.getSsElement(slotId,
                                SscXmlFormat.NOREPLYTIMER);
                        if (getElementByTagName(cfRuleElement, noReplyTimerTag) != null) {
                            ImsLog.d(slotId,
                                    "noReplyTimer is already inserted. Don't create it again");
                            SscXmlFormat.setIsNoReplyTimerOmitted(slotId, false);
                        } else {
                            String actionsTag = SscXmlFormat.getCpElement(slotId,
                                    SscXmlFormat.ACTIONS);
                            Element actionsElement = getElementByTagName(cfRuleElement, actionsTag);
                            if (actionsElement != null) {
                                Element noReplyTimerElement = doc.createElement(noReplyTimerTag);
                                actionsElement.appendChild(noReplyTimerElement);
                            }
                        }
                    }

                    updateNoReplyTimer(doc, data);
                }
            }

            return cfRuleElement;
        }

        private Element updateCfCondition(Document doc, SscServiceData data) {
            int slotId = data.getSlotId();
            ImsLog.d(slotId, "");

            Element cfRuleElement = updateCfRule(doc, data);
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
            String noReplyTimerTag = SscXmlFormat.getSsElement(slotId, SscXmlFormat.NOREPLYTIMER);
            Element noReplyTimerElement = getElementByTagName(rootElement, noReplyTimerTag);
            if (noReplyTimerElement == null) {
                ImsLog.d(noReplyTimerTag + " is null");
                return null;
            }

            CfServiceUpdateData cfData = (CfServiceUpdateData) data;
            noReplyTimerElement.setTextContent(Integer.toString(cfData.getReplyTimer()));

            return noReplyTimerElement;
        }

        private Element createCfRuleAndRuleSet(Document doc, SscServiceData data) {
            int slotId = data.getSlotId();
            ImsLog.d(slotId, "");

            boolean provisionStatus = SscXmlFormat.getProvisionStatus(slotId, SscXmlFormat.SC_CD,
                    data.getCondition());
            if (!provisionStatus) {
                ImsLog.d(slotId, "Not provisioned condition " + data.getCondition());
                return null;
            }

            Element cfServiceElement = createRuleAndRuleSet(doc, data);
            if (cfServiceElement == null) {
                return null;
            }

            Element cfRuleElement = updateCfRule(doc, data);
            if (cfRuleElement == null) {
                return null;
            }

            return cfServiceElement;
        }

        private Element createNoReplyTimer(Document doc, SscServiceData data) {
            int slotId = data.getSlotId();
            ImsLog.d(slotId, "");

            Element rootElement = doc.getDocumentElement();
            String serviceTag = SscXmlFormat.getSsElement(slotId, SscXmlFormat.CD);
            Element cfServiceElement = getElementByTagName(rootElement, serviceTag);
            if (cfServiceElement == null) {
                ImsLog.d(serviceTag + " is null");
                return null;
            }

            String noReplyTimerTag = SscXmlFormat.getSsElement(slotId, SscXmlFormat.NOREPLYTIMER);
            if (getElementByTagName(rootElement, noReplyTimerTag) != null) {
                ImsLog.d("noReplyTimer is already inserted. Don't create it again");
                SscXmlFormat.setIsNoReplyTimerOmitted(slotId, false);
                return updateNoReplyTimer(doc, data);
            }

            Element noReplyTimerElement = doc.createElement(noReplyTimerTag);
            cfServiceElement.insertBefore(noReplyTimerElement, cfServiceElement.getFirstChild());

            CfServiceUpdateData cfData = (CfServiceUpdateData) data;
            noReplyTimerElement.setTextContent(Integer.toString(cfData.getReplyTimer()));

            return cfServiceElement;
        }
    }

    private class XmlCreatorCb implements IXmlCreator {
        @Override
        public Element createXmlElement(Document doc, SscServiceData data) {
            if (data.getEventNumber() == SscConstant.EVENT_SSC_INSERT_CB) {
                return createCbRuleAndRuleSet(doc, data);
            }

            return updateCbRule(doc, data);
        }

        private Element updateCbRule(Document doc, SscServiceData data) {
            int slotId = data.getSlotId();
            ImsLog.d(slotId, "");
            int mediaType = (data.getServiceClass() == SscServiceClassUtil.SERVICE_CLASS_VIDEO)
                    ? SscXmlFormat.MEDIA_TYPE_VIDEO : SscXmlFormat.MEDIA_TYPE_AUDIO;
            String ruleId = null;
            if (data.getEventNumber() == SscConstant.EVENT_SSC_INSERT_CB) {
                ruleId = SscXmlFormat.getDefaultRuleId(slotId, mediaType,
                        data.getSsType().getSsName(), data.getCondition());
            } else {
                ruleId = SscXmlFormat.getRuleId(slotId, mediaType, data.getSsType().getSsName(),
                        data.getCondition());
            }

            if (ruleId == null) {
                return null;
            }

            Element cbRuleElement = updateRuleElement(doc, slotId, ruleId, data.getState());
            if (cbRuleElement == null) {
                return null;
            }

            String allowTag = SscXmlFormat.getSsElement(slotId, SscXmlFormat.ALLOW);
            Element allowElement = getElementByTagName(cbRuleElement, allowTag);
            if (allowElement == null) {
                allowElement = doc.createElement(allowTag);
                String actionsTag = SscXmlFormat.getCpElement(slotId, SscXmlFormat.ACTIONS);
                Element actionsElement = getElementByTagName(cbRuleElement, actionsTag);
                if (actionsElement == null) {
                    actionsElement = doc.createElement(actionsTag);
                    cbRuleElement.appendChild(actionsElement);
                }
                actionsElement.appendChild(allowElement);
            }
            allowElement.setTextContent("false");

            return cbRuleElement;
        }

        private Element createCbRuleAndRuleSet(Document doc, SscServiceData data) {
            int slotId = data.getSlotId();
            ImsLog.d(slotId, "");

            boolean provisionStatus = SscXmlFormat.getProvisionStatus(slotId, SscXmlFormat.SC_CB,
                    data.getCondition());
            if (!provisionStatus) {
                ImsLog.d(slotId, "Not provisioned condition " + data.getCondition());
                return null;
            }

            Element cbServiceElement = createRuleAndRuleSet(doc, data);
            if (cbServiceElement == null) {
                return null;
            }

            Element cbRuleElement = updateCbRule(doc, data);
            if (cbRuleElement == null) {
                return null;
            }

            return cbServiceElement;
        }
    }

    private class XmlCreatorCw implements IXmlCreator {
        @Override
        public Element createXmlElement(Document doc, SscServiceData data) {
            int slotId = data.getSlotId();
            ImsLog.d(slotId, "");
            return updateServiceElement(doc, slotId, data.getSsType().getSsName(), data.getState());
        }
    }

    @VisibleForTesting
    protected SscUtils getSscUtils() {
        return SscUtils.getInstance();
    }
}
