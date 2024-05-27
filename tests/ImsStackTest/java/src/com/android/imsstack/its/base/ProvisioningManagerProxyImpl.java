/*
 * Copyright (C) 2024 The Android Open Source Project
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
package com.android.imsstack.its.base;

import android.annotation.CallbackExecutor;
import android.telephony.ims.ProvisioningManager;
import android.telephony.ims.ProvisioningManager.FeatureProvisioningCallback;
import android.telephony.ims.feature.MmTelFeature;
import android.telephony.ims.stub.ImsConfigImplBase;
import android.telephony.ims.stub.ImsRegistrationImplBase;
import android.util.ArraySet;
import android.util.SparseArray;

import androidx.annotation.NonNull;

import com.android.imsstack.base.SystemServiceProxy.ProvisioningManagerProxy;

import java.util.Objects;
import java.util.concurrent.Executor;

/**
 * An implementation class to access the {@link ProvisioningManager}.
 */
public class ProvisioningManagerProxyImpl implements ProvisioningManagerProxy {
    private final ArraySet<ProvisioningCallbackRecord>
            mProvisioningCallbackRecords = new ArraySet<>();
    private final SparseArray<SparseArray<Integer>> mProvisioningStatus = new SparseArray<>();

    @Override
    public void registerFeatureProvisioningChangedCallback(
            @NonNull @CallbackExecutor Executor executor,
            @NonNull FeatureProvisioningCallback callback) {
        Objects.requireNonNull(executor, "executor must not be null");
        Objects.requireNonNull(callback, "callback must not be null");

        ProvisioningCallbackRecord pcr = new ProvisioningCallbackRecord(executor, callback);
        mProvisioningCallbackRecords.add(pcr);
    }

    @Override
    public void unregisterFeatureProvisioningChangedCallback(
            @NonNull FeatureProvisioningCallback callback) {
        Objects.requireNonNull(callback, "callback must not be null");

        mProvisioningCallbackRecords.removeIf((r) -> r.hasCallback(callback));
    }

    @Override
    @ImsConfigImplBase.SetConfigResult
    public int setProvisioningIntValue(int key, int value) {
        // No operations.
        return ImsConfigImplBase.CONFIG_RESULT_SUCCESS;
    }

    @Override
    public boolean getProvisioningStatusForCapability(
            @MmTelFeature.MmTelCapabilities.MmTelCapability int capability,
            @ImsRegistrationImplBase.ImsRegistrationTech int tech) {
        SparseArray<Integer> statusByCapability = mProvisioningStatus.get(tech);
        if (statusByCapability == null) {
            return false;
        }

        int status = statusByCapability.get(capability,
                ProvisioningManager.PROVISIONING_RESULT_UNKNOWN);
        return (status == ProvisioningManager.PROVISIONING_VALUE_ENABLED);
    }

    /**
     * Sets the provisioning status of MMTEL capability for specific radio tech.
     *
     * @param capability The MMTEL capability that provisioning is set for.
     * @param tech The IMS registration technology associated with the MMTEL capability that
     *             provisioning status is set for.
     * @param isProvisioned {@code true} if the capability is provisioned for the technology,
     *                      {@code false} if the capability is not provisioned for the technology
     */
    public void setProvisioningStatus(
            @MmTelFeature.MmTelCapabilities.MmTelCapability int capability,
            @ImsRegistrationImplBase.ImsRegistrationTech int tech, boolean isProvisioned) {
        SparseArray<Integer> statusByCapability = mProvisioningStatus.get(tech);
        if (statusByCapability == null) {
            statusByCapability = new SparseArray<>();
            mProvisioningStatus.put(tech, statusByCapability);
        }

        int status = (isProvisioned) ? ProvisioningManager.PROVISIONING_VALUE_ENABLED :
                ProvisioningManager.PROVISIONING_VALUE_DISABLED;
        statusByCapability.put(capability, status);
    }

    /**
     * Notifies the application that the provisioning status has been changed.
     *
     * @param capability The MMTEL capability that provisioning status has been changed for.
     * @param tech The IMS registration technology associated with the MMTEL capability that
     *             provisioning status has been changed for.
     * @param isProvisioned {@code true} if the capability is provisioned for the technology,
     *                      {@code false} if the capability is not provisioned for the technology
     */
    public void notifyProvisioningStatusChanged(
            @MmTelFeature.MmTelCapabilities.MmTelCapability int capability,
            @ImsRegistrationImplBase.ImsRegistrationTech int tech, boolean isProvisioned) {
        mProvisioningCallbackRecords.forEach((r) -> {
            r.dispatchFeatureProvisioningStatusChanged(capability, tech, isProvisioned);
        });
    }

    private static final class ProvisioningCallbackRecord {
        private final Executor mScheduler;
        private final FeatureProvisioningCallback mCallback;

        ProvisioningCallbackRecord(@NonNull @CallbackExecutor Executor scheduler,
                @NonNull FeatureProvisioningCallback callback) {
            mScheduler = scheduler;
            mCallback = callback;
        }

        boolean hasCallback(FeatureProvisioningCallback callback) {
            return mCallback.equals(callback);
        }

        void dispatchFeatureProvisioningStatusChanged(
                @MmTelFeature.MmTelCapabilities.MmTelCapability int capability,
                @ImsRegistrationImplBase.ImsRegistrationTech int tech, boolean isProvisioned) {
            mScheduler.execute(() -> {
                mCallback.onFeatureProvisioningChanged(capability, tech, isProvisioned);
            });
        }
    }
}
