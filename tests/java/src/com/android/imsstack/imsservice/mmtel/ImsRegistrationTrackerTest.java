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

import android.content.Context;
import android.content.SharedPreferences;
import android.net.Uri;
import android.os.Looper;
import android.os.Message;
import android.telephony.CarrierConfigManager;
import android.telephony.DataFailCause;
import android.telephony.TelephonyManager;
import android.telephony.ims.ImsMmTelManager;
import android.telephony.ims.ProvisioningManager;
import android.telephony.ims.feature.CapabilityChangeRequest.CapabilityPair;
import android.telephony.ims.feature.ImsFeature;
import android.telephony.ims.feature.MmTelFeature;
import android.test.mock.MockContentResolver;
import android.util.ArraySet;

import com.android.imsstack.ContextFixture;
import com.android.imsstack.core.ICommonPackageListener;
import com.android.imsstack.core.agents.ConfigInterface;
import com.android.imsstack.core.agents.dcmif.IDcNetWatcher;
import com.android.imsstack.core.config.CarrierConfig;
import com.android.imsstack.enabler.IBaseContext;
import com.android.imsstack.enabler.IContext;
import com.android.imsstack.enabler.aos.IAosRegistration;
import com.android.imsstack.enabler.aos.IAosRegistration.CapabilityPairs;
import com.android.imsstack.enabler.aos.IAosRegistrationListener;
import com.android.imsstack.imsservice.mmtel.base.IMmTelFeatureCapabilityListener;
import com.android.imsstack.util.AppContext;
import com.android.imsstack.util.MessageExecutor;

import static org.junit.Assert.assertEquals;
import static org.junit.Assert.assertNotNull;
import static org.junit.Assert.assertTrue;
import static org.mockito.ArgumentMatchers.anyInt;
import static org.mockito.ArgumentMatchers.anyString;
import static org.mockito.ArgumentMatchers.eq;
import static org.mockito.Mockito.times;
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
import java.util.Set;
import java.util.concurrent.CountDownLatch;
import java.util.concurrent.TimeUnit;

@RunWith(JUnit4.class)
public class ImsRegistrationTrackerTest {
    private ImsRegistrationTracker mRegTracker;
    private MmTelFeature.MmTelCapabilities mMmTelCapabilities;
    private ImsFeatureManager mFeatureManager;
    private IAosRegistrationListener mAosRegListener = null;
    private MockIAosRegistration mAosReg;
    private ContextFixture mContextFixture;
    private MockContentResolver mContentResolver;

    @Mock CarrierConfig mMockCarrierConfig;
    @Mock ConfigInterface mMockConfigInterface;
    @Mock IDcNetWatcher mMockIDcNetWatcher;
    @Mock IBaseContext mMockBaseContext;
    @Mock IContext mMockContext;
    @Mock ImsRegistrationTracker.CapabilityUpdateListener mMockCapabilityListener;
    @Mock IMmTelFeatureCapabilityListener mMockFeatureCapabilityListener;
    @Mock Context mContext;
    @Mock MessageExecutor mExecutor;
    @Mock SharedPreferences mSp;
    @Mock SharedPreferences.Editor mSpEditor;
    @Mock TelephonyManager mTelephonyManager;

    @Before
    public void setUp() {
        MockitoAnnotations.initMocks(this);
        mContextFixture = new ContextFixture();
        AppContext.init(mContextFixture.getTestDouble());
        mContentResolver = new MockContentResolver();

        when(mMockContext.getSlotId()).thenReturn(0);
        when(mMockContext.getPhoneId()).thenReturn(0);
        when(mMockContext.getContext()).thenReturn(mContext);
        when(mMockContext.getDefaultLooper()).thenReturn(Looper.getMainLooper());
        when(mMockBaseContext.getDefaultLooper()).thenReturn(Looper.getMainLooper());
        when(mMockConfigInterface.getCarrierConfig()).thenReturn(mMockCarrierConfig);
        when(mContext.getSharedPreferences(anyString(), anyInt())).thenReturn(mSp);
        when(mSp.edit()).thenReturn(mSpEditor);
        when(mContext.getContentResolver()).thenReturn(mContentResolver);
        when(mMockCarrierConfig.getBoolean(eq(CarrierConfig.Assets
                .KEY_SUPPORT_VOWIFI_CAPABILITY_WHEN_WIFI_ONLY_OR_PREFERRED_IN_ROAMING_BOOL)))
                .thenReturn(true);
        when(mMockCarrierConfig.getBoolean(eq(CarrierConfigManager
                .KEY_IGNORE_DATA_ENABLED_CHANGED_FOR_VIDEO_CALLS)))
                .thenReturn(false);

        when(mMockIDcNetWatcher.isVoiceRoaming()).thenReturn(false);
        when(mMockIDcNetWatcher.isRoaming()).thenReturn(false);

        mFeatureManager = new ImsFeatureManager(mMockBaseContext, mMockFeatureCapabilityListener);
        mAosReg = new MockIAosRegistration();
        mRegTracker = new FakeImsRegistrationTracker(mMockContext,
                new ImsRegistrationImpl());
        mRegTracker.setCapabilityUpdateListener(mMockCapabilityListener);
        mAosRegListener = mAosReg.getListener();
        mFeatureManager.setRegistrationTracker(mRegTracker);
        mMmTelCapabilities = new MmTelFeature.MmTelCapabilities(0);
        mTelephonyManager = mContextFixture.getTestDouble()
                .getSystemService(TelephonyManager.class);
        when(AppContext.getTelephonyManager(0)).thenReturn(mTelephonyManager);
        when(mTelephonyManager.isDataEnabled()).thenReturn(true);
    }

    @After
    public void tearDown() {
        mRegTracker.dispose();
        mContentResolver = null;
        mContextFixture = null;
        AppContext.deinit();
    }

    @Test
    public void testnotifyRegistering() {
        int features = (IAosRegistrationListener.FeatureTagMask.MMTEL
                | IAosRegistrationListener.FeatureTagMask.VIDEO);

        Set<String> tags = new ArraySet<>();
        tags.add("+g.3gpp.smsip");
        tags.add("video");

        mAosRegListener.notifyRegistering(
                IAosRegistrationListener.NetworkType.LTE, features, tags);

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

        Set<String> tags = new ArraySet<>();
        tags.add("+g.3gpp.smsip");

        mAosRegListener.notifyRegistered(IAosRegistrationListener.NetworkType.IWLAN,
                features, tags);

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
        mAosRegListener.notifyDeregistered(IAosRegistrationListener.NetworkType.LTE,
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
    public void testnotifyDeRegistered_WithError() {
        mAosRegListener.notifyDeregistered(IAosRegistrationListener.NetworkType.LTE,
                IAosRegistrationListener.ReasonCode.CODE_REGISTRATION_ERROR_BY_MISSING_911_ADDRESS);
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
    public void testnotifyDeRegistered_WithPLMNError() {
        mAosRegListener.notifyDeregistered(IAosRegistrationListener.NetworkType.LTE,
                IAosRegistrationListener.ReasonCode.CODE_PLMN_BLOCK);
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
    public void testnotifyTechnologyChangeFailed() {
        assertNotNull(mRegTracker.getRegistration());
        mAosRegListener.notifyTechnologyChangeFailed(IAosRegistrationListener.NetworkType.NONE,
                IAosRegistrationListener.ReasonCode.CODE_REGISTRATION_ERROR);

        assertNotNull(mRegTracker.getRegistration());
        mAosRegListener.notifyTechnologyChangeFailed(IAosRegistrationListener.NetworkType.LTE,
                DataFailCause.IWLAN_IKEV2_AUTH_FAILURE);
    }

    @Test
    public void testnotifyAssociatedUriChanged() {
        Uri[] uris = new Uri[2];
        uris[0] = Uri.parse("1111@vzw.com");
        uris[1] = Uri.parse("2222@vzw.com");

        assertNotNull(mRegTracker.getRegistration());
        mAosRegListener.notifyAssociatedUriChanged(uris);
    }

    @Test
    public void testnotifyCapabilitiesUpdateFailed_CrossSim() {
        mAosRegListener.notifyCapabilitiesUpdateFailed(IAosRegistrationListener.Capability
                .CALL_COMPOSER, IAosRegistrationListener.NetworkType.CROSS_SIM,
                IAosRegistrationListener.ReasonCode.CODE_REGISTRATION_ERROR);

        verify(mMockCapabilityListener).onCapabilitiesUpdateFailed(
                MmTelFeature.MmTelCapabilities.CAPABILITY_TYPE_CALL_COMPOSER,
                ImsRegistrationImpl.REGISTRATION_TECH_CROSS_SIM,
                ImsFeature.CAPABILITY_ERROR_GENERIC);
    }

    @Test
    public void testnotifyCapabilitiesUpdateFailed_Nr() {
        mAosRegListener.notifyCapabilitiesUpdateFailed(IAosRegistrationListener.Capability.SMS,
                IAosRegistrationListener.NetworkType.NR,
                IAosRegistrationListener.ReasonCode.CODE_REGISTRATION_ERROR);

        verify(mMockCapabilityListener).onCapabilitiesUpdateFailed(
                MmTelFeature.MmTelCapabilities.CAPABILITY_TYPE_SMS,
                ImsRegistrationImpl.REGISTRATION_TECH_NR, ImsFeature.CAPABILITY_ERROR_GENERIC);
    }

    @Test
    public void testnotifyCapabilitiesUpdateFailed_Lte() {
        mAosRegListener.notifyCapabilitiesUpdateFailed(IAosRegistrationListener.Capability.UT,
                IAosRegistrationListener.NetworkType.LTE,
                IAosRegistrationListener.ReasonCode.CODE_REGISTRATION_ERROR);

        verify(mMockCapabilityListener).onCapabilitiesUpdateFailed(
                MmTelFeature.MmTelCapabilities.CAPABILITY_TYPE_UT,
                ImsRegistrationImpl.REGISTRATION_TECH_LTE, ImsFeature.CAPABILITY_ERROR_GENERIC);
    }

    @Test
    public void testnotifyCapabilitiesUpdateFailed_Voice() {
        mAosRegListener.notifyCapabilitiesUpdateFailed(IAosRegistrationListener.Capability.VOICE,
                IAosRegistrationListener.NetworkType.LTE,
                IAosRegistrationListener.ReasonCode.CODE_REGISTRATION_ERROR);

        verify(mMockCapabilityListener).onCapabilitiesUpdateFailed(
                MmTelFeature.MmTelCapabilities.CAPABILITY_TYPE_VOICE,
                ImsRegistrationImpl.REGISTRATION_TECH_LTE, ImsFeature.CAPABILITY_ERROR_GENERIC);
    }

    @Test
    public void testnotifyCapabilitiesUpdateFailed_Video() {
        mAosRegListener.notifyCapabilitiesUpdateFailed(IAosRegistrationListener.Capability.VIDEO,
                IAosRegistrationListener.NetworkType.LTE,
                IAosRegistrationListener.ReasonCode.CODE_REGISTRATION_ERROR);

        verify(mMockCapabilityListener).onCapabilitiesUpdateFailed(
                MmTelFeature.MmTelCapabilities.CAPABILITY_TYPE_VIDEO,
                ImsRegistrationImpl.REGISTRATION_TECH_LTE, ImsFeature.CAPABILITY_ERROR_GENERIC);
    }

    @Test
    public void testnotifyCapabilitiesUpdateFailed_None() {
        mAosRegListener.notifyCapabilitiesUpdateFailed(IAosRegistrationListener.Capability.NONE,
                IAosRegistrationListener.NetworkType.CROSS_SIM,
                IAosRegistrationListener.ReasonCode.CODE_REGISTRATION_ERROR);

        verify(mMockCapabilityListener).onCapabilitiesUpdateFailed(
                MmTelFeature.MmTelCapabilities.CAPABILITY_TYPE_NONE,
                ImsRegistrationImpl.REGISTRATION_TECH_CROSS_SIM,
                ImsFeature.CAPABILITY_ERROR_GENERIC);
    }

    @Test
    public void testchangeCapabilities_null() {
        assertEquals(null, mRegTracker.createCapabilityPairsFromCapabilities());

        mRegTracker.changeCapabilities(null, null);
        assertEquals(null, mRegTracker.createCapabilityPairsFromCapabilities());

        mRegTracker.changeCapabilities(new ArrayList<>(), new ArrayList<>());
        assertEquals(null, mRegTracker.createCapabilityPairsFromCapabilities());

        List<CapabilityPair> disableCapabilities = new ArrayList<>();
        disableCapabilities.add(new CapabilityPair(
                MmTelFeature.MmTelCapabilities.CAPABILITY_TYPE_VOICE,
                ImsRegistrationImpl.REGISTRATION_TECH_LTE));
        disableCapabilities.add(new CapabilityPair(
                MmTelFeature.MmTelCapabilities.CAPABILITY_TYPE_VOICE,
                ImsRegistrationImpl.REGISTRATION_TECH_IWLAN));

        mRegTracker.changeCapabilities(new ArrayList<>(), disableCapabilities);
        assertEquals(null, mRegTracker.createCapabilityPairsFromCapabilities());

    }

    @Test
    public void testchangeCapabilities_VoWifiBasedOnMode() {
        ImsServiceManager oldServiceManager = ImsServiceManager.getDefault();
        ImsServiceManager.setDefault(new ImsServiceManager(mContext, mExecutor));

        when(mMockIDcNetWatcher.isVoiceRoaming()).thenReturn(true);

        List<CapabilityPair> enableCapabilities = new ArrayList<>();
        enableCapabilities.add(new CapabilityPair(
                MmTelFeature.MmTelCapabilities.CAPABILITY_TYPE_VOICE,
                ImsRegistrationImpl.REGISTRATION_TECH_IWLAN));

        CapabilityPairs capabilityPairs = new CapabilityPairs(
                IAosRegistrationListener.NetworkType.IWLAN,
                IAosRegistrationListener.Capability.VOICE);

        when(mSp.getInt(eq("VOICE_OVER_WIFI_ROAMING_ENABLED_OVERRIDE"), anyInt()))
                .thenReturn(ProvisioningManager.PROVISIONING_VALUE_DISABLED);
        when(mSp.getInt(eq("VOICE_OVER_WIFI_MODE_OVERRIDE"), anyInt()))
                .thenReturn(ImsMmTelManager.WIFI_MODE_WIFI_PREFERRED);

        mRegTracker.changeCapabilities(enableCapabilities, new ArrayList<>());
        assertEquals(new CapabilityPairs(), mRegTracker.createCapabilityPairsFromCapabilities());

        when(mSp.getInt(eq("VOICE_OVER_WIFI_ROAMING_ENABLED_OVERRIDE"), anyInt()))
                .thenReturn(ProvisioningManager.PROVISIONING_VALUE_DISABLED);
        when(mSp.getInt(eq("VOICE_OVER_WIFI_MODE_OVERRIDE"), anyInt()))
                .thenReturn(ImsMmTelManager.WIFI_MODE_CELLULAR_PREFERRED);

        mRegTracker.changeCapabilities(enableCapabilities, new ArrayList<>());
        assertEquals(new CapabilityPairs(), mRegTracker.createCapabilityPairsFromCapabilities());

        when(mSp.getInt(eq("VOICE_OVER_WIFI_ROAMING_ENABLED_OVERRIDE"), anyInt()))
                .thenReturn(ProvisioningManager.PROVISIONING_VALUE_ENABLED);
        when(mSp.getInt(eq("VOICE_OVER_WIFI_MODE_OVERRIDE"), anyInt()))
                .thenReturn(ImsMmTelManager.WIFI_MODE_CELLULAR_PREFERRED);

        mRegTracker.changeCapabilities(enableCapabilities, new ArrayList<>());
        assertEquals(new CapabilityPairs(), mRegTracker.createCapabilityPairsFromCapabilities());

        when(mSp.getInt(eq("VOICE_OVER_WIFI_ROAMING_ENABLED_OVERRIDE"), anyInt()))
                .thenReturn(ProvisioningManager.PROVISIONING_VALUE_ENABLED);
        when(mSp.getInt(eq("VOICE_OVER_WIFI_MODE_OVERRIDE"), anyInt()))
                .thenReturn(ImsMmTelManager.WIFI_MODE_WIFI_PREFERRED);

        mRegTracker.changeCapabilities(enableCapabilities, new ArrayList<>());
        assertEquals(capabilityPairs, mRegTracker.createCapabilityPairsFromCapabilities());

        when(mMockIDcNetWatcher.isVoiceRoaming()).thenReturn(false);
        mRegTracker.changeCapabilities(enableCapabilities, new ArrayList<>());
        assertEquals(capabilityPairs, mRegTracker.createCapabilityPairsFromCapabilities());

        ImsServiceManager.setDefault(oldServiceManager);
    }

    @Test
    public void testchangeCapabilities_VideoBasedOnMobileData() {
        when(mMockIDcNetWatcher.isRoaming()).thenReturn(true);
        when(mTelephonyManager.isDataRoamingEnabled()).thenReturn(true);
        when(mTelephonyManager.isDataEnabled()).thenReturn(true);

        List<CapabilityPair> enableCapabilities = new ArrayList<>();
        enableCapabilities.add(new CapabilityPair(
                MmTelFeature.MmTelCapabilities.CAPABILITY_TYPE_VIDEO,
                ImsRegistrationImpl.REGISTRATION_TECH_LTE));

        CapabilityPairs capabilityPairs = new CapabilityPairs(
                IAosRegistrationListener.NetworkType.LTE,
                IAosRegistrationListener.Capability.VIDEO);

        capabilityPairs.addCapability(IAosRegistrationListener.NetworkType.IWLAN,
                IAosRegistrationListener.Capability.VIDEO);

        mRegTracker.changeCapabilities(enableCapabilities, new ArrayList<>());
        assertEquals(capabilityPairs, mRegTracker.createCapabilityPairsFromCapabilities());

        when(mMockIDcNetWatcher.isRoaming()).thenReturn(false);
        when(mTelephonyManager.isDataEnabled()).thenReturn(true);

        mRegTracker.changeCapabilities(enableCapabilities, new ArrayList<>());
        assertEquals(capabilityPairs, mRegTracker.createCapabilityPairsFromCapabilities());

        when(mTelephonyManager.isDataEnabled()).thenReturn(false);

        mRegTracker.changeCapabilities(enableCapabilities, new ArrayList<>());
        assertEquals(new CapabilityPairs(), mRegTracker.createCapabilityPairsFromCapabilities());
    }

    @Test
    public void testchangeCapabilities_OnIgnoreDataEnbledForVideo() {
        when(mMockCarrierConfig.getBoolean(eq(CarrierConfigManager
                .KEY_IGNORE_DATA_ENABLED_CHANGED_FOR_VIDEO_CALLS)))
                .thenReturn(true);

        when(mMockIDcNetWatcher.isRoaming()).thenReturn(true);
        when(mTelephonyManager.isDataRoamingEnabled()).thenReturn(true);
        when(mTelephonyManager.isDataEnabled()).thenReturn(true);

        List<CapabilityPair> enableCapabilities = new ArrayList<>();
        enableCapabilities.add(new CapabilityPair(
                MmTelFeature.MmTelCapabilities.CAPABILITY_TYPE_VIDEO,
                ImsRegistrationImpl.REGISTRATION_TECH_LTE));

        CapabilityPairs capabilityPairs = new CapabilityPairs(
                IAosRegistrationListener.NetworkType.LTE,
                IAosRegistrationListener.Capability.VIDEO);

        capabilityPairs.addCapability(IAosRegistrationListener.NetworkType.IWLAN,
                IAosRegistrationListener.Capability.VIDEO);

        mRegTracker.changeCapabilities(enableCapabilities, new ArrayList<>());
        assertEquals(capabilityPairs, mRegTracker.createCapabilityPairsFromCapabilities());

        when(mMockIDcNetWatcher.isRoaming()).thenReturn(false);
        when(mTelephonyManager.isDataEnabled()).thenReturn(true);

        mRegTracker.changeCapabilities(enableCapabilities, new ArrayList<>());
        assertEquals(capabilityPairs, mRegTracker.createCapabilityPairsFromCapabilities());

        when(mTelephonyManager.isDataEnabled()).thenReturn(false);

        mRegTracker.changeCapabilities(enableCapabilities, new ArrayList<>());
        assertEquals(capabilityPairs, mRegTracker.createCapabilityPairsFromCapabilities());
    }

    @Test
    public void testcreateCapabilities_OnRoaming() {
        ImsServiceManager oldServiceManager = ImsServiceManager.getDefault();
        ImsServiceManager.setDefault(new ImsServiceManager(mContext, mExecutor));

        when(mMockIDcNetWatcher.isVoiceRoaming()).thenReturn(false);
        when(mMockIDcNetWatcher.isRoaming()).thenReturn(false);

        List<CapabilityPair> enableCapabilities = new ArrayList<>();
        enableCapabilities.add(new CapabilityPair(
                MmTelFeature.MmTelCapabilities.CAPABILITY_TYPE_VOICE,
                ImsRegistrationImpl.REGISTRATION_TECH_IWLAN));
        enableCapabilities.add(new CapabilityPair(
                MmTelFeature.MmTelCapabilities.CAPABILITY_TYPE_VOICE,
                ImsRegistrationImpl.REGISTRATION_TECH_LTE));

        CapabilityPairs capabilityPairs = new CapabilityPairs(
                IAosRegistrationListener.NetworkType.LTE,
                IAosRegistrationListener.Capability.VOICE);

        capabilityPairs.addCapability(IAosRegistrationListener.NetworkType.IWLAN,
                IAosRegistrationListener.Capability.VOICE);

        when(mSp.getInt(eq("VOICE_OVER_WIFI_ROAMING_ENABLED_OVERRIDE"), anyInt()))
                .thenReturn(ProvisioningManager.PROVISIONING_VALUE_DISABLED);
        when(mSp.getInt(eq("VOICE_OVER_WIFI_MODE_OVERRIDE"), anyInt()))
                .thenReturn(ImsMmTelManager.WIFI_MODE_CELLULAR_PREFERRED);

        mRegTracker.changeCapabilities(enableCapabilities, new ArrayList<>());
        assertEquals(capabilityPairs, mRegTracker.createCapabilityPairsFromCapabilities());

        when(mMockIDcNetWatcher.isVoiceRoaming()).thenReturn(true);
        when(mMockIDcNetWatcher.isRoaming()).thenReturn(true);
        capabilityPairs = new CapabilityPairs(IAosRegistrationListener.NetworkType.LTE,
                IAosRegistrationListener.Capability.VOICE);

        int eventVoiceRoaming = 1;
        mRegTracker.getMessageHandler().handleMessage(
                Message.obtain(mRegTracker.getMessageHandler(), eventVoiceRoaming));
        assertEquals(capabilityPairs, mRegTracker.createCapabilityPairsFromCapabilities());

        ImsServiceManager.setDefault(oldServiceManager);
    }

    @Test
    public void testchangeCapabilities_all() {
        List<CapabilityPair> enableCapabilities = new ArrayList<>();
        enableCapabilities.add(new CapabilityPair(
                MmTelFeature.MmTelCapabilities.CAPABILITY_TYPE_VOICE,
                ImsRegistrationImpl.REGISTRATION_TECH_LTE));
        enableCapabilities.add(new CapabilityPair(
                MmTelFeature.MmTelCapabilities.CAPABILITY_TYPE_SMS,
                ImsRegistrationImpl.REGISTRATION_TECH_IWLAN));
        enableCapabilities.add(new CapabilityPair(
                MmTelFeature.MmTelCapabilities.CAPABILITY_TYPE_VOICE,
                ImsRegistrationImpl.REGISTRATION_TECH_IWLAN));
        enableCapabilities.add(new CapabilityPair(
                MmTelFeature.MmTelCapabilities.CAPABILITY_TYPE_VIDEO,
                ImsRegistrationImpl.REGISTRATION_TECH_NR));
        enableCapabilities.add(new CapabilityPair(
                MmTelFeature.MmTelCapabilities.CAPABILITY_TYPE_UT,
                ImsRegistrationImpl.REGISTRATION_TECH_LTE));
        enableCapabilities.add(new CapabilityPair(
                MmTelFeature.MmTelCapabilities.CAPABILITY_TYPE_CALL_COMPOSER,
                ImsRegistrationImpl.REGISTRATION_TECH_CROSS_SIM));

        List<CapabilityPair> disableCapabilities = new ArrayList<>();
        disableCapabilities.add(new CapabilityPair(
                MmTelFeature.MmTelCapabilities.CAPABILITY_TYPE_VIDEO,
                ImsRegistrationImpl.REGISTRATION_TECH_LTE));
        disableCapabilities.add(new CapabilityPair(
                MmTelFeature.MmTelCapabilities.CAPABILITY_TYPE_VIDEO,
                ImsRegistrationImpl.REGISTRATION_TECH_IWLAN));

        CapabilityPairs capabilityPairs = new CapabilityPairs();
        capabilityPairs.addCapability(IAosRegistrationListener.NetworkType.LTE,
                IAosRegistrationListener.Capability.VOICE);
        capabilityPairs.addCapability(IAosRegistrationListener.NetworkType.IWLAN,
                IAosRegistrationListener.Capability.SMS
                | IAosRegistrationListener.Capability.VOICE
                | IAosRegistrationListener.Capability.VIDEO);
        capabilityPairs.addCapability(IAosRegistrationListener.NetworkType.NR,
                IAosRegistrationListener.Capability.VIDEO);
        capabilityPairs.addCapability(IAosRegistrationListener.NetworkType.LTE,
                IAosRegistrationListener.Capability.UT);
        capabilityPairs.addCapability(IAosRegistrationListener.NetworkType.CROSS_SIM,
                IAosRegistrationListener.Capability.CALL_COMPOSER);

        mRegTracker.changeCapabilities(enableCapabilities, disableCapabilities);
        assertEquals(capabilityPairs, mRegTracker.createCapabilityPairsFromCapabilities());

        mRegTracker.changeCapabilities(enableCapabilities, null);
        assertEquals(capabilityPairs, mRegTracker.createCapabilityPairsFromCapabilities());
    }

    @Test
    public void testchangeCapabilities_None() {
        List<CapabilityPair> enableCapabilities = new ArrayList<>();
        enableCapabilities.add(new CapabilityPair(
                MmTelFeature.MmTelCapabilities.CAPABILITY_TYPE_VOICE,
                ImsRegistrationImpl.REGISTRATION_TECH_LTE));
        enableCapabilities.add(new CapabilityPair(
                MmTelFeature.MmTelCapabilities.CAPABILITY_TYPE_SMS,
                ImsRegistrationImpl.REGISTRATION_TECH_IWLAN));
        enableCapabilities.add(new CapabilityPair(
                MmTelFeature.MmTelCapabilities.CAPABILITY_TYPE_NONE,
                ImsRegistrationImpl.REGISTRATION_TECH_NONE));

        List<CapabilityPair> disableCapabilities = new ArrayList<>();
        disableCapabilities.add(new CapabilityPair(
                MmTelFeature.MmTelCapabilities.CAPABILITY_TYPE_UT,
                ImsRegistrationImpl.REGISTRATION_TECH_LTE));
        disableCapabilities.add(new CapabilityPair(
                MmTelFeature.MmTelCapabilities.CAPABILITY_TYPE_CALL_COMPOSER,
                ImsRegistrationImpl.REGISTRATION_TECH_CROSS_SIM));
        mRegTracker.changeCapabilities(enableCapabilities, disableCapabilities);


        CapabilityPairs capabilityPairs = new CapabilityPairs();
        capabilityPairs.addCapability(IAosRegistrationListener.NetworkType.LTE,
                IAosRegistrationListener.Capability.VOICE);

        capabilityPairs.addCapability(IAosRegistrationListener.NetworkType.IWLAN,
                IAosRegistrationListener.Capability.SMS);

        capabilityPairs.addCapability(IAosRegistrationListener.NetworkType.NONE,
                IAosRegistrationListener.Capability.NONE);
        assertEquals(capabilityPairs, mRegTracker.createCapabilityPairsFromCapabilities());
    }

    @Test
    public void testrefreshCallRegistrationState() {
        int features = (IAosRegistrationListener.FeatureTagMask.MMTEL
                | IAosRegistrationListener.FeatureTagMask.SMSIP);

        Set<String> tags = new ArraySet<>();
        tags.add("+g.3gpp.smsip");

        mAosRegListener.notifyRegistered(IAosRegistrationListener.NetworkType.IWLAN,
                features, tags);
        assertEquals(IAosRegistrationListener.NetworkType.IWLAN,
                mRegTracker.getRegisteredNetworkType());
        mRegTracker.refreshCallRegistrationState();
        assertEquals(IAosRegistrationListener.NetworkType.LTE,
                mRegTracker.getRegisteredNetworkType());
    }

    @Test
    public void testupdateSipDelegateRegistration() throws InterruptedException {
        ImsRegistrationImpl regImpl = mRegTracker.getRegistration();
        assertNotNull(regImpl);

        CountDownLatch lock = new CountDownLatch(1);
        mAosReg.setCountDownLatch(lock);
        regImpl.updateSipDelegateRegistration();
        assertTrue(lock.await(1, TimeUnit.SECONDS));
    }

    @Test
    public void testtriggerSipDelegateDeregistration() throws InterruptedException {
        ImsRegistrationImpl regImpl = mRegTracker.getRegistration();
        assertNotNull(regImpl);

        CountDownLatch lock = new CountDownLatch(1);
        mAosReg.setCountDownLatch(lock);
        regImpl.triggerSipDelegateDeregistration();
        assertTrue(lock.await(1, TimeUnit.SECONDS));
    }

    @Test
    public void testtriggerFullNetworkRegistration() throws InterruptedException {
        ImsRegistrationImpl regImpl = mRegTracker.getRegistration();
        assertNotNull(regImpl);

        CountDownLatch lock = new CountDownLatch(1);
        mAosReg.setCountDownLatch(lock);
        regImpl.triggerFullNetworkRegistration(488, null);
        assertTrue(lock.await(1, TimeUnit.SECONDS));
    }

    @Test
    public void testtriggerDeRegistration() {
        ImsRegistrationImpl regImpl = mRegTracker.getRegistration();
        assertNotNull(regImpl);
        regImpl.triggerDeregistration(ImsRegistrationImpl.REASON_SIM_REMOVED);
    }

    @Test
    public void testOnCommonPackageReady() {
        ICommonPackageListener listener = mRegTracker.getICommonPackageListener();
        assertNotNull(listener);

        listener.onCommonPackageReady(0);
        verify(mMockIDcNetWatcher, times(1)).registerForRoamingStateChanged(
                mRegTracker.getMessageHandler(), 1, null);
    }

    @Test
    public void testOnCommonPackageStop() {
        ICommonPackageListener listener = mRegTracker.getICommonPackageListener();
        assertNotNull(listener);

        listener.onCommonPackageStop(0);
        verify(mMockIDcNetWatcher).unregisterForRoamingStateChanged(
                mRegTracker.getMessageHandler());
    }

    @Test
    public void testRoamingObserver() {
        assertNotNull(mRegTracker.getMessageHandler());
        Message msg = Message.obtain();
        msg.what = 1;
        msg.obj = null;
        mRegTracker.getMessageHandler().handleMessage(msg);

        assertEquals(null, mRegTracker.createCapabilityPairsFromCapabilities());
    }

    @Test
    public void testConfigListener() {
        ImsServiceManager oldServiceManager = ImsServiceManager.getDefault();
        ImsServiceManager.setDefault(new ImsServiceManager(mContext, mExecutor));
        when(mMockIDcNetWatcher.isVoiceRoaming()).thenReturn(true);

        List<CapabilityPair> enableCapabilities = new ArrayList<>();
        enableCapabilities.add(new CapabilityPair(
                MmTelFeature.MmTelCapabilities.CAPABILITY_TYPE_VOICE,
                ImsRegistrationImpl.REGISTRATION_TECH_IWLAN));

        when(mSp.getInt(eq("VOICE_OVER_WIFI_ROAMING_ENABLED_OVERRIDE"), anyInt()))
                .thenReturn(ProvisioningManager.PROVISIONING_VALUE_ENABLED);
        when(mSp.getInt(eq("VOICE_OVER_WIFI_MODE_OVERRIDE"), anyInt()))
                .thenReturn(ImsMmTelManager.WIFI_MODE_WIFI_PREFERRED);

        mRegTracker.changeCapabilities(enableCapabilities, null);

        CapabilityPairs capabilityPairs = new CapabilityPairs();
        capabilityPairs.addCapability(IAosRegistrationListener.NetworkType.IWLAN,
                IAosRegistrationListener.Capability.VOICE);
        assertEquals(capabilityPairs, mRegTracker.createCapabilityPairsFromCapabilities());

        ImsServiceRecord isr = ImsServiceManager.getServiceRecord(0);
        ImsConfigImpl configImpl = isr.getConfig();

        configImpl.setConfig(
                ProvisioningManager.KEY_VOICE_OVER_WIFI_MODE_OVERRIDE,
                ImsMmTelManager.WIFI_MODE_CELLULAR_PREFERRED);

        configImpl.setConfig(
                ProvisioningManager.KEY_VOICE_OVER_WIFI_ROAMING_ENABLED_OVERRIDE,
                ProvisioningManager.PROVISIONING_VALUE_DISABLED);

        when(mSp.getInt(eq("VOICE_OVER_WIFI_ROAMING_ENABLED_OVERRIDE"), anyInt()))
                .thenReturn(ProvisioningManager.PROVISIONING_VALUE_DISABLED);
        when(mSp.getInt(eq("VOICE_OVER_WIFI_MODE_OVERRIDE"), anyInt()))
                .thenReturn(ImsMmTelManager.WIFI_MODE_CELLULAR_PREFERRED);

        assertEquals(new CapabilityPairs(), mRegTracker.createCapabilityPairsFromCapabilities());

        ImsServiceManager.setDefault(oldServiceManager);
    }

    private class FakeImsRegistrationTracker extends ImsRegistrationTracker {
        FakeImsRegistrationTracker(IContext context, ImsRegistrationImpl regImpl) {
            super(context, regImpl);
        }

        @Override
        protected IAosRegistration getIAosRegistration(int slotId) {
            return mAosReg;
        }

        @Override
        protected ConfigInterface getConfigInterface(int slotId) {
            return mMockConfigInterface;
        }

        @Override
        protected IDcNetWatcher getDcNetWatcher(int slotId) {
            return mMockIDcNetWatcher;
        }
    }
}
