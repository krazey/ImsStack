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
package com.android.imsstack.its.tests.registration.tests;

import android.os.PersistableBundle;
import android.telephony.AccessNetworkConstants;
import android.telephony.Annotation;
import android.telephony.CarrierConfigManager;
import android.telephony.PreciseDataConnectionState;
import android.telephony.TelephonyManager;
import android.telephony.TelephonyManager.SimState;
import android.telephony.data.ApnSetting;

import androidx.annotation.NonNull;
import androidx.annotation.Nullable;

import com.android.imsstack.base.MSimUtils;
import com.android.imsstack.base.TelephonyManagerProxy;
import com.android.imsstack.its.base.TelephonyManagerProxyImpl;
import com.android.imsstack.its.base.TestConstants;
import com.android.imsstack.its.servercontrol.BasicScenarioTemplates;
import com.android.imsstack.its.servercontrol.ControlConnection;
import com.android.imsstack.its.servercontrol.ScenarioGeneratorUtils;
import com.android.imsstack.its.servercontrol.ServerFailureHandler;
import com.android.imsstack.its.tests.ImsStackTestBase;
import com.android.imsstack.its.tests.registration.RegistrationHelper;
import com.android.imsstack.its.tests.registration.RegistrationInfo;
import com.android.imsstack.its.tests.registration.util.TestRegistration;
import com.android.imsstack.its.util.SingleLatch;

import java.util.Objects;

/**
 * Base class for Registration ImsStack tests.
 * Provides common test fixtures (like APN settings), configuration helpers,
 * and utility methods required by most registration test scenarios.
 */
public class RegistrationTestBase extends ImsStackTestBase {
    protected final SingleLatch mEventLatch = new SingleLatch(
            RegistrationTestBase.class.getSimpleName());
    protected static final int REG_ERROR_CODE_403 = 403;
    protected static final int REG_ERROR_CODE_404 = 404;
    protected static final int REG_ERROR_CODE_5XX = 5;
    protected static final int REG_ERROR_CODE_503 = 503;
    protected static final int REG_ERROR_CODE_504 = 504;

    protected static final int ERROR_TYPE_REPEATED = 1;
    protected static final int ERROR_TYPE_CRITICAL = 2;

    protected ControlConnection mServerControlConnection;
    protected TestRegistration mRegistration;
    protected RegistrationHelper mRegistrationHelper;
    protected PersistableBundle mConfig;
    protected RegistrationInfo.Builder mInfoBuilder;

    /**
     * A constant, immutable {@link ApnSetting} configured for IMS (IPv6) over cellular.
     * This test fixture ensures all tests use a consistent, valid IMS APN profile.
     */
    protected final ApnSetting mTestImsApn = new ApnSetting.Builder()
            .setApnTypeBitmask(ApnSetting.TYPE_IMS)
            .setApnName("TestApnName")
            .setEntryName("TestEntryName")
            .setProtocol(ApnSetting.PROTOCOL_IPV6)
            .setRoamingProtocol(ApnSetting.PROTOCOL_IPV6)
            .build();

    /**
     * Initializes the {@link #mServerControlConnection} using the primary P-CSCF address.
     *
     * @param serverFailureHandler The handler to notify if the server connection failures.
     */
    protected void createControlConnection(ServerFailureHandler serverFailureHandler) {
        mServerControlConnection =
                new ControlConnection(serverFailureHandler, getPcscfAddresses().get(0));
    }

    /**
     * Initializes {@link #mConfig} with base carrier configurations common to most tests,
     * such as disabling SIP over IPSec. Tests should call this and then add or override
     * specific configurations as needed.
     */
    protected void setRegistrationBaseConfig() {
        mConfig = new PersistableBundle();
        mConfig.putBoolean(CarrierConfigManager.Ims.KEY_SIP_OVER_IPSEC_ENABLED_BOOL, false);
    }

    /**
     * Configures the test SIP server to expect and respond to a typical registration
     * and subscription flow.
     */
    protected void setDefaultRegistrationScenario() {
        ScenarioGeneratorUtils generator = new ScenarioGeneratorUtils();
        generator.addMessages(BasicScenarioTemplates.NORMAL_REGISTRATION_W_SUBSCRIPTION);
        mServerControlConnection.sendControlCommand(generator.build().toString());
    }

    /**
     * Notifies the {@link TelephonyManagerProxy} of a precise data connection state change.
     * If the {@link RegistrationInfo} is provided, the notification will be associated with
     * that specific slot. If it is {@code null}, it will default to using
     * {@link TestConstants#SLOT0}.
     *
     * @param pdcs The precise data connection state to notify.
     * @param info The registration info containing the slot ID, or {@code null} to use the default
     * slot.
     */
    protected void notifyPreciseDataConnectionState(
            @NonNull PreciseDataConnectionState pdcs, @Nullable RegistrationInfo info) {
        Objects.requireNonNull(pdcs, "pdcs must not be null.");

        int slotId = (info != null) ? info.getSlotId() : TestConstants.SLOT0;
        TelephonyManagerProxyImpl telephony = getTelephonyManagerProxy(getSubId(slotId));
        telephony.notifyPreciseDataConnectionStateChanged(pdcs);
    }

    /**
     * Factory method to build a {@link PreciseDataConnectionState} object for IWLAN
     * using the common {@link #mTestImsApn} and the specified data state.
     *
     * @param state The desired data state (e.g., {@link TelephonyManager#DATA_CONNECTED}
     * or {@link TelephonyManager#DATA_DISCONNECTED}).
     * @return A new {@link PreciseDataConnectionState} object configured for IWLAN.
     */
    protected PreciseDataConnectionState getIwlanPreciseDataConnectionState(
            @Annotation.DataState int state) {
        return new PreciseDataConnectionState.Builder()
                .setNetworkType(TelephonyManager.NETWORK_TYPE_IWLAN)
                .setTransportType(AccessNetworkConstants.TRANSPORT_TYPE_WLAN)
                .setApnSetting(mTestImsApn)
                .setState(state)
                .build();
    }

    /**
     * Simulates a SIM state change and broadcasts it to the system.
     * This method provides a comprehensive way to test SIM state changes. It mimics a real
     * event by first internally setting the application state for the target subscription and then
     * dispatching a system-wide broadcast to notify all listeners.
     * After triggering the state change, the method pauses for a predefined duration
     * ({@code SingleLatch.LONG_SLEEP_MS}). This delay is crucial for allowing asynchronous
     * teardown and cleanup procedures within the ImsStack to complete, ensuring the system reaches
     * a stable state before the test proceeds to its next steps or concludes.
     *
     * @param info The registration information containing the target slot ID. If {@code null},
     * it defaults to {@link TestConstants#SLOT0}.
     * @param simState The SIM state to be broadcast (e.g.,
     * {@link TelephonyManager#SIM_STATE_ABSENT}).
     */
    protected void simulateSimStateChange(@Nullable RegistrationInfo info,
            @SimState int simState) {
        int subId = getSubId((info != null) ? info.getSlotId() : TestConstants.SLOT0);
        TelephonyManagerProxyImpl telephony = getTelephonyManagerProxy(subId);
        telephony.setSimApplicationState(simState);
        broadcastSimApplicationStateChanged(
                TestConstants.SLOT0, MSimUtils.INVALID_SUB_ID, simState);

        mEventLatch.sleep(SingleLatch.LONG_SLEEP_MS);
    }
}
