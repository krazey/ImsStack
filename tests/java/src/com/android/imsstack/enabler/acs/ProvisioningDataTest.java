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
import static org.junit.Assert.assertFalse;
import static org.junit.Assert.assertNull;
import static org.junit.Assert.assertTrue;
import static org.mockito.Mockito.doReturn;

import android.content.Context;
import android.os.Environment;
import android.test.suitebuilder.annotation.SmallTest;

import androidx.test.InstrumentationRegistry;

import com.android.imsstack.enabler.acs.impl.ProvisioningData;

import org.junit.After;
import org.junit.Before;
import org.junit.Test;
import org.mockito.Mock;
import org.mockito.MockitoAnnotations;

import java.io.File;
import java.util.Arrays;

public class ProvisioningDataTest {
    private static final String TAG = ProvisioningData.class.getSimpleName();
    private static final File FILE_DESCRIPTOR = InstrumentationRegistry.getContext().getFilesDir();
    // path : /storage/emulated/0/Download
    private static final String FILE_DESCRIPTOR_EXTERNAL =
            Environment.getExternalStorageDirectory() + "/Download";
    private static final String LOCAL_FILE_NAME_PREFIX = "rcs_provisioning_";
    private static final String LOCAL_FILE_NAME_POSTFIX = ".xml";
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

    private static final String AC_DATA_PARTIAL = "<?xml version=\"1.0\"?>"
            + "<wap-provisioningdoc version=\"1.1\">"
            + "<characteristic type=\"VERS\">"
            + "<parm name=\"version\" value=\"3\"/>"
            + "<parm name=\"validity\" value=\"3600000\"/>"
            + "</characteristic>"
            + "<characteristic type=\"TOKEN\">"
            + "<parm name=\"token\" value=\"qazwsxedc12345\"/>"
            + "</characteristic>"
            + "<characteristic type=\"APPLICATION\">"
            + "<characteristic type=\"CPM\">"
            + "<characteristic type=\"StandaloneMsg\">"
            + "<parm name=\"MaxSizeStandalone\" value=\"204800\"/>"
            + "</characteristic>"
            + "</characteristic>"
            + "<characteristic type=\"CAPDISCOVERY\">"
            + "<parm name=\"capInfoExpiry\" value=\"3600\"/>"
            + "<parm name=\"nonRCScapInfoExpiry\" value=\"123456789\"/>"
            + "</characteristic>"
            + "</characteristic>"
            + "</wap-provisioningdoc>";
    @Mock
    Context mContext;

    private int mSubId = 1234;
    private ProvisioningData mProvisioningData;

    @Before
    public void setUp() throws Exception {
        MockitoAnnotations.initMocks(this);

        doReturn(FILE_DESCRIPTOR).when(mContext).getFilesDir();
        //doReturn(new File(FILE_DESCRIPTOR_EXTERNAL)).when(mContext).getFilesDir();
    }

    @After
    public void tearDown() throws Exception {
        // delete test file
        ProvisioningData.deleteXmlFile(mContext, mSubId);
        mProvisioningData = null;
/*
        String fileName = LOCAL_FILE_NAME_PREFIX + mSubId + LOCAL_FILE_NAME_POSTFIX;
        File file = new File(mContext.getFilesDir(), fileName);
        if (file != null && file.exists()) {
            file.delete();
        }*/
    }

    @Test
    @SmallTest
    public void parseAndSaveProvisioningData_withByteArray() throws Exception {
        mProvisioningData = new ProvisioningData(mContext, mSubId, AC_DATA.getBytes());
        assertEquals(true, mProvisioningData.isComplete());

        mProvisioningData.dumpLog();

        mProvisioningData.saveXmlFile();
        String fileName = mProvisioningData.getFileName();

        assertEquals(true, isExistTestFile(fileName));
    }

    @Test
    @SmallTest
    public void parseAndGetProvisioningData_withByteArray() throws Exception {
        mProvisioningData = new ProvisioningData(mContext, mSubId, AC_DATA.getBytes());
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
        mProvisioningData = new ProvisioningData(mContext, mSubId, AC_DATA.getBytes());

        assertTrue(mProvisioningData.isComplete());

        mProvisioningData.saveXmlFile();
        String fileName = mProvisioningData.getFileName();

        assertTrue(isExistTestFile(fileName));

        // read data from xml file
        ProvisioningData fileProvisioningData = new ProvisioningData(mContext, mSubId);

        int intValue = fileProvisioningData.getIntValue("TimerIdle", 0);
        assertEquals(660, intValue);

        long longValue = fileProvisioningData.getLongValue("MaxSizeFileTr", 0L);
        assertEquals(102400L, longValue);

        String stringValue = fileProvisioningData.getStringValue("NMS_URL", "");
        assertEquals("vcnms-c2s.enc.att.net", stringValue);
    }

    @Test
    @SmallTest
    public void updateProvisioningData_withPartialData() throws Exception {
        // write data to xml file
        mProvisioningData = new ProvisioningData(mContext, mSubId, AC_DATA.getBytes());
        assertEquals(true, mProvisioningData.isComplete());

        ProvisioningData newProvisioningData =
                new ProvisioningData(mContext, mSubId, AC_DATA_PARTIAL.getBytes());
        assertEquals(true, newProvisioningData.isComplete());

        mProvisioningData.updateData(newProvisioningData);

        mProvisioningData.dumpLog();

        int intValue = mProvisioningData.getIntValue("version", 0);
        assertEquals(3, intValue);

        long longValue = mProvisioningData.getLongValue("validity", 0L);
        assertEquals(3600000L, longValue);

        String stringValue = mProvisioningData.getStringValue("token", "");
        assertEquals("qazwsxedc12345", stringValue);

        longValue = mProvisioningData.getLongValue("MaxSizeStandalone", 0L);
        assertEquals(204800L, longValue);

        longValue = mProvisioningData.getLongValue("capInfoExpiry", 0L);
        assertEquals(3600L, longValue);

        longValue = mProvisioningData.getLongValue("nonRCScapInfoExpiry", 0L);
        assertEquals(123456789L, longValue);

        mProvisioningData.saveXmlFile();
        String fileName = mProvisioningData.getFileName();

        assertTrue(isExistTestFile(fileName));
    }

    @Test
    @SmallTest
    public void compressAndDecompress_withData() {
        byte[] compressedData = ProvisioningData.compressGzip(AC_DATA.getBytes());
        assertFalse(Arrays.equals(compressedData, AC_DATA.getBytes()));

        byte[] decompressedData = ProvisioningData.decompressGzip(compressedData);
        assertTrue(Arrays.equals(decompressedData, AC_DATA.getBytes()));

        assertNull(ProvisioningData.compressGzip(null));
        assertNull(ProvisioningData.decompressGzip(null));
    }

    @Test
    @SmallTest
    public void createDeleteXmlFile_withData() {
        boolean result = ProvisioningData.createXmlFileFromBytes(
                mContext, mSubId, AC_DATA.getBytes());
        assertTrue(result);

        String fileName = LOCAL_FILE_NAME_PREFIX + mSubId + LOCAL_FILE_NAME_POSTFIX;
        assertTrue(isExistTestFile(fileName));

        ProvisioningData.deleteXmlFile(mContext, mSubId);
        assertFalse(isExistTestFile(fileName));
    }

    private boolean isExistTestFile(String fileName) {
        File srcFile = new File(mContext.getFilesDir(), fileName);
        return srcFile.exists();
    }
}
