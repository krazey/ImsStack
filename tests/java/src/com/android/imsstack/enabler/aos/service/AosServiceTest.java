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

package com.android.imsstack.enabler.aos.service;

import static org.junit.Assert.assertEquals;
import static org.junit.Assert.assertThrows;
import static org.junit.Assert.assertTrue;
import static org.mockito.Mockito.any;
import static org.mockito.Mockito.eq;
import static org.mockito.Mockito.never;
import static org.mockito.Mockito.times;
import static org.mockito.Mockito.verify;
import static org.mockito.Mockito.verifyNoMoreInteractions;
import static org.mockito.Mockito.when;

import android.content.Intent;
import android.net.Uri;
import android.os.Looper;
import android.os.Parcel;
import android.telephony.DisconnectCause;
import android.telephony.PreciseCallState;
import android.telephony.PreciseDisconnectCause;
import android.telephony.SubscriptionManager;
import android.telephony.TelephonyManager;
import android.telephony.data.ApnSetting;
import android.testing.AndroidTestingRunner;
import android.testing.TestableLooper;
import android.util.ArraySet;
import android.util.SparseArray;

import com.android.imsstack.ImsStackTest;
import com.android.imsstack.core.agents.AgentFactory;
import com.android.imsstack.core.agents.ConfigInterface;
import com.android.imsstack.core.agents.LocationInterface;
import com.android.imsstack.core.agents.NativeStateInterface;
import com.android.imsstack.core.agents.Sim;
import com.android.imsstack.core.agents.SimInterface;
import com.android.imsstack.core.agents.TelephonyInterface;
import com.android.imsstack.core.agents.WifiInterface;
import com.android.imsstack.core.agents.dcm.DcFactory;
import com.android.imsstack.core.agents.dcmif.EApnType;
import com.android.imsstack.core.agents.dcmif.IApn;
import com.android.imsstack.core.agents.dcmif.IDcNetWatcher;
import com.android.imsstack.core.config.CarrierConfig;
import com.android.imsstack.enabler.aos.IAosInfo.CrossSimStatus;
import com.android.imsstack.enabler.aos.IAosInfo.LocationInfo;
import com.android.imsstack.enabler.aos.IAosInfo.PhoneNumberState;
import com.android.imsstack.enabler.aos.IAosInfo.RoamingPreferredVoiceNetwork;
import com.android.imsstack.enabler.aos.IAosInfo.ServiceSetting;
import com.android.imsstack.enabler.aos.IAosInfoListener;
import com.android.imsstack.enabler.aos.IAosRegistration;
import com.android.imsstack.enabler.aos.IAosRegistration.CapabilityPairs;
import com.android.imsstack.enabler.aos.IAosRegistrationListener;
import com.android.imsstack.enabler.aos.IAosRegistrationListener.Capability;
import com.android.imsstack.enabler.aos.IAosRegistrationListener.CapabilityReason;
import com.android.imsstack.enabler.aos.IAosRegistrationListener.FeatureTagMask;
import com.android.imsstack.enabler.aos.IAosRegistrationListener.NetworkType;
import com.android.imsstack.enabler.aos.IAosRegistrationListener.ReasonCode;
import com.android.imsstack.enabler.aos.IAosRegistrationListener.RegistrationState;
import com.android.imsstack.enabler.aos.IAosRegistrationListener.RegistrationType;
import com.android.imsstack.enabler.aos.IIAosService;
import com.android.imsstack.internal.imsservice.ImsServiceRegistry;
import com.android.imsstack.jni.JniIms;
import com.android.imsstack.jni.JniImsListener;
import com.android.imsstack.jni.JniImsProxy;
import com.android.imsstack.jni.JniObjectId;
import com.android.imsstack.system.ImsEventDef;

import org.junit.After;
import org.junit.Before;
import org.junit.Test;
import org.junit.runner.RunWith;
import org.mockito.ArgumentCaptor;
import org.mockito.Mock;
import org.mockito.MockitoAnnotations;

import java.util.Set;

@RunWith(AndroidTestingRunner.class)
@TestableLooper.RunWithLooper
public class AosServiceTest extends ImsStackTest {
    private static final int SLOT_0 = 0;
    public static final int PCO_NONE_VALUE = 0;
    public static final int PCO_LIMITED_SERVICE_VALUE = 5;

    private FakeAosService mAosService;
    private final long mNativeObject = 1000;

    @Mock JniIms mMockJniIms;
    @Mock SparseArray<ImsServiceRegistry> mMockImsServiceRegistrys;
    @Mock ImsServiceRegistry mMockImsServiceRegistry;
    @Mock SimInterface mMockSimInterface;
    @Mock TelephonyInterface mMockTelephonyInterface;
    @Mock IAosRegistrationListener mMockAosRegistrationListener;
    @Mock IAosInfoListener mMockAosInfoListener;
    @Mock CarrierConfig mMockCarrierConfig;
    @Mock ConfigInterface mMockConfigInterface;
    @Mock LocationInterface mMockLocationInterface;
    @Mock WifiInterface mMockWifiInterface;
    @Mock NativeStateInterface mMockNativeStateInterface;
    @Mock IDcNetWatcher mMockDcNetWatcher;

    @Before
    public void setup() throws Exception {
        if (Looper.myLooper() == null) {
            Looper.prepare();
        }

        super.setUp(getClass().getSimpleName());
        MockitoAnnotations.initMocks(this);

        when(mMockJniIms.getInterface(JniObjectId.AOS, SLOT_0)).thenReturn(mNativeObject);
        replaceInstance(JniImsProxy.class, "sJniIms", null, mMockJniIms);

        when(mMockImsServiceRegistrys.get(SLOT_0)).thenReturn(mMockImsServiceRegistry);
        replaceInstance(ImsServiceRegistry.class, "sImsServiceRegistrys", null,
                mMockImsServiceRegistrys);

        when(mMockConfigInterface.getCarrierConfig()).thenReturn(mMockCarrierConfig);

        AgentFactory.getInstance().setAgent(ConfigInterface.class, mMockConfigInterface, SLOT_0);
        AgentFactory.getInstance().setAgent(SimInterface.class, mMockSimInterface, SLOT_0);
        AgentFactory.getInstance().setAgent(
                NativeStateInterface.class, mMockNativeStateInterface, SLOT_0);
        AgentFactory.getInstance().setAgent(
                LocationInterface.class, mMockLocationInterface, SLOT_0);
        AgentFactory.getInstance().setAgent(WifiInterface.class, mMockWifiInterface);

        DcFactory.setDcAgent(IDcNetWatcher.class, mMockDcNetWatcher, SLOT_0);

        mAosService = new FakeAosService();
        mAosService.init(SLOT_0);
        mAosService.start();
    }

    @After
    public void tearDown() throws Exception {
        mAosService.stop();
        mAosService.cleanup();

        DcFactory.setDcAgent(IDcNetWatcher.class, null, SLOT_0);

        AgentFactory.getInstance().setAgent(WifiInterface.class, null);
        AgentFactory.getInstance().setAgent(LocationInterface.class, null, SLOT_0);
        AgentFactory.getInstance().setAgent(SimInterface.class, null, SLOT_0);
        AgentFactory.getInstance().setAgent(NativeStateInterface.class, null, SLOT_0);
        AgentFactory.getInstance().setAgent(ConfigInterface.class, null, SLOT_0);

        super.tearDown();
    }

    @Test
    public void initAndStart() {
        // mAosService.init(SLOT_0) and mAosService.start() are called in setup
        verify(mMockJniIms).setListener(mNativeObject, mAosService.getJniImsListenerProxy());
        verify(mMockNativeStateInterface).addListener(any(NativeStateInterface.Listener.class));
        verify(mMockImsServiceRegistry).addListener(mAosService.getServiceRegistryListener());
        verify(mMockSimInterface).addListener(mAosService);
        verify(mMockSimInterface).addIsimListener(mAosService);
    }

    @Test
    public void stopAndCleanup() {
        mAosService.stop();
        verify(mMockSimInterface).removeListener(mAosService);
        verify(mMockSimInterface).removeIsimListener(mAosService);

        mAosService.cleanup();
        verify(mMockImsServiceRegistry).removeListener(mAosService.getServiceRegistryListener());
        verify(mMockNativeStateInterface).removeListener(any(NativeStateInterface.Listener.class));
        verify(mMockJniIms).removeListener(mNativeObject);
        verify(mMockJniIms).releaseInterface(mNativeObject);
    }

    @Test
    public void addAndRemoveAosRegistrationListener() {
        mAosService.addListener(mMockAosRegistrationListener);
        assertEquals(1, mAosService.getAosRegistrationListeners().size());

        mAosService.removeListener(mMockAosRegistrationListener);
        assertEquals(0, mAosService.getAosRegistrationListeners().size());
    }

    @Test
    public void addAndRemoveAosInfoListener() {
        mAosService.addListener(mMockAosInfoListener);
        assertEquals(1, mAosService.getAosInfoListeners().size());

        mAosService.removeListener(mMockAosInfoListener);
        assertEquals(0, mAosService.getAosInfoListeners().size());
    }

    @Test
    public void updateSipDelegateRegistration() {
        byte[] airplaneSettingData = createBytes(IIAosService.J2N_REQUEST_REGISTRATION);

        mAosService.updateSipDelegateRegistration();

        verify(mMockJniIms).sendData(mNativeObject, airplaneSettingData);
    }

    @Test
    public void triggerSipDelegateDeregistration() {
        byte[] airplaneSettingData = createBytes(IIAosService.J2N_REQUEST_DEREGISTRATION);

        mAosService.triggerSipDelegateDeregistration();

        verify(mMockJniIms).sendData(mNativeObject, airplaneSettingData);
    }

    @Test
    public void triggerFullNetworkRegistration() {
        byte[] fullRegistrationData = createBytes(IIAosService.J2N_REQUEST_FULL_REGISTRATION, 500,
                "Server Internal Error");

        mAosService.triggerFullNetworkRegistration(500, "Server Internal Error");

        verify(mMockJniIms).sendData(mNativeObject, fullRegistrationData);
    }

    @Test
    public void triggerFullNetworkRegistration_reasonNull() {
        byte[] fullRegistrationData = createBytes(IIAosService.J2N_REQUEST_FULL_REGISTRATION, 500,
                "");

        mAosService.triggerFullNetworkRegistration(500, null);

        verify(mMockJniIms).sendData(mNativeObject, fullRegistrationData);
    }

    @Test
    public void changeCapabilitiesWithNullThrowsNullPointerException() {

        Throwable exception = assertThrows(NullPointerException.class, () -> {
            mAosService.changeCapabilities(null);
        });

        assertEquals("listener must not be null", exception.getMessage());
    }

    @Test
    public void changeCapabilities_sameCapability() {
        CapabilityPairs oldPairs = new CapabilityPairs(IAosRegistrationListener.NetworkType.LTE,
                IAosRegistrationListener.Capability.UT);
        CapabilityPairs newPairs = new CapabilityPairs(IAosRegistrationListener.NetworkType.LTE,
                IAosRegistrationListener.Capability.UT);
        mAosService.setCapabilityPairs(oldPairs);

        mAosService.changeCapabilities(newPairs);

        verify(mMockJniIms, never()).sendData(eq(mNativeObject), any());
    }

    @Test
    public void changeCapabilities_lteVideo() {
        byte[] capabilityData = createBytes(IIAosService.J2N_REQUEST_CAPABILITIES_CHANGED, 2,
                IAosRegistrationListener.NetworkType.LTE.getValue(),
                IAosRegistrationListener.Capability.VIDEO,
                IAosRegistrationListener.NetworkType.IWLAN.getValue(),
                IAosRegistrationListener.Capability.VIDEO);
        CapabilityPairs pairs = new CapabilityPairs(IAosRegistrationListener.NetworkType.LTE,
                IAosRegistrationListener.Capability.VIDEO);

        mAosService.changeCapabilities(pairs);

        verify(mMockJniIms).sendData(mNativeObject, capabilityData);
        assertTrue(mAosService.getCapabilityPairs().hasCapability(
                IAosRegistrationListener.NetworkType.LTE,
                IAosRegistrationListener.Capability.VIDEO));
        assertTrue(mAosService.getCapabilityPairs().hasCapability(
                IAosRegistrationListener.NetworkType.IWLAN,
                IAosRegistrationListener.Capability.VIDEO));
    }

    @Test
    public void changeCapabilities_nrVideo() {
        byte[] capabilityData = createBytes(IIAosService.J2N_REQUEST_CAPABILITIES_CHANGED, 2,
                IAosRegistrationListener.NetworkType.NR.getValue(),
                IAosRegistrationListener.Capability.VIDEO,
                IAosRegistrationListener.NetworkType.IWLAN.getValue(),
                IAosRegistrationListener.Capability.VIDEO);
        CapabilityPairs pairs = new CapabilityPairs(IAosRegistrationListener.NetworkType.NR,
                IAosRegistrationListener.Capability.VIDEO);

        mAosService.changeCapabilities(pairs);

        verify(mMockJniIms).sendData(mNativeObject, capabilityData);
        assertTrue(mAosService.getCapabilityPairs().hasCapability(
                IAosRegistrationListener.NetworkType.NR,
                IAosRegistrationListener.Capability.VIDEO));
        assertTrue(mAosService.getCapabilityPairs().hasCapability(
                IAosRegistrationListener.NetworkType.IWLAN,
                IAosRegistrationListener.Capability.VIDEO));
    }

    @Test
    public void controlRegistration() {
        byte[] registrationData = createBytes(IIAosService.J2N_REQUEST_CONTROL_REGISTRATION,
                IAosRegistration.RequestType.START.getValue(),
                IAosRegistration.Pcscf.CURRENT.getValue(),
                IAosRegistration.Cause.IMS_SERVICE.getValue());

        mAosService.controlRegistration(IAosRegistration.RequestType.START,
                IAosRegistration.Pcscf.CURRENT,
                IAosRegistration.Cause.IMS_SERVICE);

        verify(mMockJniIms).sendData(mNativeObject, registrationData);
    }

    @Test
    public void getRegisteredNetworkType() {
        mAosService.setRegisteredNetworkType(NetworkType.UTRAN);

        NetworkType registeredNetworkType = mAosService.getRegisteredNetworkType();

        assertEquals(NetworkType.UTRAN, registeredNetworkType);
    }

    // currently, not used
    @Test
    public void notifyDataRoamingSetting() {
        byte[] dataRoamingSettingData = createBytes(IIAosService.J2N_NOTIFY_DATA_ROAMING_SETTING,
                false);

        mAosService.notifyDataRoamingSetting(false);

        verify(mMockJniIms).sendData(mNativeObject, dataRoamingSettingData);
    }

    @Test
    public void notifyMobileDataSetting() {
        byte[] mobileDataSettingData = createBytes(IIAosService.J2N_NOTIFY_MOBILE_DATA_SETTING,
                true);

        mAosService.notifyMobileDataSetting(true);

        verify(mMockJniIms).sendData(mNativeObject, mobileDataSettingData);
    }

    // currently, not used
    @Test
    public void notifyRoamingPreferredVoiceNetwork() {
        byte[] preferredVoiceNetworkData = createBytes(
                IIAosService.J2N_NOTIFY_ROAMING_PREFERRED_VOICE_NETWORK,
                RoamingPreferredVoiceNetwork.CELLULAR.getValue());

        mAosService.notifyRoamingPreferredVoiceNetwork(RoamingPreferredVoiceNetwork.CELLULAR);

        verify(mMockJniIms).sendData(mNativeObject, preferredVoiceNetworkData);
    }

    // currently, not used
    @Test
    public void notifyServiceSetting() {
        byte[] serviceSettingData = createBytes(IIAosService.J2N_NOTIFY_SERVICE_SETTING,
                ServiceSetting.ON.getValue(), FeatureTagMask.MMTEL);

        mAosService.notifyServiceSetting(ServiceSetting.ON, FeatureTagMask.MMTEL);

        verify(mMockJniIms).sendData(mNativeObject, serviceSettingData);
    }

    // currently, not used
    @Test
    public void notifyTtySetting() {
        byte[] ttySettingData = createBytes(IIAosService.J2N_NOTIFY_TTY_SETTING, true);

        mAosService.notifyTtySetting(true);

        verify(mMockJniIms).sendData(mNativeObject, ttySettingData);
    }

    // currently, not used
    @Test
    public void notifyVideoSetting() {
        byte[] videoSettingData = createBytes(IIAosService.J2N_NOTIFY_VIDEO_SETTING, true);

        mAosService.notifyVideoSetting(true);

        verify(mMockJniIms).sendData(mNativeObject, videoSettingData);
    }

    // currently, not used
    @Test
    public void notifyVolteSetting() {
        byte[] volteSettingData = createBytes(IIAosService.J2N_NOTIFY_VOLTE_SETTING, true);

        mAosService.notifyVolteSetting(true);

        verify(mMockJniIms).sendData(mNativeObject, volteSettingData);
    }

    // currently, not used
    @Test
    public void notifyWfcSetting() {
        byte[] wfcSettingData = createBytes(IIAosService.J2N_NOTIFY_WFC_SETTING, true);

        mAosService.notifyWfcSetting(true);

        verify(mMockJniIms).sendData(mNativeObject, wfcSettingData);
    }

    @Test
    public void notifyAosStart() {
        byte[] aosStartData = createBytes(IIAosService.J2N_NOTIFY_AOS_START);

        mAosService.notifyAosStart();

        verify(mMockJniIms).sendData(mNativeObject, aosStartData);
    }

    @Test
    public void notifyIsimState() {
        byte[] isimStateData = createBytes(IIAosService.J2N_NOTIFY_ISIM_STATE,
                Sim.ISIM_STATE_LOADED);

        mAosService.notifyIsimState(Sim.ISIM_STATE_LOADED);

        verify(mMockJniIms).sendData(mNativeObject, isimStateData);
    }

    // currently, not used
    @Test
    public void notifyMobileDataLimit() {
        byte[] dataLimitStateData = createBytes(IIAosService.J2N_NOTIFY_MOBILE_DATA_LIMIT, false);

        mAosService.notifyMobileDataLimit(false);

        verify(mMockJniIms).sendData(mNativeObject, dataLimitStateData);
    }

    // currently, not used
    @Test
    public void notifyNetworkVideoCapability() {
        byte[] networkVideoCapabilityData =
                createBytes(IIAosService.J2N_NOTIFY_NETWORK_VIDEO_CAPABILITY, true);

        mAosService.notifyNetworkVideoCapability(true);

        verify(mMockJniIms).sendData(mNativeObject, networkVideoCapabilityData);
    }

    @Test
    public void notifyPhoneNumberState() {
        byte[] phoneNumberStateData = createBytes(IIAosService.J2N_NOTIFY_PHONE_NUMBER_STATE, 1,
                PhoneNumberState.SIM_LOADED.getValue());

        mAosService.notifyPhoneNumberState(true, PhoneNumberState.SIM_LOADED);

        verify(mMockJniIms).sendData(mNativeObject, phoneNumberStateData);
    }

    @Test
    public void notifyPowerOff() {
        byte[] powerOffData = createBytes(IIAosService.J2N_NOTIFY_POWER_OFF);

        mAosService.notifyPowerOff();

        verify(mMockJniIms).sendData(mNativeObject, powerOffData);
    }

    @Test
    public void notifyCarrierSignalPcoValueChanged_pcoDataLengthIsFour() {
        byte[] value = createBytes(IIAosService.J2N_NOTIFY_CARRIER_SIGNAL_PCO_VALUE_CHANGED,
                PCO_LIMITED_SERVICE_VALUE);

        // pcoData length is 4
        byte[] pcoData = {1, 2, 3, 5};

        Intent intent = new Intent();
        intent.setAction(TelephonyManager.ACTION_CARRIER_SIGNAL_PCO_VALUE);
        intent.putExtra(SubscriptionManager.EXTRA_SUBSCRIPTION_INDEX, 0);
        intent.putExtra(TelephonyManager.EXTRA_APN_TYPE, ApnSetting.TYPE_IMS);
        intent.putExtra(TelephonyManager.EXTRA_PCO_ID, AosService.PCO_TARGET_ID);
        intent.putExtra(TelephonyManager.EXTRA_PCO_VALUE, pcoData);

        mAosService.notifyCarrierSignalPcoValueChanged(intent);
        processAllMessages();

        verify(mMockJniIms).sendData(mNativeObject, value);
    }

    @Test
    public void notifyCarrierSignalPcoValueChanged_pcoDataLengthIsOne() {
        byte[] value = createBytes(IIAosService.J2N_NOTIFY_CARRIER_SIGNAL_PCO_VALUE_CHANGED,
                PCO_LIMITED_SERVICE_VALUE);

        // pcoData length is 1
        byte[] pcoData = {5};

        Intent intent = new Intent();
        intent.setAction(TelephonyManager.ACTION_CARRIER_SIGNAL_PCO_VALUE);
        intent.putExtra(SubscriptionManager.EXTRA_SUBSCRIPTION_INDEX, 0);
        intent.putExtra(TelephonyManager.EXTRA_APN_TYPE, ApnSetting.TYPE_IMS);
        intent.putExtra(TelephonyManager.EXTRA_PCO_ID, AosService.PCO_TARGET_ID);
        intent.putExtra(TelephonyManager.EXTRA_PCO_VALUE, pcoData);

        mAosService.notifyCarrierSignalPcoValueChanged(intent);
        processAllMessages();

        verify(mMockJniIms).sendData(mNativeObject, value);
    }

    @Test
    public void notifyCarrierSignalPcoValueChanged_pcoDataLengthIsInvalid() {
        byte[] value = createBytes(IIAosService.J2N_NOTIFY_CARRIER_SIGNAL_PCO_VALUE_CHANGED,
                PCO_NONE_VALUE);

        // Expected lengths of pcoData are 1 and 4.
        byte[] pcoData = {5, 5, 5};

        Intent intent = new Intent();
        intent.setAction(TelephonyManager.ACTION_CARRIER_SIGNAL_PCO_VALUE);
        intent.putExtra(SubscriptionManager.EXTRA_SUBSCRIPTION_INDEX, 0);
        intent.putExtra(TelephonyManager.EXTRA_APN_TYPE, ApnSetting.TYPE_IMS);
        intent.putExtra(TelephonyManager.EXTRA_PCO_ID, AosService.PCO_TARGET_ID);
        intent.putExtra(TelephonyManager.EXTRA_PCO_VALUE, pcoData);

        mAosService.notifyCarrierSignalPcoValueChanged(intent);
        processAllMessages();

        verify(mMockJniIms).sendData(mNativeObject, value);
    }

    @Test
    public void notifyCarrierSignalPcoValueChanged_unexpectedApn() {
        byte[] value = createBytes(IIAosService.J2N_NOTIFY_CARRIER_SIGNAL_PCO_VALUE_CHANGED,
                PCO_LIMITED_SERVICE_VALUE);
        byte[] pcoData = {1, 2, 3, 5};

        Intent intent = new Intent();
        intent.setAction(TelephonyManager.ACTION_CARRIER_SIGNAL_PCO_VALUE);
        intent.putExtra(SubscriptionManager.EXTRA_SUBSCRIPTION_INDEX, 0);
        // Set unexpected Apn
        intent.putExtra(TelephonyManager.EXTRA_APN_TYPE, ApnSetting.TYPE_MMS);
        intent.putExtra(TelephonyManager.EXTRA_PCO_ID, AosService.PCO_TARGET_ID);
        intent.putExtra(TelephonyManager.EXTRA_PCO_VALUE, pcoData);

        mAosService.notifyCarrierSignalPcoValueChanged(intent);
        processAllMessages();

        verify(mMockJniIms, times(0)).sendData(mNativeObject, value);
    }

    @Test
    public void notifyCarrierSignalPcoValueChanged_unexpectedTargetId() {
        byte[] value = createBytes(IIAosService.J2N_NOTIFY_CARRIER_SIGNAL_PCO_VALUE_CHANGED,
                PCO_LIMITED_SERVICE_VALUE);
        byte[] pcoData = {1, 2, 3, 5};

        Intent intent = new Intent();
        intent.setAction(TelephonyManager.ACTION_CARRIER_SIGNAL_PCO_VALUE);
        intent.putExtra(SubscriptionManager.EXTRA_SUBSCRIPTION_INDEX, 0);
        intent.putExtra(TelephonyManager.EXTRA_APN_TYPE, ApnSetting.TYPE_IMS);
        // Set unexpected Target Id
        intent.putExtra(TelephonyManager.EXTRA_PCO_ID, 0xff01);
        intent.putExtra(TelephonyManager.EXTRA_PCO_VALUE, pcoData);

        mAosService.notifyCarrierSignalPcoValueChanged(intent);
        processAllMessages();

        verify(mMockJniIms, times(0)).sendData(mNativeObject, value);
    }

    @Test
    public void notifyNasSecurityAlgorithmChanged() {
        byte[] nasSecurityAlgoData = createBytes(IIAosService.J2N_NOTIFY_NAS_ALGORITHM_CHANGED,
                false);

        mAosService.notifyNasSecurityAlgorithmChanged(false);

        verify(mMockJniIms).sendData(mNativeObject, nasSecurityAlgoData);
    }

    @Test
    public void notifyllowedNetworkTypesChanged() {
        byte[] allowedNetworkTypesData = createBytesWithLong(
                IIAosService.J2N_NOTIFY_ALLOWED_NETWORK_TYPES_CHANGED, 1L);

        mAosService.notifyAllowedNetworkTypesChanged(1L);

        verify(mMockJniIms).sendData(mNativeObject, allowedNetworkTypesData);
    }

    @Test
    public void onSimStateChanged_simLoaded() {
        byte[] simStateData = createBytes(IIAosService.J2N_NOTIFY_PHONE_NUMBER_STATE, 0,
                PhoneNumberState.SIM_LOADED.getValue());
        when(mMockSimInterface.isSimLoaded()).thenReturn(true);

        mAosService.onSimStateChanged();

        verify(mMockJniIms).sendData(mNativeObject, simStateData);
    }

    @Test
    public void onSimStateChanged_simNotLoaded() {
        when(mMockSimInterface.isSimLoaded()).thenReturn(false);

        mAosService.onSimStateChanged();

        verify(mMockJniIms, never()).sendData(eq(mNativeObject), any());
    }

    @Test
    public void onIsimStateChanged_notPresent() {
        byte[] isimStateData = createBytes(IIAosService.J2N_NOTIFY_ISIM_STATE,
                Sim.ISIM_STATE_NOT_PRESENT);
        when(mMockSimInterface.getIsimState()).thenReturn(Sim.ISIM_STATE_NOT_PRESENT);

        mAosService.onIsimStateChanged();

        verify(mMockJniIms).sendData(mNativeObject, isimStateData);
    }

    @Test
    public void onIsimStateChanged_notReady() {
        byte[] isimStateData = createBytes(IIAosService.J2N_NOTIFY_ISIM_STATE,
                Sim.ISIM_STATE_NOT_READY);
        when(mMockSimInterface.getIsimState()).thenReturn(Sim.ISIM_STATE_NOT_READY);

        mAosService.onIsimStateChanged();

        verify(mMockJniIms).sendData(mNativeObject, isimStateData);
    }

    @Test
    public void onIsimStateChanged_loaded() {
        byte[] isimStateData = createBytes(IIAosService.J2N_NOTIFY_ISIM_STATE,
                Sim.ISIM_STATE_LOADED);
        when(mMockSimInterface.getIsimState()).thenReturn(Sim.ISIM_STATE_LOADED);

        mAosService.onIsimStateChanged();

        verify(mMockJniIms).sendData(mNativeObject, isimStateData);
    }

    @Test
    public void onIsimStateChanged_refreshStarted() {
        byte[] isimStateData = createBytes(IIAosService.J2N_NOTIFY_ISIM_STATE,
                Sim.ISIM_STATE_REFRESH_STARTED);
        when(mMockSimInterface.getIsimState()).thenReturn(Sim.ISIM_STATE_REFRESH_STARTED);

        mAosService.onIsimStateChanged();

        verify(mMockJniIms).sendData(mNativeObject, isimStateData);
    }

    @Test
    public void onPreciseCallStateChanged_csCallStateIsIdle_notifyPreciseCallStateWithIdle() {
        AgentFactory.getInstance().setAgent(
                    TelephonyInterface.class, mMockTelephonyInterface, SLOT_0);
        byte[] callStateData = createBytes(IIAosService.J2N_NOTIFY_PRECISE_CALL_STATE,
                PreciseCallState.PRECISE_CALL_STATE_IDLE);
        PreciseCallState preciseCallState = new PreciseCallState(
                PreciseCallState.PRECISE_CALL_STATE_NOT_VALID,
                PreciseCallState.PRECISE_CALL_STATE_ACTIVE,
                PreciseCallState.PRECISE_CALL_STATE_IDLE,
                DisconnectCause.NOT_VALID,
                PreciseDisconnectCause.NOT_VALID);
        mAosService.setPreciseCallState(PreciseCallState.PRECISE_CALL_STATE_ACTIVE);
        when(mMockTelephonyInterface.getCsCallState()).thenReturn(TelephonyManager.CALL_STATE_IDLE);

        mAosService.onPreciseCallStateChanged(preciseCallState);

        verify(mMockJniIms).sendData(mNativeObject, callStateData);
    }

    @Test
    public void onPreciseCallStateChanged_notifyPreciseCallStateWithBackgroundCall() {
        AgentFactory.getInstance().setAgent(
                    TelephonyInterface.class, mMockTelephonyInterface, SLOT_0);
        byte[] callStateData = createBytes(IIAosService.J2N_NOTIFY_PRECISE_CALL_STATE,
                PreciseCallState.PRECISE_CALL_STATE_ACTIVE);
        PreciseCallState preciseCallState = new PreciseCallState(
                PreciseCallState.PRECISE_CALL_STATE_NOT_VALID,
                PreciseCallState.PRECISE_CALL_STATE_IDLE,
                PreciseCallState.PRECISE_CALL_STATE_ACTIVE,
                DisconnectCause.NOT_VALID,
                PreciseDisconnectCause.NOT_VALID);
        when(mMockTelephonyInterface.getCsCallState())
                .thenReturn(TelephonyManager.CALL_STATE_OFFHOOK);

        mAosService.onPreciseCallStateChanged(preciseCallState);

        verify(mMockJniIms).sendData(mNativeObject, callStateData);
    }

    @Test
    public void onPreciseCallStateChanged_notifyPreciseCallStateWithForegroundCall() {
        AgentFactory.getInstance().setAgent(
                    TelephonyInterface.class, mMockTelephonyInterface, SLOT_0);
        byte[] callStateData = createBytes(IIAosService.J2N_NOTIFY_PRECISE_CALL_STATE,
                PreciseCallState.PRECISE_CALL_STATE_ACTIVE);
        PreciseCallState preciseCallState = new PreciseCallState(
                PreciseCallState.PRECISE_CALL_STATE_NOT_VALID,
                PreciseCallState.PRECISE_CALL_STATE_ACTIVE,
                PreciseCallState.PRECISE_CALL_STATE_IDLE,
                DisconnectCause.NOT_VALID,
                PreciseDisconnectCause.NOT_VALID);
        when(mMockTelephonyInterface.getCsCallState())
                .thenReturn(TelephonyManager.CALL_STATE_OFFHOOK);

        mAosService.onPreciseCallStateChanged(preciseCallState);

        verify(mMockJniIms).sendData(mNativeObject, callStateData);
    }

    @Test
    public void onLastKnownCountryUpdated_notifyCountryChanged() {
        ArgumentCaptor<LocationInterface.Listener> listenerCaptor =
                ArgumentCaptor.forClass(LocationInterface.Listener.class);
        verify(mMockLocationInterface).addListener(listenerCaptor.capture());

        LocationInterface.Listener listener = listenerCaptor.getValue();
        listener.onLastKnownCountryUpdated();

        byte[] locationInfoData = createBytes(IIAosService.J2N_NOTIFY_LOCATION_INFO,
                LocationInfo.COUNTRY_CHANGED.getValue());
        verify(mMockJniIms).sendData(mNativeObject, locationInfoData);
    }

    @Test
    public void onInstantRequestedLocationUpdated_notifyLocationUpdatedByRequest() {
        ArgumentCaptor<LocationInterface.Listener> listenerCaptor =
                ArgumentCaptor.forClass(LocationInterface.Listener.class);
        verify(mMockLocationInterface).addListener(listenerCaptor.capture());

        LocationInterface.Listener listener = listenerCaptor.getValue();
        listener.onInstantRequestedLocationUpdated();

        byte[] locationInfoData = createBytes(IIAosService.J2N_NOTIFY_LOCATION_INFO,
                LocationInfo.FIXED.getValue());
        verify(mMockJniIms).sendData(mNativeObject, locationInfoData);
    }

    @Test
    public void onWifiStateChanged_notifyWifiSettingChanged() {
        byte[] wifiSettingData = createBytes(IIAosService.J2N_NOTIFY_WIFI_SETTING, true);
        when(mMockWifiInterface.isWifiEnabled()).thenReturn(true);
        ArgumentCaptor<WifiInterface.Listener> listenerCaptor =
                ArgumentCaptor.forClass(WifiInterface.Listener.class);
        verify(mMockWifiInterface).addListener(listenerCaptor.capture());

        WifiInterface.Listener listener = listenerCaptor.getValue();
        listener.onWifiStateChanged();

        verify(mMockJniIms).sendData(mNativeObject, wifiSettingData);
    }

    @Test
    public void onNetworkOperatorChanged_notifyChange() {
        Parcel parcel = Parcel.obtain();
        parcel.writeInt(IIAosService.J2N_NOTIFY_PLMN_CHANGED);
        parcel.writeString("123456");
        byte[] plmnChangedData = parcel.marshall();
        parcel.recycle();

        mAosService.onNetworkOperatorChanged("123456");

        verify(mMockJniIms).sendData(mNativeObject, plmnChangedData);
    }

    @Test
    public void onAirplaneModeChanged_notifyChange() {
        byte[] airplaneSettingData = createBytes(IIAosService.J2N_NOTIFY_AIRPLANE_SETTING, true);

        mAosService.onAirplaneModeChanged(true);

        verify(mMockJniIms).sendData(mNativeObject, airplaneSettingData);
    }

    @Test
    public void onVopsStateChanged_notifyChange() {
        Parcel parcel = Parcel.obtain();
        parcel.writeInt(IIAosService.J2N_NOTIFY_VOPS_STATE_CHANGED);
        parcel.writeInt(ImsEventDef.IMS_VOICE_OVER_PS_NOT_SUPPORTED);
        parcel.writeString("123456");
        byte[] vopsData = parcel.marshall();
        parcel.recycle();

        mAosService.onVopsStateChanged(ImsEventDef.IMS_VOICE_OVER_PS_NOT_SUPPORTED, "123456");

        verify(mMockJniIms).sendData(mNativeObject, vopsData);
    }

    @Test
    public void onHandoverStateChanged_doNotNotifyIpcanHandoverExceptFailure() {
        int reason = android.telephony.DataFailCause.OPERATOR_BARRED;
        byte[] ipcanHandoverFailureData = createBytes(
                IIAosService.J2N_NOTIFY_IPCAN_HANDOVER_FAILURE, IApn.IPCAN_CATEGORY_MOBILE, reason);

        mAosService.onHandoverStateChanged(IApn.HANDOVER_START,
                TelephonyManager.NETWORK_TYPE_IWLAN, reason);
        mAosService.onHandoverStateChanged(IApn.HANDOVER_SUCCESS,
                TelephonyManager.NETWORK_TYPE_IWLAN, reason);

        verify(mMockJniIms, never()).sendData(mNativeObject, ipcanHandoverFailureData);
    }

    @Test
    public void onHandoverStateChanged_notifyIpcanHandoverFailure() {
        int reason = android.telephony.DataFailCause.OPERATOR_BARRED;
        byte[] ipcanHandoverFailureData = createBytes(
                IIAosService.J2N_NOTIFY_IPCAN_HANDOVER_FAILURE, IApn.IPCAN_CATEGORY_MOBILE, reason);

        mAosService.onHandoverStateChanged(IApn.HANDOVER_FAILURE,
                TelephonyManager.NETWORK_TYPE_IWLAN, reason);

        verify(mMockJniIms).sendData(mNativeObject, ipcanHandoverFailureData);
    }

    @Test
    public void onHandoverStateChanged_notifyIkeAuthFailureForWfcActivation() {
        mAosService.addListener(mMockAosRegistrationListener);
        // Do not notify IKE auth failure if KEY_NOTIFY_IKE_AUTH_FAILURE_FOR_WFC_ACTIVATION_BOOL
        // is false.
        mAosService.onHandoverStateChanged(IApn.HANDOVER_FAILURE,
                TelephonyManager.NETWORK_TYPE_IWLAN,
                android.telephony.DataFailCause.IWLAN_IKEV2_AUTH_FAILURE);
        processAllMessages();
        verify(mMockAosRegistrationListener, never()).notifyTechnologyChangeFailed(
                RegistrationType.NORMAL, NetworkType.IWLAN,
                ReasonCode.DATA_EPDG_TUNNEL_IKEV2_AUTH_FAILURE, null);

        // Notify IKE auth failure if KEY_NOTIFY_IKE_AUTH_FAILURE_FOR_WFC_ACTIVATION_BOOL is true.
        when(mMockCarrierConfig.getBoolean(
                CarrierConfig.ImsWfc.KEY_NOTIFY_IKE_AUTH_FAILURE_FOR_WFC_ACTIVATION_BOOL))
                .thenReturn(true);
        mAosService.onHandoverStateChanged(IApn.HANDOVER_FAILURE,
                TelephonyManager.NETWORK_TYPE_IWLAN,
                android.telephony.DataFailCause.IWLAN_IKEV2_AUTH_FAILURE);
        processAllMessages();
        verify(mMockAosRegistrationListener).notifyTechnologyChangeFailed(RegistrationType.NORMAL,
                NetworkType.IWLAN, ReasonCode.DATA_EPDG_TUNNEL_IKEV2_AUTH_FAILURE, null);
    }

    @Test
    public void onCrossSimStatusChanged_doNotUpdateRegisteredNetworkTypeForCellular() {
        mAosService.addListener(mMockAosRegistrationListener);
        mAosService.setConnectedOverCrossSim(false);
        mAosService.setRegisteredNetworkType(NetworkType.LTE);

        mAosService.onCrossSimStatusChanged(true);

        assertEquals(NetworkType.LTE, mAosService.getRegisteredNetworkType());
        verifyNoMoreInteractions(mMockAosRegistrationListener);
    }

    @Test
    public void onCrossSimStatusChanged_updateRegisteredNetworkTypeToIwlan() {
        byte[] crossSimInfo = createBytes(IIAosService.J2N_NOTIFY_CROSS_SIM_STATUS,
                CrossSimStatus.DATA_DISCONNECTED.getValue());

        mAosService.addListener(mMockAosRegistrationListener);
        mAosService.setConnectedOverCrossSim(true);
        mAosService.setRegisteredNetworkType(NetworkType.CROSS_SIM);

        mAosService.onCrossSimStatusChanged(false);

        assertEquals(NetworkType.IWLAN, mAosService.getRegisteredNetworkType());
        verify(mMockAosRegistrationListener).notifyRegistered(RegistrationType.NORMAL,
                NetworkType.IWLAN, mAosService.getFeatureTagBits(), mAosService.getFeatureTags());
        verify(mMockJniIms).sendData(mNativeObject, crossSimInfo);
    }

    @Test
    public void onCrossSimStatusChanged_updateRegisteredNetworkTypeToCrossSim() {
        byte[] crossSimInfo = createBytes(IIAosService.J2N_NOTIFY_CROSS_SIM_STATUS,
                CrossSimStatus.DATA_CONNECTED.getValue());

        mAosService.addListener(mMockAosRegistrationListener);
        mAosService.setConnectedOverCrossSim(false);
        mAosService.setRegisteredNetworkType(NetworkType.IWLAN);

        mAosService.onCrossSimStatusChanged(true);

        assertEquals(NetworkType.CROSS_SIM, mAosService.getRegisteredNetworkType());
        verify(mMockAosRegistrationListener).notifyRegistered(RegistrationType.NORMAL,
                NetworkType.CROSS_SIM, mAosService.getFeatureTagBits(),
                mAosService.getFeatureTags());
        verify(mMockJniIms).sendData(mNativeObject, crossSimInfo);
    }

    @Test
    public void onPreciseDataConnectionStateChanged_shouldNotHandleNonImsTypeApn() {
        byte[] startEstTimerData = createBytes(IIAosService.J2N_REQUEST_CONTROL_REGISTRATION,
                IAosRegistration.RequestType.START_IMS_EST_TIMER.getValue(),
                IAosRegistration.Pcscf.CURRENT.getValue(),
                IAosRegistration.Cause.DATA_CONNECTING.getValue());

        mAosService.onPreciseDataConnectionStateChanged(EApnType.EMERGENCY.getType(),
                TelephonyManager.DATA_CONNECTING, 0, TelephonyManager.NETWORK_TYPE_LTE);

        verify(mMockJniIms, never()).sendData(mNativeObject, startEstTimerData);
    }

    @Test
    public void onPreciseDataConnectionStateChanged_controlRegistrationUponConnecting() {
        byte[] startEstTimerData = createBytes(IIAosService.J2N_REQUEST_CONTROL_REGISTRATION,
                IAosRegistration.RequestType.START_IMS_EST_TIMER.getValue(),
                IAosRegistration.Pcscf.CURRENT.getValue(),
                IAosRegistration.Cause.DATA_CONNECTING.getValue());

        mAosService.onPreciseDataConnectionStateChanged(EApnType.IMS.getType(),
                TelephonyManager.DATA_CONNECTING, 0, TelephonyManager.NETWORK_TYPE_LTE);

        verify(mMockJniIms).sendData(mNativeObject, startEstTimerData);
    }

    @Test
    public void onPreciseDataConnectionStateChanged_controlRegistrationUponDisConnecting() {
        byte[] stopData = createBytes(IIAosService.J2N_REQUEST_CONTROL_REGISTRATION,
                IAosRegistration.RequestType.STOP.getValue(),
                IAosRegistration.Pcscf.CURRENT.getValue(),
                IAosRegistration.Cause.DATA.getValue());
        mAosService.mRegisteredNetworkType = NetworkType.LTE;

        mAosService.onPreciseDataConnectionStateChanged(
                EApnType.IMS.getType(), TelephonyManager.DATA_DISCONNECTING, 0,
                TelephonyManager.NETWORK_TYPE_LTE);

        verify(mMockJniIms).sendData(mNativeObject, stopData);
    }

    @Test
    public void onPreciseDataConnectionStateChanged_updateDataFailureReasonUponDisconnecting() {
        int anyDataFailReason = 1;
        byte[] updateData = createBytes(IIAosService.J2N_UPDATE_DATA_FAILURE_REASON,
                anyDataFailReason);
        mAosService.mRegisteredNetworkType = NetworkType.LTE;

        mAosService.onPreciseDataConnectionStateChanged(
                EApnType.IMS.getType(), TelephonyManager.DATA_DISCONNECTING, anyDataFailReason,
                TelephonyManager.NETWORK_TYPE_LTE);

        verify(mMockJniIms).sendData(mNativeObject, updateData);
    }

    @Test
    public void onPreciseDataConnectionStateChanged_updateDataFailureReasonUponDisconnected() {
        int anyDataFailReason = 1;
        byte[] updateData = createBytes(IIAosService.J2N_UPDATE_DATA_FAILURE_REASON,
                anyDataFailReason);
        mAosService.mRegisteredNetworkType = NetworkType.LTE;

        mAosService.onPreciseDataConnectionStateChanged(
                EApnType.IMS.getType(), TelephonyManager.DATA_DISCONNECTED, anyDataFailReason,
                TelephonyManager.NETWORK_TYPE_LTE);

        verify(mMockJniIms).sendData(mNativeObject, updateData);
    }

    @Test
    public void onPreciseDataConnectionStateChanged_notifyIkeAuthFailureForWfcActivation() {
        mAosService.addListener(mMockAosRegistrationListener);
        // Do not notify IKE auth failure if KEY_NOTIFY_IKE_AUTH_FAILURE_FOR_WFC_ACTIVATION_BOOL
        // is false.
        mAosService.onPreciseDataConnectionStateChanged(EApnType.IMS.getType(),
                TelephonyManager.DATA_DISCONNECTED,
                android.telephony.DataFailCause.IWLAN_IKEV2_AUTH_FAILURE,
                TelephonyManager.NETWORK_TYPE_IWLAN);
        processAllMessages();
        verify(mMockAosRegistrationListener, never()).notifyDeregistered(RegistrationType.NORMAL,
                NetworkType.IWLAN, ReasonCode.DATA_EPDG_TUNNEL_IKEV2_AUTH_FAILURE, null);

        // Notify IKE auth failure if KEY_NOTIFY_IKE_AUTH_FAILURE_FOR_WFC_ACTIVATION_BOOL is true.
        when(mMockCarrierConfig.getBoolean(
                CarrierConfig.ImsWfc.KEY_NOTIFY_IKE_AUTH_FAILURE_FOR_WFC_ACTIVATION_BOOL))
                .thenReturn(true);
        mAosService.addListener(mMockAosRegistrationListener);
        mAosService.onPreciseDataConnectionStateChanged(EApnType.IMS.getType(),
                TelephonyManager.DATA_DISCONNECTED,
                android.telephony.DataFailCause.IWLAN_IKEV2_AUTH_FAILURE,
                TelephonyManager.NETWORK_TYPE_IWLAN);
        processAllMessages();
        verify(mMockAosRegistrationListener).notifyDeregistered(RegistrationType.NORMAL,
                NetworkType.IWLAN, ReasonCode.DATA_EPDG_TUNNEL_IKEV2_AUTH_FAILURE, null);
    }

    @Test
    public void nativeStateListener_onNativeServiceReadyWhenSimNotPresent() {
        byte[] registrationData = createBytes(IIAosService.J2N_REQUEST_CONTROL_REGISTRATION,
                IAosRegistration.RequestType.START.getValue(),
                IAosRegistration.Pcscf.CURRENT.getValue(),
                IAosRegistration.Cause.IMS_SERVICE.getValue());
        byte[] isimStateData = createBytes(IIAosService.J2N_NOTIFY_ISIM_STATE,
                Sim.ISIM_STATE_NOT_PRESENT);
        byte[] aosStartData = createBytes(IIAosService.J2N_NOTIFY_AOS_START);
        when(mMockImsServiceRegistry.isImsEnabled()).thenReturn(true);
        when(mMockSimInterface.isSimLoaded()).thenReturn(false);
        when(mMockSimInterface.getIsimState()).thenReturn(Sim.ISIM_STATE_NOT_PRESENT);
        ArgumentCaptor<NativeStateInterface.Listener> listenerCaptor =
                ArgumentCaptor.forClass(NativeStateInterface.Listener.class);
        verify(mMockNativeStateInterface).addListener(listenerCaptor.capture());

        NativeStateInterface.Listener listener = listenerCaptor.getValue();
        listener.onNativeServiceReady();

        verify(mMockJniIms, times(3)).sendData(eq(mNativeObject), any());
        verify(mMockJniIms).sendData(mNativeObject, registrationData);
        verify(mMockJniIms).sendData(mNativeObject, isimStateData);
        verify(mMockJniIms).sendData(mNativeObject, aosStartData);
    }

    @Test
    public void nativeStateListener_onNativeServiceReadyWhenSimNotReady() {
        byte[] registrationData = createBytes(IIAosService.J2N_REQUEST_CONTROL_REGISTRATION,
                IAosRegistration.RequestType.START.getValue(),
                IAosRegistration.Pcscf.CURRENT.getValue(),
                IAosRegistration.Cause.IMS_SERVICE.getValue());
        byte[] isimStateData = createBytes(IIAosService.J2N_NOTIFY_ISIM_STATE,
                Sim.ISIM_STATE_NOT_READY);
        byte[] aosStartData = createBytes(IIAosService.J2N_NOTIFY_AOS_START);
        when(mMockImsServiceRegistry.isImsEnabled()).thenReturn(true);
        when(mMockSimInterface.isSimLoaded()).thenReturn(false);
        when(mMockSimInterface.getIsimState()).thenReturn(Sim.ISIM_STATE_NOT_READY);
        ArgumentCaptor<NativeStateInterface.Listener> listenerCaptor =
                ArgumentCaptor.forClass(NativeStateInterface.Listener.class);
        verify(mMockNativeStateInterface).addListener(listenerCaptor.capture());

        NativeStateInterface.Listener listener = listenerCaptor.getValue();
        listener.onNativeServiceReady();

        verify(mMockJniIms, times(3)).sendData(eq(mNativeObject), any());
        verify(mMockJniIms).sendData(mNativeObject, registrationData);
        verify(mMockJniIms).sendData(mNativeObject, isimStateData);
        verify(mMockJniIms).sendData(mNativeObject, aosStartData);
    }

    @Test
    public void nativeStateListener_onNativeServiceReadyWhenSimLoaded() {
        byte[] capabilityData = createBytes(IIAosService.J2N_REQUEST_CAPABILITIES_CHANGED, 1,
                IAosRegistrationListener.NetworkType.LTE.getValue(),
                IAosRegistrationListener.Capability.UT);
        byte[] registrationData = createBytes(IIAosService.J2N_REQUEST_CONTROL_REGISTRATION,
                IAosRegistration.RequestType.START.getValue(),
                IAosRegistration.Pcscf.CURRENT.getValue(),
                IAosRegistration.Cause.IMS_SERVICE.getValue());
        byte[] phoneNumberStateData = createBytes(IIAosService.J2N_NOTIFY_PHONE_NUMBER_STATE, 0,
                PhoneNumberState.SIM_LOADED.getValue());
        byte[] isimStateData = createBytes(IIAosService.J2N_NOTIFY_ISIM_STATE,
                Sim.ISIM_STATE_LOADED);
        byte[] aosStartData = createBytes(IIAosService.J2N_NOTIFY_AOS_START);
        mAosService.setCapabilityPairs(new CapabilityPairs(IAosRegistrationListener.NetworkType.LTE,
                IAosRegistrationListener.Capability.UT));
        when(mMockImsServiceRegistry.isImsEnabled()).thenReturn(true);
        when(mMockSimInterface.isSimLoaded()).thenReturn(true);
        when(mMockSimInterface.getIsimState()).thenReturn(Sim.ISIM_STATE_LOADED);
        ArgumentCaptor<NativeStateInterface.Listener> listenerCaptor =
                ArgumentCaptor.forClass(NativeStateInterface.Listener.class);
        verify(mMockNativeStateInterface).addListener(listenerCaptor.capture());

        NativeStateInterface.Listener listener = listenerCaptor.getValue();
        listener.onNativeServiceReady();

        verify(mMockJniIms, times(5)).sendData(eq(mNativeObject), any());
        verify(mMockJniIms).sendData(mNativeObject, capabilityData);
        verify(mMockJniIms).sendData(mNativeObject, registrationData);
        verify(mMockJniIms).sendData(mNativeObject, phoneNumberStateData);
        verify(mMockJniIms).sendData(mNativeObject, isimStateData);
        verify(mMockJniIms).sendData(mNativeObject, aosStartData);
    }

    @Test
    public void nativeStateListener_onNativeServiceReadyWhenRefreshStarted() {
        byte[] capabilityData = createBytes(IIAosService.J2N_REQUEST_CAPABILITIES_CHANGED, 1,
                IAosRegistrationListener.NetworkType.LTE.getValue(),
                IAosRegistrationListener.Capability.UT);
        byte[] registrationData = createBytes(IIAosService.J2N_REQUEST_CONTROL_REGISTRATION,
                IAosRegistration.RequestType.START.getValue(),
                IAosRegistration.Pcscf.CURRENT.getValue(),
                IAosRegistration.Cause.IMS_SERVICE.getValue());
        byte[] isimStateData = createBytes(IIAosService.J2N_NOTIFY_ISIM_STATE,
                Sim.ISIM_STATE_REFRESH_STARTED);
        byte[] aosStartData = createBytes(IIAosService.J2N_NOTIFY_AOS_START);
        mAosService.setCapabilityPairs(new CapabilityPairs(IAosRegistrationListener.NetworkType.LTE,
                IAosRegistrationListener.Capability.UT));
        when(mMockImsServiceRegistry.isImsEnabled()).thenReturn(true);
        when(mMockSimInterface.isSimLoaded()).thenReturn(false);
        when(mMockSimInterface.getIsimState()).thenReturn(Sim.ISIM_STATE_REFRESH_STARTED);
        ArgumentCaptor<NativeStateInterface.Listener> listenerCaptor =
                ArgumentCaptor.forClass(NativeStateInterface.Listener.class);
        verify(mMockNativeStateInterface).addListener(listenerCaptor.capture());

        NativeStateInterface.Listener listener = listenerCaptor.getValue();
        listener.onNativeServiceReady();

        verify(mMockJniIms, times(4)).sendData(eq(mNativeObject), any());
        verify(mMockJniIms).sendData(mNativeObject, capabilityData);
        verify(mMockJniIms).sendData(mNativeObject, registrationData);
        verify(mMockJniIms).sendData(mNativeObject, isimStateData);
        verify(mMockJniIms).sendData(mNativeObject, aosStartData);
    }

    @Test
    public void imsServiceRegistryListener_onImsOnOffChanged() {
        byte[] registrationData = createBytes(IIAosService.J2N_REQUEST_CONTROL_REGISTRATION,
                IAosRegistration.RequestType.STOP.getValue(),
                IAosRegistration.Pcscf.CURRENT.getValue(),
                IAosRegistration.Cause.IMS_SERVICE.getValue());
        when(mMockImsServiceRegistry.isImsEnabled()).thenReturn(false);

        ImsServiceRegistry.Listener listener = mAosService.getServiceRegistryListener();
        listener.onImsOnOffChanged();

        verify(mMockJniIms).sendData(mNativeObject, registrationData);
    }

    @Test
    public void jniImsListenerProxy_notifyRegistered() {
        ArraySet<String> featureTags = new ArraySet<String>();
        featureTags.add("urn%3Aurn-7%3A3gpp-service.ims.icsi.mmtel");
        mAosService.addListener(mMockAosRegistrationListener);

        Parcel parcel = Parcel.obtain();
        parcel.writeInt(IIAosService.N2J_NOTIFY_REGISTERED);
        parcel.writeInt(RegistrationType.NORMAL);
        parcel.writeInt(NetworkType.LTE.getValue());
        parcel.writeInt(FeatureTagMask.MMTEL);
        parcel.writeInt(featureTags.size()); // count
        parcel.writeString(featureTags.valueAt(0));
        parcel.setDataPosition(0);
        JniImsListener jniImsListener = mAosService.getJniImsListenerProxy();
        jniImsListener.onMessage(parcel);
        processAllMessages();

        assertEquals(NetworkType.LTE, mAosService.getRegisteredNetworkType());
        assertEquals(RegistrationState.REGISTERED, mAosService.getRegistrationState());
        verify(mMockAosRegistrationListener).notifyRegistered(RegistrationType.NORMAL,
                NetworkType.LTE, FeatureTagMask.MMTEL, featureTags);
    }

    @Test
    public void jniImsListenerProxy_notifyRegistering() {
        ArraySet<String> featureTags = new ArraySet<String>();
        featureTags.add("urn%3Aurn-7%3A3gpp-service.ims.icsi.mmtel");
        mAosService.addListener(mMockAosRegistrationListener);

        Parcel parcel = Parcel.obtain();
        parcel.writeInt(IIAosService.N2J_NOTIFY_REGISTERING);
        parcel.writeInt(RegistrationType.NORMAL);
        parcel.writeInt(NetworkType.LTE.getValue());
        parcel.writeInt(FeatureTagMask.MMTEL);
        parcel.writeInt(featureTags.size()); // count
        parcel.writeString(featureTags.valueAt(0));
        parcel.setDataPosition(0);
        JniImsListener jniImsListener = mAosService.getJniImsListenerProxy();
        jniImsListener.onMessage(parcel);
        processAllMessages();

        assertEquals(NetworkType.NONE, mAosService.getRegisteredNetworkType());
        assertEquals(RegistrationState.REGISTERING, mAosService.getRegistrationState());
        verify(mMockAosRegistrationListener).notifyRegistering(RegistrationType.NORMAL,
                NetworkType.LTE, FeatureTagMask.MMTEL, featureTags);
    }

    @Test
    public void jniImsListenerProxy_notifyDeregistered() {
        mAosService.setRegisteredNetworkType(NetworkType.LTE);
        mAosService.addListener(mMockAosRegistrationListener);

        Parcel parcel = Parcel.obtain();
        parcel.writeInt(IIAosService.N2J_NOTIFY_DEREGISTERED);
        parcel.writeInt(RegistrationType.NORMAL);
        parcel.writeInt(NetworkType.LTE.getValue());
        parcel.writeInt(ReasonCode.REGISTRATION_ERROR.getValue());
        parcel.setDataPosition(0);
        JniImsListener jniImsListener = mAosService.getJniImsListenerProxy();
        jniImsListener.onMessage(parcel);
        processAllMessages();

        assertEquals(NetworkType.NONE, mAosService.getRegisteredNetworkType());
        assertEquals(RegistrationState.DEREGISTERED, mAosService.getRegistrationState());

        verify(mMockAosRegistrationListener).notifyDeregistered(RegistrationType.NORMAL,
                NetworkType.LTE, ReasonCode.REGISTRATION_ERROR, null);
    }

    @Test
    public void jniImsListenerProxy_notifyDeregistering() {
        mAosService.setRegisteredNetworkType(NetworkType.LTE);
        mAosService.addListener(mMockAosRegistrationListener);

        Parcel parcel = Parcel.obtain();
        parcel.writeInt(IIAosService.N2J_NOTIFY_DEREGISTERING);
        parcel.writeInt(RegistrationType.NORMAL);
        parcel.setDataPosition(0);
        JniImsListener jniImsListener = mAosService.getJniImsListenerProxy();
        jniImsListener.onMessage(parcel);
        processAllMessages();

        verify(mMockAosRegistrationListener).notifyDeregistering(RegistrationType.NORMAL);
    }

    @Test
    public void jniImsListenerProxy_notifyDeregisteredWhenInvalidNetwork() {
        mAosService.setRegisteredNetworkType(NetworkType.LTE);
        mAosService.addListener(mMockAosRegistrationListener);
        when(mMockDcNetWatcher.getNetworkType()).thenReturn(TelephonyManager.NETWORK_TYPE_NR);

        Parcel parcel = Parcel.obtain();
        parcel.writeInt(IIAosService.N2J_NOTIFY_DEREGISTERED);
        parcel.writeInt(RegistrationType.NORMAL);
        parcel.writeInt(NetworkType.NONE.getValue());
        parcel.writeInt(ReasonCode.REGISTRATION_ERROR.getValue());
        parcel.setDataPosition(0);
        JniImsListener jniImsListener = mAosService.getJniImsListenerProxy();
        jniImsListener.onMessage(parcel);
        processAllMessages();

        assertEquals(NetworkType.NONE, mAosService.getRegisteredNetworkType());
        assertEquals(RegistrationState.DEREGISTERED, mAosService.getRegistrationState());

        verify(mMockAosRegistrationListener).notifyDeregistered(RegistrationType.NORMAL,
                NetworkType.NONE, ReasonCode.REGISTRATION_ERROR, null);
    }

    @Test
    public void jniImsListenerProxy_notifyDeregisteredWhenInvalidNetworkAndPlmnBlock() {
        mAosService.setRegisteredNetworkType(NetworkType.NONE);
        mAosService.addListener(mMockAosRegistrationListener);
        when(mMockDcNetWatcher.getNetworkType()).thenReturn(TelephonyManager.NETWORK_TYPE_TD_SCDMA);

        Parcel parcel = Parcel.obtain();
        parcel.writeInt(IIAosService.N2J_NOTIFY_DEREGISTERED);
        parcel.writeInt(RegistrationType.NORMAL);
        parcel.writeInt(NetworkType.NONE.getValue());
        parcel.writeInt(ReasonCode.PLMN_BLOCK.getValue());
        parcel.setDataPosition(0);
        JniImsListener jniImsListener = mAosService.getJniImsListenerProxy();
        jniImsListener.onMessage(parcel);
        processAllMessages();

        assertEquals(NetworkType.NONE, mAosService.getRegisteredNetworkType());
        assertEquals(RegistrationState.DEREGISTERED, mAosService.getRegistrationState());

        verify(mMockAosRegistrationListener).notifyDeregistered(RegistrationType.NORMAL,
                NetworkType.UTRAN, ReasonCode.PLMN_BLOCK, null);
    }

    @Test
    public void jniImsListenerProxy_notifyDeregisteredWhenInvalidNetworkAndVopsNotSupported() {
        mAosService.setRegisteredNetworkType(NetworkType.NONE);
        mAosService.addListener(mMockAosRegistrationListener);
        when(mMockDcNetWatcher.getNetworkType()).thenReturn(TelephonyManager.NETWORK_TYPE_TD_SCDMA);

        Parcel parcel = Parcel.obtain();
        parcel.writeInt(IIAosService.N2J_NOTIFY_DEREGISTERED);
        parcel.writeInt(RegistrationType.NORMAL);
        parcel.writeInt(NetworkType.NONE.getValue());
        parcel.writeInt(ReasonCode.PLMN_BLOCK_WITH_TIMEOUT_BY_VOPS_NOT_SUPPORTED.getValue());
        parcel.setDataPosition(0);
        JniImsListener jniImsListener = mAosService.getJniImsListenerProxy();
        jniImsListener.onMessage(parcel);
        processAllMessages();

        assertEquals(NetworkType.NONE, mAosService.getRegisteredNetworkType());
        assertEquals(RegistrationState.DEREGISTERED, mAosService.getRegistrationState());

        verify(mMockAosRegistrationListener).notifyDeregistered(RegistrationType.NORMAL,
                NetworkType.UTRAN, ReasonCode.PLMN_BLOCK_WITH_TIMEOUT_BY_VOPS_NOT_SUPPORTED, null);
    }

    @Test
    public void jniImsListenerProxy_notifyDeregisteredWhenInvalidNetworkAndSsacBarred() {
        mAosService.setRegisteredNetworkType(NetworkType.NONE);
        mAosService.addListener(mMockAosRegistrationListener);
        when(mMockDcNetWatcher.getNetworkType()).thenReturn(TelephonyManager.NETWORK_TYPE_TD_SCDMA);

        Parcel parcel = Parcel.obtain();
        parcel.writeInt(IIAosService.N2J_NOTIFY_DEREGISTERED);
        parcel.writeInt(RegistrationType.NORMAL);
        parcel.writeInt(NetworkType.NONE.getValue());
        parcel.writeInt(ReasonCode.PLMN_BLOCK_WITH_TIMEOUT_BY_SSAC_BARRED.getValue());
        parcel.setDataPosition(0);
        JniImsListener jniImsListener = mAosService.getJniImsListenerProxy();
        jniImsListener.onMessage(parcel);
        processAllMessages();

        assertEquals(NetworkType.NONE, mAosService.getRegisteredNetworkType());
        assertEquals(RegistrationState.DEREGISTERED, mAosService.getRegistrationState());

        verify(mMockAosRegistrationListener).notifyDeregistered(RegistrationType.NORMAL,
                NetworkType.UTRAN, ReasonCode.PLMN_BLOCK_WITH_TIMEOUT_BY_SSAC_BARRED, null);
    }

    @Test
    public void jniImsListenerProxy_notifyTechnologyChangeFailed() {
        int reason = ReasonCode.REGISTRATION_ERROR.getValue();
        mAosService.addListener(mMockAosRegistrationListener);

        Parcel parcel = Parcel.obtain();
        parcel.writeInt(IIAosService.N2J_NOTIFY_TECHNOLOGY_CHANGE_FAILED);
        parcel.writeInt(RegistrationType.NORMAL);
        parcel.writeInt(NetworkType.LTE.getValue());
        parcel.writeInt(reason);
        parcel.setDataPosition(0);
        JniImsListener jniImsListener = mAosService.getJniImsListenerProxy();
        jniImsListener.onMessage(parcel);
        processAllMessages();

        verify(mMockAosRegistrationListener).notifyTechnologyChangeFailed(RegistrationType.NORMAL,
                NetworkType.LTE, ReasonCode.REGISTRATION_ERROR, null);
    }

    @Test
    public void jniImsListenerProxy_notifyAssociatedUriChangedWithNothing() {
        mAosService.addListener(mMockAosRegistrationListener);

        Parcel parcel = Parcel.obtain();
        parcel.writeInt(IIAosService.N2J_NOTIFY_ASSOCIATED_URI_CHANGED);
        parcel.writeInt(0); // num of URI
        parcel.setDataPosition(0);
        JniImsListener jniImsListener = mAosService.getJniImsListenerProxy();
        jniImsListener.onMessage(parcel);
        processAllMessages();

        verify(mMockAosRegistrationListener).notifyAssociatedUriChanged(null);
    }

    @Test
    public void jniImsListenerProxy_notifyAssociatedUriChangedWithUri() {
        android.net.Uri[] uris = new android.net.Uri[1];
        uris[0] = android.net.Uri.parse("sip:test@ims.com");
        mAosService.addListener(mMockAosRegistrationListener);

        Parcel parcel = Parcel.obtain();
        parcel.writeInt(IIAosService.N2J_NOTIFY_ASSOCIATED_URI_CHANGED);
        parcel.writeInt(1); // num of URI
        parcel.writeString("sip:test@ims.com");
        parcel.setDataPosition(0);
        JniImsListener jniImsListener = mAosService.getJniImsListenerProxy();
        jniImsListener.onMessage(parcel);
        processAllMessages();

        verify(mMockAosRegistrationListener).notifyAssociatedUriChanged(uris);
    }

    @Test
    public void jniImsListenerProxy_notifyCapabilityUpdateFailed() {
        mAosService.addListener(mMockAosRegistrationListener);

        Parcel parcel = Parcel.obtain();
        parcel.writeInt(IIAosService.N2J_NOTIFY_CAPABILITIES_UPDATE_FAILED);
        parcel.writeInt(Capability.VOICE);
        parcel.writeInt(NetworkType.LTE.getValue());
        parcel.writeInt(CapabilityReason.ERROR_GENERIC);
        parcel.setDataPosition(0);
        JniImsListener jniImsListener = mAosService.getJniImsListenerProxy();
        jniImsListener.onMessage(parcel);
        processAllMessages();

        verify(mMockAosRegistrationListener).notifyCapabilitiesUpdateFailed(Capability.VOICE,
                NetworkType.LTE, CapabilityReason.ERROR_GENERIC);
    }

    @Test
    public void jniImsListenerProxy_notifyRegEventState() {
        mAosService.addListener(mMockAosRegistrationListener);

        Parcel parcel = Parcel.obtain();
        parcel.writeInt(IIAosService.N2J_NOTIFY_REG_EVENT_STATE);
        parcel.writeInt(200); // statusCode
        parcel.writeInt(2); // count
        parcel.writeString("sip:test1@ims.com");
        parcel.writeString("sip:test2@ims.com");
        parcel.setDataPosition(0);
        JniImsListener jniImsListener = mAosService.getJniImsListenerProxy();
        jniImsListener.onMessage(parcel);
        processAllMessages();

        Set<Uri> impus = Set.of(Uri.parse("sip:test1@ims.com"), Uri.parse("sip:test2@ims.com"));

        verify(mMockAosRegistrationListener).notifyRegEventStateChanged(200, impus);
    }

    @Test
    public void jniImsListenerProxy_notifyAosIsimState() {
        mAosService.addListener(mMockAosInfoListener);

        Parcel parcel = Parcel.obtain();
        parcel.writeInt(IIAosService.N2J_NOTIFY_AOS_ISIM_STATE);
        parcel.writeInt(IAosInfoListener.IsimState.VALID);
        parcel.setDataPosition(0);
        JniImsListener jniImsListener = mAosService.getJniImsListenerProxy();
        jniImsListener.onMessage(parcel);
        processAllMessages();

        verify(mMockAosInfoListener).notifyAosIsimStateChanged(IAosInfoListener.IsimState.VALID);
    }

    @Test
    public void jniImsListenerProxy_updateImsFeatureChanged() {
        mAosService.addListener(mMockAosRegistrationListener);

        Parcel parcel = Parcel.obtain();
        parcel.writeInt(IIAosService.N2J_NOTIFY_IMS_FEATURE_CHANGED);
        parcel.writeInt(RegistrationType.NORMAL);
        parcel.writeInt(NetworkType.LTE.getValue());
        parcel.writeInt(FeatureTagMask.MMTEL);
        parcel.setDataPosition(0);
        JniImsListener jniImsListener = mAosService.getJniImsListenerProxy();
        jniImsListener.onMessage(parcel);
        processAllMessages();

        verify(mMockAosRegistrationListener).notifyImsFeatureChanged(RegistrationType.NORMAL,
                NetworkType.LTE, FeatureTagMask.MMTEL);
    }

    @Test
    public void jniImsListenerProxy_requestPhoneNumberRetry() {
        mAosService.addListener(mMockAosRegistrationListener);

        Parcel parcel = Parcel.obtain();
        parcel.writeInt(IIAosService.N2J_REQUEST_PHONE_NUMBER_RETRY);
        parcel.writeInt(0); // command
        parcel.setDataPosition(0);
        JniImsListener jniImsListener = mAosService.getJniImsListenerProxy();
        jniImsListener.onMessage(parcel);
        processAllMessages();

        // TODO: No implementation
    }

    @Test
    public void jniImsListenerProxy_requestWifiService() {
        mAosService.addListener(mMockAosRegistrationListener);

        Parcel parcel = Parcel.obtain();
        parcel.writeInt(IIAosService.N2J_REQUEST_WIFI_SERVICE);
        parcel.writeInt(0); // command
        parcel.setDataPosition(0);
        JniImsListener jniImsListener = mAosService.getJniImsListenerProxy();
        jniImsListener.onMessage(parcel);
        processAllMessages();

        // TODO: No implementation
    }

    @Test
    public void jniImsListenerProxy_default() {
        mAosService.addListener(mMockAosRegistrationListener);
        mAosService.addListener(mMockAosInfoListener);

        Parcel parcel = Parcel.obtain();
        parcel.writeInt(IIAosService.EVENT_N2J_INFO);
        parcel.setDataPosition(0);
        JniImsListener jniImsListener = mAosService.getJniImsListenerProxy();
        jniImsListener.onMessage(parcel);
        processAllMessages();

        verifyNoMoreInteractions(mMockAosRegistrationListener);
        verifyNoMoreInteractions(mMockAosInfoListener);
    }

    @Test
    public void hashCodeInIAosRegistration() {
        CapabilityPairs oldPairs = new CapabilityPairs(NetworkType.LTE, Capability.UT);
        oldPairs.addCapability(NetworkType.NR, Capability.CALL_COMPOSER);
        CapabilityPairs newPairs = new CapabilityPairs(NetworkType.NR, Capability.CALL_COMPOSER);
        newPairs.addCapability(NetworkType.LTE, Capability.UT);

        assertTrue(oldPairs.hashCode() == newPairs.hashCode());
        assertTrue(oldPairs.equals(newPairs));
    }

    private byte[] createBytes(int... parcelData) {
        Parcel parcel = Parcel.obtain();
        for (int i = 0; i < parcelData.length; i++) {
            parcel.writeInt(parcelData[i]);
        }

        byte[] data = parcel.marshall();
        parcel.recycle();

        return data;
    }

    private byte[] createBytes(int event, boolean isOn) {
        Parcel parcel = Parcel.obtain();
        parcel.writeInt(event);
        parcel.writeBoolean(isOn);

        byte[] data = parcel.marshall();
        parcel.recycle();

        return data;
    }

    private byte[] createBytesWithLong(int event, long code) {
        Parcel parcel = Parcel.obtain();
        parcel.writeInt(event);
        parcel.writeLong(code);

        byte[] data = parcel.marshall();
        parcel.recycle();

        return data;
    }

    private byte[] createBytes(int event, int code, String reason) {
        Parcel parcel = Parcel.obtain();
        parcel.writeInt(event);
        parcel.writeInt(code);
        parcel.writeString(reason);

        byte[] data = parcel.marshall();
        parcel.recycle();

        return data;
    }

    public static class FakeAosService extends AosService {

        public FakeAosService() {
            super();
        }

        public void setRegisteredNetworkType(NetworkType networkType) {
            mRegisteredNetworkType = networkType;
        }

        public int getFeatureTagBits() {
            return mFeatureTagBits;
        }

        public void setPreciseCallState(int state) {
            mPreciseCallState = state;
        }

        public void setConnectedOverCrossSim(boolean isConnected) {
            mIsConnectedOverCrossSim = isConnected;
        }

        public CapabilityPairs getCapabilityPairs() {
            return mCapabilityPairs;
        }

        public void setCapabilityPairs(CapabilityPairs pairs) {
            mCapabilityPairs = pairs;
        }

        public Set<IAosRegistrationListener> getAosRegistrationListeners() {
            return mAosRegistrationListeners;
        }

        public Set<IAosInfoListener> getAosInfoListeners() {
            return mAosInfoListeners;
        }

        public Set<String> getFeatureTags() {
            return mFeatureTags;
        }
    }
}
