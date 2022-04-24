package com.android.imsstack.enabler.ssc;

import android.text.TextUtils;

import com.android.imsstack.core.OperatorInfo;
import com.android.imsstack.util.ImsLog;

import java.util.HashMap;

public class SscXmlFormat {
    private static class UtXmlData {
        private boolean mIsNoReplyTimerInRule = false;
        private boolean mIsCfnlProvisioned = false;
        private String mNsSsPrefix = "";
        private String mNsCpPrefix = "";
        private HashMap<String, String> mTags = new HashMap<>();
        private HashMap<String, HashMap<Integer, String>> mAudioRuleIds = new HashMap<>();
        private HashMap<String, HashMap<Integer, String>> mVideoRuleIds = new HashMap<>();

        public void setIsNoReplyTimerInRule(boolean isNoReplyTimerInRule) {
            mIsNoReplyTimerInRule = isNoReplyTimerInRule;
        }

        public void setIsCfnlProvisioned(boolean isCfnlProvisioned) {
            mIsCfnlProvisioned = isCfnlProvisioned;
        }

        public boolean getIsNoReplyTimerInRule() {
            return mIsNoReplyTimerInRule;
        }

        public boolean getIsCfnlProvisioned() {
            return mIsCfnlProvisioned;
        }

        public HashMap<String, String> getTags() { return mTags; }
        public HashMap<String, HashMap<Integer, String>> getAudioRuleIds() { return mAudioRuleIds; }
        public HashMap<String, HashMap<Integer, String>> getVideoRuleIds() { return mVideoRuleIds; }
    };

    private static HashMap<Integer, UtXmlData> mXmlDatas = new HashMap<>();

    public static final String NS_SS_PREFIX = "ss:";
    public static final String NS_CP_PREFIX = "cp:";
    public static final String NS_XE_PREFIX = "xe:";
    public static final String NS_SS_URI
            = "xmlns(ss=http://uri.etsi.org/ngn/params/xml/simservs/xcap)";
    public static final String NS_CP_URI = "xmlns(cp=urn:ietf:params:xml:ns:common-policy)";

    // SS service Elements
    public static final String SIMSERVS = "simservs";
    public static final String CW = "communication-waiting";
    public static final String CD = "communication-diversion";
    public static final String ICB = "incoming-communication-barring";
    public static final String OCB = "outgoing-communication-barring";
    public static final String OIP = "originating-identity-presentation";
    public static final String OIR = "originating-identity-presentation-restriction";
    public static final String TIP = "terminating-identity-presentation";
    public static final String TIR = "terminating-identity-presentation-restriction";

    // SS CF Condition Elements
    public static final String CFB = "busy";
    public static final String CFNR = "no-answer";
    public static final String CFNRC = "not-reachable";
    public static final String CFNL = "not-registered";

    // SS CB Condition Elements
    public static final String BOIC = "international";
    public static final String BOIC_EXHC = "international-exHC";
    public static final String BIC_WR = "roaming";
    public static final String ACR = "anonymous";

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
    public static final String PHRASE = "phrase";

    public static final int MEDIA_AUDIO = 0;
    public static final int MEDIA_VIDEO = 1;


    public static void init(int slotId) {
        ImsLog.d("init (" + slotId + ")");

        if (OperatorInfo.isSlotIdValid(slotId) != true) {
            ImsLog.w("init() - Invalid SlotId(" + slotId + ")");
            return;
        }

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
        if (mXmlDatas.containsKey(slotId) == false) {
            ImsLog.w("wrong slot - " + slotId );
            return null;
        }

        return mXmlDatas.get(slotId);
    }

    private static String getTag(UtXmlData xmlData, String elementName) {
        HashMap<String, String> tags = xmlData.getTags();
        if (tags.containsKey(elementName) == false) {
            return null;
        }

        String namespace = tags.get(elementName);
        if (TextUtils.isEmpty(namespace)) {
            return elementName;
        }

        return namespace + ":" + elementName;
    }

    public static void setTag(int slotId, String tagName, String namespace) {
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

        if (xmlData.getTags().containsKey(tagName) == false) {
            xmlData.getTags().put(tagName, namespace);
        }
    }

    public static String getSsElement(int slotId, String elementName) {
        UtXmlData xmlData = getXmlData(slotId);
        if (xmlData == null) {
            return elementName;
        }

        String tag = getTag(xmlData, elementName);
        if (tag == null) {
            return xmlData.mNsSsPrefix + elementName;
        }

        return tag;
    }

    public static String getCpElement(int slotId, String elementName) {
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

    public static void displayTags(int slotId) {
        UtXmlData xmlData = getXmlData(slotId);
        if (xmlData == null) {
            ImsLog.d("wrong slot - " + slotId);
            return;
        }

        xmlData.getTags().forEach((name, ns) -> { ImsLog.d( "Name:" + name + ", NS: " + ns ); });
    }

    public static void setRuleId(int slotId, int mediaType, String serviceName, int condition,
            String ruleId) {
        UtXmlData xmlData = getXmlData(slotId);
        if (xmlData == null) {
            return;
        }

        HashMap<String, HashMap<Integer, String>> services = null;
        if (mediaType == MEDIA_AUDIO) {
            services = xmlData.getAudioRuleIds();
        } else { // MEDIA_VIDEO
            services = xmlData.getVideoRuleIds();
        }

        if (services.containsKey(serviceName) == false) {
            services.put(serviceName, new HashMap<Integer, String>());
        }

        HashMap<Integer, String> ruleIds = services.get(serviceName);
        ruleIds.put(condition, ruleId);
        ImsLog.d(slotId, "mediaType : " + mediaType + ", service : " + serviceName +
                ", condition : " + condition + ", ruleId : " + ruleId);
    }

    public static String getRuleId(int slotId, int mediaType, String serviceName,
            int condition) {
        UtXmlData xmlData = getXmlData(slotId);
        if (xmlData == null) {
            return null;
        }

        HashMap<String, HashMap<Integer, String>> services = null;
        if (mediaType == MEDIA_AUDIO) {
            services = xmlData.getAudioRuleIds();
        } else {
            services = xmlData.getVideoRuleIds();
        }

        if (services.containsKey(serviceName) == false) {
            ImsLog.e(slotId, "no ruleId");
            return null;
        }

        return services.get(serviceName).get(condition);
    }

    public static boolean getIsNoReplyTimerInRule(int slotId) {
        UtXmlData xmlData = getXmlData(slotId);
        if (xmlData == null) {
            return false;
        }

        return xmlData.getIsNoReplyTimerInRule();
    }

    public static void setIsNoReplyTimerInRule(int slotId, boolean isNoReplyTimerInRule) {
        UtXmlData xmlData = getXmlData(slotId);
        if (xmlData == null) {
            return;
        }

        xmlData.setIsNoReplyTimerInRule(isNoReplyTimerInRule);
    }

    public static boolean getIsCfnlProvisioned(int slotId) {
        UtXmlData xmlData = getXmlData(slotId);
        if (xmlData == null) {
            return false;
        }

        return xmlData.getIsCfnlProvisioned();
    }

    public static void setIsCfnlProvisioned(int slotId, boolean isCfnlProvisioned) {
        UtXmlData xmlData = getXmlData(slotId);
        if (xmlData == null) {
            return;
        }

        xmlData.setIsCfnlProvisioned(isCfnlProvisioned);
    }
}
