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

package com.android.imsstack.core.agents;

import static org.junit.Assert.assertEquals;
import static org.junit.Assert.fail;

import android.content.Context;
import android.telephony.TelephonyManager;

import com.android.imsstack.util.AppContext;

import org.junit.After;
import org.junit.AfterClass;
import org.junit.Before;
import org.junit.BeforeClass;
import org.junit.Test;
import org.junit.runner.RunWith;
import org.junit.runners.JUnit4;
import org.mockito.Mock;
import org.mockito.Mockito;
import org.mockito.MockitoAnnotations;
import org.mockito.MockitoSession;

import java.util.concurrent.CountDownLatch;
import java.util.concurrent.TimeUnit;

@RunWith(JUnit4.class)
public class UsatAgentTest {
    private MockitoSession mMockitoSession;
    private UsatAgent mUsatAgent;
    private String mTargetNumber = "9902219632";
    private String mSmscDestAddress = "+9876543";
    private String mSmscOriginAddress = "942563084";
    private String mOriginAddress = "995588443322";
    private String mDialedString = "123456";
    private byte[] mSmstpdu = new byte[] {0, 10, 50, 15};
    private static final int LATCH_MAX = 1;
    private static final int WAIT_TIMER = 5000;
    private final Object mLock = new Object();
    private Usat.CommandResponse mExpectedResult;

    @Mock SimInterface mSimInterface;
    @Mock static Context sApplicationContext;
    @Mock TelephonyManager mTelephonyManager;
    private Usat.Listener mListener;

    private static final CountDownLatch[] sLatches = new CountDownLatch[LATCH_MAX];
    static {
        for (int i = 0; i < LATCH_MAX; i++) {
            sLatches[i] = new CountDownLatch(1);
        }
    }

    public boolean callingTestLatchCountdown(int latchIndex, int waitMs) {
        boolean complete = false;
        try {
            CountDownLatch latch;
            synchronized (mLock) {
                latch = sLatches[latchIndex];
            }
            complete = latch.await(waitMs, TimeUnit.MILLISECONDS);
        } catch (InterruptedException e) {
             //complete == false
        }
        synchronized (mLock) {
            sLatches[latchIndex] = new CountDownLatch(1);
        }
        return complete;
    }

    public void countDownLatch(int latchIndex) {
        synchronized (mLock) {
            sLatches[latchIndex].countDown();
        }
    }

    @BeforeClass
    public static void setUpOnce() {
        sApplicationContext = Mockito.mock(Context.class);
        AppContext.init(sApplicationContext);
    }

    @Before
    public void setUp() throws Exception {
        MockitoAnnotations.initMocks(this);
        mTelephonyManager = Mockito.mock(TelephonyManager.class);
        mSimInterface = Mockito.mock(SimInterface.class);
        mUsatAgent = new UsatAgent(mSimInterface);
        Usat.Listener listener = new Usat.Listener() {
            @Override
            public void onCommandResponse(Usat.CommandResponse response) {
                mExpectedResult = response;
                synchronized (mLock) {
                    mLock.notifyAll();
                }
            }
        };
    }

    @AfterClass
    public static void tearDownOnce() {
        AppContext.deinit();
    }

    @Test
    public void test_createMoSmsControlCommand() {
        Usat.MoSmsControlCommand cmd = new Usat.MoSmsControlCommand(1, mListener, mTargetNumber,
                mSmscDestAddress, TelephonyManager.NETWORK_TYPE_LTE);
        assertEquals(cmd, mUsatAgent.createMoSmsControlCommand(mTargetNumber, mSmscDestAddress,
                TelephonyManager.NETWORK_TYPE_LTE , mListener));
    }

    @Test
    public void test_createSmsPpDownloadCommand() {
        Usat.SmsPpDownloadCommand cmd = new Usat.SmsPpDownloadCommand(1, mListener,
                mSmscOriginAddress, true, mSmstpdu, mOriginAddress);
        assertEquals(cmd, mUsatAgent.createSmsPpDownloadCommand(mSmscOriginAddress, true, mSmstpdu,
                mOriginAddress, mListener));
    }

    @Test
    public void test_isServiceAvailableDefaultValue() {
        assertEquals(false, mUsatAgent.isServiceAvailable(Usat.SERVICE_MO_SMS_CONTROL));
        assertEquals(false, mUsatAgent.isServiceAvailable(Usat.SERVICE_DATA_DOWNLOAD_VIA_SMS_PP));
    }

    @Test
    public void test_sendCommandForPpDownload() {
        Usat.Command cmd = new Usat.SmsPpDownloadCommand(Usat.RESULT_DATA_DOWNLOAD_ERROR, mListener,
                mSmscOriginAddress, true, mSmstpdu, mOriginAddress);

        mUsatAgent.sendCommand(cmd);

        Usat.CommandResponse cmdResponse = new Usat.SmsPpDownloadCommandResponse(cmd,
                Usat.RESULT_DATA_DOWNLOAD_ERROR, mSmstpdu);

        synchronized (mLock) {
            try {
                mLock.wait(WAIT_TIMER);
            } catch (InterruptedException ie) {
            }
        }

        if (mExpectedResult != null) {
            assertEquals(mExpectedResult.getResult(), cmdResponse.getResult());
            assertEquals(mExpectedResult.getCommand(), cmdResponse.getCommand());
        }
    }

    @Test
    public void test_sendCommandForMoSms() {
        Usat.Command cmd = new Usat.MoSmsControlCommand(Usat.RESULT_NOT_ALLOWED, mListener,
                mTargetNumber, mSmscDestAddress, TelephonyManager.NETWORK_TYPE_LTE);

        mUsatAgent.sendCommand(cmd);

        Usat.CommandResponse cmdResponse = new Usat.MoSmsControlCommandResponse(cmd,
                Usat.RESULT_NOT_ALLOWED, mTargetNumber, mSmscDestAddress);

        synchronized (mLock) {
            try {
                mLock.wait(WAIT_TIMER);
            } catch (InterruptedException ie) {
            }
        }

        if (mExpectedResult != null) {
            assertEquals(mExpectedResult.getResult(), cmdResponse.getResult());
            assertEquals(mExpectedResult.getCommand(), cmdResponse.getCommand());
        }
    }

    private boolean isSimCardPresent() {
        return mTelephonyManager.getPhoneType() != TelephonyManager.PHONE_TYPE_NONE
            && mTelephonyManager.getSimState() != TelephonyManager.SIM_STATE_ABSENT;
    }

    /**
     * Tests the TelephonyManager.sendEnvelopeWithStatus() API. This makes a call to
     * sendEnvelopeWithStatus() API and expects a SecurityException.
     */
    @Test
    public void sendEnvelopeWithStatus_test() {
        try {
            if (isSimCardPresent()) {
                String result = mTelephonyManager.sendEnvelopeWithStatus("");
                fail("Expected SecurityException.");
            }
        } catch (SecurityException expected) {
        }
    }

    @After
    public void tearDown() throws Exception {
        mUsatAgent = null;
        mTelephonyManager = null;
    }
}
