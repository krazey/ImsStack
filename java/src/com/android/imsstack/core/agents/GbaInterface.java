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
import android.annotation.NonNull;
import android.telephony.TelephonyManager;

import java.lang.annotation.Retention;
import java.lang.annotation.RetentionPolicy;

/**
 * Provides the APIs for getting B-TID and NAF Key of bootstrapped security association.
 */
public interface GbaInterface extends IAgent {

    /**
     * GbaCredentials used to pass the result of Generic Bootstrapping including reason code,
     * B-TID and NAF Key
     */
    final class GbaCredentials {
        /**
         * Result of GBA operation {@link AuthenticationResult}
         */
        private final @AuthenticationResult int mResult;
        /**
         * One of GBA failure reason {@link AuthenticationFailureReason}
         */
        private final @AuthenticationFailureReason int mReason;
        /**
         * Bootstrapping transaction ID received from BSF
         */
        private final String mTransactionId;
        /**
         * Ks_(ext)_NAF key received from BSF
         */
        private final String mKey;

        public GbaCredentials(@NonNull String transactionId, @NonNull String key) {
            this.mResult = RESULT_SUCCESS;
            this.mReason = GBA_FAILURE_REASON_NONE;
            this.mTransactionId = transactionId;
            this.mKey = key;
        }

        public GbaCredentials(@AuthenticationFailureReason int reason) {
            this.mResult = RESULT_FAILURE;
            this.mReason = reason;
            this.mTransactionId = null;
            this.mKey = null;
        }

        public @AuthenticationResult int getResult() {
            return mResult;
        }

        public @AuthenticationFailureReason int getReason() {
            return mReason;
        }

        public String getTransactionId() {
            return mTransactionId;
        }

        public String getKey() {
            return mKey;
        }
    }

    /**
     * Result of GBA operation
     */
    int RESULT_SUCCESS = 0;
    int RESULT_FAILURE = 1;

    @IntDef(prefix = {"RESULT_"}, value = {
            RESULT_SUCCESS,
            RESULT_FAILURE
    })
    @Retention(RetentionPolicy.SOURCE)
    public @interface AuthenticationResult {}

    /**
     * GBA failure reason which occurs ImsStack internally
     */
    int GBA_FAILURE_REASON_NONE = 1000;
    int GBA_FAILURE_REASON_KEY_INVALID = 1001;
    int GBA_FAILURE_REASON_TIMEOUT = 1002;
    int GBA_FAILURE_REASON_TLS_CIPHERSUITE_NOT_SUPPORTED = 1003;
    int GBA_FAILURE_REASON_CANCELLATION_EXCEPTION = 1004;
    int GBA_FAILURE_REASON_EXECUTION_EXCEPTION = 1005;
    int GBA_FAILURE_REASON_INTERRUPTED_EXCEPTION = 1006;

    @IntDef(prefix = {"GBA_FAILURE_REASON_"}, value = {
            TelephonyManager.GBA_FAILURE_REASON_UNKNOWN, // 0
            TelephonyManager.GBA_FAILURE_REASON_FEATURE_NOT_SUPPORTED, // 1
            TelephonyManager.GBA_FAILURE_REASON_FEATURE_NOT_READY, // 2
            TelephonyManager.GBA_FAILURE_REASON_NETWORK_FAILURE, // 3
            TelephonyManager.GBA_FAILURE_REASON_INCORRECT_NAF_ID, // 4
            TelephonyManager.GBA_FAILURE_REASON_SECURITY_PROTOCOL_NOT_SUPPORTED, // 5
            GBA_FAILURE_REASON_NONE,
            GBA_FAILURE_REASON_KEY_INVALID,
            GBA_FAILURE_REASON_TIMEOUT,
            GBA_FAILURE_REASON_TLS_CIPHERSUITE_NOT_SUPPORTED,
            GBA_FAILURE_REASON_CANCELLATION_EXCEPTION,
            GBA_FAILURE_REASON_EXECUTION_EXCEPTION,
            GBA_FAILURE_REASON_INTERRUPTED_EXCEPTION
    })
    @Retention(RetentionPolicy.SOURCE)
    public @interface AuthenticationFailureReason {}

    /**
     * This method triggers bootstrapping procedure to get B-TID and Ks_(ext)_NAF.
     *
     * @param appType icc application type.
     * @param gbaMode Gba mechanism that depends on NAF Key to be returned.
     * @param isTls If true, TLS protocol is used between UE and NAF, otherwise false.
     * @param nafFqdn A URI to specify Network Application Function(NAF) fully qualified domain
     * name (FQDN).
     * @param securityProtocol Security protocol identifier between UE and NAF.
     * @param forceBootStrapping If true, always triggers bootstrapping. If false, it will return
     * the previous bootstrapping result if valid.
     * @param timeoutSeconds The Timer to wait for the bootstrapping completion. On expiry, the
     * bootstrapping will be regarded as failure. If {@code config_gba_release_time} is not -1, the
     * {@code timeoutSeconds} value should be less than {@code config_gba_release_time}.
     *
     * @return GbaCredentials that includes the result of Gba operation, an error reason when
     * failed, B-TID and Ks_(ext)_NAF key.
     */
    GbaCredentials getGbaKey(int appType, int gbaMode, boolean isTls, String nafFqdn,
            String securityProtocol, boolean forceBootStrapping, int timeoutSeconds);
}
