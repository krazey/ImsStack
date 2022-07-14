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

import static com.android.imsstack.imsservice.mmtel.sms.SmsRLStateMachine.SmsRLState.WAIT_FOR_RPACK_FROM_NW;

import static org.junit.Assert.assertEquals;
import static org.mockito.ArgumentMatchers.anyInt;
import static org.mockito.ArgumentMatchers.anyString;
import static org.mockito.ArgumentMatchers.eq;
import static org.mockito.Mockito.verify;
import static org.mockito.Mockito.when;

import android.telephony.SmsManager;
import android.telephony.ims.stub.ImsSmsImplBase;
import android.util.Base64;

import com.android.imsstack.enabler.mts.MtsController;
import com.android.imsstack.imsservice.mmtel.ImsCallContext;
import com.android.internal.util.HexDump;

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
    //Handler mImsCallHandler = new Handler(Looper.getMainLooper());
    @Mock
    MtsController mMtsController;
    @Mock
    SmsRelayLayer.Listener mListener;

    private SmsRelayLayer mSmsRelayLayer;
    private SmsRelayLayer.MtsControllerListenerProxy mProxyListener;

    private int mSmsFormat = SmsUtils.FORMAT_INT_3GPP;
    private int mToken = 1;
    private int mRpType = SmsUtils.RP_DATA;
    private String mSmsc = "+19037029920";
    private String mDestinationAddress = "0987654321";
    private byte[] mTpdu = {21, 11, (byte) 0x0A, 81, 78, 56, 34, 12, 10, 00, 00, 06, 66,
            (byte) 0xB2, 99, (byte) 0x6C, 26, 03};
    private byte[] mMtRpData = {01, 01, 07, (byte) 0x91, 91, 30, 07, 92, 29, (byte) 0xF0, 00, 12,
            21, 11, (byte) 0x0A, 81, 78, 56, 34, 12, 10, 00, 00, 06, 66, (byte) 0xB2, 99,
            (byte) 0x6C,
            26, 03};
    private byte[] mMtRpData2 = {01, 02, 07, (byte) 0x91, 91, 30, 07, 92, 29, (byte) 0xF0, 00, 12,
            21, 11, (byte) 0x0A, 81, 78, 56, 34, 12, 10, 00, 00, 06, 66, (byte) 0xB2, 99,
            (byte) 0x6C,
            26, 03};
    //TPDU sent to framework(Smsc Address + mTpdu)
    private byte[] mFrameworkPdu = {07, (byte) 0x91, 91, 30, 07, 92, 29, (byte) 0xF0, 21, 11,
            (byte) 0x0A, 81, 78, 56, 34, 12, 10, 00, 00, 06, 66, (byte) 0xB2, 99, (byte) 0x6C, 26,
            03};
    private byte[] mMoRpData = {00, 01, 00, 07, (byte) 0x91, (byte) 0x91, 0x30, 07,
            (byte) 0x92, 0x29, (byte) 0xF0, 0x12, 21, 11, (byte) 0x0A, 81, 78, 56, 34, 12, 10, 00,
            00,
            06, 66, (byte) 0xB2, 99, (byte) 0x6C, 26, 03};
    private byte[] mMoRpAck = {02, 01};
    private byte[] mMoRpError = {04, 01, 01, 0x6f};
    private byte[] mMtRpAck = {03, 01};
    private byte[] mMtRpError = {05, 01, 01, 0x6f};
    private byte[] mMoRpSmma = {0x06, 0x01};
    private byte[] mMtRpAckFailCase = {03, 02};
    private byte[] mDeliverPdu = {20, 11, (byte) 0x0A, 81, 78, 56, 34, 12, 10, 00, 00, 06, 66,
            (byte) 0xB2, 99, (byte) 0x6C, 26, 03};
    private String mEncodedData = Base64.encodeToString(mMtRpData, Base64.DEFAULT);

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
                mSmsc, mDestinationAddress, mTpdu));
    }

    @Test
    public void test_VerifyMoSuccess()
            throws NoSuchFieldException, IllegalAccessException {
        when(mMtsController.sendMessage(anyInt(), anyString(), anyString(),
                anyInt())).thenReturn(true);
        mSmsRelayLayer.sendRPMessage(mToken, mRpType, mSmsc, mDestinationAddress, mTpdu);
        Field f = SmsRelayLayer.class.getDeclaredField("mRPMR");
        f.setAccessible(true);
        AtomicInteger rpMR = (AtomicInteger) f.get(mSmsRelayLayer);
        int rpDataMR = rpMR.get();
        SmsRPdu pdu = new SmsRPdu(rpDataMR, mRpType, mSmsc, 0, mTpdu);
        byte[] expectedMoRpData = pdu.getRpduByteArray();
        String pdu64 = Base64.encodeToString(expectedMoRpData, Base64.DEFAULT);
        verify(mMtsController).sendMessage(eq(mSmsFormat), eq(pdu64),
                                        eq(mDestinationAddress), eq(rpDataMR));
        assertEquals(WAIT_FOR_RPACK_FROM_NW, mSmsRelayLayer.mSmsRLStateMachine.getState());
        byte[] mtRpAck = new byte[2];
        mtRpAck[0] = SmsRPdu.MT_RP_ACK_MTI;
        mtRpAck[1] = (byte) (rpDataMR & 0xff);
        mProxyListener.notifyIncomingMessage(mSmsFormat,
                                             Base64.encodeToString(mtRpAck, Base64.DEFAULT));
        verify(mListener).notifyRLReportIndication(eq(mToken),
                    eq(ImsSmsImplBase.SEND_STATUS_OK), eq(SmsManager.RESULT_ERROR_NONE));
    }

    @Test
    public void test_VerifyMultipleMoSuccess() throws NoSuchFieldException, IllegalAccessException {

        int rpDataMR1, rpDataMR2;
        SmsRPdu pdu;
        byte[] expectedMoRpData;
        String pdu64;
        when(mMtsController.sendMessage(anyInt(), anyString(), anyString(),
                                        anyInt())).thenReturn(true);
        mSmsRelayLayer.sendRPMessage(1, mRpType, mSmsc, mDestinationAddress, mTpdu);
        //fetching RP-MR generated in MO RP-DATA
        Field f = SmsRelayLayer.class.getDeclaredField("mRPMR");
        f.setAccessible(true);
        AtomicInteger rpMR = (AtomicInteger) f.get(mSmsRelayLayer);
        rpDataMR1 = rpMR.get();
        mSmsRelayLayer.sendRPMessage(2, mRpType, mSmsc, mDestinationAddress, mTpdu);
        rpDataMR2 = rpDataMR1 + 1;
        pdu = new SmsRPdu(rpDataMR1, mRpType, mSmsc, 0, mTpdu);
        expectedMoRpData = pdu.getRpduByteArray();
        pdu64 = Base64.encodeToString(expectedMoRpData, Base64.DEFAULT);
        verify(mMtsController).sendMessage(eq(mSmsFormat), eq(pdu64),
                                           eq(mDestinationAddress), eq(rpDataMR1));
        pdu = new SmsRPdu(rpDataMR2, mRpType, mSmsc, 0, mTpdu);
        expectedMoRpData = pdu.getRpduByteArray();
        pdu64 = Base64.encodeToString(expectedMoRpData, Base64.DEFAULT);
        verify(mMtsController).sendMessage(eq(mSmsFormat), eq(pdu64),
                                           eq(mDestinationAddress), eq(rpDataMR2));
        byte[] mtRpAck1 = new byte[2];
        mtRpAck1[0] = SmsRPdu.MT_RP_ACK_MTI;
        mtRpAck1[1] = (byte) (rpDataMR1 & 0xff);
        mProxyListener.notifyIncomingMessage(mSmsFormat,
                Base64.encodeToString(mtRpAck1, Base64.DEFAULT));
        verify(mListener).notifyRLReportIndication(eq(1), eq(ImsSmsImplBase.SEND_STATUS_OK),
                eq(SmsManager.RESULT_ERROR_NONE));
        byte[] mtRpAck2 = new byte[2];
        mtRpAck2[0] = SmsRPdu.MT_RP_ACK_MTI;
        mtRpAck2[1] = (byte) (rpDataMR2 & 0xff);
        mProxyListener.notifyIncomingMessage(mSmsFormat,
                Base64.encodeToString(mtRpAck2, Base64.DEFAULT));
        verify(mListener).notifyRLReportIndication(eq(2), eq(ImsSmsImplBase.SEND_STATUS_OK),
                eq(SmsManager.RESULT_ERROR_NONE));

    }

    @Test
    public void test_VerifyMoFailure() throws NoSuchFieldException, IllegalAccessException {
        mSmsRelayLayer.sendRPMessage(mToken, mRpType, mSmsc, mDestinationAddress, mTpdu);

        //fetching RP-MR generated in MO RP-DATA
        Field f = SmsRelayLayer.class.getDeclaredField("mRPMR");
        f.setAccessible(true);
        AtomicInteger rpMR = (AtomicInteger) f.get(mSmsRelayLayer);
        int rpDataMR = rpMR.get() + 1;
        //created corresponding RP-Ack
        byte[] mtRpAck = new byte[2];
        mtRpAck[0] = SmsRPdu.MT_RP_ACK_MTI;
        mtRpAck[1] = (byte) (rpDataMR & 0xff);
        int result = mProxyListener.notifyIncomingMessage(mSmsFormat,
                Base64.encodeToString(mtRpAck, Base64.DEFAULT));
        assertEquals(mMtsController.MT_FAILURE, result);

    }

    @Test
    public void test_VerifyMtSuccess() throws NoSuchFieldException, IllegalAccessException {
        mProxyListener.notifyIncomingMessage(mSmsFormat,
                Base64.encodeToString(mMtRpData, Base64.DEFAULT));

        //fetching generated token
        Field f = SmsRelayLayer.class.getDeclaredField("mToken");
        f.setAccessible(true);
        AtomicInteger intToken = (AtomicInteger) f.get(mSmsRelayLayer);
        int token = intToken.get();
        int rpDataMR = mMtRpData[1] & 0xff;
        verify(mListener).notifyRLDataIndication(eq(token), eq(mSmsFormat),
                eq(SmsUtils.RP_DATA), eq(mFrameworkPdu));
        mSmsRelayLayer.sendRPMessage(token, SmsUtils.RP_ACK, mSmsc, mDestinationAddress, null);
        String pdu64 = Base64.encodeToString(mMoRpAck, Base64.DEFAULT);
        verify(mMtsController).sendMessage(eq(mSmsFormat), eq(pdu64), eq(mSmsc), eq(rpDataMR));

    }

    @Test
    public void test_VerifyMultipleMtSuccess() throws NoSuchFieldException, IllegalAccessException {
        mProxyListener.notifyIncomingMessage(mSmsFormat,
                Base64.encodeToString(mMtRpData, Base64.DEFAULT));

        //fetching generated token
        Field f = SmsRelayLayer.class.getDeclaredField("mToken");
        f.setAccessible(true);
        AtomicInteger intToken = (AtomicInteger) f.get(mSmsRelayLayer);
        int token1 = intToken.get();
        int rpDataMR1 = mMtRpData[1] & 0xff;
        mProxyListener.notifyIncomingMessage(mSmsFormat,
                Base64.encodeToString(mMtRpData2, Base64.DEFAULT));
        int token2 = intToken.get();
        int rpDataMR2 = mMtRpData2[1] & 0xff;
        verify(mListener).notifyRLDataIndication(eq(token1), eq(mSmsFormat),
                eq(SmsUtils.RP_DATA), eq(mFrameworkPdu));
        verify(mListener).notifyRLDataIndication(eq(token2), eq(mSmsFormat),
                eq(SmsUtils.RP_DATA), eq(mFrameworkPdu));
        mSmsRelayLayer.sendRPMessage(token1, SmsUtils.RP_ACK, mSmsc, mDestinationAddress, null);
        byte[] moRpAck1 = new byte[2];
        moRpAck1[0] = SmsRPdu.MO_RP_ACK_MTI;
        moRpAck1[1] = (byte) (rpDataMR1 & 0xff);
        String pdu64 = Base64.encodeToString(moRpAck1, Base64.DEFAULT);
        verify(mMtsController).sendMessage(eq(mSmsFormat), eq(pdu64), eq(mSmsc), eq(rpDataMR1));
        mSmsRelayLayer.sendRPMessage(token2, SmsUtils.RP_ACK, mSmsc, mDestinationAddress, null);
        byte[] moRpAck2 = new byte[2];
        moRpAck2[0] = SmsRPdu.MO_RP_ACK_MTI;
        moRpAck2[1] = (byte) (rpDataMR2 & 0xff);
        pdu64 = Base64.encodeToString(moRpAck2, Base64.DEFAULT);
        verify(mMtsController).sendMessage(eq(mSmsFormat), eq(pdu64), eq(mSmsc), eq(rpDataMR2));
    }

    @Test
    public void test_VerifyMtAckWithInvalidToken() {
        mProxyListener.notifyIncomingMessage(mSmsFormat,
                Base64.encodeToString(mMtRpData, Base64.DEFAULT));
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
                    mDestinationAddress, mTpdu);
        assertEquals(SmsUtils.SMSRL_RESULT_TOKEN_DOES_NOT_EXIST, result);
    }

    @Test
    public void test_VerifyMtSuccess_3GPP2() {
        String pduString = "0000021002020702A848D159E24006010008"
                + "2300031010D0011410A48CBB366F418F465C"
                + "7AF4EECE819E7E1C19000306220707183319";
        byte[] pdu = HexDump.hexStringToByteArray(pduString);
        mProxyListener.notifyIncomingMessage(SmsUtils.FORMAT_INT_3GPP2,
                Base64.encodeToString(pdu, Base64.DEFAULT));
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
