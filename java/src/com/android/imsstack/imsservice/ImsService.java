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

package com.android.imsstack.imsservice;

import android.annotation.Nullable;
import android.telephony.ims.feature.ImsFeature;
import android.telephony.ims.feature.MmTelFeature;
import android.telephony.ims.feature.RcsFeature;
import android.telephony.ims.stub.ImsConfigImplBase;
import android.telephony.ims.stub.ImsFeatureConfiguration;
import android.telephony.ims.stub.ImsRegistrationImplBase;
import android.telephony.ims.stub.SipTransportImplBase;

import com.android.imsstack.base.DeviceConfig;
import com.android.imsstack.imsservice.mmtel.ImsServiceManager;
import com.android.imsstack.imsservice.mmtel.ImsServiceRecord;
import com.android.imsstack.util.IndentingPrintWriter;
import com.android.imsstack.util.Log;
import com.android.internal.annotations.VisibleForTesting;

import java.io.FileDescriptor;
import java.io.PrintWriter;
import java.util.concurrent.Executor;

/**
 * Implements ImsService to provide VoLTE/Emergency/RCS features.
 */
public class ImsService extends android.telephony.ims.ImsService {

    @Override
    public void onCreate() {
        super.onCreate();

        logi("ImsService created");

        ImsServiceController.create(getApplicationContext());
    }

    @Override
    public void onDestroy() {
        logi("ImsService destroy...");

        ImsServiceController.destroy(getApplicationContext());

        super.onDestroy();
    }

    /** This method is added to read ImsController is ready or not
     *  Override this method to get ImsController ready state in testing class
     *  @return  True if ImsController is ready otherwise false
     */
    @VisibleForTesting
    protected boolean isImsControllerReady() {
        return ImsServiceController.isReady();
    }

    @Override
    public ImsFeatureConfiguration querySupportedImsFeatures() {
        if (!isImsControllerReady()) {
            logi("querySupportedImsFeatures :: not-ready");
            return super.querySupportedImsFeatures();
        }

        logi("querySupportedImsFeatures");

        // It will return the supported features by this ImsService.
        // Generally, the features are the same as defined in AndroidManifest.xml.
        ImsFeatureConfiguration.Builder fcBuilder = new ImsFeatureConfiguration.Builder();
        int simCount = DeviceConfig.getActiveSimCount();

        for (int i = 0; i < simCount; i++) {
            fcBuilder.addFeature(i, ImsFeature.FEATURE_MMTEL);
            fcBuilder.addFeature(i, ImsFeature.FEATURE_EMERGENCY_MMTEL);
            fcBuilder.addFeature(i, ImsFeature.FEATURE_RCS);
        }

        return fcBuilder.build();
    }

    @Override
    public @ImsServiceCapability long getImsServiceCapabilities() {
        //TODO uncomment below statements for SIP Delegate Support.
        //logi("getImsServiceCapabilities:CAPABILITY_SIP_DELEGATE_CREATION");
        //return CAPABILITY_SIP_DELEGATE_CREATION;
        logi("getImsServiceCapabilities");
        return super.getImsServiceCapabilities();

        // TODO: Replace above return statement with below for Simultaneous calling support.
        // return (super.getImsServiceCapabilities() |
        //         ImsService.CAPABILITY_SUPPORTS_SIMULTANEOUS_CALLING);
    }

    @Override
    public void readyForFeatureCreation() {
        logi("readyForFeatureCreation");
    }

    @Override
    public void enableIms(int slotId) {
        if (!ImsServiceController.isReady()) {
            logi("enableIms :: not-ready, slotId=" + slotId);
            return;
        }

        logi("enableIms :: slotId=" + slotId);

        ImsServiceRecord isr = ImsServiceManager.getServiceRecord(slotId);

        if (isr != null) {
            isr.enableIms();
        }
    }

    @Override
    public void disableIms(int slotId) {
        if (!ImsServiceController.isReady()) {
            logi("disableIms :: not-ready, slotId=" + slotId);
            return;
        }

        logi("disableIms :: slotId=" + slotId);

        ImsServiceRecord isr = ImsServiceManager.getServiceRecord(slotId);

        if (isr != null) {
            isr.disableIms();
        }
    }

    @Override
    public MmTelFeature createMmTelFeature(int slotId) {
        logi("createMmTelFeature :: slotId=" + slotId);

        ImsServiceController isc = ImsServiceController.getInstance();

        if (isc == null) {
            logi("No ImsServiceController");
            return null;
        }

        return isc.getMmTelService(slotId);
    }

    @Override
    public RcsFeature createRcsFeature(int slotId) {
        logi("createRcsFeature for slot : " + slotId);

        ImsServiceController isc = ImsServiceController.getInstance();

        if (isc == null) {
            logi("No ImsServiceController");
            return null;
        }

        return isc.getRcsFeature(slotId);
    }

    @Override
    public ImsConfigImplBase getConfig(int slotId) {
        if (!ImsServiceController.isReady()) {
            logi("getConfig :: not-ready, slotId=" + slotId);
            return null;
        }

        ImsServiceRecord isr = ImsServiceManager.getServiceRecord(slotId);

        if (isr == null) {
            logi("getConfig :: Service is down for phone" + slotId);
            return null;
        }

        return isr.getConfig();
    }

    @Override
    public ImsRegistrationImplBase getRegistration(int slotId) {
        if (!ImsServiceController.isReady()) {
            logi("getRegistration :: not-ready, slotId=" + slotId);
            return null;
        }

        ImsServiceRecord isr = ImsServiceManager.getServiceRecord(slotId);

        if (isr == null) {
            logi("getRegistration :: Service is down for phone" + slotId);
            return null;
        }

        return isr.getRegistration();
    }

    @Override
    public @Nullable SipTransportImplBase getSipTransport(int slotId) {
        logi("getSipTransport :: slotId=" + slotId);

        if (!ImsServiceController.isReady()) {
            logi("getSipTransport :: not-ready, slotId=" + slotId);
            return null;
        }

        ImsServiceRecord isr = ImsServiceManager.getServiceRecord(slotId);

        if (isr == null) {
            logi("getSipTransport :: Service is down for phone" + slotId);
            return null;
        }
        return isr.getSipTransport();
    }

    private static void logi(String s) {
        Log.i(Log.TAG, "[GII-IMPL] " + s);
    }

    @Override
    public void dump(FileDescriptor fd, PrintWriter printWriter, String[] args) {
        super.dump(fd, printWriter, args);

        IndentingPrintWriter pw = new IndentingPrintWriter(printWriter, "  ");
        pw.println("--------ImsService--------");

        ImsServiceController isc = ImsServiceController.getInstance();
        if (isc != null) {
            isc.dump(pw);
        }
    }

    @Override
    public Executor getExecutor() {
        Executor executor = ImsServiceController.getInstance().getExecutor();
        return (executor != null) ? executor : Runnable::run;
    }
}
