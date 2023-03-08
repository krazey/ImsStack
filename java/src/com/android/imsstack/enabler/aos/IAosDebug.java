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

import android.annotation.Nullable;
import android.app.Activity;

import java.util.LinkedHashMap;
import java.util.Map;
import java.util.Objects;

/**
 * This class provides an interface for using AosDebug functions.
 */
public interface IAosDebug {
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
     * This method is used to check whether to display a debug notification.
     *
     * @param activity Activity requesting permission check.
     */
    void showOrDismissNotification(Activity activity);

    /**
     * This method is used to inform AosDebug of the information when onRequestPermissionsResult is
     * invoked.
     *
     * @param requestCode The request code passed in
     *     {@link Activity#requestPermissions(String[], int)}.
     * @param permissions The requested permissions. Never null.
     * @param grantResults The grant results for the corresponding permissions
     *                     which is either
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
     * This class is a Data class for updating in AosDebug.
     */
    final class DebugData {
        public static final int KEY_SUB_ID = 0;
        public static final int KEY_REGISTER = 1;
        public static final int KEY_REGISTER_TIME = 2;
        public static final int KEY_DEREGISTER_TIME = 3;
        public static final int KEY_DEREGISTER_REASON = 4;
        public static final int KEY_REGISTERED_NETWORK_TYPE = 5;
        public static final int KEY_FEATURES = 6;
        public static final int KEY_CAPABILITIES = 7;
        public static final int KEY_DATA_CONNECTION_STATE = 8;
        public static final int KEY_IP_ADDRESSES = 9;
        public static final int KEY_INTERFACE_NAME = 10;
        public static final int KEY_MTU = 11;
        public static final int KEY_NETWORK_TYPE = 12;
        public static final int KEY_APN_NAME = 13;
        public static final int KEY_APN_TYPES = 14;
        public static final int KEY_APN_ENTRY_NAME = 15;
        public static final int KEY_PCSCF_ADDRESSES = 16;
        public static final int KEY_SERVICE_STATE = 17;
        public static final int KEY_DATA_REG_STATE = 18;
        public static final int KEY_VOICE_RAT = 19;
        public static final int KEY_CELLULAR_DATA_RAT = 20;
        public static final int KEY_LTE_ATTACH_TYPE = 21;
        public static final int KEY_ROAMING_STATE = 22;
        public static final int KEY_VOICE_ROAMING_TYPE = 23;
        public static final int KEY_DATA_ROAMING_TYPE = 24;
        public static final int KEY_NETWORK_OPERATOR = 25;
        public static final int KEY_NETWORK_OPERATOR_NUMERIC = 26;
        public static final int KEY_NETWORK_SUPPORT_VOPS = 27;
        public static final int KEY_NETWORK_SUPPORT_EMCBS = 28;
        public static final int KEY_UTRAN_RSRI = 29;
        public static final int KEY_UTRAN_RSCP = 30;
        public static final int KEY_EUTRAN_RSRP = 31;
        public static final int KEY_EUTRAN_RSRQ = 32;
        public static final int KEY_NGRAN_SSRSRP = 33;
        public static final int KEY_NGRAN_SSRSRQ = 34;
        public static final int KEY_WIFI_CONNECTION_STATE = 35;
        public static final int KEY_WIFI_ADDRESSES = 36;
        public static final int KEY_WIFI_INTERFACE_NAME = 37;
        public static final int KEY_WIFI_RSSI = 38;
        public static final int KEY_WIFI_BSSID = 39;
        public static final int KEY_WIFI_SSID = 40;
        public static final int KEY_WIFI_MAC_ADDRESS = 41;

        public static final String STR_EMPTY = "_";
        public static final String STR_IMS_REGISTERED = "REGISTERED";
        public static final String STR_IMS_DEREGISTERED = "DEREGISTERED";
        public static final String STR_CONNECTED = "CONNECTED";
        public static final String STR_DISCONNECTED = "DISCONNECTED";

        private final Map<Integer, String> mDebugData;

        public DebugData() {
            mDebugData = new LinkedHashMap<Integer, String>();
            mDebugData.put(DebugData.KEY_SUB_ID, STR_EMPTY);
            mDebugData.put(DebugData.KEY_REGISTER, STR_EMPTY);
            mDebugData.put(DebugData.KEY_REGISTER_TIME, STR_EMPTY);
            mDebugData.put(DebugData.KEY_DEREGISTER_TIME, STR_EMPTY);
            mDebugData.put(DebugData.KEY_DEREGISTER_REASON, STR_EMPTY);
            mDebugData.put(DebugData.KEY_REGISTERED_NETWORK_TYPE, STR_EMPTY);
            mDebugData.put(DebugData.KEY_FEATURES, STR_EMPTY);
            mDebugData.put(DebugData.KEY_CAPABILITIES, STR_EMPTY);
            mDebugData.put(DebugData.KEY_DATA_CONNECTION_STATE, STR_DISCONNECTED);
            mDebugData.put(DebugData.KEY_IP_ADDRESSES, STR_EMPTY);
            mDebugData.put(DebugData.KEY_INTERFACE_NAME, STR_EMPTY);
            mDebugData.put(DebugData.KEY_MTU, STR_EMPTY);
            mDebugData.put(DebugData.KEY_NETWORK_TYPE, STR_EMPTY);
            mDebugData.put(DebugData.KEY_APN_NAME, STR_EMPTY);
            mDebugData.put(DebugData.KEY_APN_TYPES, STR_EMPTY);
            mDebugData.put(DebugData.KEY_APN_ENTRY_NAME, STR_EMPTY);
            mDebugData.put(DebugData.KEY_PCSCF_ADDRESSES, STR_EMPTY);
            mDebugData.put(DebugData.KEY_SERVICE_STATE, STR_EMPTY);
            mDebugData.put(DebugData.KEY_DATA_REG_STATE, STR_EMPTY);
            mDebugData.put(DebugData.KEY_VOICE_RAT, STR_EMPTY);
            mDebugData.put(DebugData.KEY_CELLULAR_DATA_RAT, STR_EMPTY);
            mDebugData.put(DebugData.KEY_LTE_ATTACH_TYPE, STR_EMPTY);
            mDebugData.put(DebugData.KEY_ROAMING_STATE, STR_EMPTY);
            mDebugData.put(DebugData.KEY_VOICE_ROAMING_TYPE, STR_EMPTY);
            mDebugData.put(DebugData.KEY_DATA_ROAMING_TYPE, STR_EMPTY);
            mDebugData.put(DebugData.KEY_NETWORK_OPERATOR, STR_EMPTY);
            mDebugData.put(DebugData.KEY_NETWORK_OPERATOR_NUMERIC, STR_EMPTY);
            mDebugData.put(DebugData.KEY_NETWORK_SUPPORT_VOPS, STR_EMPTY);
            mDebugData.put(DebugData.KEY_NETWORK_SUPPORT_EMCBS, STR_EMPTY);
            mDebugData.put(DebugData.KEY_UTRAN_RSRI, STR_EMPTY);
            mDebugData.put(DebugData.KEY_UTRAN_RSCP, STR_EMPTY);
            mDebugData.put(DebugData.KEY_EUTRAN_RSRP, STR_EMPTY);
            mDebugData.put(DebugData.KEY_EUTRAN_RSRQ, STR_EMPTY);
            mDebugData.put(DebugData.KEY_NGRAN_SSRSRP, STR_EMPTY);
            mDebugData.put(DebugData.KEY_NGRAN_SSRSRQ, STR_EMPTY);
            mDebugData.put(DebugData.KEY_WIFI_CONNECTION_STATE, STR_DISCONNECTED);
            mDebugData.put(DebugData.KEY_WIFI_ADDRESSES, STR_EMPTY);
            mDebugData.put(DebugData.KEY_WIFI_INTERFACE_NAME, STR_EMPTY);
            mDebugData.put(DebugData.KEY_WIFI_RSSI, STR_EMPTY);
            mDebugData.put(DebugData.KEY_WIFI_BSSID, STR_EMPTY);
            mDebugData.put(DebugData.KEY_WIFI_SSID, STR_EMPTY);
            mDebugData.put(DebugData.KEY_WIFI_MAC_ADDRESS, STR_EMPTY);
        }

        public void clear() {
            mDebugData.put(DebugData.KEY_DATA_CONNECTION_STATE, STR_DISCONNECTED);
            mDebugData.put(DebugData.KEY_IP_ADDRESSES, STR_EMPTY);
            mDebugData.put(DebugData.KEY_INTERFACE_NAME, STR_EMPTY);
            mDebugData.put(DebugData.KEY_MTU, STR_EMPTY);
            mDebugData.put(DebugData.KEY_NETWORK_TYPE, STR_EMPTY);
            mDebugData.put(DebugData.KEY_APN_NAME, STR_EMPTY);
            mDebugData.put(DebugData.KEY_APN_TYPES, STR_EMPTY);
            mDebugData.put(DebugData.KEY_APN_ENTRY_NAME, STR_EMPTY);
            mDebugData.put(DebugData.KEY_PCSCF_ADDRESSES, STR_EMPTY);
            mDebugData.put(DebugData.KEY_SERVICE_STATE, STR_EMPTY);
            mDebugData.put(DebugData.KEY_DATA_REG_STATE, STR_EMPTY);
            mDebugData.put(DebugData.KEY_VOICE_RAT, STR_EMPTY);
            mDebugData.put(DebugData.KEY_CELLULAR_DATA_RAT, STR_EMPTY);
            mDebugData.put(DebugData.KEY_LTE_ATTACH_TYPE, STR_EMPTY);
            mDebugData.put(DebugData.KEY_ROAMING_STATE, STR_EMPTY);
            mDebugData.put(DebugData.KEY_VOICE_ROAMING_TYPE, STR_EMPTY);
            mDebugData.put(DebugData.KEY_DATA_ROAMING_TYPE, STR_EMPTY);
            mDebugData.put(DebugData.KEY_NETWORK_OPERATOR, STR_EMPTY);
            mDebugData.put(DebugData.KEY_NETWORK_OPERATOR_NUMERIC, STR_EMPTY);
            mDebugData.put(DebugData.KEY_NETWORK_SUPPORT_VOPS, STR_EMPTY);
            mDebugData.put(DebugData.KEY_NETWORK_SUPPORT_EMCBS, STR_EMPTY);
            mDebugData.put(DebugData.KEY_UTRAN_RSRI, STR_EMPTY);
            mDebugData.put(DebugData.KEY_UTRAN_RSCP, STR_EMPTY);
            mDebugData.put(DebugData.KEY_EUTRAN_RSRP, STR_EMPTY);
            mDebugData.put(DebugData.KEY_EUTRAN_RSRQ, STR_EMPTY);
            mDebugData.put(DebugData.KEY_NGRAN_SSRSRP, STR_EMPTY);
            mDebugData.put(DebugData.KEY_NGRAN_SSRSRQ, STR_EMPTY);
            mDebugData.put(DebugData.KEY_WIFI_CONNECTION_STATE, STR_DISCONNECTED);
            mDebugData.put(DebugData.KEY_WIFI_ADDRESSES, STR_EMPTY);
            mDebugData.put(DebugData.KEY_WIFI_INTERFACE_NAME, STR_EMPTY);
            mDebugData.put(DebugData.KEY_WIFI_RSSI, STR_EMPTY);
            mDebugData.put(DebugData.KEY_WIFI_BSSID, STR_EMPTY);
            mDebugData.put(DebugData.KEY_WIFI_SSID, STR_EMPTY);
            mDebugData.put(DebugData.KEY_WIFI_MAC_ADDRESS, STR_EMPTY);
        }

        public void put(Integer key, String data) {
            mDebugData.put(key, data);
        }

        public void putInt(Integer key, int data) {
            mDebugData.put(key, String.valueOf(data));
        }

        public String get(Integer key) {
            return mDebugData.getOrDefault(key, STR_EMPTY);
        }

        public Map<Integer, String> getDebugData() {
            return mDebugData;
        }

        @Override
        public boolean equals(Object o) {
            if (this == o) {
                return true;
            }

            if (!(o instanceof DebugData)) {
                return false;
            }

            DebugData that = (DebugData) o;

            if (this.getDebugData().size() != that.getDebugData().size()) {
                return false;
            }

            return this.getDebugData().entrySet().stream().allMatch(
                    e -> e.getValue().equals(that.getDebugData().get(e.getKey())));
        }

        @Override
        public int hashCode() {
            int code = 0;
            for (Map.Entry<Integer, String> entry : this.getDebugData().entrySet()) {
                code += Objects.hash(entry.getKey(), entry.getValue());
            }

            return code;
        }
    }
}
