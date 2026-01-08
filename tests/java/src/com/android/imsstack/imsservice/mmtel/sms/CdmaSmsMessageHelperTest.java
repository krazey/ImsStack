/*
 * Copyright (C) 2025 The Android Open Source Project
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

import com.android.imsstack.util.ImsUtils;

import org.junit.Before;
import org.junit.Test;
import org.junit.runner.RunWith;
import org.junit.runners.JUnit4;

@RunWith(JUnit4.class)
public class CdmaSmsMessageHelperTest {

    String mPduString =
            "00"    // Message Type : SMS Point to Point
                    + "00021002"  // TeleService (TLV format)
                    + "01020000"  // Service Category (TLV format)
                    + "02070282055950C840" // Destination SubAddress (TLV format)
                    + "08100003101460010610268D2285000A0100" // Bearer Data (TLV format)
                    + "0303001120"; // Originating SubAddress (TLV format)

    byte[] mPdu = ImsUtils.hexStringToBytes(mPduString);
    String mExpectedPduString =
            "00000000"  // Message Type : SMS Point to Point
                + "00001002"    // TeleService
                + "00000000"    // Service Category
                + "00"  // Digit Mode
                + "00"  // Number Mode
                + "00"  // Ton
                + "00"  // Number Plan
                + "0A"  // Number of digits
                + "44383135363534333231"    // Destination SubAddress
                + "00000000"    // Bearer Reply
                + "00"  // Reply Sequence Number
                + "00"  // Error Class
                + "00"  // Cause Code
                + "00000010" // Bearer Data Length
                + "0003101460010610268D2285000A0100";   // Bearer Data
    private CdmaSmsMessageHelper mHelper;

    @Before
    public void setup() {
        mHelper = new CdmaSmsMessageHelper();
    }

    @Test
    public void test_parseCdmaPdu_messageType() {
        byte[] pdu = ImsUtils.hexStringToBytes("00");
        mHelper.parseCdmaPdu(pdu);
        assertEquals(0x00, mHelper.mMessageType);
    }

    @Test
    public void test_parseCdmaPdu_teleservice() {
        byte[] pdu = ImsUtils.hexStringToBytes("0000021002");
        mHelper.parseCdmaPdu(pdu);
        assertEquals(0, mHelper.mMessageType);
        assertEquals(0x1002, mHelper.mTeleService);
    }

    @Test
    public void test_parseCdmaPdu_serviceCategory() {
        byte[] pdu = ImsUtils.hexStringToBytes("0001021003");
        mHelper.parseCdmaPdu(pdu);
        assertEquals(0, mHelper.mMessageType);
        assertEquals(0x1003, mHelper.mServiceCategory);
    }

    @Test
    public void test_parseCdmaPdu_smsAddress_8bit() {
        byte[] pdu = ImsUtils.hexStringToBytes("0002078282055950C840");
        mHelper.parseCdmaPdu(pdu);
        assertEquals(0, mHelper.mMessageType);
        assertEquals(1, mHelper.mSmsAddr.digitMode);
        assertEquals(0, mHelper.mSmsAddr.numberMode);
        assertEquals(4, mHelper.mSmsAddr.numberOfDigits);
        assertEquals(5, mHelper.mSmsAddr.numberPlan);
        assertEquals(0, mHelper.mSmsAddr.ton);
        assertEquals("0AB2A190", ImsUtils.bytesToHexString(mHelper.mSmsAddr.origBytes));
    }

    @Test
    public void test_parseCdmaPdu_smsAddress_4bit() {
        byte[] pdu = ImsUtils.hexStringToBytes("0002070282055950C840");
        mHelper.parseCdmaPdu(pdu);
        assertEquals(0, mHelper.mMessageType);
        assertEquals(0, mHelper.mSmsAddr.digitMode);
        assertEquals(0, mHelper.mSmsAddr.numberMode);
        assertEquals(10, mHelper.mSmsAddr.numberOfDigits);
        assertEquals(0, mHelper.mSmsAddr.numberPlan);
        assertEquals(0, mHelper.mSmsAddr.ton);
        assertEquals("44383135363534333231", ImsUtils.bytesToHexString(mHelper.mSmsAddr.origBytes));

        pdu = ImsUtils.hexStringToBytes("000207029E6AF378CB80");
        mHelper.parseCdmaPdu(pdu);
        assertEquals("3739302A234142333242", ImsUtils.bytesToHexString(mHelper.mSmsAddr.origBytes));
    }

    @Test
    public void test_parseCdmaPdu_smsAddress_8bit_dataNetwork() {
        byte[] pdu = ImsUtils.hexStringToBytes("000207C022055950C840");
        mHelper.parseCdmaPdu(pdu);
        assertEquals(0, mHelper.mMessageType);
        assertEquals(1, mHelper.mSmsAddr.digitMode);
        assertEquals(1, mHelper.mSmsAddr.numberMode);
        assertEquals(4, mHelper.mSmsAddr.numberOfDigits);
        assertEquals(0, mHelper.mSmsAddr.numberPlan);
        assertEquals(0, mHelper.mSmsAddr.ton);
    }

    @Test
    public void test_parseCdmaPdu_bearerReply() {
        byte[] pdu = ImsUtils.hexStringToBytes("0006011F");
        mHelper.parseCdmaPdu(pdu);
        assertEquals(7, mHelper.mBearerReply);
    }

    @Test
    public void test_parseCdmaPdu_causeCode() {
        byte[] pdu = ImsUtils.hexStringToBytes("0007021205");
        mHelper.parseCdmaPdu(pdu);
        assertEquals(16, mHelper.mReplySeqNo);
        assertEquals(5, mHelper.mCauseCode);
    }

    @Test
    public void test_formatpdu() {
        mHelper.parseCdmaPdu(mPdu);
        byte[] frameworkPdu = mHelper.formatPdu();
        assertEquals(mExpectedPduString, ImsUtils.bytesToHexString(frameworkPdu));
    }
}
