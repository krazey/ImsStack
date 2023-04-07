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

import static org.junit.Assert.*;
import static org.mockito.ArgumentMatchers.eq;
import static org.mockito.Mockito.times;
import static org.mockito.Mockito.verify;
import static org.mockito.Mockito.when;

import android.content.Context;
import android.os.Looper;
import android.telephony.ims.feature.MmTelFeature;
import android.test.mock.MockContentResolver;
import android.util.ArraySet;

import com.android.imsstack.enabler.IBaseContext;
import com.android.imsstack.enabler.IContext;
import com.android.imsstack.enabler.aos.IAosRegistration;
import com.android.imsstack.enabler.aos.IAosRegistrationListener;
import com.android.imsstack.imsservice.mmtel.base.IMmTelFeatureCapabilityListener;
import com.android.imsstack.imsservice.mmtel.ut.base.IUtInterface;

import org.junit.After;
import org.junit.Before;
import org.junit.Test;
import org.junit.runner.RunWith;
import org.junit.runners.JUnit4;
import org.mockito.Mock;
import org.mockito.MockitoAnnotations;

@RunWith(JUnit4.class)
public class ImsFeatureManagerTest {
    private ImsFeatureManager mFeatureManager;
    private MmTelFeature.MmTelCapabilities mMmTelCapabilities;
    private ImsRegistrationTracker mRegTracker;
    private IAosRegistrationListener mAosRegListener;
    private MockContentResolver mContentResolver;
    MockIAosRegistration mAosReg;

    @Mock Context mContext;
    @Mock IBaseContext mMockBaseContext;
    @Mock IContext mMockContext;
    @Mock IMmTelFeatureCapabilityListener mMockFeatureCapabilityListener;
    @Mock IUtInterface mMockUt;

    @Before
    public void setUp() {
        MockitoAnnotations.initMocks(this);

        mContentResolver = new MockContentResolver();
        when(mMockContext.getContext()).thenReturn(mContext);
        when(mMockBaseContext.getDefaultLooper()).thenReturn(Looper.getMainLooper());
        when(mMockContext.getDefaultLooper()).thenReturn(Looper.getMainLooper());
        when(mContext.getContentResolver()).thenReturn(mContentResolver);
        mAosReg = new MockIAosRegistration();
        mFeatureManager = new ImsFeatureManager(mMockBaseContext, mMockFeatureCapabilityListener);
        mRegTracker = new FakeImsRegistrationTracker(mMockContext, new ImsRegistrationImpl());
        mFeatureManager.setRegistrationTracker(mRegTracker);
        mMmTelCapabilities = new MmTelFeature.MmTelCapabilities(0);
        mAosRegListener = mAosReg.getListener();
    }

    @After
    public void tearDown() {
        mFeatureManager.dispose();
        mContentResolver = null;
    }

    @Test
    public void testsetUtInterface() {
        mFeatureManager.setUtInterface(mMockUt);
    }

    @Test
    public void testupdateFeatureCapabilities() {
        int features = (IAosRegistrationListener.FeatureTagMask.MMTEL
                | IAosRegistrationListener.FeatureTagMask.VIDEO
                | IAosRegistrationListener.FeatureTagMask.SMSIP);

        mAosRegListener.notifyRegistered(
                IAosRegistrationListener.NetworkType.LTE,
                features, new ArraySet<String>());

        when(mMockUt.isUtAvailable()).thenReturn(false);

        int capabilities = MmTelFeature.MmTelCapabilities.CAPABILITY_TYPE_VOICE
                | MmTelFeature.MmTelCapabilities.CAPABILITY_TYPE_VIDEO
                | MmTelFeature.MmTelCapabilities.CAPABILITY_TYPE_SMS;
        int removeCapabilities = MmTelFeature.MmTelCapabilities.CAPABILITY_TYPE_UT;

        mMmTelCapabilities.addCapabilities(capabilities);
        mMmTelCapabilities.removeCapabilities(removeCapabilities);
        verify(mMockFeatureCapabilityListener).onFeatureCapabilityChanged(
                eq(mMmTelCapabilities));
    }

    @Test
    public void testupdateFeatureCapabilities_iwlanReg() {
        int features = (IAosRegistrationListener.FeatureTagMask.MMTEL
                | IAosRegistrationListener.FeatureTagMask.SMSIP);

        mAosRegListener.notifyRegistered(
                IAosRegistrationListener.NetworkType.IWLAN,
                features, new ArraySet<String>());
        when(mMockUt.isUtAvailable()).thenReturn(true);

        int capabilities = MmTelFeature.MmTelCapabilities.CAPABILITY_TYPE_VOICE
                | MmTelFeature.MmTelCapabilities.CAPABILITY_TYPE_SMS;
        int removeCapabilities = MmTelFeature.MmTelCapabilities.CAPABILITY_TYPE_UT
                | MmTelFeature.MmTelCapabilities.CAPABILITY_TYPE_VIDEO;

        mMmTelCapabilities.addCapabilities(capabilities);
        mMmTelCapabilities.removeCapabilities(removeCapabilities);
        verify(mMockFeatureCapabilityListener).onFeatureCapabilityChanged(
                eq(mMmTelCapabilities));
    }

    @Test
    public void testupdateFeatureCapabilities_deReg() {
        mAosRegListener.notifyDeregistered(IAosRegistrationListener.NetworkType.LTE,
                IAosRegistrationListener.ReasonCode.CODE_REGISTRATION_ERROR);
        when(mMockUt.isUtAvailable()).thenReturn(true);

        int removeCapabilities = MmTelFeature.MmTelCapabilities.CAPABILITY_TYPE_UT
                | MmTelFeature.MmTelCapabilities.CAPABILITY_TYPE_VOICE
                | MmTelFeature.MmTelCapabilities.CAPABILITY_TYPE_SMS
                | MmTelFeature.MmTelCapabilities.CAPABILITY_TYPE_VIDEO;

        mMmTelCapabilities.addCapabilities(MmTelFeature.MmTelCapabilities.CAPABILITY_TYPE_NONE);
        mMmTelCapabilities.removeCapabilities(removeCapabilities);
        verify(mMockFeatureCapabilityListener).onFeatureCapabilityChanged(
                eq(mMmTelCapabilities));
    }

    @Test
    public void testupdateFeaturesOnServiceUpDown() {
        int features = (IAosRegistrationListener.FeatureTagMask.MMTEL
                | IAosRegistrationListener.FeatureTagMask.VIDEO
                | IAosRegistrationListener.FeatureTagMask.SMSIP);

        mAosRegListener.notifyRegistered(
                IAosRegistrationListener.NetworkType.LTE,
                features, new ArraySet<String>());
        when(mMockUt.isUtAvailable()).thenReturn(true);

        mFeatureManager.updateFeaturesOnServiceUpDown(true);

        int capabilities = MmTelFeature.MmTelCapabilities.CAPABILITY_TYPE_VOICE
                | MmTelFeature.MmTelCapabilities.CAPABILITY_TYPE_VIDEO
                | MmTelFeature.MmTelCapabilities.CAPABILITY_TYPE_SMS;
        int removeCapabilities = MmTelFeature.MmTelCapabilities.CAPABILITY_TYPE_UT;

        mMmTelCapabilities.addCapabilities(capabilities);
        mMmTelCapabilities.removeCapabilities(removeCapabilities);
        verify(mMockFeatureCapabilityListener, times(2)).onFeatureCapabilityChanged(
                eq(mMmTelCapabilities));
    }

    @Test
    public void testupdateFeaturesOnServiceUpDown_down() {
        mFeatureManager.updateFeaturesOnServiceUpDown(false);

        int removeCapabilities = MmTelFeature.MmTelCapabilities.CAPABILITY_TYPE_UT
                | MmTelFeature.MmTelCapabilities.CAPABILITY_TYPE_VOICE
                | MmTelFeature.MmTelCapabilities.CAPABILITY_TYPE_SMS
                | MmTelFeature.MmTelCapabilities.CAPABILITY_TYPE_VIDEO;

        mMmTelCapabilities.addCapabilities(MmTelFeature.MmTelCapabilities.CAPABILITY_TYPE_NONE);
        mMmTelCapabilities.removeCapabilities(removeCapabilities);
        verify(mMockFeatureCapabilityListener).onFeatureCapabilityChanged(
                eq(mMmTelCapabilities));
    }
    private class FakeImsRegistrationTracker extends ImsRegistrationTracker {
        FakeImsRegistrationTracker(IContext context, ImsRegistrationImpl regImpl) {
            super(context, regImpl);
        }

        @Override
        protected IAosRegistration getIAosRegistration(int slotId) {
            return mAosReg;
        }
    }
}
