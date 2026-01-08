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
import static org.junit.Assert.assertTrue;
import static org.mockito.ArgumentMatchers.any;
import static org.mockito.ArgumentMatchers.anyBoolean;
import static org.mockito.ArgumentMatchers.anyInt;
import static org.mockito.ArgumentMatchers.eq;
import static org.mockito.Mockito.doNothing;
import static org.mockito.Mockito.times;
import static org.mockito.Mockito.verify;
import static org.mockito.Mockito.verifyNoMoreInteractions;
import static org.mockito.Mockito.when;

import android.content.Context;

import androidx.test.filters.SmallTest;

import com.android.imsstack.enabler.acs.impl.AcServiceImpl;
import com.android.imsstack.enabler.acs.impl.IAcServiceImplCallback;

import org.junit.After;
import org.junit.Before;
import org.junit.Test;
import org.mockito.Mock;
import org.mockito.MockitoAnnotations;

import java.nio.charset.StandardCharsets;
import java.util.concurrent.Executor;

public class AcServiceTest {
    private static final int TEST_STATE = 2;
    private static final String VERSION = "6.0";
    private static final String PROFILE = "UP_1.0";
    private static final String CLIENT_VENDOR = "Google";
    private static final String CLIENT_VERSION = "1.0";
    private static final boolean ENABLED_BY_USER = true;
    private static final String PROVISIONING_DATA = "<?xml version=\"1.0\"?>"
            + "<wap-provisioningdoc version=\"1.1\">"
            + "<characteristic type=\"VERS\">"
            + "<parm name=\"version\" value=\"3\"/>"
            + "<parm name=\"validity\" value=\"3600000\"/>"
            + "</characteristic>"
            + "</wap-provisioningdoc>";

    private static final String PRE_PROVISIONING_DATA = "<?xml version=\"1.0\"?>"
            + "<wap-provisioningdoc version=\"1.1\">"
            + "<characteristic type=\"TOKEN\">"
            + "<parm name=\"token\" value=\"1234567890\"/>"
            + "</characteristic>"
            + "</wap-provisioningdoc>";

    private static final int SLOT_ID0 = 0;
    private static final int SLOT_ID1 = 1;

    private static class TestAcServiceCallback extends AcService.AcServiceCallback {
        private int mErrorCode;
        private String mErrorString;
        byte[] mProvisioning;
        boolean mIsDeProvision;
        byte[] mPreProvisioning;

        TestAcServiceCallback() {
            mErrorCode = 0;
            mErrorString = "";
            mProvisioning = null;
            mIsDeProvision = false;
            mPreProvisioning = null;
        }

        public void onReceivedProvisioning(byte[] data, boolean isDeProvision) {
            mProvisioning = data;
            mIsDeProvision = isDeProvision;
        }

        public void onReceivedPreProvisioning(byte[] data) {
            mPreProvisioning = data;
        }
        public void onReceivedError(int errorCode, String errorString) {
            mErrorCode = errorCode;
            mErrorString = errorString;
        }

        private byte[] getProvisioning() {
            return mProvisioning;
        }

        private boolean getDeProvisioning() {
            return mIsDeProvision;
        }

        private byte[] getPreProvisioning() {
            return mPreProvisioning;
        }

        private int getErrorCode() {
            return mErrorCode;
        }

        private String getErrorString() {
            return mErrorString;
        }
    }


    @Mock Context mContext;
    @Mock AcServiceImpl mAcServiceImpl0;
    @Mock AcServiceImpl mAcServiceImpl1;

    private Executor mExecutor = new Executor() {
        @Override
        public void execute(Runnable r) {
            r.run();
        }
    };

    private TestAcServiceCallback mTestAcServiceCallback0;
    private TestAcServiceCallback mTestAcServiceCallback1;
    private AcService mAcService0;
    private AcService mAcService1;

    @Before
    public void setUp() throws Exception {
        MockitoAnnotations.initMocks(this);

        when(mAcServiceImpl0.setCallback(any())).thenReturn(true);
        doNothing().when(mAcServiceImpl0).removeCallback(any());

        when(mAcServiceImpl0.start(anyInt())).thenReturn(true);
        doNothing().when(mAcServiceImpl0).stop();
        doNothing().when(mAcServiceImpl0).notifyProvisioningReceived(any(), anyBoolean());
        doNothing().when(mAcServiceImpl0).notifyProvisioningRemoved();
        when(mAcServiceImpl0.getState()).thenReturn(TEST_STATE);

        when(mAcServiceImpl1.setCallback(any())).thenReturn(true);
        doNothing().when(mAcServiceImpl1).removeCallback(any());

        mTestAcServiceCallback0 = new TestAcServiceCallback();
        mTestAcServiceCallback1 = new TestAcServiceCallback();

        mAcService0 = AcService.getInstance(SLOT_ID0, mAcServiceImpl0);
        mAcService1 = AcService.getInstance(SLOT_ID1, mAcServiceImpl1);
    }

    @After
    public void tearDown() throws Exception {
        mAcService0 = null;
        mAcService1 = null;

        mTestAcServiceCallback0 = null;
        mTestAcServiceCallback1 = null;

        AcService.releaseInstance(SLOT_ID0);
        AcService.releaseInstance(SLOT_ID1);
    }

    @Test
    @SmallTest
    public void createObject_withMultiSim() throws Exception {
        assertNotEquals(mAcService0, mAcService1);

        AcService newAcService0 = AcService.getInstance(SLOT_ID0, null);
        // verify if two objects comes from same slotId 0 are same
        assertEquals(mAcService0, newAcService0);

        AcService newAcService1 = AcService.getInstance(SLOT_ID1, null);
        // verify if two objects comes from same slotId 1 are same
        assertEquals(mAcService1, newAcService1);
    }

    @Test
    @SmallTest
    public void setRemoveCallback_test() throws Exception {
        // SIM slot 0
        mAcService0 = AcService.getInstance(SLOT_ID0, mAcServiceImpl0);

        assertTrue(mAcService0.setCallback(mExecutor, mTestAcServiceCallback0));
        verify(mAcServiceImpl0, times(1)).setCallback(eq(mTestAcServiceCallback0.getCallback()));

        mAcService0.removeCallback(mTestAcServiceCallback0);
        verify(mAcServiceImpl0, times(1))
                .removeCallback(eq(mTestAcServiceCallback0.getCallback()));

        verifyNoMoreInteractions(mAcServiceImpl0);

        // SIM slot 1
        assertTrue(mAcService1.setCallback(mExecutor, mTestAcServiceCallback1));
        verify(mAcServiceImpl1, times(1)).setCallback(eq(mTestAcServiceCallback1.getCallback()));

        mAcService1.removeCallback(mTestAcServiceCallback1);
        verify(mAcServiceImpl1, times(1))
                .removeCallback(eq(mTestAcServiceCallback1.getCallback()));

        verifyNoMoreInteractions(mAcServiceImpl1);
    }

    @Test
    @SmallTest
    public void otherInterface_test() throws Exception {
        AcServiceClientInfo acServiceClientInfo = new AcServiceClientInfo(
                VERSION, PROFILE, CLIENT_VENDOR, CLIENT_VERSION, ENABLED_BY_USER);

        assertTrue(mAcService0.setClientInfo(acServiceClientInfo));
        verify(mAcServiceImpl0, times(1)).setClientInfo(eq(acServiceClientInfo));

        assertTrue(mAcService0.start());
        verify(mAcServiceImpl0, times(1)).start(anyInt());

        mAcService0.stop();
        verify(mAcServiceImpl0, times(1)).stop();

        mAcService0.notifyProvisioningReceived(PROVISIONING_DATA.getBytes(), false);
        verify(mAcServiceImpl0, times(1)).notifyProvisioningReceived(
                eq(PROVISIONING_DATA.getBytes()), eq(false));

        mAcService0.notifyProvisioningRemoved();
        verify(mAcServiceImpl0, times(1)).notifyProvisioningRemoved();

        assertEquals(TEST_STATE, mAcService0.getState());
        verify(mAcServiceImpl0, times(1)).getState();

        verifyNoMoreInteractions(mAcServiceImpl0);
    }

    @Test
    @SmallTest
    public void callbackOnReceivedProvisioning_test() throws Exception {
        mAcService0 = AcService.getInstance(SLOT_ID0, mAcServiceImpl0);
        assertTrue(mAcService0.setCallback(mExecutor, mTestAcServiceCallback0));
        verify(mAcServiceImpl0, times(1)).setCallback(eq(mTestAcServiceCallback0.getCallback()));

        IAcServiceImplCallback iAcServiceImplCallback = mTestAcServiceCallback0.getCallback();

        boolean deProvisioning = true;
        iAcServiceImplCallback.onReceivedProvisioning(
                PROVISIONING_DATA.getBytes(StandardCharsets.UTF_8), deProvisioning);

        String actual = new String(
                mTestAcServiceCallback0.getProvisioning(), StandardCharsets.UTF_8);
        assertEquals(PROVISIONING_DATA, actual);
        assertEquals(deProvisioning, mTestAcServiceCallback0.getDeProvisioning());
    }

    @Test
    @SmallTest
    public void callbackOnReceivedPreProvisioning_test() throws Exception {
        mAcService0 = AcService.getInstance(SLOT_ID0, mAcServiceImpl0);
        assertTrue(mAcService0.setCallback(mExecutor, mTestAcServiceCallback0));
        verify(mAcServiceImpl0, times(1)).setCallback(eq(mTestAcServiceCallback0.getCallback()));

        IAcServiceImplCallback iAcServiceImplCallback = mTestAcServiceCallback0.getCallback();

        iAcServiceImplCallback.onReceivedPreProvisioning(
                PRE_PROVISIONING_DATA.getBytes(StandardCharsets.UTF_8));

        String actual = new String(
                mTestAcServiceCallback0.getPreProvisioning(), StandardCharsets.UTF_8);
        assertEquals(PRE_PROVISIONING_DATA, actual);
    }

    @Test
    @SmallTest
    public void callbackOnReceivedError_test() throws Exception {
        mAcService0 = AcService.getInstance(SLOT_ID0, mAcServiceImpl0);
        assertTrue(mAcService0.setCallback(mExecutor, mTestAcServiceCallback0));
        verify(mAcServiceImpl0, times(1)).setCallback(eq(mTestAcServiceCallback0.getCallback()));

        IAcServiceImplCallback iAcServiceImplCallback = mTestAcServiceCallback0.getCallback();

        int errorCode = 403;
        String errorString = "Forbidden";
        iAcServiceImplCallback.onReceivedError(errorCode, errorString);

        assertEquals(errorCode, mTestAcServiceCallback0.getErrorCode());
        assertTrue(errorString.equals(mTestAcServiceCallback0.getErrorString()));
    }
}
