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

import com.android.imsstack.base.AppContext;
import com.android.imsstack.base.MSimUtils;
import com.android.imsstack.base.TelephonyManagerProxy;
import com.android.imsstack.util.ImsLog;

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
    private ExecutorService mExecutorService = null;

    public GbaAgent(int slotId) {
        mSlotId = slotId;
    }

    @Override
    public void init(Context context) {
        ImsLog.d(this, mSlotId, "init");
    }

    @Override
    public void cleanup() {
        ImsLog.d(this, mSlotId, "clean up");

        if (mExecutorService != null) {
            mExecutorService.shutdown();
            mExecutorService = null;
        }
    }

    @Override
    public GbaCredentials getGbaKey(int appType, int gbaMode, boolean isTls, String nafFqdn,
            String securityProtocol, boolean forceBootStrapping) {
        ImsLog.d(this, mSlotId, "appType: " + appType + ", gbaMode: " + gbaMode
                + ", isTls: " + isTls + ", nafFqdn: " + nafFqdn + ", protocol: " + securityProtocol
                + ", forceBootStrapping: " + forceBootStrapping);

        Uri nafUri = getNafUri(gbaMode, isTls, nafFqdn);
        GbaCredentials credentials = null;
        try {
            credentials = requestTelephonyGbaAuthentication(appType, nafUri, securityProtocol,
                    forceBootStrapping).get(30L, TimeUnit.SECONDS);
        } catch (CancellationException e) {
            ImsLog.e(this, mSlotId, "CancellationException");
            credentials = new GbaCredentials(GBA_FAILURE_REASON_CANCELLATION_EXCEPTION);
        } catch (ExecutionException e) {
            ImsLog.e(this, mSlotId, "ExecutionException");
            credentials = new GbaCredentials(GBA_FAILURE_REASON_EXECUTION_EXCEPTION);
        } catch (InterruptedException e) {
            ImsLog.e(this, mSlotId, "InterruptedException");
            credentials = new GbaCredentials(GBA_FAILURE_REASON_INTERRUPTED_EXCEPTION);
        } catch (TimeoutException e) {
            ImsLog.e(this, mSlotId, "TimeoutException");
            credentials = new GbaCredentials(GBA_FAILURE_REASON_TIMEOUT);
        } catch (IllegalArgumentException e) {
            ImsLog.e(this, mSlotId, "IllegalArgumentException: securityProtocol is not supported");
            credentials = new GbaCredentials(GBA_FAILURE_REASON_TLS_CIPHERSUITE_NOT_SUPPORTED);
        }

        return credentials;
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
        int subId = MSimUtils.getSubId(mSlotId);
        TelephonyManagerProxy tmp = AppContext.getTelephonyManagerProxy(subId);
        tmp.bootstrapAuthenticationRequest(appType, nafUri, uspi.build(), forceBootStrapping,
                getExecutor(),
                new TelephonyManager.BootstrapAuthenticationCallback() {
                    @Override
                    public void onKeysAvailable(byte[] gbaKey, String transactionId) {
                        if (gbaKey == null || TextUtils.isEmpty(transactionId)) {
                            ImsLog.e(this, mSlotId, "onKeysAvailable with wrong value");
                            credentialsFuture.complete(
                                    new GbaCredentials(GBA_FAILURE_REASON_KEY_INVALID));
                        } else {
                            String key = Base64.encodeToString(gbaKey, Base64.NO_WRAP);
                            credentialsFuture.complete(new GbaCredentials(transactionId, key));
                        }
                    }

                    @Override
                    public void onAuthenticationFailure(int reason) {
                        ImsLog.e(this, mSlotId, "onAuthenticationFailure: " + reason);
                        credentialsFuture.complete(new GbaCredentials(reason));
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
}
