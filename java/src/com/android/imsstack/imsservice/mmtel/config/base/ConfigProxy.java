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
package com.android.imsstack.imsservice.mmtel.config.base;

import android.content.Context;
import android.content.SharedPreferences;
import android.content.SharedPreferences.Editor;
import android.os.PersistableBundle;
import android.telephony.ims.ProvisioningManager;
import android.telephony.ims.stub.ImsConfigImplBase;
import android.util.SparseArray;

import com.android.imsstack.base.MSimUtils;
import com.android.imsstack.core.agents.AgentFactory;
import com.android.imsstack.core.agents.ConfigInterface;
import com.android.imsstack.core.agents.SimInterface;
import com.android.imsstack.core.config.CarrierConfig;
import com.android.imsstack.enabler.IContext;
import com.android.imsstack.util.ImsLog;
import com.android.internal.annotations.VisibleForTesting;

import java.util.Queue;
import java.util.Set;
import java.util.concurrent.ConcurrentLinkedQueue;
import java.util.concurrent.CopyOnWriteArraySet;
import java.util.concurrent.ExecutorService;
import java.util.concurrent.Executors;

public class ConfigProxy {

    private static final String SHARED_PREFERENCE_IMSCONFIG = "imsconfig";
    private final IContext mContext;
    private final ImsConfigImplBase mConfig;
    private PersistableBundle mImsCarrierConfigs;
    private final Set<ConfigurationListener> mConfigurationListeners
            = new CopyOnWriteArraySet<ConfigurationListener>();
    private Queue<Integer> mData = null;
    private ExecutorService mRequestExecutor;

    public ConfigProxy(IContext context, ImsConfigImplBase config) {
        mContext = context;
        mConfig = config;
    }

    public final void init() {
        mRequestExecutor = Executors.newSingleThreadExecutor();
        mData = new ConcurrentLinkedQueue<>();

        if (!hasSharedPreference()) {
            SharedPreferences preferences = mContext.getContext().getSharedPreferences(
                    getPreferenceName(), Context.MODE_PRIVATE);

            if (preferences == null) {
                ImsLog.w("Shared Preference obj is null.");
                return;
            }

            Editor editor = preferences.edit();

            editor.clear();
            editor.commit();

            loadDefaultProvisionValues();
            loadOperatorSpecificDefaults();
        }
    }

    public void clear() {
        mConfigurationListeners.clear();
        mData = null;
        mRequestExecutor.shutdown();
    }

    public void addListener(ConfigurationListener listener) {
        mConfigurationListeners.add(listener);
    }

    public void removeListener(ConfigurationListener listener) {
        mConfigurationListeners.remove(listener);
    }

    /**
     * Gets the provisioning integer value.
     */
    public int getValueInt(int item) {
        ProvisioningItem provisionItem = ConfigUtils.getProvisioningItem(item);

        if ((provisionItem == null) ||
                (provisionItem.isIntegerFormat() == false &&
                provisionItem.isBooleanFormat() == false)) {
            ImsLog.w("Not an integer item or item not found:" + item);
            return ImsConfigImplBase.CONFIG_RESULT_UNKNOWN;
        }

        SharedPreferences preferences = mContext.getContext().getSharedPreferences(
                getPreferenceName(), Context.MODE_PRIVATE);

        if (preferences == null) {
            ImsLog.w("Shared Preference obj is null.");
            return ImsConfigImplBase.CONFIG_RESULT_UNKNOWN;
        }

        return preferences.getInt(provisionItem.getName(),
                ImsConfigImplBase.CONFIG_RESULT_UNKNOWN);
    }

    /**
     * Gets the provisioning string value.
     */
    public String getValueString(int item) {
        ProvisioningItem provisionItem = ConfigUtils.getProvisioningItem(item);

        if ((provisionItem == null) || (provisionItem.isStringFormat() == false)) {
            ImsLog.w("Not a string item or item not found:" + item);
            return null;
        }

        SharedPreferences preferences = mContext.getContext().getSharedPreferences(
                getPreferenceName(), Context.MODE_PRIVATE);

        if (preferences == null) {
            ImsLog.w("Shared Preference obj is null.");
            return null;
        }

        return preferences.getString(provisionItem.getName(), null);
    }

    public final void updateCarrierConfigData(PersistableBundle bundle) {
        mImsCarrierConfigs = bundle;
    }

    public PersistableBundle getCarrierConfigData() {
        return mImsCarrierConfigs;
    }

    /**
     * Sets the provisioning integer value.
     */
    public boolean setValueInt(int item, int value) {
        ProvisioningItem provisionItem = ConfigUtils.getProvisioningItem(item);

        if ((provisionItem == null) ||
                (provisionItem.isIntegerFormat() == false &&
                provisionItem.isBooleanFormat() == false)) {
            ImsLog.w("Not an integer item or item not found :" + item);
            return false;
        }

        if ((provisionItem.isBooleanFormat() == true) &&
                ((value != ProvisioningManager.PROVISIONING_VALUE_ENABLED) &&
                (value != ProvisioningManager.PROVISIONING_VALUE_DISABLED))) {
            ImsLog.w("Not a valid boolean value for item:" + item + ", value:" + value);
            return false;
        }

        SharedPreferences preferences = mContext.getContext().getSharedPreferences(
                getPreferenceName(), Context.MODE_PRIVATE);

        if (preferences == null) {
            ImsLog.w("Shared Preference obj is null.");
            return false;
        }

        int oldValue = preferences.getInt(provisionItem.getName(),
                ImsConfigImplBase.CONFIG_RESULT_UNKNOWN);

        if (oldValue == value) {
            return true;
        }

        Editor editor = preferences.edit();
        editor.putInt(provisionItem.getName(),value);
        editor.apply();

        mData.offer(item);
        if (mData.size() == 1) {
            mRequestExecutor.execute(() -> notifyConfigItemsChanged());
        }

        return true;
    }

    /**
     * Sets the provisioning string value.
     */
    public boolean setValueString(int item, String value) {
        ProvisioningItem provisionItem = ConfigUtils.getProvisioningItem(item);

        if ((provisionItem == null) || (provisionItem.isStringFormat() == false)) {
            ImsLog.w("Not a string item or item not found:" + item);
            return false;
        }

        SharedPreferences preferences = mContext.getContext().getSharedPreferences(
                getPreferenceName(), Context.MODE_PRIVATE);

        if (preferences == null) {
            ImsLog.w("Shared Preference obj is null.");
            return false;
        }

        String oldValue = preferences.getString(provisionItem.getName(), null);

        if ((oldValue != null) && oldValue.equals(value)) {
            return true;
        }

        Editor editor = preferences.edit();
        editor.putString(provisionItem.getName(), value);
        editor.apply();

        mData.offer(item);
        if (mData.size() == 1) {
            mRequestExecutor.execute(() -> notifyConfigItemsChanged());
        }

        return true;
    }

    /**
     * Notify the change of IMS configuration to the application.
     * It can be used to notify the change of IMS provisioning items
     * for voice/video call, Wi-Fi calling.
     *
     * Value: 0 (disabled), 1 (enabled)
     */
    public void notifyImsConfigChanged(IContext context, int item, int value) {
        log("notifyImsConfigChanged :: item=" + item
                + ", value=" + value + ", slot=" + context.getSlotId());

        mConfig.notifyProvisionedValueChanged(item, value);
        mConfig.notifyProvisionedValueChanged(item, String.valueOf(value));
    }

    public final void loadDefaultProvisionValues() {
        SharedPreferences preferences = mContext.getContext().getSharedPreferences(
                getPreferenceName(), Context.MODE_PRIVATE);

        if (preferences == null) {
            ImsLog.w("Shared Preference obj is null.");
            return;
        }

        Editor editor = preferences.edit();

        SparseArray<ProvisioningItem> provisioningItems = ConfigUtils.getProvisioningItems();

        ConfigInterface config = AgentFactory.getInstance().getAgent(
                    ConfigInterface.class, mContext.getSlotId());

        if (config != null) {
            CarrierConfig cc = config.getCarrierConfig();

            for (int i = 0; i < provisioningItems.size(); i++) {
                ProvisioningItem item = provisioningItems.get(i);
                if (item == null) {
                    continue;
                }

                String carrierConfigKey = item.getCarrierConfigKey();
                if (carrierConfigKey == null) {
                    continue;
                }

                if (item.isStringFormat()) {
                    editor.putString(item.getName(), cc.getString(carrierConfigKey));
                } else if (item.isBooleanFormat()) {
                    int value = cc.getBoolean(carrierConfigKey) ? 1 : 0;
                    editor.putInt(item.getName(), value);
                } else {
                    editor.putInt(item.getName(), cc.getInt(carrierConfigKey));
                }
            }

            ProvisioningItem item = provisioningItems.get(
                    ProvisioningManager.KEY_SIP_INVITE_RESPONSE_RETRANSMIT_INTERVAL_MS);

            int value = cc.getInt(item.getCarrierConfigKey());
            editor.putInt(item.getName(), 64*value);

            /* Set default to ENABLED for VoLTE/ViLTE/VoWiFi and SMSoIP */
            item = provisioningItems.get(ProvisioningManager.KEY_VOLTE_PROVISIONING_STATUS);
            editor.putInt(item.getName(), ProvisioningManager.PROVISIONING_VALUE_ENABLED);

            item = provisioningItems.get(ProvisioningManager.KEY_VT_PROVISIONING_STATUS);
            editor.putInt(item.getName(), ProvisioningManager.PROVISIONING_VALUE_ENABLED);

            item = provisioningItems.get(ProvisioningManager.KEY_VOICE_OVER_WIFI_ENABLED_OVERRIDE);
            editor.putInt(item.getName(), ProvisioningManager.PROVISIONING_VALUE_ENABLED);

            item = provisioningItems.get(ProvisioningManager.KEY_SMS_OVER_IP_ENABLED);
            editor.putInt(item.getName(), ProvisioningManager.PROVISIONING_VALUE_ENABLED);
        }

        editor.apply();
    }

    @VisibleForTesting
    void notifyConfigItemsChanged() {
        while (!mData.isEmpty()) {
            int item = mData.peek();
            for (ConfigurationListener l : mConfigurationListeners) {
                l.onImsConfigurationChanged(item);
            }

            mData.poll();
        }
    }

    private void loadOperatorSpecificDefaults() {
        SharedPreferences preferences = mContext.getContext().getSharedPreferences(
                getPreferenceName(), Context.MODE_PRIVATE);

        if (preferences == null) {
            ImsLog.w("Shared Preference obj is null.");
            return;
        }

        Editor editor = preferences.edit();
        if (editor == null) {
            ImsLog.w("Editor is null");
            return;
        }

        SparseArray<ProvisioningItem> provisioningItems = ConfigUtils.getProvisioningItems();

        ProvisioningItem item = provisioningItems.get(
                ProvisioningManager.KEY_SIP_INVITE_CANCELLATION_TIMER_MS);
        editor.putInt(item.getName(), 6);

        item = provisioningItems.get(ProvisioningManager.KEY_TRANSITION_TO_LTE_DELAY_MS);
        editor.putInt(item.getName(), 5);

        item = provisioningItems.get(ProvisioningManager.KEY_ENABLE_SILENT_REDIAL);
        editor.putInt(item.getName(), 1);

        item = provisioningItems.get(ProvisioningManager.KEY_REGISTRATION_DOMAIN_NAME);
        editor.putString(item.getName(), "vzims.com");

        /* Set default to ENABLED for EAB */
        item = provisioningItems.get(ProvisioningManager.KEY_EAB_PROVISIONING_STATUS);
        editor.putInt(item.getName(), ProvisioningManager.PROVISIONING_VALUE_ENABLED);

        editor.apply();
    }

    private boolean hasSharedPreference() {
        SharedPreferences preferences = mContext.getContext().getSharedPreferences(
                getPreferenceName(), Context.MODE_PRIVATE);

        return (preferences != null) ? !preferences.getAll().isEmpty() : false;
    }

    private String getPreferenceName() {
        SimInterface sim = AgentFactory.getInstance().getAgent(
                SimInterface.class, mContext.getSlotId());
        int subId = (sim != null) ? sim.getSubId() : MSimUtils.INVALID_SUB_ID;

        return SHARED_PREFERENCE_IMSCONFIG + "_sub" + subId;
    }

    private static void log(String s) {
        ImsLog.d("[GII-CONFIG] " + s);
    }
}
