/*
 * Copyright (C) 2021 The Android Open Source Project
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

package com.android.imsstack.imsservice.uce;

import android.os.PersistableBundle;
import android.telephony.CarrierConfigManager;
import android.telephony.ims.feature.CapabilityChangeRequest;
import android.telephony.ims.feature.ImsFeature;
import android.telephony.ims.feature.RcsFeature;
import android.telephony.ims.stub.CapabilityExchangeEventListener;
import android.telephony.ims.stub.ImsRegistrationImplBase;
import android.telephony.ims.stub.RcsCapabilityExchangeImplBase;

import androidx.annotation.NonNull;

import com.android.imsstack.base.AppContext;
import com.android.imsstack.base.SystemServiceProxy.CarrierConfigManagerProxy;
import com.android.imsstack.enabler.IContext;
import com.android.imsstack.util.ImsLog;
import com.android.imsstack.util.IndentingPrintWriter;

import java.util.concurrent.Executor;
import java.util.concurrent.Executors;

public class RcsFeatureImpl extends RcsFeature {
    private final IContext mIContext;
    private final Executor mCallbackExecutor = Executors.newSingleThreadExecutor();
    public RcsFeatureImpl(IContext iContext) {
        super(iContext.getExecutor());
        mIContext = iContext;
    }

    @Override
    public void onFeatureReady() {
        logi("RCS:onFeatureReady");
        super.onFeatureReady();
    }

    @NonNull
    @Override
    public RcsCapabilityExchangeImplBase createCapabilityExchangeImpl(
            @NonNull CapabilityExchangeEventListener listener) {
        logi("createCapabilityExchangeImpl");
        return new RcsCapExchangeImpl(listener, mIContext.getSlotId(), mIContext.getContext(),
                mIContext.getExecutor(), mCallbackExecutor);
    }

    @Override
    public void destroyCapabilityExchangeImpl(
             RcsCapabilityExchangeImplBase capExchangeImpl) {
        logi("destroyCapabilityExchangeImpl");
        super.destroyCapabilityExchangeImpl(capExchangeImpl);
    }

    @Override
    public void onFeatureRemoved() {
         logi("RCS:onFeatureRemoved");
         super.onFeatureRemoved();
    }

    @Override
    public boolean queryCapabilityConfiguration(int capability, int radioTech) {
        logi("queryCapabilityConfiguration");
        if (radioTech == ImsRegistrationImplBase.REGISTRATION_TECH_NONE) {
            return false;
        }
        //TODO add check for IWLAN if supported
        switch (capability) {
            case RcsImsCapabilities.CAPABILITY_TYPE_PRESENCE_UCE:
                logi("queryCapabilityConfiguration presence" + radioTech + "capability:"
                        + capability);
                return radioTech == ImsRegistrationImplBase.REGISTRATION_TECH_LTE;
            case RcsImsCapabilities.CAPABILITY_TYPE_OPTIONS_UCE:
                logi("queryCapabilityConfiguration option" + radioTech + "capability:"
                        + capability);
                return radioTech == ImsRegistrationImplBase.REGISTRATION_TECH_LTE;
            default:
                logi("Default case radioTech" + radioTech + "capability:" + capability);
                return false;
        }
    }

    @Override
    public void changeEnabledCapabilities(CapabilityChangeRequest request,
             CapabilityCallbackProxy callback) {
        logi("changeEnabledCapabilities");
        if (request == null) {
            logi("changeEnabledCapabilities :: request is null ");
            return;
        }
        int capabilities = getCapabilities();
        for (CapabilityChangeRequest.CapabilityPair pair : request.getCapabilitiesToEnable()) {
            switch (pair.getRadioTech()) {
                case ImsRegistrationImplBase.REGISTRATION_TECH_LTE:
                    notifyCapabilitiesStatusChanged(new RcsImsCapabilities(capabilities));
                    logi("notifyCapabilitiesStatusChanged for radio LTE");
                    return;
                case ImsRegistrationImplBase.REGISTRATION_TECH_IWLAN:
                case ImsRegistrationImplBase.REGISTRATION_TECH_NR:
                    //TODO add capability changes for IWLAN if supported
                    logi("RadioTech" + pair.getRadioTech());
                    return;
                default:
                    logi("Default case RadioTech" + pair.getRadioTech());
            }
        }

        if (callback == null) {
            logi("changeEnabledCapabilities :: callback is null");
            return;
        }

        for (CapabilityChangeRequest.CapabilityPair pair :request.getCapabilitiesToDisable())
        {
            callback.onChangeCapabilityConfigurationError(pair.getCapability(),
                    pair.getRadioTech(), ImsFeature.CAPABILITY_ERROR_GENERIC);
            logi("onChangeCapabilityConfigurationError error  disable is sent");
        }
    }

    /**
     * Starts the feature and notifies that it is ready.
     */
    public void start() {
        logi("start");
        setFeatureState(ImsFeature.STATE_READY);
    }

    /**
     * Dump this instance into a readable format for dumpsys usage.
     */
    public void dump(IndentingPrintWriter pw) {
        pw.println("RcsFeature:");
        pw.increaseIndent();
        pw.println("featureState=" + getFeatureState());
        pw.decreaseIndent();
    }

    private int getCapabilities() {
        CarrierConfigManagerProxy ccmp =
                AppContext.getInstance().getSystemServiceProxy(CarrierConfigManagerProxy.class);
        PersistableBundle b = ccmp.getConfigForSubId(mIContext.getSubId(),
                CarrierConfigManager.Ims.KEY_ENABLE_PRESENCE_CAPABILITY_EXCHANGE_BOOL,
                CarrierConfigManager.KEY_USE_RCS_SIP_OPTIONS_BOOL);
        int capabilities = RcsImsCapabilities.CAPABILITY_TYPE_NONE;
        boolean isPresenceEnabled =
                b.getBoolean(CarrierConfigManager.Ims.KEY_ENABLE_PRESENCE_CAPABILITY_EXCHANGE_BOOL);
        if (isPresenceEnabled) {
            logi("getCapabilities presence");
            capabilities = RcsImsCapabilities.CAPABILITY_TYPE_PRESENCE_UCE;
            return capabilities;
        }
        boolean isCapOptionEnabled =
                b.getBoolean(CarrierConfigManager.KEY_USE_RCS_SIP_OPTIONS_BOOL);
        if (isCapOptionEnabled) {
            logi("getCapabilities Option");
            capabilities = RcsImsCapabilities.CAPABILITY_TYPE_OPTIONS_UCE;
            return capabilities;
        }
        return capabilities;
    }

    private static void logi(String s) {
        ImsLog.i("[ISIL] " + s);
    }
}
