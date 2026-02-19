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
package com.android.imsstack.imsservice.mmtel;

import android.os.PersistableBundle;
import android.telephony.ims.RcsClientConfiguration;
import android.telephony.ims.stub.ImsConfigImplBase;

import com.android.imsstack.enabler.IContext;
import com.android.imsstack.imsservice.mmtel.config.base.ConfigProxy;
import com.android.imsstack.imsservice.mmtel.config.base.ConfigurationListener;
import com.android.imsstack.internal.imsservice.ImsServiceRegistry;
import com.android.imsstack.util.ImsLog;

public final class ImsConfigImpl extends ImsConfigImplBase {
    private static final boolean DBG = ImsLog.isDebuggable();

    private final ConfigProxy mConfigProxy;
    private final int mSlotId;

    public ImsConfigImpl(IContext context) {
        super();

        mSlotId = context.getSlotId();
        mConfigProxy = new ConfigProxy(context,this);
        ImsServiceRegistry.getInstance(mSlotId).setImsConfig(this);

        init();
    }

    public void dispose() {
        log("dispose");

        ImsServiceRegistry.getInstance(mSlotId).setImsConfig(null);
        clear();
    }

    public void init() {
        mConfigProxy.init();
    }

    public void clear() {
        mConfigProxy.clear();
    }

    public void addListener(ConfigurationListener listener) {
        mConfigProxy.addListener(listener);
    }

    public void removeListener(ConfigurationListener listener) {
        mConfigProxy.removeListener(listener);
    }

    @Override
    public int getConfigInt(int item) {
        if (DBG) {
            log("getConfigInt :: item=" + item);
        }

        return mConfigProxy.getValueInt(item);
    }

    @Override
    public String getConfigString(int item) {
        if (DBG) {
            log("getConfigString :: item=" + item);
        }

        return mConfigProxy.getValueString(item);
    }

    @Override
    public int setConfig(int item, int value) {
        if (DBG) {
            log("setConfig :: item=" + item + ", value=" + value);
        }

        if (mConfigProxy.setValueInt(item, value)) {
            return CONFIG_RESULT_SUCCESS;
        }

        return CONFIG_RESULT_FAILED;
    }

    @Override
    public int setConfig(int item, String value) {
        if (DBG) {
            log("setConfig :: item=" + item + ", value=" + value);
        }

        if (mConfigProxy.setValueString(item, value)) {
            return CONFIG_RESULT_SUCCESS;
        }

        return CONFIG_RESULT_FAILED;
    }

    /**
     * @hide
     */
    public void updateImsCarrierConfigs(PersistableBundle bundle) {
        mConfigProxy.updateCarrierConfigData(bundle);
    }

    /**
     * Default messaging application parameters are sent to the ACS client
     * using this interface.
     * @param rcc RCS client configuration {@link RcsClientConfiguration}
     */
    public void setRcsClientConfiguration(RcsClientConfiguration rcc) {
        // Base Implementation - Should be overridden : Support from Android U, not required for T
    }

    /**
     * Reconfiguration triggered by the RCS application. Most likely cause
     * is the 403 forbidden to a SIP/HTTP request
     */
    public void triggerAutoConfiguration() {
        // Base Implementation - Should be overridden : Support from Android U, not required for T
    }

    /**
     * The framework has received an RCS autoconfiguration XML file for provisioning.
     *
     * @param config The XML file to be read, if not compressed, it should be in ASCII/UTF8 format.
     * @param isCompressed The XML file is compressed in gzip format and must be decompressed
     *         before being read.
     *
     */
    public void notifyRcsAutoConfigurationReceived(byte[] config, boolean isCompressed) {
        //Support from Android U, not required for T
    }

    /**
     * The RCS autoconfiguration XML file is removed or invalid.
     */
    public void notifyRcsAutoConfigurationRemoved() {
        //Support from Android U, not required for T
    }

    private static void log(String s) {
        ImsLog.d("[ISIL] " + s);
    }
}
