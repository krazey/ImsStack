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
import static org.mockito.ArgumentMatchers.any;
import static org.mockito.ArgumentMatchers.anyInt;
import static org.mockito.ArgumentMatchers.anyString;
import static org.mockito.ArgumentMatchers.eq;
import static org.mockito.Mockito.timeout;
import static org.mockito.Mockito.times;
import static org.mockito.Mockito.verify;
import static org.mockito.Mockito.when;

import android.telephony.SmsManager;
import android.telephony.ims.stub.ImsSmsImplBase;

import com.android.imsstack.core.agents.Usat;
import com.android.imsstack.core.agents.UsatInterface;
import com.android.imsstack.imsservice.mmtel.ImsCallContext;
import com.android.imsstack.util.ImsUtils;

import org.junit.After;
import org.junit.Before;
import org.junit.Test;
import org.junit.runner.RunWith;
import org.junit.runners.JUnit4;
import org.mockito.ArgumentCaptor;
import org.mockito.Captor;
import org.mockito.Mock;
import org.mockito.Mockito;
import org.mockito.MockitoAnnotations;

@RunWith(JUnit4.class)
public class SmsTransferLayerTest {
    private static final String TAG = "[GII-SmsTL Test] ";
    //For status result not applicable
    private static final int STATUS_RESULT_NA = 0;

    @Mock ImsCallContext mImsCallContext;
    @Mock SmsTransferLayer.Listener mListener;
    @Mock SmsRelayLayer mSmsRL;
    @Mock private UsatInterface mMockUsatInterface;
    @Mock private Usat.MoSmsControlCommandResponse mMockUsatCmdRes;
    @Mock private Usat.MoSmsControlCommand mMoSmsCCmd;
    @Captor ArgumentCaptor<String> mRpAddrCaptor;
    @Captor ArgumentCaptor<String> mTpAddrCaptor;
    @Captor ArgumentCaptor<Usat.Listener> mUsatListenerCaptor;

    private TestSmsTransferLayer mSmsTransferLayer;
    private SmsTransferLayer.SmsRLListenerProxy mProxyListener;
    private int mToken = 1;
    private int mSmsFormat = SmsUtils.FORMAT_INT_3GPP;
    private int mMessageRef = 1;
    private String mSmsc = "0791190791700590";
    private String mDecodedSmsc = "+917019075009";
    private String mDestinationAddress = "900080006200";
    private byte[] mPdu = ImsUtils.hexStringToBytes("21110A81785634121000000666B2996C2603");
    private byte[] mTpdu = ImsUtils.hexStringToBytes("01060C81090008002600000006536A905A9D02");
    private String mTpduString = "01060A817856341200000006536A905A9D02";
    private String mDestAddr = "8765432100";
    private String mUsatTpDestAddr = "1234567890";
    private String mEncodedRpDestAddr = "07812160130300F4";
    private String mUsatRpDestAddr = "12063130004";
    private String mUsatTpduString = "01060A812143658709000006536A905A9D02";
    private int mTlMessageType = SmsUtils.TP_SMS_DELIVER;
    private int mResult = ImsSmsImplBase.SEND_STATUS_OK;
    private int mRpMessageType = SmsUtils.RP_DATA;
    private int mReason = SmsManager.RESULT_ERROR_NONE;

    @Before
    public void setUp() throws Exception {
        MockitoAnnotations.initMocks(this);

        when(mImsCallContext.getUsatInterface()).thenReturn(mMockUsatInterface);
        when(mMockUsatInterface.createMoSmsControlCommand(anyString(), anyString(), anyInt(),
                any())).thenReturn(mMoSmsCCmd);
        when(mMockUsatCmdRes.getResult()).thenReturn(Usat.RESULT_ALLOWED);

        mSmsTransferLayer = new TestSmsTransferLayer(mImsCallContext, mSmsRL);
        mSmsTransferLayer.setListener(mListener);
    }

    @After
    public void tearDown() throws Exception {
        mSmsTransferLayer = null;
    }

    @Test
    public void test_sendMemoryAvailabilityNotification() {
        mSmsTransferLayer.sendMemoryAvailabilityNotification(mToken, mSmsc);
        verify(mSmsRL).sendRPMessage(eq(mToken), eq(SmsUtils.RP_SMMA), eq(mSmsc), eq(mSmsc),
                eq(null), eq(STATUS_RESULT_NA), eq(false));
    }

    @Test
    public void test_sendMemoryAvailabilityNotification_fail() {
        mSmsTransferLayer.sendMemoryAvailabilityNotification(mToken, mSmsc);
        mProxyListener  = mSmsTransferLayer.getListener();
        mResult = ImsSmsImplBase.SEND_STATUS_ERROR_RETRY;
        mProxyListener.notifyRLReportIndication(mToken, 1, mResult, mReason, 0);

        verify(mSmsRL, Mockito.times(1)).sendRPMessage(eq(mToken), eq(SmsUtils.RP_SMMA), eq(mSmsc),
                eq(mSmsc),
                eq(null), eq(STATUS_RESULT_NA), eq(false));
        verify(mListener, Mockito.times(1)).notifyMemoryAvailableResult(mToken, mResult, 0);
    }

    @Test
    public void test_sendMemoryAvailabilityNotification_newPass_newFail() {
        mProxyListener  = mSmsTransferLayer.getListener();

        // First SMMA (new-passed)
        mSmsTransferLayer.sendMemoryAvailabilityNotification(mToken, mSmsc);
        mResult = ImsSmsImplBase.SEND_STATUS_OK;
        mProxyListener.notifyRLReportIndication(mToken, 1, mResult, mReason, 0);

        verify(mSmsRL, Mockito.times(1)).sendRPMessage(eq(mToken), eq(SmsUtils.RP_SMMA), eq(mSmsc),
                eq(mSmsc),
                eq(null), eq(STATUS_RESULT_NA), eq(false));
        verify(mListener, Mockito.times(1)).notifyMemoryAvailableResult(mToken, mResult, 0);

        // Second SMMA  (new-failed)
        mToken += 1;
        mSmsTransferLayer.sendMemoryAvailabilityNotification(mToken, mSmsc);
        mResult = ImsSmsImplBase.SEND_STATUS_ERROR_RETRY;

        mProxyListener.notifyRLReportIndication(mToken, 1, mResult, mReason, 0);

        verify(mSmsRL, Mockito.times(1)).sendRPMessage(eq(mToken), eq(SmsUtils.RP_SMMA), eq(mSmsc),
                eq(mSmsc),
                eq(null), eq(STATUS_RESULT_NA), eq(false));
        verify(mListener, Mockito.times(1)).notifyMemoryAvailableResult(mToken, mResult, 0);

        // Third SMMA  (retry)
        mToken += 1;
        mSmsTransferLayer.sendMemoryAvailabilityNotification(mToken, mSmsc);

        verify(mSmsRL, Mockito.times(1)).sendRPMessage(eq(mToken), eq(SmsUtils.RP_SMMA), eq(mSmsc),
                eq(mSmsc),
                eq(null), eq(STATUS_RESULT_NA), eq(true));
    }

    @Test
    public void test_sendMemoryAvailabilityNotification_newPass_newPass() {
        mProxyListener  = mSmsTransferLayer.getListener();

        // First SMMA (new-passed)
        mSmsTransferLayer.sendMemoryAvailabilityNotification(mToken, mSmsc);
        mResult = ImsSmsImplBase.SEND_STATUS_OK;
        mProxyListener.notifyRLReportIndication(mToken, 1, mResult, mReason, 0);

        verify(mSmsRL, Mockito.times(1)).sendRPMessage(eq(mToken), eq(SmsUtils.RP_SMMA), eq(mSmsc),
                eq(mSmsc),
                eq(null), eq(STATUS_RESULT_NA), eq(false));
        verify(mListener, Mockito.times(1)).notifyMemoryAvailableResult(mToken, mResult, 0);

        // Second SMMA  (new-passed)
        mToken += 1;
        mSmsTransferLayer.sendMemoryAvailabilityNotification(mToken, mSmsc);
        mResult = ImsSmsImplBase.SEND_STATUS_OK;

        mProxyListener.notifyRLReportIndication(mToken, 1, mResult, mReason, 0);

        verify(mSmsRL, Mockito.times(1)).sendRPMessage(eq(mToken), eq(SmsUtils.RP_SMMA), eq(mSmsc),
                eq(mSmsc),
                eq(null), eq(STATUS_RESULT_NA), eq(false));
        verify(mListener, Mockito.times(1)).notifyMemoryAvailableResult(mToken, mResult, 0);

        // Third SMMA  (not retry)
        mToken += 1;
        mSmsTransferLayer.sendMemoryAvailabilityNotification(mToken, mSmsc);

        verify(mSmsRL, Mockito.times(1)).sendRPMessage(eq(mToken), eq(SmsUtils.RP_SMMA), eq(mSmsc),
                eq(mSmsc),
                eq(null), eq(STATUS_RESULT_NA), eq(false));
    }

    @Test
    public void test_sendMemoryAvailabilityNotification_newFail_retryFail() {
        mProxyListener  = mSmsTransferLayer.getListener();

        // First SMMA (new-failed)
        mSmsTransferLayer.sendMemoryAvailabilityNotification(mToken, mSmsc);
        mResult = ImsSmsImplBase.SEND_STATUS_ERROR_RETRY;
        mProxyListener.notifyRLReportIndication(mToken, 1, mResult, mReason, 0);

        verify(mSmsRL, Mockito.times(1)).sendRPMessage(eq(mToken), eq(SmsUtils.RP_SMMA), eq(mSmsc),
                eq(mSmsc),
                eq(null), eq(STATUS_RESULT_NA), eq(false));
        verify(mListener, Mockito.times(1)).notifyMemoryAvailableResult(mToken, mResult, 0);

        // Second SMMA  (retry-failed)
        mToken += 1;
        mSmsTransferLayer.sendMemoryAvailabilityNotification(mToken, mSmsc);
        mResult = ImsSmsImplBase.SEND_STATUS_ERROR_RETRY;

        mProxyListener.notifyRLReportIndication(mToken, 1, mResult, mReason, 0);

        verify(mSmsRL, Mockito.times(1)).sendRPMessage(eq(mToken), eq(SmsUtils.RP_SMMA), eq(mSmsc),
                eq(mSmsc),
                eq(null), eq(STATUS_RESULT_NA), eq(true));
        verify(mListener, Mockito.times(1)).notifyMemoryAvailableResult(mToken, mResult, 0);

        // Third SMMA  (new)
        mToken += 1;
        mSmsTransferLayer.sendMemoryAvailabilityNotification(mToken, mSmsc);

        verify(mSmsRL, Mockito.times(1)).sendRPMessage(eq(mToken), eq(SmsUtils.RP_SMMA), eq(mSmsc),
                eq(mSmsc),
                eq(null), eq(STATUS_RESULT_NA), eq(false));
    }

    @Test
    public void test_sendMemoryAvailabilityNotification_newFail_retryPass() {
        mProxyListener  = mSmsTransferLayer.getListener();

        // First SMMA (new-failed)
        mSmsTransferLayer.sendMemoryAvailabilityNotification(mToken, mSmsc);
        mResult = ImsSmsImplBase.SEND_STATUS_ERROR_RETRY;
        mProxyListener.notifyRLReportIndication(mToken, 1, mResult, mReason, 0);

        verify(mSmsRL, Mockito.times(1)).sendRPMessage(eq(mToken), eq(SmsUtils.RP_SMMA), eq(mSmsc),
                eq(mSmsc),
                eq(null), eq(STATUS_RESULT_NA), eq(false));
        verify(mListener, Mockito.times(1)).notifyMemoryAvailableResult(mToken, mResult, 0);

        // Second SMMA  (retry-passed)
        mToken += 1;
        mSmsTransferLayer.sendMemoryAvailabilityNotification(mToken, mSmsc);
        mResult = ImsSmsImplBase.SEND_STATUS_OK;

        mProxyListener.notifyRLReportIndication(mToken, 1, mResult, mReason, 0);

        verify(mSmsRL, Mockito.times(1)).sendRPMessage(eq(mToken), eq(SmsUtils.RP_SMMA), eq(mSmsc),
                eq(mSmsc),
                eq(null), eq(STATUS_RESULT_NA), eq(true));
        verify(mListener, Mockito.times(1)).notifyMemoryAvailableResult(mToken, mResult, 0);

        // Third SMMA  (new)
        mToken += 1;
        mSmsTransferLayer.sendMemoryAvailabilityNotification(mToken, mSmsc);

        verify(mSmsRL, Mockito.times(1)).sendRPMessage(eq(mToken), eq(SmsUtils.RP_SMMA), eq(mSmsc),
                eq(mSmsc),
                eq(null), eq(STATUS_RESULT_NA), eq(false));
    }

    @Test
    public void test_sendMoTPdu() {
        assertEquals(SmsUtils.RESULT_SUCCESS, mSmsTransferLayer.sendMoTPdu(mToken,
                mSmsFormat, mMessageRef, mSmsc, mPdu));
        mSmsTransferLayer.sendMoTPdu(mToken, SmsUtils.RP_ACK, mMessageRef, mSmsc, mPdu);
        verify(mSmsRL).sendRPMessage(anyInt(), anyInt(), anyString(), anyString(),
                any(), anyInt(), eq(false));
    }

    @Test
    public void test_sendMoTPdu_USAT_RESULT_ALLOWED() {
        when(mMockUsatInterface.isServiceAvailable(Usat.SERVICE_MO_SMS_CONTROL)).thenReturn(true);
        when(mMockUsatCmdRes.getResult()).thenReturn(Usat.RESULT_ALLOWED);
        when(mMockUsatCmdRes.getCommand()).thenReturn(mMoSmsCCmd);
        byte[] tpdu = ImsUtils.hexStringToBytes(mTpduString);
        mSmsTransferLayer.sendMoTPdu(mToken, SmsUtils.FORMAT_INT_3GPP, mMessageRef, mSmsc, tpdu);
        verify(mMockUsatInterface).createMoSmsControlCommand(mRpAddrCaptor.capture(),
                                                             mTpAddrCaptor.capture(), anyInt(),
                                                             mUsatListenerCaptor.capture());
        verify(mMockUsatInterface).sendCommand(any());
        String rpAddr = mRpAddrCaptor.getValue();
        assertEquals(mDecodedSmsc, rpAddr);
        String tpAddr = mTpAddrCaptor.getValue();
        assertEquals(mDestAddr, tpAddr);

        Usat.Listener usatListener = mUsatListenerCaptor.getValue();
        assertNotNull(usatListener);
        when(mMoSmsCCmd.getRpDestinationAddress()).thenReturn(rpAddr);
        when(mMoSmsCCmd.getTpDestinationAddress()).thenReturn(tpAddr);
        usatListener.onCommandResponse(mMockUsatCmdRes);
        verify(mSmsRL).sendRPMessage(eq(mToken), eq(SmsUtils.RP_DATA), eq(mSmsc), eq(tpAddr),
                eq(tpdu), eq(STATUS_RESULT_NA), eq(false));
    }

    @Test
    public void test_sendMoTPdu_USAT_RESULT_ALLOWED_WITH_MODIFICATION() throws Exception  {
        when(mMockUsatInterface.isServiceAvailable(Usat.SERVICE_MO_SMS_CONTROL)).thenReturn(true);
        when(mMockUsatCmdRes.getResult()).thenReturn(Usat.RESULT_ALLOWED_WITH_MODIFICATION);
        when(mMockUsatCmdRes.getCommand()).thenReturn(mMoSmsCCmd);
        mSmsTransferLayer.sendMoTPdu(mToken, SmsUtils.FORMAT_INT_3GPP,
                                             mMessageRef, mSmsc,
                                             ImsUtils.hexStringToBytes(mTpduString));
        verify(mMockUsatInterface).createMoSmsControlCommand(mRpAddrCaptor.capture(),
                                                             mTpAddrCaptor.capture(), anyInt(),
                                                             mUsatListenerCaptor.capture());
        verify(mMockUsatInterface).sendCommand(any());

        String rpAddr = mRpAddrCaptor.getValue();
        assertEquals(mDecodedSmsc, rpAddr);
        String tpAddr = mTpAddrCaptor.getValue();
        assertEquals(mDestAddr, tpAddr);

        Usat.Listener usatListener = mUsatListenerCaptor.getValue();
        assertNotNull(usatListener);
        when(mMoSmsCCmd.getRpDestinationAddress()).thenReturn(rpAddr);
        when(mMoSmsCCmd.getTpDestinationAddress()).thenReturn(tpAddr);
        when(mMockUsatCmdRes.getRpDestinationAddress()).thenReturn(mUsatRpDestAddr);
        when(mMockUsatCmdRes.getTpDestinationAddress()).thenReturn(mUsatTpDestAddr);
        usatListener.onCommandResponse(mMockUsatCmdRes);
        byte[] usatTpdu = ImsUtils.hexStringToBytes(mUsatTpduString);
        verify(mSmsRL).sendRPMessage(eq(mToken), eq(SmsUtils.RP_DATA), eq(mEncodedRpDestAddr),
                             eq(mUsatTpDestAddr), eq(usatTpdu), eq(STATUS_RESULT_NA), eq(false));
    }

    @Test
    public void test_sendMoTPdu_USAT_RESULT_NOT_ALLOWED() {
        when(mMockUsatInterface.isServiceAvailable(Usat.SERVICE_MO_SMS_CONTROL)).thenReturn(true);
        when(mMockUsatCmdRes.getResult()).thenReturn(Usat.RESULT_NOT_ALLOWED);
        when(mMockUsatCmdRes.getCommand()).thenReturn(mMoSmsCCmd);
        byte[] tpdu = ImsUtils.hexStringToBytes(mTpduString);
        mSmsTransferLayer.sendMoTPdu(mToken, SmsUtils.FORMAT_INT_3GPP, mMessageRef, mSmsc, tpdu);
        verify(mMockUsatInterface).createMoSmsControlCommand(mRpAddrCaptor.capture(),
                                                             mTpAddrCaptor.capture(), anyInt(),
                                                             mUsatListenerCaptor.capture());
        verify(mMockUsatInterface).sendCommand(any());

        String rpAddr = mRpAddrCaptor.getValue();
        assertEquals(mDecodedSmsc, rpAddr);
        String tpAddr = mTpAddrCaptor.getValue();
        assertEquals(mDestAddr, tpAddr);

        Usat.Listener usatListener = mUsatListenerCaptor.getValue();
        assertNotNull(usatListener);

        usatListener.onCommandResponse(mMockUsatCmdRes);
        verify(mListener, Mockito.times(1)).notifySmsResult(mToken, tpdu[1] & 0xff,
                                                            ImsSmsImplBase.SEND_STATUS_ERROR,
                                                            SmsManager.RESULT_OPERATION_NOT_ALLOWED,
                                                            ImsSmsImplBase.RESULT_NO_NETWORK_ERROR);

    }

    @Test
    public void test_calculateTpDestAddrLengthByte() {
        //for odd number of digits
        byte[] tpDestAddrBytes = ImsUtils.hexStringToBytes("9112345678F9");
        int len = mSmsTransferLayer.calculateTpDestAddrLengthByte(tpDestAddrBytes);
        assertEquals(len, 9);

        //for even number of Digits
        tpDestAddrBytes = ImsUtils.hexStringToBytes("911234567899");
        len = mSmsTransferLayer.calculateTpDestAddrLengthByte(tpDestAddrBytes);
        assertEquals(len, 10);
    }

    @Test
    public void test_calculateIndexAfterTpDestAddr() {
        //without padding in TP-DA
        byte[] tpdu1 = ImsUtils.hexStringToBytes("01060A817856341200000006536A905A9D02");
        //with padding in TP-DA
        byte[] tpdu2 = ImsUtils.hexStringToBytes("01060781214365F7000006536A905A9D02");

        int index1 = mSmsTransferLayer.calculateIndexAfterTpDestAddr(tpdu1);
        int index2 = mSmsTransferLayer.calculateIndexAfterTpDestAddr(tpdu2);
        assertEquals(index1, 9);
        assertEquals(index2, 8);
    }

    @Test
    public void testCdmaPduError() {
        byte[] newCdmaPdu = mSmsTransferLayer.generateCdmaPdu(null);
        assertEquals(newCdmaPdu.length, 0);
    }

    @Test
    public void testCdmaPduGenerate() {
        String pduString = "00000210020102000002070282055950C840"
                         + "08100003101460010610268D2285000A0100";
        byte[] pdu = ImsUtils.hexStringToBytes(pduString);
        byte[] cdmaPdu = mSmsTransferLayer.generateCdmaPdu(pdu);
        String expectedPduString = "0000000000001002000000000000"
                                 + "00000A4438313536353433323100"
                                 + "0000000000000000001000031014"
                                 + "60010610268D2285000A0100";
        byte[] expectedPdu = ImsUtils.hexStringToBytes(expectedPduString);
        for (int i = 0; i < cdmaPdu.length; i++) {
            assertEquals(cdmaPdu[i], expectedPdu[i]);
        }
        assertTrue((cdmaPdu.length > 0) ? true : false);
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
                eq(mTpdu), eq(STATUS_RESULT_NA), eq(false));
        mProxyListener.notifyRLReportIndication(1, mTpdu[1] & 0xff, ImsSmsImplBase.SEND_STATUS_OK,
                SmsManager.RESULT_ERROR_NONE, successCause);
        verify(mSmsRL, timeout(3000).times(1)).sendRPMessage(eq(2), eq(mRpMessageType), eq(mSmsc),
                eq(mDestinationAddress), eq(mTpdu), eq(STATUS_RESULT_NA), eq(false));
        mProxyListener.notifyRLReportIndication(2, mTpdu[1] & 0xff, ImsSmsImplBase.SEND_STATUS_OK,
                SmsManager.RESULT_ERROR_NONE, successCause);
        verify(mSmsRL, timeout(3000).times(1)).sendRPMessage(eq(3), eq(mRpMessageType), eq(mSmsc),
                eq(mDestinationAddress), eq(mTpdu), eq(STATUS_RESULT_NA), eq(false));
        mProxyListener.notifyRLReportIndication(3, mTpdu[1] & 0xff, ImsSmsImplBase.SEND_STATUS_OK,
                SmsManager.RESULT_ERROR_NONE, successCause);
        verify(mSmsRL, timeout(3000).times(1)).sendRPMessage(eq(4), eq(mRpMessageType), eq(mSmsc),
                eq(mDestinationAddress), eq(mTpdu), eq(STATUS_RESULT_NA), eq(false));
    }

    private static class TestSmsTransferLayer extends SmsTransferLayer {
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
        mSmsTransferLayer.sendReportTPdu(mToken, mMessageRef, mResult, null);
        byte[] deliverReportPdu = mSmsTransferLayer.generateDeliverReportPdu(mResult);
        verify(mSmsRL).sendRPMessage(eq(mToken), eq(SmsUtils.RP_ACK), eq(null), eq(null),
                eq(deliverReportPdu), eq(mResult));
    }

    @Test
    public void test_notifyRLDataIndication_3GPP2() {
        String pduString = "00000210020102000002070282055950C840"
                         + "08100003101460010610268D2285000A0100";
        byte[] pdu = ImsUtils.hexStringToBytes(pduString);
        mProxyListener  = mSmsTransferLayer.getListener();
        mProxyListener.notifyRLDataIndication(mToken, SmsUtils.FORMAT_INT_3GPP2,
                                              SmsUtils.RP_DATA, pdu);
        String formattedTpduString = "0000000000001002000000000000"
                                 + "00000A4438313536353433323100"
                                 + "0000000000000000001000031014"
                                 + "60010610268D2285000A0100";
        byte[] formattedTpdu = ImsUtils.hexStringToBytes(formattedTpduString);
        verify(mListener).notifySmsReceived(eq(mToken), eq(SmsUtils.FORMAT_INT_3GPP2),
                                            eq(SmsUtils.TP_SMS_DELIVER), eq(formattedTpdu));
    }

    @Test
    public void test_notifyRLDataIndication_3GPP2_Failure() {
        //2nd byte of Pdu String has been modified with Invalid parameter ID
        String invalidPduString = "00FF021002020702A848D159E24006010008"
                         + "2300031010D0011410A48CBB366F418F465C"
                         + "7AF4EECE819E7E1C19000306220707183319";
        byte[] invalidPdu = ImsUtils.hexStringToBytes(invalidPduString);
        mProxyListener  = mSmsTransferLayer.getListener();
        int result = mProxyListener.notifyRLDataIndication(mToken, SmsUtils.FORMAT_INT_3GPP2,
                                              SmsUtils.RP_DATA, invalidPdu);
        verify(mListener, times(0)).notifySmsReceived(eq(mToken), eq(SmsUtils.FORMAT_INT_3GPP2),
                                            eq(SmsUtils.TP_SMS_DELIVER), any());
    }

    @Test
    public void test_notifyRLReportIndication() {
        mProxyListener  = mSmsTransferLayer.getListener();
        mProxyListener.notifyRLReportIndication(mToken, 1, mResult, mReason, 0);
        verify(mListener).notifySmsResult(mToken, 1, mResult, mReason, 0);
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

        //Invalid case i.e. default
        byte[] expectedDeliver = {0x00,
                (byte) SmsTransferLayer.TP_FCS_UNSPECIFIED_ERROR_CAUSE, 0x00};
        byte[] actualDeliver = mSmsTransferLayer.generateDeliverReportPdu(0);
        for (int i = 0; i < expectedDeliver.length; i++) {
            assertEquals(expectedDeliver[i], actualDeliver[i]);
        }
    }
}
