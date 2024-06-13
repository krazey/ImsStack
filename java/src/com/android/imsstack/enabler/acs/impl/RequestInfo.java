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

import android.annotation.NonNull;
import android.os.Build;
import android.telephony.SubscriptionManager;
import android.text.TextUtils;

import com.android.imsstack.base.AppContext;
import com.android.imsstack.base.TelephonyManagerProxy;
import com.android.imsstack.enabler.acs.AcServiceClientInfo;
import com.android.imsstack.util.ImsLog;
import com.android.imsstack.util.ImsUtils;
import com.android.internal.annotations.VisibleForTesting;

import java.io.UnsupportedEncodingException;
import java.net.MalformedURLException;
import java.net.URL;
import java.net.URLEncoder;

/**
 * This class handles the HTTP and HTTPS Request URI and value of UserAgent header
 */
public class RequestInfo {
    private static final String PREFIX = "IM-client/OMA1.0";
    private static final String DEFAULT_CHAT_SET = "UTF-8";
    private static final String PROTOCOL_HTTP = "http://";
    private static final String PROTOCOL_HTTPS = "https://";

    private final RequestInfoBuilder mBuilder;
    private String mGBAProductToken;
    private String mRcsState = "0";
    private String mOtp;
    private String mToken;

    // 0 : OS does not allow user to select SMS application
    // 1 :  RCS messaging client is selected as the default SMS application
    // 2 :  RCS messaging client is not selected as the default SMS application
    private String mDefaultSmsApp = "1";
    private String mDefaultVvmApp;

    /**
     * create RequestInfo object by RequestInfoBuilder
     * @param builder object includes all required values
     * @return instance of RequestInfo
     */
    private RequestInfo(RequestInfoBuilder builder) {
        mBuilder = builder;
    }

    /**
     * generate final UserAgent value
     * @return UserAgent string
     */
    public String generateUserAgent() {
        // this scheme is only available for TMUS
        // for ATT mTerminalName + "/" + mTerminalVersion
        StringBuilder builder = new StringBuilder();

        builder.append(PREFIX);
        builder.append(" " + mBuilder.mTerminalVendor);
        builder.append("/" + mBuilder.mTerminalModel);
        builder.append("-" + mBuilder.mTerminalSwVersion);
        builder.append(" " + mBuilder.mClientVendor);
        builder.append("/" + mBuilder.mClientVersion);

        if (!TextUtils.isEmpty(mGBAProductToken)) {
            builder.append(";" + mGBAProductToken);
        }

        ImsLog.i("[" + mBuilder.mSlotId + "] userAgent :" + builder.toString());

        return builder.toString();
    }

    /**
     * set GBA product token which should be included UserAgent value if the AC server required
     * @param gbaProductToken product token
     */
    public void setGBAProductToken(String gbaProductToken) {
        mGBAProductToken = gbaProductToken;
    }

    /**
     * change AC version included in AC request by the device
     * @param acVersion new AC version
     */
    public void updateAcVersion(String acVersion) {
        ImsLog.i("[" + mBuilder.mSlotId + "] Rcs version from "
                + mBuilder.mAcVersion + " to " + acVersion);
        mBuilder.mAcVersion = acVersion;
    }

    /**
     * change RCS state included in the AC request by the device.
     * @param rcsState new RCS state.
     */
    public void updateRcsState(String rcsState) {
        ImsLog.i("[" + mBuilder.mSlotId + "] Rcs State from " + mRcsState + " to " + rcsState);
        mRcsState = rcsState;
    }

    /**
     * set OTP
     * @param otp OTP value received from server
     */
    public void setOtp(String otp) {
        if (!ImsUtils.IS_USER) {
            ImsLog.i("[" + mBuilder.mSlotId + "] Otp " + otp);
        }
        mOtp = otp;
    }

    /**
     * set token
     * @param token token value in previous response from server
     */
    public void setToken(String token) {
        if (!ImsUtils.IS_USER) {
            ImsLog.i("[" + mBuilder.mSlotId + "] token " + token);
        }
        mToken = token;
    }

    /**
     * set default SMS App name
     * @param defaultSmsApp Default SMS App
     */
    public void setDefaultSmsApp(String defaultSmsApp) {
        if (!ImsUtils.IS_USER) {
            ImsLog.i("[" + mBuilder.mSlotId + "] DefaultSmsApp " + defaultSmsApp);
        }
        mDefaultSmsApp = defaultSmsApp;
    }

    /**
     * set default VVM App name
     * @param defaultVvmApp Default VVM App
     */
    public void setDefaultVvmApp(String defaultVvmApp) {
        if (!ImsUtils.IS_USER) {
            ImsLog.i("[" + mBuilder.mSlotId + "] DefaultVvmApp " + defaultVvmApp);
        }
        mDefaultVvmApp = defaultVvmApp;
    }

    /**
     * update SMS port want to receive OTP. Default port is 37273.
     * @param smsPort SMS port number
     */
    public void updateSmsPort(String smsPort) {
        ImsLog.i(
                "[" + mBuilder.mSlotId + "] sms port from " + mBuilder.mSmsPort + " to " + smsPort);
        mBuilder.mSmsPort = smsPort;
    }

    /**
     * get request URI for HTTP.
     * @return instance of URL will be used HTTP stack
     */
    public URL getHttpUrl() {
        URL url = null;

        try {
            url = new URL(PROTOCOL_HTTP + getAbsoluteUri());
            ImsLog.d("[" + mBuilder.mSlotId + "] " + "URL : " + url.toString());
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
            ImsLog.d("[" + mBuilder.mSlotId + "] " + "URL : " + url.toString());
        } catch (MalformedURLException e) {
            ImsLog.e(e.getMessage());
        }

        return url;
    }

    private String getAbsoluteUri() {
        // TODO : need to support changing host name for testing
        if (!TextUtils.isEmpty(mBuilder.mMnc) && !TextUtils.isEmpty(mBuilder.mMcc)) {
            return new StringBuilder()
                    .append("config.rcs.mnc")
                    .append(mBuilder.mMnc)
                    .append(".mcc")
                    .append(mBuilder.mMcc)
                    .append(".pub.3gppnetwork.org")
                    .toString();
        }
        return null;
    }

    private String encodeParameters() {
        StringBuilder builder = new StringBuilder();
        try {
            builder.append(nullCheckACParamValue(String.format("?vers=%s",
                    URLEncoder.encode(mBuilder.mAcVersion, DEFAULT_CHAT_SET))));
            builder.append(nullCheckACParamValue(String.format("&IMSI=%s",
                    URLEncoder.encode(mBuilder.mImsi, DEFAULT_CHAT_SET))));
            builder.append(nullCheckACParamValue(String.format("&rcs_version=%s",
                    URLEncoder.encode(mBuilder.mRcsVersion, DEFAULT_CHAT_SET))));

            // ATT & TMO does not require
            builder.append(nullCheckACParamValue(String.format("&rcs_profile=%s",
                    URLEncoder.encode(mBuilder.mRcsProfile, DEFAULT_CHAT_SET))));

            // ATT does not require
            builder.append(nullCheckACParamValue(String.format("&client_vendor=%s",
                    URLEncoder.encode(mBuilder.mClientVendor, DEFAULT_CHAT_SET))));

            // ATT does not require
            builder.append(nullCheckACParamValue(String.format("&client_version=%s",
                    URLEncoder.encode(mBuilder.mClientVersion, DEFAULT_CHAT_SET))));

            builder.append(nullCheckACParamValue(String.format("&terminal_vendor=%s",
                    URLEncoder.encode(mBuilder.mTerminalVendor, DEFAULT_CHAT_SET))));
            builder.append(nullCheckACParamValue(String.format("&terminal_model=%s",
                    URLEncoder.encode(mBuilder.mTerminalModel, DEFAULT_CHAT_SET))));
            builder.append(nullCheckACParamValue(String.format("&terminal_sw_version=%s",
                    URLEncoder.encode(mBuilder.mTerminalSwVersion, DEFAULT_CHAT_SET))));
            builder.append(nullCheckACParamValue(String.format("&IMEI=%s",
                    URLEncoder.encode(mBuilder.mImei, DEFAULT_CHAT_SET))));
            builder.append(nullCheckACParamValue(String.format("&default_sms_app=%s",
                    URLEncoder.encode(mDefaultSmsApp, DEFAULT_CHAT_SET))));
            builder.append(nullCheckACParamValue(String.format("&rcs_state=%s",
                    URLEncoder.encode(mRcsState, DEFAULT_CHAT_SET))));
            builder.append(nullCheckACParamValue(String.format("&provisioning_version=%s",
                    URLEncoder.encode(mBuilder.mProvisioningVersion, DEFAULT_CHAT_SET))));
            builder.append(nullCheckACParamValue(String.format("&SMS_port=%s",
                    URLEncoder.encode(mBuilder.mSmsPort, DEFAULT_CHAT_SET))));

            // TMO requires
            if (!TextUtils.isEmpty(mDefaultVvmApp)) {
                builder.append(nullCheckACParamValue(String.format("&default_vvm_app=%s",
                        URLEncoder.encode(mDefaultVvmApp, DEFAULT_CHAT_SET))));
            }

            builder.append(encodeOtp());
            builder.append(encodeToken());

            // TODO : need to check friendly_device_name for VZW

        } catch (UnsupportedEncodingException | NullPointerException e) {
            ImsLog.e("[" + mBuilder.mSlotId + "] " + "encodeParameters : " + e.getMessage());
            return "";
        }

        return builder.toString();
    }

    private String nullCheckACParamValue(String parameter) {
        if (parameter.indexOf("=") == (parameter.length() - 1) || parameter.contains("Not+used")) {
            return "";
        }
        return parameter;
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

    /**
     * This class handles values to create request URI and UserAgent header.
     */
    public static class RequestInfoBuilder {
        private static final String PROFILE_UP_1_0 = "UP_1.0";

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
        private boolean mRcsEnabledByUser = true;
        private int mSlotId = SubscriptionManager.INVALID_PHONE_INDEX;
        private int mSubId = SubscriptionManager.INVALID_SUBSCRIPTION_ID;

        /**
         * creator Builder
         *
         * @param slotId Slot or Phone Id.
         * @param acServiceClientInfo Rcs Client information from message application.
         * @return instance of Builder.
         */
        RequestInfoBuilder(int slotId, int subId,
                @NonNull AcServiceClientInfo acServiceClientInfo) {
            this(slotId, subId, acServiceClientInfo, AppContext.getTelephonyManagerProxy(subId));
        }

        @VisibleForTesting
        public RequestInfoBuilder(int slotId, int subId, AcServiceClientInfo acServiceClientInfo,
                TelephonyManagerProxy tmp) {
            mSlotId = slotId;
            mSubId = subId;

            mClientVendor = acServiceClientInfo.getClientVendor();
            mClientVersion = acServiceClientInfo.getClientVersion();
            mRcsEnabledByUser = acServiceClientInfo.isRcsEnabledByUser();
            setRcsProfile(acServiceClientInfo.getRcsProfile());
            setRcsVersion(acServiceClientInfo.getRcsVersion());

            setTerminalVendor(Build.MANUFACTURER);
            setTerminalVersion(Build.DISPLAY);
            setTerminalName(Build.MODEL);

            String mcc = "";
            String mnc = "";
            String mccMnc = tmp.getSimOperator();
            if (!TextUtils.isEmpty(mccMnc)) {
                try {
                    mcc = mccMnc.substring(0, 3);
                    mnc = mccMnc.substring(3);

                } catch (NumberFormatException e) {
                    ImsLog.e(e.getMessage());
                }
            }
            String imsi = tmp.getSubscriberId();
            String imei = tmp.getImei(mSlotId);

            setMccMnc(mcc, mnc);
            setImsi(imsi);
            setImei(imei);
        }

        /**
         * set AC version in previous received provisioning data.
         * @param acVersion is current provisioning version or 0 which means initial value.
         * @return instance of Builder
         */
        public RequestInfoBuilder setAcVersion(String acVersion) {
            ImsLog.i("[" + mSlotId + "] acVersion from " + mAcVersion + " to " + acVersion);
            mAcVersion = acVersion;
            return this;
        }

        /**
         * set International Mobile Subscriber ID
         * @param imsi is the IMSI value of associated SIM
         * @return instance of Builder
         */
        public RequestInfoBuilder setImsi(String imsi) {
            mImsi = imsi;
            return this;
        }

        /**
         * set International Mobile Equipment ID
         * @param imei is the IMEI value of device
         * @return instance of Builder
         */
        public RequestInfoBuilder setImei(String imei) {
            mImei = imei;
            return this;
        }

        /**
         * set MCC and MNC
         * @param mcc is MCC value of associated SIM
         * @param mnc is MNC value of associated SIM
         * @return instance of Builder
         */
        public RequestInfoBuilder setMccMnc(String mcc, String mnc) {
            mMcc = mcc;
            mMnc = mnc;
            return this;
        }

        /**
         * set RCS version
         * @param rcsVersion is RCS version which client supports
         * @return instance of Builder
         */
        public RequestInfoBuilder setRcsVersion(String rcsVersion) {
            if (TextUtils.isEmpty(rcsVersion)) {
                ImsLog.i("[" + mSlotId + "] version is empty, default value " + mRcsVersion
                        + " will be used");
                return this;
            }

            ImsLog.i("[" + mSlotId + "] Rcs version is different"
                    + " old : " + mRcsVersion
                    + " new : " + rcsVersion);
            mRcsVersion = rcsVersion;

            return this;
        }

        /**
         * set RCS profile
         * @param rcsProfile is RCS profile which client supports
         * @return instance of Builder
         */
        public RequestInfoBuilder setRcsProfile(String rcsProfile) {
            if (TextUtils.isEmpty(rcsProfile)) {
                ImsLog.i("[" + mSlotId + "] profile is empty, default value"
                        + mRcsProfile + ", " + mProvisioningVersion + " will be used");
                return this;
            }

            mRcsProfile = rcsProfile;
            if (TextUtils.equals(rcsProfile, PROFILE_UP_1_0)) {
                mRcsVersion = "6.0";
                mProvisioningVersion = "2.0";
            } else {
                mRcsVersion = "7.0";
                mProvisioningVersion = "4.0";
            }

            return this;
        }

        /**
         * set Terminal vendor name
         * @param terminalVendor name of terminal vendor
         * @return instance of Builder
         */
        public RequestInfoBuilder setTerminalVendor(String terminalVendor) {
            mTerminalVendor = terminalVendor;
            return this;
        }

        /**
         * set Terminal software version
         * @param terminalSwVersion software version of terminal
         * @return instance of Builder
         */
        public RequestInfoBuilder setTerminalVersion(String terminalSwVersion) {
            mTerminalSwVersion = terminalSwVersion;
            return this;
        }

        /**
         * set Terminal name
         * @param terminalModel name of terminal
         * @return instance of Builder
         */
        public RequestInfoBuilder setTerminalName(String terminalModel) {
            mTerminalModel = terminalModel;
            return this;
        }

        /**
         * set SMS port number which device wants to receive SMS for OTP
         * if this function is not called, initial value 37273 will be used.
         * @param smsPort version of client
         * @return instance of Builder
         */
        public RequestInfoBuilder setSmsPort(String smsPort) {
            mSmsPort = smsPort;
            return this;
        }

        /**
         * create UserAgent and check required values
         * @return instance of RequestInfo
         */
        public RequestInfo build() {
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

            return new RequestInfo(this);
        }

        /**
         * Return String value includes all attributes, and should be used for debugging only.
         * @return String includes all attributes
         */
        public String toString() {
            StringBuilder buffer = new StringBuilder();
            buffer.append("SlotId:" + mSlotId);
            buffer.append(", AcVersion:" + mAcVersion);
            buffer.append(", RcsProfile:" + mRcsProfile);
            buffer.append(", RcsVersion:" + mRcsVersion);
            buffer.append(", ProvisioningVersion:" + mProvisioningVersion);
            buffer.append(", TerminalVendor:" + mTerminalVendor);
            buffer.append(", TerminalSwVersion:" + mTerminalSwVersion);
            buffer.append(", TerminalModel:" + mTerminalModel);
            buffer.append(", ClientVendor:" + mClientVendor);
            buffer.append(", ClientVersion:" + mClientVersion);
            buffer.append(", SmsPort:" + mSmsPort);
            buffer.append(", RcsEnabledByUser:" + mRcsEnabledByUser);

            if (!ImsUtils.IS_USER) {
                buffer.append(", Imsi:" + mImsi);
                buffer.append(", Imei:" + mImei);
                buffer.append(", Mcc:" + mMcc);
                buffer.append(", Mnc:" + mMnc);
            }

            return buffer.toString();
        }
    }
}
