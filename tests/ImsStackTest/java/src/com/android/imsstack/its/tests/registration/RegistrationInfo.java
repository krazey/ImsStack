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
import android.telephony.CarrierConfigManager;
import android.telephony.ServiceState;
import android.telephony.TelephonyManager;
import android.telephony.ims.feature.CapabilityChangeRequest;
import android.telephony.ims.stub.ImsRegistrationImplBase;

import androidx.annotation.NonNull;
import androidx.annotation.Nullable;

import com.android.imsstack.its.base.TestConstants;

import java.util.LinkedHashMap;
import java.util.Map;
import java.util.Objects;
import java.util.function.BiConsumer;

/**
 * Represents IMS registration information for IMS integration tests.
 * This class encapsulates the information required for IMS registration.
 */
public final class RegistrationInfo {
    private final int mSlotId;
    private final PersistableBundle mConfig;
    private final int mSimCarrierId;
    private final SimSupportMode mSimSupportMode;
    private final int mSimApplicationState;

    private final CapabilityChangeRequest mCapabilityRequest;
    private final int mNetworkCapability;
    private final ServiceState mServiceState;
    private final int mExpectedRegTech;

    /**
     * Defines the available modes for SIM card support.
     * Each mode indicates the level of support for ISIM and USIM applications.
     *     {@code BOTH_ISIM_USIM}: Both ISIM and USIM are fully supported.
     *     {@code ONLY_USIM}: Only USIM is supported, ISIM is not.
     *     {@code INCOMP_ISIM_USIM}: ISIM is incompletely supported, USIM is fully supported.
     *     {@code NEITHER_ISIM_USIM}: Neither ISIM nor USIM is supported.
     */
    public enum SimSupportMode {
        BOTH_ISIM_USIM,
        INCOMP_ISIM_USIM,
        ONLY_USIM,
        NEITHER_ISIM_USIM;

        @Override
        public String toString() {
            return switch (this) {
                case BOTH_ISIM_USIM -> "BOTH_ISIM_USIM";
                case INCOMP_ISIM_USIM -> "INCOMP_ISIM_USIM";
                case ONLY_USIM -> "ONLY_USIM";
                case NEITHER_ISIM_USIM -> "NEITHER_ISIM_USIM";
            };
        }
    }
    /**
     * Constructs a new RegistrationInfo object with the specified parameters.
     *
     * @param slotId            The slot ID for the IMS registration.
     * @param config            The configuration for the IMS registration.
     * @param simCarrierId      the SIM carrier-id from the IMS registration.
     * @param simSupportMode    The mode indicating the level of support for SIM applications.
     *                          {@link SimSupportMode}
     * @param simApplicationState The SIM application state.
     * @param capabilityRequest The request to change capabilities. {@link CapabilityChangeRequest}
     * @param networkCapability The network capability for the IMS registration.
     * @param serviceState      The service state. {@link ServiceState}
     * @param expectedRegTech   The expected registration technology, such as
     *                          {@link ImsRegistrationImplBase#REGISTRATION_TECH_LTE}.
     */
    public RegistrationInfo(int slotId, PersistableBundle config, int simCarrierId,
            SimSupportMode simSupportMode, int simApplicationState,
            CapabilityChangeRequest capabilityRequest, int networkCapability,
            ServiceState serviceState, int expectedRegTech) {
        mSlotId = slotId;
        mConfig = config;
        mSimCarrierId = simCarrierId;
        mSimSupportMode = simSupportMode;
        mSimApplicationState = simApplicationState;
        mCapabilityRequest = capabilityRequest;
        mNetworkCapability = networkCapability;
        mServiceState = serviceState;
        mExpectedRegTech = expectedRegTech;
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
     * Returns the configuration from the registration information.
     *
     * @return The configuration, or {@code null} if no configuration is available.
     */
    @Nullable
    public PersistableBundle getConfig() {
        return mConfig;
    }

    /**
     * Returns the SIM carrier-id from the registration information.
     *
     * @return The SIM carrier-id.
     */
    public int getSimCarrierId() {
        return mSimCarrierId;
    }

    /**
     * Returns the SIM support mode for the registration information.
     *
     * @return The {@link SimSupportMode} indicating the level of support for SIM applications.
     */
    public SimSupportMode getSimSupportMode() {
        return mSimSupportMode;
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
     * Returns the request to change capabilities from the registration information.
     *
     * @return The request to change capabilities, or
     *         {@code null} if no capabilities change request is available.
     */
    @Nullable
    public CapabilityChangeRequest getCapabilityRequest() {
        return mCapabilityRequest;
    }

    /**
     * Indicates whether the capabilities change request has been changed.
     *
     * @return {@code true} if the capabilities change request has been changed,
     *         {@code false} otherwise.
     */
    public boolean isCapabilityRequestChanged() {
        return mCapabilityRequest != null;
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
     * Returns the expected registration technology.
     *
     * @return The expected registration technology. Possible values are:
     * {@link ImsRegistrationImplBase#REGISTRATION_TECH_LTE},
     * {@link ImsRegistrationImplBase#REGISTRATION_TECH_NR},
     * {@link ImsRegistrationImplBase#REGISTRATION_TECH_IWLAN}.
     */
    public int getExpectedRegTech() {
        return mExpectedRegTech;
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
        sb.append(", Config=");
        sb.append((mConfig != null) ? mConfig.toString() : "null");
        sb.append(", SimCarrierId=");
        sb.append(mSimCarrierId);
        sb.append(", SimSupportMode=");
        sb.append(mSimSupportMode.toString());
        sb.append(", SimApplicationState=");
        sb.append(mSimApplicationState);
        sb.append(", EnableCapabilityRequest=");
        sb.append((mCapabilityRequest != null) ? mCapabilityRequest.toString() : "null");
        sb.append(", NetworkCapability=");
        sb.append(mNetworkCapability);
        sb.append(", ServiceState=");
        sb.append((mServiceState != null) ? mServiceState.toString() : "null");
        sb.append(", RegistrationTech=");
        sb.append(mExpectedRegTech);
        sb.append(" ]");

        return sb.toString();
    }

    /**
     * Builder class for creating the {@link RegistrationInfo}.
     */
    public static final class Builder {
        private int mSlotId = TestConstants.SLOT0;
        private PersistableBundle mConfig;
        private int mSimCarrierId = -1;
        private SimSupportMode mSimSupportMode = SimSupportMode.BOTH_ISIM_USIM;
        private int mSimApplicationState = TelephonyManager.SIM_STATE_LOADED;
        private int mNetworkCapability = NetworkCapabilities.NET_CAPABILITY_IMS;
        private ServiceState mServiceState;
        private int mExpectedRegTech = ImsRegistrationImplBase.REGISTRATION_TECH_LTE;

        private final CapabilityPairs mEnablePairs = new CapabilityPairs();
        private final CapabilityPairs mDisablePairs = new CapabilityPairs();

        /**
         * Constructs a new {@code RegistrationInfo.Builder} with an empty configuration.
         */
        public Builder() {
            mConfig = new PersistableBundle();
        }

        /**
         * Adds the default IMS configuration values to this builder.
         * This includes settings for IPsec and the preferred SIP transport type.
         *
         * @return This {@code Builder} object to allow for chaining of method calls.
         */
        @NonNull
        public Builder withDefaultConfig() {
            mConfig.putBoolean(CarrierConfigManager.Ims.KEY_SIP_OVER_IPSEC_ENABLED_BOOL, false);
            mConfig.putInt(CarrierConfigManager.Ims.KEY_SIP_PREFERRED_TRANSPORT_INT,
                    CarrierConfigManager.Ims.PREFERRED_TRANSPORT_TCP);
            return this;
        }

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
         * Sets the SIM carrier-id for the registration information.
         *
         * @param simCarrierId The SIM carrier-id to set.
         * @return This {@code Builder} object to allow for chaining of method calls.
         */
        @NonNull
        public Builder setSimCarrierId(int simCarrierId) {
            mSimCarrierId = simCarrierId;
            return this;
        }

        /**
         * Sets the SIM support mode for the registration information.
         *
         * @param simSupportMode The SIM support mode to set.
         * @return This {@code Builder} object to allow for chaining of method calls.
         */
        @NonNull
        public Builder setSimSupportMode(SimSupportMode simSupportMode) {
            mSimSupportMode = simSupportMode;
            return this;
        }

        /**
         * Sets the SIM application state for the registration information.
         * @param simApplicationState The SIM application state to set. Possible values are:
         *                            {@link TelephonyManager#SIM_STATE_UNKNOWN},
         *                            {@link TelephonyManager#SIM_STATE_PIN_REQUIRED},
         *                            {@link TelephonyManager#SIM_STATE_PUK_REQUIRED},
         *                            {@link TelephonyManager#SIM_STATE_NETWORK_LOCKED},
         *                            {@link TelephonyManager#SIM_STATE_NOT_READY},
         *                            {@link TelephonyManager#SIM_STATE_PERM_DISABLED},
         *                            {@link TelephonyManager#SIM_STATE_LOADED}
         * @return This {@code Builder} object to allow for chaining of method calls.
         */
        @NonNull
        public Builder setSimApplicationState(int simApplicationState) {
            mSimApplicationState = simApplicationState;
            return this;
        }

        /**
         * Sets the capability for enabling the specified capability for multiple radio
         * technologies.
         * <p>
         * NOTE:
         * 1. MmTel capabilities are managed exclusively between enabling capability and disabling
         *    capability. If the same capability exists in both enabling capabilities and disabling
         *    capabilities, the disabling capability will be removed.
         * 2. MmTel capabilities are not automatically reset when the ImsStackTest ends.
         *    If necessary, perform initialization in the tearDown method of the test class.
         *
         * @param capability  The MmTel capability to enable.
         * @param radioTechs  The list of supported radio technologies.
         * @return This {@code Builder} object to allow for chaining of method calls.
         */
        @NonNull
        public Builder setEnableCapability(int capability, int... radioTechs) {

            for (int radioTech : radioTechs) {
                mEnablePairs.updateCapability(radioTech, capability, true);

                if (!mDisablePairs.getPairs().isEmpty()) {
                    mDisablePairs.updateCapability(radioTech, capability, false);
                }
            }

            return this;
        }

        /**
         * Sets the capability for disabling the specified capability for multiple radio
         * technologies.
         * <p>
         * NOTE:
         * 1. MmTel capabilities are managed exclusively between enabling capability and disabling
         *    capability. If the same capability exists in both enabling capabilities and disabling
         *    capabilities, the enabling capability will be removed.
         * 2. MmTel capabilities are not automatically reset when the ImsStackTest ends.
         *    If necessary, perform initialization in the tearDown method of the test class.
         *
         * @param capability  The MmTel capability to disable.
         * @param radioTechs  The list of supported radio technologies.
         * @return This {@code Builder} object to allow for chaining of method calls.
         */
        @NonNull
        public Builder setDisableCapability(int capability, int... radioTechs) {

            for (int radioTech : radioTechs) {
                mDisablePairs.updateCapability(radioTech, capability, true);

                if (!mEnablePairs.getPairs().isEmpty()) {
                    mEnablePairs.updateCapability(radioTech, capability, false);
                }
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
         * Sets the expected registration technology for the registration information.
         *
         * @param expectedRegTech The expected registration technology. Possible values are:
         *        {@link ImsRegistrationImplBase#REGISTRATION_TECH_LTE},
         *        {@link ImsRegistrationImplBase#REGISTRATION_TECH_NR},
         *        {@link ImsRegistrationImplBase#REGISTRATION_TECH_IWLAN}.
         * @return This {@code Builder} object to allow for chaining of method calls.
         */
        @NonNull
        public Builder setExpectedRegTech(int expectedRegTech) {
            mExpectedRegTech = expectedRegTech;
            return this;
        }

        /**
         * Builds the registration information using the current state of this builder.
         *
         * @return A new {@code RegistrationInfo} object with the specified configuration.
         */
        public RegistrationInfo build() {
            return new RegistrationInfo(mSlotId, mConfig, mSimCarrierId, mSimSupportMode,
                    mSimApplicationState, createCapabilityRequest(), mNetworkCapability,
                    mServiceState, mExpectedRegTech);
        }


        private CapabilityChangeRequest createCapabilityRequest() {
            if (!mEnablePairs.getPairs().isEmpty() || !mDisablePairs.getPairs().isEmpty()) {
                CapabilityChangeRequest request = new CapabilityChangeRequest();

                addCapabilities(mEnablePairs, request::addCapabilitiesToEnableForTech);
                addCapabilities(mDisablePairs, request::addCapabilitiesToDisableForTech);

                return request;
            }

            return null;
        }

        private void addCapabilities(CapabilityPairs pairs, BiConsumer<Integer, Integer> consumer) {
            for (Map.Entry<Integer, Integer> entry : pairs.getPairs().entrySet()) {
                consumer.accept(entry.getValue(), entry.getKey());
            }
        }
    }

    /**
     * Represents a collection of capability pairs for various radio technologies.
     * <p>
     * The key of {@code Map<Integer, Integer>} represents the radio technology,
     * which is defined by {@code ImsRegistrationImplBase.ImsRegistrationTech}.
     * </p>
     * <p>
     * The value of {@code Map<Integer, Integer>} represents the capabilities
     * associated with the corresponding radio technology. The capabilities used
     * in MmTelFeature are defined as:
     * {@code MmTelFeature.MmTelCapabilities#CAPABILITY_TYPE_VOICE},
     * {@code MmTelFeature.MmTelCapabilities#CAPABILITY_TYPE_VIDEO},
     * {@code MmTelFeature.MmTelCapabilities#CAPABILITY_TYPE_UT},
     * {@code MmTelFeature.MmTelCapabilities#CAPABILITY_TYPE_SMS}, and
     * {@code MmTelFeature.MmTelCapabilities#CAPABILITY_TYPE_CALL_COMPOSER}.
     * </p>
     */
    private static final class CapabilityPairs {

        private final Map<Integer, Integer> mPairs = new LinkedHashMap<Integer, Integer>();

        /**
         * Returns the capability pairs stored in this object.
         *
         * @return The capability pairs stored in this object.
         */
        public Map<Integer, Integer> getPairs() {
            return mPairs;
        }

        /**
         * Adds or removes the specified capability for the given radio technology.
         * If the capability is being added, it will be bitwise OR-ed with the existing capability.
         * If the capability is being removed, it will be bitwise AND-ed with the complement of
         * the existing capability.
         * If the result is 0, the capability for the given radio technology will be removed from
         * the map.
         *
         * @param radioTech   The radio technology.
         * @param capability  The capability to add or remove.
         * @param add         {@code true} to add the capability, {@code false} to remove it.
         */
        public void updateCapability(Integer radioTech, Integer capability, boolean add) {
            if (add) {
                mPairs.put(radioTech, mPairs.getOrDefault(radioTech, 0) | capability);
            } else {
                mPairs.computeIfPresent(radioTech, (key, existingCapability) -> {
                    int updatedCapability = existingCapability & ~capability;
                    return (updatedCapability > 0) ? updatedCapability : null;
                });
            }
        }
    }
}
