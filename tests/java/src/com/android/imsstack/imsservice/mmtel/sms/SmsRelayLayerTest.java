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

import static com.android.imsstack.imsservice.mmtel.sms.SmsRLStateMachine.SmsRLState
                                                        .IDLE;
import static com.android.imsstack.imsservice.mmtel.sms.SmsRLStateMachine.SmsRLState
                                                        .WAIT_FOR_RPACK_FROM_NW;

import static org.junit.Assert.assertEquals;
import static org.mockito.ArgumentMatchers.any;
import static org.mockito.ArgumentMatchers.anyInt;
import static org.mockito.ArgumentMatchers.anyBoolean;
import static org.mockito.ArgumentMatchers.anyString;
import static org.mockito.ArgumentMatchers.eq;
import static org.mockito.Mockito.mock;
import static org.mockito.Mockito.timeout;
import static org.mockito.Mockito.never;
import static org.mockito.Mockito.verify;
import static org.mockito.Mockito.when;

import android.content.Context;
import android.net.Uri;
import android.os.Handler;
import android.telephony.CarrierConfigManager;
import android.telephony.SmsManager;
import android.telephony.ims.stub.ImsSmsImplBase;

import com.android.imsstack.base.AppContext;
import com.android.imsstack.base.SystemServiceProxy;
import com.android.imsstack.base.SystemServiceProxy.SmsManagerProxy;
import com.android.imsstack.core.agents.AgentFactory;
import com.android.imsstack.core.agents.ConfigInterface;
import com.android.imsstack.core.config.CarrierConfig;
import com.android.imsstack.enabler.mts.MtsController;
import com.android.imsstack.imsservice.mmtel.ImsCallContext;
import com.android.imsstack.util.ImsUtils;

import org.junit.After;
import org.junit.Before;
import org.junit.Test;
import org.junit.runner.RunWith;
import org.junit.runners.JUnit4;
import org.mockito.Mock;
import org.mockito.MockitoAnnotations;

import java.lang.reflect.Field;
import java.util.concurrent.atomic.AtomicInteger;

@RunWith(JUnit4.class)
public class SmsRelayLayerTest {
    private static final String TAG = "SmsRelayLayerTest";

    @Mock
    ImsCallContext mImsCallContext;
    @Mock
    MtsController mMtsController;
    @Mock
    SmsRelayLayer.Listener mListener;
    @Mock
    Context mContext;
    @Mock
    SystemServiceProxy mMockSystemServiceProxy;
    @Mock
    SmsManagerProxy mMockSmsManagerProxySub1;
    @Mock
    SmsManagerProxy mMockSmsManagerProxySub2;
    @Mock
    CarrierConfig mMockCarrierConfig;
    @Mock
    ConfigInterface mMockConfigInterface;

    private SmsRelayLayer mSmsRelayLayer;
    private SmsRelayLayer.MtsControllerListenerProxy mProxyListener;

    private int mSmsFormat = SmsUtils.FORMAT_INT_3GPP;
    private int mToken = 1;
    private int mRpType = SmsUtils.RP_DATA;
    private String mSmsc = "07912160130300F4";
    private String mDecodedSmsc = "+12063130004";
    private String mDestinationAddress = "6503907941";
    private int mStatusResultNA = 0;
    private static int sSuccessCause = 0;
    // 0x16 is Memory Capacity Exceeded
    private static final int CAUSE_MEMORY_CAPACITY_EXCEEDED = 0x16;
    private byte[] mTpdu = ImsUtils.hexStringToBytes("21110A81785634121000000666B2996C2603");
    private byte[] mMtRpData = ImsUtils.hexStringToBytes("010107919130079229"
                                                        + "F0001221110A81785634121000000666"
                                                        + "B2996C2603");
    private static final int SUB_ID_1 = 1;
    private static final int SUB_ID_2 = 2;
    private static final Uri SMSC_IDENTITY_SUB1 = Uri.parse("tel:+1111111111");
    private static final Uri SMSC_IDENTITY_SUB2 = Uri.parse("tel:+2222222222");

    @Before
    public void setUp() throws Exception {
        MockitoAnnotations.openMocks(this);

        when(mMockConfigInterface.getCarrierConfig()).thenReturn(mMockCarrierConfig);
        AgentFactory.getInstance().setAgent(ConfigInterface.class, mMockConfigInterface, SUB_ID_1);
        AgentFactory.getInstance().setAgent(ConfigInterface.class, mMockConfigInterface, SUB_ID_2);
        AgentFactory.getInstance().setAgent(ConfigInterface.class, mMockConfigInterface, -1);
        when(mMockCarrierConfig.getInt(
                CarrierConfigManager.ImsSms.KEY_SMS_TR1_TIMER_MILLIS_INT))
                .thenReturn(SmsUtils.TIMER_TR1M);
        when(mMockCarrierConfig.getInt(
                CarrierConfigManager.ImsSms.KEY_SMS_TR2_TIMER_MILLIS_INT))
                .thenReturn(SmsUtils.TIMER_TR2);

        AppContext.init(mContext);
        AppContext.getInstance().setSystemServiceProxy(mMockSystemServiceProxy);

        SmsManagerProxy mockSmsManagerProxyDefault = mock(SmsManagerProxy.class);
        when(mMockSystemServiceProxy.getSystemService(SmsManagerProxy.class))
                .thenReturn(mockSmsManagerProxyDefault);

        when(mockSmsManagerProxyDefault.createForSubscriptionId(eq(SUB_ID_1)))
                .thenReturn(mMockSmsManagerProxySub1);
        when(mockSmsManagerProxyDefault.createForSubscriptionId(eq(SUB_ID_2)))
                .thenReturn(mMockSmsManagerProxySub2);
        when(mMockSmsManagerProxySub1.getSmscIdentity()).thenReturn(SMSC_IDENTITY_SUB1);
        when(mMockSmsManagerProxySub2.getSmscIdentity()).thenReturn(SMSC_IDENTITY_SUB2);
        when(mImsCallContext.getCallHandler()).thenReturn(mock(Handler.class));
        mSmsRelayLayer = new SmsRelayLayer(mImsCallContext, mMtsController);
        mSmsRelayLayer.mDeliverCause.put(ImsSmsImplBase.DELIVER_STATUS_OK, 0);
        mSmsRelayLayer.mDeliverCause.put(ImsSmsImplBase.DELIVER_STATUS_ERROR_GENERIC,
                SmsRPErrorCause.SMS_PROTOCOL_ERROR.getRPCauseCode());
        mSmsRelayLayer.mDeliverCause.put(ImsSmsImplBase.DELIVER_STATUS_ERROR_NO_MEMORY,
                CAUSE_MEMORY_CAPACITY_EXCEEDED);
        mSmsRelayLayer.mDeliverCause.put(
                ImsSmsImplBase.DELIVER_STATUS_ERROR_REQUEST_NOT_SUPPORTED,
                SmsRPErrorCause.SMS_MESSAGE_TYPE_NOT_IMPLEMENTED.getRPCauseCode());
        mSmsRelayLayer.mDeliverCause.put(SmsRelayLayer.DELIVER_STATUS_ERROR_INVALID_MR_VALUE,
                SmsRPErrorCause.SMS_INVALID_MESSAGE_REF.getRPCauseCode());
        mProxyListener = mSmsRelayLayer.new MtsControllerListenerProxy();
        mSmsRelayLayer.setListener(mListener);
        when(mImsCallContext.getSlotId()).thenReturn(SUB_ID_1);
    }

    @Test
    public void test_sendRPMessage_exception() {
        when(mMtsController.sendMessage(anyInt(), any(), anyString(), anyString(), anyInt(),
                anyBoolean())).thenThrow(new RuntimeException("Test Exception"));
        when(mImsCallContext.getSubId()).thenReturn(SUB_ID_1);
        assertEquals(SmsUtils.SMSRL_RESULT_EXCEPTION, mSmsRelayLayer.sendRPMessage(mToken, mRpType,
                mSmsc, mDestinationAddress, mTpdu, mStatusResultNA));
    }

    @Test
    public void test_targetAddress() {
        when(mMtsController.sendMessage(anyInt(), any(), anyString(), anyString(), anyInt(),
                anyBoolean())).thenReturn(true);

        // RP_DATA with valid subId should use PSI
        when(mImsCallContext.getSubId()).thenReturn(SUB_ID_1);
        when(mImsCallContext.getSlotId()).thenReturn(SUB_ID_1);
        mSmsRelayLayer.sendRPMessage(mToken, mRpType, mSmsc, mDestinationAddress, mTpdu,
                mStatusResultNA);
        verify(mMtsController).sendMessage(anyInt(), any(), eq(SMSC_IDENTITY_SUB1.toString()),
                eq(mDestinationAddress), anyInt(), eq(false));

        // RP_DATA with invalid subId should use decoded smsc
        when(mImsCallContext.getSubId()).thenReturn(-1);
        mSmsRelayLayer.sendRPMessage(mToken + 1, mRpType, mSmsc, mDestinationAddress, mTpdu,
                mStatusResultNA);
        verify(mMtsController).sendMessage(anyInt(), any(), eq(mDecodedSmsc),
                eq(mDestinationAddress), anyInt(), eq(false));

        // RP_SMMA should use smsc
        mSmsRelayLayer.sendRPMessage(mToken + 2, SmsUtils.RP_SMMA, mSmsc, null, null,
                mStatusResultNA);
        verify(mMtsController).sendMessage(anyInt(), any(), eq(mSmsc), eq(mSmsc), anyInt(),
                eq(false));
    }

    @Test
    public void test_VerifyMoSuccess()
            throws NoSuchFieldException, IllegalAccessException {
        when(mMtsController.sendMessage(anyInt(), any(), anyString(), anyString(),
                anyInt(), anyBoolean())).thenReturn(true);
        when(mImsCallContext.getSubId()).thenReturn(-1);
        mSmsRelayLayer.sendRPMessage(mToken, mRpType, mSmsc, mDestinationAddress, mTpdu,
                                     mStatusResultNA);
        Field f = SmsRelayLayer.class.getDeclaredField("mRPMR");
        f.setAccessible(true);
        AtomicInteger rpMR = (AtomicInteger) f.get(mSmsRelayLayer);
        int rpDataMR = rpMR.get();
        SmsRPdu pdu = new SmsRPdu(rpDataMR, mRpType, mSmsc, 0, mTpdu);
        byte[] expectedMoRpData = pdu.getRpduByteArray();
        verify(mMtsController, timeout(1000).times(1)).sendMessage(eq(mSmsFormat),
                                        eq(expectedMoRpData), eq(mDecodedSmsc),
                                        eq(mDestinationAddress), eq(rpDataMR), eq(false));
        assertEquals(WAIT_FOR_RPACK_FROM_NW, mSmsRelayLayer.mSmsRLStateMachine.getState());
        byte[] mtRpAck = new byte[2];
        mtRpAck[0] = SmsRPdu.MT_RP_ACK_MTI;
        mtRpAck[1] = (byte) (rpDataMR & 0xff);
        mProxyListener.notifyIncomingMessage(mSmsFormat, mtRpAck);
        int tpMR = mTpdu[1] & 0xff;
        verify(mListener).notifyRLReportIndication(eq(mToken), eq(tpMR),
                    eq(ImsSmsImplBase.SEND_STATUS_OK), eq(SmsManager.RESULT_ERROR_NONE),
                    eq(sSuccessCause));
    }

    @Test
    public void test_VerifyRPSMMA()
            throws NoSuchFieldException, IllegalAccessException {
        when(mMtsController.sendMessage(anyInt(), any(), anyString(), anyString(),
                anyInt(), anyBoolean())).thenReturn(true);
        when(mImsCallContext.getSubId()).thenReturn(-1);
        mSmsRelayLayer.sendRPMessage(mToken, SmsUtils.RP_SMMA, mSmsc, null, null,
                                     mStatusResultNA);
        Field f = SmsRelayLayer.class.getDeclaredField("mRPMR");
        f.setAccessible(true);
        AtomicInteger rpMR = (AtomicInteger) f.get(mSmsRelayLayer);
        int rpDataMR = rpMR.get();
        SmsRPdu pdu = new SmsRPdu(rpDataMR, SmsUtils.RP_SMMA, null, 0, null, false);
        byte[] expectedMoRpData = pdu.getRpduByteArray();
        verify(mMtsController).setListener(any());
        verify(mMtsController, timeout(1000).times(1)).sendMessage(eq(mSmsFormat),
                                        eq(expectedMoRpData), eq(mSmsc),
                                        eq(mSmsc), eq(rpDataMR), eq(false));
        assertEquals(WAIT_FOR_RPACK_FROM_NW, mSmsRelayLayer.mSmsRLStateMachine.getState());
        byte[] mtRpAck = new byte[2];
        mtRpAck[0] = SmsRPdu.MT_RP_ACK_MTI;
        mtRpAck[1] = (byte) (rpDataMR & 0xff);
        mProxyListener.notifyIncomingMessage(mSmsFormat, mtRpAck);
        verify(mListener).notifyRLReportIndication(eq(mToken), eq(0),
                    eq(ImsSmsImplBase.SEND_STATUS_OK), eq(SmsManager.RESULT_ERROR_NONE),
                    eq(sSuccessCause));
        assertEquals(IDLE, mSmsRelayLayer.mSmsRLStateMachine.getState());
    }

    @Test
    public void test_VerifyMultipleMoSuccess() throws NoSuchFieldException,
                                                     IllegalAccessException {

        int rpDataMR1, rpDataMR2;
        SmsRPdu pdu;
        byte[] expectedMoRpData;
        when(mMtsController.sendMessage(anyInt(), any(), anyString(), anyString(),
                                        anyInt(), anyBoolean())).thenReturn(true);
        when(mImsCallContext.getSubId()).thenReturn(-1);
        mSmsRelayLayer.sendRPMessage(1, mRpType, mSmsc, mDestinationAddress, mTpdu,
                                     mStatusResultNA);
        //fetching RP-MR generated in MO RP-DATA
        Field f = SmsRelayLayer.class.getDeclaredField("mRPMR");
        f.setAccessible(true);
        AtomicInteger rpMR = (AtomicInteger) f.get(mSmsRelayLayer);
        rpDataMR1 = rpMR.get();
        mSmsRelayLayer.sendRPMessage(2, mRpType, mSmsc, mDestinationAddress, mTpdu,
                                     mStatusResultNA);
        rpDataMR2 = rpDataMR1 + 1;
        pdu = new SmsRPdu(rpDataMR1, mRpType, mSmsc, 0, mTpdu, false);
        expectedMoRpData = pdu.getRpduByteArray();
        verify(mMtsController).sendMessage(eq(mSmsFormat), eq(expectedMoRpData), eq(mDecodedSmsc),
                                           eq(mDestinationAddress), eq(rpDataMR1), eq(false));
        pdu = new SmsRPdu(rpDataMR2, mRpType, mSmsc, 0, mTpdu, false);
        expectedMoRpData = pdu.getRpduByteArray();
        verify(mMtsController).sendMessage(eq(mSmsFormat), eq(expectedMoRpData), eq(mDecodedSmsc),
                                           eq(mDestinationAddress), eq(rpDataMR2), eq(false));
        byte[] mtRpAck1 = new byte[2];
        mtRpAck1[0] = SmsRPdu.MT_RP_ACK_MTI;
        mtRpAck1[1] = (byte) (rpDataMR1 & 0xff);
        mProxyListener.notifyIncomingMessage(mSmsFormat, mtRpAck1);
        int tpMR = mTpdu[1] & 0xff;
        verify(mListener).notifyRLReportIndication(eq(1), eq(tpMR),
                eq(ImsSmsImplBase.SEND_STATUS_OK),
                eq(SmsManager.RESULT_ERROR_NONE), eq(sSuccessCause));
        byte[] mtRpAck2 = new byte[2];
        mtRpAck2[0] = SmsRPdu.MT_RP_ACK_MTI;
        mtRpAck2[1] = (byte) (rpDataMR2 & 0xff);
        mProxyListener.notifyIncomingMessage(mSmsFormat, mtRpAck2);
        verify(mListener).notifyRLReportIndication(eq(2), eq(tpMR),
                                eq(ImsSmsImplBase.SEND_STATUS_OK),
                                eq(SmsManager.RESULT_ERROR_NONE), eq(sSuccessCause));
    }


    @Test
    public void test_VerifyMtAckWithInvalidToken() {
        when(mImsCallContext.getSubId()).thenReturn(-1);
        mProxyListener.notifyIncomingMessage(mSmsFormat, mMtRpData);
        int token;
        try {
            Field f = SmsRelayLayer.class.getDeclaredField("mToken");
            f.setAccessible(true);
            AtomicInteger intToken = (AtomicInteger) f.get(mSmsRelayLayer);
            token = intToken.get() + 1;
        } catch (Exception e) {
            e.printStackTrace();
            return;
        }
        int result = mSmsRelayLayer.sendRPMessage(token, SmsUtils.RP_ACK, mSmsc,
                    mDestinationAddress, mTpdu, ImsSmsImplBase.DELIVER_STATUS_OK);
        assertEquals(SmsUtils.SMSRL_RESULT_TOKEN_DOES_NOT_EXIST, result);
    }

    private int getMtToken() {
        try {
            Field f = SmsRelayLayer.class.getDeclaredField("mToken");
            f.setAccessible(true);
            AtomicInteger intToken = (AtomicInteger) f.get(mSmsRelayLayer);
            return intToken.get() | SmsRelayLayer.MT_TOKEN_MASK;
        } catch (Exception e) {
            e.printStackTrace();
            return 0;
        }
    }

    @Test
    public void test_VerifyMtSuccess_3GPP2() {
        String pduString = "0000021002020702A848D159E24006010008"
                + "2300031010D0011410A48CBB366F418F465C"
                + "7AF4EECE819E7E1C19000306220707183319";
        byte[] pdu = ImsUtils.hexStringToBytes(pduString);
        mProxyListener.notifyIncomingMessage(SmsUtils.FORMAT_INT_3GPP2, pdu);
        int token = getMtToken();
        verify(mListener).notifyRLDataIndication(eq(token), eq(SmsUtils.FORMAT_INT_3GPP2),
                eq(SmsUtils.RP_DATA), eq(pdu));
    }

    @Test
    public void test_getPSIValue_usesCorrectSubId() {
        // Test with SUB_ID_1
        when(mImsCallContext.getSubId()).thenReturn(SUB_ID_1);
        String psi1 = mSmsRelayLayer.getPSIValue();
        assertEquals(SMSC_IDENTITY_SUB1.toString(), psi1);
        verify(mMockSmsManagerProxySub1).getSmscIdentity();

        // Test with SUB_ID_2
        when(mImsCallContext.getSubId()).thenReturn(SUB_ID_2);
        when(mImsCallContext.getSlotId()).thenReturn(SUB_ID_2);
        String psi2 = mSmsRelayLayer.getPSIValue();
        assertEquals(SMSC_IDENTITY_SUB2.toString(), psi2);
        verify(mMockSmsManagerProxySub2).getSmscIdentity();
    }

    @Test
    public void test_sendRPMessage_usesPSIFromCorrectSubId() {
        when(mMtsController.sendMessage(anyInt(), any(), anyString(), anyString(), anyInt(),
                 anyBoolean())).thenReturn(true);

        // Scenario 1: Send with SUB_ID_1
        when(mImsCallContext.getSubId()).thenReturn(SUB_ID_1);
        when(mImsCallContext.getSlotId()).thenReturn(SUB_ID_1);
        mSmsRelayLayer.sendRPMessage(mToken, mRpType, mSmsc, mDestinationAddress, mTpdu,
                mStatusResultNA);
        verify(mMtsController).sendMessage(anyInt(), any(), eq(SMSC_IDENTITY_SUB1.toString()),
                eq(mDestinationAddress), anyInt(), eq(false));

        // Scenario 2: Send with SUB_ID_2
        when(mImsCallContext.getSubId()).thenReturn(SUB_ID_2);
        when(mImsCallContext.getSlotId()).thenReturn(SUB_ID_2);
        mSmsRelayLayer.sendRPMessage(mToken + 1, mRpType, mSmsc, mDestinationAddress, mTpdu,
                mStatusResultNA);
        verify(mMtsController).sendMessage(anyInt(), any(), eq(SMSC_IDENTITY_SUB2.toString()),
                eq(mDestinationAddress), anyInt(), eq(false));
    }

    private int setupOutgoingMessage() {
        when(mMtsController.sendMessage(anyInt(), any(), anyString(), anyString(),
                anyInt(), anyBoolean())).thenReturn(true);
        when(mImsCallContext.getSubId()).thenReturn(SUB_ID_1);
        when(mImsCallContext.getSlotId()).thenReturn(SUB_ID_1);
        mSmsRelayLayer.sendRPMessage(mToken, mRpType, mSmsc, mDestinationAddress, mTpdu,
                mStatusResultNA);

        try {
            Field f = SmsRelayLayer.class.getDeclaredField("mRPMR");
            f.setAccessible(true);
            AtomicInteger rpMR = (AtomicInteger) f.get(mSmsRelayLayer);
            return rpMR.get();
        } catch (Exception e) {
            throw new RuntimeException(e);
        }
    }

    @Test
    public void testNotifyStatusForOutgoingMessage_moSuccess() {
        mProxyListener.notifyStatusForOutgoingMessage(MtsController.MO_SUCCESS, mSmsFormat, 123);
        // Verify no interactions with listener, as it should return early.
        verify(mListener, never()).notifyRLReportIndication(anyInt(), anyInt(), anyInt(),
                anyInt(), anyInt());
    }

    @Test
    public void testNotifyStatusForOutgoingMessage_listenerNull() {
        int messageReference = setupOutgoingMessage();
        mSmsRelayLayer.setListener(null);
        mProxyListener.notifyStatusForOutgoingMessage(MtsController.MO_ERROR_GENERIC, mSmsFormat,
                messageReference);
        // No crash and a log message is expected. No easy way to verify log.
    }

    @Test
    public void testNotifyStatusForOutgoingMessage_invalidMessageReference() {
        setupOutgoingMessage();
        mProxyListener.notifyStatusForOutgoingMessage(MtsController.MO_ERROR_GENERIC, mSmsFormat,
                999); // invalid MR
        verify(mListener, never()).notifyRLReportIndication(anyInt(), anyInt(), anyInt(),
                anyInt(), anyInt());
    }

    @Test
    public void testNotifyStatusForOutgoingMessage_errorGeneric() {
        int messageReference = setupOutgoingMessage();
        mProxyListener.notifyStatusForOutgoingMessage(MtsController.MO_ERROR_GENERIC, mSmsFormat,
                messageReference);
        int tpMR = mTpdu[1] & 0xff;
        verify(mListener).notifyRLReportIndication(eq(mToken), eq(tpMR),
                eq(ImsSmsImplBase.SEND_STATUS_ERROR),
                eq(SmsManager.RESULT_NETWORK_ERROR), eq(0));
    }

    @Test
    public void testNotifyStatusForOutgoingMessage_errorRetry() {
        int messageReference = setupOutgoingMessage();
        mProxyListener.notifyStatusForOutgoingMessage(MtsController.MO_ERROR_RETRY, mSmsFormat,
                messageReference);
        int tpMR = mTpdu[1] & 0xff;
        verify(mListener).notifyRLReportIndication(eq(mToken), eq(tpMR),
                eq(ImsSmsImplBase.SEND_STATUS_ERROR_RETRY),
                eq(SmsManager.RESULT_NETWORK_ERROR), eq(0));
    }

    @Test
    public void testNotifyStatusForOutgoingMessage_errorFallback() {
        int messageReference = setupOutgoingMessage();
        mProxyListener.notifyStatusForOutgoingMessage(MtsController.MO_ERROR_FALLBACK, mSmsFormat,
                messageReference);
        int tpMR = mTpdu[1] & 0xff;
        verify(mListener).notifyRLReportIndication(eq(mToken), eq(tpMR),
                eq(ImsSmsImplBase.SEND_STATUS_ERROR_FALLBACK),
                eq(SmsManager.RESULT_NETWORK_ERROR), eq(0));
    }

    @Test
    public void testNotifyIncomingMessage_listenerNull() {
        mSmsRelayLayer.setListener(null);
        when(mImsCallContext.getSubId()).thenReturn(SUB_ID_1);
        mSmsRelayLayer.mDeliverCause.put(ImsSmsImplBase.DELIVER_STATUS_ERROR_GENERIC,
                SmsRPErrorCause.SMS_PROTOCOL_ERROR.getRPCauseCode());
        mProxyListener.notifyIncomingMessage(mSmsFormat, mMtRpData);

        // When listener is null, sendRPError should be called.
        // It should be an RP-ERROR with cause DELIVER_STATUS_ERROR_GENERIC
        SmsRPdu mtData = new SmsRPdu(mMtRpData);
        int rpCause = mSmsRelayLayer.mDeliverCause.get(
                ImsSmsImplBase.DELIVER_STATUS_ERROR_GENERIC);
        SmsRPdu rpErrorPdu = new SmsRPdu(mtData.getMessageRef(), SmsUtils.RP_ERROR,
                mtData.getOrigAddr(), rpCause, null);
        byte[] expectedRpError = rpErrorPdu.getRpduByteArray();

        verify(mMtsController).sendMessage(eq(mSmsFormat), eq(expectedRpError),
                eq(SMSC_IDENTITY_SUB1.toString()), eq(mtData.getOrigAddr()),
                eq(mtData.getMessageRef()));
    }

    @Test
    public void testNotifyIncomingMessage_rpAckInvalidMessageRef() {
        byte[] mtRpAck = new byte[2];
        mtRpAck[0] = SmsRPdu.MT_RP_ACK_MTI;
        mtRpAck[1] = (byte) 99; // invalid MR
        when(mImsCallContext.getSubId()).thenReturn(SUB_ID_1);
        mSmsRelayLayer.mDeliverCause.put(SmsRelayLayer.DELIVER_STATUS_ERROR_INVALID_MR_VALUE,
                SmsRPErrorCause.SMS_INVALID_MESSAGE_REF.getRPCauseCode());
        mProxyListener.notifyIncomingMessage(mSmsFormat, mtRpAck);

        // Should send RP-Error
        SmsRPdu mtData = new SmsRPdu(mtRpAck);
        int rpCause = mSmsRelayLayer.mDeliverCause.get(
                SmsRelayLayer.DELIVER_STATUS_ERROR_INVALID_MR_VALUE);
        SmsRPdu rpErrorPdu = new SmsRPdu(mtData.getMessageRef(), SmsUtils.RP_ERROR,
                mtData.getOrigAddr(), rpCause, null);
        byte[] expectedRpError = rpErrorPdu.getRpduByteArray();

        verify(mMtsController).sendMessage(eq(mSmsFormat), eq(expectedRpError),
                eq(SMSC_IDENTITY_SUB1.toString()), eq(mtData.getOrigAddr()),
                eq(mtData.getMessageRef()));
    }

    @Test
    public void testNotifyIncomingMessage_rpError() {
        int messageReference = setupOutgoingMessage();

        byte[] mtRpError = new byte[4];
        mtRpError[0] = SmsRPdu.MT_RP_ERROR_MTI;
        mtRpError[1] = (byte) (messageReference & 0xff);
        mtRpError[2] = (byte) 0x01; // Cause length
        mtRpError[3] = (byte) SmsRPErrorCause.SMS_INVALID_MESSAGE_REF.getRPCauseCode();

        mProxyListener.notifyIncomingMessage(mSmsFormat, mtRpError);

        int tpMR = mTpdu[1] & 0xff;
        verify(mListener).notifyRLReportIndication(eq(mToken), eq(tpMR),
                eq(ImsSmsImplBase.SEND_STATUS_ERROR),
                eq(SmsManager.RESULT_INVALID_ARGUMENTS),
                eq(SmsRPErrorCause.SMS_INVALID_MESSAGE_REF.getRPCauseCode()));
    }

    @Test
    public void testNotifyIncomingMessage_rpErrorInvalidMessageRef() {
        setupOutgoingMessage();

        byte[] mtRpError = new byte[4];
        mtRpError[0] = SmsRPdu.MT_RP_ERROR_MTI;
        mtRpError[1] = (byte) 99; // invalid MR
        mtRpError[2] = (byte) 0x01; // Cause length
        mtRpError[3] = (byte) SmsRPErrorCause.SMS_INVALID_MESSAGE_REF.getRPCauseCode();

        mProxyListener.notifyIncomingMessage(mSmsFormat, mtRpError);

        verify(mListener, never()).notifyRLReportIndication(anyInt(), anyInt(), anyInt(),
                anyInt(), anyInt());
    }


    @Test
    public void testSendRpMessage_nullSmsc() {
        int result = mSmsRelayLayer.sendRPMessage(mToken, mRpType, null, mDestinationAddress,
                mTpdu, mStatusResultNA);
        assertEquals(SmsUtils.SMS_RESULT_INVALID_SMSC_ADDRESS, result);
    }

    @Test
    public void testSendRpMessage_rpErrorInvalidToken() {
        int result = mSmsRelayLayer.sendRPMessage(999, SmsUtils.RP_ERROR, mSmsc,
                mDestinationAddress, null, ImsSmsImplBase.DELIVER_STATUS_ERROR_GENERIC);
        assertEquals(SmsUtils.SMSRL_RESULT_TOKEN_DOES_NOT_EXIST, result);
    }

    @Test
    public void testSendRpMessage_invalidRpType() {
        int result = mSmsRelayLayer.sendRPMessage(mToken, 99, mSmsc, mDestinationAddress,
                mTpdu, mStatusResultNA);
        assertEquals(SmsUtils.SMSRL_RESULT_INVALID_RP_MESSAGE_TYPE, result);
    }

    @Test
    public void test_VerifyMoFailure() {
        when(mImsCallContext.getSubId()).thenReturn(SUB_ID_1);
        int messageReference = setupOutgoingMessage();
        int invalidMessageReference = messageReference + 1;
        mSmsRelayLayer.mDeliverCause.put(SmsRelayLayer.DELIVER_STATUS_ERROR_INVALID_MR_VALUE,
                SmsRPErrorCause.SMS_INVALID_MESSAGE_REF.getRPCauseCode());

        // Receive an RP-ACK with an invalid message reference.
        byte[] mtRpAck = new byte[2];
        mtRpAck[0] = SmsRPdu.MT_RP_ACK_MTI;
        mtRpAck[1] = (byte) (invalidMessageReference & 0xff);

        mProxyListener.notifyIncomingMessage(mSmsFormat, mtRpAck);

        // Should send RP-Error for the invalid ACK
        SmsRPdu mtData = new SmsRPdu(mtRpAck);
        int rpCause = mSmsRelayLayer.mDeliverCause.get(
                SmsRelayLayer.DELIVER_STATUS_ERROR_INVALID_MR_VALUE);
        SmsRPdu rpErrorPdu = new SmsRPdu(mtData.getMessageRef(), SmsUtils.RP_ERROR,
                mtData.getOrigAddr(), rpCause, null);
        byte[] expectedRpError = rpErrorPdu.getRpduByteArray();

        verify(mMtsController).sendMessage(eq(mSmsFormat), eq(expectedRpError),
                eq(SMSC_IDENTITY_SUB1.toString()), eq(mtData.getOrigAddr()),
                eq(mtData.getMessageRef()));

        // The state machine should still be waiting for the correct RP-ACK.
        assertEquals(WAIT_FOR_RPACK_FROM_NW, mSmsRelayLayer.mSmsRLStateMachine.getState());

        // Verify original message is not ACKed.
        verify(mListener, never()).notifyRLReportIndication(eq(mToken), anyInt(),
                eq(ImsSmsImplBase.SEND_STATUS_OK), anyInt(), anyInt());
    }

    @Test
    public void test_VerifyMtSuccess_invalidSubId() {
        when(mImsCallContext.getSubId()).thenReturn(-1);
        when(mMtsController.sendMessage(anyInt(), any(), anyString(), anyString(),
                anyInt())).thenReturn(true);

        mProxyListener.notifyIncomingMessage(mSmsFormat, mMtRpData);

        SmsRPdu mtPdu = new SmsRPdu(mMtRpData);
        int token = getMtToken();

        verify(mListener).notifyRLDataIndication(eq(token), eq(mSmsFormat),
                eq(SmsUtils.RP_DATA), any());

        mSmsRelayLayer.sendRPMessage(token, SmsUtils.RP_ACK, null, null,
                null, ImsSmsImplBase.DELIVER_STATUS_OK);

        SmsRPdu moAckPdu = new SmsRPdu(mtPdu.getMessageRef(), SmsUtils.RP_ACK, null, 0, null);
        byte[] expectedMoRpAck = moAckPdu.getRpduByteArray();

        // When subId is invalid, targetAddress and smscAddress should be the original address
        // from the MT message.
        verify(mMtsController).sendMessage(eq(mSmsFormat), eq(expectedMoRpAck),
                eq(mtPdu.getOrigAddr()), eq(mtPdu.getOrigAddr()), eq(mtPdu.getMessageRef()));
    }

    @Test
    public void test_VerifyMultipleMtSuccess() {
        when(mImsCallContext.getSubId()).thenReturn(SUB_ID_1);
        when(mMtsController.sendMessage(anyInt(), any(), anyString(), anyString(),
                anyInt())).thenReturn(true);

        // First MT SMS
        mProxyListener.notifyIncomingMessage(mSmsFormat, mMtRpData);
        SmsRPdu mtPdu1 = new SmsRPdu(mMtRpData);
        int token1 = getMtToken();
        verify(mListener).notifyRLDataIndication(eq(token1), eq(mSmsFormat),
                eq(SmsUtils.RP_DATA), any());

        // Send ACK for first SMS
        mSmsRelayLayer.sendRPMessage(token1, SmsUtils.RP_ACK, null, null,
                                     null, ImsSmsImplBase.DELIVER_STATUS_OK);
        SmsRPdu moAckPdu1 = new SmsRPdu(mtPdu1.getMessageRef(), SmsUtils.RP_ACK, null, 0, null);
        byte[] expectedMoRpAck1 = moAckPdu1.getRpduByteArray();
        String targetAddress = SMSC_IDENTITY_SUB1.toString();
        verify(mMtsController).sendMessage(eq(mSmsFormat), eq(expectedMoRpAck1), eq(targetAddress),
                                             eq(mtPdu1.getOrigAddr()),
                                             eq(mtPdu1.getMessageRef()));

        // Second MT SMS
        byte[] mMtRpData2 = ImsUtils.hexStringToBytes("010207919130079229"
                                                        + "F0001221110A81785634121000000666"
                                                        + "B2996C2603");
        mProxyListener.notifyIncomingMessage(mSmsFormat, mMtRpData2);
        SmsRPdu mtPdu2 = new SmsRPdu(mMtRpData2);
        int token2 = getMtToken();
        verify(mListener).notifyRLDataIndication(eq(token2), eq(mSmsFormat),
                eq(SmsUtils.RP_DATA), any());

        // Send ACK for second SMS
        mSmsRelayLayer.sendRPMessage(token2, SmsUtils.RP_ACK, null, null,
                                     null, ImsSmsImplBase.DELIVER_STATUS_OK);
        SmsRPdu moAckPdu2 = new SmsRPdu(mtPdu2.getMessageRef(), SmsUtils.RP_ACK, null, 0, null);
        byte[] expectedMoRpAck2 = moAckPdu2.getRpduByteArray();
        verify(mMtsController).sendMessage(eq(mSmsFormat), eq(expectedMoRpAck2), eq(targetAddress),
                                             eq(mtPdu2.getOrigAddr()),
                                             eq(mtPdu2.getMessageRef()));
    }

    @Test
    public void test_VerifyMtSuccess() {
        when(mImsCallContext.getSubId()).thenReturn(SUB_ID_1);
        when(mMtsController.sendMessage(anyInt(), any(), anyString(), anyString(),
                anyInt())).thenReturn(true);

        mProxyListener.notifyIncomingMessage(mSmsFormat, mMtRpData);

        SmsRPdu mtPdu = new SmsRPdu(mMtRpData);
        int token = getMtToken();

        verify(mListener).notifyRLDataIndication(eq(token), eq(mSmsFormat),
                eq(SmsUtils.RP_DATA), any());

        mSmsRelayLayer.sendRPMessage(token, SmsUtils.RP_ACK, null, null,
                null, ImsSmsImplBase.DELIVER_STATUS_OK);

        SmsRPdu moAckPdu = new SmsRPdu(mtPdu.getMessageRef(), SmsUtils.RP_ACK, null, 0, null);
        byte[] expectedMoRpAck = moAckPdu.getRpduByteArray();
        String targetAddress = SMSC_IDENTITY_SUB1.toString();

        verify(mMtsController).sendMessage(eq(mSmsFormat), eq(expectedMoRpAck), eq(targetAddress),
                eq(mtPdu.getOrigAddr()), eq(mtPdu.getMessageRef()));
    }

    @After
    public void tearDown() throws Exception {
        mImsCallContext = null;
        mMtsController = null;
        mSmsRelayLayer = null;
        mProxyListener = null;
        AppContext.deinit();
    }
}
