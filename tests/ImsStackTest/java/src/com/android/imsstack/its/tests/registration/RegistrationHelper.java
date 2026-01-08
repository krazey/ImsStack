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

import static android.telephony.ims.stub.ImsRegistrationImplBase.REGISTRATION_TECH_IWLAN;
import static android.telephony.ims.stub.ImsRegistrationImplBase.REGISTRATION_TECH_NR;

import android.location.LocationManager;
import android.os.PersistableBundle;
import android.telephony.ServiceState;
import android.telephony.TelephonyManager;
import android.telephony.ims.feature.CapabilityChangeRequest;
import android.telephony.ims.stub.ImsRegistrationImplBase;

import androidx.annotation.NonNull;

import com.android.imsstack.its.base.CarrierConfigManagerProxyImpl;
import com.android.imsstack.its.base.ConnectivityManagerProxyImpl;
import com.android.imsstack.its.base.ImsMmTelManagerProxyImpl;
import com.android.imsstack.its.base.LocationManagerProxyImpl;
import com.android.imsstack.its.base.ServiceStateBuilder;
import com.android.imsstack.its.base.SmsManagerProxyImpl;
import com.android.imsstack.its.base.SubscriptionManagerProxyImpl;
import com.android.imsstack.its.base.SystemProxyResolver;
import com.android.imsstack.its.base.TelephonyManagerProxyImpl;
import com.android.imsstack.its.imsservice.ImsServiceConnector;
import com.android.imsstack.its.imsservice.reg.ImsRegistrationWrapper;
import com.android.imsstack.its.tests.ImsStackTestBase;
import com.android.imsstack.its.util.SingleLatch;

import java.util.Objects;

public class RegistrationHelper {

    private final SingleLatch mEventLatch = new SingleLatch(
            RegistrationHelper.class.getSimpleName());

    /**
     * Performs IMS registration based on the provided {@link ImsStackTestBase} instance.
     * This method calls the overloaded method using a default {@link RegistrationInfo} object
     * created with {@code new RegistrationInfo.Builder().withDefaultConfig().build()}.
     *
     * @param testBase The {@link ImsStackTestBase} instance to perform registration.
     *                 Must not be null.
     * @return {@code true} if registration is successful, {@code false} otherwise.
     * @throws NullPointerException if {@code testBase} is null.
     */
    public boolean performRegistration(@NonNull ImsStackTestBase testBase) {
        Objects.requireNonNull(testBase, "testBase must not be null.");
        return performRegistration(testBase,
                new RegistrationInfo.Builder().withDefaultConfig().build());
    }

    /**
     * Performs IMS registration using the provided {@link ImsStackTestBase} instance and
     * registration information. If the {@code info} parameter is {@code null}, a default
     * registration information will be used.
     *
     * @param testBase The {@link ImsStackTestBase} instance to perform registration.
     *                 Must not be {@code null}.
     * @param info The {@link RegistrationInfo} object containing IMS registration information.
     *             Must not be {@code null}.
     * @return {@code true} if registration is successful, {@code false} otherwise.
     * @throws NullPointerException if {@code testBase} is {@code null}.
     */
    public boolean performRegistration(@NonNull ImsStackTestBase testBase,
            @NonNull RegistrationInfo info) {
        Objects.requireNonNull(testBase, "testBase must not be null.");
        Objects.requireNonNull(info, "info must not be null.");

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

        triggerRegistration(testBase,
                new RegistrationInfo.Builder().withDefaultConfig().build());
    }

    /**
     * Triggers IMS registration based on the provided {@link ImsStackTestBase} instance and
     * a specific carrier configuration {@link PersistableBundle}.
     * This is a convenience method that internally builds a {@link RegistrationInfo} object
     * with the provided configuration using {@link RegistrationInfo.Builder} and delegates
     * the execution to {@link #triggerRegistration(ImsStackTestBase, RegistrationInfo)}.
     *
     * @param testBase The {@link ImsStackTestBase} instance to perform registration.
     *                 Must not be {@code null}.
     * @param config The {@link PersistableBundle} containing specific carrier configuration
     *               to be applied during registration. Must not be {@code null}.
     * @throws NullPointerException if {@code testBase} or {@code config} is {@code null}.
     * @see #triggerRegistration(ImsStackTestBase, RegistrationInfo)
     */
    public void triggerRegistration(@NonNull ImsStackTestBase testBase,
            @NonNull PersistableBundle config) {
        Objects.requireNonNull(testBase, "testBase must not be null.");
        Objects.requireNonNull(config, "config must not be null.");

        triggerRegistration(testBase,
                new RegistrationInfo.Builder().withDefaultConfig().addConfig(config).build());
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
     *             Must not be null.
     * @throws NullPointerException if {@code testBase} is null.
     */
    public void triggerRegistration(@NonNull ImsStackTestBase testBase,
            @NonNull RegistrationInfo info) {
        Objects.requireNonNull(testBase, "testBase must not be null.");
        Objects.requireNonNull(info, "info must not be null.");

        startImsStackWithRegistrationInfo(testBase, info);

        if (info.isCapabilityRequestChanged()) {
            performMmTelCapabilityChange(info.getCapabilityRequest());
        } else {
            testBase.enableAllMmTelCapabilities();
        }

        mEventLatch.sleep(SingleLatch.SHORT_SLEEP_MS);
        getConnectivityManagerProxy().notifyNetworkAvailable(info.getNetworkCapability());

        notifyDataConnected(testBase, info);
    }

    /**
     * Notifies the system of a successful precise data connection state change (DATA_CONNECTED)
     * for the expected IMS registration technology specified in the {@link RegistrationInfo}.
     * If the expected registration technology is not explicitly supported (NR, or IWLAN),
     * it defaults to notifying for LTE technology.
     *
     * @param testBase The {@link ImsStackTestBase} instance to perform registration.
     *                 Must not be null.
     * @param info The {@link RegistrationInfo} containing the expected registration technology.
     *             Must not be null.
     * @see TelephonyManager#DATA_CONNECTED
     * @see ImsRegistrationImplBase#REGISTRATION_TECH_LTE
     * @see ImsRegistrationImplBase#REGISTRATION_TECH_NR
     * @see ImsRegistrationImplBase#REGISTRATION_TECH_IWLAN
     */
    public void notifyDataConnected(@NonNull ImsStackTestBase testBase,
            @NonNull RegistrationInfo info) {
        Objects.requireNonNull(testBase, "testBase must not be null.");
        Objects.requireNonNull(info, "info must not be null.");

        final int expectedTech = info.getExpectedRegTech();
        if (expectedTech == REGISTRATION_TECH_NR) {
            testBase.notifyPreciseDataConnectionState(testBase.getNrPreciseDataConnectionState(
                    TelephonyManager.DATA_CONNECTED), info.getSlotId());
        } else if (expectedTech == REGISTRATION_TECH_IWLAN) {
            testBase.notifyPreciseDataConnectionState(testBase.getIwlanPreciseDataConnectionState(
                    TelephonyManager.DATA_CONNECTED), info.getSlotId());
        } else {
            testBase.notifyPreciseDataConnectionState(testBase.getLtePreciseDataConnectionState(
                    TelephonyManager.DATA_CONNECTED), info.getSlotId());
        }
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

        initSystemProxiesWithRegistrationInfo(testBase, info);

        triggerInServiceWithServiceState(slotId, info.getServiceState());

        int subId = getSubId(slotId);
        TelephonyManagerProxyImpl telephony = getTelephonyManagerProxy(subId);

        if (testBase.isEnablerStoppable()) {
            testBase.updateCarrierConfig(slotId, subId);
        }
        testBase.startEnabler(slotId);

        getCarrierConfigManagerProxy().notifyCarrierConfigChanged(
                slotId, subId, telephony.getSimCarrierId(), telephony.getSimSpecificCarrierId());
    }

    /**
     * Initializes the default values of the system proxies.
     *
     * @param testBase The {@link ImsStackTestBase} instance.
     * @param info The {@link RegistrationInfo} object containing IMS registration information.
     * @throws NullPointerException if {@code testBase} or {@code info} is null.
     */
    public final void initSystemProxiesWithRegistrationInfo(@NonNull ImsStackTestBase testBase,
            @NonNull RegistrationInfo info) {
        Objects.requireNonNull(testBase, "testBase must not be null.");
        Objects.requireNonNull(info, "info must not be null.");

        int subId = getSubId(info.getSlotId());
        ImsMmTelManagerProxyImpl imsMmTel = getImsMmTelManagerProxy(subId);
        if (imsMmTel != null) {
            imsMmTel.setDefaultValues();
        }

        LocationManagerProxyImpl location = getLocationManagerProxy();
        location.setProviderEnablement(LocationManager.FUSED_PROVIDER, true);
        location.setProviderEnablement(LocationManager.GPS_PROVIDER, true);
        location.setProviderEnablement(LocationManager.NETWORK_PROVIDER, true);

        getSmsManagerProxy(subId).setDefaultValues();

        getSubscriptionManagerProxy().setDefaultValues();

        setTelephonyValues(info);

        TelephonyManagerProxyImpl telephony = getTelephonyManagerProxy(subId);

        telephony.setSimApplicationState(info.getSimApplicationState());
        testBase.broadcastSimApplicationStateChanged(info.getSlotId(), subId,
                info.getSimApplicationState());

        if (info.getSimCarrierId() != -1) {
            telephony.setSimCarrierId(info.getSimCarrierId());
        }
    }

    private void setTelephonyValues(@NonNull RegistrationInfo info) {
        TelephonyManagerProxyImpl telephony = getTelephonyManagerProxy(getSubId(info.getSlotId()));

        telephony.setActiveModemCount(1);
        telephony.setSupportedModemCount(2);
        telephony.clearEmergencyNumbers();

        telephony.clearUsimApplication();
        telephony.clearIsimApplication();

        switch (info.getSimSupportMode()) {
            case BOTH_ISIM_USIM -> {
                telephony.initUsimApplication();
                telephony.initIsimApplication();
            }
            case ONLY_USIM -> {
                telephony.initUsimApplication();
            }
            case INCOMP_ISIM_USIM -> {
                telephony.initUsimApplication();
                telephony.initIsimApplication();
                telephony.clearIsimRecords();
            }
            default -> { }
        }

        telephony.initNetworkInfo();
    }

    /**
     * Triggers the IMS stack into service with the specified {@link ServiceState}.
     * If the provided {@code serviceState} is null, a default {@link ServiceState} will be created
     * with LTE CS and LTE PS network registration information.
     *
     * @param slotId          The slot ID.
     * @param serviceState    The {@link ServiceState} to set.
     */
    public void triggerInServiceWithServiceState(int slotId, ServiceState serviceState) {
        if (serviceState == null) {
            serviceState = new ServiceStateBuilder()
                    .addNetworkRegistrationInfoForLteCs()
                    .addNetworkRegistrationInfoForLtePs()
                    .build();
        }

        TelephonyManagerProxyImpl telephony = getTelephonyManagerProxy(getSubId(slotId));
        telephony.setServiceState(serviceState);
        telephony.notifyServiceStateChanged(serviceState);
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

    private static LocationManagerProxyImpl getLocationManagerProxy() {
        return SystemProxyResolver.getLocationManagerProxy();
    }

    protected final SmsManagerProxyImpl getSmsManagerProxy(int subId) {
        return (SmsManagerProxyImpl) SystemProxyResolver.getSmsManagerProxy()
                .createForSubscriptionId(subId);
    }

    private static TelephonyManagerProxyImpl getTelephonyManagerProxy(int subId) {
        return (TelephonyManagerProxyImpl) SystemProxyResolver.getTelephonyManagerProxy()
                .createForSubscriptionId(subId);
    }

    private static ImsMmTelManagerProxyImpl getImsMmTelManagerProxy(int subId) {
        return (ImsMmTelManagerProxyImpl) SystemProxyResolver.getImsManagerProxy()
                .getImsMmTelManagerProxy(subId);
    }

    private static ImsServiceConnector getImsServiceConnector() {
        return ImsServiceConnector.getInstance();
    }
}
