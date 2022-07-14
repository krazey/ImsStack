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

import static com.android.imsstack.imsservice.mmtel.sms.SmsRLStateMachine.SmsRLState.IDLE;
import static com.android.imsstack.imsservice.mmtel.sms.SmsRLStateMachine.SmsRLState.WAIT_FOR_RPACK_FROM_NW;
import static com.android.imsstack.imsservice.mmtel.sms.SmsRLStateMachine.SmsRLState.WAIT_TO_SEND_RPACK_TO_NW;
import static com.android.imsstack.imsservice.mmtel.sms.SmsUtils.SMSRL_RESULT_INVALID_STATE;

import static org.junit.Assert.assertEquals;
import static org.mockito.ArgumentMatchers.anyInt;
import static org.mockito.ArgumentMatchers.anyString;
import static org.mockito.ArgumentMatchers.eq;
import static org.mockito.Mockito.times;
import static org.mockito.Mockito.verify;
import static org.mockito.Mockito.when;

import android.os.Handler;
import android.telephony.SmsManager;
import android.telephony.ims.stub.ImsSmsImplBase;
import android.util.Base64;

import com.android.imsstack.enabler.mts.MtsController;
import com.android.imsstack.imsservice.mmtel.ImsCallContext;
import com.android.imsstack.util.MessageExecutor;

import org.junit.After;
import org.junit.Before;
import org.junit.Test;
import org.mockito.Mock;
import org.mockito.Mockito;

import java.io.ByteArrayOutputStream;

public class SmsRLStateMachineTest {

    @Mock
    SmsRelayLayer.Listener mListener;
    @Mock
    ImsCallContext mContext;
    @Mock
    MtsController mMtsController;
    private Handler mHandler = null;

    int mMessageType = 2;
    private byte[] mMoRpAck = {02, 01};
    private byte[] mMoRpError = {04, 01, 01, 0x6f};
    private byte[] mMtRpAck = {03, 01};
    private byte[] mMtRpError = {05, 01, 01, 0x6f};
    private String mDestinationAddress = "0987654321";
    private String mSmsc = "+19037029920";
    SmsRLStateMachine mSmsRLStateMachine;

    private SmsRLStateMachine.SmsRLState mCurrentState;
    private byte[] mMtRpData = {01, 01, 07, (byte) 0x91, 91, 30, 07, 92, 29, (byte) 0xF0, 00, 12,
            21, 11, (byte) 0x0A, 81, 78, 56, 34, 12, 10, 00, 00, 06, 66, (byte) 0xB2, 99,
            (byte) 0x6C, 26, 03};
    private int mToken = 1;
    private int mRpType = SmsUtils.RP_DATA;

    private byte[] mTpdu = {21, 11, (byte) 0x0A, 81, 78, 56, 34, 12, 10, 00, 00, 06, 66,
            (byte) 0xB2, 99, (byte) 0x6C, 26, 03};

    private byte[] mMoRpData = {00, 01, 00, 07, (byte) 0x91, (byte) 0x91, 0x30, 07,
            (byte) 0x92, 0x29, (byte) 0xF0, 0x12, 21, 11, (byte) 0x0A, 81, 78, 56, 34, 12, 10, 00,
            00, 06, 66, (byte) 0xB2, 99, (byte) 0x6C, 26, 03};


    @Before
    public void setUp() {
        mContext = Mockito.mock(ImsCallContext.class);
        mMtsController = Mockito.mock(MtsController.class);
        mListener = Mockito.mock(SmsRelayLayer.Listener.class);

        mCurrentState = IDLE;
        Handler mHandler = new MessageExecutor("testMachine");
        Mockito.when(mContext.getCallHandler()).thenReturn(mHandler);
        mSmsRLStateMachine = new SmsRLStateMachine(mCurrentState, mToken, mMessageType,
                mMtsController, mContext, mListener, mDestinationAddress, mHandler);
    }

    @Test
    public void onRPDataFromNetworkWithOutTimerExpiry_AllState() {

        // set State to IDLE
        mSmsRLStateMachine.setState(IDLE);
        SmsRPdu mtRpdata = new SmsRPdu(mMtRpData);

        int result = mSmsRLStateMachine.onRPDataFromNetwork(mtRpdata);
        verify(mListener, times(1)).notifyRLDataIndication(mToken,
                SmsUtils.FORMAT_INT_3GPP, SmsUtils.RP_DATA, getPdu(mtRpdata));
        assertEquals(WAIT_TO_SEND_RPACK_TO_NW, mSmsRLStateMachine.getState());
        assertEquals(SmsUtils.RESULT_SUCCESS, result);

        // set State to WAIT_FOR_RPACK
        mSmsRLStateMachine.setState(WAIT_FOR_RPACK_FROM_NW);
        result = mSmsRLStateMachine.onRPDataFromNetwork(mtRpdata);
        assertEquals(WAIT_FOR_RPACK_FROM_NW, mSmsRLStateMachine.getState());
        assertEquals(SmsUtils.SMSRL_RESULT_INVALID_STATE, result);

        // set State to WAIT_TO_SEND_RPACK
        mSmsRLStateMachine.setState(WAIT_TO_SEND_RPACK_TO_NW);
        result = mSmsRLStateMachine.onRPDataFromNetwork(mtRpdata);
        assertEquals(WAIT_TO_SEND_RPACK_TO_NW, mSmsRLStateMachine.getState());
        assertEquals(SmsUtils.SMSRL_RESULT_INVALID_STATE, result);
    }

    @Test
    public void onRPDataFromNetworkWithTimerExpiry_Idle() {
        // Timer is set only for the IDLE state
        //TODO b/240369151 need to add timer with out delay
    }

    public byte[] getPdu(SmsRPdu mtRPData) {
        byte[] tpdu = mtRPData.getUserData();
        byte[] rpdu = mtRPData.getRpduByteArray();
        int originAddressLength = rpdu[SmsUtils.RPDU_ORIGIN_ADDR_LENGTH_INDEX];
        ByteArrayOutputStream bo = new ByteArrayOutputStream(originAddressLength
                + tpdu.length + 1);
        bo.write(originAddressLength);
        bo.write(rpdu, SmsUtils.RPDU_ORIGIN_ADDR_VALUE_INDEX, originAddressLength);
        bo.write(tpdu, 0, tpdu.length);
        byte[] frameworkPdu = bo.toByteArray();
        return frameworkPdu;
    }

    @Test
    public void onRPErrorFromTL_AllStateTest() {
        SmsRPdu mMoRpError = new SmsRPdu(1, SmsUtils.RP_ERROR, mSmsc, 0, mTpdu);
        int result = mSmsRLStateMachine.onRPErrorFromTL(mMoRpError);

        mSmsRLStateMachine.setState(IDLE);
        result = mSmsRLStateMachine.onRPErrorFromTL(mMoRpError);
        assertEquals(SMSRL_RESULT_INVALID_STATE, result);
        assertEquals(IDLE, mSmsRLStateMachine.getState());

        mSmsRLStateMachine.setState(WAIT_FOR_RPACK_FROM_NW);
        result = mSmsRLStateMachine.onRPErrorFromTL(mMoRpError);
        assertEquals(SMSRL_RESULT_INVALID_STATE, result);
        assertEquals(WAIT_FOR_RPACK_FROM_NW, mSmsRLStateMachine.getState());

        mSmsRLStateMachine.setState(WAIT_TO_SEND_RPACK_TO_NW);
        result = mSmsRLStateMachine.onRPErrorFromTL(mMoRpError);
        assertEquals(SMSRL_RESULT_INVALID_STATE, result);
        assertEquals(WAIT_TO_SEND_RPACK_TO_NW, mSmsRLStateMachine.getState());
    }

    @Test
    public void onRPErrorFromNetwork_allStateTest() {
        SmsRPdu mMtRpError = new SmsRPdu(this.mMtRpError);
        mSmsRLStateMachine.onRPErrorFromNetwork(mMtRpError);
        // TODO method is not implemented
    }

    @Test
    public void onRPAckFromNetworkTest_Idle() {
        SmsRPdu mtRpAck = new SmsRPdu(mMtRpAck);
        mSmsRLStateMachine.setState(IDLE);
        mSmsRLStateMachine.onRPAckFromNetwork(mtRpAck);
        verify(mListener, times(0)).notifyRLReportIndication(mToken,
                ImsSmsImplBase.SEND_STATUS_OK,
                SmsManager.RESULT_ERROR_NONE);
        assertEquals(IDLE, mCurrentState);
    }

    @Test
    public void onRPAckFromNetworkTest_WaitForRpack() {
        SmsRPdu mtRpAck = new SmsRPdu(mMtRpAck);
        mSmsRLStateMachine.setState(WAIT_FOR_RPACK_FROM_NW);
        mSmsRLStateMachine.onRPAckFromNetwork(mtRpAck);
        verify(mListener).notifyRLReportIndication(mToken,
                ImsSmsImplBase.SEND_STATUS_OK,
                SmsManager.RESULT_ERROR_NONE);
        assertEquals(IDLE, mCurrentState);
    }

    @Test
    public void onRPAckFromNetworkTest_WaitToSendRPACK() {
        SmsRPdu mtRpAck = new SmsRPdu(mMtRpAck);
        mSmsRLStateMachine.setState(WAIT_TO_SEND_RPACK_TO_NW);
        mSmsRLStateMachine.onRPAckFromNetwork(mtRpAck);
        verify(mListener, times(0)).notifyRLReportIndication(mToken,
                ImsSmsImplBase.SEND_STATUS_OK,
                SmsManager.RESULT_ERROR_NONE);
    }

    @Test
    public void onRpDatafromTL_timerWithOutExpiry_Idle() throws InterruptedException {
        mSmsRLStateMachine.setState(IDLE);
        when(mMtsController.sendMessage(anyInt(), anyString(), anyString(),
                 anyInt())).thenReturn(true);
        SmsRPdu moRPData = new SmsRPdu(1, SmsUtils.RP_DATA, mSmsc, 0, mTpdu);
        byte[] encodedPdu = moRPData.getRpduByteArray();
        String pdu64 = Base64.encodeToString(encodedPdu, Base64.DEFAULT);
        int result = mSmsRLStateMachine.onRPDataFromTL(moRPData);

        verify(mMtsController).sendMessage(eq(SmsUtils.FORMAT_INT_3GPP),
                eq(pdu64), eq(mDestinationAddress), eq(1));
        assertEquals(WAIT_FOR_RPACK_FROM_NW, mSmsRLStateMachine.getState());

        assertEquals(SmsUtils.RESULT_SUCCESS, result);
    }

    @Test
    public void onRpDatafromTL_timerWithOutExpiry_WaitToSendRpack() throws InterruptedException {
        mSmsRLStateMachine.setState(WAIT_TO_SEND_RPACK_TO_NW);
        SmsRPdu moRPData = new SmsRPdu(1, SmsUtils.RP_DATA, mSmsc, 0, mTpdu);
        int result = mSmsRLStateMachine.onRPDataFromTL(moRPData);
        assertEquals(SMSRL_RESULT_INVALID_STATE, result);
    }

    @Test
    public void onRpDatafromTL_timerWithOutExpiry_WaitForRPack() throws InterruptedException {
        mSmsRLStateMachine.setState(WAIT_FOR_RPACK_FROM_NW);
        SmsRPdu moRPData = new SmsRPdu(1, SmsUtils.RP_DATA, mSmsc, 0, mTpdu);
        int result = mSmsRLStateMachine.onRPDataFromTL(moRPData);
        assertEquals(SMSRL_RESULT_INVALID_STATE, result);
    }

    @Test
    public void onRpDatafromTLTest_WaitToSendRPack() throws InterruptedException {
        mSmsRLStateMachine.setState(WAIT_TO_SEND_RPACK_TO_NW);
        SmsRPdu moRPData = new SmsRPdu(1, SmsUtils.RP_DATA, mSmsc, 0, mTpdu);
        int result = mSmsRLStateMachine.onRPDataFromTL(moRPData);
        assertEquals(SMSRL_RESULT_INVALID_STATE, result);
    }

    @Test
    public void onRpDatafromTL_timerWithExpiry_IDLE() {
        //TODO b/240369151 need to add timer with out delay
    }

    @Test
    public void onRPAckFromTLTest_Idle() {
        mSmsRLStateMachine.setState(IDLE);
        SmsRPdu moRpAck = new SmsRPdu(1, SmsUtils.RP_ACK, mDestinationAddress, 0, null);

        byte[] encodedPdu = moRpAck.getRpduByteArray();
        String pdu64 = Base64.encodeToString(encodedPdu, Base64.DEFAULT);
        mSmsRLStateMachine.onRPAckFromTL(moRpAck);
        verify(mMtsController, times(0)).sendMessage(SmsUtils.FORMAT_INT_3GPP, pdu64,
                moRpAck.getDestAddr(), moRpAck.getMessageRef());
    }

    @Test
    public void onRPAckFromTLTest_WaitToSendPPack() {
        mSmsRLStateMachine.setState(WAIT_TO_SEND_RPACK_TO_NW);
        SmsRPdu moRpAck = new SmsRPdu(1, SmsUtils.RP_ACK, mDestinationAddress, 0, null);

        byte[] encodedPdu = moRpAck.getRpduByteArray();
        String pdu64 = Base64.encodeToString(encodedPdu, Base64.DEFAULT);
        int result = mSmsRLStateMachine.onRPAckFromTL(moRpAck);
        verify(mMtsController).sendMessage(SmsUtils.FORMAT_INT_3GPP, pdu64,
                moRpAck.getDestAddr(), moRpAck.getMessageRef());
    }

    @Test
    public void onRPAckFromTLTest_WaitForRPACK() {
        mSmsRLStateMachine.setState(WAIT_FOR_RPACK_FROM_NW);
        SmsRPdu moRpAck = new SmsRPdu(1, SmsUtils.RP_ACK, mDestinationAddress, 0, null);

        byte[] encodedPdu = moRpAck.getRpduByteArray();
        String pdu64 = Base64.encodeToString(encodedPdu, Base64.DEFAULT);
        mSmsRLStateMachine.onRPAckFromTL(moRpAck);
        verify(mMtsController, times(0)).sendMessage(SmsUtils.FORMAT_INT_3GPP, pdu64,
                moRpAck.getDestAddr(), moRpAck.getMessageRef());

    }

    @Test
    public void onTR1TimerExpiredTest_Idle() {
        mSmsRLStateMachine.setState(IDLE);
        mSmsRLStateMachine.onTR1TimerExpired();
        verify(mListener, times(0)).notifyRLReportIndication(mSmsRLStateMachine.mToken,
                ImsSmsImplBase.SEND_STATUS_ERROR_RETRY,
                SmsManager.RESULT_ERROR_GENERIC_FAILURE);
        assertEquals(IDLE,
                mSmsRLStateMachine.getState());
    }

    @Test
    public void onTR1TimerExpiredTest_WaitForRPack() {
        mSmsRLStateMachine.setState(WAIT_FOR_RPACK_FROM_NW);
        mSmsRLStateMachine.onTR1TimerExpired();
        verify(mListener).notifyRLReportIndication(mSmsRLStateMachine.mToken,
                ImsSmsImplBase.SEND_STATUS_ERROR_RETRY,
                SmsManager.RESULT_ERROR_GENERIC_FAILURE);
        assertEquals(IDLE,
                mSmsRLStateMachine.getState());
    }

    @Test
    public void onTR1TimerExpiredTest_WaitToSendPack() {
        mSmsRLStateMachine.setState(WAIT_TO_SEND_RPACK_TO_NW);
        mSmsRLStateMachine.onTR1TimerExpired();
        verify(mListener, times(0)).notifyRLReportIndication(mSmsRLStateMachine.mToken,
                ImsSmsImplBase.SEND_STATUS_ERROR_RETRY,
                SmsManager.RESULT_ERROR_GENERIC_FAILURE);
        assertEquals(WAIT_TO_SEND_RPACK_TO_NW,
                mSmsRLStateMachine.getState());
    }

    @Test
    public void onTR2TimerExpired_allStateTest() {
        mSmsRLStateMachine.setState(IDLE);
        mSmsRLStateMachine.onTR2TimerExpired();
        assertEquals(IDLE,
                mSmsRLStateMachine.getState());

        mSmsRLStateMachine.setState(WAIT_TO_SEND_RPACK_TO_NW);
        mSmsRLStateMachine.onTR2TimerExpired();
        assertEquals(IDLE,
                mSmsRLStateMachine.getState());

        mSmsRLStateMachine.setState(WAIT_FOR_RPACK_FROM_NW);
        mSmsRLStateMachine.onTR2TimerExpired();
        assertEquals(WAIT_FOR_RPACK_FROM_NW,
                mSmsRLStateMachine.getState());
        //TODO method is not implemented.
    }

    @Test
    public void onSipResponseForRPMessage_allStateTest() {
        int result = mSmsRLStateMachine.onSipResponseForRPMessage(false, 1);
        assertEquals(SMSRL_RESULT_INVALID_STATE, result);
        //TODO method is not implemented
    }

    @After
    public void tearDown() throws Exception {
        mSmsRLStateMachine = null;
        mListener = null;
        mMtsController = null;
        mContext = null;
        mHandler = null;
    }
}
