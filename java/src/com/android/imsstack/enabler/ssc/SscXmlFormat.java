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

import com.android.imsstack.util.ImsLog;

import java.util.HashMap;

public class SscXmlFormat {
    private static class UtXmlData {
        private boolean mIsNoReplyTimerOmitted = false;
        private boolean mIsNoReplyTimerInRule = false;
        private boolean mIsCfnlExistInServer = false;
        private String mNsSsPrefix = "";
        private String mNsCpPrefix = "";
        // namespace, tag name
        private HashMap<String, String> mTags = new HashMap<>();
        // service name, condition, rule ID for audio
        private HashMap<String, HashMap<Integer, String>> mAudioRuleIds = new HashMap<>();
        // service name, condition, rule ID for video
        private HashMap<String, HashMap<Integer, String>> mVideoRuleIds = new HashMap<>();

        /**
         * mCfProvisionStatus and mCbProvisionStatus are used to check whether the conditioins are
         * provisioned by XCAP server only when creating ruleset and rule element to support IR.92
         * v9.0. If not provisioned, UE will not create rule element for condition.
         * The params consist of condition and provisioned status
         */
        private HashMap<Integer, Boolean> mCfProvisionStatus = new HashMap<>();
        private HashMap<Integer, Boolean> mCbProvisionStatus = new HashMap<>();

        /**
         * mCfMeidaCapability and mCbMeidaCapability are used to check whether the media
         * capabilities are supported by XCAP server only when creating ruleset and rule
         * element to support IR.92 v9.0. If not supported, UE will not create media element for
         * media type.
         * The params consist of media type and status
         */
        private HashMap<Integer, Boolean> mCfMeidaCapability = new HashMap<>();
        private HashMap<Integer, Boolean> mCbMeidaCapability = new HashMap<>();

        private UtXmlData() {
            mCfProvisionStatus.put(SscConstant.CONDITION_CFU, true);
            mCfProvisionStatus.put(SscConstant.CONDITION_CFB, true);
            mCfProvisionStatus.put(SscConstant.CONDITION_CFNR, true);
            mCfProvisionStatus.put(SscConstant.CONDITION_CFNRC, true);
            mCfProvisionStatus.put(SscConstant.CONDITION_CFNL, true);

            mCbProvisionStatus.put(SscConstant.CONDITION_BAIC, true);
            mCbProvisionStatus.put(SscConstant.CONDITION_BAOC, true);
            mCbProvisionStatus.put(SscConstant.CONDITION_BOIC, true);
            mCbProvisionStatus.put(SscConstant.CONDITION_BOIC_EXHC, true);
            mCbProvisionStatus.put(SscConstant.CONDITION_BIC_WR, true);
            mCbProvisionStatus.put(SscConstant.CONDITION_ACR, true);

            mCfMeidaCapability.put(MEDIA_TYPE_AUDIO, false);
            mCfMeidaCapability.put(MEDIA_TYPE_VIDEO, false);

            mCbMeidaCapability.put(MEDIA_TYPE_AUDIO, false);
            mCbMeidaCapability.put(MEDIA_TYPE_VIDEO, false);
        }

        private void setIsNoReplyTimerOmitted(boolean isNoReplyTimerOmitted) {
            mIsNoReplyTimerOmitted = isNoReplyTimerOmitted;
        }

        private void setIsNoReplyTimerInRule(boolean isNoReplyTimerInRule) {
            mIsNoReplyTimerInRule = isNoReplyTimerInRule;
        }

        private void setIsCfnlExist(boolean isCfnlExistInServer) {
            mIsCfnlExistInServer = isCfnlExistInServer;
        }

        private boolean getIsNoReplyTimerOmitted() {
            return mIsNoReplyTimerOmitted;
        }

        private boolean getIsNoReplyTimerInRule() {
            return mIsNoReplyTimerInRule;
        }

        private boolean getIsCfnlExist() {
            return mIsCfnlExistInServer;
        }

        private HashMap<String, String> getTags() {
            return mTags;
        }

        private HashMap<String, HashMap<Integer, String>> getAudioRuleIds() {
            return mAudioRuleIds;
        }

        private HashMap<String, HashMap<Integer, String>> getVideoRuleIds() {
            return mVideoRuleIds;
        }
    };

    private static HashMap<Integer, UtXmlData> mXmlDatas = new HashMap<>();

    public static final String NS_SS_PREFIX = "ss:";
    public static final String NS_CP_PREFIX = "cp:";
    public static final String NS_XE_PREFIX = "xe:";
    public static final String NS_SS_URI
            = "xmlns(ss=http://uri.etsi.org/ngn/params/xml/simservs/xcap)";
    public static final String NS_CP_URI = "xmlns(cp=urn:ietf:params:xml:ns:common-policy)";

    // SS Service Elements
    public static final String SIMSERVS = "simservs";
    public static final String CW = "communication-waiting";
    public static final String CD = "communication-diversion";
    public static final String ICB = "incoming-communication-barring";
    public static final String OCB = "outgoing-communication-barring";
    public static final String OIP = "originating-identity-presentation";
    public static final String OIR = "originating-identity-presentation-restriction";
    public static final String TIP = "terminating-identity-presentation";
    public static final String TIR = "terminating-identity-presentation-restriction";

    // SS Service Capability Elements
    public static final String SC_CD = "communication-diversion-serv-cap";
    public static final String SC_CB = "communication-barring-serv-cap";

    // SS CF Condition Elements
    public static final String CFB = "busy";
    public static final String CFNR = "no-answer";
    public static final String CFNRC = "not-reachable";
    public static final String CFNL = "not-registered";

    // SS CF Service Capability Condition Elements
    public static final String SC_CFU = "serv-cap-unconditional";
    public static final String SC_CFB = "serv-cap-busy";
    public static final String SC_CFNR = "serv-cap-no-answer";
    public static final String SC_CFNRC = "serv-cap-not-reachable";
    public static final String SC_CFNL = "serv-cap-not-registered";

    // SS CB Condition Elements
    public static final String BOIC = "international";
    public static final String BOIC_EXHC = "international-exHC";
    public static final String BIC_WR = "roaming";
    public static final String ACR = "anonymous";

    // SS CB Service Capability Condition Elements
    public static final String SC_CBU = "serv-cap-unconditional";
    public static final String SC_BOIC = "serv-cap-international";
    public static final String SC_BOIC_EXHC = "serv-cap-international-exHC";
    public static final String SC_BIC_WR = "serv-cap-roaming";
    public static final String SC_ACR = "serv-cap-anonymous";

    // SS Elements
    public static final String NOREPLYTIMER = "NoReplyTimer";
    public static final String FORWARD_TO = "forward-to";
    public static final String TARGET = "target";
    public static final String DEFAULT_BEHAVIOUR = "default-behaviour";
    public static final String RULE_DEACTIVATED = "rule-deactivated";
    public static final String MEDIA = "media";
    public static final String ALLOW = "allow";

    // SS Actions
    public static final String NOTIFY_CALLER = "notify-caller";
    public static final String NOTIFY_SERVED_USER = "notify-served-user";
    public static final String NOTIFY_SERVED_USER_ON_OUTBOUND_CALL
            = "notify-served-user-on-outbound-call";
    public static final String REVEAL_SERVED_USER_IDENTITY_TO_CALLER
            = "reveal-served-user-identity-to-caller";
    public static final String REVEAL_IDENTITY_TO_USER = "reveal-identity-to-caller";
    public static final String REVEAL_IDENTITY_TO_TARGET = "reveal-identity-to-target";

    // SS Service Capability Elements
    public static final String SC_CONDITIONS = "serv-cap-conditions";
    public static final String SC_MEDIA = "serv-cap-media";

    // CP Elements
    public static final String RULESET = "ruleset";
    public static final String RULE = "rule";
    public static final String CONDITIONS = "conditions";
    public static final String ACTIONS = "actions";
    public static final String ONE = "one";
    public static final String IDENTITY = "identity";

    // XE Elements
    public static final String XCAPERROR = "xcap-error";

    // Text Contents
    public static final String AUDIO = "audio";
    public static final String VIDEO = "video";
    public static final String PRESENTATION_RESTRICTED = "presentation-restricted";
    public static final String PRESENTATION_NOT_RESTRICTED = "presentation-not-restricted";
    public static final String PERMANENT_MODE = "permanent";
    public static final String TEMPORARY_MODE = "temporary";

    // Attributes
    public static final String ID = "id";
    public static final String ACTIVE = "active";
    public static final String PROVISIONED = "provisioned";
    public static final String PHRASE = "phrase";

    // CF default rule ids
    public static final String RULE_ID_CFU = "call-diversion-unconditional";
    public static final String RULE_ID_CFB = "call-diversion-busy";
    public static final String RULE_ID_CFNR = "call-diversion-no-reply";
    public static final String RULE_ID_CFNRC = "call-diversion-not-reachable";
    public static final String RULE_ID_CFNL = "call-diversion-not-logged-in";

    // CB default rule ids
    public static final String RULE_ID_BAIC = "call-barring-all-incoming";
    public static final String RULE_ID_BAOC = "call-barring-all-outgoing";
    public static final String RULE_ID_BOIC = "call-barring-outgoing-international";
    public static final String RULE_ID_BOIC_EXHC = "call-barring-outgoing-internationalExHC";
    public static final String RULE_ID_BIC_WR = "call-barring-incoming-in-roaming";
    public static final String RULE_ID_ACR = "call-barring-anonymous-incoming";

    public static final int MEDIA_TYPE_AUDIO = 0;
    public static final int MEDIA_TYPE_VIDEO = 1;

    public static void init(int slotId) {
        ImsLog.d("init (" + slotId + ")");

        setUtXmlData(slotId, new UtXmlData());
    }

    private static void setUtXmlData(int slotId, UtXmlData data) {
        mXmlDatas.remove(slotId);
        mXmlDatas.put(slotId, data);
    }

    public static void reset(int slotId) {
        UtXmlData xmlData  = getXmlData(slotId);
        if (xmlData != null) {
            xmlData.getTags().clear();
            xmlData.getAudioRuleIds().clear();
            xmlData.getVideoRuleIds().clear();
        }
    }

    private static UtXmlData getXmlData(int slotId) {
        if (!mXmlDatas.containsKey(slotId)) {
            ImsLog.w("wrong slot - " + slotId );
            return null;
        }

        return mXmlDatas.get(slotId);
    }

    private static String getTag(UtXmlData xmlData, String elementName) {
        HashMap<String, String> tags = xmlData.getTags();
        if (!tags.containsKey(elementName)) {
            return null;
        }

        String namespace = tags.get(elementName);
        if (TextUtils.isEmpty(namespace)) {
            return elementName;
        }

        return namespace + ":" + elementName;
    }

    protected static void setTag(int slotId, String tagName, String namespace) {
        UtXmlData xmlData = getXmlData(slotId);
        if (xmlData == null) {
            return;
        }

        if (NS_SS_PREFIX.equals(namespace + ":")) {
            xmlData.mNsSsPrefix = NS_SS_PREFIX;
        }

        if (NS_CP_PREFIX.equals(namespace + ":")) {
            xmlData.mNsCpPrefix = NS_CP_PREFIX;
        }

        xmlData.getTags().put(tagName, namespace);
    }

    protected static String getSsElement(int slotId, String elementName) {
        if (SscConfig.isOmitNamespaceSs(slotId)) {
            return elementName;
        }

        UtXmlData xmlData = getXmlData(slotId);
        if (xmlData == null) {
            return NS_SS_PREFIX + elementName;
        }

        String tag = getTag(xmlData, elementName);
        if (tag == null) {
            return xmlData.mNsSsPrefix + elementName;
        }

        return tag;
    }

    protected static String getCpElement(int slotId, String elementName) {
        if (SscConfig.isOmitNamespaceCp(slotId)) {
            return elementName;
        }

        UtXmlData xmlData = getXmlData(slotId);
        if (xmlData == null) {
            return NS_CP_PREFIX + elementName;
        }

        String tag = getTag(xmlData, elementName);
        if (tag == null) {
            return xmlData.mNsCpPrefix + elementName;
        }

        return tag;
    }

    protected static void displayTags(int slotId) {
        UtXmlData xmlData = getXmlData(slotId);
        if (xmlData == null) {
            ImsLog.d("wrong slot - " + slotId);
            return;
        }

        xmlData.getTags().forEach((name, ns) -> { ImsLog.d( "Name:" + name + ", NS: " + ns ); });
    }

    protected static void setRuleId(int slotId, int mediaType, String serviceName, int condition,
            String ruleId) {
        UtXmlData xmlData = getXmlData(slotId);
        if (xmlData == null) {
            return;
        }

        HashMap<String, HashMap<Integer, String>> services = null;
        if (mediaType == MEDIA_TYPE_AUDIO) {
            services = xmlData.getAudioRuleIds();
        } else { // MEDIA_TYPE_VIDEO
            services = xmlData.getVideoRuleIds();
        }

        if (!services.containsKey(serviceName)) {
            services.put(serviceName, new HashMap<Integer, String>());
        }

        HashMap<Integer, String> ruleIds = services.get(serviceName);
        ruleIds.put(condition, ruleId);
        ImsLog.d(slotId, "mediaType : " + mediaType + ", service : " + serviceName +
                ", condition : " + condition + ", ruleId : " + ruleId);
    }

    protected static String getRuleId(int slotId, int mediaType, String serviceName,
            int condition) {
        UtXmlData xmlData = getXmlData(slotId);
        if (xmlData == null) {
            return null;
        }

        HashMap<String, HashMap<Integer, String>> services = null;
        if (mediaType == MEDIA_TYPE_AUDIO) {
            services = xmlData.getAudioRuleIds();
        } else {
            services = xmlData.getVideoRuleIds();
        }

        if (!services.containsKey(serviceName)) {
            ImsLog.e(slotId, "no ruleId");
            return null;
        }

        return services.get(serviceName).get(condition);
    }

    protected static String getDefaultRuleId(int slotId, int mediaType, String serviceName,
            int condition) {
        String ruleId = null;
        if (CD.equals(serviceName)) {
            switch (condition) {
                case SscConstant.CONDITION_CFU:
                    ruleId = RULE_ID_CFU;
                    break;
                case SscConstant.CONDITION_CFB:
                    ruleId = RULE_ID_CFB;
                    break;
                case SscConstant.CONDITION_CFNR:
                    ruleId = RULE_ID_CFNR;
                    break;
                case SscConstant.CONDITION_CFNRC:
                    ruleId = RULE_ID_CFNRC;
                    break;
                case SscConstant.CONDITION_CFNL:
                    ruleId = RULE_ID_CFNL;
                    break;
                default:
                    // do nothing
                    break;
            }
        } else if (ICB.equals(serviceName)) {
            switch (condition) {
                case SscConstant.CONDITION_BAIC:
                    ruleId = RULE_ID_BAIC;
                    break;
                case SscConstant.CONDITION_BIC_WR:
                    ruleId = RULE_ID_BIC_WR;
                    break;
                case SscConstant.CONDITION_ACR:
                    ruleId = RULE_ID_ACR;
                    break;
                default:
                    // do nothing
                    break;
            }
        } else if (OCB.equals(serviceName)) {
            switch (condition) {
                case SscConstant.CONDITION_BAOC:
                    ruleId = RULE_ID_BAOC;
                    break;
                case SscConstant.CONDITION_BOIC:
                    ruleId = RULE_ID_BOIC;
                    break;
                case SscConstant.CONDITION_BOIC_EXHC:
                    ruleId = RULE_ID_BOIC_EXHC;
                    break;
                default:
                    // do nothing
                    break;
            }
        }

        if (ruleId != null && mediaType == MEDIA_TYPE_VIDEO) {
            ruleId += "-video";
        }

        ImsLog.d(slotId, "default rule id is " + ruleId);
        return ruleId;
    }

    protected static String getRuleConditionTag(int slotId, String serviceName, int condition) {
        String ruleConditionTag = null;
        if (CD.equals(serviceName)) {
            switch (condition) {
                case SscConstant.CONDITION_CFU:
                    ruleConditionTag = "";
                    break;
                case SscConstant.CONDITION_CFB:
                    ruleConditionTag = CFB;
                    break;
                case SscConstant.CONDITION_CFNR:
                    ruleConditionTag = CFNR;
                    break;
                case SscConstant.CONDITION_CFNRC:
                    ruleConditionTag = CFNRC;
                    break;
                case SscConstant.CONDITION_CFNL:
                    ruleConditionTag = CFNL;
                    break;
                default:
                    // do nothing
                    break;
            }
        } else if (ICB.equals(serviceName) || OCB.equals(serviceName)) {
            switch (condition) {
                case SscConstant.CONDITION_BAIC:
                case SscConstant.CONDITION_BAOC:
                    ruleConditionTag = "";
                    break;
                case SscConstant.CONDITION_BIC_WR:
                    ruleConditionTag = BIC_WR;
                    break;
                case SscConstant.CONDITION_ACR:
                    ruleConditionTag = ACR;
                    break;
                case SscConstant.CONDITION_BOIC:
                    ruleConditionTag = BOIC;
                    break;
                case SscConstant.CONDITION_BOIC_EXHC:
                    ruleConditionTag = BOIC_EXHC;
                    break;
                default:
                    // do nothing
                    break;
            }
        }

        if (TextUtils.isEmpty(ruleConditionTag)) {
            // in case of unconditional, no rule condition tag
            return "";
        }

        return getSsElement(slotId, ruleConditionTag);
    }

    protected static boolean getIsNoReplyTimerOmitted(int slotId) {
        UtXmlData xmlData = getXmlData(slotId);
        if (xmlData == null) {
            return false;
        }

        return xmlData.getIsNoReplyTimerOmitted();
    }

    protected static void setIsNoReplyTimerOmitted(int slotId, boolean isNoReplyTimerOmitted) {
        UtXmlData xmlData = getXmlData(slotId);
        if (xmlData == null) {
            return;
        }

        xmlData.setIsNoReplyTimerOmitted(isNoReplyTimerOmitted);
    }

    protected static boolean getIsNoReplyTimerInRule(int slotId) {
        UtXmlData xmlData = getXmlData(slotId);
        if (xmlData == null) {
            return false;
        }

        return xmlData.getIsNoReplyTimerInRule();
    }

    protected static void setIsNoReplyTimerInRule(int slotId, boolean isNoReplyTimerInRule) {
        UtXmlData xmlData = getXmlData(slotId);
        if (xmlData == null) {
            return;
        }

        xmlData.setIsNoReplyTimerInRule(isNoReplyTimerInRule);
    }

    protected static boolean getCfnlExist(int slotId) {
        UtXmlData xmlData = getXmlData(slotId);
        if (xmlData == null) {
            return false;
        }

        return xmlData.getIsCfnlExist();
    }

    protected static void setCfnlExist(int slotId, boolean isCfnlExist) {
        UtXmlData xmlData = getXmlData(slotId);
        if (xmlData == null) {
            return;
        }

        xmlData.setIsCfnlExist(isCfnlExist);
    }

    protected static boolean getProvisionStatus(int slotId, String serviceName, int condition) {
        UtXmlData xmlData = getXmlData(slotId);
        if (xmlData == null) {
            return false;
        }

        if (SC_CD.equals(serviceName)) {
            if (xmlData.mCfProvisionStatus.containsKey(condition)) {
                return xmlData.mCfProvisionStatus.get(condition);
            }
        } else if (SC_CB.equals(serviceName)) {
            if (xmlData.mCbProvisionStatus.containsKey(condition)) {
                return xmlData.mCbProvisionStatus.get(condition);
            }
        }

        return false;
    }

    protected static void setProvisionStatus(int slotId, String serviceName, int condition,
            boolean provisioned) {
        UtXmlData xmlData = getXmlData(slotId);
        if (xmlData == null) {
            return;
        }

        if (SC_CD.equals(serviceName)) {
            xmlData.mCfProvisionStatus.put(condition, provisioned);
        } else if (SC_CB.equals(serviceName)) {
            xmlData.mCbProvisionStatus.put(condition, provisioned);
        }
    }

    protected static boolean isNamespaceSsSupported(int slotId) {
        UtXmlData xmlData = getXmlData(slotId);
        if (xmlData == null) {
            return false;
        }

        return NS_SS_PREFIX.equals(xmlData.mNsSsPrefix);
    }

    protected static boolean isNamespaceCpSupported(int slotId) {
        UtXmlData xmlData = getXmlData(slotId);
        if (xmlData == null) {
            return false;
        }

        return NS_CP_PREFIX.equals(xmlData.mNsCpPrefix);
    }

    protected static boolean getMediaCapability(int slotId, String serviceName,
            int mediaType) {
        UtXmlData xmlData = getXmlData(slotId);
        if (xmlData == null) {
            return false;
        }

        if (SC_CD.equals(serviceName)) {
            if (xmlData.mCfMeidaCapability.containsKey(mediaType)) {
                return xmlData.mCfMeidaCapability.get(mediaType);
            }
        } else if (SC_CB.equals(serviceName)) {
            if (xmlData.mCbMeidaCapability.containsKey(mediaType)) {
                return xmlData.mCbMeidaCapability.get(mediaType);
            }
        }

        return false;
    }

    protected static void setMediaCapability(int slotId, String serviceName, int mediaType,
            boolean mediaCapability) {
        UtXmlData xmlData = getXmlData(slotId);
        if (xmlData == null) {
            return;
        }

        if (SC_CD.equals(serviceName)) {
            xmlData.mCfMeidaCapability.put(mediaType, mediaCapability);
        } else if (SC_CB.equals(serviceName)) {
            xmlData.mCbMeidaCapability.put(mediaType, mediaCapability);
        }
    }
}
