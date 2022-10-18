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

import android.content.Context;
import android.net.Uri;
import android.telephony.CarrierConfigManager;
import android.telephony.TelephonyManager;
import android.telephony.gba.UaSecurityProtocolIdentifier;
import android.text.TextUtils;
import android.util.Base64;
import android.util.Pair;

import com.android.imsstack.util.AppContext;
import com.android.imsstack.util.ImsLog;
import com.android.imsstack.util.MSimUtils;

import java.util.concurrent.CancellationException;
import java.util.concurrent.CompletableFuture;
import java.util.concurrent.ExecutionException;
import java.util.concurrent.ExecutorService;
import java.util.concurrent.Executors;
import java.util.concurrent.TimeUnit;
import java.util.concurrent.TimeoutException;

/**
 * Provides the APIs for getting key pair of bootstrapped security association.
 */
public class GbaAgent implements GbaInterface {
    private final int mSlotId;
    private Context mContext;
    private ExecutorService mExecutorService = null;

    public GbaAgent(int slotId) {
        mSlotId = slotId;
    }

    @Override
    public void init(Context context) {
        ImsLog.d(mSlotId, "init");

        mContext = context;
    }

    @Override
    public void cleanup() {
        ImsLog.d(mSlotId, "clean up");

        if (mExecutorService != null) {
            mExecutorService.shutdown();
            mExecutorService = null;
        }
    }

    @Override
    public Pair<String, String> getGbaKey(int appType, int gbaMode, boolean isTls, String nafFqdn,
            String securityProtocol, boolean forceBootStrapping) {
        ImsLog.d(mSlotId, "appType : " + appType + ",gbaMode : " + gbaMode + ", isTls : " + isTls
                + ", nafFqdn : " + nafFqdn + ", Protocol : " + securityProtocol
                + ", forceBootStrapping : " + forceBootStrapping);

        Uri nafUri = getNafUri(gbaMode, isTls, nafFqdn);
        GbaCredentials credentials = null;
        try {
            credentials = requestTelephonyGbaAuthentication(appType, nafUri, securityProtocol,
                    forceBootStrapping).get(30L, TimeUnit.SECONDS);
        } catch (CancellationException e) {
            ImsLog.e(mSlotId, "CancellationException");
        } catch (ExecutionException e) {
            ImsLog.e(mSlotId, "ExecutionException");
        } catch (InterruptedException e) {
            ImsLog.e(mSlotId, "InterruptedException");
        } catch (TimeoutException e) {
            ImsLog.e(mSlotId, "TimeoutException");
        } catch (IllegalArgumentException e) {
            ImsLog.e(mSlotId, "IllegalArgumentException : securityProtocol is not supported");
        }

        if (credentials == null) {
            return null;
        }

        return Pair.create(credentials.getTransactionId(), credentials.getKey());
    }

    private CompletableFuture<GbaCredentials> requestTelephonyGbaAuthentication(int appType,
            Uri nafUri, String securityProtocol, boolean forceBootStrapping) {
        int protocol =
                UaSecurityProtocolIdentifier.UA_SECURITY_PROTOCOL_3GPP_HTTP_DIGEST_AUTHENTICATION;
        if (!TextUtils.isEmpty(securityProtocol)) {
            protocol = UaSecurityProtocolIdentifier.UA_SECURITY_PROTOCOL_3GPP_TLS_DEFAULT;
        }

        UaSecurityProtocolIdentifier.Builder uspi = new UaSecurityProtocolIdentifier.Builder()
                .setOrg(UaSecurityProtocolIdentifier.ORG_3GPP).setProtocol(protocol);

        if (!TextUtils.isEmpty(securityProtocol)) {
            int tlsCipherSuite = UaCipherSuite.getInstance().getCipherSuiteValue(securityProtocol);
            uspi.setTlsCipherSuite(tlsCipherSuite);
        }

        CompletableFuture<GbaCredentials> credentialsFuture = new CompletableFuture<>();

        TelephonyManager tm = getTelephonyManager();
        tm.bootstrapAuthenticationRequest(appType, nafUri, uspi.build(), forceBootStrapping,
                getExecutor(),
                new TelephonyManager.BootstrapAuthenticationCallback() {
                    @Override
                    public void onKeysAvailable(byte[] gbaKey, String transactionId) {
                        if (gbaKey == null || TextUtils.isEmpty(transactionId)) {
                            ImsLog.e(mSlotId, "onKeysAvailable with wrong value");
                            credentialsFuture.complete(null);
                        } else {
                            String key = Base64.encodeToString(gbaKey, Base64.NO_WRAP);
                            GbaCredentials credentials = new GbaCredentials(transactionId, key);
                            credentialsFuture.complete(credentials);
                        }
                    }

                    @Override
                    public void onAuthenticationFailure(int reason) {
                        ImsLog.e(mSlotId, "reason : " + reason);
                        credentialsFuture.complete(null);
                    }
                });

        return credentialsFuture;
    }

    private ExecutorService getExecutor() {
        if (mExecutorService == null) {
            mExecutorService = Executors.newSingleThreadExecutor();
        }

        return mExecutorService;
    }

    private Uri getNafUri(int gbaMode, boolean isTls, String nafFqdn) {
        String nafPrefix;
        switch (gbaMode) {
            case CarrierConfigManager.GBA_U: // 2
                nafPrefix = "3GPP-bootstrapping-uicc";
                break;
            case CarrierConfigManager.GBA_DIGEST: // 3
                nafPrefix = "3GPP-bootstrapping-digest";
                break;
            case CarrierConfigManager.GBA_ME: // 1, FALL-THROUGH
            default:
                nafPrefix = "3GPP-bootstrapping";
                break;
        }

        String scheme = isTls ? "https" : "http";
        String authority = nafPrefix + "@" + nafFqdn;
        return new Uri.Builder().scheme(scheme).encodedAuthority(authority).build();
    }

    private TelephonyManager getTelephonyManager() {
        int subId = MSimUtils.getSubId(mSlotId);
        return AppContext.getTelephonyManager(subId);
    }

    private class GbaCredentials {
        private final String mTransactionId;
        private final String mKey;

        private GbaCredentials(String transactionId, String key) {
            this.mTransactionId = transactionId;
            this.mKey = key;
        }

        private String getTransactionId() {
            return mTransactionId;
        }

        private String getKey() {
            return mKey;
        }
    }
}
