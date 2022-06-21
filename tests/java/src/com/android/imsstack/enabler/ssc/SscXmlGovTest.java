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

import static org.junit.Assert.assertNotNull;

import com.android.imsstack.enabler.ssc.data.CbServiceQueryData;
import com.android.imsstack.enabler.ssc.data.CbServiceUpdateData;
import com.android.imsstack.enabler.ssc.data.CfServiceQueryData;
import com.android.imsstack.enabler.ssc.data.CfServiceUpdateData;
import com.android.imsstack.enabler.ssc.data.SscServiceData;
import com.android.imsstack.enabler.ssc.data.SscServiceQueryData;

import org.junit.After;
import org.junit.Before;
import org.junit.Test;
import org.junit.runner.RunWith;
import org.junit.runners.JUnit4;
import org.w3c.dom.Document;
import org.w3c.dom.Element;
import org.xml.sax.InputSource;

import java.io.StringReader;

import javax.xml.parsers.DocumentBuilder;
import javax.xml.parsers.DocumentBuilderFactory;

@RunWith(JUnit4.class)
public class SscXmlGovTest {
    private static final int SLOT_0 = 0;

    private SscXmlGov mSscXmlGov;

    // test configurations
    protected static boolean sTimerInCfnr = false;

    @Before
    public void setup() {
        SscXmlFormat.init(SLOT_0);
        mSscXmlGov = SscXmlGov.getInstance(SLOT_0);
    }

    @After
    public void tearDown() {
    }

    @Test
    public void test() {

    }

    protected static SscServiceQueryData createDocumentQueryData() {
        return new SscServiceQueryData(SLOT_0, ESsType.NONE, SscConstant.EVENT_SSC_QUERY_DOCUMENT,
                0, 0);
    }

    protected static SscServiceQueryData createQueryData(ESsType ssType, int transactionId,
            int condition) {
        if (ssType == ESsType.CF) {
            return new CfServiceQueryData(SLOT_0, ssType, SscConstant.EVENT_SSC_QUERY_CF,
                    transactionId, condition, "", SscServiceClassUtil.SERVICE_CLASS_NONE);
        } else if (ssType == ESsType.ICB || ssType == ESsType.OCB) {
            return new CbServiceQueryData(SLOT_0, ssType, SscConstant.EVENT_SSC_QUERY_CB,
                    transactionId, condition, SscServiceClassUtil.SERVICE_CLASS_NONE);
        }

        return createDocumentQueryData();
    }

    protected static SscServiceData createUpdateData(ESsType ssType, int transactionId, int action,
            int condition) {
        if (ssType == ESsType.CF) {
            return new CfServiceUpdateData(SLOT_0, ssType, SscConstant.EVENT_SSC_UPDATE_CF,
                    transactionId, action, condition, null, 0,
                    SscServiceClassUtil.SERVICE_CLASS_VOICE);
        }

        return new CbServiceUpdateData(SLOT_0, ssType, SscConstant.EVENT_SSC_UPDATE_CB,
                transactionId, action, condition, null, SscServiceClassUtil.SERVICE_CLASS_VOICE,
                null);
    }

    protected static SscServiceData createInsertData(ESsType ssType, int transactionId, int action,
            int condition, String targetNumber) {
        if (ssType == ESsType.CF) {
            return new CfServiceUpdateData(SLOT_0, ssType, SscConstant.EVENT_SSC_INSERT_CF,
                    transactionId, action, condition, targetNumber, 0,
                    SscServiceClassUtil.SERVICE_CLASS_VOICE);
        }

        return new CbServiceUpdateData(SLOT_0, ssType, SscConstant.EVENT_SSC_INSERT_CB,
                transactionId, action, condition, null, SscServiceClassUtil.SERVICE_CLASS_VOICE,
                null);
    }

    protected static Document removeRuleSet(Document doc, String serviceName) {
        //Element serviceElement = doc.getElementsByTagNameNS("*", serviceName);

        return doc;
    }

    protected static Document removeRule(Document doc, String ruleId) {
        Element rule = doc.getElementById(ruleId);
        assertNotNull(rule);
        rule.getParentNode().removeChild(rule);

        return doc;
    }

    protected static Document createEntireXmlDoc(boolean timerInCfnr) {
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
                + "</cp:ruleset>"
                + "</ss:incoming-communication-barring>"
                // OCB
                + "<ss:outgoing-communication-barring active=\"true\">"
                + "<cp:ruleset xmlns:cp=\"urn:ietf:params:xml:ns:common-policy\">"
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
                + (timerInCfnr ? "" : "<ss:NoReplyTimer>25</ss:NoReplyTimer>")
                + "<cp:ruleset>"
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
                + (timerInCfnr ? "<ss:NoReplyTimer>25</ss:NoReplyTimer>" : "")
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
                + "<ss:originating-identity-presentation-restriction active=\"true\">"
                + "<ss:default-behaviour>presentation-not-restricted</ss:default-behaviour>"
                + "</ss:originating-identity-presentation-restriction>"
                // TIP
                + "<ss:terminating-identity-presentation active=\"true\"/>"
                // TIR
                + "<ss:terminating-identity-presentation-restriction active=\"true\">"
                + "<ss:default-behaviour>presentation-not-restricted</ss:default-behaviour>"
                + "</ss:terminating-identity-presentation-restriction>"
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
}
