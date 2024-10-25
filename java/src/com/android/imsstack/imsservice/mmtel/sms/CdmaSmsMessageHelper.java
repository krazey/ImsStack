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
     * Decodes 3GPP2 As per 3GPP2 C.S0015-0
     */
    public void parseCdmaPdu(byte[] pdu) {
        ByteArrayInputStream bais = new ByteArrayInputStream(pdu);
        DataInputStream dis = new DataInputStream(bais);
        CdmaSmsSubaddress subAddr = new CdmaSmsSubaddress();
        mSmsAddr = new SmsAddress();

        try {
            mMessageType = dis.readByte();

            while (dis.available() > 0) {
                int parameterId = dis.readByte();
                int parameterLen = dis.readUnsignedByte();
                byte[] parameterData = new byte[parameterLen];

                switch (parameterId) {
                    case TELESERVICE_IDENTIFIER:
                        /*
                         * 16 bit parameter that identifies which upper layer
                         * service access point is sending or should receive
                         * this message
                         */
                        mTeleService = dis.readUnsignedShort();
                        log("teleservice = " + mTeleService);
                        break;
                    case SERVICE_CATEGORY:
                        /*
                         * 16 bit parameter that identifies type of service as
                         * in 3GPP2 C.S0015-0 Table 3.4.3.2-1
                         */
                        mServiceCategory = dis.readUnsignedShort();
                        break;
                    case ORIGINATING_ADDRESS:
                        dis.read(parameterData, 0, parameterLen);
                        BitwiseInputStream addrBis = new BitwiseInputStream(parameterData);
                        mSmsAddr.digitMode = addrBis.read(1);
                        mSmsAddr.numberMode = addrBis.read(1);
                        int numberType = 0;
                        if (mSmsAddr.digitMode == CdmaSmsAddress.DIGIT_MODE_8BIT_CHAR) {
                            numberType = addrBis.read(3);
                            mSmsAddr.ton = numberType;

                            if (mSmsAddr.numberMode == CdmaSmsAddress.NUMBER_MODE_NOT_DATA_NETWORK)
                                mSmsAddr.numberPlan = addrBis.read(4);
                        }

                        mSmsAddr.numberOfDigits = addrBis.read(8);

                        byte[] data = new byte[mSmsAddr.numberOfDigits];
                        byte b = 0x00;

                        if (mSmsAddr.digitMode == CdmaSmsAddress.DIGIT_MODE_4BIT_DTMF) {
                            /* As per 3GPP2 C.S0005-0 Table 2.7.1.3.2.4-4 */
                            for (int index = 0; index < mSmsAddr.numberOfDigits; index++) {
                                b = (byte) (0xF & addrBis.read(4));
                                // convert the value if it is 4-bit DTMF to 8
                                // bit
                                data[index] = convertDtmfToAscii(b);
                            }
                        } else if (mSmsAddr.digitMode == CdmaSmsAddress.DIGIT_MODE_8BIT_CHAR) {
                            if (mSmsAddr.numberMode == CdmaSmsAddress.NUMBER_MODE_NOT_DATA_NETWORK) {
                                for (int index = 0; index < mSmsAddr.numberOfDigits; index++) {
                                    b = (byte) (0xFF & addrBis.read(8));
                                    data[index] = b;
                                }

                            } else if (mSmsAddr.numberMode == CdmaSmsAddress.NUMBER_MODE_DATA_NETWORK) {
                                if (numberType == 2)
                                    loge("TODO: Addr is email id");
                                else
                                    loge("TODO: Addr is data network address");
                            } else {
                                loge("Addr is of incorrect type");
                            }
                        } else {
                            loge("Incorrect Digit mode");
                        }
                        mSmsAddr.origBytes = data;
                        break;
                    case ORIGINATING_SUB_ADDRESS:
                        dis.read(parameterData, 0, parameterLen);
                        BitwiseInputStream subAddrBis = new BitwiseInputStream(parameterData);
                        subAddr.type = subAddrBis.read(3);
                        subAddr.odd = subAddrBis.readByteArray(1)[0];
                        int subAddrLen = subAddrBis.read(8);
                        byte[] subdata = new byte[subAddrLen];
                        for (int index = 0; index < subAddrLen; index++) {
                            b = (byte) (0xFF & subAddrBis.read(4));
                            // convert the value if it is 4-bit DTMF to 8 bit
                            subdata[index] = convertDtmfToAscii(b);
                        }
                        subAddr.origBytes = subdata;
                        break;
                    case BEARER_REPLY_OPTION:
                        dis.read(parameterData, 0, parameterLen);
                        BitwiseInputStream replyOptBis = new BitwiseInputStream(parameterData);
                        mBearerReply = replyOptBis.read(6);
                        break;
                    case CAUSE_CODES:
                        dis.read(parameterData, 0, parameterLen);
                        BitwiseInputStream ccBis = new BitwiseInputStream(parameterData);
                        mReplySeqNo = ccBis.readByteArray(6)[0];
                        mErrorClass = ccBis.readByteArray(2)[0];
                        if (mErrorClass != 0x00)
                            mCauseCode = ccBis.readByteArray(8)[0];
                        break;
                    case BEARER_DATA:
                        dis.read(parameterData, 0, parameterLen);
                        mBearerData = parameterData;
                        break;
                    default:
                        throw new Exception("unsupported parameterId (" + parameterId + ")");
                }
            }
            bais.close();
            dis.close();
        } catch (Exception ex) {
            loge("parsePduFromEfRecord: conversion from pdu to SmsMessage failed" + ex);
        }

    }

    /**
     * Formats Tpdu as per 3GPP2 C.S0023 section 3.4.27 which is compatible with
     * framework's CDMA SMS parser
     */
    public byte[] formatPdu() {
        ByteArrayOutputStream baos = new ByteArrayOutputStream(100);
        DataOutputStream dos = new DataOutputStream(new BufferedOutputStream(baos));
        try {
            dos.writeInt(mMessageType);
            dos.writeInt(mTeleService);
            dos.writeInt(mServiceCategory);

            dos.writeByte(mSmsAddr.digitMode);
            dos.writeByte(mSmsAddr.numberMode);
            dos.writeByte(mSmsAddr.ton);
            dos.writeByte(mSmsAddr.numberPlan);
            dos.writeByte(mSmsAddr.numberOfDigits);
            dos.write(mSmsAddr.origBytes, 0, mSmsAddr.origBytes.length); // digits

            dos.writeInt(mBearerReply);
            // CauseCode values:
            dos.writeByte(mReplySeqNo);
            dos.writeByte(mErrorClass);
            dos.writeByte(mCauseCode);
            //encoded BearerData:
            dos.writeInt(mBearerData.length);
            dos.write(mBearerData, 0, mBearerData.length);
            dos.close();

            return baos.toByteArray();
        } catch (IOException ex) {
            loge("createPdu: conversion from object to byte array failed: " + ex);
            return null;
        }
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
