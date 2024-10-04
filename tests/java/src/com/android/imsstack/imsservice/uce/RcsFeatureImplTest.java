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

package com.android.imsstack.imsservice.uce;

import static org.junit.Assert.assertEquals;
import static org.junit.Assert.assertFalse;
import static org.junit.Assert.assertNotNull;
import static org.junit.Assert.assertTrue;
import static org.mockito.ArgumentMatchers.any;
import static org.mockito.ArgumentMatchers.anyInt;
import static org.mockito.Mockito.when;

import android.os.PersistableBundle;
import android.os.RemoteException;
import android.telephony.ims.ImsRcsManager;
import android.telephony.ims.aidl.IImsCapabilityCallback;
import android.telephony.ims.feature.CapabilityChangeRequest;
import android.telephony.ims.feature.RcsFeature.RcsImsCapabilities;
import android.telephony.ims.stub.CapabilityExchangeEventListener;
import android.telephony.ims.stub.ImsRegistrationImplBase;
import android.telephony.ims.stub.RcsCapabilityExchangeImplBase;

import com.android.imsstack.base.SystemServiceProxy.CarrierConfigManagerProxy;
import com.android.imsstack.base.TestAppContext;
import com.android.imsstack.enabler.uce.impl.RcsCapOptionsResponseCallback;
import com.android.imsstack.enabler.uce.impl.RcsCapPublishResponseCallback;
import com.android.imsstack.enabler.uce.impl.RcsCapSubscribeResponseCallback;
import com.android.imsstack.enabler.uce.interf.IUceApi;
import com.android.imsstack.imsservice.base.ImsContext;
import com.android.imsstack.util.Log;
import com.android.imsstack.util.MessageExecutor;

import org.junit.After;
import org.junit.Before;
import org.junit.Test;
import org.junit.runner.RunWith;
import org.junit.runners.JUnit4;
import org.mockito.Mock;
import org.mockito.MockitoAnnotations;

import java.util.concurrent.Executor;

@RunWith(JUnit4.class)
public class RcsFeatureImplTest {
    private static final int STATE_READY = 2;

    @Mock private CapabilityExchangeEventListener mCapabilityExchangeEventListener;
    @Mock private RcsCapPublishResponseCallback mRcsCapPublishResponseCallback;
    @Mock private RcsCapSubscribeResponseCallback mRcsCapSubscribeResponseCallback;
    @Mock private RcsCapOptionsResponseCallback mRcsCapOptionsResponseCallback;
    @Mock private IUceApi mUceApi;
    @Mock private Executor mMessageExecutor;

    private MessageExecutor mExecutor = null;
    private ImsContext mImsContext;
    private TestAppContext mTestAppContext;
    private TestRcsFeatureImpl mFeature;

    private static class CapabilityCallback extends IImsCapabilityCallback.Stub {
        boolean mIsOnCapabilitiesStatusChanged = false;
        boolean mIsOnChangeCapabilityConfigurationError = false;

        @Override
        public void onQueryCapabilityConfiguration(int capability, int radioTech, boolean enabled)
                throws RemoteException {
            Log.i(this, "onQueryCapabilityConfiguration for verifying");
        }

        @Override
        public void onChangeCapabilityConfigurationError(int capability, int radioTech, int reason)
                throws RemoteException {
            Log.i(this, "onChangeCapabilityConfigurationError for verifying");
            mIsOnChangeCapabilityConfigurationError = true;
        }

        @Override
        public void onCapabilitiesStatusChanged(int config) throws RemoteException {
            Log.i(this, "onCapabilitiesStatusChanged for verifying");
            mIsOnCapabilitiesStatusChanged = true;
        }
    }

    @Before
    public void setUp() {
        MockitoAnnotations.initMocks(this);
        mExecutor = new MessageExecutor(RcsFeatureImplTest.class.getSimpleName());
        mTestAppContext = new TestAppContext();
        mTestAppContext.setUp();

        mImsContext = new ImsContext(mTestAppContext.getContext(), mExecutor, TestAppContext.SLOT0);
        mFeature = new TestRcsFeatureImpl(mImsContext);
        mFeature.initialize(mTestAppContext.getContext(), TestAppContext.SLOT0);
        mFeature.setFeatureState(STATE_READY);
    }

    @After
    public void tearDown() {
        mTestAppContext.tearDown();
        mTestAppContext = null;
        mFeature = null;
        mCapabilityExchangeEventListener = null;

        if (mExecutor != null) {
            mExecutor.getLooper().quit();
            mExecutor = null;
        }
    }

    @Test
    public void onFeatureReadyTest() {
        int status = mFeature.getFeatureState();
        assertEquals(STATE_READY, status);
    }

    @Test
    public void createCapabilityExchangeImplTest() {
        assertNotNull(
                mFeature.createCapabilityExchangeImpl(mCapabilityExchangeEventListener));
    }

    @Test
    public void queryCapabilityConfiguration() {
        boolean flag =
                mFeature.queryCapabilityConfiguration(
                        ImsRcsManager.CAPABILITY_TYPE_PRESENCE_UCE,
                        ImsRegistrationImplBase.REGISTRATION_TECH_LTE);

        assertTrue(flag);

        flag =
                mFeature.queryCapabilityConfiguration(
                        ImsRcsManager.CAPABILITY_TYPE_PRESENCE_UCE,
                        ImsRegistrationImplBase.REGISTRATION_TECH_NONE);
        assertFalse(flag);

        flag =
                mFeature.queryCapabilityConfiguration(
                        ImsRcsManager.CAPABILITY_TYPE_OPTIONS_UCE,
                        ImsRegistrationImplBase.REGISTRATION_TECH_NONE);
        assertFalse(flag);

        flag =
                mFeature.queryCapabilityConfiguration(
                        ImsRcsManager.CAPABILITY_TYPE_OPTIONS_UCE,
                        ImsRegistrationImplBase.REGISTRATION_TECH_LTE);
        assertTrue(flag);
    }

    @Test
    public void changeEnabledCapabilities() throws RemoteException {
        // Carrier Configuration stubbing
        PersistableBundle bundle = new PersistableBundle();
        CarrierConfigManagerProxy ccmp =
                mTestAppContext.getSystemServiceProxy(CarrierConfigManagerProxy.class);
        when(ccmp.getConfigForSubId(anyInt(), any())).thenReturn(bundle);

        // CapabilityChangeRequest can not be mocked It is a final class.
        CapabilityChangeRequest request = new CapabilityChangeRequest();
        request.addCapabilitiesToEnableForTech(
                RcsImsCapabilities.CAPABILITY_TYPE_PRESENCE_UCE,
                ImsRegistrationImplBase.REGISTRATION_TECH_LTE);
        request.addCapabilitiesToEnableForTech(
                RcsImsCapabilities.CAPABILITY_TYPE_OPTIONS_UCE,
                ImsRegistrationImplBase.REGISTRATION_TECH_LTE);
        CapabilityCallback capabilityCallback = new CapabilityCallback();
        mFeature.addCapabilityCallback(capabilityCallback);
        // verify API changeEnabledCapabilities call
        mFeature.changeEnabledCapabilities(request, null);
        assertTrue(capabilityCallback.mIsOnCapabilitiesStatusChanged);
        assertFalse(capabilityCallback.mIsOnChangeCapabilityConfigurationError);
    }

    class TestRcsFeatureImpl extends RcsFeatureImpl {
        TestRcsFeatureImpl(ImsContext imsContext) {
            super(imsContext);
        }

        @Override
        public RcsCapabilityExchangeImplBase createCapabilityExchangeImpl(
                CapabilityExchangeEventListener listener) {
            return new RcsCapExchangeImpl(
                    mCapabilityExchangeEventListener,
                    TestAppContext.SLOT0,
                    mTestAppContext.getContext(),
                    mUceApi,
                    mRcsCapSubscribeResponseCallback,
                    mRcsCapOptionsResponseCallback,
                    mRcsCapPublishResponseCallback,
                    mExecutor,
                    mMessageExecutor);
        }
    }
}