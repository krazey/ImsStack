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

import com.android.imsstack.util.ImsLog;

/**
 * This class handles the UserAgent value which is included ACS request
 */
public class UserAgent {
    private static final String PREFIX = "IM-client/OMA1.0";

    private final String mTerminalVendor;
    private final String mTerminalVersion;
    private final String mTerminalName;
    private final String mClientVendor;
    private final String mClientVersion;
    private final int mSlotId;

    private String mGBAProductToken;

    /**
     * set GBA product token which should be included UserAgent value if the AC server required
     * @param gbaProductToken product token
     */
    public void setGBAProductToken(String gbaProductToken) {
        mGBAProductToken = gbaProductToken;
    }

    /**
     * generate final UserAgent value
     * @return UserAgent value
     */
    public String generate() {
        // this scheme is only available for TMUS
        // for ATT mTerminalName + "/" + mTerminalVersion
        String userAgent = PREFIX + " " + mTerminalVendor + "/" + mTerminalName + "-"
                + mTerminalVersion + " " + mClientVendor + "/" + mClientVersion;

        if (mGBAProductToken != null && !mGBAProductToken.isEmpty()) {
            userAgent += ";" + mGBAProductToken;
        }

        ImsLog.i("[" + mSlotId + "] " + "userAgent :" + userAgent);

        return userAgent;
    }

    /**
     * create UserAgent object by Builder
     * @param Builder object includes all required values
     * @return instance of UserAgent
     */
    private UserAgent(Builder builder) {
        mTerminalVendor = builder.mTerminalVendor;
        mTerminalVersion = builder.mTerminalVersion;
        mTerminalName = builder.mTerminalName;
        mClientVendor = builder.mClientVendor;
        mClientVersion = builder.mClientVersion;
        mSlotId = builder.mSlotId;
    }

    /**
     * Builder for creation UserAgent instance and validation of required values.
     */
    private static class Builder {
        private String mTerminalVendor = "";
        private String mTerminalVersion = "";
        private String mTerminalName = "";
        private String mClientVendor = "";
        private String mClientVersion = "";
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
         * @param terminalVersion software version of terminal
         * @return instance of Builder
         */
        public Builder setTerminalVersion(String terminalVersion) {
            mTerminalVersion = terminalVersion;
            return this;
        }

        /**
         * set Terminal name
         * @param terminalName name of terminal
         * @return instance of Builder
         */
        public Builder setTerminalName(String terminalName) {
            mTerminalName = terminalName;
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
         * create UserAgent and check required values
         * @return instance of UserAgent
         */
        public UserAgent build() {
            if (mSlotId == SubscriptionManager.INVALID_PHONE_INDEX
                    || mTerminalVendor.isEmpty() || mTerminalVersion.isEmpty()
                    || mTerminalName.isEmpty() || mClientVendor.isEmpty()
                    || mClientVersion.isEmpty()) {
                throw new IllegalArgumentException("slotId : " + mSlotId
                        + " terminalVendor : " + mTerminalVendor
                        + " terminalVersion : " + mTerminalVersion
                        + " terminalName : " + mTerminalName
                        + " clientVendor : " + mClientVendor
                        + " clientVersion : " + mClientVersion);
            }

            return new UserAgent(this);
        }
    }
}
