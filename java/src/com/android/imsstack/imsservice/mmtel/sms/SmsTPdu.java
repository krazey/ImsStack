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

import com.android.imsstack.util.ImsLog;
import com.android.imsstack.util.ImsUtils;

import java.util.Arrays;

/** Implements the parser for TPDU as per 3GPP TS 23.040 */
public class SmsTPdu {
    private static final String TAG = "[GII-SmsTPdu] ";

    private static final int MTI_DELIVER = 0;
    private static final int MTI_DELIVER_REPORT = 0;
    private static final int MTI_SUBMIT = 1;
    private static final int MTI_SUBMIT_REPORT = 1;
    private static final int MTI_COMMAND = 2;
    private static final int MTI_STATUS_REPORT = 2;
    private static final int MTI_RESERVED = 3;
    private static final int USER_DATA_HEADER_PRESENT = 1;
    private static final int ONE_BIT = 0x01;
    private static final int TWO_BITS = 0x03;
    private static final int EIGHT_BITS = 0xFF;
    private static final int IEI_CONCATENATED_SMS_8BIT = 0x00;
    private static final int IEI_CONCATENATED_SMS_16BIT = 0x08;
    private static final int IEI_CONCATENATED_SMS_8BIT_LENGTH = 3;
    private static final int IEI_CONCATENATED_SMS_16BIT_LENGTH = 4;

    private int mMessageTypeIndicator = -1;
    private int mReplyPath;
    private int mMessageRef;
    private int mMoreMessageToSend;
    private int mLoopPrevention;
    private int mProtocolIdentifier;
    private int mDataCodingScheme;
    private int mRejectDuplicate;
    private int mStatusReportRequest;
    private int mStatusReportIndication;
    private int mStatusReportQualifier;
    private int mStatus;
    private int mParameterIndicator = -1;
    private int mUserDataHeaderIndicator;
    private int mValidityPeriodFormat;
    private int mFailureCause;
    private int mUserDataLength;
    private byte[] mPdu;
    private byte[] mUserDataHeader;
    private byte[] mUserData;
    private byte[] mOriginatingAddress;
    private byte[] mDestinationAddress; // Also used for RecipientAddress in StatusReport
    private Direction mDirection;
    private int mRpMti;

    // Fields Parsed from User Data Header
    private int mUdhIei = -1; // IEI for concatenated SMS if present
    private int mUdhReference = -1; // Reference for concatenated SMS if present
    private int mUdhMaxNum = -1; // Max segments for concatenated SMS if present
    private int mUdhSeqNum = -1; // Sequence number for concatenated SMS if present

    public enum Direction {
        MS_TO_SC,
        SC_TO_MS
    }

    public SmsTPdu(SmsRPdu smsRpdu, Direction direction) {
        mPdu = smsRpdu.getUserData();
        mDirection = direction;
        mRpMti = smsRpdu.getMessageType();
    }

    /** Decodes of TPdu as per 3GPP TS 23.040 */
    public void parse() {
        if (mPdu == null || mPdu.length == 0) {
            loge("PDU is null or empty, cannot parse.");
            return;
        }
        PduParser parser = new PduParser(mPdu, this);

        int firstOctet = parser.getOctet();
        mMessageTypeIndicator = firstOctet & TWO_BITS;
        logd("TP-MTI: " + mMessageTypeIndicator);

        if (mDirection == Direction.MS_TO_SC) {
            switch (mMessageTypeIndicator) {
                case MTI_DELIVER_REPORT:
                    parseSmsDeliverReport(parser, firstOctet);
                    break;
                case MTI_SUBMIT:
                    parseSmsSubmit(parser, firstOctet);
                    break;
                case MTI_COMMAND:
                case MTI_RESERVED:
                default:
                    loge("Unable to parse PDU, unsupported TP-MTI: " + mMessageTypeIndicator);
            }
        } else if (mDirection == Direction.SC_TO_MS) {
            switch (mMessageTypeIndicator) {
                case MTI_SUBMIT_REPORT:
                    parseSmsSubmitReport(parser, firstOctet);
                    break;
                case MTI_STATUS_REPORT:
                    parseSmsStatusReport(parser, firstOctet);
                    break;
                case MTI_DELIVER:
                case MTI_RESERVED: // This should be processed in the same way as MTI == 0 (Deliver)
                default:
                    parseSmsDeliver(parser, firstOctet);
            }
        } else {
            loge("Unable to parse PDU, invalid direction: " + mDirection);
            return;
        }
    }

    private void parseParameterIndicator(PduParser parser) {
        if (!parser.hasMoreData()) {
            return;
        }

        // TP-PI
        mParameterIndicator = parser.getOctet();

        int currentPI = mParameterIndicator;
        // Only parse first PI octet's flags for known parameters
        while ((currentPI & 0x80) != 0) {
            if (!parser.hasMoreData()) {
                return;
            }
            currentPI = parser.getOctet();
        }
    }

    private void parseOptionalParameters(PduParser parser) {
        if (!parser.hasMoreData()) {
            return; // No optional parameters
        }

        // Check if reserved bits (3-6) are non-zero; if so, ignore PI flags
        if ((mParameterIndicator & 0x78) != 0) {
            return;
        }

        // TP-PID (Optional)
        if ((mParameterIndicator & 0x01) != 0) {
            if (!parser.hasMoreData()) {
                return;
            }
            mProtocolIdentifier = parser.getOctet();
        }

        // TP-DCS (Optional)
        if ((mParameterIndicator & 0x02) != 0) {
            if (!parser.hasMoreData()) {
                return;
            }
            mDataCodingScheme = parser.getOctet();
        }

        // TP-UDL & TP-UD (Optional)
        if ((mParameterIndicator & 0x04) != 0) {
            if (!parser.hasMoreData()) {
                return;
            }
            mUserDataLength = parser.getOctet();
            if (mUserDataHeaderIndicator == USER_DATA_HEADER_PRESENT) {
                mUserDataHeader = parser.parseUserDataHeader();
            }
            mUserData = parser.parseUserData();
        }
    }

    private void parseSmsSubmitReport(PduParser parser, int firstOctet) {
        mUserDataHeaderIndicator = (firstOctet >> 6) & ONE_BIT; // TP-UDHI

        if (!parser.hasMoreData()) {
            return;
        }

        if (mRpMti == SmsUtils.RP_ERROR) {
            mFailureCause = parser.getOctet(); // TP-FCS (not present in RP-ACK)
        }

        parseParameterIndicator(parser);

        if (!parser.hasMoreData(PduParser.TIMESTAMPS_OCTETS)) {
            return;
        }
        parser.getTimeStamp(); // TP-SCTS
        parseOptionalParameters(parser);
    }

    private void parseSmsDeliverReport(PduParser parser, int firstOctet) {
        mUserDataHeaderIndicator = (firstOctet >> 6) & ONE_BIT; // TP-UDHI

        if (!parser.hasMoreData()) {
            return;
        }

        if (mRpMti == SmsUtils.RP_ERROR) {
            mFailureCause = parser.getOctet(); // TP-FCS (not present in RP-ACK)
        }

        parseParameterIndicator(parser);
        parseOptionalParameters(parser);
    }

    private void parseSmsSubmit(PduParser parser, int firstOctet) {
        mRejectDuplicate = (firstOctet >> 2) & ONE_BIT; // TP-RD
        mValidityPeriodFormat = (firstOctet >> 3) & TWO_BITS; // TP-VPF
        mStatusReportRequest = (firstOctet >> 5) & ONE_BIT; // TP-SRR
        mUserDataHeaderIndicator = (firstOctet >> 6) & ONE_BIT; // TP-UDHI
        mReplyPath = (firstOctet >> 7) & ONE_BIT; // TP-RP

        mMessageRef = parser.getOctet(); // TP-MR
        mDestinationAddress = parser.getAddress(); // TP-DA
        mProtocolIdentifier = parser.getOctet(); // TP-PID
        mDataCodingScheme = parser.getOctet(); // TP-DCS
        parser.parseValidityPeriod(mValidityPeriodFormat); // TP-VP

        mUserDataLength = parser.getOctet(); // TP-UDL
        if (mUserDataHeaderIndicator == USER_DATA_HEADER_PRESENT) {
            mUserDataHeader = parser.parseUserDataHeader(); // TP-UDH
        }
        mUserData = parser.parseUserData(); // TP-UD
    }

    private void parseSmsDeliver(PduParser parser, int firstOctet) {
        mMoreMessageToSend = (firstOctet >> 2) & ONE_BIT; // TP-MMS
        mLoopPrevention = (firstOctet >> 3) & ONE_BIT; // TP-LP
        mStatusReportIndication = (firstOctet >> 5) & ONE_BIT; // TP-SRI
        mUserDataHeaderIndicator = (firstOctet >> 6) & ONE_BIT; // TP-UDHI
        mReplyPath = (firstOctet >> 7) & ONE_BIT; // TP-RP

        mOriginatingAddress = parser.getAddress(); // TP-OA
        mProtocolIdentifier = parser.getOctet(); // TP-PID
        mDataCodingScheme = parser.getOctet(); // TP-DCS
        parser.getTimeStamp(); // TP-SCTS

        mUserDataLength = parser.getOctet(); // TP-UDL
        if (mUserDataHeaderIndicator == USER_DATA_HEADER_PRESENT) {
            mUserDataHeader = parser.parseUserDataHeader(); // TP-UDH
        }
        mUserData = parser.parseUserData(); // TP-UD
    }

    private void parseSmsStatusReport(PduParser parser, int firstOctet) {
        mMoreMessageToSend = (firstOctet >> 2) & ONE_BIT; // TP-MMS
        mLoopPrevention = (firstOctet >> 3) & ONE_BIT; // TP-LP
        mStatusReportQualifier = (firstOctet >> 5) & ONE_BIT; // TP-SRQ
        mUserDataHeaderIndicator = (firstOctet >> 6) & ONE_BIT; // TP-UDHI
        // Bit 7 reserved (0)

        mMessageRef = parser.getOctet(); // TP-MR
        mDestinationAddress = parser.getAddress(); // TP-RA
        parser.getTimeStamp(); // TP-SCTS
        parser.getTimeStamp(); // TP-DT
        mStatus = parser.getOctet(); // TP-ST

        parseParameterIndicator(parser);
        parseOptionalParameters(parser);
    }

    public String getDataCodingSchemeHex() {
        return String.format("0x%02X", mDataCodingScheme);
    }

    // --- PduParser Inner Class ---
    private static class PduParser {
        private static final int VALIDITY_PERIOD_FORMAT_NONE = 0x00;
        private static final int VALIDITY_PERIOD_FORMAT_RELATIVE = 0x02;
        private static final int VALIDITY_PERIOD_FORMAT_NONE_OCTETS = 0;
        private static final int VALIDITY_PERIOD_FORMAT_RELATIVE_OCTETS = 1;
        private static final int VALIDITY_PERIOD_FORMAT_ENHANCED_OCTETS = 7;
        private static final int TIMESTAMPS_OCTETS = 7;
        byte[] mPdu;
        int mCur;
        SmsTPdu mOuter; // Reference to outer class to set UDH fields

        PduParser(byte[] pdu, SmsTPdu outer) {
            mPdu = pdu;
            mCur = 0;
            mOuter = outer;
        }

        boolean hasMoreData() {
            return mCur < mPdu.length;
        }

        boolean hasMoreData(int expectedOctets) {
            return mCur + expectedOctets <= mPdu.length;
        }

        int getOctet() {
            if (hasMoreData()) {
                return mPdu[mCur++] & EIGHT_BITS;
            } else {
                loge("Failed to get next octet, reached end of array");
                return -1;
            }
        }

        byte[] getAddress() {
            if (!hasMoreData(2)) { // Need at least length and type octets
                loge("Failed to parse the address, not enough data for length/type");
                mCur = mPdu.length;
                return null;
            }

            byte[] address = null;
            int addressLengthInDigits = mPdu[mCur] & EIGHT_BITS; // Length is number of digits
            int bcdOctets = (addressLengthInDigits + 1) / 2;
            int totalLength = 2 + bcdOctets; // Length octet + Type octet + BCD octets

            if (hasMoreData(totalLength)) {
                address = Arrays.copyOfRange(mPdu, mCur, mCur + totalLength);
                mCur += totalLength;
            } else {
                loge("Failed to parse the address, not enough data for BCD part");
                mCur = mPdu.length;
            }
            return address;
        }

        long getTimeStamp() {
            long timestamp = 0;
            if (hasMoreData(TIMESTAMPS_OCTETS)) {
                // Skipped parsing logic for now, just consume bytes
                mCur += TIMESTAMPS_OCTETS;
            } else {
                loge("Failed to parse the timestamp, not enough data");
                mCur = mPdu.length;
            }
            return timestamp;
        }

        void parseValidityPeriod(int format) {
            int validityPeriodLengthOctets = 0;
            switch (format) {
                case VALIDITY_PERIOD_FORMAT_NONE:
                    validityPeriodLengthOctets = VALIDITY_PERIOD_FORMAT_NONE_OCTETS;
                    break;
                case VALIDITY_PERIOD_FORMAT_RELATIVE:
                    validityPeriodLengthOctets = VALIDITY_PERIOD_FORMAT_RELATIVE_OCTETS;
                    break;
                default: // VALIDITY_PERIOD_FORMAT_ENHANCED or VALIDITY_PERIOD_FORMAT_ABSOLUTE
                    validityPeriodLengthOctets = VALIDITY_PERIOD_FORMAT_ENHANCED_OCTETS;
                    break;
            }

            if (hasMoreData(validityPeriodLengthOctets)) {
                // Skipped actual value parsing logic for now, just consume bytes
                mCur += validityPeriodLengthOctets;
            } else if (validityPeriodLengthOctets > 0) {
                loge("Failed to parse Validity Period, not enough data. ");
                mCur = mPdu.length;
            }
        }

        /**
         * Parses the UDH including the length octet, saves it, decodes known IEs, and returns the
         * full UDH including the length byte.
         */
        byte[] parseUserDataHeader() {
            if (!hasMoreData()) {
                loge("Failed to parse User Data Header length, reached end of the array");
                return null;
            }
            int headerLength = mPdu[mCur] & EIGHT_BITS; // Length of header *data*
            int totalHeaderLength = headerLength + 1;

            if (hasMoreData(totalHeaderLength)) {
                byte[] fullHeader = Arrays.copyOfRange(mPdu, mCur, mCur + totalHeaderLength);
                decodeUdhData(fullHeader, 1, headerLength);
                mCur += totalHeaderLength;
                return fullHeader;
            } else {
                loge("Failed to parse User Data Header data, not enough data");
                mCur = mPdu.length;
            }
            return null;
        }

        /**
         * Decodes known Information Elements from the UDH data portion.
         *
         * @param fullHeader The full UDH byte array (including length byte).
         * @param offset The starting offset of the header *data* within fullHeader (usually 1).
         * @param length The length of the header *data* (UDHL value).
         */
        private void decodeUdhData(byte[] fullHeader, int offset, int length) {
            if (fullHeader == null || length == 0 || offset + length > fullHeader.length) {
                return;
            }

            int cursor = offset;
            int end = offset + length;
            while (cursor < end) {
                if (cursor + 1 >= end) {
                    loge("UDH Decode Error: Not enough bytes for IEI and IEIL");
                    return;
                }

                int iei = fullHeader[cursor++] & EIGHT_BITS;
                int ieil = fullHeader[cursor++] & EIGHT_BITS;

                if (cursor + ieil > end) {
                    loge("UDH Decode Error: Not enough bytes for IE data");
                    return;
                }

                // --- Decode Concatenated SMS ---
                if (iei == IEI_CONCATENATED_SMS_8BIT && ieil == IEI_CONCATENATED_SMS_8BIT_LENGTH) {
                    mOuter.mUdhReference = fullHeader[cursor] & EIGHT_BITS;
                    mOuter.mUdhMaxNum = fullHeader[cursor + 1] & EIGHT_BITS;
                    mOuter.mUdhSeqNum = fullHeader[cursor + 2] & EIGHT_BITS;
                    mOuter.mUdhIei = iei;
                } else if (iei == IEI_CONCATENATED_SMS_16BIT
                        && ieil == IEI_CONCATENATED_SMS_16BIT_LENGTH) {
                    mOuter.mUdhReference =
                            ((fullHeader[cursor] & EIGHT_BITS) << 8)
                                    | (fullHeader[cursor + 1] & EIGHT_BITS);
                    mOuter.mUdhMaxNum = fullHeader[cursor + 2] & EIGHT_BITS;
                    mOuter.mUdhSeqNum = fullHeader[cursor + 3] & EIGHT_BITS;
                    mOuter.mUdhIei = iei;
                }

                cursor += ieil;
            }
        }

        byte[] parseUserData() {
            if (!hasMoreData()) {
                return null;
            }
            byte[] userData = Arrays.copyOfRange(mPdu, mCur, mPdu.length);
            mCur = mPdu.length;
            return userData;
        }
    }

    // For logging purpose
    private String byteArrayToString(byte[] array) {
        if (array == null) {
            return "Not Present";
        }
        return ImsLog.hiddenString(ImsUtils.bytesToHexString(array));
    }

    private static void loge(String s) {
        ImsLog.e(TAG + s);
    }

    private static void logd(String s) {
        ImsLog.d(TAG + s);
    }

    @Override
    public String toString() {
        StringBuilder builder = new StringBuilder();
        builder.append("\n  Direction: ").append(mDirection);
        builder.append("\n  TP-MTI: ").append(getMessageTypeIndicator());
        builder.append(" (").append(getMtiString()).append(")");
        builder.append("\n  TP-RP (Reply Path): ").append(mReplyPath);
        builder.append("\n  TP-UDHI (UDH Indicator): ").append(mUserDataHeaderIndicator);

        if (mDirection == Direction.MS_TO_SC) {
            if (mMessageTypeIndicator == MTI_SUBMIT) {
                builder.append("\n  TP-SRR (Status Report Request): ").append(mStatusReportRequest);
                builder.append("\n  TP-VPF (Validity Period Format): ")
                        .append(mValidityPeriodFormat);
                builder.append("\n  TP-RD (Reject Duplicate): ").append(mRejectDuplicate);
                builder.append("\n  TP-MR (Message Reference): ").append(mMessageRef);
                builder.append("\n  TP-DA (Destination Address): ")
                        .append(byteArrayToString(mDestinationAddress));
                builder.append("\n  TP-PID (Protocol ID): ")
                        .append(String.format("0x%02X", mProtocolIdentifier));
                builder.append("\n  TP-DCS (Data Coding Scheme): ")
                        .append(getDataCodingSchemeHex());
                builder.append("\n  TP-UDL (User Data Length): ").append(mUserDataLength);
            } else if (mMessageTypeIndicator == MTI_DELIVER_REPORT) {
                builder.append("\n  TP-FCS (Failure Cause): ")
                        .append(String.format("0x%02X", mFailureCause));
                appendOptionalParams(builder);
            }
        } else { // SC_TO_MS
            if (mMessageTypeIndicator == MTI_DELIVER || mMessageTypeIndicator == MTI_RESERVED) {
                builder.append("\n  TP-SRI (Status Report Indication): ")
                        .append(mStatusReportIndication);
                builder.append("\n  TP-MMS (More Messages to Send): ").append(mMoreMessageToSend);
                builder.append("\n  TP-OA (Originating Address): ")
                        .append(byteArrayToString(mOriginatingAddress));
                builder.append("\n  TP-PID (Protocol ID): ")
                        .append(String.format("0x%02X", mProtocolIdentifier));
                builder.append("\n  TP-DCS (Data Coding Scheme): ")
                        .append(getDataCodingSchemeHex());
                builder.append("\n  TP-UDL (User Data Length): ")
                        .append(mUserDataLength); // Note: Value context depends on DCS/UDHI
            } else if (mMessageTypeIndicator == MTI_STATUS_REPORT) {
                builder.append("\n  TP-SRQ (Status Report Qualifier): ")
                        .append(mStatusReportQualifier);
                builder.append("\n  TP-LP (Loop Prevention): ").append(mLoopPrevention);
                builder.append("\n  TP-MMS (More Messages to Send): ").append(mMoreMessageToSend);
                builder.append("\n  TP-MR (Message Reference): ").append(mMessageRef);
                builder.append("\n  TP-RA (Recipient Address): ")
                        .append(byteArrayToString(mDestinationAddress));
                builder.append("\n  TP-ST (Status): ").append(String.format("0x%02X", mStatus));
                appendOptionalParams(builder);
            } else if (mMessageTypeIndicator == MTI_SUBMIT_REPORT) {
                builder.append("\n  TP-FCS (Failure Cause): ")
                        .append(String.format("0x%02X", mFailureCause));
                appendOptionalParams(builder);
            }
        }

        if (mUserDataHeaderIndicator == USER_DATA_HEADER_PRESENT && mUserDataHeader != null) {
            builder.append("\n  TP-UDH (User Data Header): ")
                    .append(byteArrayToString(mUserDataHeader));
            if (mUdhIei != -1
                    && (mUdhIei == IEI_CONCATENATED_SMS_8BIT
                            || mUdhIei == IEI_CONCATENATED_SMS_16BIT)) {
                builder.append("\n    Concat IEI: ").append(String.format("0x%02X", mUdhIei));
                builder.append("\n    Concat Ref: ").append(mUdhReference);
                builder.append("\n    Concat Max: ").append(mUdhMaxNum);
                builder.append("\n    Concat Seq: ").append(mUdhSeqNum);
            }
        } else {
            builder.append("\n  TP-UDH (User Data Header): Not Present");
        }

        builder.append("\n  TP-UD (User Data): ").append(byteArrayToString(mUserData));
        builder.append("\n  Raw T-PDU: ").append(byteArrayToString(mPdu));
        return builder.toString();
    }

    private void appendOptionalParams(StringBuilder builder) {
        if (mParameterIndicator != -1) {
            builder.append("\n  TP-PI (Parameter Indicator): ")
                    .append(String.format("0x%02X", mParameterIndicator));
            if ((mParameterIndicator & 0x01) != 0) {
                builder.append("\n    TP-PID: ")
                        .append(String.format("0x%02X", mProtocolIdentifier));
            }
            if ((mParameterIndicator & 0x02) != 0) {
                builder.append("\n    TP-DCS: ").append(getDataCodingSchemeHex());
            }
            if ((mParameterIndicator & 0x04) != 0) {
                builder.append("\n    TP-UDL: ").append(mUserDataLength);
            }
        } else {
            builder.append("\n  TP-PI (Parameter Indicator): Not Present");
        }
    }

    private boolean isReportPdu() {
        if (mDirection == Direction.MS_TO_SC) {
            return mMessageTypeIndicator == MTI_DELIVER_REPORT;
        } else { // Direction == Direction.SC_TO_MS
            return mMessageTypeIndicator == MTI_SUBMIT_REPORT
                    || mMessageTypeIndicator == MTI_STATUS_REPORT;
        }
    }

    private String getMessageTypeIndicator() {
        return String.format("%2s", Integer.toBinaryString(mMessageTypeIndicator))
                .replace(' ', '0');
    }

    private String getMtiString() {
        boolean isReport = isReportPdu();
        switch (mMessageTypeIndicator) {
            case MTI_DELIVER:
                return isReport ? "DELIVER-REPORT" : "DELIVER";
            case MTI_SUBMIT:
                return isReport ? "SUBMIT-REPORT" : "SUBMIT";
            case MTI_STATUS_REPORT:
                return "STATUS-REPORT";
            default:
                return "COMMAND or RESERVED";
        }
    }
}
