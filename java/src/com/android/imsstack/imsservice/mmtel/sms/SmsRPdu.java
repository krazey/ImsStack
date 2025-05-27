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

import android.telephony.PhoneNumberUtils;

import com.android.imsstack.imsservice.mmtel.sms.SmsTPdu.Direction;
import com.android.imsstack.util.ImsLog;
import com.android.imsstack.util.ImsUtils;

import java.io.ByteArrayOutputStream;
import java.util.Arrays;

/**
 * Implements the encoder and parser for RPDU as per 3GPP TS 24.011
 **/
public final class SmsRPdu {
    private static final String TAG = "[GII-SmsPdu] ";
    private static final boolean DBG = ImsLog.isDebuggable();
    private static final int MAX_RPDU_LENGTH = 248;
    //Max RP-UserData Length excluding IEI and Length bytes
    private static final int MAX_TPDU_LENGTH = 232;

    // Message Type Indicator(MTI) byte As per 3GPP 24.011 section. 8.2.2
    public static final byte MO_RP_DATA_MTI = 0x00;
    public static final byte MO_RP_ACK_MTI = 0x02;
    public static final byte MO_RP_ERROR_MTI = 0x04;
    // RP-SMMA - Short Message Memory Available notification
    public static final byte MO_RP_SMMA_MTI = 0x06;

    public static final byte MT_RP_DATA_MTI = 0x01;
    public static final byte MT_RP_ACK_MTI = 0x03;
    public static final byte MT_RP_ERROR_MTI = 0x05;

    // IEI- Information Element Identifier as per 3GPP 24.011 section. 7.3
    public static final byte RP_UD_IEI = 0x41;
    public static final byte RP_CAUSE_IEI = 0x42;

    //TODO: More Values to be added as per specification
    //RP-Cause Values as per 3GPP 24.011 Table 8.4
    public static final byte RP_CAUSE_PROTOCOL_UNSPECIFIED = 0x6f;

    public static final byte RP_CAUSE_LENGTH_INDICATOR = 0x01;

    private byte mMessageTypeInd;
    private int mRpMessageType;
    private int mMessageRef;
    private String mOrigAddr;
    private String mDestAddr;
    private int mRpCause;
    private byte[] mRpUserData;
    private byte[] mRpdu;
    private SmsTPdu mTpdu;
    private boolean mIsRetry;

    public SmsRPdu(int messageRef, int messageType,
                   String destinationAddress, int cause,
                   byte[] userData) {
        this(messageRef, messageType, destinationAddress, cause, userData, false);
    }

    public SmsRPdu(int messageRef, int messageType,
                    String destinationAddress, int cause,
                    byte[] userData,
                    boolean isRetry) {
        mMessageRef = messageRef;
        mRpMessageType = messageType;
        mDestAddr = destinationAddress;
        mRpCause = cause;
        mRpUserData = userData;
        mIsRetry = isRetry;
        encodeRpduByteArray();
        parseTpdu(Direction.MS_TO_SC);
    }

    public SmsRPdu(byte[] pdu) {
        mRpdu = pdu;
        parsePdu();
        parseTpdu(Direction.SC_TO_MS);
    }

    /**
     * Returns the RPdu Message Type
     * @return The RPdu Message Type
     */
    public int getMessageType() {
        return mRpMessageType;
    }

    public String getOrigAddr() {
        return mOrigAddr;
    }

    public String getDestAddr() {
        return mDestAddr;
    }

    public int getMessageRef() {
        return mMessageRef;
    }

    public byte[] getUserData() {
        return mRpUserData;
    }

    public int getRPCause() {
        return mRpCause & 0xff;
    }
    public byte[] getRpduByteArray() {
        return mRpdu;
    }

    public boolean isRetry() {
        return mIsRetry;
    }
    /**
     * Encodes of RPdu as per 3GPP TS 24.011
     */
    private void encodeRpduByteArray() {
        switch (mRpMessageType) {
            case SmsUtils.RP_DATA:
                mRpdu = getRpDataPdu();
                break;
            case SmsUtils.RP_ACK:
                mRpdu = getRpAckPdu();
                break;
            case SmsUtils.RP_ERROR:
                mRpdu = getRpErrorPdu();
                break;
            case SmsUtils.RP_SMMA:
                mRpdu = getRpSmmaPdu();
                break;
            default:
                loge("Unsupported message type");
                break;
        }
    }

    private void parseTpdu(Direction direction) {
        if (mRpUserData == null) {
            log("RpUserData is null");
            return;
        }
        try {
            mTpdu = new SmsTPdu(this, direction);
            mTpdu.parse();
            if (DBG) {
                log("DecodedSmsPdu: " + this);
            }
        } catch (RuntimeException e) {
            loge("Error parsing pdu: " + e.getMessage());
        }
    }

    /**
     *  Encoding RP-Data as Per 3GPP 24.011, Section 7.3.1.2
     */
    private byte[] getRpDataPdu() {
        log("getRpDataPdu");
        mMessageTypeInd = MO_RP_DATA_MTI;
        ByteArrayOutputStream bo = new ByteArrayOutputStream(MAX_RPDU_LENGTH);
        byte[] destinationAddress = ImsUtils.hexStringToBytes(mDestAddr);
        if (destinationAddress == null || destinationAddress.length == 0) {
            return null;
        }
        bo.write(MO_RP_DATA_MTI);
        byte mr = (byte) mMessageRef;
        bo.write(mr);
        // Originator address length is 0 for MO RP-Data
        bo.write(0x00);

        // destination address
        bo.write(destinationAddress, 0, destinationAddress.length);
        // user data length
        bo.write((byte) mRpUserData.length);
        bo.write(mRpUserData, 0, mRpUserData.length);
        return bo.toByteArray();
    }

    /**
     *  Encoding RP-Ack as Per 3GPP 24.011, Section 7.3.3
     */
    private byte[] getRpAckPdu() {
        log("getRpAckPdu");
        mMessageTypeInd = MO_RP_ACK_MTI;
        ByteArrayOutputStream bo = new ByteArrayOutputStream(MAX_RPDU_LENGTH);
        bo.write(MO_RP_ACK_MTI);
        bo.write((byte) mMessageRef);
        if (mRpUserData != null) {
            // RP_UD IEI
            bo.write(RP_UD_IEI);
            // user data length
            bo.write((byte) mRpUserData.length);
            bo.write(mRpUserData, 0, mRpUserData.length);
        }
        return bo.toByteArray();
    }

    /**
     *  Encoding RP-Error as Per 3GPP 24.011, Section 7.3.4
     */
    private byte[] getRpErrorPdu() {
        log("getRpErrorPdu");
        mMessageTypeInd = MO_RP_ERROR_MTI;
        ByteArrayOutputStream bo = new ByteArrayOutputStream(MAX_RPDU_LENGTH);
        bo.write(MO_RP_ERROR_MTI);
        bo.write((byte) mMessageRef);
        // FIXME: Need to update Accurate Cause Values
        // RP-CAUSE Header as per 3GPP 24.011 section 8.2.5.4
        bo.write(RP_CAUSE_LENGTH_INDICATOR);
        bo.write((byte) mRpCause);
        if (mRpUserData != null) {
            // RP_UD IEI
            bo.write(RP_UD_IEI);
            // user data length
            bo.write((byte) mRpUserData.length);
            bo.write(mRpUserData, 0, mRpUserData.length);
        }

        return bo.toByteArray();
    }

    /**
     *  Encoding RP-Data as Per 3GPP 24.011, Section 7.3.2
     */
    private byte[] getRpSmmaPdu() {
        log("getRpSmmaPdu");
        mMessageTypeInd = MO_RP_SMMA_MTI;
        ByteArrayOutputStream bo = new ByteArrayOutputStream(MAX_RPDU_LENGTH);
        bo.write(MO_RP_SMMA_MTI);

        bo.write((byte) mMessageRef);
        return bo.toByteArray();
    }

    /**
     * Parses the PDU as per 3GPP TS 24.011
     */
    private void parsePdu() {
        byte[] pdu = mRpdu;
        PduParser p = new PduParser(pdu);
        // RP-Message-Type-Indicator(RP-MTI)
        int firstByte = p.getByte();
        //RP-MTI is a 3-bit field from least significant bit
        mMessageTypeInd = (byte) (firstByte & 0x7);

        logi("SMS Mti: " + mMessageTypeInd);
        switch (mMessageTypeInd) {
                // RP-Message-Type-Indicator 24.011 Seection 8.2.2
            case MT_RP_DATA_MTI:
                mRpMessageType = SmsUtils.RP_DATA;
                parseRpData(p);
                break;
            case MT_RP_ACK_MTI:
                mRpMessageType = SmsUtils.RP_ACK;
                parseRpAck(p);
                break;
            case MT_RP_ERROR_MTI:
                mRpMessageType = SmsUtils.RP_ERROR;
                parseRpError(p);
                break;
            default:
                throw new RuntimeException("Unsupported message type");
        }
    }

    private void parseRpData(PduParser p) {
        try {
            mMessageRef = p.getByte();
            if (DBG) {
                log("mr =" + mMessageRef);
            }
            mOrigAddr = p.getAddress();
            if (DBG) {
                log("parseRpData :: originating Address =" + ImsLog.hiddenString(mOrigAddr));
            }
            mDestAddr = p.getAddress();
            mRpUserData = p.getUserDataForRpData();
            if (mRpUserData == null) {
                loge("parseRpData :: User Data returned null");
                return;
            }
        } catch (RuntimeException ex) {
            mOrigAddr = null;
            mRpUserData = null;
            loge("parseRpData :: RP Data parsing failed: " + ex.toString());
        }
    }

    private void parseRpError(PduParser p) {
        try {
            mMessageRef = p.getByte();
            if (DBG) {
                log("parseRpError :: mr =" + mMessageRef);
            }
            int causeLen = p.getByte();
            mRpCause = p.getByte();
            //Ignore and skip the diagnostic information if any and check for RP-UserData
            int index = p.getIndex() + causeLen - 1;
            if (mRpdu.length <= index) return;
            for (int i = 1; i < causeLen; i++) p.getByte();
            mRpUserData = p.getUserDataForRpAckOrRpError();
        } catch (RuntimeException ex) {
            mRpUserData = null;
            loge("parseRpError failed: " + ex.toString());
        }
    }

    private void parseRpAck(PduParser p) {
        try {
            mMessageRef = p.getByte();
            if (DBG) {
                log("parseRpAck :: mr =" + mMessageRef);
            }

            if (mRpdu.length <= 2) return;

            mRpUserData = p.getUserDataForRpAckOrRpError();
        } catch (RuntimeException ex) {
            mRpUserData = null;
            loge("parseRpAck failed: " + ex.toString());
        }
    }

    private static class PduParser {
        byte[] mPdu;
        int mCur;

        PduParser(byte[] pdu) {
            mPdu = pdu;
            mCur = 0;
        }

        String getAddress() {
            int len;
            String ret;
            // length of SC Address
            len = getByte();
            if (len == 0) {
                // no SC address
                logi("getAddress :: no sc address");
                ret = null;
            } else {
                // SC address
                try {
                    ret = PhoneNumberUtils.calledPartyBCDToString(
                                    mPdu,
                                    mCur,
                                    len,
                                    PhoneNumberUtils.BCD_EXTENDED_TYPE_CALLED_PARTY);
                    if (DBG) {
                        log("getAddress :: sc address = " + ImsLog.hiddenString(ret));
                    }
                } catch (RuntimeException tr) {
                    loge("getAddress :: invalid SC address: " + tr.toString());
                    ret = null;
                }
            }
            mCur += len;
            return ret;
        }

        int getByte() {
            return mPdu[mCur++] & 0xff;
        }

        int getIndex() {
            return mCur;
        }

        byte[] getUserDataForRpData() {
            int udlen = getByte();
            if (udlen > MAX_TPDU_LENGTH) {
                loge("TPDU Length exceeds Max Length");
                return null;
            }
            byte[] ud = Arrays.copyOfRange(mPdu, mCur, mCur + udlen);
            mCur += ud.length;
            return ud;
        }

        byte[] getUserDataForRpAckOrRpError() {
            //For RP-Ack and RPError the RP-UserData starts with Identifier 0x41(49)
            int udIdentifier = getByte();
            if (udIdentifier != RP_UD_IEI) {
                loge("getUserDataForRpAckOrRpError :: Invalid RP-UserData IEI");
                return null;
            }
            int udlen = getByte();
            if (udlen > MAX_TPDU_LENGTH) {
                loge("getUserDataForRpAckOrRpError :: TPDU Length exceeds Max Length");
                return null;
            }
            byte[] ud = Arrays.copyOfRange(mPdu, mCur, mCur + udlen);
            mCur += ud.length;
            return ud;
        }
    }

    private static void log(String s) {
        ImsLog.d(TAG + s);
    }

    private static void loge(String s) {
        ImsLog.e(TAG + s);
    }

    private static void logi(String s) {
        ImsLog.i(TAG + s);
    }

    // For logging purpose
    private String byteArrayToString(byte[] array) {
        if (array == null) {
            return "Not Present";
        }
        return ImsLog.hiddenString(ImsUtils.bytesToHexString(array));
    }

    private byte[] getRpduHeader() {
        if (mRpdu != null && mRpUserData != null && mRpdu.length > mRpUserData.length) {
            return Arrays.copyOfRange(mRpdu, 0, mRpdu.length - mRpUserData.length);
        } else {
            return mRpdu;
        }
    }

    private String getRpMessageTypeString() {
        log("RP-MTI: " + mMessageTypeInd);
        return switch (mMessageTypeInd) {
            case MO_RP_DATA_MTI -> "RP-DATA MS_TO_SC";
            case MT_RP_DATA_MTI -> "RP-DATA SC_TO_MS";
            case MO_RP_ACK_MTI -> "RP-ACK MS_TO_SC";
            case MT_RP_ACK_MTI -> "RP-ACK SC_TO_MS";
            case MO_RP_ERROR_MTI -> "RP-ERROR MS_TO_SC";
            case MT_RP_ERROR_MTI -> "RP-ERROR SC_TO MS";
            case MO_RP_SMMA_MTI -> "RP-SMMA MS_TO_SC";
            default -> "UNKNOWN (" + mMessageTypeInd + ")";
        };
    }

    private String getMessageTypeIndicator() {
        return String.format("%3s", Integer.toBinaryString(mMessageTypeInd)).replace(' ', '0');
    }

    @Override
    public String toString() {
        StringBuilder builder = new StringBuilder();
        builder.append("\n  RP Message Type: ").append(getRpMessageTypeString());
        builder.append("\n  RP-MTI (Message Type Indicator): ").append(getMessageTypeIndicator());
        builder.append("\n  RP-MR (Message Reference): ").append(mMessageRef);

        if (mOrigAddr != null) {
            builder.append("\n  RP-OA (Origination Address): ")
                    .append(ImsLog.hiddenString(mOrigAddr));
        }
        if (mDestAddr != null) {
            builder.append("\n  RP-DA (Destination Address): ")
                    .append(ImsLog.hiddenString(mDestAddr));
        }
        if (mRpMessageType == SmsUtils.RP_ERROR) {
            builder.append("\n  RP-Cause: ").append(String.format("0x%02X", getRPCause()));
        }

        builder.append("\n  R-PDU Header: ").append(byteArrayToString(getRpduHeader()));

        if (mTpdu != null) {
            builder.append("\n").append(mTpdu.toString());
        } else {
            builder.append("\n  T-PDU: Not Present");
        }
        builder.append("\n");

        return builder.toString();
    }
}
