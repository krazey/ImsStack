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
package com.android.imsstack.its.tests;

import static android.telephony.ims.feature.MmTelFeature.MmTelCapabilities.CAPABILITY_TYPE_SMS;
import static android.telephony.ims.feature.MmTelFeature.MmTelCapabilities.CAPABILITY_TYPE_UT;
import static android.telephony.ims.feature.MmTelFeature.MmTelCapabilities.CAPABILITY_TYPE_VIDEO;
import static android.telephony.ims.feature.MmTelFeature.MmTelCapabilities.CAPABILITY_TYPE_VOICE;
import static android.telephony.ims.stub.ImsRegistrationImplBase.REGISTRATION_TECH_IWLAN;
import static android.telephony.ims.stub.ImsRegistrationImplBase.REGISTRATION_TECH_LTE;
import static android.telephony.ims.stub.ImsRegistrationImplBase.REGISTRATION_TECH_NR;

import static org.junit.Assert.fail;

import android.content.Context;
import android.content.Intent;
import android.location.LocationManager;
import android.net.InetAddresses;
import android.net.NetworkCapabilities;
import android.os.PersistableBundle;
import android.os.SystemProperties;
import android.telephony.BarringInfo;
import android.telephony.DisconnectCause;
import android.telephony.PreciseCallState;
import android.telephony.PreciseDataConnectionState;
import android.telephony.PreciseDisconnectCause;
import android.telephony.ServiceState;
import android.telephony.SubscriptionManager;
import android.telephony.TelephonyManager;
import android.telephony.ims.feature.CapabilityChangeRequest;

import androidx.annotation.NonNull;
import androidx.annotation.Nullable;

import com.android.imsstack.ServiceLoader;
import com.android.imsstack.base.AppContext;
import com.android.imsstack.base.ImsPrivateProperties;
import com.android.imsstack.base.MSimUtils;
import com.android.imsstack.core.agents.AgentFactory;
import com.android.imsstack.core.agents.ConfigInterface;
import com.android.imsstack.core.config.CarrierConfig;
import com.android.imsstack.core.config.ServiceCaps;
import com.android.imsstack.imsservice.ImsServiceController;
import com.android.imsstack.its.base.BroadcastReceiverProxyImpl;
import com.android.imsstack.its.base.CarrierConfigManagerProxyImpl;
import com.android.imsstack.its.base.ConnectivityManagerProxyImpl;
import com.android.imsstack.its.base.ContentProviderProxyImpl;
import com.android.imsstack.its.base.ImsManagerProxyImpl;
import com.android.imsstack.its.base.ImsMmTelManagerProxyImpl;
import com.android.imsstack.its.base.IpSecManagerProxyImpl;
import com.android.imsstack.its.base.LocationManagerProxyImpl;
import com.android.imsstack.its.base.ProvisioningManagerProxyImpl;
import com.android.imsstack.its.base.SensorManagerProxyImpl;
import com.android.imsstack.its.base.ServiceStateBuilder;
import com.android.imsstack.its.base.SmsManagerProxyImpl;
import com.android.imsstack.its.base.SubscriptionManagerProxyImpl;
import com.android.imsstack.its.base.SystemProxyResolver;
import com.android.imsstack.its.base.TelephonyManagerProxyImpl;
import com.android.imsstack.its.core.agents.WifiAgent;
import com.android.imsstack.its.imsservice.ImsServiceConnector;
import com.android.imsstack.its.imsservice.mmtel.ImsMmTelFeatureWrapper;
import com.android.imsstack.its.util.SingleLatch;
import com.android.imsstack.jni.NativeCommands;
import com.android.imsstack.util.Log;

import java.io.IOException;
import java.io.OutputStream;
import java.net.InetAddress;
import java.util.ArrayList;
import java.util.Collections;
import java.util.List;

/**
 * A test base class to provide common functionalities for each test.
 */
public class ImsStackTestBase {
    protected interface TestValueInitializer {
        void init(int slotId, int simApplicationState);
    }

    protected static final int APN_EMERGENCY = NetworkCapabilities.NET_CAPABILITY_EIMS;
    protected static final int APN_IMS = NetworkCapabilities.NET_CAPABILITY_IMS;
    protected static final int APN_DEFAULT = NetworkCapabilities.NET_CAPABILITY_INTERNET;
    protected static final int APN_XCAP = NetworkCapabilities.NET_CAPABILITY_XCAP;
    /**
     * System property to store the test P-CSCF addresses.
     * Multiple P-CSCF addresses are configured as a comma-separated list.
     */
    private static final String PROP_PCSCF_LIST = "persist.ims.test.pcscf_list";
    private static List<InetAddress> sPcscfAddresses = new ArrayList<>();

    protected final BroadcastReceiverProxyImpl mBroadcastReceiverProxy;
    protected final CarrierConfigManagerProxyImpl mCarrierConfigManagerProxy;
    protected final ConnectivityManagerProxyImpl mConnectivityManagerProxy;
    protected final ContentProviderProxyImpl mContentProviderProxy;
    protected final ImsManagerProxyImpl mImsManagerProxy;
    protected final IpSecManagerProxyImpl mIpSecManagerProxy;
    protected final LocationManagerProxyImpl mLocationManagerProxy;
    protected final SensorManagerProxyImpl mSensorManagerProxy;
    protected final SmsManagerProxyImpl mSmsManagerProxy;
    protected final SubscriptionManagerProxyImpl mSubscriptionManagerProxy;
    protected final TelephonyManagerProxyImpl mTelephonyManagerProxy;
    protected final WifiAgent mWifiAgent = WifiAgent.getInstance();
    protected final ImsServiceConnector mImsServiceConnector = ImsServiceConnector.getInstance();
    private final SingleLatch mEventLatch = new SingleLatch("ImsStackTestBase");
    private TestValueInitializer mTestValueInitializer = this::initTestValues;
    private boolean mEnablerStoppable = true;

    public ImsStackTestBase() {
        mBroadcastReceiverProxy = SystemProxyResolver.getBroadcastReceiverProxy();
        mCarrierConfigManagerProxy = SystemProxyResolver.getCarrierConfigManagerProxy();
        mConnectivityManagerProxy = SystemProxyResolver.getConnectivityManagerProxy();
        mContentProviderProxy = SystemProxyResolver.getContentProviderProxy();
        mImsManagerProxy = SystemProxyResolver.getImsManagerProxy();
        mIpSecManagerProxy = SystemProxyResolver.getIpSecManagerProxy();
        mLocationManagerProxy = SystemProxyResolver.getLocationManagerProxy();
        mSensorManagerProxy = SystemProxyResolver.getSensorManagerProxy();
        mSmsManagerProxy = SystemProxyResolver.getSmsManagerProxy();
        mSubscriptionManagerProxy = SystemProxyResolver.getSubscriptionManagerProxy();
        mTelephonyManagerProxy = SystemProxyResolver.getTelephonyManagerProxy();
    }

    /**
     * Initializes the P-CSCF addresses for test purpose.
     */
    public static void initPcscfAddresses() {
        String pcscfAddresses = SystemProperties.get(PROP_PCSCF_LIST, "");
        if (!pcscfAddresses.isEmpty()) {
            setPcscfAddresses(pcscfAddresses);
        }
    }

    /**
     * Sets the P-CSCF addresses. The multiple P-CSCF addresses can be configured with
     * the comma-separated address list.
     *
     * @param pcscfAddresses The numeric P-CSCF addresses.
     */
    public static void setPcscfAddresses(@NonNull String pcscfAddresses) {
        sPcscfAddresses.clear();
        String[] addresses = pcscfAddresses.split(",", -1);

        if (addresses != null) {
            for (String address : addresses) {
                if (!address.isEmpty()) {
                    try {
                        sPcscfAddresses.add(InetAddresses.parseNumericAddress(address));
                    } catch (IllegalArgumentException e) {
                        Log.e(ImsStackTestBase.class,
                                "Invalid InetAddress format: " + e.toString());
                    }
                }
            }
        }
    }

    /**
     * Retrieves the P-CSCF addresses.
     *
     * @return An unmodifiable list of InetAddresses representing the P-CSCF addresses.
     * @throws IllegalStateException if the P-CSCF addresses have not been initialized.
     */
    @NonNull
    public static List<InetAddress> getPcscfAddresses() {
        if (sPcscfAddresses.isEmpty()) {
            throw new IllegalStateException("P-CSCF addresses have not been initialized.");
        }

        return Collections.unmodifiableList(sPcscfAddresses);
    }

    /**
     * Initializes the default values of the system proxies.
     *
     * @param slotId The slot id.
     * @param simApplicationState The USIM application state to be configured.
     */
    public final void initSystemProxies(int slotId, int simApplicationState) {
        int subId = getSubId(slotId);
        ImsMmTelManagerProxyImpl imsMmTel = getImsMmTelManagerProxy(subId);
        if (imsMmTel != null) {
            imsMmTel.setDefaultValues();
        }

        mLocationManagerProxy.setProviderEnablement(LocationManager.FUSED_PROVIDER, true);
        mLocationManagerProxy.setProviderEnablement(LocationManager.GPS_PROVIDER, true);
        mLocationManagerProxy.setProviderEnablement(LocationManager.NETWORK_PROVIDER, true);

        SmsManagerProxyImpl sms = getSmsManagerProxy(subId);
        sms.setDefaultValues();

        mSubscriptionManagerProxy.setDefaultValues();

        TelephonyManagerProxyImpl telephony = getTelephonyManagerProxy(subId);
        telephony.setDefaultValues();
        // TODO: Need to be removed when ImsService can handle the startImsTraffic.
        telephony.setHalVersion(-2, -2);
        telephony.setSimApplicationState(simApplicationState);
        broadcastSimApplicationStateChanged(slotId, subId, simApplicationState);

        // This ensures each test to initialize their own configuration before ImsStack starts.
        if (mTestValueInitializer != null) {
            mTestValueInitializer.init(slotId, simApplicationState);
        } else {
            initTestValues(slotId, simApplicationState);
        }
    }

    /**
     * Returns the subscription id for the given slot.
     *
     * @param slotId The slot id.
     * @return The corresponding subscription id.
     */
    public int getSubId(int slotId) {
        return mSubscriptionManagerProxy.getSubscriptionId(slotId);
    }

    /**
     * Checks whether stopping enabler is capable or not.
     *
     * @return {@code true} if stopping enabler is capable, {@code false} otherwise.
     */
    public final boolean isEnablerStoppable() {
        return mEnablerStoppable;
    }

    /**
     * Sets the flag for stopping enabler when the test is ended.
     *
     * @param enablerStoppable The flag specifying whether stopping enabler is capable or not.
     */
    public final void setEnablerStoppable(boolean enablerStoppable) {
        mEnablerStoppable = enablerStoppable;
    }

    /**
     * Starts the native enablers.
     * If the native enablers are already running, this operation does nothing.
     *
     * @param slotId The slot id.
     */
    public final void startEnabler(int slotId) {
        NativeCommands.startEnabler(slotId);
    }

    /**
     * Stops the native enablers.
     * If the native enablers are not running, this operation does nothing.
     *
     * @param slotId The slot id.
     */
    public final void stopEnabler(int slotId) {
        NativeCommands.stopEnabler(slotId);

        // Clear the cached information such as IMPI/IMPU/Domain.
        ImsPrivateProperties.Persistent.set(
                ImsPrivateProperties.Persistent.KEY_CONFIG_IMPI, "", slotId);
        ImsPrivateProperties.Persistent.set(
                ImsPrivateProperties.Persistent.KEY_CONFIG_IMPU_LIST, "", slotId);
        ImsPrivateProperties.Persistent.set(
                ImsPrivateProperties.Persistent.KEY_CONFIG_HOME_DOMAIN_NAME, "", slotId);
    }

    /**
     * Sets up the default procedure.
     *
     * @param slotId The slot id.
     */
    public void setUpBase(int slotId) {
        setUpNetwork(slotId);
        setUpImsService(slotId);
    }

    /**
     * Tears down the default procedure.
     *
     * @param slotId The slot id.
     */
    public void tearDownBase(int slotId) {
        clearImsStackState(slotId);
    }

    /**
     * Sets up the network related information.
     *
     * @param slotId The slot id.
     */
    public void setUpNetwork(int slotId) {
        mWifiAgent.waitForWifiConnected();

        mConnectivityManagerProxy.setNetwork(
                mWifiAgent.getNetwork(), mWifiAgent.getLinkProperties());
        mConnectivityManagerProxy.setPcscfServers(sPcscfAddresses);
    }

    /**
     * Sets up the ImsService.
     *
     * @param slotId The slot id.
     */
    public void setUpImsService(int slotId) {
        if (!mImsServiceConnector.isConnectionInProgress()) {
            mImsServiceConnector.start();
        }
        mImsServiceConnector.waitForServiceConnected();

        ImsServiceController.start(AppContext.getInstance(), slotId);
        setUpMmTelFeature();
    }

    /**
     * Sets up the MmTel feature.
     */
    public void setUpMmTelFeature() {
        mImsServiceConnector.createMmTelWrappers();
    }

    /**
     * Starts the ImsStack for the specified slot.
     *
     * @param slotId The slot id.
     * @param config The test configuration to be set.
     */
    public void startImsStack(int slotId, @Nullable PersistableBundle config) {
        startImsStack(slotId, TelephonyManager.SIM_STATE_LOADED, config);
    }

    /**
     * Starts the ImsStack for the specified slot.
     *
     * @param slotId The slot id.
     * @param simApplicationState The USIM application state to be configured.
     * @param config The test configuration to be set.
     */
    public void startImsStack(int slotId, int simApplicationState,
            @Nullable PersistableBundle config) {
        writeTestConfig(slotId, config);
        initSystemProxies(slotId, simApplicationState);
        triggerInService(slotId);
        int subId = getSubId(slotId);
        TelephonyManagerProxyImpl telephony = getTelephonyManagerProxy(subId);
        int carrierId = telephony.getSimCarrierId();
        int specificCarrierId = telephony.getSimSpecificCarrierId();

        if (isEnablerStoppable()) {
            updateCarrierConfig(slotId, subId);
        }
        startEnabler(slotId);
        mCarrierConfigManagerProxy.notifyCarrierConfigChanged(
                slotId, subId, carrierId, specificCarrierId);
    }

    /**
     * Starts the ImsStack for the specified slot with no service state.
     *
     * @param slotId The slot id.
     * @param config The test configuration to be set.
     */
    public void startImsStackWithNoService(int slotId, @Nullable PersistableBundle config) {
        writeTestConfig(slotId, config);
        initSystemProxies(slotId, TelephonyManager.SIM_STATE_LOADED);
        triggerOutOfService(slotId);
        int subId = getSubId(slotId);
        TelephonyManagerProxyImpl telephony = getTelephonyManagerProxy(subId);
        int carrierId = telephony.getSimCarrierId();
        int specificCarrierId = telephony.getSimSpecificCarrierId();

        if (isEnablerStoppable()) {
            updateCarrierConfig(slotId, subId);
        }
        startEnabler(slotId);
        mCarrierConfigManagerProxy.notifyCarrierConfigChanged(
                slotId, subId, carrierId, specificCarrierId);
    }

    /**
     * Clears the state of ImsStack for the specified slot.
     *
     * @param slotId The slot id.
     */
    public void clearImsStackState(int slotId) {
        int subId = getSubId(slotId);
        TelephonyManagerProxyImpl telephony = getTelephonyManagerProxy(subId);
        telephony.notifyBarringInfoChanged(new BarringInfo());
        telephony.notifyCallStateChanged(TelephonyManager.CALL_STATE_IDLE);
        telephony.notifyPreciseCallStateChanged(new PreciseCallState(
                PreciseCallState.PRECISE_CALL_STATE_IDLE,
                PreciseCallState.PRECISE_CALL_STATE_IDLE,
                PreciseCallState.PRECISE_CALL_STATE_IDLE,
                DisconnectCause.NORMAL,
                PreciseDisconnectCause.NORMAL));
        telephony.notifyPreciseDataConnectionStateChanged(
                new PreciseDataConnectionState.Builder().build());

        telephony.setSimApplicationState(TelephonyManager.SIM_STATE_ABSENT);
        broadcastSimApplicationStateChanged(
                slotId, MSimUtils.INVALID_SUB_ID, TelephonyManager.SIM_STATE_ABSENT);

        mConnectivityManagerProxy.notifyNetworkLost(APN_IMS);
        mConnectivityManagerProxy.notifyNetworkLost(APN_EMERGENCY);
        mConnectivityManagerProxy.notifyNetworkLost(APN_XCAP);

        if (isEnablerStoppable()) {
            stopEnabler(slotId);
        }

        mEventLatch.sleep(SingleLatch.SHORT_SLEEP_MS);
    }

    /**
     * Triggers the IN_SERVICE state for the specified slot.
     *
     * @param slotId The slot id.
     */
    public void triggerInService(int slotId) {
        int subId = getSubId(slotId);
        TelephonyManagerProxyImpl telephony = getTelephonyManagerProxy(subId);
        ServiceState ss = new ServiceStateBuilder()
                .addNetworkRegistrationInfoForLteCs()
                .addNetworkRegistrationInfoForLtePs()
                .build();
        telephony.setServiceState(ss);
        telephony.notifyServiceStateChanged(ss);
    }

    /**
     * Triggers the OUT_OF_SERVICE state for the specified slot.
     *
     * @param slotId The slot id.
     */
    public void triggerOutOfService(int slotId) {
        int subId = getSubId(slotId);
        TelephonyManagerProxyImpl telephony = getTelephonyManagerProxy(subId);
        ServiceState ss = new ServiceStateBuilder()
                .addNetworkRegistrationInfoForNoService()
                .build();
        telephony.setServiceState(ss);
        telephony.notifyServiceStateChanged(ss);
    }

    /**
     * Writes the test configuration.
     *
     * NOTE: This method removes the previous test configuration so if the configuration should be
     * changed for each test, the corresponding test should overwrite the configuration first
     * before running the test case.
     *
     * @param slotId The slot id.
     * @param config The test configuration to be set. If {@code config} is null, the test
     *               configuration will be clear.
     */
    public void writeTestConfig(int slotId, @Nullable PersistableBundle config) {
        ConfigInterface ci = AgentFactory.getInstance().getAgent(ConfigInterface.class, slotId);

        // When a ConfigAgent is created once and maintained,
        // the cached test configuration needs to be updated.
        if (ci != null) {
            PersistableBundle testConfig = ci.readTestConfig();

            if (config != null) {
                testConfig.putAll(config);
            } else {
                testConfig.clear();
            }
        } else {
            AppContext.getInstance().deleteFile(CarrierConfig.TEST_CARRIER_CONFIG_FILE);

            if (config != null && !config.isEmpty()) {
                try (OutputStream os = AppContext.getInstance().openFileOutput(
                        CarrierConfig.TEST_CARRIER_CONFIG_FILE,
                        Context.MODE_APPEND)) {
                    config.writeToStream(os);
                    Log.d(this, "writeTestConfig: Ok");
                    return;
                } catch (IOException e) {
                    Log.d(this, "writeTestConfig: " + e.toString());
                }

                fail("Initializing the test configuration failed.");
            }
        }
    }

    /**
     * Enable or disable a capability for multiple radio technologies.
     *
     * @param enabled The flag specifying whether the {@code capability} is enabled or not.
     * @param capability The MmTel capability.
     * @param radioTechs The list of supported radio technology.
     */
    public void changeMmTelCapability(boolean enabled, int capability, int... radioTechs) {
        CapabilityChangeRequest request = new CapabilityChangeRequest();
        if (enabled) {
            for (int radioTech : radioTechs) {
                request.addCapabilitiesToEnableForTech(capability, radioTech);
            }
        } else {
            for (int radioTech : radioTechs) {
                request.addCapabilitiesToDisableForTech(capability, radioTech);
            }
        }

        ImsMmTelFeatureWrapper mmTelFeature = mImsServiceConnector.getMmTelFeature();
        mmTelFeature.changeCapabilitiesConfiguration(request, null);
    }

    /**
     * Disables all MMTEL capabilities.
     */
    public void disableAllMmTelCapabilities() {
        changeMmTelCapability(false, CAPABILITY_TYPE_VOICE,
                REGISTRATION_TECH_LTE, REGISTRATION_TECH_NR, REGISTRATION_TECH_IWLAN);
        changeMmTelCapability(false, CAPABILITY_TYPE_VIDEO,
                REGISTRATION_TECH_LTE, REGISTRATION_TECH_NR, REGISTRATION_TECH_IWLAN);
        changeMmTelCapability(false, CAPABILITY_TYPE_SMS,
                REGISTRATION_TECH_LTE, REGISTRATION_TECH_NR, REGISTRATION_TECH_IWLAN);
        changeMmTelCapability(false, CAPABILITY_TYPE_UT,
                REGISTRATION_TECH_LTE, REGISTRATION_TECH_NR, REGISTRATION_TECH_IWLAN);
    }

    /**
     * Enables all MMTEL capabilities.
     */
    public void enableAllMmTelCapabilities() {
        changeMmTelCapability(true, CAPABILITY_TYPE_VOICE,
                REGISTRATION_TECH_LTE, REGISTRATION_TECH_NR, REGISTRATION_TECH_IWLAN);
        changeMmTelCapability(true, CAPABILITY_TYPE_VIDEO,
                REGISTRATION_TECH_LTE, REGISTRATION_TECH_NR, REGISTRATION_TECH_IWLAN);
        changeMmTelCapability(true, CAPABILITY_TYPE_SMS,
                REGISTRATION_TECH_LTE, REGISTRATION_TECH_NR, REGISTRATION_TECH_IWLAN);
        changeMmTelCapability(true, CAPABILITY_TYPE_UT,
                REGISTRATION_TECH_LTE, REGISTRATION_TECH_NR, REGISTRATION_TECH_IWLAN);
    }

    /**
     * Sets the {@link TestValueInitializer} to override the default test values.
     *
     * @param initializer The {@link TestValueInitializer} instance or null.
     */
    public void setTestValueInitializer(TestValueInitializer initializer) {
        mTestValueInitializer = initializer;
    }

    /**
     * Broadcasts the change of SIM application state.
     *
     * @param slotId The slot id.
     * @param subId The subscription id.
     * @param simState The SIM application state.
     */
    public void broadcastSimApplicationStateChanged(int slotId, int subId, int simState) {
        Intent intent = new Intent(TelephonyManager.ACTION_SIM_APPLICATION_STATE_CHANGED);
        intent.putExtra(TelephonyManager.EXTRA_SIM_STATE, simState);
        intent.putExtra(SubscriptionManager.EXTRA_SLOT_INDEX, slotId);
        intent.putExtra(SubscriptionManager.EXTRA_SUBSCRIPTION_INDEX, subId);
        mBroadcastReceiverProxy.sendIntent(intent);
        // Waits for 500ms to handle the SIM state change.
        mEventLatch.sleep(SingleLatch.SHORT_SLEEP_MS / 2);
    }

    /**
     * Updates the carrier configuration immediately.
     *
     * @param slotId The slot id.
     * @param subId  The subscription id.
     */
    public void updateCarrierConfig(int slotId, int subId) {
        ServiceCaps.updateServiceCapabilities(AppContext.getInstance(), slotId, subId);
        ServiceLoader.updateCarrierConfig(slotId);
    }

    protected final TelephonyManagerProxyImpl getTelephonyManagerProxy(int subId) {
        return (TelephonyManagerProxyImpl) mTelephonyManagerProxy.createForSubscriptionId(subId);
    }

    protected final SmsManagerProxyImpl getSmsManagerProxy(int subId) {
        return (SmsManagerProxyImpl) mSmsManagerProxy.createForSubscriptionId(subId);
    }

    protected final ImsMmTelManagerProxyImpl getImsMmTelManagerProxy(int subId) {
        return (ImsMmTelManagerProxyImpl) mImsManagerProxy.getImsMmTelManagerProxy(subId);
    }

    protected final ProvisioningManagerProxyImpl getProvisioningManagerProxy(int subId) {
        return (ProvisioningManagerProxyImpl) mImsManagerProxy.getProvisioningManagerProxy(subId);
    }

    protected void initTestValues(int slotId, int simApplicationState) {
        // Subclass can override this method to initialize the additional default test values
        // before the ImsStack starts.
        // This method can be used for all the tests which inherit this class.
        // If the subclass wants to use this for each test,
        // it can set the value initializer with {@link #setTestValueInitializer()}
        // in a specific test.
    }

    protected static void logd(Object o, String s) {
        Log.d(o, s);
    }

    protected static void loge(Object o, String s) {
        Log.e(o, s);
    }

    protected static void logi(Object o, String s) {
        Log.i(o, s);
    }
}
