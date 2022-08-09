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
import static org.junit.Assert.assertTrue;
import static org.mockito.ArgumentMatchers.eq;
import static org.mockito.Mockito.timeout;
import static org.mockito.Mockito.times;
import static org.mockito.Mockito.verify;

import android.os.HandlerThread;
import android.telephony.SmsManager;
import android.telephony.ims.stub.ImsSmsImplBase;

import com.android.imsstack.enabler.mts.MtsController;
import com.android.imsstack.imsservice.mmtel.ImsCallContext;
import com.android.internal.util.HexDump;

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
    //For status result not applicable
    private static final int STATUS_RESULT_NA = 0;
    @Mock ImsCallContext mImsCallContext;
    @Mock SmsTransferLayer.Listener mListener;
    @Mock SmsRelayLayer mSmsRL;
    @Mock MtsController.Listener mMockMtsControllerListener = new MtsController.Listener();
    private static final long MAX_WAIT_TIME_MS = 10000;
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
        mSmsRL.sendRPMessage(mToken, SmsUtils.RP_ACK, mSmsc, null, mPdu, 1);
        verify(mSmsRL).sendRPMessage(eq(mToken), eq(SmsUtils.RP_ACK), eq(mSmsc), eq(null),
                eq(mPdu), eq(1));
    }
    @Test
    public void testCdmaPduError() {
        byte[] newCdmaPdu = mSmsTransferLayer.generateCdmaPdu(null);
        assertEquals(newCdmaPdu.length, 0);
    }
    @Test
    public void testCdmaPduGenerate() {
        String pduString = "0000021002020702A848D159E24006010008"
                         + "2300031010D0011410A48CBB366F418F465C"
                         + "7AF4EECE819E7E1C19000306220707183319";
        byte[] pdu = HexDump.hexStringToByteArray(pduString);
        byte[] newCdmaPdu = mSmsTransferLayer.generateCdmaPdu(pdu);
        assertTrue((newCdmaPdu.length > 0) ? true : false);
    }
    @Test
    public void test_sendMoTPduSuccess() {
        int successCause = 0;
        mSmsTransferLayer.sendMoTPdu(1, mSmsFormat, mMessageRef, mSmsc, mTpdu);
        mSmsTransferLayer.sendMoTPdu(2, mSmsFormat, mMessageRef, mSmsc, mTpdu);
        mSmsTransferLayer.sendMoTPdu(3, mSmsFormat, mMessageRef, mSmsc, mTpdu);
        mSmsTransferLayer.sendMoTPdu(4, mSmsFormat, mMessageRef, mSmsc, mTpdu);
        mProxyListener = mSmsTransferLayer.getListener();
        verify(mSmsRL).sendRPMessage(eq(1), eq(mRpMessageType), eq(mSmsc), eq(mDestinationAddress),
                eq(mTpdu), eq(STATUS_RESULT_NA));
        mProxyListener.notifyRLReportIndication(1, mTpdu[1] & 0xff, ImsSmsImplBase.SEND_STATUS_OK,
                SmsManager.RESULT_ERROR_NONE, successCause);
        verify(mSmsRL, timeout(1000).times(1)).sendRPMessage(eq(2), eq(mRpMessageType), eq(mSmsc),
                eq(mDestinationAddress), eq(mTpdu), eq(STATUS_RESULT_NA));
        mProxyListener.notifyRLReportIndication(2, mTpdu[1] & 0xff, ImsSmsImplBase.SEND_STATUS_OK,
                SmsManager.RESULT_ERROR_NONE, successCause);
        verify(mSmsRL, timeout(1000).times(1)).sendRPMessage(eq(3), eq(mRpMessageType), eq(mSmsc),
                eq(mDestinationAddress), eq(mTpdu), eq(STATUS_RESULT_NA));
        mProxyListener.notifyRLReportIndication(3, mTpdu[1] & 0xff, ImsSmsImplBase.SEND_STATUS_OK,
                SmsManager.RESULT_ERROR_NONE, successCause);
        verify(mSmsRL, timeout(1000).times(1)).sendRPMessage(eq(4), eq(mRpMessageType), eq(mSmsc),
                eq(mDestinationAddress), eq(mTpdu), eq(STATUS_RESULT_NA));
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
        byte[] deliverReportPdu = mSmsTransferLayer.generateDeliverReportPdu(mResult);
        verify(mSmsRL).sendRPMessage(eq(mToken), eq(SmsUtils.RP_ACK), eq(null), eq(null),
                eq(deliverReportPdu), eq(mResult));
    }

    @Test
    public void test_notifyRLDataIndication() {
        mListener.notifySmsReceived(mToken, mSmsFormat, mRpMessageType, mPdu);
        verify(mListener).notifySmsReceived(mToken, mSmsFormat, mRpMessageType, mPdu);
    }
    @Test
    public void test_notifyRLDataIndication_3GPP2() {
        String pduString = "0000021002020702A848D159E24006010008"
                         + "2300031010D0011410A48CBB366F418F465C"
                         + "7AF4EECE819E7E1C19000306220707183319";
        byte[] pdu = HexDump.hexStringToByteArray(pduString);
        mProxyListener  = mSmsTransferLayer.getListener();
        mProxyListener.notifyRLDataIndication(mToken, SmsUtils.FORMAT_INT_3GPP2,
                                              SmsUtils.RP_DATA, pdu);
        byte[] newCdmaPdu = mSmsTransferLayer.generateCdmaPdu(pdu);
        verify(mListener).notifySmsReceived(eq(mToken), eq(SmsUtils.FORMAT_INT_3GPP2),
                                            eq(SmsUtils.TP_SMS_DELIVER), eq(newCdmaPdu));
    }
    @Test
    public void test_notifyRLDataIndication_3GPP2_Failure() {
        //2nd byte of Pdu String has been modified with Invalid parameter ID
        String invalidPduString = "00FF021002020702A848D159E24006010008"
                         + "2300031010D0011410A48CBB366F418F465C"
                         + "7AF4EECE819E7E1C19000306220707183319";
        byte[] invalidPdu = HexDump.hexStringToByteArray(invalidPduString);
        mProxyListener  = mSmsTransferLayer.getListener();
        int result = mProxyListener.notifyRLDataIndication(mToken, SmsUtils.FORMAT_INT_3GPP2,
                                              SmsUtils.RP_DATA, invalidPdu);
        byte[] newCdmaPdu = mSmsTransferLayer.generateCdmaPdu(invalidPdu);
        assertEquals(SmsUtils.SMSTL_RESULT_GENERATE_CDMA_PDU_FAILED, result);
        verify(mListener, times(0)).notifySmsReceived(eq(mToken), eq(SmsUtils.FORMAT_INT_3GPP2),
                                            eq(SmsUtils.TP_SMS_DELIVER), eq(newCdmaPdu));
    }
    @Test
    public void test_notifyRLReportIndication() {
        mListener.notifySmsResult(mToken, 1, mResult, mReason, 0);
        verify(mListener, Mockito.times(1)).notifySmsResult(mToken, 1, mResult, mReason, 0);
    }

    @Test
    public void test_generateDeliverReportPdu() throws Exception {
        byte[] expectedDeliverReport = {0x00, 0x00};
        byte[] actualDeliverReport = mSmsTransferLayer.generateDeliverReportPdu(
                ImsSmsImplBase.DELIVER_STATUS_OK);
        for (int i = 0; i < expectedDeliverReport.length; i++) {
            assertEquals(expectedDeliverReport[i], actualDeliverReport[i]);
        }

        byte[] expectedDeliverGenericerrorReport = {0x00,
                (byte) SmsTransferLayer.TP_FCS_UNSPECIFIED_ERROR_CAUSE, 0x00};
        byte[] actualDeliverGenericerrorReport = mSmsTransferLayer.generateDeliverReportPdu(
                ImsSmsImplBase.DELIVER_STATUS_ERROR_GENERIC);
        for (int i = 0; i < expectedDeliverGenericerrorReport.length; i++) {
            assertEquals(expectedDeliverGenericerrorReport[i], actualDeliverGenericerrorReport[i]);
        }

        byte[] expectedDeliverNoMemoryReport = {0x00,
                (byte) SmsTransferLayer.TP_FCS_MEMORY_CAPACITY_EXCEEDED, 0x00};
        byte[] actualDeliverNoMemoryReport = mSmsTransferLayer.generateDeliverReportPdu(
                ImsSmsImplBase.DELIVER_STATUS_ERROR_NO_MEMORY);
        for (int i = 0; i < expectedDeliverNoMemoryReport.length; i++) {
            assertEquals(expectedDeliverNoMemoryReport[i], actualDeliverNoMemoryReport[i]);
        }

        byte[] expectedDeliverRequestNotSupportedReport = {0x00,
                (byte) SmsTransferLayer.TP_FCS_TPDU_NOT_SUPPORTED, 0x00};
        byte[] actualDeliverRequestNotSupportedReport = mSmsTransferLayer.generateDeliverReportPdu(
                ImsSmsImplBase.DELIVER_STATUS_ERROR_REQUEST_NOT_SUPPORTED);
        for (int i = 0; i < expectedDeliverRequestNotSupportedReport.length; i++) {
            assertEquals(expectedDeliverRequestNotSupportedReport[i],
                    actualDeliverRequestNotSupportedReport[i]);
        }
    }

    @After
    public void tearDown() throws Exception {
        mImsCallContext = null;
        mSmsRL = null;
        mSmsTransferLayer = null;
        mListener = null;
    }
}
