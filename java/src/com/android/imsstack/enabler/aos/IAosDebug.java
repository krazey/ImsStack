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

package com.android.imsstack.enabler.aos;

import android.app.Activity;

import androidx.annotation.NonNull;
import androidx.annotation.Nullable;

import java.util.Collections;
import java.util.EnumMap;
import java.util.Map;
import java.util.Objects;
import java.util.Set;

/**
 * Provides an interface for interacting with AoS (Always On Service) debugging features.
 * This interface allows for initializing and cleaning up debug resources,
 * managing debug notifications, retrieving debug messages, and handling permission results.
 * It also defines a {@link DebugData} class for storing and managing debug information.
 */
public interface IAosDebug {

    /**
     * Defines keys used to identify specific debug data items within the {@link DebugData} class.
     * Each key is associated with a unique integer value for efficient storage and retrieval.
     */
    enum DebugKey {
        SUB_ID(0),
        REGISTER(1),
        REGISTER_TIME(2),
        DEREGISTER_TIME(3),
        DEREGISTER_REASON(4),
        REGISTERED_NETWORK_TYPE(5),
        FEATURES(6),
        CAPABILITIES(7),
        DATA_CONNECTION_STATE(8),
        IP_ADDRESSES(9),
        INTERFACE_NAME(10),
        MTU(11),
        NETWORK_TYPE(12),
        APN_NAME(13),
        APN_TYPES(14),
        APN_ENTRY_NAME(15),
        PCSCF_ADDRESSES(16),
        SERVICE_STATE(17),
        DATA_REG_STATE(18),
        VOICE_RAT(19),
        CELLULAR_DATA_RAT(20),
        LTE_ATTACH_TYPE(21),
        ROAMING_STATE(22),
        VOICE_ROAMING_TYPE(23),
        DATA_ROAMING_TYPE(24),
        NETWORK_OPERATOR(25),
        NETWORK_OPERATOR_NUMERIC(26),
        NETWORK_SUPPORT_VOPS(27),
        NETWORK_SUPPORT_EMCBS(28),
        UTRAN_LEVEL(29),
        UTRAN_DBM(30),
        EUTRAN_RSRP(31),
        EUTRAN_RSRQ(32),
        NGRAN_SSRSRP(33),
        NGRAN_SSRSRQ(34),
        WIFI_CONNECTION_STATE(35),
        WIFI_ADDRESSES(36),
        WIFI_INTERFACE_NAME(37),
        WIFI_RSSI(38),
        WIFI_BSSID(39),
        WIFI_SSID(40),
        WIFI_MAC_ADDRESS(41);

        private final int mValue;

        DebugKey(int value) {
            mValue = value;
        }

        /**
         * Returns the integer value associated with this debug key.
         *
         * @return The integer value.
         */
        public int getValue() {
            return mValue;
        }
    }

     /**
      * A set of {@link DebugKey}s that should be preserved (not cleared) when the {@code clear()}
      * method of {@link DebugData} is called.
      */
    Set<DebugKey> PRESERVED_KEYS = Set.of(
            DebugKey.SUB_ID,
            DebugKey.REGISTER,
            DebugKey.REGISTER_TIME,
            DebugKey.DEREGISTER_TIME,
            DebugKey.DEREGISTER_REASON,
            DebugKey.REGISTERED_NETWORK_TYPE,
            DebugKey.FEATURES,
            DebugKey.CAPABILITIES);

    /**
     * Initializes the AosDebug object.
     *
     */
    void init();

    /**
     * Clean up object.
     */
    void cleanup();

    /**
     * Shows or dismisses the debug notification based on the following conditions:
     * - When the debug screen is disabled, the notification is dismissed.
     * - When the activity is null (invoked internally), or if the permission has been granted,
     *   the notification channel is created and the notification is sent.
     * - When the activity is not null and the permission has not been granted,
     *   the permission is requested.
     *
     * @param activity The activity requesting permission check, or null if invoked internally.
     */
    void showOrDismissNotification(@Nullable Activity activity);

    /**
     * This method is used to inform AosDebug of the information when onRequestPermissionsResult is
     * invoked.
     *
     * @param requestCode The request code passed in
     *     {@link Activity#requestPermissions(String[], int)}.
     * @param permissions The requested permissions. Never null.
     * @param grantResults The grant results for the corresponding permissions which is either
     *                     {@link android.content.pm.PackageManager#PERMISSION_GRANTED} or
     *                     {@link android.content.pm.PackageManager#PERMISSION_DENIED}. Never null.
     * @param activity The activity whose visibility requested the permission.
     *
     * @see Activity#requestPermissions(String[], int)
     */
    void notifyPermissionsResult(int requestCode, @Nullable String[] permissions,
            int[] grantResults, Activity activity);

    /**
     * This method is used to get the text to display on the debug screen.
     *
     * @return String Text to display on the debug screen.
     */
    String getDebugMessage();

    /**
     * A data class for storing and managing debug information in {@link IAosDebug}.
     */
    final class DebugData {

        public static final String STR_EMPTY = "_";
        public static final String STR_IMS_REGISTERED = "REGISTERED";
        public static final String STR_IMS_DEREGISTERED = "DEREGISTERED";
        public static final String STR_CONNECTED = "CONNECTED";
        public static final String STR_DISCONNECTED = "DISCONNECTED";

        private final Map<DebugKey, String> mDebugData = new EnumMap<>(DebugKey.class);

        public DebugData() {
            for (DebugKey key : DebugKey.values()) {
                mDebugData.put(key, STR_EMPTY);
            }

            clear();
        }

        /**
         * Clears the debug data for clearable keys.
         */
        public void clear() {
            for (DebugKey key : DebugKey.values()) {
                if (!PRESERVED_KEYS.contains(key)) {
                    mDebugData.put(key, STR_EMPTY);
                }
            }

            mDebugData.put(DebugKey.DATA_CONNECTION_STATE, STR_DISCONNECTED);
            mDebugData.put(DebugKey.WIFI_CONNECTION_STATE, STR_DISCONNECTED);
        }

        /**
         * Puts a String value for the specified key in the debug data map.
         *
         * @param key  The debug key. Must not be null.
         * @param data The String value to put.
         */
        public void put(@NonNull DebugKey key, String data) {
            Objects.requireNonNull(key, "key must not be null");
            mDebugData.put(key, data);
        }

        /**
         * Puts an integer value for the specified key in the debug data map.
         * The integer value is converted to a String before being stored.
         *
         * @param key  The debug key. Must not be null.
         * @param data The integer value to put.
         */
        public void putInt(@NonNull DebugKey key, int data) {
            Objects.requireNonNull(key, "key must not be null");
            mDebugData.put(key, String.valueOf(data));
        }

        /**
         * Gets the String value associated with the specified key from the debug data map.
         * If the key is not found, the default value {@code STR_EMPTY} ("_") is returned.
         *
         * @param key The debug key to retrieve the value for. Must not be null.
         * @return The String value associated with the key, or {@code STR_EMPTY} if not found.
         */
        public String get(@NonNull DebugKey key) {
            Objects.requireNonNull(key, "key must not be null");
            return mDebugData.getOrDefault(key, STR_EMPTY);
        }

        /**
         * Returns an unmodifiable view of the debug data map.
         *
         * @return An unmodifiable view of the debug data map.
         */
        public Map<DebugKey, String> getDebugData() {
            return Collections.unmodifiableMap(mDebugData);
        }
    }
}
