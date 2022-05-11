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

package com.android.imsstack.enabler.acs.impl;

import android.telephony.SubscriptionManager;
import android.text.TextUtils;

import com.android.imsstack.util.ImsLog;

import java.io.UnsupportedEncodingException;
import java.net.MalformedURLException;
import java.net.URL;
import java.net.URLEncoder;

/**
 * This class handles the HTTP and HTTPS Request URI for ACS request
 */
public class RequestUri {
    private static final String TAG = RequestUri.class.getSimpleName();
    private static final String DEFAULT_CHAT_SET = "UTF-8";
    private static final String PROTOCOL_HTTP = "http://";
    private static final String PROTOCOL_HTTPS = "https://";

    private final int mSlotId;
    private final String mImsi;
    private final String mImei;
    private final String mMcc;
    private final String mMnc;
    private final String mRcsVersion;
    private final String mRcsProfile;
    private final String mTerminalVendor;
    private final String mTerminalModel;
    private final String mTerminalSwVersion;
    private final String mClientVendor;
    private final String mClientVersion;
    private final String mProvisioningVersion;

    private String mAcVersion;
    private String mDefaultSmsApp;
    private String mRcsState;
    private String mDefaultVvmApp;
    private String mSmsPort;

    private String mOtp;
    private String mToken;

    /**
     * change AC version included in AC request by the device
     * @param acVersion new AC version
     */
    public void updateAcVersion(String acVersion) {
        mAcVersion = acVersion;
    }

    /**
     * change RCS state included in the AC request by the device.
     * @param rcsState new RCS state.
     */
    public void updateRcsState(String rcsState) {
        mRcsState = rcsState;
    }

    /**
     * set OTP
     * @param otp OTP value received from server
     */
    public void setOtp(String otp) {
        mOtp = otp;
    }

    /**
     * set token
     * @param token token value in previous response from server
     */
    public void setToken(String token) {
        mToken = token;
    }

    /**
     * update SMS port want to receive OTP. Default port is 37273.
     * @param smsPort SMS port number
     */
    public void updateSmsPort(String smsPort) {
        mSmsPort = smsPort;
    }

    /**
     * get request URI for HTTP.
     * @return instance of URL will be used HTTP stack
     */
    public URL getHttpUrl() {
        URL url = null;

        try {
            url = new URL(PROTOCOL_HTTP + getAbsoluteUri());
            ImsLog.d("URL : " + url.toString());
        } catch (MalformedURLException e) {
            ImsLog.e(e.getMessage());
        }

        return url;
    }

    /**
     * get request URI for HTTPS.
     * @return instance of URL will be used HTTPs stack
     */
    public URL getHttpsUrl() {
        URL url = null;

        try {
            url = new URL(PROTOCOL_HTTPS + getAbsoluteUri() + encodeParameters());
            ImsLog.d("URL : " + url.toString());
        } catch (MalformedURLException e) {
            ImsLog.e(e.getMessage());
        }

        return url;
    }

    /**
     * create UserAgent object by Builder
     * @param Builder object includes all required values
     * @return instance of RequestUri
     */
    private RequestUri(Builder builder) {
        mAcVersion = builder.mAcVersion;
        mImsi = builder.mImsi;
        mImei = builder.mImei;
        mMcc = builder.mMcc;
        mMnc = builder.mMnc;
        mRcsVersion = builder.mRcsVersion;
        mRcsProfile = builder.mRcsProfile;
        mTerminalVendor = builder.mTerminalVendor;
        mTerminalSwVersion = builder.mTerminalSwVersion;
        mTerminalModel = builder.mTerminalModel;
        mClientVendor = builder.mClientVendor;
        mClientVersion = builder.mClientVersion;
        mProvisioningVersion = builder.mProvisioningVersion;

        mSmsPort = builder.mSmsPort;

        mSlotId = builder.mSlotId;
    }

    private String getAbsoluteUri() {
        if (!mMnc.isEmpty() && !mMcc.isEmpty()) {
            return new StringBuilder()
                    .append("config.rcs.mnc")
                    .append(mMnc)
                    .append(".mcc")
                    .append(mMcc)
                    .append(".pub.3gppnetwork.org")
                    .toString();
        }
        return null;
    }

    private String encodeOtp() {
        String retVal = "";
        if (!TextUtils.isEmpty(mOtp)) {
            retVal = "?OTP=" + mOtp;
        }

        return retVal;
    }

    private String encodeToken() {
        String retVal = "&token=";
        if (!TextUtils.isEmpty(mToken)) {
            retVal += mToken;
        }

        return retVal;
    }

    private String encodeParameters() {
        String encodeParam;
        try {
            encodeParam = nullCheckACParamValue(String.format("?vers=%s",
                    URLEncoder.encode(mAcVersion, DEFAULT_CHAT_SET)));
            encodeParam += nullCheckACParamValue(String.format("&IMSI=%s",
                    URLEncoder.encode(mImsi, DEFAULT_CHAT_SET)));
            encodeParam += nullCheckACParamValue(String.format("&rcs_version=%s",
                    URLEncoder.encode(mRcsVersion, DEFAULT_CHAT_SET)));

            // ATT & TMO does not require
            encodeParam += nullCheckACParamValue(String.format("&rcs_profile=%s",
                    URLEncoder.encode(mRcsProfile, DEFAULT_CHAT_SET)));

            // ATT does not require
            encodeParam += nullCheckACParamValue(String.format("&client_vendor=%s",
                    URLEncoder.encode(mClientVendor, DEFAULT_CHAT_SET)));

            // ATT does not require
            encodeParam += nullCheckACParamValue(String.format("&client_version=%s",
                    URLEncoder.encode(mClientVersion, DEFAULT_CHAT_SET)));

            encodeParam += nullCheckACParamValue(String.format("&terminal_vendor=%s",
                    URLEncoder.encode(mTerminalVendor, DEFAULT_CHAT_SET)));
            encodeParam += nullCheckACParamValue(String.format("&terminal_model=%s",
                    URLEncoder.encode(mTerminalModel, DEFAULT_CHAT_SET)));
            encodeParam += nullCheckACParamValue(String.format("&terminal_sw_version=%s",
                    URLEncoder.encode(mTerminalSwVersion, DEFAULT_CHAT_SET)));
            encodeParam += nullCheckACParamValue(String.format("&IMEI=%s",
                    URLEncoder.encode(mImei, DEFAULT_CHAT_SET)));
            encodeParam += nullCheckACParamValue(String.format("&default_sms_app=%s",
                    URLEncoder.encode(mDefaultSmsApp, DEFAULT_CHAT_SET)));
            encodeParam += nullCheckACParamValue(String.format("&rcs_state=%s",
                    URLEncoder.encode(mRcsState, DEFAULT_CHAT_SET)));

            // ATT & TMO requires
            encodeParam += nullCheckACParamValue(String.format("&provisioning_version=%s",
                    URLEncoder.encode(mProvisioningVersion, DEFAULT_CHAT_SET)));

            // TMO requires
            encodeParam += nullCheckACParamValue(String.format("&default_vvm_app=%s",
                    URLEncoder.encode(mDefaultVvmApp, DEFAULT_CHAT_SET)));

        } catch (UnsupportedEncodingException e) {
            ImsLog.e("encodeParameters : " + e.getMessage());
            encodeParam = "";
        }

        return encodeParam;
    }

    private String nullCheckACParamValue(String parameter) {
        if (parameter.indexOf("=") == (parameter.length() - 1) || parameter.contains("Not+used")) {
            return "";
        }
        return parameter;
    }

    /**
     * Builder for creation UserAgent instance and validation of required values.
     */
    private static class Builder {
        private String mAcVersion = "0";
        private String mImsi = "";
        private String mImei = "";
        private String mMcc = "";
        private String mMnc = "";

        // RcsProfile  RcsVersion   ProvisioningVersion
        // UP_1.0      6.0          2.0
        // UP_2.0      7.0          4.0
        // UP_2.2      7.0          4.0
        private String mRcsProfile = "UP_1.0";
        private String mRcsVersion = "6.0";
        private String mProvisioningVersion = "2.0";
        private String mTerminalVendor = "";
        private String mTerminalSwVersion = "";
        private String mTerminalModel = "";
        private String mClientVendor = "";
        private String mClientVersion = "";
        private String mSmsPort = "37273";
        private int mSlotId = SubscriptionManager.INVALID_PHONE_INDEX;

        /**
         * creator Builder
         * @param slotId Slot or Phone Id
         * @return instance of Builder
         */
        Builder(int slotId) {
            mSlotId = slotId;
        }

        /**
         * set AC version in previous received provisioning data.
         * @param acVersion is current provisioning version or 0 which means initial value.
         * @return instance of Builder
         */
        public Builder setAcVersion(String acVersion) {
            mAcVersion = acVersion;
            return this;
        }

        /**
         * set International Mobile Subscriber ID
         * @param imsi is the IMSI value of associated SIM
         * @return instance of Builder
         */
        public Builder setmImsi(String imsi) {
            mImsi = imsi;
            return this;
        }

        /**
         * set International Mobile Equipment ID
         * @param imei is the IMEI value of device
         * @return instance of Builder
         */
        public Builder setImei(String imei) {
            mImei = imei;
            return this;
        }

        /**
         * set MCC and MNC
         * @param mcc is MCC value of associated SIM
         * @param mnc is MNC value of associated SIM
         * @return instance of Builder
         */
        public Builder setMccMnc(String mcc, String mnc) {
            mMcc = mcc;
            mMnc = mnc;
            return this;
        }

        /**
         * set RCS version
         * @param rcsVersion is RCS version which client supports
         * @return instance of Builder
         */
        public Builder setRcsVersion(String rcsVersion) {
            mRcsVersion = rcsVersion;
            return this;
        }

        /**
         * set RCS profile
         * @param rcsProfile is RCS profile which client supports
         * @return instance of Builder
         */
        public Builder setRcsProfile(String rcsProfile) {
            mRcsProfile = rcsProfile;
            return this;
        }

        /**
         * set Terminal vendor name
         * @param terminalVendor name of terminal vendor
         * @return instance of Builder
         */
        public Builder setTerminalVendor(String terminalVendor) {
            mTerminalVendor = terminalVendor;
            return this;
        }

        /**
         * set Terminal software version
         * @param terminalSwVersion software version of terminal
         * @return instance of Builder
         */
        public Builder setTerminalVersion(String terminalSwVersion) {
            mTerminalSwVersion = terminalSwVersion;
            return this;
        }

        /**
         * set Terminal name
         * @param terminalModel name of terminal
         * @return instance of Builder
         */
        public Builder setTerminalName(String terminalModel) {
            mTerminalModel = terminalModel;
            return this;
        }

        /**
         * set client vendor name
         * @param clientVendor name of client vendor
         * @return instance of Builder
         */
        public Builder setClientVendor(String clientVendor) {
            mClientVendor = clientVendor;
            return this;
        }

        /**
         * set client version
         * @param clientVersion version of client
         * @return instance of Builder
         */
        public Builder setClientVersion(String clientVersion) {
            mClientVersion = mClientVersion;
            return this;
        }

        /**
         * set SMS port number which device wants to receive SMS for OTP
         * if this function is not called, initial value 37273 will be used.
         * @param smsPort version of client
         * @return instance of Builder
         */
        public Builder setSmsPort(String smsPort) {
            mSmsPort = smsPort;
            return this;
        }

        /**
         * create UserAgent and check required values
         * @return instance of UserAgent
         */
        public RequestUri build() {
            // TODO : check mandatory and optional parameters
            if (mSlotId == SubscriptionManager.INVALID_PHONE_INDEX
                    || mTerminalVendor.isEmpty() || mTerminalSwVersion.isEmpty()
                    || mTerminalModel.isEmpty() || mClientVendor.isEmpty()
                    || mClientVersion.isEmpty()) {
                throw new IllegalArgumentException("slotId : " + mSlotId
                        + " terminalVendor : " + mTerminalVendor
                        + " terminalVersion : " + mTerminalSwVersion
                        + " terminalName : " + mTerminalModel
                        + " clientVendor : " + mClientVendor
                        + " clientVersion : " + mClientVersion);
            }

            return new RequestUri(this);
        }
    }
}
