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

import android.telephony.ServiceState;
import android.telephony.ims.feature.CapabilityChangeRequest;

import androidx.annotation.NonNull;
import androidx.annotation.Nullable;

import com.android.imsstack.its.base.CarrierConfigManagerProxyImpl;
import com.android.imsstack.its.base.ConnectivityManagerProxyImpl;
import com.android.imsstack.its.base.ServiceStateBuilder;
import com.android.imsstack.its.base.SubscriptionManagerProxyImpl;
import com.android.imsstack.its.base.SystemProxyResolver;
import com.android.imsstack.its.base.TelephonyManagerProxyImpl;
import com.android.imsstack.its.imsservice.ImsServiceConnector;
import com.android.imsstack.its.imsservice.reg.ImsRegistrationWrapper;
import com.android.imsstack.its.tests.ImsStackTestBase;
import com.android.imsstack.its.util.SingleLatch;

import java.util.Objects;

public class RegistrationHelper {

    private final SingleLatch mEventLatch = new SingleLatch("RegistrationHelper");

    /**
     * Performs IMS registration based on the provided {@link ImsStackTestBase} instance.
     *
     * @param testBase The {@link ImsStackTestBase} instance to perform registration.
     *                 Must not be null.
     * @return {@code true} if registration is successful, {@code false} otherwise.
     * @throws NullPointerException if {@code testBase} is null.
     */
    public boolean performRegistration(@NonNull ImsStackTestBase testBase) {
        Objects.requireNonNull(testBase, "testBase must not be null.");
        return performRegistration(testBase, null);
    }

    /**
     * Performs IMS registration using the provided {@link ImsStackTestBase} instance and
     * registration information. If the {@code info} parameter is {@code null}, a default
     * registration information will be used.
     *
     * @param testBase The {@link ImsStackTestBase} instance to perform registration.
     *                 Must not be {@code null}.
     * @param info The {@link RegistrationInfo} object containing IMS registration information.
     *             If {@code null}, a default registration information will be used.
     * @return {@code true} if registration is successful, {@code false} otherwise.
     * @throws NullPointerException if {@code testBase} is {@code null}.
     */
    public boolean performRegistration(@NonNull ImsStackTestBase testBase,
            @Nullable RegistrationInfo info) {
        Objects.requireNonNull(testBase, "testBase must not be null.");

        triggerRegistration(testBase, info);

        ImsRegistrationWrapper imsRegistration = getImsServiceConnector().getRegistration();
        imsRegistration.waitForRegistered();

        return imsRegistration.isRegistered();
    }

    /**
     * Triggers IMS registration using the provided {@link ImsStackTestBase} instance.
     *
     * @param testBase The {@link ImsStackTestBase} instance to perform registration.
     *                 Must not be {@code null}.
     * @throws NullPointerException if {@code testBase} is null.
     */
    public void triggerRegistration(@NonNull ImsStackTestBase testBase) {
        Objects.requireNonNull(testBase, "testBase must not be null.");

        triggerRegistration(testBase, null);
    }

    /**
     * Triggers IMS registration based on the provided {@link ImsStackTestBase} instance and the
     * {@link RegistrationInfo} object containing IMS registration information.
     * If the {@code info} parameter is null, a default {@link RegistrationInfo} object will be
     * created using {@link RegistrationInfo.Builder}.
     * This method starts the IMS stack and performs necessary configuration changes
     * such as enabling IMS capabilities and notifying the network availability.
     *
     * @param testBase The {@link ImsStackTestBase} instance to perform registration.
     *                 Must not be null.
     * @param info The {@link RegistrationInfo} object containing IMS registration information.
     *             If null, a default registration information will be used.
     * @throws NullPointerException if {@code testBase} is null.
     */
    public void triggerRegistration(@NonNull ImsStackTestBase testBase,
            @Nullable RegistrationInfo info) {
        Objects.requireNonNull(testBase, "testBase must not be null.");

        if (info == null) {
            info = new RegistrationInfo.Builder().build();
        }

        if (info.isServiceStateChanged()) {
            startImsStackWithRegistrationInfo(testBase, info);
        } else {
            testBase.startImsStack(info.getSlotId(), info.getConfig());
        }

        if (info.isCapabilityRequestChanged()) {
            performMmTelCapabilityChange(info.getCapabilityRequest());
        } else {
            testBase.enableAllMmTelCapabilities();
        }

        mEventLatch.sleep(SingleLatch.SHORT_SLEEP_MS);
        getConnectivityManagerProxy().notifyNetworkAvailable(info.getNetworkCapability());
    }

    /**
     * Starts the IMS stack for the specified slot with the provided registration information.
     * If the registration information includes a configuration, it will be set using
     * the {@code ImsStackTestBase#writeTestConfig} method.
     * The system proxies will be initialized for the specified slot,
     * and the IMS stack will be triggered into service. If a service state is provided in the
     * registration information, the stack will be triggered into service with that specific state.
     *
     * @param testBase The {@link ImsStackTestBase} instance.
     * @param info     The {@link RegistrationInfo} object containing IMS registration information.
     * @throws NullPointerException if {@code testBase} or {@code info} is null.
     */
    public void startImsStackWithRegistrationInfo(@NonNull ImsStackTestBase testBase,
            @NonNull RegistrationInfo info) {
        Objects.requireNonNull(testBase, "testBase must not be null.");
        Objects.requireNonNull(info, "info must not be null.");

        int slotId = info.getSlotId();
        testBase.writeTestConfig(slotId, info.getConfig());
        testBase.initSystemProxies(slotId, info.getSimApplicationState());

        if (info.isServiceStateChanged()) {
            triggerInServiceWithServiceState(slotId, info.getServiceState());
        } else {
            testBase.triggerInService(slotId);
        }

        int subId = getSubId(slotId);
        TelephonyManagerProxyImpl telephony = getTelephonyManagerProxy(subId);
        int carrierId = telephony.getSimCarrierId();
        int specificCarrierId = telephony.getSimSpecificCarrierId();

        getCarrierConfigManagerProxy().notifyCarrierConfigChanged(
                slotId, subId, carrierId, specificCarrierId);
    }

    /**
     * Triggers the IMS stack into service with the specified {@link ServiceState}.
     *
     * @param slotId          The slot ID.
     * @param serviceState    The {@link ServiceState} to set.
     * @throws NullPointerException if {@code testBase} or {@code serviceState} is null.
     */
    public void triggerInServiceWithServiceState(int slotId, @NonNull ServiceState serviceState) {
        Objects.requireNonNull(serviceState, "serviceState must not be null.");

        TelephonyManagerProxyImpl telephony = getTelephonyManagerProxy(getSubId(slotId));
        telephony.setServiceState(serviceState);
        telephony.notifyServiceStateChanged(serviceState);
    }

    /**
     * Triggers the IMS stack into service with NR network.
     *
     * @param slotId The slot ID.
     */
    public void triggerInServiceWithNr(int slotId) {
        ServiceState ss = new ServiceStateBuilder()
                .addNetworkRegistrationInfoForNrCs()
                .addNetworkRegistrationInfoForNr()
                .build();
        triggerInServiceWithServiceState(slotId, ss);
    }

    /**
     * Triggers the IMS stack into service with emergency-only network.
     *
     * @param slotId The slot ID.
     */
    public void triggerInServiceWithEmergencyOnly(int slotId) {
        ServiceState ss = new ServiceStateBuilder()
                .addNetworkRegistrationInfoForEmergencyOnly()
                .build();
        triggerInServiceWithServiceState(slotId, ss);
    }

    /**
     * Performs a change in the MmTel capability based on the provided request.
     *
     * @param request The capability change request. Must not be null.
     * @throws NullPointerException if the {@code request} parameter is null.
     */
    public void performMmTelCapabilityChange(@NonNull CapabilityChangeRequest request) {
        Objects.requireNonNull(request, "request must not be null.");

        getImsServiceConnector().getMmTelFeature().changeCapabilitiesConfiguration(request, null);
    }

    private static int getSubId(int slotId) {
        return getSubscriptionManagerProxy().getSubscriptionId(slotId);
    }

    private static SubscriptionManagerProxyImpl getSubscriptionManagerProxy() {
        return SystemProxyResolver.getSubscriptionManagerProxy();
    }

    private static CarrierConfigManagerProxyImpl getCarrierConfigManagerProxy() {
        return SystemProxyResolver.getCarrierConfigManagerProxy();
    }

    private static ConnectivityManagerProxyImpl getConnectivityManagerProxy() {
        return SystemProxyResolver.getConnectivityManagerProxy();
    }

    private static TelephonyManagerProxyImpl getTelephonyManagerProxy(int subId) {
        return (TelephonyManagerProxyImpl) SystemProxyResolver.getTelephonyManagerProxy()
                .createForSubscriptionId(subId);
    }

    private static ImsServiceConnector getImsServiceConnector() {
        return ImsServiceConnector.getInstance();
    }
}
