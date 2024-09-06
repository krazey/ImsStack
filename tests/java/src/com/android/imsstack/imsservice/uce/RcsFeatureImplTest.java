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
import static org.junit.Assert.assertTrue;
import static org.mockito.ArgumentMatchers.any;
import static org.mockito.ArgumentMatchers.anyInt;
import static org.mockito.Mockito.when;

import android.content.Context;
import android.os.PersistableBundle;
import android.os.RemoteException;
import android.telephony.ims.ImsRcsManager;
import android.telephony.ims.aidl.IImsCapabilityCallback;
import android.telephony.ims.feature.CapabilityChangeRequest;
import android.telephony.ims.feature.RcsFeature.RcsImsCapabilities;
import android.telephony.ims.stub.CapabilityExchangeEventListener;
import android.telephony.ims.stub.ImsRegistrationImplBase;
import android.telephony.ims.stub.RcsCapabilityExchangeImplBase;

import androidx.test.core.app.ApplicationProvider;

import com.android.imsstack.base.SystemServiceProxy.CarrierConfigManagerProxy;
import com.android.imsstack.base.TestAppContext;
import com.android.imsstack.enabler.uce.impl.RcsCapOptionsResponseCallBack;
import com.android.imsstack.enabler.uce.impl.RcsCapPublishResponseCallBack;
import com.android.imsstack.enabler.uce.impl.RcsCapSubscribeResponseCallBack;
import com.android.imsstack.enabler.uce.interf.IUceApi;
import com.android.imsstack.imsservice.base.ImsContext;
import com.android.imsstack.util.Log;
import com.android.imsstack.util.MessageExecutor;

import org.junit.After;
import org.junit.Assert;
import org.junit.Before;
import org.junit.Test;
import org.junit.runner.RunWith;
import org.junit.runners.JUnit4;
import org.mockito.Mock;
import org.mockito.Mockito;
import org.mockito.MockitoAnnotations;

@RunWith(JUnit4.class)
public class RcsFeatureImplTest {
    private static final int STATE_READY = 2;

    @Mock private Context mMockContext;
    @Mock private CapabilityExchangeEventListener mCapabilityExchangeEventListener;
    @Mock private RcsCapPublishResponseCallBack mRcsCapPublishResponseCallBack;
    @Mock private RcsCapSubscribeResponseCallBack mRcsCapSubscribeResponseCallBack;
    @Mock private RcsCapOptionsResponseCallBack mRcsCapOptionsResponseCallBack;
    @Mock private IUceApi mUceApi;

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
            Log.i("RcsFeature", "onQueryCapabilityConfiguration ..for verifying ");
        }

        @Override
        public void onChangeCapabilityConfigurationError(int capability, int radioTech, int reason)
                throws RemoteException {
            Log.i("RcsFeature", "onChangeCapabilityConfigurationError ..for verifying ");
            mIsOnChangeCapabilityConfigurationError = true;
        }

        @Override
        public void onCapabilitiesStatusChanged(int config) throws RemoteException {
            Log.i("RcsFeature", "onCapabilitiesStatusChanged ..for verifying ");
            mIsOnCapabilitiesStatusChanged = true;
        }
    }

    @Before
    public void setUp() {
        MockitoAnnotations.initMocks(this);
        mExecutor = new MessageExecutor(RcsFeatureImplTest.class.getSimpleName());
        mMockContext = Mockito.spy(ApplicationProvider.getApplicationContext());
        mTestAppContext = new TestAppContext(mMockContext);
        mTestAppContext.setUp();

        mImsContext = new ImsContext(mMockContext, mExecutor, TestAppContext.SLOT0);
        mFeature = new TestRcsFeatureImpl(mImsContext);
        mFeature.initialize(mMockContext, TestAppContext.SLOT0);
        mFeature.setFeatureState(STATE_READY);
    }

    @Test
    public void onFeatureReadyTest() {
        int status = mFeature.getFeatureState();
        assertEquals(STATE_READY, status);
    }

    @Test
    public void createCapabilityExchangeImplTest() {
        Assert.assertNotNull(
                mFeature.createCapabilityExchangeImpl(mCapabilityExchangeEventListener));
    }

    @Test
    public void  queryCapabilityConfiguration() {
        boolean flag =  mFeature.queryCapabilityConfiguration(
                ImsRcsManager.CAPABILITY_TYPE_PRESENCE_UCE,
                ImsRegistrationImplBase.REGISTRATION_TECH_LTE);

        assertTrue(flag);

        flag =  mFeature.queryCapabilityConfiguration(ImsRcsManager.CAPABILITY_TYPE_PRESENCE_UCE,
                ImsRegistrationImplBase.REGISTRATION_TECH_NONE);
        assertFalse(flag);

        flag =  mFeature.queryCapabilityConfiguration(ImsRcsManager.CAPABILITY_TYPE_OPTIONS_UCE,
                ImsRegistrationImplBase.REGISTRATION_TECH_NONE);
        assertFalse(flag);

        flag =  mFeature.queryCapabilityConfiguration(ImsRcsManager.CAPABILITY_TYPE_OPTIONS_UCE,
                ImsRegistrationImplBase.REGISTRATION_TECH_LTE);
        assertTrue(flag);
    }

    @Test
    public void changeEnabledCapabilities() throws RemoteException {
        //Carrier Configuration stubbing
        mCapabilityExchangeEventListener = Mockito.mock(CapabilityExchangeEventListener.class);

        PersistableBundle bundle = new PersistableBundle();
        CarrierConfigManagerProxy ccmp =
                mTestAppContext.getSystemServiceProxy(CarrierConfigManagerProxy.class);
        when(ccmp.getConfigForSubId(anyInt(), any())).thenReturn(bundle);

        // CapabilityChangeRequest can not be mocked It is a final class.
        CapabilityChangeRequest request =  new CapabilityChangeRequest();
        request.addCapabilitiesToEnableForTech(RcsImsCapabilities.CAPABILITY_TYPE_PRESENCE_UCE,
                ImsRegistrationImplBase.REGISTRATION_TECH_LTE);
        request.addCapabilitiesToEnableForTech(RcsImsCapabilities.CAPABILITY_TYPE_OPTIONS_UCE,
                ImsRegistrationImplBase.REGISTRATION_TECH_LTE);
        CapabilityCallback capabilityCallback = new CapabilityCallback();
        mFeature.addCapabilityCallback(capabilityCallback);
        //verify API changeEnabledCapabilities call
        mFeature.changeEnabledCapabilities(request, null);
        Assert.assertTrue(capabilityCallback.mIsOnCapabilitiesStatusChanged);
        Assert.assertFalse(capabilityCallback.mIsOnChangeCapabilityConfigurationError);
    }

    @After
    public void tearDown() {
        mTestAppContext.tearDown();
        mTestAppContext = null;
    }

    @After
    public void cleanUp() {
        mFeature = null;
        mCapabilityExchangeEventListener = null;

        if (mExecutor != null) {
            mExecutor.getLooper().quit();
            mExecutor = null;
        }
    }

    class TestRcsFeatureImpl extends RcsFeatureImpl {
        TestRcsFeatureImpl(ImsContext imsContext) {
            super(imsContext);
        }

        @Override
        public RcsCapabilityExchangeImplBase createCapabilityExchangeImpl(
                CapabilityExchangeEventListener listener) {
            return new RcsCapExchangeImpl(mCapabilityExchangeEventListener, TestAppContext.SLOT0,
                    mMockContext, mUceApi, mRcsCapSubscribeResponseCallBack,
                    mRcsCapOptionsResponseCallBack, mRcsCapPublishResponseCallBack);
        }
    }
}
