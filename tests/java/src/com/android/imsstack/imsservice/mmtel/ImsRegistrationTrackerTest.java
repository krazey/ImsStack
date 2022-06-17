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

import android.os.Looper;
import android.telephony.ims.feature.CapabilityChangeRequest.CapabilityPair;
import android.telephony.ims.feature.MmTelFeature;
import android.util.ArraySet;

import com.android.imsstack.enabler.IBaseContext;
import com.android.imsstack.enabler.IContext;
import com.android.imsstack.enabler.aos.IAosRegistration;
import com.android.imsstack.enabler.aos.IAosRegistrationListener;
import com.android.imsstack.imsservice.mmtel.base.IMmTelFeatureCapabilityListener;

import static org.junit.Assert.assertEquals;
import static org.mockito.ArgumentMatchers.eq;
import static org.mockito.Mockito.verify;
import static org.mockito.Mockito.when;

import org.junit.After;
import org.junit.Before;
import org.junit.Test;
import org.junit.runner.RunWith;
import org.junit.runners.JUnit4;
import org.mockito.Mock;
import org.mockito.MockitoAnnotations;

import java.util.ArrayList;
import java.util.List;

@RunWith(JUnit4.class)
public class ImsRegistrationTrackerTest {
    private ImsRegistrationTracker mRegTracker;
    private MmTelFeature.MmTelCapabilities mMmTelCapabilities;
    private ImsFeatureManager mFeatureManager;
    private IAosRegistrationListener mAosRegListener = null;
    private MockIAosRegistration mAosReg;

    @Mock IBaseContext mMockBaseContext;
    @Mock IContext mMockContext;
    @Mock ImsRegistrationTracker.CapabilityUpdateListener mMockCapabilityListener;
    @Mock IMmTelFeatureCapabilityListener mMockFeatureCapabilityListener;

    @Before
    public void setUp() {
        MockitoAnnotations.initMocks(this);

        when(mMockContext.getSlotId()).thenReturn(0);
        when(mMockBaseContext.getDefaultLooper()).thenReturn(Looper.getMainLooper());

        mFeatureManager = new ImsFeatureManager(mMockBaseContext, mMockFeatureCapabilityListener);
        mAosReg = new MockIAosRegistration();
        mRegTracker = new FakeImsRegistrationTracker(mMockContext,
                new ImsRegistrationImpl());
        mRegTracker.setCapabilityUpdateListener(mMockCapabilityListener);
        mAosRegListener = mAosReg.getListener();
        mFeatureManager.setRegistrationTracker(mRegTracker);
        mMmTelCapabilities = new MmTelFeature.MmTelCapabilities(0);
    }

    @After
    public void tearDown() {
        mRegTracker.dispose();
    }

    @Test
    public void testnotifyRegistering() {
        int features = (IAosRegistrationListener.FeatureTagMask.MMTEL
                | IAosRegistrationListener.FeatureTagMask.VIDEO);

        mAosRegListener.notifyRegistering(
                IAosRegistrationListener.NetworkType.LTE, features, new ArraySet<String>());

        assertEquals(IAosRegistrationListener.NetworkType.LTE,
                mRegTracker.getRegisteredNetworkType());
        assertEquals(false, mRegTracker.isRegistered());
        assertEquals(IAosRegistrationListener.FeatureTagMask.NONE,
                mRegTracker.getRegisteredFeatures());

        mAosRegListener.notifyRegistering(
                IAosRegistrationListener.NetworkType.IWLAN, features, new ArraySet<String>());

        assertEquals(IAosRegistrationListener.NetworkType.LTE,
                mRegTracker.getRegisteredNetworkType());
        assertEquals(false, mRegTracker.isRegistered());
        assertEquals(IAosRegistrationListener.FeatureTagMask.NONE,
                mRegTracker.getRegisteredFeatures());
    }

    @Test
    public void testnotifyRegistered_lte() {
        int features = (IAosRegistrationListener.FeatureTagMask.MMTEL
                | IAosRegistrationListener.FeatureTagMask.VIDEO
                | IAosRegistrationListener.FeatureTagMask.SMSIP);

        mAosRegListener.notifyRegistered(
                IAosRegistrationListener.NetworkType.LTE, features, new ArraySet<String>());

        assertEquals(true, mRegTracker.isRegistered());
        assertEquals(IAosRegistrationListener.NetworkType.LTE,
                mRegTracker.getRegisteredNetworkType());
        assertEquals(features, mRegTracker.getRegisteredFeatures());
        assertEquals(true, mRegTracker.isCallRegistered());
        assertEquals(true, mRegTracker.isCallVideoRegistered());
        assertEquals(true, mRegTracker.isSmsRegistered());
        assertEquals(true, mRegTracker.isCallVoiceAndVideoRegistered());

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
    public void testnotifyRegistered_wlan() {
        int features = (IAosRegistrationListener.FeatureTagMask.MMTEL
                | IAosRegistrationListener.FeatureTagMask.SMSIP);

        mAosRegListener.notifyRegistered(IAosRegistrationListener.NetworkType.IWLAN,
                features, new ArraySet<String>());

        assertEquals(true, mRegTracker.isRegistered());
        assertEquals(IAosRegistrationListener.NetworkType.IWLAN,
                mRegTracker.getRegisteredNetworkType());
        assertEquals(features, mRegTracker.getRegisteredFeatures());
        assertEquals(true, mRegTracker.isCallRegistered());
        assertEquals(false, mRegTracker.isCallVideoRegistered());
        assertEquals(true, mRegTracker.isSmsRegistered());
        assertEquals(false, mRegTracker.isCallVoiceAndVideoRegistered());

        int capabilities = MmTelFeature.MmTelCapabilities.CAPABILITY_TYPE_VOICE
                | MmTelFeature.MmTelCapabilities.CAPABILITY_TYPE_SMS;
        int removeCapabilities = MmTelFeature.MmTelCapabilities.CAPABILITY_TYPE_VIDEO
                | MmTelFeature.MmTelCapabilities.CAPABILITY_TYPE_UT;

        mMmTelCapabilities.addCapabilities(capabilities);
        mMmTelCapabilities.removeCapabilities(removeCapabilities);
        verify(mMockFeatureCapabilityListener).onFeatureCapabilityChanged(
                eq(mMmTelCapabilities));
    }

    @Test
    public void testnotifyDeRegistered() {
        mAosRegListener.notifyDeregistered(
                IAosRegistrationListener.ReasonCode.CODE_REGISTRATION_ERROR);
        assertEquals(false, mRegTracker.isRegistered());
        assertEquals(IAosRegistrationListener.FeatureTagMask.NONE,
                mRegTracker.getRegisteredFeatures());
        assertEquals(false, mRegTracker.isCallRegistered());
        assertEquals(false, mRegTracker.isCallVideoRegistered());
        assertEquals(false, mRegTracker.isSmsRegistered());
        assertEquals(false, mRegTracker.isCallVoiceAndVideoRegistered());

        int capabilities = MmTelFeature.MmTelCapabilities.CAPABILITY_TYPE_NONE;
        int removeCapabilities = MmTelFeature.MmTelCapabilities.CAPABILITY_TYPE_VOICE
                | MmTelFeature.MmTelCapabilities.CAPABILITY_TYPE_VIDEO
                | MmTelFeature.MmTelCapabilities.CAPABILITY_TYPE_SMS;

        mMmTelCapabilities.addCapabilities(capabilities);
        mMmTelCapabilities.removeCapabilities(removeCapabilities);
        verify(mMockFeatureCapabilityListener).onFeatureCapabilityChanged(
                eq(mMmTelCapabilities));
    }
    @Test
    public void testchangeCapabilities() {
        List<CapabilityPair> enableCapabilities = new ArrayList<>();
        enableCapabilities.add(new CapabilityPair(IAosRegistrationListener.NetworkType.LTE,
                IAosRegistrationListener.Capability.VOICE));
        enableCapabilities.add(new CapabilityPair(IAosRegistrationListener.NetworkType.IWLAN,
                IAosRegistrationListener.Capability.VOICE));
        enableCapabilities.add(new CapabilityPair(IAosRegistrationListener.NetworkType.NR,
                IAosRegistrationListener.Capability.VOICE));

        List<CapabilityPair> disableCapabilities = new ArrayList<>();
        disableCapabilities.add(new CapabilityPair(IAosRegistrationListener.NetworkType.NR,
                IAosRegistrationListener.Capability.VIDEO));
        disableCapabilities.add(new CapabilityPair(IAosRegistrationListener.NetworkType.LTE,
                IAosRegistrationListener.Capability.VIDEO));
        disableCapabilities.add(new CapabilityPair(IAosRegistrationListener.NetworkType.IWLAN,
                IAosRegistrationListener.Capability.VIDEO));

        mRegTracker.changeCapabilities(enableCapabilities, disableCapabilities);
        mRegTracker.changeCapabilities(new ArrayList<>(), new ArrayList<>());
        mRegTracker.changeCapabilities(enableCapabilities, null);
    }

    @Test
    public void testrefreshCallRegistrationState() {
        mRegTracker.refreshCallRegistrationState();
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
