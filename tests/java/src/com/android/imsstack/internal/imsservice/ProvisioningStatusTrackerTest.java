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
package com.android.imsstack.internal.imsservice;

import static org.junit.Assert.assertEquals;
import static org.mockito.ArgumentMatchers.any;
import static org.mockito.Mockito.times;
import static org.mockito.Mockito.verify;
import static org.mockito.Mockito.verifyNoMoreInteractions;
import static org.mockito.Mockito.when;

import android.telephony.ims.ProvisioningManager.FeatureProvisioningCallback;
import android.telephony.ims.feature.MmTelFeature;
import android.telephony.ims.feature.MmTelFeature.MmTelCapabilities;
import android.telephony.ims.stub.ImsRegistrationImplBase;

import androidx.test.filters.SmallTest;

import com.android.imsstack.base.SystemServiceProxy.ImsManagerProxy;
import com.android.imsstack.base.SystemServiceProxy.ProvisioningManagerProxy;
import com.android.imsstack.base.SystemServiceProxy.SubscriptionManagerProxy;
import com.android.imsstack.base.TestAppContext;
import com.android.imsstack.core.agents.AgentFactory;
import com.android.imsstack.core.agents.Sim;
import com.android.imsstack.core.agents.SimInterface;

import org.junit.After;
import org.junit.Before;
import org.junit.Test;
import org.junit.runner.RunWith;
import org.junit.runners.JUnit4;
import org.mockito.ArgumentCaptor;
import org.mockito.Mock;
import org.mockito.MockitoAnnotations;

@RunWith(JUnit4.class)
public class ProvisioningStatusTrackerTest {
    @Mock SimInterface mSimInterface;
    @Mock ProvisioningStatusTracker.Listener mListener;

    private TestAppContext mTestAppContext;
    private ProvisioningManagerProxy mProvisioningManagerProxy;
    private ProvisioningStatusTracker mProvisioningStatusTracker;

    @Before
    public void setUp() throws Exception {
        MockitoAnnotations.initMocks(this);

        mTestAppContext = new TestAppContext();
        mTestAppContext.setUp();

        AgentFactory.getInstance()
                .setAgent(SimInterface.class, mSimInterface, TestAppContext.SLOT0);

        SubscriptionManagerProxy smp =
                mTestAppContext.getSystemServiceProxy(SubscriptionManagerProxy.class);
        when(smp.getSubscriptionId(TestAppContext.SLOT0)).thenReturn(TestAppContext.SUB_ID_1);

        ImsManagerProxy imp = mTestAppContext.getSystemServiceProxy(ImsManagerProxy.class);
        mProvisioningManagerProxy = imp.getProvisioningManagerProxy(TestAppContext.SUB_ID_1);

        mProvisioningStatusTracker = ProvisioningStatusTracker.getInstance(TestAppContext.SLOT0);
    }

    @After
    public void tearDown() throws Exception {
        mProvisioningStatusTracker.releaseInstance(TestAppContext.SLOT0);
        mProvisioningStatusTracker = null;
        mProvisioningManagerProxy = null;

        mTestAppContext.tearDown();
        mTestAppContext = null;

        mListener = null;
        mSimInterface = null;
    }

    @Test
    @SmallTest
    public void start_addSimStateListener() {
        mProvisioningStatusTracker.start();

        ArgumentCaptor<Sim.Listener> captor =
                ArgumentCaptor.forClass(Sim.Listener.class);
        verify(mSimInterface).addListener(captor.capture());
    }

    @Test
    @SmallTest
    public void start_registerFeatureProvisioningChangedCallback() {
        mProvisioningStatusTracker.start();

        verify(mProvisioningManagerProxy).registerFeatureProvisioningChangedCallback(any(), any());
    }

    @Test
    @SmallTest
    public void onSimStateChanged_subIdIsNotChanged_ignored() {
        // register callback for initial subscriber (SUB_ID_1)
        mProvisioningStatusTracker.start();
        verify(mProvisioningManagerProxy).registerFeatureProvisioningChangedCallback(any(), any());

        // onSimStateChanged for same subscriber (SUB_ID_1)
        ArgumentCaptor<Sim.Listener> captor =
                ArgumentCaptor.forClass(Sim.Listener.class);
        verify(mSimInterface).addListener(captor.capture());
        Sim.Listener listener = captor.getValue();
        listener.onSimStateChanged();

        verifyNoMoreInteractions(mProvisioningManagerProxy);
    }

    @Test
    @SmallTest
    public void onSimStateChanged_subIdIsChanged_registerCallbackAgain() {
        // register callback for initial subscriber (SUB_ID_1)
        mProvisioningStatusTracker.start();

        // onSimStateChanged for different subscriber (SUB_ID_2)
        SubscriptionManagerProxy smp =
                mTestAppContext.getSystemServiceProxy(SubscriptionManagerProxy.class);
        when(smp.getSubscriptionId(TestAppContext.SLOT0)).thenReturn(TestAppContext.SUB_ID_2);

        ArgumentCaptor<Sim.Listener> captor =
                ArgumentCaptor.forClass(Sim.Listener.class);
        verify(mSimInterface).addListener(captor.capture());
        Sim.Listener listener = captor.getValue();
        listener.onSimStateChanged();

        // re-register callback
        verify(mProvisioningManagerProxy).unregisterFeatureProvisioningChangedCallback(any());
        verify(mProvisioningManagerProxy, times(2))
                .registerFeatureProvisioningChangedCallback(any(), any());
    }

    @Test
    @SmallTest
    public void addListener_notifyListenerOfInformationChange() {
        // register callback
        mProvisioningStatusTracker.start();
        ArgumentCaptor<FeatureProvisioningCallback> captor =
                ArgumentCaptor.forClass(FeatureProvisioningCallback.class);
        verify(mProvisioningManagerProxy)
                .registerFeatureProvisioningChangedCallback(any(), captor.capture());
        FeatureProvisioningCallback callback = captor.getValue();


        // add listener
        mProvisioningStatusTracker.addListener(mListener);

        // invoke callback from framework
        int capability = MmTelFeature.MmTelCapabilities.CAPABILITY_TYPE_VOICE;
        int tech = ImsRegistrationImplBase.REGISTRATION_TECH_LTE;
        boolean isProvisioned = true;
        callback.onFeatureProvisioningChanged(capability, tech, isProvisioned);

        // notify the listener the information received from the framework
        verify(mListener).onFeatureProvisioningChanged(capability, tech, isProvisioned);
    }

    @Test
    @SmallTest
    public void removeListener_doNotNotifyListenerOfInformationChange() {
        // register callback
        mProvisioningStatusTracker.start();
        ArgumentCaptor<FeatureProvisioningCallback> captor =
                ArgumentCaptor.forClass(FeatureProvisioningCallback.class);
        verify(mProvisioningManagerProxy)
                .registerFeatureProvisioningChangedCallback(any(), captor.capture());
        FeatureProvisioningCallback callback = captor.getValue();

        // remove listener
        mProvisioningStatusTracker.addListener(mListener);
        mProvisioningStatusTracker.removeListener(mListener);

        // invoke callback from framework
        int capability = MmTelCapabilities.CAPABILITY_TYPE_VOICE;
        int tech = ImsRegistrationImplBase.REGISTRATION_TECH_LTE;
        boolean isProvisioned = true;
        callback.onFeatureProvisioningChanged(capability, tech, isProvisioned);

        // do not notify the listener the information received from the framework
        verifyNoMoreInteractions(mListener);
    }

    @Test
    @SmallTest
    public void getProvisioningStatusForCapability_returnCurrentStatus() {
        int capability = MmTelCapabilities.CAPABILITY_TYPE_VOICE;
        int tech = ImsRegistrationImplBase.REGISTRATION_TECH_LTE;
        boolean isProvisioned = true;
        when(mProvisioningManagerProxy.getProvisioningStatusForCapability(capability, tech))
                .thenReturn(isProvisioned);

        boolean result = mProvisioningStatusTracker.getProvisioningStatusForCapability(
                capability, tech);

        verify(mProvisioningManagerProxy).getProvisioningStatusForCapability(capability, tech);
        assertEquals(isProvisioned, result);
    }
}
