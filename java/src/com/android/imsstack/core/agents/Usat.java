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

package com.android.imsstack.core.agents;

import android.annotation.IntDef;

import java.lang.annotation.Retention;
import java.lang.annotation.RetentionPolicy;
import java.util.Objects;

/**
 * This interface provides the definition of command and constant values to manage and control
 * USAT functions such as the call control by USIM, MO SMS control by USIM,
 * data download vis SMS-PP and so on.
 */
public interface Usat {
    // These services are defined as the position of service available field in USIM (one-based),
    // but converted to the zero-based value here.
    /** Call control by USIM: 30th bit in UST */
    int SERVICE_CALL_CONTROL = 29;
    /** MO short message control by USIM: 31th bit in UST */
    int SERVICE_MO_SMS_CONTROL = 30;
    /** Data download via SMS-PP: 28th bit in UST */
    int SERVICE_DATA_DOWNLOAD_VIA_SMS_PP = 27;
    /** Media type support: 103th bit in UST */
    int SERVICE_MEDIA_TYPE_SUPPORT = 102;

    @IntDef(value = {
        SERVICE_CALL_CONTROL,
        SERVICE_MO_SMS_CONTROL,
        SERVICE_DATA_DOWNLOAD_VIA_SMS_PP,
        SERVICE_MEDIA_TYPE_SUPPORT
    })
    @Retention(RetentionPolicy.SOURCE)
    public @interface ServiceType {}

    /** Types for SERVICE_CALL_CONTROL */
    int CALL_CONTROL_TYPE_NONE = 0;
    /** Mobile originated call */
    int CALL_CONTROL_TYPE_MO_CALL = 1;
    /** Supplementary service */
    int CALL_CONTROL_TYPE_SS = 2;
    /** USSD (Unstructured Supplementary Services Data) */
    int CALL_CONTROL_TYPE_USSD = 3;

    @IntDef(value = {
        CALL_CONTROL_TYPE_NONE,
        CALL_CONTROL_TYPE_MO_CALL,
        CALL_CONTROL_TYPE_SS,
        CALL_CONTROL_TYPE_USSD
    })
    @Retention(RetentionPolicy.SOURCE)
    public @interface CallControlType {}

    /** Media types for SERVICE_CALL_CONTROL */
    int MEDIA_TYPE_NONE = -1;
    int MEDIA_TYPE_VOICE = 0;
    int MEDIA_TYPE_VIDEO = 1;

    @IntDef(value = {
        MEDIA_TYPE_NONE,
        MEDIA_TYPE_VOICE,
        MEDIA_TYPE_VIDEO
    })
    @Retention(RetentionPolicy.SOURCE)
    public @interface MediaType {}

    /**
     * Result values of the response of the envelope message.
     *  0x00 : "Allowed, no modification"
     *  0x01 : "Not allowed"
     *  0x02 : "Allowed with modifications"
     */
    /** Result for SERVICE_CALL_CONTROL and SERVICE_MO_SMS_CONTROL */
    int RESULT_ALLOWED = 0;
    int RESULT_NOT_ALLOWED = 1;
    int RESULT_ALLOWED_WITH_MODIFICATION = 2;
    /** Result for SERVICE_DATA_DOWNLOAD_VIA_SMS_PP */
    int RESULT_DATA_DOWNLOAD_OK = 11;
    int RESULT_DATA_DOWNLOAD_ERROR = 12;

    @IntDef(value = {
        RESULT_ALLOWED,
        RESULT_NOT_ALLOWED,
        RESULT_ALLOWED_WITH_MODIFICATION,
        RESULT_DATA_DOWNLOAD_OK,
        RESULT_DATA_DOWNLOAD_ERROR
    })
    @Retention(RetentionPolicy.SOURCE)
    public @interface Result {}

    /**
     * Base class for USAT command.
     */
    class Command {
        private final int mCid;
        private final @ServiceType int mServiceType;
        private final Listener mListener;
        private boolean mAborted;

        Command(int cid, @ServiceType int serviceType, Listener listener) {
            mCid = cid;
            mServiceType = serviceType;
            mListener = listener;
        }

        void markAsAborted() {
            mAborted = true;
        }

        int getCid() {
            return mCid;
        }

        Listener getListener() {
            return mListener;
        }

        boolean isAborted() {
            return mAborted;
        }

        /**
         * Returns the service type of this command.
         *
         * @return The service type. Valid values are
         *         {@link #SERVICE_CALL_CONTROL},
         *         {@link #SERVICE_MO_SMS_CONTROL},
         *         {@link #SERVICE_DATA_DOWNLOAD_VIA_SMS_PP}.
         */
        public @ServiceType int getServiceType() {
            return mServiceType;
        }

        @Override
        public int hashCode() {
            return Objects.hash(mCid, mServiceType);
        }

        @Override
        public boolean equals(Object o) {
            if (o == null) {
                return false;
            }

            if (this == o) {
                return true;
            }

            Command other = (Command) o;

            return mCid == other.mCid
                    && mServiceType == other.mServiceType;
        }

        @Override
        public String toString() {
            return "[ Usat.Command: cid=" + mCid + ", serviceType=" + mServiceType + " ]";
        }
    }

    /**
     * USAT command for call control.
     */
    class CallControlCommand extends Command {
        private final @CallControlType int mCcType;
        private final String mDialedString;
        private final int mNetworkType;
        private final @MediaType int mMediaType;

        CallControlCommand(int cid, Listener listener,
                @CallControlType int ccType, String dialedString,
                int networkType, @MediaType int mediaType) {
            super(cid, SERVICE_CALL_CONTROL, listener);
            mCcType = ccType;
            mDialedString = dialedString;
            mNetworkType = networkType;
            mMediaType = mediaType;
        }

        /**
         * Returns the call control type of this command.
         *
         * @return The service type. Valid values are
         *         {@link #CALL_CONTROL_TYPE_MO_CALL},
         *         {@link #CALL_CONTROL_TYPE_SS},
         *         {@link #CALL_CONTROL_TYPE_USSD}.
         */
        public @CallControlType int getCcType() {
            return mCcType;
        }

        /**
         * Returns the dialed number / SS string / USSD string of this command.
         * The type of returned value will be determined by the return value of
         * {@link #getCcType()}.
         *
         * @return The dialed string - dialed number or SS string or USSD string.
         */
        public String getDialedString() {
            return mDialedString;
        }

        /**
         * Returns the network type which the call is performed on.
         * If the network type is {@link TelephonyManager#NETWORK_TYPE_UNKNOWN} or
         * {@link TelephonyManager#NETWORK_TYPE_IWLAN}, the location information will not be
         * included in the command parameter.
         *
         * @return The network type.
         *         {@link TelephonyManager#NETWORK_TYPE_UNKNOWN},
         *         {@link TelephonyManager#NETWORK_TYPE_NR},
         *         {@link TelephonyManager#NETWORK_TYPE_LTE},
         *         {@link TelephonyManager#NETWORK_TYPE_HSPA},
         *         {@link TelephonyManager#NETWORK_TYPE_GPRS},
         *         {@link TelephonyManager#NETWORK_TYPE_IWLAN}
         */
        public int getNetworkType() {
            return mNetworkType;
        }

        /**
         * Returns the media type of this command.
         *
         * @return The media type. Valid values are
         *         {@link #MEDIA_TYPE_NONE},
         *         {@link #MEDIA_TYPE_VOICE},
         *         {@link #MEDIA_TYPE_VIDEO}.
         */
        public @MediaType int getMediaType() {
            return mMediaType;
        }
    }

    /**
     * USAT command for MO SMS control.
     */
    class MoSmsControlCommand extends Command {
        private final String mRpDestinationAddress;
        private final String mTpDestinationAddress;
        private final int mNetworkType;

        MoSmsControlCommand(int cid, Listener listener,
                String rpDestinationAddress, String tpDestinationAddress,
                int networkType) {
            super(cid, SERVICE_MO_SMS_CONTROL, listener);
            mRpDestinationAddress = rpDestinationAddress;
            mTpDestinationAddress = tpDestinationAddress;
            mNetworkType = networkType;
        }

        /**
         * Returns the RP_Destination_Address of the service center.
         *
         * @return The RP_Destination_Address.
         */
        public String getRpDestinationAddress() {
            return mRpDestinationAddress;
        }

        /**
         * Returns the TP_Destination_Address.
         *
         * @return The TP_Destination_Address.
         */
        public String getTpDestinationAddress() {
            return mTpDestinationAddress;
        }

        /**
         * Returns the network type which the call is performed on.
         * If the network type is {@link TelephonyManager#NETWORK_TYPE_UNKNOWN} or
         * {@link TelephonyManager#NETWORK_TYPE_IWLAN}, the location information will not be
         * included in the command parameter.
         *
         * @return The network type.
         *         {@link TelephonyManager#NETWORK_TYPE_UNKNOWN},
         *         {@link TelephonyManager#NETWORK_TYPE_NR},
         *         {@link TelephonyManager#NETWORK_TYPE_LTE},
         *         {@link TelephonyManager#NETWORK_TYPE_HSPA},
         *         {@link TelephonyManager#NETWORK_TYPE_GPRS},
         *         {@link TelephonyManager#NETWORK_TYPE_IWLAN}
         */
        public int getNetworkType() {
            return mNetworkType;
        }
    }

    /**
     * USAT command for SMS-PP download.
     */
    class SmsPpDownloadCommand extends Command {
        private final String mRpOriginatingAddress;
        private final boolean mUriTruncated;
        private final byte[] mTpdu;
        private final String mOriginatingAddress;

        SmsPpDownloadCommand(int cid, Listener listener, String rpOriginatingAddress,
                boolean uriTruncated, byte[] tpdu, String originatingAddress) {
            super(cid, SERVICE_DATA_DOWNLOAD_VIA_SMS_PP, listener);
            mRpOriginatingAddress = rpOriginatingAddress;
            mUriTruncated = uriTruncated;
            mTpdu = tpdu;
            mOriginatingAddress = originatingAddress;
        }

        /**
         * Checks if the URI is truncated or not.
         *
         * @return true if the URI is truncated, false otherwise.
         */
        public boolean isUriTruncated() {
            return mUriTruncated;
        }

        /**
         * Returns the RP_Originating_Address of the service center.
         *
         * @return The RP_Originating_Address.
         */
        public String getRpOriginatingAddress() {
            return mRpOriginatingAddress;
        }

        /**
         * Returns the SMS T-PDU data.
         *
         * @return The SMS T-PDU data.
         */
        public byte[] getTpdu() {
            return mTpdu;
        }

        /**
         * Returns the originating address of the sender of the short message.
         *
         * @return The originating address or null if the URI supports and it's longer than
         *         the maximum length that can be transmitted to the UICC.
         */
        public String getOriginatingAddress() {
            return mOriginatingAddress;
        }
    }

    /**
     * Base class for USAT command response.
     */
    class CommandResponse {
        private final Command mCommand;
        private final int mResult;

        CommandResponse(Command command, @Result int result) {
            mCommand = command;
            mResult = result;
        }

        /**
         * Returns the {@link Command} that was previously sent of this response.
         *
         * @return The {@link Command} that was previously sent.
         */
        public Command getCommand() {
            return mCommand;
        }

        /**
         * Returns the result type of this response.
         *
         * @return The result type. Valid values are
         *         {@link #RESULT_ALLOWED},
         *         {@link #RESULT_NOT_ALLOWED},
         *         {@link #RESULT_ALLOWED_WITH_MODIFICATION},
         *         {@link #RESULT_DATA_DOWNLOAD_OK},
         *         {@link #RESULT_DATA_DOWNLOAD_ERROR}.
         */
        public @Result int getResult() {
            return mResult;
        }
    }

    /**
     * USAT command response for the call control.
     */
    class CallControlCommandResponse extends CommandResponse {
        private final @CallControlType int mCcType;
        private final String mDialedString;
        private final @MediaType int mMediaType;

        CallControlCommandResponse(Command command, @Result int result,
                @CallControlType int ccType, String dialedString, @MediaType int mediaType) {
            super(command, result);
            mCcType = ccType;
            mDialedString = dialedString;
            mMediaType = mediaType;
        }

        /**
         * Returns the call control type of this response.
         *
         * @return The service type. Valid values are
         *         {@link #CALL_CONTROL_TYPE_MO_CALL},
         *         {@link #CALL_CONTROL_TYPE_SS},
         *         {@link #CALL_CONTROL_TYPE_USSD}.
         */
        public @CallControlType int getCcType() {
            return mCcType;
        }

        /**
         * Returns the dialed number / SS string / USSD string of this response.
         * The type of returned value will be determined by the return value of
         * {@link #getCcType()}.
         *
         * @return The dialed string - dialed number or SS string or USSD string.
         */
        public String getDialedString() {
            return mDialedString;
        }

        /**
         * Returns the media type of this response.
         *
         * @return The media type. Valid values are
         *         {@link #MEDIA_TYPE_VOICE},
         *         {@link #MEDIA_TYPE_VIDEO}.
         */
        public @MediaType int getMediaType() {
            return mMediaType;
        }
    }

    /**
     * USAT command response for MO SMS control.
     */
    class MoSmsControlCommandResponse extends CommandResponse {
        private final String mRpDestinationAddress;
        private final String mTpDestinationAddress;

        MoSmsControlCommandResponse(Command command, @Result int result,
                String rpDestinationAddress, String tpDestinationAddress) {
            super(command, result);
            mRpDestinationAddress = rpDestinationAddress;
            mTpDestinationAddress = tpDestinationAddress;
        }

        /**
         * Returns the RP_Destination_Address of the service center.
         *
         * @return The RP_Destination_Address or null if it's not allowed or not modified.
         */
        public String getRpDestinationAddress() {
            return mRpDestinationAddress;
        }

        /**
         * Returns the TP_Destination_Address.
         *
         * @return The TP_Destination_Address or null if it's not allowed or not modified.
         */
        public String getTpDestinationAddress() {
            return mTpDestinationAddress;
        }
    }

    /**
     * USAT command response for SMS-PP download.
     */
    class SmsPpDownloadCommandResponse extends CommandResponse {
        private final byte[] mResponseData;

        SmsPpDownloadCommandResponse(Command command, @Result int result, byte[] responseData) {
            super(command, result);
            mResponseData = responseData;
        }

        /**
         * Returns the response data from the UICC.
         *
         * If the result value is {@link #RESULT_DATA_DOWNLOAD_OK}, then this response date will
         * be filled in the TP-User-Data element of the RP-ACK message and the caller sends back
         * to the network.
         * If the result value is {@link #RESULT_DATA_DOWNLOAD_ERROR}, then it will be a null and
         * the caller will send back an RP-ERROR message to the network with the TP-FCS value
         * indicating "SIM Application Toolkit Busy".
         *
         * @return The response data from the UICC.
         *         The length of this data is 1 to 128 if present.
         */
        public byte[] getResponseData() {
            return mResponseData;
        }
    }

    /**
     * Listener interface to receive the response from UICC for USAT command.
     */
    public interface Listener {
        /**
         * Invoked when the command response is received from the UICC.
         *
         * @param response The command response from UICC.
         */
        void onCommandResponse(CommandResponse response);
    }
}
