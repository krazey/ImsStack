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

import org.junit.After;
import org.junit.Before;
import org.junit.Test;
import org.junit.runner.RunWith;
import org.junit.runners.JUnit4;

@RunWith(JUnit4.class)
public class SmsRPduTest {
    private static final String LOG_TAG = "SmsRPduTest";
    private SmsRPdu mMoRpData;
    private SmsRPdu mMoRpAck;
    private SmsRPdu mMoRpError;
    private SmsRPdu mMoRpSmma;
    private SmsRPdu mExpectedMtRpData;
    private SmsRPdu mExpectedMtRpAck;
    private SmsRPdu mExpectedMtRpError;

    private int mMessageRef = 0x01;
    private int mMessageType; // = SmsUtils.RP_DATA;
    private String mDestinationAddress = "+19037029920";
    private String mOrigAddr = null;
    private int mCause = 0;
    private byte[] mUserData = {21, 11, (byte) 0x0A, 81, 78, 56, 34, 12, 10, 00, 00, 06, 66,
            (byte) 0xB2, 99, (byte) 0x6C, 26, 03};
    private byte[] mMoExpectedRpData = {00, 01, 00, 07, (byte) 0x91, (byte) 0x91, 0x30, 07,
            (byte) 0x92, 0x29, (byte) 0xF0, 0x12, 21, 11, (byte) 0x0A, 81, 78, 56, 34, 12, 10, 00,
            00, 06, 66, (byte) 0xB2, 99, (byte) 0x6C, 26, 03};
    private byte[] mMoExpectedRpAck = {02, 01};
    private byte[] mMoExpectedRpError = {04, 01, 01, 0x6f};

    private byte[] mMtRpData = {01, 01, 07, (byte) 0x91, 91, 30, 07, 92, 29, (byte) 0xF0, 00, 12,
            21, 11, (byte) 0x0A, 81, 78, 56, 34, 12, 10, 00, 00, 06, 66, (byte) 0xB2, 99,
            (byte) 0x6C, 26, 03};
    private byte[] mMtRpAck = {03, 01};
    private byte[] mMtRpError = {05, 01, 01, 0x6f};
    private byte[] mMoExpectedRpSmma = {0x06, 0x01};

    @Before
    public void setUp() throws Exception {
        mMoRpData = new SmsRPdu(mMessageRef, SmsUtils.RP_DATA, mDestinationAddress, mCause,
                mUserData);
        mMoRpAck = new SmsRPdu(mMessageRef, SmsUtils.RP_ACK, null, 0, null);
        mMoRpError = new SmsRPdu(mMessageRef, SmsUtils.RP_ERROR, null,
                SmsRPdu.RP_CAUSE_PROTOCOL_UNSPECIFIED, null);
        mMoRpSmma = new SmsRPdu(mMessageRef, SmsUtils.RP_SMMA, null, 0, null);
        mExpectedMtRpData = new SmsRPdu(mMtRpData);
        mExpectedMtRpAck = new SmsRPdu(mMtRpAck);
        mExpectedMtRpError = new SmsRPdu(mMtRpError);
    }

    @Test
    public void test_getMessageType() {
        assertEquals(SmsUtils.RP_DATA, mExpectedMtRpData.getMessageType());
        assertEquals(SmsUtils.RP_ACK, mExpectedMtRpAck.getMessageType());
        assertEquals(SmsUtils.RP_ERROR, mExpectedMtRpError.getMessageType());
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
        assertEquals(mMessageRef, mExpectedMtRpData.getMessageRef());
        assertEquals(mMessageRef, mExpectedMtRpAck.getMessageRef());
        assertEquals(mMessageRef, mExpectedMtRpError.getMessageRef());
    }

    @Test
    public void test_getUserData() {
        assertEquals(mUserData, mMoRpData.getUserData());
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

    @After
    public void tearDown() throws Exception {
        mExpectedMtRpData = null;
        mExpectedMtRpAck = null;
        mExpectedMtRpError = null;
        mMoExpectedRpData = null;
        mMoExpectedRpAck = null;
        mMoExpectedRpError = null;
        mMoExpectedRpSmma = null;
    }
}
