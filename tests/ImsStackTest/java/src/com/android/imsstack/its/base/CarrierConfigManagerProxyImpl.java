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
import android.content.Context;
import android.os.PersistableBundle;
import android.telephony.CarrierConfigManager;
import android.telephony.CarrierConfigManager.CarrierConfigChangeListener;
import android.telephony.SubscriptionManager;
import android.telephony.TelephonyManager;
import android.util.ArraySet;
import android.util.SparseArray;

import androidx.annotation.NonNull;
import androidx.annotation.Nullable;

import com.android.imsstack.base.SystemServiceProxy.CarrierConfigManagerProxy;

import java.util.concurrent.Executor;

/**
 * An implementation class to access the {@link CarrierConfigManager}.
 */
public class CarrierConfigManagerProxyImpl implements CarrierConfigManagerProxy {
    private static final String[] CONFIG_SUBSET_METADATA_KEYS = new String[] {
            CarrierConfigManager.KEY_CARRIER_CONFIG_VERSION_STRING,
            CarrierConfigManager.KEY_CARRIER_CONFIG_APPLIED_BOOL
    };
    private final SparseArray<PersistableBundle> mConfigs = new SparseArray<>();
    private final ArraySet<CarrierConfigChangeListenerRecord> mListenerRecords = new ArraySet<>();
    private final Context mContext;
    private final PersistableBundle mDefaultConfig;

    CarrierConfigManagerProxyImpl(Context context) {
        mContext = context;
        mDefaultConfig = CarrierConfigManager.getDefaultConfig();
        mDefaultConfig.putBoolean(CarrierConfigManager.KEY_CARRIER_CONFIG_APPLIED_BOOL, true);

        // Sets the IMS service capabilities by default.
        mDefaultConfig.putBoolean(CarrierConfigManager.KEY_CARRIER_VOLTE_AVAILABLE_BOOL, true);
        mDefaultConfig.putBoolean(CarrierConfigManager.KEY_CARRIER_VT_AVAILABLE_BOOL, true);
        mDefaultConfig.putBoolean(
                CarrierConfigManager.ImsSms.KEY_SMS_OVER_IMS_SUPPORTED_BOOL, true);
    }

    @Override
    public boolean isConfigForIdentifiedCarrier(PersistableBundle bundle) {
        return CarrierConfigManager.isConfigForIdentifiedCarrier(bundle);
    }

    @Override
    public @NonNull PersistableBundle getDefaultConfig() {
        return CarrierConfigManager.getDefaultConfig();
    }

    @Override
    public @NonNull PersistableBundle getConfigForSubId(int subId) {
        PersistableBundle config = mConfigs.get(subId);
        if (config != null) {
            return config;
        }

        TelephonyManagerProxyImpl tmp = SystemProxyResolver.getTelephonyManagerProxy();
        int carrierId = tmp.getSimCarrierId();

        if (carrierId == TelephonyManager.UNKNOWN_CARRIER_ID
                || carrierId == TestConstants.CARRIER_ID) {
            return mDefaultConfig;
        }

        SubscriptionManagerProxyImpl smp = SystemProxyResolver.getSubscriptionManagerProxy();
        int simSubId = SubscriptionManager.getSubscriptionId(smp.getSlotIndex(subId));
        TelephonyManager tm = mContext.getSystemService(TelephonyManager.class);
        tm = tm.createForSubscriptionId(simSubId);
        int simCarrierId = tm.getSimCarrierId();

        if (carrierId == simCarrierId) {
            CarrierConfigManager ccm = mContext.getSystemService(CarrierConfigManager.class);
            return ccm.getConfigForSubId(simSubId);
        }

        return mDefaultConfig;
    }

    @Override
    public @NonNull PersistableBundle getConfigForSubId(int subId, @NonNull String... keys) {
        PersistableBundle allConfigs = getConfigForSubId(subId);
        PersistableBundle configSubset = new PersistableBundle(
                keys.length + CONFIG_SUBSET_METADATA_KEYS.length);
        for (String carrierConfigKey : keys) {
            Object value = allConfigs.get(carrierConfigKey);
            if (value == null) {
                // Filter out keys without values.
                continue;
            }
            putConfigValue(configSubset, carrierConfigKey, value);
        }

        // Configs in CONFIG_SUBSET_METADATA_KEYS should always be included.
        for (String generalKey : CONFIG_SUBSET_METADATA_KEYS) {
            putConfigValue(configSubset, generalKey, allConfigs.get(generalKey));
        }

        return configSubset;
    }

    @Override
    public void registerCarrierConfigChangeListener(
            @NonNull @CallbackExecutor Executor executor,
            @NonNull CarrierConfigChangeListener listener) {
        mListenerRecords.add(new CarrierConfigChangeListenerRecord(executor, listener));
    }

    @Override
    public void unregisterCarrierConfigChangeListener(
            @NonNull CarrierConfigChangeListener listener) {
        final ArraySet<CarrierConfigChangeListenerRecord> recordsToRemove = new ArraySet<>();
        mListenerRecords.forEach((r) -> {
            if (r.hasCarrierConfigChangeListener(listener)) {
                recordsToRemove.add(r);
            }
        });

        recordsToRemove.forEach(mListenerRecords::remove);
    }

    /**
     * Notifies the listener that the carrier configuration has changed.
     *
     * @param logicalSlotIndex  The logical SIM slot index on which to monitor and get
     *                          notification. It is guaranteed to be valid.
     * @param subscriptionId    The subscription on the SIM slot. May be
     *                          {@link SubscriptionManager#INVALID_SUBSCRIPTION_ID}.
     * @param carrierId         The optional carrier Id, may be
     *                          {@link TelephonyManager#UNKNOWN_CARRIER_ID}.
     *                          See {@link TelephonyManager#getSimCarrierId()}.
     * @param specificCarrierId The optional fine-grained carrierId, may be {@link
     *                          TelephonyManager#UNKNOWN_CARRIER_ID}. A specific carrierId may
     *                          be different from the carrierId above in a MVNO scenario. See
     *                          detail in {@link TelephonyManager#getSimSpecificCarrierId()}.
     */
    public void notifyCarrierConfigChanged(int logicalSlotIndex, int subscriptionId,
            int carrierId, int specificCarrierId) {
        mListenerRecords.forEach((r) -> {
            r.dispatchCarrierConfigChanged(
                    logicalSlotIndex, subscriptionId, carrierId, specificCarrierId);
        });
    }

    /**
     * Sets the carrier configuration for the specified subscription.
     *
     * @param subId The subscription id.
     * @param config The carrier configuration to be set.
     */
    public void setConfigForSubId(int subId, @Nullable PersistableBundle config) {
        if (config != null) {
            for (String generalKey : CONFIG_SUBSET_METADATA_KEYS) {
                if (!config.containsKey(generalKey)) {
                    PersistableBundle defaultConfig = getDefaultConfig();
                    putConfigValue(config, generalKey, defaultConfig.get(generalKey));
                }
            }
        }
        mConfigs.put(subId, config);
    }

    private void putConfigValue(PersistableBundle config, String key, Object value) {
        if (value == null) {
            config.putString(key, null);
        } else if (value instanceof Boolean) {
            config.putBoolean(key, (Boolean) value);
        } else if (value instanceof Integer) {
            config.putInt(key, (Integer) value);
        } else if (value instanceof Long) {
            config.putLong(key, (Long) value);
        } else if (value instanceof Double) {
            config.putDouble(key, (Double) value);
        } else if (value instanceof String) {
            config.putString(key, (String) value);
        } else if (value instanceof boolean[]) {
            config.putBooleanArray(key, (boolean[]) value);
        } else if (value instanceof int[]) {
            config.putIntArray(key, (int[]) value);
        } else if (value instanceof long[]) {
            config.putLongArray(key, (long[]) value);
        } else if (value instanceof double[]) {
            config.putDoubleArray(key, (double[]) value);
        } else if (value instanceof String[]) {
            config.putStringArray(key, (String[]) value);
        } else if (value instanceof PersistableBundle) {
            config.putPersistableBundle(key, (PersistableBundle) value);
        } else {
            throw new IllegalArgumentException("Unsupported type: " + value.getClass());
        }
    }

    private static final class CarrierConfigChangeListenerRecord {
        private final Executor mScheduler;
        private final CarrierConfigChangeListener mListener;

        CarrierConfigChangeListenerRecord(Executor executor,
                CarrierConfigChangeListener listener) {
            mScheduler = executor;
            mListener = listener;
        }

        boolean hasCarrierConfigChangeListener(CarrierConfigChangeListener listener) {
            return mListener.equals(listener);
        }

        void dispatchCarrierConfigChanged(int logicalSlotIndex, int subscriptionId,
                int carrierId, int specificCarrierId) {
            mScheduler.execute(() -> {
                mListener.onCarrierConfigChanged(
                        logicalSlotIndex, subscriptionId, carrierId, specificCarrierId);
            });
        }
    }
}
