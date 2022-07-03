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
import android.os.Environment;
import android.test.suitebuilder.annotation.SmallTest;
import android.util.Log;

import androidx.test.InstrumentationRegistry;

import com.android.imsstack.enabler.acs.impl.ProvisioningData;

import com.google.common.io.Files;

import org.junit.After;
import org.junit.Before;
import org.junit.Test;
import org.mockito.Mock;
import org.mockito.MockitoAnnotations;

import java.io.File;
import java.io.IOException;
import java.nio.charset.StandardCharsets;

public class ProvisioningDataTest {
    private static final String TAG = ProvisioningData.class.getSimpleName();
    private static final File FILE_DESCRIPTOR = InstrumentationRegistry.getContext().getFilesDir();
    private static final String FILE_NAME = "provisionindData.xml";
    private static final String AC_DATA = "<?xml version=\"1.0\"?>"
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

    @Mock
    Context mContext;

    private ProvisioningData mProvisioningData;

    @Before
    public void setUp() throws Exception {
        MockitoAnnotations.initMocks(this);

        doReturn(FILE_DESCRIPTOR).when(mContext).getFilesDir();
    }

    @After
    public void tearDown() throws Exception {
        mProvisioningData = null;
    }

    @Test
    @SmallTest
    public void parseAndSaveProvisioningData_withByteArray() throws Exception {
        mProvisioningData = new ProvisioningData(AC_DATA.getBytes(StandardCharsets.UTF_8));
        assertEquals(true, mProvisioningData.isComplete());

        mProvisioningData.dumpLog();

        mProvisioningData.saveXmlFile(FILE_DESCRIPTOR, FILE_NAME);
        assertEquals(true, isExistTestFile(FILE_NAME));

        copyFile();
    }

    @Test
    @SmallTest
    public void parseAndGetProvisioningData_withByteArray() throws Exception {
        mProvisioningData = new ProvisioningData(AC_DATA.getBytes(StandardCharsets.UTF_8));
        assertEquals(true, mProvisioningData.isComplete());

        int intValue = mProvisioningData.getIntValue("version", 0);
        assertEquals(1, intValue);

        long longValue = mProvisioningData.getLongValue("validity", 0L);
        assertEquals(2160000L, longValue);

        String stringValue = mProvisioningData.getStringValue("token", "");
        assertEquals("qawsedrftg", stringValue);

        longValue = mProvisioningData.getLongValue("MaxSizeStandalone", 0L);
        assertEquals(1048576, longValue);
    }

    @Test
    @SmallTest
    public void parseAndGetProvisioningData_withFile() throws Exception {
        // write data to xml file
        mProvisioningData = new ProvisioningData(AC_DATA.getBytes(StandardCharsets.UTF_8));
        assertEquals(true, mProvisioningData.isComplete());
        mProvisioningData.saveXmlFile(FILE_DESCRIPTOR, FILE_NAME);
        assertEquals(true, isExistTestFile(FILE_NAME));

        // read data from xml file
        ProvisioningData fileProvisioningData = new ProvisioningData(FILE_DESCRIPTOR, FILE_NAME);

        int intValue = fileProvisioningData.getIntValue("TimerIdle", 0);
        assertEquals(660, intValue);

        long longValue = fileProvisioningData.getLongValue("MaxSizeFileTr", 0L);
        assertEquals(102400L, longValue);

        String stringValue = fileProvisioningData.getStringValue("NMS_URL", "");
        assertEquals("vcnms-c2s.enc.att.net", stringValue);
    }

    private boolean isExistTestFile(String fileName) {
        File srcFile = new File(FILE_DESCRIPTOR, fileName);
        return srcFile.exists();
    }

    private void copyFile() {
        // path : /storage/emulated/0/Download
        String targetPath = Environment.getExternalStorageDirectory() + "/Download";

        File desFile = new File(targetPath, FILE_NAME);
        File srcFile = new File(FILE_DESCRIPTOR, FILE_NAME);
        try {
            Files.copy(srcFile, desFile);
        } catch (IOException e) {
            Log.i(TAG, e.getMessage());
        }

        Log.i(TAG, "test result file copied : " + targetPath + "/" + FILE_NAME);
    }
}
