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

import static com.android.imsstack.base.TestAppContext.SLOT0;

import static org.junit.Assert.assertEquals;
import static org.junit.Assert.assertNotNull;
import static org.junit.Assert.assertFalse;
import static org.junit.Assert.assertTrue;
import static org.mockito.ArgumentMatchers.any;
import static org.mockito.ArgumentMatchers.anyInt;
import static org.mockito.ArgumentMatchers.anyString;
import static org.mockito.ArgumentMatchers.eq;
import static org.mockito.Mockito.never;
import static org.mockito.Mockito.times;
import static org.mockito.Mockito.verify;
import static org.mockito.Mockito.verifyNoInteractions;
import static org.mockito.Mockito.when;

import android.content.SharedPreferences;
import android.net.Uri;
import android.os.Handler;
import android.telephony.AccessNetworkConstants.AccessNetworkType;
import android.telephony.CarrierConfigManager;
import android.telephony.DataFailCause;
import android.telephony.ims.ImsMmTelManager;
import android.telephony.ims.ProvisioningManager;
import android.telephony.ims.feature.CapabilityChangeRequest.CapabilityPair;
import android.telephony.ims.feature.ImsFeature;
import android.telephony.ims.feature.MmTelFeature;
import android.util.ArraySet;

import androidx.test.filters.SmallTest;

import com.android.imsstack.ContextFixture;
import com.android.imsstack.base.AppContext;
import com.android.imsstack.base.ContentProviderProxy.SettingsProxy;
import com.android.imsstack.base.TelephonyManagerProxy;
import com.android.imsstack.base.TestAppContext;
import com.android.imsstack.core.agents.AgentFactory;
import com.android.imsstack.core.agents.ConfigInterface;
import com.android.imsstack.core.agents.SimInterface;
import com.android.imsstack.core.agents.Usat;
import com.android.imsstack.core.agents.UsatInterface;
import com.android.imsstack.core.agents.dcmif.IDcNetWatcher;
import com.android.imsstack.core.config.CarrierConfig;
import com.android.imsstack.enabler.IBaseContext;
import com.android.imsstack.enabler.aos.AosFactory;
import com.android.imsstack.enabler.aos.IAosRegistration;
import com.android.imsstack.enabler.aos.IAosRegistration.CapabilityPairs;
import com.android.imsstack.enabler.aos.IAosRegistrationListener;
import com.android.imsstack.enabler.aos.IAosRegistrationListener.ReasonCode;
import com.android.imsstack.enabler.aos.service.AosService;
import com.android.imsstack.imsservice.mmtel.base.IMmTelFeatureCapabilityListener;
import com.android.imsstack.internal.ImsStackRegistry;
import com.android.imsstack.util.IndentingPrintWriter;
import com.android.imsstack.util.MessageExecutor;

import org.junit.After;
import org.junit.Before;
import org.junit.Test;
import org.junit.runner.RunWith;
import org.junit.runners.JUnit4;
import org.mockito.ArgumentCaptor;
import org.mockito.Mock;
import org.mockito.Mockito;
import org.mockito.MockitoAnnotations;

import java.io.StringWriter;
import java.util.ArrayList;
import java.util.Collections;
import java.util.List;
import java.util.Set;
import java.util.concurrent.CountDownLatch;
import java.util.concurrent.TimeUnit;

@RunWith(JUnit4.class)
public class ImsRegistrationTrackerTest {
    private static final int SIP_STATUS_CODE_200_OK = 200;
    private static final Set<Uri> URI_SET = Set.of(Uri.parse("1111@test.ims.com"));
    private TestAppContext mTestAppContext;
    private ImsRegistrationTracker mRegTracker;
    private MmTelFeature.MmTelCapabilities mMmTelCapabilities;
    private ImsFeatureManager mFeatureManager;
    private IAosRegistrationListener mAosRegListener = null;
    private MockIAosRegistration mAosReg;
    private ContextFixture mContextFixture;
    private TelephonyManagerProxy mTelephonyManagerProxy;

    @Mock SettingsProxy mSettingsProxy;
    @Mock CarrierConfig mMockCarrierConfig;
    @Mock ConfigInterface mMockConfigInterface;
    @Mock Handler mMockHandler;
    @Mock IDcNetWatcher mMockIDcNetWatcher;
    @Mock IBaseContext mMockBaseContext;
    @Mock ImsRegistrationTracker.CapabilityUpdateListener mMockCapabilityListener;
    @Mock IMmTelFeatureCapabilityListener mMockFeatureCapabilityListener;
    @Mock MessageExecutor mExecutor;
    @Mock SharedPreferences mSp;
    @Mock SharedPreferences.Editor mSpEditor;
    @Mock SimInterface mMockSimInterface;
    @Mock UsatInterface mMockUsatInterface;

    @Before
    public void setUp() {
        MockitoAnnotations.initMocks(this);
        mContextFixture = new ContextFixture();
        mTestAppContext = new TestAppContext(mContextFixture.getTestDouble());
        mTestAppContext.setUp();

        AgentFactory.getInstance().setAgent(SimInterface.class, mMockSimInterface, SLOT0);
        when(mMockSimInterface.getUsatInterface()).thenReturn(mMockUsatInterface);

        when(mTestAppContext.getContentProviderProxy().getGlobalSettings())
                .thenReturn(mSettingsProxy);
        mTelephonyManagerProxy = mTestAppContext.getSystemServiceProxy(TelephonyManagerProxy.class);
        when(mTelephonyManagerProxy.isDataEnabled()).thenReturn(true);
        when(mMockBaseContext.getSlotId()).thenReturn(SLOT0);
        when(mMockBaseContext.getPhoneId()).thenReturn(SLOT0);
        when(mMockBaseContext.getContext()).thenReturn(mTestAppContext.getContext());
        when(mMockBaseContext.getDefaultLooper())
                .thenReturn(AppContext.getInstance().getMainLooper());
        when(mMockBaseContext.getDefaultHandler()).thenReturn(mMockHandler);
        when(mMockConfigInterface.getCarrierConfig()).thenReturn(mMockCarrierConfig);
        when(mTestAppContext.getContext().getSharedPreferences(anyString(), anyInt()))
                .thenReturn(mSp);
        when(mSp.edit()).thenReturn(mSpEditor);
        when(mMockCarrierConfig.getBoolean(eq(CarrierConfig.ImsWfc
                .KEY_SUPPORT_VOWIFI_CAPABILITY_WHEN_WIFI_ONLY_OR_PREFERRED_IN_ROAMING_BOOL)))
                .thenReturn(true);
        when(mMockCarrierConfig.getBoolean(eq(CarrierConfig.ImsSms
                .KEY_SUPPORT_SMS_CAPABILITY_IN_WIFI_ROAMING_BOOL)))
                .thenReturn(true);
        when(mMockCarrierConfig.getBoolean(eq(CarrierConfigManager
                .KEY_IGNORE_DATA_ENABLED_CHANGED_FOR_VIDEO_CALLS)))
                .thenReturn(false);

        when(mMockIDcNetWatcher.isVoiceRoaming()).thenReturn(false);
        when(mMockIDcNetWatcher.isRoaming()).thenReturn(false);

        mFeatureManager = new ImsFeatureManager(mMockBaseContext, mMockFeatureCapabilityListener);
        mAosReg = new MockIAosRegistration();
        mMmTelCapabilities = new MmTelFeature.MmTelCapabilities(0);

        mRegTracker = new FakeImsRegistrationTracker(mMockBaseContext, new ImsRegistrationImpl());
        mRegTracker.setCapabilityUpdateListener(mMockCapabilityListener);
        mFeatureManager.setRegistrationTracker(mRegTracker);
        mAosRegListener = mAosReg.getListener();
    }

    @After
    public void tearDown() {
        mRegTracker.dispose();
        mSettingsProxy = null;
        mContextFixture = null;
        AgentFactory.getInstance().setAgent(SimInterface.class, null, SLOT0);
        mTestAppContext.tearDown();
        mTestAppContext = null;
    }

    @Test
    public void testnotifyRegistering() {
        int features = (IAosRegistrationListener.FeatureTagMask.MMTEL
                | IAosRegistrationListener.FeatureTagMask.VIDEO);

        Set<String> tags = new ArraySet<>();
        tags.add("+g.3gpp.smsip");
        tags.add("video");

        mAosRegListener.notifyRegistering(IAosRegistrationListener.RegistrationType.NORMAL,
                IAosRegistrationListener.NetworkType.LTE, features, tags);

        assertEquals(IAosRegistrationListener.NetworkType.LTE,
                mRegTracker.getRegisteredNetworkType());
        assertEquals(false, mRegTracker.isRegistered());
        assertEquals(IAosRegistrationListener.FeatureTagMask.NONE,
                mRegTracker.getRegisteredFeatures());

        mAosRegListener.notifyRegistering(IAosRegistrationListener.RegistrationType.NORMAL,
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

        mAosRegListener.notifyRegistered(IAosRegistrationListener.RegistrationType.NORMAL,
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

        mAosRegListener.notifyRegistered(IAosRegistrationListener.RegistrationType.NORMAL,
                IAosRegistrationListener.NetworkType.IWLAN, features, tags);

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
        mAosRegListener.notifyDeregistered(IAosRegistrationListener.RegistrationType.NORMAL,
                IAosRegistrationListener.NetworkType.LTE,
                IAosRegistrationListener.ReasonCode.REGISTRATION_ERROR, null,
                DataFailCause.NONE);
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
        mAosRegListener.notifyDeregistered(IAosRegistrationListener.RegistrationType.NORMAL,
                IAosRegistrationListener.NetworkType.LTE,
                IAosRegistrationListener.ReasonCode
                        .WFC_SUB_RESP_403, null, DataFailCause.NONE);
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
        mAosRegListener.notifyDeregistered(IAosRegistrationListener.RegistrationType.NORMAL,
                IAosRegistrationListener.NetworkType.LTE,
                IAosRegistrationListener.ReasonCode.PLMN_BLOCK, null,
                DataFailCause.NONE);
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
    public void testnotifyDeRegistered_WithTimeoutError() {
        mAosRegListener.notifyDeregistered(IAosRegistrationListener.RegistrationType.NORMAL,
                IAosRegistrationListener.NetworkType.LTE,
                IAosRegistrationListener.ReasonCode.REG_RESP_NETWORK_TIMEOUT, null,
                DataFailCause.NONE);
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
    public void testNotifyDeRegistering_NotifyCapabilityChanges() {
        // update registered features
        int features = (IAosRegistrationListener.FeatureTagMask.MMTEL
                | IAosRegistrationListener.FeatureTagMask.VIDEO
                | IAosRegistrationListener.FeatureTagMask.SMSIP);
        mAosRegListener.notifyRegistered(IAosRegistrationListener.RegistrationType.NORMAL,
                IAosRegistrationListener.NetworkType.LTE, features, new ArraySet<String>());

        // notify de-registering
        mAosRegListener.notifyDeregistering(IAosRegistrationListener.RegistrationType.NORMAL);

        assertEquals(IAosRegistrationListener.FeatureTagMask.NONE,
                mRegTracker.getRegisteredFeatures());
        MmTelFeature.MmTelCapabilities expectedCapabilities = new MmTelFeature.MmTelCapabilities(
                MmTelFeature.MmTelCapabilities.CAPABILITY_TYPE_NONE);
        verify(mMockFeatureCapabilityListener, times(2))
                .onFeatureCapabilityChanged(eq(expectedCapabilities));
    }

    @Test
    public void testNotifyDeRegistering_ShouldNotNotifyCapabilityChangesIfNotRegistered() {
        // notify de-registering
        mAosRegListener.notifyDeregistering(IAosRegistrationListener.RegistrationType.NORMAL);

        assertEquals(IAosRegistrationListener.FeatureTagMask.NONE,
                mRegTracker.getRegisteredFeatures());
        verify(mMockFeatureCapabilityListener, never())
                .onFeatureCapabilityChanged(any(MmTelFeature.MmTelCapabilities.class));
    }

    @Test
    public void testnotifyTechnologyChangeFailed() {
        assertNotNull(mRegTracker.getRegistration());
        mAosRegListener.notifyTechnologyChangeFailed(
                IAosRegistrationListener.RegistrationType.NORMAL,
                IAosRegistrationListener.NetworkType.NONE,
                ReasonCode.REGISTRATION_ERROR, null);
    }

    @Test
    public void testnotifyAssociatedUriChanged() {
        Uri[] uris = new Uri[2];
        uris[0] = Uri.parse("1111@test.ims.com");
        uris[1] = Uri.parse("2222@test.ims.com");

        assertNotNull(mRegTracker.getRegistration());
        mAosRegListener.notifyAssociatedUriChanged(uris);
    }

    @Test
    public void testnotifyCapabilitiesUpdateFailed_CrossSim() {
        mAosRegListener.notifyCapabilitiesUpdateFailed(IAosRegistrationListener.Capability
                .CALL_COMPOSER, IAosRegistrationListener.NetworkType.CROSS_SIM,
                IAosRegistrationListener.CapabilityReason.ERROR_GENERIC);

        verify(mMockCapabilityListener).onCapabilitiesUpdateFailed(
                MmTelFeature.MmTelCapabilities.CAPABILITY_TYPE_CALL_COMPOSER,
                ImsRegistrationImpl.REGISTRATION_TECH_CROSS_SIM,
                ImsFeature.CAPABILITY_ERROR_GENERIC);
    }

    @Test
    public void testnotifyCapabilitiesUpdateFailed_Nr() {
        mAosRegListener.notifyCapabilitiesUpdateFailed(IAosRegistrationListener.Capability.SMS,
                IAosRegistrationListener.NetworkType.NR,
                IAosRegistrationListener.CapabilityReason.ERROR_GENERIC);

        verify(mMockCapabilityListener).onCapabilitiesUpdateFailed(
                MmTelFeature.MmTelCapabilities.CAPABILITY_TYPE_SMS,
                ImsRegistrationImpl.REGISTRATION_TECH_NR, ImsFeature.CAPABILITY_ERROR_GENERIC);
    }

    @Test
    public void testnotifyCapabilitiesUpdateFailed_Lte() {
        mAosRegListener.notifyCapabilitiesUpdateFailed(IAosRegistrationListener.Capability.UT,
                IAosRegistrationListener.NetworkType.LTE,
                IAosRegistrationListener.CapabilityReason.ERROR_GENERIC);

        verify(mMockCapabilityListener).onCapabilitiesUpdateFailed(
                MmTelFeature.MmTelCapabilities.CAPABILITY_TYPE_UT,
                ImsRegistrationImpl.REGISTRATION_TECH_LTE, ImsFeature.CAPABILITY_ERROR_GENERIC);
    }

    @Test
    public void testnotifyCapabilitiesUpdateFailed_Voice() {
        mAosRegListener.notifyCapabilitiesUpdateFailed(IAosRegistrationListener.Capability.VOICE,
                IAosRegistrationListener.NetworkType.LTE,
                IAosRegistrationListener.CapabilityReason.ERROR_GENERIC);

        verify(mMockCapabilityListener).onCapabilitiesUpdateFailed(
                MmTelFeature.MmTelCapabilities.CAPABILITY_TYPE_VOICE,
                ImsRegistrationImpl.REGISTRATION_TECH_LTE, ImsFeature.CAPABILITY_ERROR_GENERIC);
    }

    @Test
    public void testnotifyCapabilitiesUpdateFailed_Video() {
        mAosRegListener.notifyCapabilitiesUpdateFailed(IAosRegistrationListener.Capability.VIDEO,
                IAosRegistrationListener.NetworkType.LTE,
                IAosRegistrationListener.CapabilityReason.ERROR_GENERIC);

        verify(mMockCapabilityListener).onCapabilitiesUpdateFailed(
                MmTelFeature.MmTelCapabilities.CAPABILITY_TYPE_VIDEO,
                ImsRegistrationImpl.REGISTRATION_TECH_LTE, ImsFeature.CAPABILITY_ERROR_GENERIC);
    }

    @Test
    public void testnotifyCapabilitiesUpdateFailed_None() {
        mAosRegListener.notifyCapabilitiesUpdateFailed(IAosRegistrationListener.Capability.NONE,
                IAosRegistrationListener.NetworkType.CROSS_SIM,
                IAosRegistrationListener.CapabilityReason.ERROR_GENERIC);

        verify(mMockCapabilityListener).onCapabilitiesUpdateFailed(
                MmTelFeature.MmTelCapabilities.CAPABILITY_TYPE_NONE,
                ImsRegistrationImpl.REGISTRATION_TECH_CROSS_SIM,
                ImsFeature.CAPABILITY_ERROR_GENERIC);
    }

    @Test
    public void testNotifyImsFeatureChanged_ShouldNotifyRegisteredCapabilityChangesOnly() {
        // update registered features
        int features = (IAosRegistrationListener.FeatureTagMask.MMTEL
                | IAosRegistrationListener.FeatureTagMask.SMSIP);
        mAosRegListener.notifyRegistered(IAosRegistrationListener.RegistrationType.NORMAL,
                IAosRegistrationListener.NetworkType.LTE, features, new ArraySet<String>());

        // notify IMS feature changes
        int updatedFeatures = (IAosRegistrationListener.FeatureTagMask.MMTEL
                | IAosRegistrationListener.FeatureTagMask.VIDEO);
        mAosRegListener.notifyImsFeatureChanged(IAosRegistrationListener.RegistrationType.NORMAL,
                IAosRegistrationListener.NetworkType.LTE, updatedFeatures);

        MmTelFeature.MmTelCapabilities expectedCapabilities = new MmTelFeature.MmTelCapabilities(
                MmTelFeature.MmTelCapabilities.CAPABILITY_TYPE_VOICE);
        verify(mMockFeatureCapabilityListener, times(2))
                .onFeatureCapabilityChanged(eq(expectedCapabilities));
    }

    @Test
    public void testNotifyImsFeatureChanged_NotifyDisabledCapability() {
        // update registered features
        int features = (IAosRegistrationListener.FeatureTagMask.MMTEL
                | IAosRegistrationListener.FeatureTagMask.VIDEO
                | IAosRegistrationListener.FeatureTagMask.SMSIP);
        mAosRegListener.notifyRegistered(IAosRegistrationListener.RegistrationType.NORMAL,
                IAosRegistrationListener.NetworkType.LTE, features, new ArraySet<String>());

        // notify IMS feature changes
        int updatedFeatures = IAosRegistrationListener.FeatureTagMask.NONE;
        mAosRegListener.notifyImsFeatureChanged(IAosRegistrationListener.RegistrationType.NORMAL,
                IAosRegistrationListener.NetworkType.LTE, updatedFeatures);

        MmTelFeature.MmTelCapabilities expectedCapabilities = new MmTelFeature.MmTelCapabilities(
                MmTelFeature.MmTelCapabilities.CAPABILITY_TYPE_NONE);
        verify(mMockFeatureCapabilityListener, times(2))
                .onFeatureCapabilityChanged(eq(expectedCapabilities));
    }

    @Test
    public void testNotifyImsFeatureChanged_NotifyEnabledCapability() {
        // update registered features
        int features = (IAosRegistrationListener.FeatureTagMask.MMTEL
                | IAosRegistrationListener.FeatureTagMask.VIDEO
                | IAosRegistrationListener.FeatureTagMask.SMSIP
                | IAosRegistrationListener.FeatureTagMask.USSI);
        mAosRegListener.notifyRegistered(IAosRegistrationListener.RegistrationType.NORMAL,
                IAosRegistrationListener.NetworkType.LTE, features, new ArraySet<String>());

        // notify IMS feature changes
        int updatedFeatures = (IAosRegistrationListener.FeatureTagMask.MMTEL
                | IAosRegistrationListener.FeatureTagMask.VIDEO
                | IAosRegistrationListener.FeatureTagMask.SMSIP);
        mAosRegListener.notifyImsFeatureChanged(IAosRegistrationListener.RegistrationType.NORMAL,
                IAosRegistrationListener.NetworkType.LTE, updatedFeatures);

        int capabilities = MmTelFeature.MmTelCapabilities.CAPABILITY_TYPE_VOICE
                | MmTelFeature.MmTelCapabilities.CAPABILITY_TYPE_VIDEO
                | MmTelFeature.MmTelCapabilities.CAPABILITY_TYPE_SMS;
        MmTelFeature.MmTelCapabilities expectedCapabilities =
                new MmTelFeature.MmTelCapabilities(capabilities);
        verify(mMockFeatureCapabilityListener, times(2))
                .onFeatureCapabilityChanged(eq(expectedCapabilities));
    }

    @Test
    public void testNotifyImsFeatureChanged_ShouldNotNotifyIfNotNormalRegType() {
        // update registered features
        int features = (IAosRegistrationListener.FeatureTagMask.MMTEL
                | IAosRegistrationListener.FeatureTagMask.VIDEO
                | IAosRegistrationListener.FeatureTagMask.SMSIP);
        mAosRegListener.notifyRegistered(IAosRegistrationListener.RegistrationType.NORMAL,
                IAosRegistrationListener.NetworkType.LTE, features, new ArraySet<String>());

        // notify IMS feature changes
        int updatedFeatures = IAosRegistrationListener.FeatureTagMask.MMTEL;
        mAosRegListener.notifyImsFeatureChanged(IAosRegistrationListener.RegistrationType.EMERGENCY,
                IAosRegistrationListener.NetworkType.LTE, updatedFeatures);

        MmTelFeature.MmTelCapabilities expectedCapabilities = new MmTelFeature.MmTelCapabilities(
                MmTelFeature.MmTelCapabilities.CAPABILITY_TYPE_VOICE);
        verify(mMockFeatureCapabilityListener, never())
                .onFeatureCapabilityChanged(eq(expectedCapabilities));
    }

    @Test
    public void testNotifyImsFeatureChanged_ShouldNotNotifyIfNotRegisteredNetworkType() {
        // update registered features
        int features = (IAosRegistrationListener.FeatureTagMask.MMTEL
                | IAosRegistrationListener.FeatureTagMask.VIDEO
                | IAosRegistrationListener.FeatureTagMask.SMSIP);
        mAosRegListener.notifyRegistered(IAosRegistrationListener.RegistrationType.NORMAL,
                IAosRegistrationListener.NetworkType.LTE, features, new ArraySet<String>());

        // notify IMS feature changes
        int updatedFeatures = IAosRegistrationListener.FeatureTagMask.MMTEL;
        mAosRegListener.notifyImsFeatureChanged(IAosRegistrationListener.RegistrationType.NORMAL,
                IAosRegistrationListener.NetworkType.NR, updatedFeatures);

        MmTelFeature.MmTelCapabilities expectedCapabilities = new MmTelFeature.MmTelCapabilities(
                MmTelFeature.MmTelCapabilities.CAPABILITY_TYPE_VOICE);
        verify(mMockFeatureCapabilityListener, never())
                .onFeatureCapabilityChanged(eq(expectedCapabilities));
    }

    @Test
    public void testNotifyImsFeatureChanged_ShouldNotNotifyIfNotChanged() {
        // update registered features
        int features = (IAosRegistrationListener.FeatureTagMask.MMTEL
                | IAosRegistrationListener.FeatureTagMask.VIDEO
                | IAosRegistrationListener.FeatureTagMask.SMSIP);
        mAosRegListener.notifyRegistered(IAosRegistrationListener.RegistrationType.NORMAL,
                IAosRegistrationListener.NetworkType.LTE, features, new ArraySet<String>());

        // notify IMS feature changes
        mAosRegListener.notifyImsFeatureChanged(IAosRegistrationListener.RegistrationType.NORMAL,
                IAosRegistrationListener.NetworkType.LTE, features);

        int capabilities = MmTelFeature.MmTelCapabilities.CAPABILITY_TYPE_VOICE
                | MmTelFeature.MmTelCapabilities.CAPABILITY_TYPE_VIDEO
                | MmTelFeature.MmTelCapabilities.CAPABILITY_TYPE_SMS;
        MmTelFeature.MmTelCapabilities expectedCapabilities =
                new MmTelFeature.MmTelCapabilities(capabilities);
        // onFeatureCapabilityChanged is invoked only once when it registered first time.
        verify(mMockFeatureCapabilityListener).onFeatureCapabilityChanged(eq(expectedCapabilities));
    }

    @Test
    public void testNotifyImsFeatureChanged_ShouldNotNotifyIfNotRegisteredFeatureChange() {
        // update registered features
        int features = IAosRegistrationListener.FeatureTagMask.USSI;
        mAosRegListener.notifyRegistered(IAosRegistrationListener.RegistrationType.NORMAL,
                IAosRegistrationListener.NetworkType.LTE, features, new ArraySet<String>());

        // notify IMS feature changes
        int updatedFeatures = IAosRegistrationListener.FeatureTagMask.MMTEL
                | IAosRegistrationListener.FeatureTagMask.USSI;
        mAosRegListener.notifyImsFeatureChanged(IAosRegistrationListener.RegistrationType.NORMAL,
                IAosRegistrationListener.NetworkType.LTE, updatedFeatures);

        MmTelFeature.MmTelCapabilities expectedCapabilities = new MmTelFeature.MmTelCapabilities(
                MmTelFeature.MmTelCapabilities.CAPABILITY_TYPE_VOICE);
        verify(mMockFeatureCapabilityListener, never())
                .onFeatureCapabilityChanged(eq(expectedCapabilities));
    }

    @Test
    public void testNotifyRegEventStateChanged_simInterfaceIsNull() {
        AgentFactory.getInstance().setAgent(SimInterface.class, null, SLOT0);

        mAosRegListener.notifyRegEventStateChanged(SIP_STATUS_CODE_200_OK, URI_SET);

        verifyNoInteractions(mMockUsatInterface);
    }

    @Test
    public void testNotifyRegEventStateChanged_configInterfaceIsNull() {
        when(mMockConfigInterface.getCarrierConfig()).thenReturn(null);

        mAosRegListener.notifyRegEventStateChanged(SIP_STATUS_CODE_200_OK, URI_SET);

        verifyNoInteractions(mMockUsatInterface);
    }

    @Test
    public void testNotifyRegEventStateChanged_configIsInvalidValue() {
        when(mMockCarrierConfig.getInt(CarrierConfig.Ims.KEY_USAT_REG_EVENT_DOWNLOAD_POLICY_INT))
                .thenReturn(-1);

        mAosRegListener.notifyRegEventStateChanged(SIP_STATUS_CODE_200_OK, URI_SET);

        verifyNoInteractions(mMockUsatInterface);
    }

    @Test
    public void testNotifyRegEventStateChanged_configIsNotDownload() {
        when(mMockCarrierConfig.getInt(CarrierConfig.Ims.KEY_USAT_REG_EVENT_DOWNLOAD_POLICY_INT))
                .thenReturn(CarrierConfig.Ims.USAT_REG_EVENT_NOT_DOWNLOAD);

        mAosRegListener.notifyRegEventStateChanged(SIP_STATUS_CODE_200_OK, URI_SET);

        verifyNoInteractions(mMockUsatInterface);
    }

    @Test
    public void testNotifyRegEventStateChanged_configIsConditional() {
        when(mMockCarrierConfig.getInt(CarrierConfig.Ims.KEY_USAT_REG_EVENT_DOWNLOAD_POLICY_INT))
                .thenReturn(CarrierConfig.Ims.USAT_REG_EVENT_CONDITIONAL_DOWNLOAD);
        when(mMockUsatInterface.isInSetupEventList(anyInt())).thenReturn(true);
        Set<String> iariSet = Set.of("urn::3gpp-application.ims.iari.ttc-iss");
        when(mMockSimInterface.getUiccIari()).thenReturn(iariSet);

        mAosRegListener.notifyRegEventStateChanged(SIP_STATUS_CODE_200_OK, URI_SET);

        verify(mMockUsatInterface).createRegEventDownloadCommand(
                eq(SIP_STATUS_CODE_200_OK), eq(URI_SET), any(Usat.Listener.class));
    }

    @Test
    public void testNotifyRegEventStateChanged_configIsUnconditional() {
        when(mMockCarrierConfig.getInt(CarrierConfig.Ims.KEY_USAT_REG_EVENT_DOWNLOAD_POLICY_INT))
                .thenReturn(CarrierConfig.Ims.USAT_REG_EVENT_UNCONDITIONAL_DOWNLOAD);
        when(mMockUsatInterface.isInSetupEventList(anyInt())).thenReturn(false);
        when(mMockSimInterface.getUiccIari()).thenReturn(Collections.emptySet());

        mAosRegListener.notifyRegEventStateChanged(SIP_STATUS_CODE_200_OK, URI_SET);

        verify(mMockUsatInterface).createRegEventDownloadCommand(
                eq(SIP_STATUS_CODE_200_OK), eq(URI_SET), any(Usat.Listener.class));
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
        ImsServiceManager serviceManager =
                new ImsServiceManager(mTestAppContext.getContext(), mExecutor);
        ImsServiceManager.setDefault(serviceManager);

        try {
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

            CapabilityPairs defaultCapPairs = new CapabilityPairs();
            mRegTracker.changeCapabilities(enableCapabilities, new ArrayList<>());
            assertEquals(defaultCapPairs, mRegTracker.createCapabilityPairsFromCapabilities());

            when(mSp.getInt(eq("VOICE_OVER_WIFI_ROAMING_ENABLED_OVERRIDE"), anyInt()))
                    .thenReturn(ProvisioningManager.PROVISIONING_VALUE_DISABLED);
            when(mSp.getInt(eq("VOICE_OVER_WIFI_MODE_OVERRIDE"), anyInt()))
                    .thenReturn(ImsMmTelManager.WIFI_MODE_CELLULAR_PREFERRED);

            mRegTracker.changeCapabilities(enableCapabilities, new ArrayList<>());
            assertEquals(defaultCapPairs, mRegTracker.createCapabilityPairsFromCapabilities());

            when(mSp.getInt(eq("VOICE_OVER_WIFI_ROAMING_ENABLED_OVERRIDE"), anyInt()))
                    .thenReturn(ProvisioningManager.PROVISIONING_VALUE_ENABLED);
            when(mSp.getInt(eq("VOICE_OVER_WIFI_MODE_OVERRIDE"), anyInt()))
                    .thenReturn(ImsMmTelManager.WIFI_MODE_CELLULAR_PREFERRED);

            mRegTracker.changeCapabilities(enableCapabilities, new ArrayList<>());
            assertEquals(defaultCapPairs, mRegTracker.createCapabilityPairsFromCapabilities());

            when(mSp.getInt(eq("VOICE_OVER_WIFI_ROAMING_ENABLED_OVERRIDE"), anyInt()))
                    .thenReturn(ProvisioningManager.PROVISIONING_VALUE_ENABLED);
            when(mSp.getInt(eq("VOICE_OVER_WIFI_MODE_OVERRIDE"), anyInt()))
                    .thenReturn(ImsMmTelManager.WIFI_MODE_WIFI_PREFERRED);

            mRegTracker.changeCapabilities(enableCapabilities, new ArrayList<>());
            assertEquals(capabilityPairs, mRegTracker.createCapabilityPairsFromCapabilities());

            when(mMockIDcNetWatcher.isVoiceRoaming()).thenReturn(false);
            mRegTracker.changeCapabilities(enableCapabilities, new ArrayList<>());
            assertEquals(capabilityPairs, mRegTracker.createCapabilityPairsFromCapabilities());
        } finally {
            serviceManager.dispose();
            ImsServiceManager.setDefault(oldServiceManager);
        }
    }

    @Test
    public void testchangeCapabilities_VideoBasedOnMobileData() {
        when(mMockIDcNetWatcher.isRoaming()).thenReturn(true);
        when(mTelephonyManagerProxy.isDataRoamingEnabled()).thenReturn(true);
        when(mTelephonyManagerProxy.isDataEnabled()).thenReturn(true);

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
        when(mTelephonyManagerProxy.isDataEnabled()).thenReturn(true);

        mRegTracker.changeCapabilities(enableCapabilities, new ArrayList<>());
        assertEquals(capabilityPairs, mRegTracker.createCapabilityPairsFromCapabilities());

        when(mTelephonyManagerProxy.isDataEnabled()).thenReturn(false);

        mRegTracker.changeCapabilities(enableCapabilities, new ArrayList<>());
        assertEquals(new CapabilityPairs(), mRegTracker.createCapabilityPairsFromCapabilities());
    }

    @Test
    public void testchangeCapabilities_WithoutSmsCapability() {
        when(mMockCarrierConfig.getBoolean(eq(CarrierConfigManager.ImsSms
                .KEY_SMS_OVER_IMS_SUPPORTED_BOOL)))
                .thenReturn(false);

        assertEquals(null, mRegTracker.createCapabilityPairsFromCapabilities());

        when(mMockCarrierConfig.getBoolean(eq(CarrierConfigManager.ImsSms
                .KEY_SMS_OVER_IMS_SUPPORTED_BOOL)))
                .thenReturn(true);

        // SMS over IMS is enabled, but no RAT is supported
        assertEquals(null, mRegTracker.createCapabilityPairsFromCapabilities());

        int[] emptyArray = new int[0];
        when(mMockCarrierConfig.getIntArray(
                CarrierConfigManager.ImsSms.KEY_SMS_OVER_IMS_SUPPORTED_RATS_INT_ARRAY))
                .thenReturn(emptyArray);

        // SMS over IMS is enabled, but supported RAT array is empty
        assertEquals(null, mRegTracker.createCapabilityPairsFromCapabilities());
    }

    @Test
    public void testchangeCapabilities_WithSmsCapability() {
        when(mMockCarrierConfig.getBoolean(eq(CarrierConfigManager.ImsSms
                .KEY_SMS_OVER_IMS_SUPPORTED_BOOL)))
                .thenReturn(true);
        int[] intArray = {AccessNetworkType.EUTRAN, AccessNetworkType.IWLAN,
                AccessNetworkType.UTRAN, AccessNetworkType.NGRAN};

        when(mMockCarrierConfig.getIntArray(
                CarrierConfigManager.ImsSms.KEY_SMS_OVER_IMS_SUPPORTED_RATS_INT_ARRAY))
                .thenReturn(intArray);

        CapabilityPairs capabilityPairs = new CapabilityPairs(
                IAosRegistrationListener.NetworkType.LTE,
                IAosRegistrationListener.Capability.SMS);

        capabilityPairs.addCapability(IAosRegistrationListener.NetworkType.IWLAN,
                IAosRegistrationListener.Capability.SMS);

        capabilityPairs.addCapability(IAosRegistrationListener.NetworkType.UTRAN,
                IAosRegistrationListener.Capability.SMS);

        capabilityPairs.addCapability(IAosRegistrationListener.NetworkType.NR,
                IAosRegistrationListener.Capability.SMS);

        assertEquals(capabilityPairs, mRegTracker.createCapabilityPairsFromCapabilities());
    }

    @Test
    public void testchangeCapabilities_SmsCapabilityInRoaming() {
        when(mMockCarrierConfig.getBoolean(eq(CarrierConfigManager.ImsSms
                .KEY_SMS_OVER_IMS_SUPPORTED_BOOL)))
                .thenReturn(true);
        when(mMockCarrierConfig.getBoolean(eq(CarrierConfig.ImsSms
                .KEY_SUPPORT_SMS_CAPABILITY_IN_WIFI_ROAMING_BOOL)))
                .thenReturn(false);

        int[] intArray = {AccessNetworkType.EUTRAN, AccessNetworkType.IWLAN};

        when(mMockCarrierConfig.getIntArray(
                CarrierConfigManager.ImsSms.KEY_SMS_OVER_IMS_SUPPORTED_RATS_INT_ARRAY))
                .thenReturn(intArray);

        when(mMockIDcNetWatcher.isRoaming()).thenReturn(true);

        CapabilityPairs capabilityPairs = new CapabilityPairs(
                IAosRegistrationListener.NetworkType.LTE,
                IAosRegistrationListener.Capability.SMS);

        assertEquals(capabilityPairs, mRegTracker.createCapabilityPairsFromCapabilities());

        when(mMockIDcNetWatcher.isRoaming()).thenReturn(false);

        capabilityPairs.addCapability(IAosRegistrationListener.NetworkType.IWLAN,
                IAosRegistrationListener.Capability.SMS);

        assertEquals(capabilityPairs, mRegTracker.createCapabilityPairsFromCapabilities());
    }

    @Test
    public void testchangeCapabilities_OnIgnoreDataEnbledForVideo() {
        when(mMockCarrierConfig.getBoolean(eq(CarrierConfigManager
                .KEY_IGNORE_DATA_ENABLED_CHANGED_FOR_VIDEO_CALLS)))
                .thenReturn(true);

        when(mMockIDcNetWatcher.isRoaming()).thenReturn(true);
        when(mTelephonyManagerProxy.isDataRoamingEnabled()).thenReturn(true);
        when(mTelephonyManagerProxy.isDataEnabled()).thenReturn(true);

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
        when(mTelephonyManagerProxy.isDataEnabled()).thenReturn(true);

        mRegTracker.changeCapabilities(enableCapabilities, new ArrayList<>());
        assertEquals(capabilityPairs, mRegTracker.createCapabilityPairsFromCapabilities());

        when(mTelephonyManagerProxy.isDataEnabled()).thenReturn(false);

        mRegTracker.changeCapabilities(enableCapabilities, new ArrayList<>());
        assertEquals(capabilityPairs, mRegTracker.createCapabilityPairsFromCapabilities());
    }

    @Test
    public void testcreateCapabilities_OnRoaming() {
        ImsServiceManager oldServiceManager = ImsServiceManager.getDefault();
        ImsServiceManager serviceManager =
                new ImsServiceManager(mTestAppContext.getContext(), mExecutor);
        ImsServiceManager.setDefault(serviceManager);

        try {
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

            assertEquals(capabilityPairs, mRegTracker.createCapabilityPairsFromCapabilities());
        } finally {
            serviceManager.dispose();
            ImsServiceManager.setDefault(oldServiceManager);
        }
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

        mAosRegListener.notifyRegistered(IAosRegistrationListener.RegistrationType.NORMAL,
                IAosRegistrationListener.NetworkType.IWLAN, features, tags);
        assertEquals(IAosRegistrationListener.NetworkType.IWLAN,
                mRegTracker.getRegisteredNetworkType());
        mRegTracker.refreshCallRegistrationState();
        assertEquals(IAosRegistrationListener.NetworkType.LTE,
                mRegTracker.getRegisteredNetworkType());

        verify(mMockIDcNetWatcher, times(2)).addListener(any(IDcNetWatcher.Listener.class));
        verify(mMockIDcNetWatcher, times(1)).removeListener(any(IDcNetWatcher.Listener.class));
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
    public void testOnImsServiceStarted() {
        ImsStackRegistry.ImsServiceListener listener = mRegTracker.getImsServiceListener();
        assertNotNull(listener);

        listener.onImsServiceStarted(SLOT0);
        listener.onImsServiceStopped(SLOT0);
        listener.onImsServiceStarted(SLOT0);
        verify(mMockIDcNetWatcher, times(2)).addListener(any(IDcNetWatcher.Listener.class));
        verify(mMockIDcNetWatcher, times(1)).removeListener(any(IDcNetWatcher.Listener.class));
    }

    @Test
    public void testOnImsServiceStopped() {
        ImsStackRegistry.ImsServiceListener listener = mRegTracker.getImsServiceListener();
        assertNotNull(listener);

        listener.onImsServiceStopped(SLOT0);
        listener.onImsServiceStarted(SLOT0);
        listener.onImsServiceStopped(SLOT0);
        verify(mMockIDcNetWatcher, times(2)).addListener(any(IDcNetWatcher.Listener.class));
        verify(mMockIDcNetWatcher, times(2)).removeListener(any(IDcNetWatcher.Listener.class));
    }

    @Test
    public void testOnImsServiceStopped_onRegistered() {
        ImsStackRegistry.ImsServiceListener listener = mRegTracker.getImsServiceListener();
        assertNotNull(listener);

        mRegTracker.updateFeatures(IAosRegistrationListener.FeatureTagMask.MMTEL);
        mRegTracker.updateNetworkType(IAosRegistrationListener.NetworkType.LTE);
        assertTrue(mRegTracker.isRegistered());
        assertEquals(IAosRegistrationListener.NetworkType.LTE,
                mRegTracker.getRegisteredNetworkType());

        listener.onImsServiceStopped(SLOT0);

        assertFalse(mRegTracker.isRegistered());
        assertEquals(IAosRegistrationListener.NetworkType.NONE,
                mRegTracker.getRegisteredNetworkType());
    }

    @Test
    public void testOnRoamingStateChanged() {
        ArgumentCaptor<IDcNetWatcher.Listener> listenerCaptor =
                ArgumentCaptor.forClass(IDcNetWatcher.Listener.class);
        verify(mMockIDcNetWatcher).addListener(listenerCaptor.capture());
        IDcNetWatcher.Listener listener = listenerCaptor.getValue();
        assertNotNull(listener);
        listener.onRoamingStateChanged(true);

        verify(mMockHandler).post(any(Runnable.class));
    }

    @Test
    public void testConfigListener() {
        ImsServiceManager oldServiceManager = ImsServiceManager.getDefault();
        ImsServiceManager serviceManager =
                new ImsServiceManager(mTestAppContext.getContext(), mExecutor);
        ImsServiceManager.setDefault(serviceManager);

        try {
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

            ImsServiceRecord isr = ImsServiceManager.getServiceRecord(SLOT0);
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

            assertEquals(new CapabilityPairs(),
                    mRegTracker.createCapabilityPairsFromCapabilities());
        } finally {
            serviceManager.dispose();
            ImsServiceManager.setDefault(oldServiceManager);
        }
    }

    @Test
    public void testRttCapability() {
        ImsServiceManager oldServiceManager = ImsServiceManager.getDefault();
        ImsServiceManager serviceManager =
                new ImsServiceManager(mTestAppContext.getContext(), mExecutor);
        ImsServiceManager.setDefault(serviceManager);

        try {
            ImsServiceRecord isr = ImsServiceManager.getServiceRecord(SLOT0);
            ImsConfigImpl configImpl = isr.getConfig();
            // Rtt Disabled
            configImpl.setConfig(
                    ProvisioningManager.KEY_RTT_ENABLED,
                    ProvisioningManager.PROVISIONING_VALUE_DISABLED);

            when(mSp.getInt(eq("RTT_ENABLED"), anyInt()))
                    .thenReturn(ProvisioningManager.PROVISIONING_VALUE_DISABLED);

            // Enabled Capabilities from framework
            List<CapabilityPair> enableCapabilities = new ArrayList<>();
            enableCapabilities.add(new CapabilityPair(
                    MmTelFeature.MmTelCapabilities.CAPABILITY_TYPE_VOICE,
                    ImsRegistrationImpl.REGISTRATION_TECH_LTE));
            mRegTracker.changeCapabilities(enableCapabilities, null);

            CapabilityPairs capabilityPairs = new CapabilityPairs();
            capabilityPairs.addCapability(IAosRegistrationListener.NetworkType.LTE,
                    IAosRegistrationListener.Capability.VOICE);
            // Capability : LTE -> Voice when rtt disabled
            assertEquals(capabilityPairs, mRegTracker.createCapabilityPairsFromCapabilities());

            // Rtt Enabled
            configImpl.setConfig(
                    ProvisioningManager.KEY_RTT_ENABLED,
                    ProvisioningManager.PROVISIONING_VALUE_ENABLED);

            when(mSp.getInt(eq("RTT_ENABLED"), anyInt()))
                    .thenReturn(ProvisioningManager.PROVISIONING_VALUE_ENABLED);

            capabilityPairs.addCapability(IAosRegistrationListener.NetworkType.LTE,
                    IAosRegistrationListener.Capability.TEXT);
            // Capability : LTE -> Voice & LTE & Text when rtt enabled
            assertEquals(capabilityPairs, mRegTracker.createCapabilityPairsFromCapabilities());

            // Enabled Capabilities from framework
            enableCapabilities = new ArrayList<>();
            enableCapabilities.add(new CapabilityPair(
                    MmTelFeature.MmTelCapabilities.CAPABILITY_TYPE_UT,
                    ImsRegistrationImpl.REGISTRATION_TECH_LTE));

            List<CapabilityPair> disableCapabilities = new ArrayList<>();
            disableCapabilities.add(new CapabilityPair(
                    MmTelFeature.MmTelCapabilities.CAPABILITY_TYPE_VOICE,
                    ImsRegistrationImpl.REGISTRATION_TECH_LTE));

            mRegTracker.changeCapabilities(enableCapabilities, disableCapabilities);

            capabilityPairs = new CapabilityPairs();
            capabilityPairs.addCapability(IAosRegistrationListener.NetworkType.LTE,
                    IAosRegistrationListener.Capability.UT);
            // Capability : LTE -> UT since no voice, no text even if rtt enabled
            assertEquals(capabilityPairs, mRegTracker.createCapabilityPairsFromCapabilities());

        } finally {
            serviceManager.dispose();
            ImsServiceManager.setDefault(oldServiceManager);
        }
    }

    @Test
    public void testRttCapabilityInRoaming() {
        ImsServiceManager oldServiceManager = ImsServiceManager.getDefault();
        ImsServiceManager serviceManager =
                new ImsServiceManager(mTestAppContext.getContext(), mExecutor);
        ImsServiceManager.setDefault(serviceManager);

        try {
            ImsServiceRecord isr = ImsServiceManager.getServiceRecord(SLOT0);
            ImsConfigImpl configImpl = isr.getConfig();

            // Rtt Enabled
            configImpl.setConfig(
                    ProvisioningManager.KEY_RTT_ENABLED,
                    ProvisioningManager.PROVISIONING_VALUE_ENABLED);

            when(mSp.getInt(eq("RTT_ENABLED"), anyInt()))
                    .thenReturn(ProvisioningManager.PROVISIONING_VALUE_ENABLED);

            // In Roaming
            when(mMockIDcNetWatcher.isRoaming()).thenReturn(true);

            // In Roaming rtt not enabled
            when(mMockCarrierConfig.getBoolean(eq(CarrierConfigManager
                .KEY_RTT_SUPPORTED_WHILE_ROAMING_BOOL))).thenReturn(false);

            // Enabled Capabilities from framework
            List<CapabilityPair> enableCapabilities = new ArrayList<>();
            enableCapabilities.add(new CapabilityPair(
                    MmTelFeature.MmTelCapabilities.CAPABILITY_TYPE_VOICE,
                    ImsRegistrationImpl.REGISTRATION_TECH_LTE));
            mRegTracker.changeCapabilities(enableCapabilities, null);

            // Capability : LTE -> Voice no text as rtt enabled but not in roaming
            CapabilityPairs capabilityPairs = new CapabilityPairs();
            capabilityPairs.addCapability(IAosRegistrationListener.NetworkType.LTE,
                    IAosRegistrationListener.Capability.VOICE);
            assertEquals(capabilityPairs, mRegTracker.createCapabilityPairsFromCapabilities());

            // In Roaming rtt enabled
            when(mMockCarrierConfig.getBoolean(eq(CarrierConfigManager
                .KEY_RTT_SUPPORTED_WHILE_ROAMING_BOOL))).thenReturn(true);

            capabilityPairs.addCapability(IAosRegistrationListener.NetworkType.LTE,
                    IAosRegistrationListener.Capability.TEXT);
            // Capability : LTE -> Voice & LTE -> Text
            assertEquals(capabilityPairs, mRegTracker.createCapabilityPairsFromCapabilities());
        } finally {
            serviceManager.dispose();
            ImsServiceManager.setDefault(oldServiceManager);
        }
    }

    @Test
    @SmallTest
    public void dumpVerifiesPresenceOfKey() {
        // GIVEN: A mocked AosService is injected into the factory to intercept nested dump calls
        AosService mockAosService = Mockito.mock(AosService.class);
        AosFactory.getInstance().replaceService(SLOT0, mockAosService);

        // GIVEN: The tracker is registered to set its internal state (mFeatures, networkType)
        int features = IAosRegistrationListener.FeatureTagMask.MMTEL
                | IAosRegistrationListener.FeatureTagMask.VIDEO;
        IAosRegistrationListener.NetworkType network = IAosRegistrationListener.NetworkType.LTE;

        mAosRegListener.notifyRegistered(
                IAosRegistrationListener.RegistrationType.NORMAL,
                network, features, new ArraySet<>());

        // GIVEN: An event occurs to populate the LocalLog

        // Capture the NetWatcherListener registered during init
        ArgumentCaptor<IDcNetWatcher.Listener> listenerCaptor =
                ArgumentCaptor.forClass(IDcNetWatcher.Listener.class);
        verify(mMockIDcNetWatcher).addListener(listenerCaptor.capture());
        IDcNetWatcher.Listener netListener = listenerCaptor.getValue();
        assertNotNull(netListener);

        // Trigger the roaming event
        netListener.onRoamingStateChanged(true);

        // Capture and run the Runnable posted to the mock handler to populate the log
        ArgumentCaptor<Runnable> runnableCaptor = ArgumentCaptor.forClass(Runnable.class);
        verify(mMockHandler).post(runnableCaptor.capture());
        runnableCaptor.getValue().run();

        // GIVEN: Prepare tools to capture the dump output
        final String dumpsysIndentPrefix = "  ";
        StringWriter stringWriter = new StringWriter();
        IndentingPrintWriter pw = new IndentingPrintWriter(stringWriter, dumpsysIndentPrefix);

        // WHEN: The dump() method is called
        mRegTracker.dump(pw);
        pw.flush();
        String output = stringWriter.toString();


        // THEN: The output string loosely contains all key information

        // Check for section header
        assertTrue("Output should contain the 'Registration:' header",
                output.contains("Registration:"));

        // Check for key state values
        assertTrue("Dump should contain the correct registered features",
                output.contains("registered="
                        + IAosRegistrationListener.FeatureTagMask.toString(features)));
        assertTrue("Dump should contain the correct network type",
                output.contains("networkType=" + network));

        // Check for log header
        assertTrue("Output should contain the 'Most recent logs:' header",
                output.contains("Most recent logs:"));

        // Check for log content from onRoamingStateChanged
        String expectedLog = "RoamingStateChanged: roaming=true";
        assertTrue("Dump should contain the roaming log",
                output.contains(expectedLog));

        // THEN: Verify that the nested AosFactory.dump() (which calls AosService.dump()) was made
        ArgumentCaptor<IndentingPrintWriter> captor =
                ArgumentCaptor.forClass(IndentingPrintWriter.class);

        // This verifies that AosFactory.dump() was called, which in turn calls mock service's dump
        verify(mockAosService).dump(captor.capture());

        // Verify the *same* print writer was passed down to the nested call
        assertEquals(pw, captor.getValue());
    }

    private class FakeImsRegistrationTracker extends ImsRegistrationTracker {
        FakeImsRegistrationTracker(IBaseContext context, ImsRegistrationImpl regImpl) {
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
