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
import static org.junit.Assert.assertNull;
import static org.junit.Assert.assertTrue;
import static org.mockito.ArgumentMatchers.any;
import static org.mockito.ArgumentMatchers.eq;
import static org.mockito.Mockito.never;
import static org.mockito.Mockito.times;
import static org.mockito.Mockito.verify;
import static org.mockito.Mockito.verifyNoMoreInteractions;
import static org.mockito.Mockito.when;

import android.os.Handler;
import android.os.Looper;
import android.os.Message;
import android.os.Parcel;
import android.testing.AndroidTestingRunner;
import android.testing.TestableLooper;
import android.util.ArraySet;
import android.util.SparseArray;

import com.android.imsstack.ImsStackTest;
import com.android.imsstack.core.agents.AgentFactory;
import com.android.imsstack.core.agents.Sim;
import com.android.imsstack.core.agents.SimInterface;
import com.android.imsstack.core.agents.dcmif.IApn;
import com.android.imsstack.enabler.IUIMS;
import com.android.imsstack.enabler.aos.IAosInfo.IsimState;
import com.android.imsstack.enabler.aos.IAosInfo.LocationInfo;
import com.android.imsstack.enabler.aos.IAosInfo.PhoneNumberState;
import com.android.imsstack.enabler.aos.IAosInfo.PreciseCallState;
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
import com.android.imsstack.enabler.aos.IIAosService;
import com.android.imsstack.internal.imsservice.ImsServiceRegistry;
import com.android.imsstack.jni.JniIms;
import com.android.imsstack.jni.JniImsListener;
import com.android.imsstack.jni.JniImsProxy;
import com.android.imsstack.system.IJNIUpCallEvt;
import com.android.imsstack.system.JNIUpCallEvtManager;

import org.junit.After;
import org.junit.Before;
import org.junit.Test;
import org.junit.runner.RunWith;
import org.mockito.Mock;
import org.mockito.MockitoAnnotations;

@RunWith(AndroidTestingRunner.class)
@TestableLooper.RunWithLooper
public class AosServiceTest extends ImsStackTest {
    private static final int SLOT_0 = 0;

    private AosService mAosService;
    private final long mNativeObject = 1000;

    @Mock IJNIUpCallEvt mMockJniUpCallEvt;
    @Mock JniIms mMockJniIms;
    @Mock JNIUpCallEvtManager mMockJniUpCallEvtManager;
    @Mock SparseArray<ImsServiceRegistry> mMockImsServiceRegistrys;
    @Mock ImsServiceRegistry mMockImsServiceRegistry;
    @Mock SimInterface mMockSimInterface;
    @Mock IAosRegistrationListener mMockAosRegistrationListener;
    @Mock IAosInfoListener mMockAosInfoListener;

    @Before
    public void setup() throws Exception {
        if (Looper.myLooper() == null) {
            Looper.prepare();
        }

        super.setUp(getClass().getSimpleName());
        MockitoAnnotations.initMocks(this);

        when(mMockJniIms.getInterface(IUIMS.AOS_SERVICE, SLOT_0)).thenReturn(mNativeObject);
        replaceInstance(JniImsProxy.class, "sJniIms", null, mMockJniIms);

        when(mMockJniUpCallEvtManager.getJNIUpCallEvt(SLOT_0)).thenReturn(mMockJniUpCallEvt);
        replaceInstance(JNIUpCallEvtManager.class, "sJNIUpCallEvtManager", null,
                mMockJniUpCallEvtManager);

        when(mMockImsServiceRegistrys.get(SLOT_0)).thenReturn(mMockImsServiceRegistry);
        replaceInstance(ImsServiceRegistry.class, "sImsServiceRegistrys", null,
                mMockImsServiceRegistrys);

        AgentFactory.getInstance().setAgent(SimInterface.class, mMockSimInterface, SLOT_0);

        mAosService = new AosService();
        mAosService.init(SLOT_0);
        mAosService.start();
    }

    @After
    public void tearDown() throws Exception {
        mAosService.stop();
        mAosService.cleanup();

        AgentFactory.getInstance().setAgent(SimInterface.class, null, SLOT_0);

        super.tearDown();
    }

    @Test
    public void initAndStart() {
        // mAosService.init(SLOT_0) and mAosService.start() are called in setup
        verify(mMockJniIms).setListener(mNativeObject, mAosService.mNativeListener);
        verify(mMockJniUpCallEvt).registerForNativeBootComplete(mAosService.mHandler,
                AosService.EVENT_NATIVE_BOOT_COMPLETED, null);
        verify(mMockImsServiceRegistry).addListener(mAosService.mListener);
        verify(mMockSimInterface).addListener(mAosService);
        verify(mMockSimInterface).addIsimListener(mAosService);
    }

    @Test
    public void stopAndCleanup() {
        Handler handler = mAosService.mHandler;

        mAosService.stop();
        verify(mMockSimInterface).removeListener(mAosService);
        verify(mMockSimInterface).removeIsimListener(mAosService);

        mAosService.cleanup();
        verify(mMockImsServiceRegistry).removeListener(mAosService.mListener);
        verify(mMockJniUpCallEvt).unregisterForNativeBootComplete(handler);
        verify(mMockJniIms).removeListener(mNativeObject);
        verify(mMockJniIms).releaseInterface(mNativeObject);
    }

    @Test
    public void addAndRemoveAosRegistrationListener() {
        mAosService.addListener(mMockAosRegistrationListener);
        assertEquals(1, mAosService.mAosRegistationListeners.size());

        mAosService.removeListener(mMockAosRegistrationListener);
        assertEquals(0, mAosService.mAosRegistationListeners.size());
    }

    @Test
    public void addAndRemoveAosInfoListener() {
        mAosService.addListener(mMockAosInfoListener);
        assertEquals(1, mAosService.mAosInfoListeners.size());

        mAosService.removeListener(mMockAosInfoListener);
        assertEquals(0, mAosService.mAosInfoListeners.size());
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
    public void changeCapabilities_nullParam() {
        mAosService.changeCapabilities(null);

        assertNull(mAosService.mCapabilityPairs);
    }

    @Test
    public void changeCapabilities_sameCapability() {
        CapabilityPairs oldPairs = new CapabilityPairs(IAosRegistrationListener.NetworkType.LTE,
                IAosRegistrationListener.Capability.UT);
        CapabilityPairs newPairs = new CapabilityPairs(IAosRegistrationListener.NetworkType.LTE,
                IAosRegistrationListener.Capability.UT);
        mAosService.mCapabilityPairs = oldPairs;

        mAosService.changeCapabilities(newPairs);

        verify(mMockJniIms, never()).sendData(eq(mNativeObject), any());
    }

    @Test
    public void changeCapabilities_lteVideo() {
        byte[] capabilityData = createBytes(IIAosService.J2N_REQUEST_CAPABILITIES_CHANGED, 2,
                IAosRegistrationListener.NetworkType.LTE, IAosRegistrationListener.Capability.VIDEO,
                IAosRegistrationListener.NetworkType.IWLAN,
                IAosRegistrationListener.Capability.VIDEO);
        CapabilityPairs pairs = new CapabilityPairs(IAosRegistrationListener.NetworkType.LTE,
                IAosRegistrationListener.Capability.VIDEO);

        mAosService.changeCapabilities(pairs);

        verify(mMockJniIms).sendData(mNativeObject, capabilityData);
        assertTrue(mAosService.mCapabilityPairs.hasCapability(
                IAosRegistrationListener.NetworkType.LTE,
                IAosRegistrationListener.Capability.VIDEO));
        assertTrue(mAosService.mCapabilityPairs.hasCapability(
                IAosRegistrationListener.NetworkType.IWLAN,
                IAosRegistrationListener.Capability.VIDEO));
    }

    @Test
    public void changeCapabilities_nrVideo() {
        byte[] capabilityData = createBytes(IIAosService.J2N_REQUEST_CAPABILITIES_CHANGED, 2,
                IAosRegistrationListener.NetworkType.NR, IAosRegistrationListener.Capability.VIDEO,
                IAosRegistrationListener.NetworkType.IWLAN,
                IAosRegistrationListener.Capability.VIDEO);
        CapabilityPairs pairs = new CapabilityPairs(IAosRegistrationListener.NetworkType.NR,
                IAosRegistrationListener.Capability.VIDEO);

        mAosService.changeCapabilities(pairs);

        verify(mMockJniIms).sendData(mNativeObject, capabilityData);
        assertTrue(mAosService.mCapabilityPairs.hasCapability(
                IAosRegistrationListener.NetworkType.NR,
                IAosRegistrationListener.Capability.VIDEO));
        assertTrue(mAosService.mCapabilityPairs.hasCapability(
                IAosRegistrationListener.NetworkType.IWLAN,
                IAosRegistrationListener.Capability.VIDEO));
    }

    @Test
    public void controlRegistration() {
        byte[] registrationData = createBytes(IIAosService.J2N_REQUEST_CONTROL_REGISTRATION,
                IAosRegistration.RequestType.START, IAosRegistration.Pcscf.CURRENT,
                IAosRegistration.Cause.IMS_SERVICE);

        mAosService.controlRegistration(IAosRegistration.RequestType.START,
                IAosRegistration.Pcscf.CURRENT, IAosRegistration.Cause.IMS_SERVICE);

        verify(mMockJniIms).sendData(mNativeObject, registrationData);
    }

    @Test
    public void getRegisteredNetworkType() {
        mAosService.mRegisteredNetworkType = NetworkType.UTRAN;

        int registeredNetworkType = mAosService.getRegisteredNetworkType();

        assertEquals(NetworkType.UTRAN, registeredNetworkType);
    }

    @Test
    public void notifyAirplaneSetting() {
        byte[] airplaneSettingData = createBytes(IIAosService.J2N_NOTIFY_AIRPLANE_SETTING, true);

        mAosService.notifyAirplaneSetting(true);

        verify(mMockJniIms).sendData(mNativeObject, airplaneSettingData);
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
                RoamingPreferredVoiceNetwork.CELLULAR);

        mAosService.notifyRoamingPreferredVoiceNetwork(RoamingPreferredVoiceNetwork.CELLULAR);

        verify(mMockJniIms).sendData(mNativeObject, preferredVoiceNetworkData);
    }

    // currently, not used
    @Test
    public void notifyServiceSetting() {
        byte[] serviceSettingData = createBytes(IIAosService.J2N_NOTIFY_SERVICE_SETTING,
                ServiceSetting.ON, IUIMS.APP_SIP_DELEGATE);

        mAosService.notifyServiceSetting(ServiceSetting.ON, IUIMS.APP_SIP_DELEGATE);

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
    public void notifyIpcanHandoverFailure() {
        byte[] ipcanHandoverFailureData = createBytes(
                IIAosService.J2N_NOTIFY_IPCAN_HANDOVER_FAILURE, IApn.IPCAN_CATEGORY_MOBILE,
                android.telephony.DataFailCause.OPERATOR_BARRED);

        mAosService.notifyIpcanHandoverFailure(IApn.IPCAN_CATEGORY_MOBILE,
                android.telephony.DataFailCause.OPERATOR_BARRED);

        verify(mMockJniIms).sendData(mNativeObject, ipcanHandoverFailureData);
    }

    @Test
    public void notifyIsimState() {
        byte[] isimStateData = createBytes(IIAosService.J2N_NOTIFY_ISIM_STATE, IsimState.LOADED);

        mAosService.notifyIsimState(IsimState.LOADED);

        verify(mMockJniIms).sendData(mNativeObject, isimStateData);
    }

    @Test
    public void notifyLocationInfo() {
        byte[] locationInfoData = createBytes(IIAosService.J2N_NOTIFY_LOCATION_INFO,
                LocationInfo.COUNTRY_CHANGED);

        mAosService.notifyLocationInfo(LocationInfo.COUNTRY_CHANGED);

        verify(mMockJniIms).sendData(mNativeObject, locationInfoData);
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
                PhoneNumberState.SIM_LOADED);

        mAosService.notifyPhoneNumberState(true, PhoneNumberState.SIM_LOADED);

        verify(mMockJniIms).sendData(mNativeObject, phoneNumberStateData);
    }

    @Test
    public void notifyPlmnChanged() {
        byte[] plmnChangedData = createBytes(IIAosService.J2N_NOTIFY_PLMN_CHANGED);

        mAosService.notifyPlmnChanged();

        verify(mMockJniIms).sendData(mNativeObject, plmnChangedData);
    }

    @Test
    public void notifyPowerOff() {
        byte[] powerOffData = createBytes(IIAosService.J2N_NOTIFY_POWER_OFF);

        mAosService.notifyPowerOff();

        verify(mMockJniIms).sendData(mNativeObject, powerOffData);
    }

    @Test
    public void notifyPreciseCallState() {
        byte[] callStateData = createBytes(IIAosService.J2N_NOTIFY_PRECISE_CALL_STATE,
                PreciseCallState.ACTIVE);

        mAosService.notifyPreciseCallState(PreciseCallState.ACTIVE);

        verify(mMockJniIms).sendData(mNativeObject, callStateData);
    }

    @Test
    public void onSimStateChanged_simLoaded() {
        byte[] simStateData = createBytes(IIAosService.J2N_NOTIFY_PHONE_NUMBER_STATE, 0,
                PhoneNumberState.SIM_LOADED);
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
                IsimState.NOT_PRESENT);
        when(mMockSimInterface.getIsimState()).thenReturn(Sim.ISIM_STATE_NOT_PRESENT);

        mAosService.onIsimStateChanged();

        verify(mMockJniIms).sendData(mNativeObject, isimStateData);
    }

    @Test
    public void onIsimStateChanged_notReady() {
        byte[] isimStateData = createBytes(IIAosService.J2N_NOTIFY_ISIM_STATE, IsimState.NOT_READY);
        when(mMockSimInterface.getIsimState()).thenReturn(Sim.ISIM_STATE_NOT_READY);

        mAosService.onIsimStateChanged();

        verify(mMockJniIms).sendData(mNativeObject, isimStateData);
    }

    @Test
    public void onIsimStateChanged_loaded() {
        byte[] isimStateData = createBytes(IIAosService.J2N_NOTIFY_ISIM_STATE, IsimState.LOADED);
        when(mMockSimInterface.getIsimState()).thenReturn(Sim.ISIM_STATE_LOADED);

        mAosService.onIsimStateChanged();

        verify(mMockJniIms).sendData(mNativeObject, isimStateData);
    }

    @Test
    public void onIsimStateChanged_refreshStarted() {
        byte[] isimStateData = createBytes(IIAosService.J2N_NOTIFY_ISIM_STATE,
                IsimState.REFRESH_STARTED);
        when(mMockSimInterface.getIsimState()).thenReturn(Sim.ISIM_STATE_REFRESH_STARTED);

        mAosService.onIsimStateChanged();

        verify(mMockJniIms).sendData(mNativeObject, isimStateData);
    }

    @Test
    public void aosServiceHandler_bootCompletedWhenSimNotPresent() {
        byte[] registrationData = createBytes(IIAosService.J2N_REQUEST_CONTROL_REGISTRATION,
                IAosRegistration.RequestType.START, IAosRegistration.Pcscf.CURRENT,
                IAosRegistration.Cause.IMS_SERVICE);
        byte[] isimStateData = createBytes(IIAosService.J2N_NOTIFY_ISIM_STATE,
                IsimState.NOT_PRESENT);
        when(mMockImsServiceRegistry.isImsEnabled()).thenReturn(true);
        when(mMockSimInterface.isSimLoaded()).thenReturn(false);
        when(mMockSimInterface.getIsimState()).thenReturn(Sim.ISIM_STATE_NOT_PRESENT);

        Message.obtain(mAosService.mHandler, AosService.EVENT_NATIVE_BOOT_COMPLETED).sendToTarget();
        processAllMessages();

        verify(mMockJniIms, times(2)).sendData(eq(mNativeObject), any());
        verify(mMockJniIms).sendData(mNativeObject, registrationData);
        verify(mMockJniIms).sendData(mNativeObject, isimStateData);
    }

    @Test
    public void aosServiceHandler_bootCompletedWhenSimNotReady() {
        byte[] registrationData = createBytes(IIAosService.J2N_REQUEST_CONTROL_REGISTRATION,
                IAosRegistration.RequestType.START, IAosRegistration.Pcscf.CURRENT,
                IAosRegistration.Cause.IMS_SERVICE);
        byte[] isimStateData = createBytes(IIAosService.J2N_NOTIFY_ISIM_STATE, IsimState.NOT_READY);
        when(mMockImsServiceRegistry.isImsEnabled()).thenReturn(true);
        when(mMockSimInterface.isSimLoaded()).thenReturn(false);
        when(mMockSimInterface.getIsimState()).thenReturn(Sim.ISIM_STATE_NOT_READY);

        Message.obtain(mAosService.mHandler, AosService.EVENT_NATIVE_BOOT_COMPLETED).sendToTarget();
        processAllMessages();

        verify(mMockJniIms, times(2)).sendData(eq(mNativeObject), any());
        verify(mMockJniIms).sendData(mNativeObject, registrationData);
        verify(mMockJniIms).sendData(mNativeObject, isimStateData);
    }

    @Test
    public void aosServiceHandler_bootCompletedWhenSimLoaded() {
        byte[] capabilityData = createBytes(IIAosService.J2N_REQUEST_CAPABILITIES_CHANGED, 1,
                IAosRegistrationListener.NetworkType.LTE, IAosRegistrationListener.Capability.UT);
        byte[] registrationData = createBytes(IIAosService.J2N_REQUEST_CONTROL_REGISTRATION,
                IAosRegistration.RequestType.START, IAosRegistration.Pcscf.CURRENT,
                IAosRegistration.Cause.IMS_SERVICE);
        byte[] phoneNumberStateData = createBytes(IIAosService.J2N_NOTIFY_PHONE_NUMBER_STATE, 0,
                PhoneNumberState.SIM_LOADED);
        byte[] isimStateData = createBytes(IIAosService.J2N_NOTIFY_ISIM_STATE, IsimState.LOADED);
        mAosService.mCapabilityPairs = new CapabilityPairs(IAosRegistrationListener.NetworkType.LTE,
                IAosRegistrationListener.Capability.UT);
        when(mMockImsServiceRegistry.isImsEnabled()).thenReturn(true);
        when(mMockSimInterface.isSimLoaded()).thenReturn(true);
        when(mMockSimInterface.getIsimState()).thenReturn(Sim.ISIM_STATE_LOADED);

        Message.obtain(mAosService.mHandler, AosService.EVENT_NATIVE_BOOT_COMPLETED).sendToTarget();
        processAllMessages();

        verify(mMockJniIms, times(4)).sendData(eq(mNativeObject), any());
        verify(mMockJniIms).sendData(mNativeObject, capabilityData);
        verify(mMockJniIms).sendData(mNativeObject, registrationData);
        verify(mMockJniIms).sendData(mNativeObject, phoneNumberStateData);
        verify(mMockJniIms).sendData(mNativeObject, isimStateData);
    }

    @Test
    public void aosServiceHandler_bootCompletedWhenRefreshStarted() {
        byte[] capabilityData = createBytes(IIAosService.J2N_REQUEST_CAPABILITIES_CHANGED, 1,
                IAosRegistrationListener.NetworkType.LTE, IAosRegistrationListener.Capability.UT);
        byte[] registrationData = createBytes(IIAosService.J2N_REQUEST_CONTROL_REGISTRATION,
                IAosRegistration.RequestType.START, IAosRegistration.Pcscf.CURRENT,
                IAosRegistration.Cause.IMS_SERVICE);
        byte[] isimStateData = createBytes(IIAosService.J2N_NOTIFY_ISIM_STATE,
                IsimState.REFRESH_STARTED);
        mAosService.mCapabilityPairs = new CapabilityPairs(IAosRegistrationListener.NetworkType.LTE,
                IAosRegistrationListener.Capability.UT);
        when(mMockImsServiceRegistry.isImsEnabled()).thenReturn(true);
        when(mMockSimInterface.isSimLoaded()).thenReturn(false);
        when(mMockSimInterface.getIsimState()).thenReturn(Sim.ISIM_STATE_REFRESH_STARTED);

        Message.obtain(mAosService.mHandler, AosService.EVENT_NATIVE_BOOT_COMPLETED).sendToTarget();
        processAllMessages();

        verify(mMockJniIms, times(3)).sendData(eq(mNativeObject), any());
        verify(mMockJniIms).sendData(mNativeObject, capabilityData);
        verify(mMockJniIms).sendData(mNativeObject, registrationData);
        verify(mMockJniIms).sendData(mNativeObject, isimStateData);
    }

    @Test
    public void imsServiceRegistryListener_onImsOnOffChanged() {
        byte[] registrationData = createBytes(IIAosService.J2N_REQUEST_CONTROL_REGISTRATION,
                IAosRegistration.RequestType.STOP, IAosRegistration.Pcscf.CURRENT,
                IAosRegistration.Cause.IMS_SERVICE);
        when(mMockImsServiceRegistry.isImsEnabled()).thenReturn(false);

        ImsServiceRegistry.Listener listener = mAosService.mListener;
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
        parcel.writeInt(NetworkType.LTE);
        parcel.writeInt(FeatureTagMask.MMTEL);
        parcel.writeInt(featureTags.size()); // count
        parcel.writeString(featureTags.valueAt(0));
        parcel.setDataPosition(0);
        JniImsListener jniImsListener = mAosService.mNativeListener;
        jniImsListener.onMessage(parcel);
        processAllMessages();

        assertEquals(NetworkType.LTE, mAosService.getRegisteredNetworkType());
        verify(mMockAosRegistrationListener).notifyRegistered(NetworkType.LTE, FeatureTagMask.MMTEL,
                featureTags);
    }

    @Test
    public void jniImsListenerProxy_notifyRegistering() {
        ArraySet<String> featureTags = new ArraySet<String>();
        featureTags.add("urn%3Aurn-7%3A3gpp-service.ims.icsi.mmtel");
        mAosService.addListener(mMockAosRegistrationListener);

        Parcel parcel = Parcel.obtain();
        parcel.writeInt(IIAosService.N2J_NOTIFY_REGISTERING);
        parcel.writeInt(NetworkType.LTE);
        parcel.writeInt(FeatureTagMask.MMTEL);
        parcel.writeInt(featureTags.size()); // count
        parcel.writeString(featureTags.valueAt(0));
        parcel.setDataPosition(0);
        JniImsListener jniImsListener = mAosService.mNativeListener;
        jniImsListener.onMessage(parcel);
        processAllMessages();

        assertEquals(NetworkType.NONE, mAosService.getRegisteredNetworkType());
        verify(mMockAosRegistrationListener).notifyRegistering(NetworkType.LTE,
                FeatureTagMask.MMTEL, featureTags);
    }

    @Test
    public void jniImsListenerProxy_notifyDeregistered() {
        mAosService.mRegisteredNetworkType = NetworkType.LTE;
        mAosService.addListener(mMockAosRegistrationListener);

        Parcel parcel = Parcel.obtain();
        parcel.writeInt(IIAosService.N2J_NOTIFY_DEREGISTERED);
        parcel.writeInt(ReasonCode.CODE_REGISTRATION_ERROR);
        parcel.setDataPosition(0);
        JniImsListener jniImsListener = mAosService.mNativeListener;
        jniImsListener.onMessage(parcel);
        processAllMessages();

        assertEquals(NetworkType.NONE, mAosService.getRegisteredNetworkType());
        verify(mMockAosRegistrationListener).notifyDeregistered(ReasonCode.CODE_REGISTRATION_ERROR);
    }

    @Test
    public void jniImsListenerProxy_notifyTechnologyChangeFailed() {
        int reason = android.telephony.DataFailCause.OPERATOR_BARRED;
        mAosService.addListener(mMockAosRegistrationListener);

        Parcel parcel = Parcel.obtain();
        parcel.writeInt(IIAosService.N2J_NOTIFY_TECHNOLOGY_CHANGE_FAILED);
        parcel.writeInt(NetworkType.LTE);
        parcel.writeInt(reason);
        parcel.setDataPosition(0);
        JniImsListener jniImsListener = mAosService.mNativeListener;
        jniImsListener.onMessage(parcel);
        processAllMessages();

        verify(mMockAosRegistrationListener).notifyTechnologyChangeFailed(NetworkType.LTE, reason);
    }

    @Test
    public void jniImsListenerProxy_notifyAssociatedUriChangedWithNothing() {
        mAosService.addListener(mMockAosRegistrationListener);

        Parcel parcel = Parcel.obtain();
        parcel.writeInt(IIAosService.N2J_NOTIFY_ASSOCIATED_URI_CHANGED);
        parcel.writeInt(0); // num of URI
        parcel.setDataPosition(0);
        JniImsListener jniImsListener = mAosService.mNativeListener;
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
        JniImsListener jniImsListener = mAosService.mNativeListener;
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
        parcel.writeInt(NetworkType.LTE);
        parcel.writeInt(CapabilityReason.ERROR_GENERIC);
        parcel.setDataPosition(0);
        JniImsListener jniImsListener = mAosService.mNativeListener;
        jniImsListener.onMessage(parcel);
        processAllMessages();

        verify(mMockAosRegistrationListener).notifyCapabilitiesUpdateFailed(Capability.VOICE,
                NetworkType.LTE, CapabilityReason.ERROR_GENERIC);
    }

    @Test
    public void jniImsListenerProxy_notifyAosIsimState() {
        mAosService.addListener(mMockAosInfoListener);

        Parcel parcel = Parcel.obtain();
        parcel.writeInt(IIAosService.N2J_NOTIFY_AOS_ISIM_STATE);
        parcel.writeInt(IAosInfoListener.IsimState.VALID);
        parcel.setDataPosition(0);
        JniImsListener jniImsListener = mAosService.mNativeListener;
        jniImsListener.onMessage(parcel);
        processAllMessages();

        verify(mMockAosInfoListener).notifyAosIsimStateChanged(IAosInfoListener.IsimState.VALID);
    }

    @Test
    public void jniImsListenerProxy_notifyRegEventState() {
        mAosService.addListener(mMockAosRegistrationListener);

        Parcel parcel = Parcel.obtain();
        parcel.writeInt(IIAosService.N2J_NOTIFY_REG_EVENT_STATE);
        parcel.writeInt(0); // state
        parcel.setDataPosition(0);
        JniImsListener jniImsListener = mAosService.mNativeListener;
        jniImsListener.onMessage(parcel);
        processAllMessages();

        // TODO: No implementation
    }

    @Test
    public void jniImsListenerProxy_requestPhoneNumberRetry() {
        mAosService.addListener(mMockAosRegistrationListener);

        Parcel parcel = Parcel.obtain();
        parcel.writeInt(IIAosService.N2J_REQUEST_PHONE_NUMBER_RETRY);
        parcel.writeInt(0); // command
        parcel.setDataPosition(0);
        JniImsListener jniImsListener = mAosService.mNativeListener;
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
        JniImsListener jniImsListener = mAosService.mNativeListener;
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
        JniImsListener jniImsListener = mAosService.mNativeListener;
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

    private byte[] createBytes(int event, int code, String reason) {
        Parcel parcel = Parcel.obtain();
        parcel.writeInt(event);
        parcel.writeInt(code);
        parcel.writeString(reason);

        byte[] data = parcel.marshall();
        parcel.recycle();

        return data;
    }
}
