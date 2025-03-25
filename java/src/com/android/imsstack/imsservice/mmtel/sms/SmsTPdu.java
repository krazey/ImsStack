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

    // Fields Parsed from User Data Header
    private int mUdhIei = -1; // IEI for concatenated SMS if present
    private int mUdhReference = -1; // Reference for concatenated SMS if present
    private int mUdhMaxNum = -1; // Max segments for concatenated SMS if present
    private int mUdhSeqNum = -1; // Sequence number for concatenated SMS if present

    public enum Direction {
        MS_TO_SC,
        SC_TO_MS
    }

    public SmsTPdu(byte[] pdu, Direction direction) {
        mPdu = pdu;
        mDirection = direction;
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

    private void parseSmsSubmitReport(PduParser parser, int firstOctet) {
        // TODO: add implementation
    }

    private void parseSmsDeliverReport(PduParser parser, int firstOctet) {
        // TODO: add implementation
    }

    private void parseSmsSubmit(PduParser parser, int firstOctet) {
        // TODO: add implementation
    }

    private void parseSmsDeliver(PduParser parser, int firstOctet) {
        // TODO: add implementation
    }

    private void parseSmsStatusReport(PduParser parser, int firstOctet) {
        // TODO: add implementation
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

        int getOctet() {
            if (hasMoreData()) {
                return mPdu[mCur++] & EIGHT_BITS;
            } else {
                loge("Failed to get next octet, reached end of array");
                return -1;
            }
        }
    }

    // For logging purpose
    String byteArrayToString(byte[] array) {
        if (array == null) {
            return "null";
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
        // TODO : add implementation
        return "";
    }
}
