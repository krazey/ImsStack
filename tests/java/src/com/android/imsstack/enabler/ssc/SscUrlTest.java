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

import com.android.imsstack.enabler.ssc.data.SscServiceData;

import org.junit.Before;
import org.junit.Test;
import org.junit.runner.RunWith;
import org.junit.runners.JUnit4;

@RunWith(JUnit4.class)
public class SscUrlTest {
    private static final int SLOT_0 = 0;

    private String mXui = "tel:+1234567890";

    private SscUrl mSscUrl;

    @Before
    public void setup() {
        SscXmlFormat.init(SLOT_0);

        mSscUrl = SscUrl.getInstance();
    }

    @Test
    public void getUpdateUri_insertCf() {
        String expectedUri = "/simservs.ngn.etsi.org/users/" + mXui + "/simservs.xml"
                + "/~~/simservs/communication-diversion";

        SscXmlFormat.setTag(SLOT_0, SscXmlFormat.SIMSERVS, "");
        SscXmlFormat.setTag(SLOT_0, SscXmlFormat.CD, "");

        SscServiceData insertData = getInsertCfData(ESsType.CF, SscConstant.ACTION_ACTIVATION,
                SscConstant.CONDITION_CFB, null);

        String insertUri = mSscUrl.getUpdateUri(insertData, mXui);

        assertEquals(expectedUri, insertUri);
    }

    @Test
    public void getUpdateUri_insertIcb() {
        String expectedUri = "/simservs.ngn.etsi.org/users/" + mXui + "/simservs.xml"
                + "/~~/simservs/incoming-communication-barring";

        SscXmlFormat.setTag(SLOT_0, SscXmlFormat.SIMSERVS, "");
        SscXmlFormat.setTag(SLOT_0, SscXmlFormat.ICB, "");

        SscServiceData insertData = getInsertCbData(ESsType.ICB, SscConstant.ACTION_ACTIVATION,
                SscConstant.CONDITION_BAIC);

        String insertUri = mSscUrl.getUpdateUri(insertData, mXui);

        assertEquals(expectedUri, insertUri);
    }

    @Test
    public void getUpdateUri_insertOcb() {
        String expectedUri = "/simservs.ngn.etsi.org/users/" + mXui + "/simservs.xml"
                + "/~~/simservs/outgoing-communication-barring";

        SscXmlFormat.setTag(SLOT_0, SscXmlFormat.SIMSERVS, "");
        SscXmlFormat.setTag(SLOT_0, SscXmlFormat.OCB, "");

        SscServiceData insertData = getInsertCbData(ESsType.OCB, SscConstant.ACTION_ACTIVATION,
                SscConstant.CONDITION_BAOC);

        String insertUri = mSscUrl.getUpdateUri(insertData, mXui);

        assertEquals(expectedUri, insertUri);
    }

    private SscServiceData getInsertCfData(ESsType ssType, int action, int condition,
            String targetNumber) {
        return SscXmlGovTest.createInsertData(ssType, 0, action, condition, targetNumber);
    }

    private SscServiceData getInsertCbData(ESsType ssType, int action, int condition) {
        return SscXmlGovTest.createInsertData(ssType, 0, action, condition, null);
    }
}
