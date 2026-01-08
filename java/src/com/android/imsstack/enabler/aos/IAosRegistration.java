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
package com.android.imsstack.enabler.aos;

import android.annotation.IntRange;
import android.annotation.Nullable;

import androidx.annotation.NonNull;

import com.android.imsstack.enabler.aos.IAosRegistrationListener.NetworkType;
import com.android.imsstack.enabler.aos.IAosRegistrationListener.RegistrationState;

import java.util.Arrays;
import java.util.LinkedHashMap;
import java.util.Map;
import java.util.Objects;
import java.util.stream.Collectors;

/**
 * This class provides the interworking interface between Java and native layer
 * for AoS(Always On Service) functionalities.
 * It manages the IMS registration state, and sends any information to native layer
 * to run AoS enabler.
 */
public interface IAosRegistration {

    /**
     * Registers a new Listener to receive registration updates.
     *
     * @param listener The non-null listener to be registered.
     * @throws NullPointerException if the listener is null.
     */
    void addListener(@NonNull IAosRegistrationListener listener);

    /**
     * Removes a listener previously registered with {@link #addListener(IAosRegistrationListener)}
     *
     * @param listener The non-null listener to be removed.
     * @throws NullPointerException if the listener is null.
     */
    void removeListener(@NonNull IAosRegistrationListener listener);

    /**
    * Called by the framework to request that the ImsService perform the network registration
    * of all SIP delegates associated with this ImsService.
    */
    void updateSipDelegateRegistration();

    /**
     * Called by the framework to request that the ImsService perform the network deregistration
     * of all SIP delegates associated with this ImsService.
     */
    void triggerSipDelegateDeregistration();

    /**
     * Called by the framework to notify the ImsService that a SIP delegate connection has received
     * a SIP message containing a permanent failure response (with a SIP status code between
     * 100 and 699) or an indication that a SIP response timer has timed out in response to
     * an outgoing SIP message.
     *
     * @param sipCode The SIP status code indicating the reason for the failure.
     *                Must be within the range of 100 to 699 (inclusive).
     * @param sipReason Optional additional information about the failure reason.
     * @throws IllegalArgumentException if the sipCode is outside the valid range.
     */
    void triggerFullNetworkRegistration(@IntRange(from = 100, to = 699) int sipCode,
            @Nullable String sipReason);

    /**
     * This method is called when capabilities are changed.
     * If the capabilities changed by calling this method is not updated,
     * the following API is called. {@link IAosRegistrationListener#notifyCapabilitiesUpdateFailed}
     *
     * @param capabilityPairs A non-null map containing the enabled capabilities for each network
     *                        type.
     * @throws NullPointerException if {@code capabilityPairs} is null.
     */
    void changeCapabilities(@NonNull CapabilityPairs capabilityPairs);

    /**
     * This method is called when controlling registration.
     *
     * @param requestType Type of int {@link RequestType}.
     * @param pcscfOrder Type of int {@link Pcscf}.
     * @param cause Type of int {@link Cause}.
     */
    void controlRegistration(RequestType requestType, Pcscf pcscfOrder, Cause cause);

    /**
     * This method is returns the network type in which the IMS registered.
     *
     * @return NetworkType {@link IAosRegistrationListener.NetworkType}
     *    {@code IAosRegistrationListener.NetworkType.NONE} if IMS is not registered,
     *    The NetworkType is returns, if IMS is registered.
     */
    NetworkType getRegisteredNetworkType();

    /**
     * Retrieves the current IMS registration state.
     *
     * @return The registration state as a {@link RegistrationState} enum value.
     */
    RegistrationState getRegistrationState();

    /**
     * Represents the types of requests that can be made.
     * Each request type is associated with a corresponding integer value.
     */
    enum RequestType {
        START(0),
        REFRESH(1),
        STOP(2),
        START_IMS_EST_TIMER(3);

        private final int mValue;

        RequestType(int value) {
            mValue = value;
        }

        public int getValue() {
            return mValue;
        }
    }

    /**
     * Represents the order of P-CSCF addresses.
     * Each P-CSCF order is associated with a corresponding integer value.
     */
    enum Pcscf {
        FIRST(0),
        CURRENT(1),
        NEXT(2);

        private final int mValue;

        Pcscf(int value) {
            mValue = value;
        }

        public int getValue() {
            return mValue;
        }
    }

    /**
     * Defines possible causes for a specific event or failure.
     */
    enum Cause {
        UNKNOWN(0),
        DATA(1),
        RADIO(2),
        IMS_SERVICE(3),
        IMS_SUBSCRIBER(4),
        DATA_CONNECTING(5),

        // From modem
        RADIO_SIM_REMOVED(11),
        RADIO_SIM_REFRESH(12),
        RADIO_ALLOWED_NETWORK_TYPES_CHANGED(13),

        // From framework
        RADIO_POWER_OFF(21),
        NON_IMS_CAPABLE_NETWORK(22),
        DATA_STALL(23),
        HANDOVER_FAILED(24),
        VOPS_NOT_SUPPORTED(25),
        WIFI_OFF(26);

        private final int mValue;

        /**
         * Constructs a Cause with the given value.
         *
         * @param value The integer value of the cause.
         */
        Cause(int value) {
            mValue = value;
        }

        /**
         * Returns the integer value of the cause.
         *
         * @return The integer value of the cause.
         */
        public int getValue() {
            return mValue;
        }

        @Override
        public String toString() {
            return name();
        }

        /**
         * Returns the Cause enum constant corresponding to the given integer value.
         *
         * @param value The integer value to look up.
         * @return The Cause enum constant with the given value, or
         *         {@link #UNKNOWN} if no such constant exists.
         */
        public static Cause of(int value) {
            return Arrays.stream(values())
                .filter(cause -> cause.mValue == value)
                .findFirst()
                .orElse(UNKNOWN);
        }
    }

    /**
     * Represents a set of capability pairs associated with different network types.
     */
    final class CapabilityPairs {

        /**
         * The key of {@code Map<NetworkType, Integer>} is networkType.
         * {@link IAosRegistrationListener.NetworkType}
         *
         * The value of {@code Map<NetworkType, Integer>} is Capability.
         * {@link IAosRegistrationListener.Capability}
         */
        private final Map<NetworkType, Integer> mCapabilities;

        /**
         * Constructs an empty CapabilityPairs object.
         */
        public CapabilityPairs() {
            this(null, null);
        }

        /**
         * Constructs a CapabilityPairs object with an initial capability for the given network
         * type.
         *
         * @param networkType The network type.
         * @param capability The capability to be added.
         */
        public CapabilityPairs(NetworkType networkType, Integer capability) {
            mCapabilities = new LinkedHashMap<>();
            if (networkType != null && capability != null) {
                addCapability(networkType, capability);
            }
        }

        /**
         * Adds a capability to the specified network type.
         * If the network type already exists, the capability is bitwise ORed with the existing
         * value.
         *
         * @param networkType The network type.
         * @param capability The capability to be added.
         */
        public void addCapability(NetworkType networkType, Integer capability) {
            mCapabilities.put(networkType,
                    mCapabilities.getOrDefault(networkType, 0) | capability);
        }

        /**
         * Checks if the specified capability is present for the given network type.
         *
         * @param networkType The network type.
         * @param capability The capability to be checked.
         * @return {@code true} if the capability is present, {@code false} otherwise.
         */
        public boolean hasCapability(NetworkType networkType, Integer capability) {
            return (mCapabilities.getOrDefault(networkType, 0) & capability) > 0;
        }

        /**
         * This method returns a pair of capabilities.
         *
         * @return mCapabilities is type of {@code Map<Integer, Integer>}.
         */
        public Map<NetworkType, Integer> getCapabilities() {
            return mCapabilities;
        }

        @Override
        public boolean equals(Object o) {
            if (this == o) {
                return true;
            }

            if (!(o instanceof CapabilityPairs that) || that.getCapabilities() == null) {
                return false;
            }

            if (this.getCapabilities().size() != that.getCapabilities().size()) {
                return false;
            }

            return this.getCapabilities().entrySet().stream().allMatch(
                    e -> e.getValue().equals(that.getCapabilities().get(e.getKey())));
        }

        @Override
        public int hashCode() {
            return getCapabilities().entrySet().stream()
                .mapToInt(entry -> Objects.hash(entry.getKey(), entry.getValue()))
                .sum();
        }

        @Override
        public String toString() {
            return getCapabilities().entrySet().stream().map(
                    entry -> "(Network=" + entry.getKey() + ", Capabilities="
                            + IAosRegistrationListener.Capability.toString(entry.getValue()) + ")")
                    .collect(Collectors.joining(
                            ", ", "{ Size=" + getCapabilities().size() + ", ", " }"));
        }
    }
}