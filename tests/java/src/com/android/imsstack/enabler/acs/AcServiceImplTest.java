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
import static org.junit.Assert.assertNotEquals;
import static org.junit.Assert.assertNotNull;
import static org.junit.Assert.assertTrue;
import static org.mockito.ArgumentMatchers.any;
import static org.mockito.ArgumentMatchers.anyBoolean;
import static org.mockito.ArgumentMatchers.anyInt;
import static org.mockito.ArgumentMatchers.anyString;
import static org.mockito.ArgumentMatchers.eq;
import static org.mockito.Mockito.doNothing;
import static org.mockito.Mockito.doReturn;
import static org.mockito.Mockito.never;
import static org.mockito.Mockito.times;
import static org.mockito.Mockito.verify;
import static org.mockito.Mockito.verifyNoMoreInteractions;

import android.content.Context;
import android.os.Handler;
import android.os.HandlerThread;
import android.testing.TestableLooper;

import androidx.test.filters.SmallTest;
import androidx.test.platform.app.InstrumentationRegistry;

import com.android.imsstack.enabler.acs.impl.AcServiceImpl;
import com.android.imsstack.enabler.acs.impl.CallbackManager;
import com.android.imsstack.enabler.acs.impl.ConfigContainer;
import com.android.imsstack.enabler.acs.impl.IAcServiceImplCallback;
import com.android.imsstack.enabler.acs.impl.ProvisioningData;
import com.android.imsstack.enabler.acs.impl.RetryManager;

import org.junit.After;
import org.junit.Before;
import org.junit.Test;
import org.mockito.Mock;
import org.mockito.MockitoAnnotations;

import java.io.File;
import java.io.FileOutputStream;
import java.io.IOException;

public class AcServiceImplTest {
    private static final String AC_DATA = "<?xml version=\"1.0\"?>"
            + "<wap-provisioningdoc version=\"1.1\">"
            + "<unknowntag type=\"VERS\">"
            + "<parm name=\"version\" value=\"1\"/>"
            + "<parm name=\"validity\" value=\"2160000\"/>"
            + "</unknowntag>"
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
    private static final String LOCAL_FILE_NAME_PREFIX = "rcs_provisioning_";
    private static final String LOCAL_FILE_NAME_POSTFIX = ".xml";

    private static final File FILE_DESCRIPTOR =
            InstrumentationRegistry.getInstrumentation().getContext().getFilesDir();
    private static final int SLOT_ID0 = 0;
    private static final int SUB_ID0 = 123;
    private static final int SLOT_ID1 = 1;
    private static final int SUB_ID1 = 456;

    // AcServiceClientInfo
    private static final String VERSION = "6.0";
    private static final String PROFILE = "UP_1.0";
    private static final String CLIENT_VENDOR = "Google";
    private static final String CLIENT_VERSION = "1.0";
    private static final boolean ENABLED_BY_USER = true;

    @Mock Context mContext;
    @Mock ProvisioningData mProvisioningData;
    @Mock CallbackManager mCallbackManager;
    @Mock ConfigContainer mConfigContainer;
    @Mock RetryManager mRetryManager;
    @Mock IAcServiceImplCallback mIAcServiceImplCallback;

    private Handler mHandler;
    private HandlerThread mHandlerThread;
    private TestableLooper mLooper;

    private AcServiceImpl mAcServiceImpl;

    @Before
    public void setUp() throws Exception {
        MockitoAnnotations.initMocks(this);

        doReturn(FILE_DESCRIPTOR).when(mContext).getFilesDir();

        // CallbackManager
        doReturn(true).when(mCallbackManager).registerCallback(any());
        doNothing().when(mCallbackManager).unregisterCallback(any());
        doNothing().when(mCallbackManager).notifyOnReceivedProvisioning(any(), anyBoolean());
        doNothing().when(mCallbackManager).notifyOnReceivedPreProvisioning(any());
        doNothing().when(mCallbackManager).notifyOnReceivedError(anyInt(), anyString());

        // ProvisioningData
        doReturn(AC_DATA.getBytes()).when(mProvisioningData).compressGzip(any());
        doReturn(AC_DATA.getBytes()).when(mProvisioningData).decompressGzip(any());
        doReturn(true).when(mProvisioningData).createXmlFileFromBytes(any());

        // ConfigContainer
        doNothing().when(mConfigContainer).updateClientInfo(any());
        doNothing().when(mConfigContainer).resetAcValue();
        doNothing().when(mConfigContainer).resetClientInfo();

        // IAcServiceImplCallback

        mHandlerThread = new HandlerThread(AcServiceImplTest.class.getSimpleName());
        mHandlerThread.start();

        //mHandler = mAcServiceImpl.getHandler();
        mLooper = new TestableLooper(mHandlerThread.getLooper());
    }

    @After
    public void tearDown() throws Exception {
        if (mLooper != null) {
            mLooper.destroy();
            mLooper = null;
        }

        mHandlerThread = null;
    }

    @Test
    @SmallTest
    public void createObject_withMultiSim() throws Exception {
        AcServiceImpl mAcServiceImpl0 = AcServiceImpl.getInstance(mContext, SLOT_ID0, SUB_ID0);
        AcServiceImpl mAcServiceImpl1 = AcServiceImpl.getInstance(mContext, SLOT_ID1, SUB_ID1);
        assertNotEquals(mAcServiceImpl0, mAcServiceImpl1);


        AcServiceImpl newAcServiceImpl0 = AcServiceImpl.getInstance(mContext, SLOT_ID0, SUB_ID0);
        // verify if two objects are same comes from same slotId 0
        assertEquals(mAcServiceImpl0, newAcServiceImpl0);

        AcServiceImpl newAcServiceImpl1 = AcServiceImpl.getInstance(mContext, SLOT_ID1, SUB_ID1);
        // verify if two objects are same comes from same slotId 1
        assertEquals(mAcServiceImpl1, newAcServiceImpl1);
    }

    @Test
    @SmallTest
    public void setRemoveCallback_withCallback() throws Exception {
        mAcServiceImpl = createTarget(SLOT_ID0, SUB_ID0);
        assertNotNull(mAcServiceImpl);

        assertTrue(mAcServiceImpl.setCallback(mIAcServiceImplCallback));
        verify(mCallbackManager, times(1)).registerCallback(eq(mIAcServiceImplCallback));

        mAcServiceImpl.removeCallback(mIAcServiceImplCallback);
        verify(mCallbackManager, times(1)).unregisterCallback(eq(mIAcServiceImplCallback));

        verifyNoMoreInteractions(mCallbackManager);

    }

    @Test
    @SmallTest
    public void setClientInfo_test() throws Exception {
        mAcServiceImpl = createTarget(SLOT_ID0, SUB_ID0);
        assertNotNull(mAcServiceImpl);

        AcServiceClientInfo acServiceClientInfo = new AcServiceClientInfo(
                VERSION, PROFILE, CLIENT_VENDOR, CLIENT_VERSION, ENABLED_BY_USER);

        assertTrue(mAcServiceImpl.setClientInfo(acServiceClientInfo));
        verify(mConfigContainer, times(1)).updateClientInfo(eq(acServiceClientInfo));

        verifyNoMoreInteractions(mConfigContainer);
    }

    @Test
    @SmallTest
    public void notifyProvisioningReceived_test() throws Exception {
        mAcServiceImpl = createTarget(SLOT_ID0, SUB_ID0);
        assertNotNull(mAcServiceImpl);

        mAcServiceImpl.notifyProvisioningReceived(AC_DATA.getBytes(), false);

        processAllMessages();

        // verify update ProvisioningData
        verify(mProvisioningData, never()).decompressGzip(any());
        verify(mProvisioningData, times(1)).createXmlFileFromBytes(any());

        // verify update ConfigContainer
        verify(mConfigContainer, times(1)).resetAcValue();
        verify(mConfigContainer, times(1)).resetClientInfo();

        verifyNoMoreInteractions(mConfigContainer);

        // TODO : need to check timer
    }

    @Test
    @SmallTest
    public void notifyProvisioningRemoved_test() throws Exception {
        mAcServiceImpl = createTarget(SLOT_ID0, SUB_ID0);
        assertNotNull(mAcServiceImpl);

        mAcServiceImpl.notifyProvisioningRemoved();

        processAllMessages();

        // verify update ProvisioningData
        verify(mProvisioningData, times(1)).deleteXmlFile();

        // verify update ConfigContainer
        verify(mConfigContainer, times(1)).resetAcValue();
        verify(mConfigContainer, times(1)).resetClientInfo();

        verifyNoMoreInteractions(mConfigContainer);

        // TODO : need to check timer
    }

    private AcServiceImpl createTarget(int slotId, int subId) {
        return new AcServiceImpl(slotId, subId, mContext, mHandlerThread.getLooper(),
                mProvisioningData, mCallbackManager, mConfigContainer, mRetryManager);
    }

    private void processAllMessages() {
        while (!mLooper.getLooper().getQueue().isIdle()) {
            mLooper.processAllMessages();
        }
    }

    private boolean isExistTestFile(String fileName) {
        File srcFile = new File(mContext.getFilesDir(), fileName);
        return srcFile.exists();
    }

    private void createTestFile(String fileName) {
        try {
            File file = new File(mContext.getFilesDir(), fileName);
            FileOutputStream outputStream = new FileOutputStream(file);
            outputStream.write(AC_DATA.getBytes());
            outputStream.close();
        } catch (IOException e) {
            throw new AssertionError();
        }
    }

    private void deleteTestFile(String fileName) {
        File file = new File(mContext.getFilesDir(), fileName);
        if (file != null && file.exists()) {
            file.delete();
        }
    }
}
