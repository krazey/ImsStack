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

package com.android.imsstack.imsservice.mmtel.sms;

import static org.junit.Assert.assertEquals;
import static org.junit.Assert.assertNotNull;
import static org.mockito.ArgumentMatchers.eq;
import static org.mockito.Mockito.timeout;
import static org.mockito.Mockito.verify;

import android.os.HandlerThread;
import android.telephony.SmsManager;
import android.telephony.ims.stub.ImsSmsImplBase;

import com.android.imsstack.enabler.mts.MtsController;
import com.android.imsstack.imsservice.mmtel.ImsCallContext;

import org.junit.After;
import org.junit.Before;
import org.junit.Test;
import org.junit.runner.RunWith;
import org.junit.runners.JUnit4;
import org.mockito.ArgumentCaptor;
import org.mockito.Mock;
import org.mockito.Mockito;

@RunWith(JUnit4.class)
public class SmsTransferLayerTest {
    private static final String TAG = "[GII-SmsTL Test] ";
    @Mock ImsCallContext mImsCallContext;
    @Mock SmsTransferLayer.Listener mListener;
    @Mock SmsRelayLayer mSmsRL;
    @Mock MtsController.Listener mMockMtsControllerListener = new MtsController.Listener();

    private static final long MAX_WAIT_TIME_MS = 10000;
    //private SmsTransferLayer mSmsTransferLayer;
    private TestSmsTransferLayer mSmsTransferLayer;
    private SmsTransferLayer.SmsRLListenerProxy mProxyListener;
    private int mToken = 1;
    private int mSmsFormat = SmsUtils.FORMAT_INT_3GPP;
    private int mMessageRef = 1;
    private String mSmsc = "+19037029920";
    private String mDestinationAddress = "90008000*100";
    private byte[] mPdu = { 21, 11, (byte) 0x0A, 81, 78, 56, 34, 12, 10, 00, 00, 06, 66,
            (byte) 0xB2, 99, (byte) 0x6C, 26, 03 };
    private byte[] mTpdu = {01, 06, (byte) 0x0C, (byte) 0x81, (byte) 0x09, 00, (byte) 0x08, 00, 26,
            00, 00, 00, 06, 53, (byte) 0x6A,  (byte) 0x90, (byte) 0x5A, (byte) 0x9D, 02};
    private int mTlMessageType = SmsUtils.TP_SMS_DELIVER;
    private int mResult = ImsSmsImplBase.SEND_STATUS_OK;
    private int mRpMessageType = SmsUtils.RP_DATA;
    private int mReason = SmsManager.RESULT_ERROR_NONE;
    private HandlerThread mHt;
    private boolean mReady = false;
    private Object mLock = new Object();

    @Before
    public void setUp() throws Exception {
        mImsCallContext = Mockito.mock(ImsCallContext.class);
        mSmsRL = Mockito.mock(SmsRelayLayer.class);
        mSmsTransferLayer = new TestSmsTransferLayer(mImsCallContext, mSmsRL);
        mListener = Mockito.mock(SmsTransferLayer.Listener.class);
        mSmsTransferLayer.setListener(mListener);
    }

    private SmsRelayLayer.Listener setupListener() {
        ArgumentCaptor<SmsRelayLayer.Listener> callbackArg =
                ArgumentCaptor.forClass(SmsRelayLayer.Listener.class);
        verify(mSmsRL).setListener(callbackArg.capture());
        SmsRelayLayer.Listener mListener = callbackArg.getValue();
        assertNotNull(mListener);
        return mListener;
    }

    @Test
    public void test_sendMoTPdu() {
        assertEquals(SmsUtils.SMSTL_RESULT_EXCEPTION, mSmsTransferLayer.sendMoTPdu(mToken,
                mSmsFormat, mMessageRef, mSmsc, mPdu));
        mSmsTransferLayer.sendMoTPdu(mToken, SmsUtils.RP_ACK, mMessageRef, mSmsc, mPdu);
        mSmsRL.sendRPMessage(mToken, SmsUtils.RP_ACK, mSmsc, null, mPdu);
        verify(mSmsRL).sendRPMessage(eq(mToken), eq(SmsUtils.RP_ACK), eq(mSmsc), eq(null),
                eq(mPdu));
    }

    @Test
    public void test_sendMoTPduSuccess() {
        mSmsTransferLayer.sendMoTPdu(1, mSmsFormat, mMessageRef, mSmsc, mTpdu);
        mSmsTransferLayer.sendMoTPdu(2, mSmsFormat, mMessageRef, mSmsc, mTpdu);
        mSmsTransferLayer.sendMoTPdu(3, mSmsFormat, mMessageRef, mSmsc, mTpdu);
        mSmsTransferLayer.sendMoTPdu(4, mSmsFormat, mMessageRef, mSmsc, mTpdu);
        mProxyListener = mSmsTransferLayer.getListener();
        verify(mSmsRL).sendRPMessage(eq(1), eq(mRpMessageType), eq(mSmsc), eq(mDestinationAddress),
                eq(mTpdu));
        mProxyListener.notifyRLReportIndication(1, ImsSmsImplBase.SEND_STATUS_OK,
                SmsManager.RESULT_ERROR_NONE);
        verify(mSmsRL, timeout(1000).times(1)).sendRPMessage(eq(2), eq(mRpMessageType), eq(mSmsc),
                eq(mDestinationAddress), eq(mTpdu));
        mProxyListener.notifyRLReportIndication(2, ImsSmsImplBase.SEND_STATUS_OK,
                SmsManager.RESULT_ERROR_NONE);
        verify(mSmsRL).sendRPMessage(eq(3), eq(mRpMessageType), eq(mSmsc), eq(mDestinationAddress),
                eq(mTpdu));
        mProxyListener.notifyRLReportIndication(3, ImsSmsImplBase.SEND_STATUS_OK,
                SmsManager.RESULT_ERROR_NONE);
        verify(mSmsRL).sendRPMessage(eq(4), eq(mRpMessageType), eq(mSmsc), eq(mDestinationAddress),
                eq(mTpdu));
    }

    private class TestSmsTransferLayer extends SmsTransferLayer {
        TestSmsTransferLayer(ImsCallContext callContext, SmsRelayLayer smsRL) {
            super(callContext, smsRL);
        }

        private SmsRLListenerProxy getListener() {
            return mSmsRLListener;
        }
    }

    @Test
    public void test_sendMoTPduFailure() {
        mSmsTransferLayer.sendMoTPdu(mToken, mSmsFormat, mMessageRef, mSmsc, mTpdu);
        int result = mSmsTransferLayer.sendMoTPdu(mToken, mSmsFormat, mMessageRef, mSmsc, mTpdu);
        assertEquals(SmsUtils.SMSTL_RESULT_DUPLICATE_TOKEN, result);
    }

    @Test
    public void test_sendReportTPdu() {
        mSmsTransferLayer.sendReportTPdu(mToken, mTlMessageType, mMessageRef, mResult);
        verify(mSmsRL).sendRPMessage(eq(mToken), eq(SmsUtils.RP_ACK), eq(null), eq(null), eq(null));
    }


    @Test
    public void test_notifyRLDataIndication() {
        mListener.notifySmsReceived(mToken, mSmsFormat, mRpMessageType, mPdu);
        verify(mListener).notifySmsReceived(mToken, mSmsFormat, mRpMessageType, mPdu);
    }

    @Test
    public void test_notifyRLReportIndication() {
        mListener.notifySmsResult(mToken, mResult, mReason);
        verify(mListener, Mockito.times(1)).notifySmsResult(mToken, mResult, mReason);
    }

    @After
    public void tearDown() throws Exception {
        mImsCallContext = null;
        mSmsRL = null;
        mSmsTransferLayer = null;
        mListener = null;
    }
}
