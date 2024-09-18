/*
 * Copyright (C) 2024 The Android Open Source Project
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

import com.android.imsstack.util.ImsLog;
import com.android.internal.telephony.cdma.sms.CdmaSmsAddress;
import com.android.internal.telephony.cdma.sms.CdmaSmsSubaddress;
import com.android.internal.telephony.cdma.sms.SmsEnvelope;
import com.android.internal.util.BitwiseInputStream;

import java.io.BufferedOutputStream;
import java.io.ByteArrayInputStream;
import java.io.ByteArrayOutputStream;
import java.io.DataInputStream;
import java.io.DataOutputStream;
import java.io.IOException;

/**
 * Sms Message in CDMA Format
 */
public class CdmaSmsMessageHelper {

	private static final String TAG = "[CdmaSmsMessageHelper] ";

    //Parameter Identifiers as per section 3.4.2.1 of 3GPP2 C.S0015
    private final static byte TELESERVICE_IDENTIFIER                    = 0x00;
    private final static byte SERVICE_CATEGORY                          = 0x01;
    private final static byte ORIGINATING_ADDRESS                       = 0x02;
    private final static byte ORIGINATING_SUB_ADDRESS                   = 0x03;
    private final static byte BEARER_REPLY_OPTION                       = 0x06;
    private final static byte CAUSE_CODES                               = 0x07;
    private final static byte BEARER_DATA                               = 0x08;

    /**
     * Provides the type of a SMS message like point to point, broadcast or acknowledge
     */
    public int mMessageType;

    /**
     * The 16-bit Teleservice parameter identifies which upper layer service access point is sending
     * or receiving the message.
     * (See 3GPP2 C.S0015-B, v2, 3.4.3.1)
     */
    public int mTeleService = SmsEnvelope.TELESERVICE_NOT_SET;

    /**
     * The 16-bit service category parameter identifies the type of service provided
     * by the SMS message.
     * (See 3GPP2 C.S0015-B, v2, 3.4.3.2)
     */
    public int mServiceCategory;

    /**
     * The 6-bit bearer reply parameter is used to request the return of a
     * SMS Acknowledge Message.
     * (See 3GPP2 C.S0015-B, v2, 3.4.3.5)
     */
    public int mBearerReply;

    /**
     * Cause Code values:
     * The cause code parameters are an indication whether an SMS error has occurred and if so,
     * whether the condition is considered temporary or permanent.
     * ReplySeqNo 6-bit value,
     * ErrorClass 2-bit value,
     * CauseCode 0-bit or 8-bit value
     * (See 3GPP2 C.S0015-B, v2, 3.4.3.6)
     */
    public byte mReplySeqNo;
    public byte mErrorClass;
    public byte mCauseCode;

    /**
     * encoded bearer data
     * (See 3GPP2 C.S0015-B, v2, 3.4.3.7)
     */
    public byte[] mBearerData;

    private SmsAddress mSmsAddr;

    private static class SmsAddress {
        /**
         * Digit Mode Indicator is a 1-bit value that indicates whether
         * the address digits are 4-bit DTMF codes or 8-bit codes.  (See
         * 3GPP2 C.S0015-B, v2, 3.4.3.3)
         */
        public int digitMode;

        /**
         * Number Mode Indicator is 1-bit value that indicates whether the
         * address type is a data network address or not.  (See 3GPP2
         * C.S0015-B, v2, 3.4.3.3)
         */
        public int numberMode;

        /**
         * This field shall be set to the number of address digits
         * (See 3GPP2 C.S0015-B, v2, 3.4.3.3)
         */
        public int numberOfDigits;

        /**
         * Numbering Plan identification is a 0 or 4-bit value that
         * indicates which numbering plan identification is set.  (See
         * 3GPP2, C.S0015-B, v2, 3.4.3.3 and C.S005-D, table2.7.1.3.2.4-3)
         */
        public int numberPlan;

        /**
         * Number Types for data networks.
         * (See 3GPP2 C.S005-D, table2.7.1.3.2.4-2 for complete table)
         * (See 3GPP2 C.S0015-B, v2, 3.4.3.3 for data network subset)
         */
        public int ton;

        public byte[] origBytes;
    }

    /**
     * Converts a 4-Bit DTMF encoded symbol from the calling address number to ASCII character
     */
    private byte convertDtmfToAscii(byte dtmfDigit) {
        byte asciiDigit;

        switch (dtmfDigit) {
        case  0: asciiDigit = 68; break; // 'D'
        case  1: asciiDigit = 49; break; // '1'
        case  2: asciiDigit = 50; break; // '2'
        case  3: asciiDigit = 51; break; // '3'
        case  4: asciiDigit = 52; break; // '4'
        case  5: asciiDigit = 53; break; // '5'
        case  6: asciiDigit = 54; break; // '6'
        case  7: asciiDigit = 55; break; // '7'
        case  8: asciiDigit = 56; break; // '8'
        case  9: asciiDigit = 57; break; // '9'
        case 10: asciiDigit = 48; break; // '0'
        case 11: asciiDigit = 42; break; // '*'
        case 12: asciiDigit = 35; break; // '#'
        case 13: asciiDigit = 65; break; // 'A'
        case 14: asciiDigit = 66; break; // 'B'
        case 15: asciiDigit = 67; break; // 'C'
        default:
            asciiDigit = 32; // Invalid DTMF code
            break;
        }

        return asciiDigit;
    }

    private static void log(String s) {
        ImsLog.d(TAG + s);
    }

    private static void loge(String s) {
        ImsLog.e(TAG + s);
    }

}
