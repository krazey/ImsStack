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

package com.android.imsstack.enabler.acs;

import static org.junit.Assert.assertEquals;
import static org.mockito.Mockito.doReturn;

import android.content.Context;
import android.test.suitebuilder.annotation.SmallTest;

import androidx.test.InstrumentationRegistry;

import com.android.imsstack.enabler.acs.impl.DataContainer;

import org.junit.After;
import org.junit.Before;
import org.junit.Test;
import org.mockito.Mock;
import org.mockito.MockitoAnnotations;

public class DataContainerTest {
    private static final String TAG = DataContainerTest.class.getSimpleName();
    private static final String AC_DATA =
            "<?xml version=\"1.0\"?>"
            + "<wap-provisioningdoc version=\"1.1\">"
            + "<characteristic type=\"VERS\">"
            + "<parm name=\"version\" value=\"1\"/>"
            + "<parm name=\"validity\" value=\"2160000\"/>"
            + "</characteristic>"
            + "<characteristic type=\"TOKEN\">"
            + "<parm name=\"token\" value=\"qawsedrftg\"/>"
            + "</characteristic>"
            + "<characteristic type=\"APPLICATION\">"
            + "<parm name=\"AppID\" value=\"ap2002\"/>"
            + "<parm name=\"Name\" value=\"RCS settings\"/>"
            + "<parm name=\"AppRef\" value=\"RCSe-Settings\"/>"
            + "<characteristic type=\"IMS\">"
            + "<parm name=\"To-AppRef\" value=\"IMS-Settings\"/>"
            + "</characteristic>"
            + "<characteristic type=\"SERVICES\">"
            + "<parm name=\"presencePrfl\" value=\"0\"/>"
            + "<parm name=\"ChatAuth\" value=\"1\"/>"
            + "<parm name=\"GroupChatAuth\" value=\"1\"/>"
            + "<parm name=\"ftAuth\" value=\"0\"/>"
            + "<parm name=\"standaloneMsgAuth\" value=\"1\"/>"
            + "<parm name=\"IR94VideoAuth\" value=\"1\"/>"
            + "<characteristic type=\"Ext\"/>"
            + "</characteristic>"
            + "<characteristic type=\"PRESENCE\">"
            + "<characteristic type=\"Ext\"/>"
            + "<parm name=\"client-obj-datalimit\" value=\"130000\"/>"
            + "<parm name=\"source-throttlepublish\" value=\"30\"/>"
            + "<parm name=\"max-number-ofsubscriptions-inpresence-list\" value=\"100\"/>"
            + "</characteristic>"
            + "<characteristic type=\"IM\">"
            + "<parm name=\"imMsgTech\" value=\"1\"/>"
            + "<parm name=\"imCapAlwaysON\" value=\"0\"/>"
            + "<parm name=\"GroupChatFullStandFwd\" value=\"1\"/>"
            + "<parm name=\"GroupChatOnlyFStandFwd\" value=\"0\"/>"
            + "<parm name=\"SmsFallBackAuth\" value=\"1\"/>"
            + "<parm name=\"imCapNonRCS\" value=\"0\"/>"
            + "<parm name=\"imWarnIW\" value=\"0\"/>"
            + "<parm name=\"AutAccept\" value=\"1\"/>"
            + "<parm name=\"AutAcceptGroupChat\" value=\"1\"/>"
            + "<parm name=\"imSessionStart\" value=\"0\"/>"
            + "<parm name=\"TimerIdle\" value=\"660\"/>"
            + "<parm name=\"multiMediaChat\" value=\"0\"/>"
            + "<parm name=\"MaxSize\" value=\"20480\"/>"
            + "<parm name=\"ftWarnSize\" value=\"10240\"/>"
            + "<parm name=\"MaxSizeFileTr\" value=\"102400\"/>"
            + "<parm name=\"MaxSizeFileTrIncoming\" value=\"102400\"/>"
            + "<parm name=\"ftAutAccept\" value=\"1\"/>"
            + "<parm name=\"ftHTTPCSURI\" value=\"https://ftcontentserver.rcs.mnc410.mcc310.pub.3gppnetwork.org/mStoreRoot/V1/FCS/ftCsUploadPath/\"/>"
            + "<parm name=\"ftHTTPCSUser\" value=\"1xxxxxxxxxx\"/>"
            + "<parm name=\"ftHTTPCSPwd\" value=\"mavenir\"/>"
            + "<parm name=\"ftDefaultMech\" value=\"HTTP\"/>"
            + "<parm name=\"max_adhoc_group_size\" value=\"100\"/>"
            + "<parm name=\"conf-fcty-uri\" value=\"sip:n-way_messaging@one.att.net\"/>"
            + "</characteristic>"
            + "<characteristic type=\"CPM\">"
            + "<characteristic type=\"StandaloneMsg\">"
            + "<parm name=\"MaxSizeStandalone\" value=\"1048576\"/>"
            + "</characteristic>"
            + "<characteristic type=\"Ext\"/>"
            + "</characteristic>"
            + "<characteristic type=\"CAPDISCOVERY\">"
            + "<parm name=\"disableInitialAddressBookScan\" value=\"1\"/>"
            + "<parm name=\"pollingPeriod\" value=\"0\"/>"
            + "<parm name=\"capInfoExpiry\" value=\"21600\"/>"
            + "<parm name=\"nonRCScapInfoExpiry\" value=\"259200\"/>"
            + "<parm name=\"defaultDisc\" value=\"1\"/>"
            + "<parm name=\"capDiscCommonStack\" value=\"0\"/>"
            + "<characteristic type=\"Ext\"/>"
            + "</characteristic>"
            + "<characteristic type=\"SERVICEPROVIDEREXT\">"
            + "<parm name=\"NMS_URL\" value=\"vcnms-c2s.enc.att.net\"/>"
            + "<parm name=\"NC_URL\" value=\"cns.att.net\"/>"
            + "</characteristic>"
            + "</characteristic>"
            + "</wap-provisioningdoc>";

    @Mock Context mContext;

    private DataContainer mDataContainer;
    private int mSlotId = 0;
    private int mSubId = 1;

    @Before
    public void setUp() throws Exception {
        MockitoAnnotations.initMocks(this);

        doReturn(InstrumentationRegistry.getContext().getFilesDir()).when(mContext).getFilesDir();

        mDataContainer = new DataContainer(mContext, mSlotId, mSubId);
        mDataContainer.resetProvisioningData();
    }

    @After
    public void tearDown() throws Exception {
        mDataContainer = null;
    }

    @Test
    @SmallTest
    public void getFileName_withSubId() throws Exception {
        int subId = 1234;
        String expectedFileName = "rcs_provisioning_data_" + subId + ".xml";
        String fileName;

        mDataContainer = new DataContainer(mContext, mSlotId, subId);
        fileName = mDataContainer.getFileName();
        assertEquals(expectedFileName, fileName);
        mDataContainer = null;

        subId = 5678;
        expectedFileName = "rcs_provisioning_data_" + subId + ".xml";
        mDataContainer = new DataContainer(mContext, mSlotId, subId);
        fileName = mDataContainer.getFileName();
        assertEquals(expectedFileName, fileName);
    }

    @Test
    @SmallTest
    public void getUpdateValue_withProvisioningDataVersionValidityToken() throws Exception {
        String data = mDataContainer.getProvisioningData();
        assertEquals(null, data);

        boolean isValid = mDataContainer.isValidProvisioning();
        assertEquals(false, isValid);

        mDataContainer.setProvisioningData(AC_DATA);
        data = mDataContainer.getProvisioningData();
        assertEquals(AC_DATA, data);

        isValid = mDataContainer.isValidProvisioning();
        assertEquals(true, isValid);

        int version = mDataContainer.getVersion();
        long validity = mDataContainer.getValidity();
        assertEquals(DataContainer.VERSION_UNKNOWN, version);
        assertEquals(DataContainer.VALIDITY_UNKNOWN, validity);

        int expectedVersion = 11;
        long expectedValidity = 360000L;
        mDataContainer.updateVersionValidity(expectedVersion, expectedValidity);

        version = mDataContainer.getVersion();
        validity = mDataContainer.getValidity();
        assertEquals(expectedVersion, version);
        assertEquals(expectedValidity, validity);

        String token = mDataContainer.getToken();
        assertEquals(DataContainer.TOKEN_UNKNOWN, token);

        String expectedToken = "ABC123";
        mDataContainer.updateToken(expectedToken);

        token = mDataContainer.getToken();
        assertEquals(expectedToken, token);
    }

    @Test
    @SmallTest
    public void getValue_fromFile() throws Exception {
        int subId = 1234;
        int expectedVersion = 11;
        long expectedValidity = 360000L;
        String expectedToken = "ABC123";

        mDataContainer = new DataContainer(mContext, mSlotId, subId);
        mDataContainer.setProvisioningData(AC_DATA);
        mDataContainer.updateVersionValidity(expectedVersion, expectedValidity);
        mDataContainer.updateToken(expectedToken);
        mDataContainer = null;

        DataContainer newDataContainer = new DataContainer(mContext, mSlotId, subId);

        int version = newDataContainer.getVersion();
        long validity = newDataContainer.getValidity();
        String token = newDataContainer.getToken();

        assertEquals(expectedVersion, version);
        assertEquals(expectedValidity, validity);
        assertEquals(expectedToken, token);
    }

    @Test
    @SmallTest
    public void getValue_afterReset() throws Exception {
        int subId = 1234;
        int expectedVersion = 11;
        long expectedValidity = 360000L;
        String expectedToken = "ABC123";

        mDataContainer = new DataContainer(mContext, mSlotId, subId);
        mDataContainer.setProvisioningData(AC_DATA);
        mDataContainer.updateVersionValidity(expectedVersion, expectedValidity);
        mDataContainer.updateToken(expectedToken);

        mDataContainer.resetProvisioningData();

        int version = mDataContainer.getVersion();
        long validity = mDataContainer.getValidity();
        String token = mDataContainer.getToken();

        // verify that cached values are reset
        assertEquals(DataContainer.VERSION_UNKNOWN, version);
        assertEquals(DataContainer.VALIDITY_UNKNOWN, validity);
        assertEquals(DataContainer.TOKEN_UNKNOWN, token);

        mDataContainer = null;

        // verify that values in file are reset
        DataContainer newDataContainer = new DataContainer(mContext, mSlotId, subId);

        version = newDataContainer.getVersion();
        validity = newDataContainer.getValidity();
        token = newDataContainer.getToken();

        assertEquals(DataContainer.VERSION_UNKNOWN, version);
        assertEquals(DataContainer.VALIDITY_UNKNOWN, validity);
        assertEquals(DataContainer.TOKEN_UNKNOWN, token);
    }
}
