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

import java.util.Arrays;
import java.util.LinkedHashMap;
import java.util.Map;
import java.util.Objects;
import java.util.Optional;

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
    void controlRegistration(int requestType, int pcscfOrder, int cause);

    /**
     * This method is returns the network type in which the IMS registered.
     *
     * @return int returns NetworkType {@link IAosRegistrationListener.NetworkType}
     *    {@code IAosRegistrationListener.NetworkType.NONE} if IMS is not registered,
     *    The NetworkType is returns, if IMS is registered.
     */
    int getRegisteredNetworkType();

    /**
     * This method provides the IMS registration state
     *
     * @return int returns RegistrationState {@link IAosRegistrationListener.RegistrationState}
     */
    int getRegistrationState();

    /**
     * Request Type
     */
    class RequestType {
        public static final int START = 0;
        public static final int REFRESH = 1;
        public static final int STOP = 2;
        public static final int START_IMS_EST_TIMER = 3;
    }

    /**
     * PCSCF
     */
    class Pcscf {
        public static final int FIRST = 0;
        public static final int CURRENT = 1;
        public static final int NEXT = 2;
    }

    /**
     * Defines possible causes for a specific event or failure.
     */
    enum Cause {
        UNKNOWN(0, "UNKNOWN"),
        DATA(1, "DATA"),
        RADIO(2, "RADIO"),
        IMS_SERVICE(3, "IMS_SERVICE"),
        IMS_SUBSCRIBER(4, "IMS_SUBSCRIBER"),
        DATA_CONNECTING(5, "DATA_CONNECTING"),

        // From modem
        RADIO_SIM_REMOVED(11, "RADIO_SIM_REMOVED"),
        RADIO_SIM_REFRESH(12, "RADIO_SIM_REFRESH"),
        RADIO_ALLOWED_NETWORK_TYPES_CHANGED(13, "RADIO_ALLOWED_NETWORK_TYPES_CHANGED"),

        // From framework
        RADIO_POWER_OFF(21, "RADIO_POWER_OFF"),
        NON_IMS_CAPABLE_NETWORK(22, "NON_IMS_CAPABLE_NETWORK"),
        DATA_STALL(23, "DATA_STALL"),
        HANDOVER_FAILED(24, "HANDOVER_FAILED"),
        VOPS_NOT_SUPPORTED(25, "VOPS_NOT_SUPPORTED"),
        WIFI_OFF(26, "WIFI_OFF");

        private final int mValue;
        private final String mLabel;

        /**
         * Constructs a Cause with the given value and label.
         *
         * @param value The integer value of the cause.
         * @param label The label of the cause.
         */
        Cause(int value, String label) {
            mValue = value;
            mLabel = label;
        }

        /**
         * Returns the integer value of the cause.
         *
         * @return The integer value of the cause.
         */
        public int getValue() {
            return mValue;
        }

        /**
         * Returns the label of the cause.
         *
         * @return The label of the cause.
         */
        public String getLabel() {
            return mLabel;
        }

        /**
         * Returns the Cause enum constant corresponding to the given integer value.
         *
         * @param value The integer value to look up.
         * @return An Optional containing the Cause enum constant with the given value,
         *         or an empty Optional if no such constant exists.
         */
        public static Optional<Cause> fromValue(int value) {
            return Arrays.stream(values())
                    .filter(cause -> cause.mValue == value)
                    .findFirst();
        }
    }

    /**
     * CapabilityPairs
     */
    final class CapabilityPairs {

        /**
         * The key of {@code Map<Integer, Integer>} is networkType.
         * {@link IAosRegistrationListener.NetworkType}
         *
         * The value of {@code Map<Integer, Integer>} is Capability.
         * {@link IAosRegistrationListener.Capability}
         */
        private final Map<Integer, Integer> mCapabilities;

        public CapabilityPairs() {
            mCapabilities = new LinkedHashMap<Integer, Integer>();
        }

        public CapabilityPairs(Integer networkType, Integer capability) {
            mCapabilities = new LinkedHashMap<Integer, Integer>();
            addCapability(networkType, capability);
        }

        public void addCapability(Integer networkType, Integer capability) {
            mCapabilities.put(networkType,
                    mCapabilities.getOrDefault(networkType, 0) | capability);
        }

        public boolean hasCapability(Integer networkType, Integer capability) {
            return (mCapabilities.getOrDefault(networkType, 0) & capability) > 0;
        }

        /**
         * This method returns a pair of capabilities.
         *
         * @return mCapabilities is type of {@code Map<Integer, Integer>}.
         */
        public Map<Integer, Integer> getCapabilities() {
            return mCapabilities;
        }

        @Override
        public boolean equals(Object o) {
            if (this == o) {
                return true;
            }

            if (!(o instanceof CapabilityPairs)) {
                return false;
            }

            CapabilityPairs that = (CapabilityPairs)o;

            if (this.getCapabilities().size() != that.getCapabilities().size()) {
                return false;
            }

            return this.getCapabilities().entrySet().stream().allMatch(
                    e -> e.getValue().equals(that.getCapabilities().get(e.getKey())));
        }

        @Override
        public int hashCode() {
            int code = 0;
            for (Map.Entry<Integer, Integer> entry : this.getCapabilities().entrySet()) {
                code += Objects.hash(entry.getKey(), entry.getValue());
            }

            return code;
        }

        @Override
        public String toString() {
            StringBuilder sb = new StringBuilder();
            sb.append("{ Size=").append(getCapabilities().size()).append(", ");
            for (Map.Entry<Integer, Integer> entry : getCapabilities().entrySet()) {
                sb.append("( Network=").append(entry.getKey()).append(", Capabilities=")
                        .append(entry.getValue()).append(" )");
            }
            sb.append(" }");

            return sb.toString();
        }
    }
}