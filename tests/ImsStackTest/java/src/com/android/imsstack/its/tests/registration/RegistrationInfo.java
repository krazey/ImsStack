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
package com.android.imsstack.its.tests.registration;

import android.net.NetworkCapabilities;
import android.os.PersistableBundle;
import android.telephony.ServiceState;
import android.telephony.TelephonyManager;
import android.telephony.ims.feature.CapabilityChangeRequest;

import androidx.annotation.NonNull;
import androidx.annotation.Nullable;

import com.android.imsstack.its.base.TestConstants;

import java.util.Objects;

/**
 * Represents IMS registration information for IMS integration tests.
 * This class encapsulates the information required for IMS registration.
 */
public final class RegistrationInfo {
    private final int mSlotId;
    private final int mSimApplicationState;
    private final PersistableBundle mConfig;
    private final CapabilityChangeRequest mEnableCapabilityRequest;
    private final int mNetworkCapability;
    private final ServiceState mServiceState;

    /**
     * Constructs a new RegistrationInfo object with the specified parameters.
     *
     * @param slotId               The slot ID for the IMS registration.
     * @param config               The configuration for the IMS registration.
     * @param enableCapabilityRequest The request to enable capabilities.
     *                                {@link CapabilityChangeRequest}
     * @param networkCapability    The network capability for the IMS registration.
     * @param serviceState         The service state. {@link ServiceState}
     */
    public RegistrationInfo(int slotId, int simApplicationState, PersistableBundle config,
            CapabilityChangeRequest enableCapabilityRequest, int networkCapability,
            ServiceState serviceState) {
        mSlotId = slotId;
        mSimApplicationState = simApplicationState;
        mConfig = config;
        mEnableCapabilityRequest = enableCapabilityRequest;
        mNetworkCapability = networkCapability;
        mServiceState = serviceState;
    }

    /**
     * Returns the slot ID from the registration information.
     *
     * @return The slot ID.
     */
    public int getSlotId() {
        return mSlotId;
    }

    /**
     * Returns the SIM application state from the registration information.
     *
     * @return The SIM application state.
     */
    public int getSimApplicationState() {
        return mSimApplicationState;
    }

    /**
     * Returns the configuration from the registration information.
     *
     * @return The configuration, or {@code null} if no configuration is available.
     */
    @Nullable
    public PersistableBundle getConfig() {
        return mConfig;
    }

    /**
     * Returns the request to enable capabilities from the registration information.
     *
     * @return The request to enable capabilities, or
     *         {@code null} if no enable capabilities is available.
     */
    @Nullable
    public CapabilityChangeRequest getEnableCapabilityRequest() {
        return mEnableCapabilityRequest;
    }

    /**
     * Indicates whether enable capabilities has been changed.
     *
     * @return {@code true} if enable capabilities has been changed, {@code false} otherwise.
     */
    public boolean isEnableCapabilityRequestChanged() {
        return mEnableCapabilityRequest != null;
    }

    /**
     * Returns the network capability from the registration information.
     *
     * @return The network capability. Possible values are:
     *        {@link NetworkCapabilities#NET_CAPABILITY_IMS},
     *        {@link NetworkCapabilities#NET_CAPABILITY_XCAP},
     *        {@link NetworkCapabilities#NET_CAPABILITY_EIMS}.
     */
    public int getNetworkCapability() {
        return mNetworkCapability;
    }

    /**
     * Returns the service state.
     *
     * @return The service state, {@link ServiceState} or
     *         {@code null} if no service state is available.
     */
    @Nullable
    public ServiceState getServiceState() {
        return mServiceState;
    }

    /**
     * Indicates whether the service state has been changed.
     *
     * @return {@code true} if the service state has been changed, {@code false} otherwise.
     */
    public boolean isServiceStateChanged() {
        return mServiceState != null;
    }

    @Override
    public String toString() {
        StringBuilder sb = new StringBuilder();

        sb.append("[ RegistrationInfo :: SlotId=");
        sb.append(mSlotId);
        sb.append(", SimApplicationState=");
        sb.append(mSimApplicationState);
        sb.append(", Config=");
        sb.append((mConfig != null) ? mConfig.toString() : "null");
        sb.append(", EnableCapabilityRequest=");
        sb.append((mEnableCapabilityRequest != null)
                ? mEnableCapabilityRequest.toString() : "null");
        sb.append(", NetworkCapability=");
        sb.append(mNetworkCapability);
        sb.append(", ServiceState=");
        sb.append((mServiceState != null) ? mServiceState.toString() : "null");
        sb.append(" ]");

        return sb.toString();
    }

    /**
     * Builder class for creating the {@link RegistrationInfo}.
     */
    public static final class Builder {
        private int mSlotId = TestConstants.SLOT0;
        private int mSimApplicationState = TelephonyManager.SIM_STATE_LOADED;
        private PersistableBundle mConfig;
        private CapabilityChangeRequest mEnableCapabilityRequest;
        private int mNetworkCapability = NetworkCapabilities.NET_CAPABILITY_IMS;
        private ServiceState mServiceState;

        /**
         * Constructs a new {@code RegistrationInfo.Builder} with default values.
         */
        public Builder() {}

        /**
         * Sets the slot ID for the registration information.
         *
         * @param slotId The slot ID to set.
         * @return This {@code Builder} object to allow for chaining of method calls.
         */
        @NonNull
        public Builder setSlotId(int slotId) {
            mSlotId = slotId;
            return this;
        }

        /**
         *
         * Sets the SIM application state for the registration information.
         * @param simApplicationState The SIM application state to set.
         * @return This {@code Builder} object to allow for chaining of method calls.
         */
        @NonNull
        public Builder setSimApplicationState(int simApplicationState) {
            mSimApplicationState = simApplicationState;
            return this;
        }

        /**
         * Sets the configuration for the registration information.
         * This method replaces any existing configuration with the specified one.
         *
         * @param config The configuration to set. Must not be null.
         * @return This {@code Builder} object to allow for chaining of method calls.
         * @throws NullPointerException if the {@code config} parameter is null.
         */
        @NonNull
        public Builder setConfig(@NonNull PersistableBundle config) {
            Objects.requireNonNull(config, "config must not be null.");

            mConfig = config;
            return this;
        }

        /**
         * Adds the configuration to the existing configuration for the registration information.
         * If the current configuration {@code config} will be merged into the current configuration
         * {@code mConfig}, if it is not null.
         *
         * @param config The configuration to add. Must not be null.
         * @return This {@code Builder} object to allow for chaining of method calls.
         * @throws NullPointerException if the {@code config} parameter is null.
         */
        @NonNull
        public Builder addConfig(@NonNull PersistableBundle config) {
            Objects.requireNonNull(config, "config must not be null.");

            mConfig.putAll(config);
            return this;
        }

        /**
         * Sets the capability request for enabling the specified capability for multiple radio
         * technologies.
         *
         * @param capability  The MmTel capability to enable.
         * @param radioTechs  The list of supported radio technologies.
         * @return This {@code Builder} object to allow for chaining of method calls.
         */
        @NonNull
        public Builder setEnableCapabilityRequest(int capability, int... radioTechs) {
            if (mEnableCapabilityRequest == null) {
                mEnableCapabilityRequest = new CapabilityChangeRequest();
            }

            for (int radioTech : radioTechs) {
                mEnableCapabilityRequest.addCapabilitiesToEnableForTech(capability, radioTech);
            }
            return this;
        }

        /**
         * Sets the capability request for disabling the specified capability for multiple radio
         * technologies.
         *
         * @param capability  The MmTel capability to disable.
         * @param radioTechs  The list of supported radio technologies.
         * @return This {@code Builder} object to allow for chaining of method calls.
         */
        @NonNull
        public Builder setDisableCapabilityRequest(int capability, int... radioTechs) {
            if (mEnableCapabilityRequest == null) {
                mEnableCapabilityRequest = new CapabilityChangeRequest();
            }

            for (int radioTech : radioTechs) {
                mEnableCapabilityRequest.addCapabilitiesToDisableForTech(capability, radioTech);
            }
            return this;
        }

        /**
         * Sets the network capability for the registration information.
         *
         * @param networkCapability The network capability. Possible values are:
         *                   {@link NetworkCapabilities#NET_CAPABILITY_IMS},
         *                   {@link NetworkCapabilities#NET_CAPABILITY_XCAP},
         *                   {@link NetworkCapabilities#NET_CAPABILITY_EIMS}.
         * @return This {@code Builder} object to allow for chaining of method calls.
         */
        @NonNull
        public Builder setNetworkCapability(int networkCapability) {
            mNetworkCapability = networkCapability;
            return this;
        }

        /**
         * Sets the service state.
         *
         * @param serviceState The service state. {@link ServiceState}
         * @return This {@code Builder} object to allow for chaining of method calls.
         */
        @NonNull
        public Builder setServiceState(ServiceState serviceState) {
            mServiceState = serviceState;
            return this;
        }

        /**
         * Builds the registration information using the current state of this builder.
         *
         * @return A new {@code RegistrationInfo} object with the specified configuration.
         */
        public RegistrationInfo build() {
            return new RegistrationInfo(mSlotId, mSimApplicationState, mConfig,
                    mEnableCapabilityRequest, mNetworkCapability, mServiceState);
        }
    }
}
