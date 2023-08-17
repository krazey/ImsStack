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
import static com.android.imsstack.imsservice.mmtel.sms.SmsRLStateMachine.SmsRLState
                                                       .WAIT_FOR_RPACK_FROM_NW;
import static com.android.imsstack.imsservice.mmtel.sms.SmsRLStateMachine.SmsRLState
                                                       .WAIT_TO_SEND_RPACK_TO_NW;
import static com.android.imsstack.imsservice.mmtel.sms.SmsUtils.RESULT_SUCCESS;
import static com.android.imsstack.imsservice.mmtel.sms.SmsUtils.SMSRL_RESULT_INVALID_STATE;

import static org.junit.Assert.assertEquals;
import static org.junit.Assert.assertTrue;
import static org.mockito.ArgumentMatchers.any;
import static org.mockito.ArgumentMatchers.anyInt;
import static org.mockito.ArgumentMatchers.anyString;
import static org.mockito.ArgumentMatchers.eq;
import static org.mockito.Mockito.times;
import static org.mockito.Mockito.verify;
import static org.mockito.Mockito.when;

import android.os.Handler;
import android.telephony.CarrierConfigManager;
import android.telephony.SmsManager;
import android.telephony.ims.stub.ImsSmsImplBase;

import com.android.imsstack.core.agents.AgentFactory;
import com.android.imsstack.core.agents.ConfigInterface;
import com.android.imsstack.core.config.CarrierConfig;
import com.android.imsstack.enabler.mts.MtsController;
import com.android.imsstack.imsservice.mmtel.ImsCallContext;
import com.android.imsstack.util.MessageExecutor;
import com.android.internal.util.HexDump;

import org.junit.After;
import org.junit.Assert;
import org.junit.Before;
import org.junit.Test;
import org.mockito.Mock;
import org.mockito.Mockito;

import java.io.ByteArrayOutputStream;
import java.util.concurrent.Semaphore;
import java.util.concurrent.TimeUnit;

public class SmsRLStateMachineTest {
    private static final int SLOT_ID = 0;
    private CarrierConfig mMockCarrierConfig;
    private ConfigInterface mMockConfigInterface;
    @Mock
    SmsRelayLayer.Listener mListener;
    @Mock
    ImsCallContext mContext;
    @Mock
    MtsController mMtsController;
    private Handler mHandler = null;
    int mMessageType = 2;
    private byte[] mMtRpAck = HexDump.hexStringToByteArray("0301");
    private byte[] mMtRpError = HexDump.hexStringToByteArray("0501016f");
    private String mDestinationAddress = "0987654321";
    private String mPsiSmsc = "+19037029920";
    private String mSmsc = "07912160130300F4";
    TestSmsRLStateMachine mSmsRLStateMachine;
    private SmsRLStateMachine.SmsRLState mCurrentState;
    private byte[] mMtRpData = HexDump.hexStringToByteArray("010107919130079229"
                                                        + "F0001221110A81785634121000000666"
                                                        + "B2996C2603");
    private int mToken = 1;
    private byte[] mTpdu = HexDump.hexStringToByteArray("21110A81785634121000000666B2996C2603");

    private static Semaphore sSemaphore = new Semaphore(0);

    @Before
    public void setUp() {
        mMockCarrierConfig = Mockito.mock(CarrierConfig.class);
        mMockConfigInterface = Mockito.mock(ConfigInterface.class);
        when(mMockConfigInterface.getCarrierConfig()).thenReturn(mMockCarrierConfig);
        AgentFactory.getInstance().setAgent(ConfigInterface.class, mMockConfigInterface, SLOT_ID);
        mContext = Mockito.mock(ImsCallContext.class);
        mMtsController = Mockito.mock(MtsController.class);
        mListener = Mockito.mock(SmsRelayLayer.Listener.class);

        mCurrentState = IDLE;
        mHandler = new MessageExecutor("testMachine");
        Mockito.when(mContext.getCallHandler()).thenReturn(mHandler);
        when(mMockCarrierConfig.getInt(
                CarrierConfigManager.ImsSms.KEY_SMS_TR1_TIMER_MILLIS_INT))
                .thenReturn(SmsUtils.TIMER_TR1M);
        when(mMockCarrierConfig.getInt(
                CarrierConfigManager.ImsSms.KEY_SMS_TR2_TIMER_MILLIS_INT))
                .thenReturn(SmsUtils.TIMER_TR2);
        mSmsRLStateMachine = new TestSmsRLStateMachine(mToken, mMessageType,
                mMtsController, mContext, mListener, mPsiSmsc, mDestinationAddress);
    }

    @Test
    public void getConfigTimer_test() {
        when(mMockCarrierConfig.getInt(
                CarrierConfigManager.ImsSms.KEY_SMS_TR1_TIMER_MILLIS_INT))
                .thenReturn(SmsUtils.TIMER_TR1M);
        assertEquals(SmsUtils.TIMER_TR1M, mSmsRLStateMachine.getTimerTR1());
        when(mMockCarrierConfig.getInt(
                CarrierConfigManager.ImsSms.KEY_SMS_TR2_TIMER_MILLIS_INT))
                .thenReturn(SmsUtils.TIMER_TR2);
        assertEquals(SmsUtils.TIMER_TR2, mSmsRLStateMachine.getTimerTR2());
    }

    @Test
    public void getSendStatus_test() {
        int[] retryCauseArray = {41, 29};
        int[] fallbackArray = {1, 8, 10};
        when(mMockCarrierConfig.getIntArray(
                CarrierConfigManager.ImsSms.KEY_SMS_RP_CAUSE_VALUES_TO_RETRY_OVER_IMS_INT_ARRAY))
                .thenReturn(retryCauseArray);
        when(mMockCarrierConfig.getIntArray(
                CarrierConfigManager.ImsSms.KEY_SMS_RP_CAUSE_VALUES_TO_FALLBACK_INT_ARRAY))
                .thenReturn(fallbackArray);
        assertEquals(ImsSmsImplBase.SEND_STATUS_ERROR_RETRY, mSmsRLStateMachine.getSendStatus(41));
        assertEquals(ImsSmsImplBase.SEND_STATUS_ERROR_FALLBACK,
                     mSmsRLStateMachine.getSendStatus(8));
        assertEquals(ImsSmsImplBase.SEND_STATUS_ERROR,
                     mSmsRLStateMachine.getSendStatus(11));
        //Test when both Arrays are empty, the status should be fetched from SmsRPErrorCause
        when(mMockCarrierConfig.getIntArray(
                CarrierConfigManager.ImsSms.KEY_SMS_RP_CAUSE_VALUES_TO_RETRY_OVER_IMS_INT_ARRAY))
                .thenReturn(null);
        when(mMockCarrierConfig.getIntArray(
                CarrierConfigManager.ImsSms.KEY_SMS_RP_CAUSE_VALUES_TO_FALLBACK_INT_ARRAY))
                .thenReturn(null);
        assertEquals(ImsSmsImplBase.SEND_STATUS_ERROR,
                     mSmsRLStateMachine.getSendStatus(8));
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
    public void onRpDatafromTL_timerTR2() {
        mSmsRLStateMachine.setState(WAIT_TO_SEND_RPACK_TO_NW);
        Assert.assertNotNull(mHandler);
        Assert.assertNotNull(mSmsRLStateMachine.mTR2TimerHandler);
        mHandler.post(()->mSmsRLStateMachine.mTR2TimerHandler.run());
        assertTrue(waitForEvent(sSemaphore, 1));
        verify(mListener, Mockito.timeout(100).times(0)).notifyRLReportIndication(mToken,
                mSmsRLStateMachine.mTpMr, ImsSmsImplBase.SEND_STATUS_ERROR_RETRY,
                SmsManager.RESULT_ERROR_GENERIC_FAILURE, 0);
        SmsRLStateMachine.SmsRLState state = mSmsRLStateMachine.getState();
        assertEquals(IDLE, state);
    }

    @Test
    public void onRPDataFromNetworkTimerTR1() {
        mSmsRLStateMachine.setState(WAIT_FOR_RPACK_FROM_NW);
        Assert.assertNotNull(mHandler);
        Assert.assertNotNull(mSmsRLStateMachine.mTR1TimerHandler);

        mHandler.post(()->mSmsRLStateMachine.mTR1TimerHandler.run());
        assertTrue(waitForEvent(sSemaphore, 1));
        verify(mListener, Mockito.timeout(100).times(1)).notifyRLReportIndication(mToken,
                mSmsRLStateMachine.mTpMr, ImsSmsImplBase.SEND_STATUS_ERROR_RETRY,
                SmsManager.RESULT_ERROR_GENERIC_FAILURE, 0);
        assertEquals(IDLE, mSmsRLStateMachine.getState());
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
        when(mMtsController.sendMessage(anyInt(), any(), anyString(), anyString(),
                anyInt())).thenReturn(true);
        SmsRPdu moRpError = new SmsRPdu(1, SmsUtils.RP_ERROR, mSmsc,
                             SmsRelayLayer.GENERIC_ERROR_CAUSE_VALUE, mTpdu);
        byte[] encodedPdu = moRpError.getRpduByteArray();
        int result;
        mSmsRLStateMachine.setState(IDLE);
        result = mSmsRLStateMachine.onRPErrorFromTL(moRpError);
        assertEquals(SMSRL_RESULT_INVALID_STATE, result);
        assertEquals(IDLE, mSmsRLStateMachine.getState());
        verify(mMtsController, times(0)).sendMessage(SmsUtils.FORMAT_INT_3GPP, encodedPdu, mPsiSmsc,
                mDestinationAddress, moRpError.getMessageRef());

        mSmsRLStateMachine.setState(WAIT_FOR_RPACK_FROM_NW);
        result = mSmsRLStateMachine.onRPErrorFromTL(moRpError);
        assertEquals(SMSRL_RESULT_INVALID_STATE, result);
        assertEquals(WAIT_FOR_RPACK_FROM_NW, mSmsRLStateMachine.getState());
        verify(mMtsController, times(0)).sendMessage(SmsUtils.FORMAT_INT_3GPP, encodedPdu, mPsiSmsc,
                mDestinationAddress, moRpError.getMessageRef());

        mSmsRLStateMachine.setState(WAIT_TO_SEND_RPACK_TO_NW);
        result = mSmsRLStateMachine.onRPErrorFromTL(moRpError);
        verify(mMtsController, times(1)).sendMessage(SmsUtils.FORMAT_INT_3GPP, encodedPdu, mPsiSmsc,
                mDestinationAddress, moRpError.getMessageRef());
        assertEquals(RESULT_SUCCESS, result);
        assertEquals(IDLE, mSmsRLStateMachine.getState());
    }

    @Test
    public void onRPErrorFromNetwork_allStateTest() {
        int[] fallbackCauseArray = {111, 41};
        when(mMockCarrierConfig.getIntArray(
                CarrierConfigManager.ImsSms.KEY_SMS_RP_CAUSE_VALUES_TO_FALLBACK_INT_ARRAY))
                .thenReturn(fallbackCauseArray);
        SmsRPdu mtRpError = new SmsRPdu(mMtRpError);
        int causeCode = mtRpError.getRPCause();
        mSmsRLStateMachine.setState(IDLE);
        mSmsRLStateMachine.onRPErrorFromNetwork(mtRpError);
        assertEquals(IDLE, mSmsRLStateMachine.getState());
        verify(mListener, times(0)).notifyRLReportIndication(mSmsRLStateMachine.mToken, 0,
                                SmsRPErrorCause.getSendSmsStatusByRPCauseCode(causeCode),
                                SmsRPErrorCause.getSendSmsStatusReasonByRPCauseCode(causeCode),
                                causeCode);

        mSmsRLStateMachine.setState(WAIT_TO_SEND_RPACK_TO_NW);
        mSmsRLStateMachine.onRPErrorFromNetwork(mtRpError);
        assertEquals(WAIT_TO_SEND_RPACK_TO_NW, mSmsRLStateMachine.getState());
        verify(mListener, times(0)).notifyRLReportIndication(mSmsRLStateMachine.mToken, 0,
                                SmsRPErrorCause.getSendSmsStatusByRPCauseCode(causeCode),
                                SmsRPErrorCause.getSendSmsStatusReasonByRPCauseCode(causeCode),
                                causeCode);

        mSmsRLStateMachine.setState(WAIT_FOR_RPACK_FROM_NW);
        mSmsRLStateMachine.onRPErrorFromNetwork(mtRpError);
        verify(mListener, times(1)).notifyRLReportIndication(mSmsRLStateMachine.mToken, 0,
                                mSmsRLStateMachine.getSendStatus(causeCode),
                                SmsRPErrorCause.getSendSmsStatusReasonByRPCauseCode(causeCode),
                                causeCode);
        assertEquals(IDLE, mSmsRLStateMachine.getState());
    }

    @Test
    public void onRPAckFromNetworkTest_Idle() {
        SmsRPdu mtRpAck = new SmsRPdu(mMtRpAck);
        mSmsRLStateMachine.setState(IDLE);
        mSmsRLStateMachine.onRPAckFromNetwork(mtRpAck);
        verify(mListener, times(0)).notifyRLReportIndication(mToken, 0,
                ImsSmsImplBase.SEND_STATUS_OK,
                SmsManager.RESULT_ERROR_NONE, 1);
        assertEquals(IDLE, mCurrentState);
    }

    @Test
    public void onRPAckFromNetworkTest_WaitForRpack() {
        SmsRPdu mtRpAck = new SmsRPdu(mMtRpAck);
        mSmsRLStateMachine.setState(WAIT_FOR_RPACK_FROM_NW);
        mSmsRLStateMachine.onRPAckFromNetwork(mtRpAck);
        verify(mListener).notifyRLReportIndication(mToken, 0,
                ImsSmsImplBase.SEND_STATUS_OK,
                SmsManager.RESULT_ERROR_NONE, 0);
        assertEquals(IDLE, mCurrentState);
    }

    @Test
    public void onRPAckFromNetworkTest_WaitToSendRPACK() {
        SmsRPdu mtRpAck = new SmsRPdu(mMtRpAck);
        mSmsRLStateMachine.setState(WAIT_TO_SEND_RPACK_TO_NW);
        mSmsRLStateMachine.onRPAckFromNetwork(mtRpAck);
        verify(mListener, times(0)).notifyRLReportIndication(mToken, 0,
                ImsSmsImplBase.SEND_STATUS_OK,
                SmsManager.RESULT_ERROR_NONE, 0);
    }

    @Test
    public void onRpDatafromTL_timerWithOutExpiry_Idle() throws InterruptedException {
        mSmsRLStateMachine.setState(IDLE);
        when(mMtsController.sendMessage(anyInt(), any(), anyString(), anyString(),
                 anyInt())).thenReturn(true);
        SmsRPdu moRPData = new SmsRPdu(1, SmsUtils.RP_DATA, mSmsc, 0, mTpdu);
        byte[] encodedPdu = moRPData.getRpduByteArray();
        int result = mSmsRLStateMachine.onRPDataFromTL(moRPData);

        verify(mMtsController).sendMessage(eq(SmsUtils.FORMAT_INT_3GPP),
                eq(encodedPdu), eq(mPsiSmsc),
                eq(mDestinationAddress), eq(1));
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
    public void onRPAckFromTLTest_Idle() {
        mSmsRLStateMachine.setState(IDLE);
        SmsRPdu moRpAck = new SmsRPdu(1, SmsUtils.RP_ACK, mSmsc, 0, null);

        byte[] encodedPdu = moRpAck.getRpduByteArray();
        mSmsRLStateMachine.onRPAckFromTL(moRpAck);
        verify(mMtsController, times(0)).sendMessage(SmsUtils.FORMAT_INT_3GPP, encodedPdu, mPsiSmsc,
                mDestinationAddress, moRpAck.getMessageRef());
    }

    @Test
    public void onRPAckFromTLTest_WaitToSendRPack() {
        mSmsRLStateMachine.setState(WAIT_TO_SEND_RPACK_TO_NW);
        SmsRPdu moRpAck = new SmsRPdu(1, SmsUtils.RP_ACK, mSmsc, 0, null);

        byte[] encodedPdu = moRpAck.getRpduByteArray();
        mSmsRLStateMachine.onRPAckFromTL(moRpAck);
        verify(mMtsController).sendMessage(SmsUtils.FORMAT_INT_3GPP, encodedPdu, mPsiSmsc,
                mDestinationAddress, moRpAck.getMessageRef());
    }

    @Test
    public void onRPAckFromTLTest_WaitForRPACK() {
        mSmsRLStateMachine.setState(WAIT_FOR_RPACK_FROM_NW);
        SmsRPdu moRpAck = new SmsRPdu(1, SmsUtils.RP_ACK, mSmsc, 0, null);

        byte[] encodedPdu = moRpAck.getRpduByteArray();
        mSmsRLStateMachine.onRPAckFromTL(moRpAck);
        verify(mMtsController, times(0)).sendMessage(SmsUtils.FORMAT_INT_3GPP, encodedPdu, mPsiSmsc,
                moRpAck.getDestAddr(), moRpAck.getMessageRef());

    }

    @Test
    public void onTR1TimerExpiredTest_Idle() {
        mSmsRLStateMachine.setState(IDLE);
        mSmsRLStateMachine.onTR1TimerExpired();
        verify(mListener, times(0)).notifyRLReportIndication(mSmsRLStateMachine.mToken, 0,
                ImsSmsImplBase.SEND_STATUS_ERROR_RETRY,
                SmsManager.RESULT_ERROR_GENERIC_FAILURE, 1);
        assertEquals(IDLE,
                mSmsRLStateMachine.getState());
    }

    @Test
    public void onTR1TimerExpiredTest_WaitForRPack() {
        mSmsRLStateMachine.setState(WAIT_FOR_RPACK_FROM_NW);
        mSmsRLStateMachine.onTR1TimerExpired();
        verify(mListener).notifyRLReportIndication(mSmsRLStateMachine.mToken, 0,
                ImsSmsImplBase.SEND_STATUS_ERROR_RETRY,
                SmsManager.RESULT_ERROR_GENERIC_FAILURE, 0);
        assertEquals(IDLE,
                mSmsRLStateMachine.getState());
    }

    @Test
    public void onTR1TimerExpiredTest_WaitToSendPack() {
        mSmsRLStateMachine.setState(WAIT_TO_SEND_RPACK_TO_NW);
        mSmsRLStateMachine.onTR1TimerExpired();
        verify(mListener, times(0)).notifyRLReportIndication(mSmsRLStateMachine.mToken, 0,
                ImsSmsImplBase.SEND_STATUS_ERROR_RETRY,
                SmsManager.RESULT_ERROR_GENERIC_FAILURE, 1);
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
        mSmsRLStateMachine.setState(WAIT_TO_SEND_RPACK_TO_NW);
        int result = mSmsRLStateMachine.onSipResponseForRPMessage(false, ImsSmsImplBase
                                                                .SEND_STATUS_ERROR_FALLBACK);
        verify(mListener, times(0)).notifyRLReportIndication(mSmsRLStateMachine.mToken, 0,
                                 ImsSmsImplBase.SEND_STATUS_ERROR_FALLBACK,
                                 SmsManager.RESULT_NETWORK_ERROR, 0);
        assertEquals(WAIT_TO_SEND_RPACK_TO_NW, mSmsRLStateMachine.getState());
        assertEquals(SMSRL_RESULT_INVALID_STATE, result);

        mSmsRLStateMachine.setState(IDLE);
        result = mSmsRLStateMachine.onSipResponseForRPMessage(false, ImsSmsImplBase
                                                                    .SEND_STATUS_ERROR_FALLBACK);
        verify(mListener, times(0)).notifyRLReportIndication(mSmsRLStateMachine.mToken, 0,
                                 ImsSmsImplBase.SEND_STATUS_ERROR_FALLBACK,
                                 SmsManager.RESULT_NETWORK_ERROR, 0);
        assertEquals(IDLE, mSmsRLStateMachine.getState());
        assertEquals(SMSRL_RESULT_INVALID_STATE, result);

        mSmsRLStateMachine.setState(WAIT_FOR_RPACK_FROM_NW);
        result = mSmsRLStateMachine.onSipResponseForRPMessage(false, ImsSmsImplBase
                                                       .SEND_STATUS_ERROR_FALLBACK);
        assertEquals(RESULT_SUCCESS, result);
        assertEquals(IDLE, mSmsRLStateMachine.getState());
        verify(mListener, times(1)).notifyRLReportIndication(mSmsRLStateMachine.mToken, 0,
                                 ImsSmsImplBase.SEND_STATUS_ERROR_FALLBACK,
                                 SmsManager.RESULT_NETWORK_ERROR, 0);
    }

    private static class TestSmsRLStateMachine extends SmsRLStateMachine {
        TestSmsRLStateMachine(int token, int messageType, MtsController mtsController,
                ImsCallContext context, SmsRelayLayer.Listener listener, String psiSmsc,
                String destinationAddress) {
            super(token, messageType, mtsController, context, listener, psiSmsc,
                destinationAddress);
        }

        protected Runnable mTR1TimerHandler = new Runnable() {
            @Override
            public void run() {
                onTR1TimerExpired();
                sSemaphore.release();
            }
        };

        protected Runnable mTR2TimerHandler = new Runnable() {
            @Override
            public void run() {
                onTR2TimerExpired();
                sSemaphore.release();
            }
        };
    }

    private boolean waitForEvent(Semaphore semaphore, int expectedNumberOfEvents) {
        for (int i = 0; i < expectedNumberOfEvents; i++) {
            try {
                if (!semaphore.tryAcquire(2000, TimeUnit.MILLISECONDS)) {
                    return false;
                }
            } catch (Exception ex) {
                return false;
            }
        }
        return true;
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
