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
import static org.mockito.Mockito.timeout;
import static org.mockito.Mockito.verify;
import static org.mockito.Mockito.when;

import android.telephony.SmsManager;
import android.telephony.ims.stub.ImsSmsImplBase;

import com.android.imsstack.enabler.mts.MtsController;
import com.android.imsstack.imsservice.mmtel.ImsCallContext;
import com.android.imsstack.util.ImsUtils;

import org.junit.After;
import org.junit.Before;
import org.junit.Test;
import org.junit.runner.RunWith;
import org.junit.runners.JUnit4;
import org.mockito.Mock;
import org.mockito.Mockito;

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
    private byte[] mTpdu = ImsUtils.hexStringToBytes("21110A81785634121000000666B2996C2603");
    private byte[] mMtRpData = ImsUtils.hexStringToBytes("010107919130079229"
                                                        + "F0001221110A81785634121000000666"
                                                        + "B2996C2603");

    @Before
    public void setUp() throws Exception {
        mImsCallContext = Mockito.mock(ImsCallContext.class);
        //mImsCallHandler = Mockito.mock(Handler.class);
        mMtsController = Mockito.mock(MtsController.class);
        mSmsRelayLayer = new SmsRelayLayer(mImsCallContext, mMtsController);
        mProxyListener = mSmsRelayLayer.new MtsControllerListenerProxy();
        mListener = Mockito.mock(SmsRelayLayer.Listener.class);
        mSmsRelayLayer.setListener(mListener);
        //when(mImsCallContext.getCallHandler()).thenReturn(mImsCallHandler);
    }

    @Test
    public void test_sendRPMessageDefault() {
        assertEquals(SmsUtils.SMSRL_RESULT_EXCEPTION, mSmsRelayLayer.sendRPMessage(mToken, mRpType,
                mSmsc, mDestinationAddress, mTpdu, mStatusResultNA));
    }

    @Test
    public void test_targetAddress() {
       //TODO: b/242794606 test the targetAddress that needs to be passed to MtsController
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
    /* TODO: adjust unit test caused by API change
    @Test
    public void test_VerifyMoFailure() throws NoSuchFieldException, IllegalAccessException {
        mSmsRelayLayer.sendRPMessage(mToken, mRpType, mSmsc, mDestinationAddress, mTpdu,
                                     mStatusResultNA);
        when(mImsCallContext.getSubId()).thenReturn(-1);
        //fetching RP-MR generated in MO RP-DATA
        Field f = SmsRelayLayer.class.getDeclaredField("mRPMR");
        f.setAccessible(true);
        AtomicInteger rpMR = (AtomicInteger) f.get(mSmsRelayLayer);
        int rpDataMR = rpMR.get() + 1;
        //created corresponding RP-Ack
        byte[] mtRpAck = new byte[2];
        mtRpAck[0] = SmsRPdu.MT_RP_ACK_MTI;
        mtRpAck[1] = (byte) (rpDataMR & 0xff);
        int result = mProxyListener.notifyIncomingMessage(mSmsFormat, mtRpAck);
        assertEquals(mMtsController.MT_FAILURE, result);
    }

    @Test
    public void test_VerifyMtSuccess() throws NoSuchFieldException, IllegalAccessException {
        when(mImsCallContext.getSubId()).thenReturn(-1);
        mProxyListener.notifyIncomingMessage(mSmsFormat, mMtRpData);
        SmsRPdu mtPdu = new SmsRPdu(mMtRpData);
        String origAddr = mtPdu.getOrigAddr();
        //fetching generated token
        Field f = SmsRelayLayer.class.getDeclaredField("mToken");
        f.setAccessible(true);
        AtomicInteger intToken = (AtomicInteger) f.get(mSmsRelayLayer);
        int token = intToken.get();
        int rpDataMR = mMtRpData[1] & 0xff;
        verify(mListener).notifyRLDataIndication(eq(token), eq(mSmsFormat),
                eq(SmsUtils.RP_DATA), eq(mFrameworkPdu));
        mSmsRelayLayer.sendRPMessage(token, SmsUtils.RP_ACK, null, null,
                                     null, ImsSmsImplBase.DELIVER_STATUS_OK);
        verify(mMtsController).sendMessage(eq(mSmsFormat), eq(mMoRpAck), eq(origAddr), eq(origAddr),
                                             eq(rpDataMR));
    }

    @Test
    public void test_VerifyMultipleMtSuccess() throws NoSuchFieldException,
                                                 IllegalAccessException {
        when(mImsCallContext.getSubId()).thenReturn(-1);
        mProxyListener.notifyIncomingMessage(mSmsFormat, mMtRpData);
        SmsRPdu mtPdu = new SmsRPdu(mMtRpData);
        //fetching generated token
        Field f = SmsRelayLayer.class.getDeclaredField("mToken");
        f.setAccessible(true);
        AtomicInteger intToken = (AtomicInteger) f.get(mSmsRelayLayer);
        int token1 = intToken.get();
        int rpDataMR1 = mMtRpData[1] & 0xff;
        mProxyListener.notifyIncomingMessage(mSmsFormat, mMtRpData2);
        int token2 = intToken.get();
        int rpDataMR2 = mMtRpData2[1] & 0xff;
        verify(mListener).notifyRLDataIndication(eq(token1), eq(mSmsFormat),
                eq(SmsUtils.RP_DATA), eq(mFrameworkPdu));
        verify(mListener).notifyRLDataIndication(eq(token2), eq(mSmsFormat),
                eq(SmsUtils.RP_DATA), eq(mFrameworkPdu));
        mSmsRelayLayer.sendRPMessage(token1, SmsUtils.RP_ACK, null, null,
                                     null, ImsSmsImplBase.DELIVER_STATUS_OK);
        byte[] moRpAck1 = new byte[2];
        moRpAck1[0] = SmsRPdu.MO_RP_ACK_MTI;
        moRpAck1[1] = (byte) (rpDataMR1 & 0xff);
        verify(mMtsController).sendMessage(eq(mSmsFormat), eq(moRpAck1), eq(mtPdu.getOrigAddr()),
                                             eq(mtPdu.getOrigAddr()), eq(rpDataMR1));
        mSmsRelayLayer.sendRPMessage(token2, SmsUtils.RP_ACK, mSmsc, mDestinationAddress,
                                     null, ImsSmsImplBase.DELIVER_STATUS_OK);
        byte[] moRpAck2 = new byte[2];
        moRpAck2[0] = SmsRPdu.MO_RP_ACK_MTI;
        moRpAck2[1] = (byte) (rpDataMR2 & 0xff);
        verify(mMtsController).sendMessage(eq(mSmsFormat), eq(moRpAck2), eq(mtPdu.getOrigAddr()),
                                             eq(mtPdu.getOrigAddr()), eq(rpDataMR2));
    }
    */

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

    @Test
    public void test_VerifyMtSuccess_3GPP2() {
        String pduString = "0000021002020702A848D159E24006010008"
                + "2300031010D0011410A48CBB366F418F465C"
                + "7AF4EECE819E7E1C19000306220707183319";
        byte[] pdu = ImsUtils.hexStringToBytes(pduString);
        mProxyListener.notifyIncomingMessage(SmsUtils.FORMAT_INT_3GPP2, pdu);
        int token = 0;
        try {
            Field f = SmsRelayLayer.class.getDeclaredField("mToken");
            f.setAccessible(true);
            AtomicInteger intToken = (AtomicInteger) f.get(mSmsRelayLayer);
            token = intToken.get();
        } catch (Exception e) {
            e.printStackTrace();
            return;
        }
        verify(mListener).notifyRLDataIndication(eq(token), eq(SmsUtils.FORMAT_INT_3GPP2),
                eq(SmsUtils.RP_DATA), eq(pdu));
    }

    @After
    public void tearDown() throws Exception {
        mImsCallContext = null;
        mMtsController = null;
        mSmsRelayLayer = null;
        mProxyListener = null;
    }
}
