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

import com.android.imsstack.enabler.ssc.data.SscServiceQueryData;

import org.junit.After;
import org.junit.Before;
import org.junit.Test;
import org.junit.runner.RunWith;
import org.junit.runners.JUnit4;
import org.w3c.dom.Document;
import org.xml.sax.InputSource;

import java.io.StringReader;

import javax.xml.parsers.DocumentBuilder;
import javax.xml.parsers.DocumentBuilderFactory;

@RunWith(JUnit4.class)
public class SscXmlParserTest {
    private static final int SLOT_0 = 0;

    private SscXmlParser mSscXmlParser;

    // test configurations
    private boolean mCfnlExist = false;
    private boolean mTimerInCfnr = false;
    private boolean mCbRuleActive = false;
    private boolean mCfRuleActive = false;
    private boolean mCwEnable = false;
    private boolean mOipEnable = false;
    private boolean mTipEnable = false;
    private String mErrorPhrase = "";
    private String mForwardNumber = "tel:+1234567890";
    private String mDefaultBehaviour = SscXmlFormat.PRESENTATION_NOT_RESTRICTED;

    @Before
    public void setup() {
        SscXmlFormat.init(SLOT_0);
        mSscXmlParser = new SscXmlParser();
    }

    @After
    public void tearDown() {
    }

    @Test
    public void getSscServiceFromDoc_serviceCapability() {
        SscServiceQueryData queryData = createQueryData();
        queryData.setResponseCode(SscConstant.HTTP_OK);
        mSscXmlParser.getSscServiceFromDoc(queryData, createEntireXmldoc(), null);

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
                SscXmlFormat.AUDIO));
        assertEquals(false, SscXmlFormat.getMediaCapability(SLOT_0, SscXmlFormat.SC_CD,
                SscXmlFormat.VIDEO));

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
                SscXmlFormat.AUDIO));
        assertEquals(true, SscXmlFormat.getMediaCapability(SLOT_0, SscXmlFormat.SC_CB,
                SscXmlFormat.VIDEO));
    }

    private SscServiceQueryData createQueryData() {
        return new SscServiceQueryData(SLOT_0, ESsType.NONE, SscConstant.EVENT_SSC_BASE, 0, 0);
    }

    private Document createEntireXmldoc() {
        String xml = "<ss:simservs xmlns:SOAP-ENV=\"http://schemas.xmlsoap.org/soap/envelope/\" "
                + "xmlns:SOAP-ENC=\"http://schemas.xmlsoap.org/soap/encoding/\" "
                + "xmlns:xsi=\"http://www.w3.org/2001/XMLSchema-instance\" "
                + "xmlns:xsd=\"http://www.w3.org/2001/XMLSchema\" "
                + "xmlns:cp=\"urn:ietf:params:xml:ns:common-policy\" "
                + "xmlns:ss=\"http://uri.etsi.org/ngn/params/xml/simservs/xcap\" "
                + "xmlns:ocp=\"urn:oma:xml:xdm:common-policy\" "
                + "xmlns:utns=\"urn:com:att:tlv:utx\" "
                + "xmlns:xe=\"urn:ietf:params:xml:ns:xcap-error\" "
                + "xmlns:data=\"http://com/alu/icm/fs5000dbv5_0/data\" "
                + "xmlns:prs=\"http://www.nokia.com/prs/SubscriptionAPI\"> \n"
                // ICB
                + "<ss:incoming-communication-barring active=\"true\">\n"
                + "<cp:ruleset xmlns:cp=\"urn:ietf:params:xml:ns:common-policy\">\n"
                + "<cp:rule id=\"call-barring-all-incoming\">\n"
                + "<cp:conditions>\n"
                + (mCbRuleActive ? "" : "<ss:rule-deactivated/>\n")
                + "</cp:conditions>\n"
                + "<cp:actions>\n"
                + "<ss:allow>false</ss:allow>\n"
                + "</cp:actions>\n"
                + "</cp:rule>\n"
                + "<cp:rule id=\"call-barring-incoming-in-roaming\">\n"
                + "<cp:conditions>\n"
                + (mCbRuleActive ? "" : "<ss:rule-deactivated/>\n")
                + "<ss:roaming/>\n"
                + "</cp:conditions>\n"
                + "<cp:actions>\n"
                + "<ss:allow>false</ss:allow>\n"
                + "</cp:actions>\n"
                + "</cp:rule>\n"
                + "</cp:ruleset>\n"
                + "</ss:incoming-communication-barring>\n"
                // OCB
                + "<ss:outgoing-communication-barring active=\"true\">\n"
                + "<cp:ruleset xmlns:cp=\"urn:ietf:params:xml:ns:common-policy\">\n"
                + "<cp:rule id=\"call-barring-all-outgoing\">\n"
                + "<cp:conditions>\n"
                + (mCbRuleActive ? "" : "<ss:rule-deactivated/>\n")
                + "</cp:conditions>\n"
                + "<cp:actions>\n"
                + "<ss:allow>false</ss:allow>\n"
                + "</cp:actions>\n"
                + "</cp:rule>\n"
                + "<cp:rule id=\"call-barring-outgoing-international\">\n"
                + "<cp:conditions>\n"
                + (mCbRuleActive ? "" : "<ss:rule-deactivated/>\n")
                + "<ss:international/>\n"
                + "</cp:conditions>\n"
                + "<cp:actions>\n"
                + "<ss:allow>false</ss:allow>\n"
                + "</cp:actions>\n"
                + "</cp:rule>\n"
                + "<cp:rule id=\"call-barring-outgoing-internationalExHC\">\n"
                + "<cp:conditions>\n"
                + (mCbRuleActive ? "" : "<ss:rule-deactivated/>\n")
                + "<ss:international-exHC/>\n"
                + "</cp:conditions>\n"
                + "<cp:actions>\n"
                + "<ss:allow>false</ss:allow>\n"
                + "</cp:actions>\n"
                + "</cp:rule>\n"
                + "</cp:ruleset>\n"
                + "</ss:outgoing-communication-barring>"
                // CW
                + "<ss:communication-waiting active=\""
                + (mCwEnable ? "true" : "false") + "\"/> \n"
                // CD
                + "<ss:communication-diversion active=\"true\"> \n"
                + (mTimerInCfnr ? "" : "<ss:NoReplyTimer>20</ss:NoReplyTimer> \n")
                + "<cp:ruleset> \n"
                + "<cp:rule id=\"call-diversion-unconditional\"> \n"
                + "<cp:conditions> \n"
                + (mCfRuleActive ? "" : "<ss:rule-deactivated/>\n")
                + "</cp:conditions> \n"
                + "<cp:actions> \n"
                + "<ss:forward-to> \n"
                + "<ss:target>" + mForwardNumber + "</ss:target> \n"
                + "<ss:notify-caller>true</ss:notify-caller> \n"
                + "<ss:reveal-identity-to-caller>true</ss:reveal-identity-to-caller>\n"
                + "<ss:reveal-served-user-identity-to-caller>false"
                + "</ss:reveal-served-user-identity-to-caller>\n"
                + "<ss:notify-served-user>false</ss:notify-served-user> \n"
                + "<ss:notify-served-user-on-outbound-call>false"
                + "</ss:notify-served-user-on-outbound-call> \n"
                + "<ss:reveal-identity-to-target>false</ss:reveal-identity-to-target> \n"
                + "</ss:forward-to> \n"
                + "</cp:actions> \n"
                + "</cp:rule> \n"
                + "<cp:rule id=\"call-diversion-busy\"> \n"
                + "<cp:conditions> \n"
                + (mCfRuleActive ? "" : "<ss:rule-deactivated/>\n")
                + "<ss:busy/> \n"
                + "</cp:conditions> \n"
                + "<cp:actions> \n"
                + "<ss:forward-to> \n"
                + "<ss:target>" + mForwardNumber + "</ss:target> \n"
                + "<ss:notify-caller>true</ss:notify-caller> \n"
                + "<ss:reveal-identity-to-caller>true</ss:reveal-identity-to-caller> \n"
                + "<ss:reveal-served-user-identity-to-caller>false"
                + "</ss:reveal-served-user-identity-to-caller> \n"
                + "<ss:reveal-identity-to-target>false</ss:reveal-identity-to-target> \n"
                + "</ss:forward-to> \n"
                + "</cp:actions> \n"
                + "</cp:rule> \n"
                + "<cp:rule id=\"call-diversion-no-reply\"> \n"
                + "<cp:conditions> \n"
                + (mCfRuleActive ? "" : "<ss:rule-deactivated/>\n")
                + "<ss:no-answer/> \n"
                + "</cp:conditions> \n"
                + "<cp:actions> \n"
                + "<ss:forward-to> \n"
                + "<ss:target>" + mForwardNumber + "</ss:target> \n"
                + (mTimerInCfnr ? "<ss:NoReplyTimer>20</ss:NoReplyTimer> \n" : "")
                + "<ss:notify-caller>true</ss:notify-caller> \n"
                + "<ss:reveal-identity-to-caller>true</ss:reveal-identity-to-caller> \n"
                + "<ss:reveal-served-user-identity-to-caller>false"
                + "</ss:reveal-served-user-identity-to-caller> \n"
                + "<ss:reveal-identity-to-target>false</ss:reveal-identity-to-target> \n"
                + "</ss:forward-to> \n"
                + "</cp:actions> \n"
                + "</cp:rule> \n"
                + "<cp:rule id=\"call-diversion-not-reachable\"> \n"
                + "<cp:conditions> \n"
                + (mCfRuleActive ? "" : "<ss:rule-deactivated/>\n")
                + "<ss:not-reachable/> \n"
                + "</cp:conditions> \n"
                + "<cp:actions> \n"
                + "<ss:forward-to> \n"
                + "<ss:target>" + mForwardNumber + "</ss:target> \n"
                + "<ss:notify-caller>true</ss:notify-caller> \n"
                + "<ss:reveal-identity-to-caller>true</ss:reveal-identity-to-caller> \n"
                + "<ss:reveal-served-user-identity-to-caller>false"
                + "</ss:reveal-served-user-identity-to-caller> \n"
                + "<ss:reveal-identity-to-target>false</ss:reveal-identity-to-target> \n"
                + "</ss:forward-to> \n"
                + "</cp:actions> \n"
                + "</cp:rule> \n"
                + (mCfnlExist ? "<cp:rule id=\"call-diversion-not-loggedin\"> \n"
                + "<cp:conditions> \n"
                + (mCfRuleActive ? "" : "<ss:rule-deactivated/>\n")
                + "<ss:not-registered/> \n"
                + "</cp:conditions> \n"
                + "<cp:actions> \n"
                + "<ss:forward-to> \n"
                + "<ss:target>" + mForwardNumber + "</ss:target> \n"
                + "<ss:notify-caller>true</ss:notify-caller> \n"
                + "<ss:reveal-identity-to-caller>true</ss:reveal-identity-to-caller> \n"
                + "<ss:reveal-served-user-identity-to-caller>false"
                + "</ss:reveal-served-user-identity-to-caller> \n"
                + "<ss:reveal-identity-to-target>false</ss:reveal-identity-to-target> \n"
                + "</ss:forward-to> \n"
                + "</cp:actions> \n"
                + "</cp:rule> \n" : "")
                + "</cp:ruleset> \n"
                + "</ss:communication-diversion> \n"
                // CB service capability
                + "<ss:communication-barring-serv-cap active=\"true\"> \n"
                + "<ss:serv-cap-conditions> \n"
                + "<ss:serv-cap-anonymous provisioned=\"false\"/> \n"
                + "<ss:serv-cap-communication-diverted provisioned=\"false\"/> \n"
                + "<ss:serv-cap-external-list provisioned=\"false\"/> \n"
                + "<ss:serv-cap-identity provisioned=\"false\"/> \n"
                + "<ss:serv-cap-international provisioned=\"true\"/> \n"
                + "<ss:serv-cap-international-exHC provisioned=\"true\"/> \n"
                + "<ss:serv-cap-unconditional provisioned=\"false\"/> \n"
                + "<ss:serv-cap-media> \n"
                + "<ss:media>audio</ss:media> \n"
                + "<ss:media>video</ss:media> \n"
                + "<ss:media>message</ss:media> \n"
                + "<ss:media>image</ss:media> \n"
                + "</ss:serv-cap-media> \n"
                + "<ss:serv-cap-other-identity provisioned=\"false\"/> \n"
                + "<ss:serv-cap-presence-status provisioned=\"false\"/> \n"
                + "<ss:serv-cap-roaming provisioned=\"true\"/> \n"
                + "<ss:serv-cap-rule-deactivated provisioned=\"true\"/> \n"
                + "<ss:serv-cap-validity provisioned=\"false\"/> \n"
                + "</ss:serv-cap-conditions> \n"
                + "</ss:communication-barring-serv-cap>"
                // CD service capability
                + "<ss:communication-diversion-serv-cap active=\"true\"> \n"
                + "<ss:serv-cap-conditions> \n"
                + "<ss:serv-cap-anonymous provisioned=\"false\"/> \n"
                + "<ss:serv-cap-busy provisioned=\"true\"/> \n"
                + "<ss:serv-cap-external-list provisioned=\"false\"/> \n"
                + "<ss:serv-cap-identity provisioned=\"false\"/> \n"
                + "<ss:serv-cap-media> \n"
                + "<ss:media>audio</ss:media> \n"
                + "</ss:serv-cap-media> \n"
                + "<ss:serv-cap-not-registered provisioned=\"false\"/> \n"
                + "<ss:serv-cap-no-answer provisioned=\"false\"/> \n"
                + "<ss:serv-cap-not-reachable provisioned=\"true\"/> \n"
                + "<ss:serv-cap-presence-status provisioned=\"false\"/> \n"
                + "<ss:serv-cap-rule-deactivated provisioned=\"true\"/>\n"
                + "<ss:serv-cap-selective provisioned=\"false\"/> \n"
                + "<ss:serv-cap-validity provisioned=\"false\"/> \n"
                + "<ss:serv-cap-default provisioned=\"true\"/> \n"
                + "</ss:serv-cap-conditions> \n"
                + "<ss:serv-cap-actions> \n"
                + "<ss:serv-cap-target> \n"
                + "<ss:telephony-type/> \n"
                + "</ss:serv-cap-target> \n"
                + "<ss:serv-cap-notify-caller provisioned=\"false\"/> \n"
                + "<ss:serv-cap-notify-served-user provisioned=\"false\"/> \n"
                + "<ss:serv-cap-notify-served-user-on-outbound-call provisioned=\"false\"/> \n"
                + "<ss:serv-cap-reveal-identity-to-caller provisioned=\"false\"/> \n"
                + "<ss:serv-cap-reveal-served-user-identity-to-caller provisioned=\"false\"/> \n"
                + "<ss:serv-cap-reveal-identity-to-target provisioned=\"false\"/> \n"
                + "</ss:serv-cap-actions> \n"
                + "</ss:communication-diversion-serv-cap> \n"
                // OIP
                + "<ss:originating-identity-presentation active=\""
                + (mOipEnable ? "true" : "false") + "\"/> \n"
                // OIR
                + "<ss:originating-identity-presentation-restriction active=\"true\"> \n"
                + "<ss:default-behaviour>" + mDefaultBehaviour + "</ss:default-behaviour> \n"
                + "</ss:originating-identity-presentation-restriction> \n"
                // TIP
                + "<ss:terminating-identity-presentation active=\""
                + (mTipEnable ? "true" : "false") + "\"/> \n"
                // TIR
                + "<ss:terminating-identity-presentation-restriction active=\"true\"> \n"
                + "<ss:default-behaviour>" + mDefaultBehaviour + "</ss:default-behaviour> \n"
                + "</ss:terminating-identity-presentation-restriction> \n"
                + "</ss:simservs> \n";

        DocumentBuilderFactory factory = DocumentBuilderFactory.newInstance();
        DocumentBuilder builder;
        InputSource is;
        Document document;
        try {
            factory.setNamespaceAware(true);
            builder = factory.newDocumentBuilder();
            is = new InputSource(new StringReader(xml));
            document = builder.parse(is);
        } catch (Exception e) {
            e.printStackTrace();
            return null;
        }

        return document;
    }
}
