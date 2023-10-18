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
import static org.junit.Assert.assertNull;

import com.android.internal.util.HexDump;

import org.junit.After;
import org.junit.Before;
import org.junit.Test;
import org.junit.runner.RunWith;
import org.junit.runners.JUnit4;

@RunWith(JUnit4.class)
public class SmsRPduTest {
    private SmsRPdu mMoRpData;
    private SmsRPdu mMoRpAck;
    private SmsRPdu mMoRpError;
    private SmsRPdu mMoRpSmma;
    private SmsRPdu mMoRpAckWithUD;
    private SmsRPdu mSmsRPduMtRpData;
    private SmsRPdu mSmsRPduMtRpAck;
    private SmsRPdu mSmsRPduMtRpError;
    private SmsRPdu mSmsRPduMtRpAckWithRpUserData;
    private SmsRPdu mSmsRPduMtRpErrorWithRpUserData;;
    private SmsRPdu mSmsRPduMtRpAckwithInvalidIEI;
    private SmsRPdu mSmsRPduMtRpErrorWithDiagnostics;

    private int mMessageRef = 0x01;
    private String mDestinationAddress = "07919130079229F0";
    private String mOrigAddr = null;
    private int mCause = 0;
    private byte[] mUserData = HexDump.hexStringToByteArray("21110A81785634121000"
                                                                + "000666B2996C2603");
    private byte[] mMoExpectedRpData = HexDump.hexStringToByteArray("00010007919130079229F0"
                                                        + "1221110A81785634121000000666B2996C2603");
    private byte[] mMoExpectedRpAck = HexDump.hexStringToByteArray("0201");
    private byte[] mMoExpectedRpError = HexDump.hexStringToByteArray("0401016f");
    private byte[] mMtRpData = HexDump.hexStringToByteArray("010107919130079229F000122111"
                                                            + "0A81785634121000000666B2996C2603");
    private byte[] mMtRpAck = HexDump.hexStringToByteArray("0301");
    private byte[] mMtRpError = HexDump.hexStringToByteArray("0501016f");
    private byte[] mMtRpAckWithRpUserData = HexDump.hexStringToByteArray("0301410A01"
                                                                        + "C20022905002058322");
    private byte[] mMtRpErrorWithRpUserData = HexDump.hexStringToByteArray("0509016F410A"
                                                                        + "01C20022905002058322");
    private byte[] mMtRpErrorWithDiagnostics = HexDump.hexStringToByteArray("0509036F6F6F"
                                                                            + "410A01C200229"
                                                                            + "05002058322");
    private byte[] mMtRpAckWithoutIEI = HexDump.hexStringToByteArray("03010A01"
                                                                    + "C20022905002058322");
    private byte[] mMtRpUserData = HexDump.hexStringToByteArray("01C20022905002058322");
    private byte[] mMoExpectedRpSmma = HexDump.hexStringToByteArray("0601");
    private byte[] mMoExpectedRpAckWithUserData = HexDump.hexStringToByteArray("020141020000");
    private byte[] mUserDataForRPAck = HexDump.hexStringToByteArray("0000");

    @Before
    public void setUp() throws Exception {
        mMoRpData = new SmsRPdu(mMessageRef, SmsUtils.RP_DATA, mDestinationAddress, mCause,
                mUserData);
        mMoRpAck = new SmsRPdu(mMessageRef, SmsUtils.RP_ACK, null, 0, null);
        mMoRpError = new SmsRPdu(mMessageRef, SmsUtils.RP_ERROR, null,
                SmsRPdu.RP_CAUSE_PROTOCOL_UNSPECIFIED, null);
        mMoRpSmma = new SmsRPdu(mMessageRef, SmsUtils.RP_SMMA, null, 0, null);
        mMoRpAckWithUD = new SmsRPdu(mMessageRef, SmsUtils.RP_ACK, null, 0, mUserDataForRPAck);
        mSmsRPduMtRpData = new SmsRPdu(mMtRpData);
        mSmsRPduMtRpAck = new SmsRPdu(mMtRpAck);
        mSmsRPduMtRpError = new SmsRPdu(mMtRpError);
        mSmsRPduMtRpAckWithRpUserData = new SmsRPdu(mMtRpAckWithRpUserData);
        mSmsRPduMtRpErrorWithRpUserData = new SmsRPdu(mMtRpErrorWithRpUserData);
        mSmsRPduMtRpAckwithInvalidIEI = new SmsRPdu(mMtRpAckWithoutIEI);
        mSmsRPduMtRpErrorWithDiagnostics = new SmsRPdu(mMtRpErrorWithDiagnostics);
    }

    @After
    public void tearDown() throws Exception {
        mSmsRPduMtRpData = null;
        mSmsRPduMtRpAck = null;
        mSmsRPduMtRpAck = null;
        mMoExpectedRpData = null;
        mMoExpectedRpAck = null;
        mMoExpectedRpError = null;
        mMoExpectedRpSmma = null;
    }

    @Test
    public void test_getMessageType() {
        assertEquals(SmsUtils.RP_DATA, mSmsRPduMtRpData.getMessageType());
        assertEquals(SmsUtils.RP_ACK, mSmsRPduMtRpAck.getMessageType());
        assertEquals(SmsUtils.RP_ERROR, mSmsRPduMtRpError.getMessageType());
    }

    @Test
    public void test_getOrigAddr() {
        assertEquals(mOrigAddr, mMoRpData.getOrigAddr());
    }

    @Test
    public void test_getDestAddr() {
        assertEquals(mDestinationAddress, mMoRpData.getDestAddr());
    }

    @Test
    public void test_getMessageRef() {
        assertEquals(mMessageRef, mSmsRPduMtRpData.getMessageRef());
        assertEquals(mMessageRef, mSmsRPduMtRpAck.getMessageRef());
        assertEquals(mMessageRef, mSmsRPduMtRpError.getMessageRef());
    }

    @Test
    public void test_getUserData() {
        byte[] actual = mMoRpData.getUserData();
        for (int i = 0; i < mUserData.length; i++) {
            assertEquals(mUserData[i], actual[i]);
        }

        actual = mSmsRPduMtRpData.getUserData();
        for (int i = 0; i < mUserData.length; i++) {
            assertEquals(mUserData[i], actual[i]);
        }

        actual = mSmsRPduMtRpAckWithRpUserData.getUserData();
        for (int i = 0; i < mMtRpUserData.length; i++) {
            assertEquals(mMtRpUserData[i], actual[i]);
        }

        actual = mSmsRPduMtRpErrorWithRpUserData.getUserData();
        for (int i = 0; i < mMtRpUserData.length; i++) {
            assertEquals(mMtRpUserData[i], actual[i]);
        }

        actual = mSmsRPduMtRpErrorWithDiagnostics.getUserData();
        for (int i = 0; i < mMtRpUserData.length; i++) {
            assertEquals(mMtRpUserData[i], actual[i]);
        }

        assertNull(mSmsRPduMtRpAckwithInvalidIEI.getUserData());
    }

    @Test
    public void test_getRPCause() {
        assertEquals(SmsRPdu.RP_CAUSE_PROTOCOL_UNSPECIFIED, mSmsRPduMtRpError.getRPCause());
        assertEquals(SmsRPdu.RP_CAUSE_PROTOCOL_UNSPECIFIED,
                        mSmsRPduMtRpErrorWithRpUserData.getRPCause());
        assertEquals(SmsRPdu.RP_CAUSE_PROTOCOL_UNSPECIFIED,
                        mSmsRPduMtRpErrorWithDiagnostics.getRPCause());
    }

    @Test
    public void test_getRpDataPdu() {
        byte [] actual = mMoRpData.getRpduByteArray();
        for (int i = 0; i < mMoExpectedRpData.length; i++) {
            assertEquals(mMoExpectedRpData[i], actual[i]);
        }
    }

    @Test
    public void test_getRpAckPdu() {
        byte[] actual = mMoRpAck.getRpduByteArray();
        for (int i = 0; i < mMoExpectedRpAck.length; i++) {
            assertEquals(mMoExpectedRpAck[i], actual[i]);
        }
    }

    @Test
    public void test_getRpErrorPdu() {
        byte [] actual = mMoRpError.getRpduByteArray();
        for (int i = 0; i < mMoExpectedRpError.length; i++) {
            assertEquals(mMoExpectedRpError[i], actual[i]);
        }
    }

    @Test
    public void test_getRpSmmaPdu() {
        byte[] actual = mMoRpSmma.getRpduByteArray();
        for (int i = 0; i < mMoExpectedRpSmma.length; i++) {
            assertEquals(mMoExpectedRpSmma[i], actual[i]);
        }
    }

    @Test
    public void test_getRpDataPdu_Null() {
        mDestinationAddress = "";
        SmsRPdu smsRPdu = new SmsRPdu(mMessageRef, SmsUtils.RP_DATA, mDestinationAddress, mCause,
                mUserData);
        byte[] actual = smsRPdu.getRpduByteArray();
        assertNull(actual);
    }

    @Test
    public void test_getRpAckPduWithUserData() {
        byte[] actual = mMoRpAckWithUD.getRpduByteArray();
        for (int i = 0; i < mMoExpectedRpAckWithUserData.length; i++) {
            assertEquals(mMoExpectedRpAckWithUserData[i], actual[i]);
        }
    }
}
