/*
 * Copyright (C) 2023 The Android Open Source Project
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
package com.android.imsstack.base;

import android.annotation.CallbackExecutor;
import android.content.Context;
import android.net.Uri;
import android.os.OutcomeReceiver;
import android.telephony.AccessNetworkConstants;
import android.telephony.Annotation.NetworkType;
import android.telephony.Annotation.UiccAppType;
import android.telephony.Annotation.UiccAppTypeExt;
import android.telephony.CellInfo;
import android.telephony.NetworkRegistrationInfo;
import android.telephony.ServiceState;
import android.telephony.TelephonyCallback;
import android.telephony.TelephonyManager;
import android.telephony.TelephonyManager.AuthType;
import android.telephony.TelephonyManager.BootstrapAuthenticationCallback;
import android.telephony.TelephonyManager.CellInfoCallback;
import android.telephony.TelephonyManager.SimState;
import android.telephony.emergency.EmergencyNumber;
import android.telephony.gba.UaSecurityProtocolIdentifier;

import androidx.annotation.NonNull;
import androidx.annotation.Nullable;

import com.android.imsstack.util.MessageExecutor;

import java.util.ArrayList;
import java.util.Collections;
import java.util.List;
import java.util.Map;
import java.util.Set;
import java.util.concurrent.CompletableFuture;
import java.util.concurrent.Executor;
import java.util.concurrent.TimeUnit;

/**
 * An implementation class to access the APIs of {@link TelephonyManager}.
 */
public class TelephonyManagerProxyImpl implements TelephonyManagerProxy {
    private final Context mContext;
    private final TelephonyManager mTelephonyManager;

    TelephonyManagerProxyImpl(@NonNull Context context) {
        mContext = context;
        mTelephonyManager = mContext.getSystemService(TelephonyManager.class);
    }

    TelephonyManagerProxyImpl(@NonNull Context context, int subId) {
        mContext = context;
        TelephonyManager tm = mContext.getSystemService(TelephonyManager.class);
        if (tm != null) {
            mTelephonyManager = tm.createForSubscriptionId(subId);
        } else {
            mTelephonyManager = null;
            throw new IllegalStateException("TelephonyManager unavailable.");
        }
    }

    @Override
    public @NonNull TelephonyManagerProxy createForSubscriptionId(int subId) {
        return new TelephonyManagerProxyImpl(mContext, subId);
    }

    @Override
    public void registerTelephonyCallback(@NonNull @CallbackExecutor Executor executor,
            @NonNull TelephonyCallback callback) {
        TelephonyManager tm = getTelephonyManager();
        if (tm != null) {
            tm.registerTelephonyCallback(executor, callback);
        } else {
            throw new IllegalStateException("TelephonyManager unavailable.");
        }
    }

    @Override
    public void unregisterTelephonyCallback(@NonNull TelephonyCallback callback) {
        TelephonyManager tm = getTelephonyManager();
        if (tm != null) {
            tm.unregisterTelephonyCallback(callback);
        }
    }

    @Override
    public int getActiveModemCount() {
        TelephonyManager tm = getTelephonyManager();
        return tm != null ? tm.getActiveModemCount() : 1;
    }

    @Override
    public int getSupportedModemCount() {
        TelephonyManager tm = getTelephonyManager();
        return tm != null ? tm.getSupportedModemCount() : 1;
    }

    @Override
    public String getImei(int slotIndex) {
        TelephonyManager tm = getTelephonyManager();
        return tm != null ? tm.getImei(slotIndex) : null;
    }

    @Override
    public String getDeviceSoftwareVersion(int slotIndex) {
        TelephonyManager tm = getTelephonyManager();
        return tm != null ? tm.getDeviceSoftwareVersion(slotIndex) : null;
    }

    @Override
    public boolean isEmergencyNumber(@NonNull String number) {
        TelephonyManager tm = getTelephonyManager();
        return tm != null ? tm.isEmergencyNumber(number) : false;
    }

    @Override
    public boolean hasIccCard() {
        TelephonyManager tm = getTelephonyManager();
        return tm != null ? tm.hasIccCard() : false;
    }

    @Override
    public boolean isApplicationOnUicc(@UiccAppType int appType) {
        TelephonyManager tm = getTelephonyManager();
        return tm != null ? tm.isApplicationOnUicc(appType) : false;
    }

    @Override
    public @SimState int getSimApplicationState() {
        TelephonyManager tm = getTelephonyManager();
        return tm != null ? tm.getSimApplicationState() : TelephonyManager.SIM_STATE_UNKNOWN;
    }

    @Override
    public @SimState int getSimState(int slotIndex) {
        TelephonyManager tm = getTelephonyManager();
        return tm != null ? tm.getSimState(slotIndex) : TelephonyManager.SIM_STATE_UNKNOWN;
    }

    @Override
    public @SimState int getSimCardState() {
        TelephonyManager tm = getTelephonyManager();
        return tm != null ? tm.getSimCardState() : TelephonyManager.SIM_STATE_UNKNOWN;
    }

    @Override
    public int getSimCarrierId() {
        TelephonyManager tm = getTelephonyManager();
        return tm != null ? tm.getSimCarrierId() : TelephonyManager.UNKNOWN_CARRIER_ID;
    }

    @Override
    public @Nullable CharSequence getSimCarrierIdName() {
        TelephonyManager tm = getTelephonyManager();
        return tm != null ? tm.getSimCarrierIdName() : null;
    }

    @Override
    public int getSimSpecificCarrierId() {
        TelephonyManager tm = getTelephonyManager();
        return tm != null ? tm.getSimSpecificCarrierId() : TelephonyManager.UNKNOWN_CARRIER_ID;
    }

    @Override
    public int getCarrierIdFromSimMccMnc() {
        TelephonyManager tm = getTelephonyManager();
        return tm != null ? tm.getCarrierIdFromSimMccMnc() : TelephonyManager.UNKNOWN_CARRIER_ID;
    }

    @Override
    public String getSubscriberId() {
        TelephonyManager tm = getTelephonyManager();
        return tm != null ? tm.getSubscriberId() : null;
    }

    @Override
    public String getSimOperator() {
        TelephonyManager tm = getTelephonyManager();
        return tm != null ? tm.getSimOperator() : "";
    }

    @Override
    public String getSimCountryIso() {
        TelephonyManager tm = getTelephonyManager();
        return tm != null ? tm.getSimCountryIso() : "";
    }

    @Override
    public String getSimSerialNumber() {
        TelephonyManager tm = getTelephonyManager();
        return tm != null ? tm.getSimSerialNumber() : null;
    }

    @Override
    public String getGroupIdLevel1() {
        TelephonyManager tm = getTelephonyManager();
        return tm != null ? tm.getGroupIdLevel1() : null;
    }

    @Override
    public String getSimOperatorName() {
        TelephonyManager tm = getTelephonyManager();
        return tm != null ? tm.getSimOperatorName() : "";
    }

    @Override
    public @NonNull byte[] getSimServiceTable(@UiccAppType int appType) {
        TelephonyManager tm = getTelephonyManager();
        if (tm == null) {
            return new byte[0];
        }
        CompletableFuture<byte[]> future = new CompletableFuture<>();
        OutcomeReceiver<byte[], Exception> callback = new OutcomeReceiver<>() {
            @Override
            public void onResult(byte[] serviceTable) {
                future.complete(serviceTable);
            }
            @Override
            public void onError(@NonNull Exception ex) {
                future.complete(new byte[0]);
            }
        };

        tm.getSimServiceTable(appType, ExecutorHolder.sExecutor, callback);

        try {
            return future.get(1, TimeUnit.SECONDS);
        } catch (Exception e) {
            return new byte[0];
        }
    }

    @Override
    public String getIsimDomain() {
        TelephonyManager tm = getTelephonyManager();
        return tm != null ? tm.getIsimDomain() : null;
    }

    @Override
    public String getImsPrivateUserIdentity() {
        TelephonyManager tm = getTelephonyManager();
        if (tm != null) {
            return tm.getImsPrivateUserIdentity();
        } else {
            throw new IllegalStateException("TelephonyManager unavailable.");
        }
    }

    @Override
    @SuppressWarnings("unchecked")
    public List<Uri> getImsPublicUserIdentities() {
        TelephonyManager tm = getTelephonyManager();
        return tm != null ? tm.getImsPublicUserIdentities() : Collections.EMPTY_LIST;
    }

    @Override
    public List<String> getImsPcscfAddresses() {
        TelephonyManager tm = getTelephonyManager();
        return tm != null ? tm.getImsPcscfAddresses() : Collections.emptyList();
    }

    @Override
    public Set<String> requestUiccIari() {
        TelephonyManager tm = getTelephonyManager();
        if (tm == null) {
            return Collections.emptySet();
        }
        CompletableFuture<Set<String>> future = new CompletableFuture<>();
        OutcomeReceiver<Set<String>, Exception> callback = new OutcomeReceiver<>() {
            @Override
            public void onResult(Set<String> iariSet) {
                future.complete(iariSet);
            }
            @Override
            public void onError(@NonNull Exception ex) {
                future.complete(Collections.emptySet());
            }
        };

        tm.requestUiccIari(ExecutorHolder.sExecutor, callback);

        try {
            return future.get(1, TimeUnit.SECONDS);
        } catch (Exception e) {
            return Collections.emptySet();
        }
    }

    @Override
    public String getIccAuthentication(@UiccAppType int appType, @AuthType int authType,
            String data) {
        TelephonyManager tm = getTelephonyManager();
        return tm != null ? tm.getIccAuthentication(appType, authType, data) : null;
    }

    @Override
    public String sendEnvelopeWithStatus(String content) {
        TelephonyManager tm = getTelephonyManager();
        return tm != null ? tm.sendEnvelopeWithStatus(content) : "";
    }

    @Override
    public boolean isDataEnabled() {
        TelephonyManager tm = getTelephonyManager();
        return tm != null ? tm.isDataEnabled() : false;
    }

    @Override
    public boolean isDataRoamingEnabled() {
        TelephonyManager tm = getTelephonyManager();
        return tm != null ? tm.isDataRoamingEnabled() : false;
    }

    @Override
    public @Nullable ServiceState getServiceState(int slotIndex) {
        TelephonyManager tm = getTelephonyManager();
        // NOTE: For IMS emergency call when there is no SIM, a hidden API is used at this time.
        return tm != null ? tm.getServiceStateForSlot(slotIndex) : null;
    }

    @Override
    public @NetworkType int getDataNetworkType(int slotIndex) {
        ServiceState ss = getServiceState(slotIndex);
        return ss != null ? getDataNetworkType(ss) : TelephonyManager.NETWORK_TYPE_UNKNOWN;
    }

    @Override
    public @NetworkType int getVoiceNetworkType(int slotIndex) {
        ServiceState ss = getServiceState(slotIndex);
        return ss != null ? getVoiceNetworkType(ss) : TelephonyManager.NETWORK_TYPE_UNKNOWN;
    }

    @Override
    public String getNetworkOperator() {
        TelephonyManager tm = getTelephonyManager();
        return tm != null ? tm.getNetworkOperator() : "";
    }

    @Override
    public String getNetworkCountryIso() {
        TelephonyManager tm = getTelephonyManager();
        return tm != null ? tm.getNetworkCountryIso() : "";
    }

    @Override
    public List<CellInfo> getAllCellInfo() {
        TelephonyManager tm = getTelephonyManager();
        return tm != null ? tm.getAllCellInfo() : null;
    }

    @Override
    public void requestCellInfoUpdate(@NonNull @CallbackExecutor Executor executor,
            @NonNull CellInfoCallback callback) {
        TelephonyManager tm = getTelephonyManager();
        if (tm != null) {
            tm.requestCellInfoUpdate(executor, callback);
        } else {
            executor.execute(() -> {
                callback.onCellInfo(new ArrayList<CellInfo>());
            });
        }
    }

    @Override
    public void bootstrapAuthenticationRequest(@UiccAppTypeExt int appType, @NonNull Uri nafId,
            @NonNull UaSecurityProtocolIdentifier securityProtocol, boolean forceBootStrapping,
            @NonNull Executor executor, @NonNull BootstrapAuthenticationCallback callback) {
        TelephonyManager tm = getTelephonyManager();
        if (tm != null) {
            tm.bootstrapAuthenticationRequest(
                    appType, nafId, securityProtocol, forceBootStrapping, executor, callback);
        } else {
            executor.execute(() -> {
                callback.onAuthenticationFailure(
                        TelephonyManager.GBA_FAILURE_REASON_FEATURE_NOT_READY);
            });
        }
    }

    @Override
    public @NonNull Map<Integer, List<EmergencyNumber>> getEmergencyNumberList() {
        TelephonyManager tm = getTelephonyManager();
        try {
            return (tm != null) ? tm.getEmergencyNumberList() : Collections.emptyMap();
        } catch (UnsupportedOperationException | IllegalStateException e) {
            return Collections.emptyMap();
        }
    }

    private TelephonyManager getTelephonyManager() {
        return mTelephonyManager != null
                ? mTelephonyManager
                : mContext.getSystemService(TelephonyManager.class);
    }

    /**
     * Returns the data network type from the given {@link ServiceState} object.
     *
     * @param ss The {@link ServiceState} object.
     * @return The data network type.
     */
    private static int getDataNetworkType(ServiceState ss) {
        final NetworkRegistrationInfo nri = ss.getNetworkRegistrationInfo(
                NetworkRegistrationInfo.DOMAIN_PS, AccessNetworkConstants.TRANSPORT_TYPE_WWAN);
        return nri != null
                ? nri.getAccessNetworkTechnology()
                : TelephonyManager.NETWORK_TYPE_UNKNOWN;
    }

    /**
     * Returns the voice network type from the given {@link ServiceState} object.
     *
     * @param ss The {@link ServiceState} object.
     * @return The voice network type.
     */
    private static int getVoiceNetworkType(ServiceState ss) {
        final NetworkRegistrationInfo nri = ss.getNetworkRegistrationInfo(
                NetworkRegistrationInfo.DOMAIN_CS, AccessNetworkConstants.TRANSPORT_TYPE_WWAN);
        return nri != null
                ? nri.getAccessNetworkTechnology()
                : TelephonyManager.NETWORK_TYPE_UNKNOWN;
    }

    /*
     * An executor holder class to defer initialization until needed.
     *
     * Currently, this executor is used to communicate with the Phone process and read the SIM's
     * service table, so communicating with the Phone process with a single instance is sufficient.
     */
    private static class ExecutorHolder {
        static final MessageExecutor sExecutor =
                new MessageExecutor(TelephonyManagerProxyImpl.class.getSimpleName());
    }
}
