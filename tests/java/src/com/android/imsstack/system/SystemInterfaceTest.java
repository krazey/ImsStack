/*
 * Copyright (C) 2023 The Android Open Source Project
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

package com.android.imsstack.system;

import static org.junit.Assert.assertEquals;
import static org.junit.Assert.assertNotEquals;
import static org.junit.Assert.assertNotNull;
import static org.junit.Assert.assertNull;
import static org.mockito.ArgumentMatchers.any;
import static org.mockito.ArgumentMatchers.anyInt;
import static org.mockito.ArgumentMatchers.anyLong;
import static org.mockito.ArgumentMatchers.anyString;
import static org.mockito.ArgumentMatchers.eq;
import static org.mockito.Mockito.doThrow;
import static org.mockito.Mockito.mock;
import static org.mockito.Mockito.never;
import static org.mockito.Mockito.times;
import static org.mockito.Mockito.verify;
import static org.mockito.Mockito.verifyNoMoreInteractions;
import static org.mockito.Mockito.when;

import android.content.Context;
import android.content.SharedPreferences;
import android.os.Looper;
import android.os.Parcel;
import android.telephony.ServiceState;
import android.telephony.TelephonyManager;
import android.telephony.ims.ImsMmTelManager;
import android.telephony.ims.ProvisioningManager;
import android.telephony.ims.stub.ImsConfigImplBase;

import androidx.test.filters.SmallTest;

import com.android.imsstack.ContextFixture;
import com.android.imsstack.base.AppContext;
import com.android.imsstack.base.MSimUtils;
import com.android.imsstack.core.agents.ImsRadioInterface;
import com.android.imsstack.core.agents.LocationInterface;
import com.android.imsstack.core.agents.Sim;
import com.android.imsstack.core.agents.WifiInterface;
import com.android.imsstack.core.agents.dcmif.EApnType;
import com.android.imsstack.core.agents.dcmif.EIpVersion;
import com.android.imsstack.core.agents.dcmif.IApn;
import com.android.imsstack.core.agents.dcmif.IDcUtils;
import com.android.imsstack.core.config.CarrierConfig;
import com.android.imsstack.internal.imsservice.ImsServiceRegistry;
import com.android.imsstack.internal.imsservice.MmTelFeatureRegistry;
import com.android.imsstack.jni.JniIms;
import com.android.imsstack.jni.JniImsProxy;
import com.android.imsstack.jni.JniObjectId;
import com.android.imsstack.jni.JniSystemListener;
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

import java.io.FileDescriptor;
import java.util.List;

@RunWith(JUnit4.class)
public class SystemInterfaceTest {
    private static class TestMessageExecutor extends MessageExecutor {
        TestMessageExecutor(Looper looper) {
            super(looper);
        }

        @Override
        public void execute(Runnable r) {
            r.run();
        }
    }

    private static final int SLOT0 = 0;
    private static final long NATIVE_OBJECT = 1L;

    @Mock private SharedPreferences mSp;
    @Mock private SharedPreferences.Editor mSpEditor;
    @Mock private WifiInterface mWifiInterface;
    @Mock private MmTelFeatureRegistry mMmTelFeatureRegistry;
    @Mock private JniIms mJniIms;
    @Mock private DefaultSystemCallInterface mDefaultSystemCall;
    @Mock private SystemCallInterface mSystemCall;

    private MessageExecutor mMessageExecutor;
    private ContextFixture mContextFixture;
    private SystemInterface mSystemInterface;

    @Before
    public void setUp() throws Exception {
        MockitoAnnotations.initMocks(this);

        mMessageExecutor = new TestMessageExecutor(Looper.getMainLooper());
        mContextFixture = new ContextFixture();
        Context context = mContextFixture.getTestDouble();
        AppContext.init(context);
        when(mJniIms.getInterface(eq(JniObjectId.SYSTEM), eq(SLOT0))).thenReturn(NATIVE_OBJECT);
        JniImsProxy.setJniIms(mJniIms);
        mSystemInterface = new SystemInterface(mMessageExecutor);
        SystemInterface.setSystemInterface(mSystemInterface);
    }

    @After
    public void tearDown() throws Exception {
        mSystemInterface.stop(SLOT0);
        mSystemInterface.cleanup();
        SystemInterface.setSystemInterface(null);
        JniImsProxy.setJniIms(null);
        AppContext.deinit();

        mSp = null;
        mSpEditor = null;
        mWifiInterface = null;
        mMmTelFeatureRegistry = null;
        mJniIms = null;
        mDefaultSystemCall = null;
        mSystemCall = null;
        mSystemInterface = null;
        mContextFixture = null;
        mMessageExecutor = null;
    }

    @Test
    @SmallTest
    public void testInit() {
        mSystemInterface.init();

        verify(mJniIms).getInterface(eq(JniObjectId.SYSTEM), eq(SLOT0));
        verify(mJniIms).setSystemListener(eq(NATIVE_OBJECT), any(JniSystemListener.class));

        // Expected: no actions
        mSystemInterface.init();

        verifyNoMoreInteractions(mJniIms);
    }

    @Test
    @SmallTest
    public void testCleanup() {
        // Expected: no actions
        mSystemInterface.cleanup();

        mSystemInterface.init();
        mSystemInterface.cleanup();

        verify(mJniIms).removeSystemListener(eq(NATIVE_OBJECT));
        verify(mJniIms).releaseInterface(eq(NATIVE_OBJECT));
    }

    @Test
    @SmallTest
    public void testStartAndStop() {
        mSystemInterface.start(SLOT0);
        ISystem system = mSystemInterface.getSystem(SLOT0);

        assertNotNull(system);

        // Expected: no actions
        mSystemInterface.start(SLOT0);

        assertEquals(system, mSystemInterface.getSystem(SLOT0));

        mSystemInterface.stop(SLOT0);

        assertNull(mSystemInterface.getSystem(SLOT0));

        // Expected: no actions
        mSystemInterface.stop(SLOT0);

        assertNull(mSystemInterface.getSystem(SLOT0));

        // When there is no ISystem for a specific slot.
        assertNull(mSystemInterface.getSystem(SLOT0 + 10));
    }

    @Test
    @SmallTest
    public void testNotifyLowBatteryStateChanged() throws Exception {
        setUpSystemInterface();
        setUpSystemWithLooper(ImsEventDef.IMS_EVENT_POWER_LOW_BATTERY);

        mSystemInterface.notifyLowBatteryStateChanged();

        Parcel data = getDataForSystem();
        try {
            assertEquals(SLOT0, data.readInt());
            assertEquals(SystemConstants.NOTIFY_EVENT, data.readInt());
            assertEquals(ImsEventDef.IMS_EVENT_POWER_LOW_BATTERY, data.readInt());
            assertEquals(ImsEventDef.IMS_POWER_LOW_CHANGED, data.readInt());
        } finally {
            data.recycle();
        }
    }

    @Test
    @SmallTest
    public void testNotifyLowBatteryState() throws Exception {
        setUpSystemInterface();
        setUpSystemWithLooper(ImsEventDef.IMS_EVENT_POWER_LOW_BATTERY);

        mSystemInterface.notifyLowBatteryState();

        Parcel data = getDataForSystem();
        try {
            assertEquals(SLOT0, data.readInt());
            assertEquals(SystemConstants.NOTIFY_EVENT, data.readInt());
            assertEquals(ImsEventDef.IMS_EVENT_POWER_LOW_BATTERY, data.readInt());
            assertEquals(ImsEventDef.IMS_POWER_LOW_BATTERY, data.readInt());
        } finally {
            data.recycle();
        }
    }

    @Test
    @SmallTest
    public void testNotifyLowBatteryStateForSlot() throws Exception {
        setUpSystemInterface();
        setUpSystemWithLooper(ImsEventDef.IMS_EVENT_POWER_LOW_BATTERY);

        mSystemInterface.notifyLowBatteryState(SLOT0);

        Parcel data = getDataForSystem();
        try {
            assertEquals(SLOT0, data.readInt());
            assertEquals(SystemConstants.NOTIFY_EVENT, data.readInt());
            assertEquals(ImsEventDef.IMS_EVENT_POWER_LOW_BATTERY, data.readInt());
            assertEquals(ImsEventDef.IMS_POWER_LOW_BATTERY, data.readInt());
        } finally {
            data.recycle();
        }
    }

    @Test
    @SmallTest
    public void testNotifyTimerExpired() throws Exception {
        setUpSystemInterface();

        long tid = 1L;
        mSystemInterface.notifyTimerExpired(tid);

        Parcel data = getDataForSystem();
        try {
            assertEquals(MSimUtils.DEFAULT_SLOT_ID, data.readInt());
            assertEquals(SystemConstants.NOTIFY_TIMER_EXPIRED, data.readInt());
            assertEquals(tid, data.readLong());
        } finally {
            data.recycle();
        }
    }

    @Test
    @SmallTest
    public void testNotifyBatteryLevelChanged() throws Exception {
        setUpSystemInterface();

        int level = 5;
        mSystemInterface.notifyBatteryLevelChanged(level);

        Parcel data = getDataForSystem();
        try {
            assertEquals(MSimUtils.DEFAULT_SLOT_ID, data.readInt());
            assertEquals(SystemConstants.NOTIFY_BATTERY_LEVEL_CHANGED, data.readInt());
            assertEquals(level, data.readInt());
        } finally {
            data.recycle();
        }
    }

    @Test
    @SmallTest
    public void testNotifyWifiStateChanged() throws Exception {
        setUpSystemInterface();

        int state = WifiInterface.STATE_ENABLED;
        mSystemInterface.notifyWifiStateChanged(state);

        Parcel data = getDataForSystem();
        try {
            assertEquals(MSimUtils.DEFAULT_SLOT_ID, data.readInt());
            assertEquals(SystemConstants.NOTIFY_WIFI_STATE_CHANGED, data.readInt());
            assertEquals(state, data.readInt());
        } finally {
            data.recycle();
        }
    }

    @Test
    @SmallTest
    public void testNotifyWifiConnectionStateChanged() throws Exception {
        setUpSystemInterface();

        int state = WifiInterface.CONNECTION_STATE_CONNECTED;
        mSystemInterface.notifyWifiConnectionStateChanged(state);

        Parcel data = getDataForSystem();
        try {
            assertEquals(MSimUtils.DEFAULT_SLOT_ID, data.readInt());
            assertEquals(SystemConstants.NOTIFY_WIFI_CONNECTION_STATE_CHANGED, data.readInt());
            assertEquals(state, data.readInt());
        } finally {
            data.recycle();
        }
    }

    @Test
    @SmallTest
    public void testNotifyAirplaneModeChanged() throws Exception {
        setUpSystemInterface();
        ISystem system = setUpSystemWithLooper();

        int airplaneMode = 1;
        system.notifyAirplaneModeChanged(airplaneMode);

        Parcel data = getDataForSystem();
        try {
            assertEquals(SLOT0, data.readInt());
            assertEquals(SystemConstants.NOTIFY_AIRPLANE_MODE_CHANGED, data.readInt());
            assertEquals(airplaneMode, data.readInt());
        } finally {
            data.recycle();
        }
    }

    @Test
    @SmallTest
    public void testNotifyDataConnectionFailed() throws Exception {
        setUpSystemInterface();
        ISystem system = setUpSystemWithLooper();

        system.notifyDataConnectionFailed(EApnType.IMS.getType());

        Parcel data = getDataForSystem();
        try {
            assertEquals(SLOT0, data.readInt());
            assertEquals(SystemConstants.NOTIFY_DATA_CONNECTION_FAILED, data.readInt());
            assertEquals(EApnType.IMS.getType(), data.readInt());
        } finally {
            data.recycle();
        }
    }

    @Test
    @SmallTest
    public void testNotifyDataConnectionIpcanChanged() throws Exception {
        setUpSystemInterface();
        ISystem system = setUpSystemWithLooper();

        system.notifyDataConnectionIpcanChanged(
                EApnType.IMS.getType(), IApn.IPCAN_CATEGORY_MOBILE);

        Parcel data = getDataForSystem();
        try {
            assertEquals(SLOT0, data.readInt());
            assertEquals(SystemConstants.NOTIFY_DATA_CONNECTION_IPCAN_CHANGED, data.readInt());
            assertEquals(EApnType.IMS.getType(), data.readInt());
            assertEquals(IApn.IPCAN_CATEGORY_MOBILE, data.readInt());
        } finally {
            data.recycle();
        }
    }

    @Test
    @SmallTest
    public void testNotifyDataConnectionStateChanged() throws Exception {
        setUpSystemInterface();
        ISystem system = setUpSystemWithLooper();

        system.notifyDataConnectionStateChanged(
                EApnType.IMS.getType(), TelephonyManager.DATA_CONNECTED);

        Parcel data = getDataForSystem();
        try {
            assertEquals(SLOT0, data.readInt());
            assertEquals(SystemConstants.NOTIFY_DATA_CONNECTION_STATE_CHANGED, data.readInt());
            assertEquals(EApnType.IMS.getType(), data.readInt());
            assertEquals(TelephonyManager.DATA_CONNECTED, data.readInt());
        } finally {
            data.recycle();
        }
    }

    @Test
    @SmallTest
    public void testNotifyNetworkTypeChanged() throws Exception {
        setUpSystemInterface();
        ISystem system = setUpSystemWithLooper();

        int networkType = 1;
        system.notifyNetworkTypeChanged(networkType);

        Parcel data = getDataForSystem();
        try {
            assertEquals(SLOT0, data.readInt());
            assertEquals(SystemConstants.NOTIFY_NETWORK_TYPE_CHANGED, data.readInt());
            assertEquals(networkType, data.readInt());
        } finally {
            data.recycle();
        }
    }

    @Test
    @SmallTest
    public void testNotifyVoiceNetworkTypeChanged() throws Exception {
        setUpSystemInterface();
        ISystem system = setUpSystemWithLooper();

        int networkType = 1;
        system.notifyVoiceNetworkTypeChanged(networkType);

        Parcel data = getDataForSystem();
        try {
            assertEquals(SLOT0, data.readInt());
            assertEquals(SystemConstants.NOTIFY_VOICE_NETWORK_TYPE_CHANGED, data.readInt());
            assertEquals(networkType, data.readInt());
        } finally {
            data.recycle();
        }
    }

    @Test
    @SmallTest
    public void testNotifyServiceStateChanged() throws Exception {
        setUpSystemInterface();
        ISystem system = setUpSystemWithLooper();

        system.notifyServiceStateChanged(ServiceState.STATE_IN_SERVICE);

        Parcel data = getDataForSystem();
        try {
            assertEquals(SLOT0, data.readInt());
            assertEquals(SystemConstants.NOTIFY_SERVICE_STATE_CHANGED, data.readInt());
            assertEquals(ServiceState.STATE_IN_SERVICE, data.readInt());
        } finally {
            data.recycle();
        }
    }

    @Test
    @SmallTest
    public void testNotifyConfigurationChanged() throws Exception {
        setUpSystemInterface();
        ISystem system = setUpSystemWithLooper();

        int configs = 0;
        system.notifyConfigurationChanged(configs);

        Parcel data = getDataForSystem();
        try {
            assertEquals(SLOT0, data.readInt());
            assertEquals(SystemConstants.NOTIFY_CONFIGURATION_CHANGED, data.readInt());
            assertEquals(configs, data.readInt());
        } finally {
            data.recycle();
        }
    }

    @Test
    @SmallTest
    public void testNotifyEvent() throws Exception {
        setUpSystemInterface();
        ISystem system = setUpSystemWithLooper();

        registerEvent(ImsEventDef.IMS_EVENT_VOLTE_SETTING);
        int event = ImsEventDef.IMS_EVENT_VOLTE_SETTING;
        int param1 = 1;
        int param2 = 200;
        system.notifyEvent(event, param1, param2);

        Parcel data = getDataForSystem();
        try {
            assertEquals(SLOT0, data.readInt());
            assertEquals(SystemConstants.NOTIFY_EVENT, data.readInt());
            assertEquals(event, data.readInt());
            assertEquals(param1, data.readInt());
            assertEquals(param2, data.readInt());
        } finally {
            data.recycle();
        }

        // sendDataForSystem will not be called.
        unregisterEvent(ImsEventDef.IMS_EVENT_VOLTE_SETTING);
        system.notifyEvent(event, param1, param2);

        // Expected: sendDataForSystem is called only once totally.
        verify(mJniIms).sendDataForSystem(anyLong(), any());
    }

    @Test
    @SmallTest
    public void testNotifyIsimState() throws Exception {
        setUpSystemInterface();
        ISystem system = setUpSystemWithLooper();

        int event = 102;
        String state = "LOADED";
        system.notifyIsimState(event, state);

        Parcel data = getDataForSystem();
        try {
            assertEquals(SLOT0, data.readInt());
            assertEquals(SystemConstants.NOTIFY_ISIM_EVENT, data.readInt());
            assertEquals(event, data.readInt());
            assertEquals(state, data.readString());
        } finally {
            data.recycle();
        }
    }

    @Test
    @SmallTest
    public void testNotifyIsimAuthenticationResponse() throws Exception {
        setUpSystemInterface();
        ISystem system = setUpSystemWithLooper();

        int event = 105;
        String response = "2wiJcwlEJWIKiBAxNH/V4c5hU1tz7uOGFzKaEJ86XNgRATMNIe3uyjb0nToA";
        long owner = 1L;
        system.notifyIsimAuthenticationResponse(event, response, owner);

        Parcel data = getDataForSystem();
        try {
            assertEquals(SLOT0, data.readInt());
            assertEquals(SystemConstants.NOTIFY_ISIM_EVENT, data.readInt());
            assertEquals(event, data.readInt());
            assertEquals(response, data.readString());
            assertEquals(owner, data.readLong());
        } finally {
            data.recycle();
        }
    }

    @Test
    @SmallTest
    public void testNotifyUsimAuthenticationResponse() throws Exception {
        setUpSystemInterface();
        ISystem system = setUpSystemWithLooper();

        int event = 106;
        String response = "2wiJcwlEJWIKiBAxNH/V4c5hU1tz7uOGFzKaEJ86XNgRATMNIe3uyjb0nToA";
        long owner = 1L;
        system.notifyUsimAuthenticationResponse(event, response, owner);

        Parcel data = getDataForSystem();
        try {
            assertEquals(SLOT0, data.readInt());
            assertEquals(SystemConstants.NOTIFY_USIM_EVENT, data.readInt());
            assertEquals(event, data.readInt());
            assertEquals(response, data.readString());
            assertEquals(owner, data.readLong());
        } finally {
            data.recycle();
        }
    }

    @Test
    @SmallTest
    public void testNotifyRadioConnectionFailed() throws Exception {
        setUpSystemInterface();
        ISystem system = setUpSystemWithLooper();

        int event = 1;
        int id = 100;
        int failureReason = 2;
        int causeCode = 3;
        int waitTimeMillis = 3000;
        system.notifyRadioConnectionFailed(event, id, failureReason, causeCode, waitTimeMillis);

        Parcel data = getDataForSystem();
        try {
            assertEquals(SLOT0, data.readInt());
            assertEquals(SystemConstants.NOTIFY_RADIO_EVENT, data.readInt());
            assertEquals(event, data.readInt());
            assertEquals(id, data.readInt());
            assertEquals(failureReason, data.readInt());
            assertEquals(causeCode, data.readInt());
            assertEquals(waitTimeMillis, data.readInt());
        } finally {
            data.recycle();
        }
    }

    @Test
    @SmallTest
    public void testNotifyRadioConnectionSetupPrepared() throws Exception {
        setUpSystemInterface();
        ISystem system = setUpSystemWithLooper();

        int event = 2;
        int id = 100;
        system.notifyRadioConnectionSetupPrepared(event, id);

        Parcel data = getDataForSystem();
        try {
            assertEquals(SLOT0, data.readInt());
            assertEquals(SystemConstants.NOTIFY_RADIO_EVENT, data.readInt());
            assertEquals(event, data.readInt());
            assertEquals(id, data.readInt());
        } finally {
            data.recycle();
        }
    }

    @Test
    @SmallTest
    public void testNotifySsacInfo() throws Exception {
        setUpSystemInterface();
        ISystem system = setUpSystemWithLooper();

        int event = 3;
        int voiceFactor = 50;
        int voiceTimeSec = 30;
        int videoFactor = 95;
        int videoTimeSec = 300;
        system.notifySsacInfo(event, voiceFactor, voiceTimeSec, videoFactor, videoTimeSec);

        Parcel data = getDataForSystem();
        try {
            assertEquals(SLOT0, data.readInt());
            assertEquals(SystemConstants.NOTIFY_RADIO_EVENT, data.readInt());
            assertEquals(event, data.readInt());
            assertEquals(voiceFactor, data.readInt());
            assertEquals(voiceTimeSec, data.readInt());
            assertEquals(videoFactor, data.readInt());
            assertEquals(videoTimeSec, data.readInt());
        } finally {
            data.recycle();
        }
    }

    @Test
    @SmallTest
    public void testOnAdvancedCallingSettingChangedWhenEnabled() throws Exception {
        setUpSystemInterface();
        setUpSystemWithLooper(ImsEventDef.IMS_EVENT_VOLTE_SETTING, mMmTelFeatureRegistry);

        when(mMmTelFeatureRegistry.isAdvancedCallingSettingEnabled()).thenReturn(true);
        MmTelFeatureRegistry.Listener listener = getMmTelFeatureRegistryListener();
        listener.onAdvancedCallingSettingChanged();

        Parcel data = getDataForSystem();
        try {
            assertEquals(SLOT0, data.readInt());
            assertEquals(SystemConstants.NOTIFY_EVENT, data.readInt());
            assertEquals(ImsEventDef.IMS_EVENT_VOLTE_SETTING, data.readInt());
            assertEquals(ImsEventDef.IMS_VOLTE_SETTING_ON, data.readInt());
        } finally {
            data.recycle();
        }
    }

    @Test
    @SmallTest
    public void testOnAdvancedCallingSettingChangedWhenDisabled() throws Exception {
        setUpSystemInterface();
        setUpSystemWithLooper(ImsEventDef.IMS_EVENT_VOLTE_SETTING, mMmTelFeatureRegistry);

        when(mMmTelFeatureRegistry.isAdvancedCallingSettingEnabled()).thenReturn(false);
        MmTelFeatureRegistry.Listener listener = getMmTelFeatureRegistryListener();
        listener.onAdvancedCallingSettingChanged();

        Parcel data = getDataForSystem();
        try {
            assertEquals(SLOT0, data.readInt());
            assertEquals(SystemConstants.NOTIFY_EVENT, data.readInt());
            assertEquals(ImsEventDef.IMS_EVENT_VOLTE_SETTING, data.readInt());
            assertEquals(ImsEventDef.IMS_VOLTE_SETTING_OFF, data.readInt());
        } finally {
            data.recycle();
        }
    }

    @Test
    @SmallTest
    public void testOnVoWiFiSettingChangedWhenEnabled() throws Exception {
        setUpSystemInterface();
        setUpSystemWithLooper(ImsEventDef.IMS_EVENT_WFC_SETTING_CHANGED, mMmTelFeatureRegistry);

        when(mMmTelFeatureRegistry.isVoWiFiSettingEnabled()).thenReturn(true);
        when(mMmTelFeatureRegistry.getVoWiFiModeSetting())
                .thenReturn(ImsMmTelManager.WIFI_MODE_WIFI_PREFERRED);
        MmTelFeatureRegistry.Listener listener = getMmTelFeatureRegistryListener();
        listener.onVoWiFiSettingChanged();

        Parcel data = getDataForSystem();
        try {
            assertEquals(SLOT0, data.readInt());
            assertEquals(SystemConstants.NOTIFY_EVENT, data.readInt());
            assertEquals(ImsEventDef.IMS_EVENT_WFC_SETTING_CHANGED, data.readInt());
            assertEquals(ImsEventDef.IMS_WFC_ON, data.readInt());
            assertEquals(ImsMmTelManager.WIFI_MODE_WIFI_PREFERRED, data.readInt());
        } finally {
            data.recycle();
        }
    }

    @Test
    @SmallTest
    public void testOnVoWiFiSettingChangedWhenDisabled() throws Exception {
        setUpSystemInterface();
        setUpSystemWithLooper(ImsEventDef.IMS_EVENT_WFC_SETTING_CHANGED, mMmTelFeatureRegistry);

        when(mMmTelFeatureRegistry.isVoWiFiSettingEnabled()).thenReturn(false);
        when(mMmTelFeatureRegistry.getVoWiFiModeSetting())
                .thenReturn(ImsMmTelManager.WIFI_MODE_WIFI_PREFERRED);
        MmTelFeatureRegistry.Listener listener = getMmTelFeatureRegistryListener();
        listener.onVoWiFiSettingChanged();

        Parcel data = getDataForSystem();
        try {
            assertEquals(SLOT0, data.readInt());
            assertEquals(SystemConstants.NOTIFY_EVENT, data.readInt());
            assertEquals(ImsEventDef.IMS_EVENT_WFC_SETTING_CHANGED, data.readInt());
            assertEquals(ImsEventDef.IMS_WFC_OFF, data.readInt());
            assertEquals(ImsMmTelManager.WIFI_MODE_WIFI_PREFERRED, data.readInt());
        } finally {
            data.recycle();
        }
    }

    @Test
    @SmallTest
    public void testOnRttModeChanged() throws Exception {
        setUpSystemInterface();
        setUpSystemWithLooper(ImsEventDef.IMS_EVENT_RTT_SETTING, mMmTelFeatureRegistry);

        int rttMode = 1;
        when(mMmTelFeatureRegistry.getRttMode()).thenReturn(rttMode);
        MmTelFeatureRegistry.Listener listener = getMmTelFeatureRegistryListener();
        listener.onRttModeChanged();

        Parcel data = getDataForSystem();
        try {
            assertEquals(SLOT0, data.readInt());
            assertEquals(SystemConstants.NOTIFY_EVENT, data.readInt());
            assertEquals(ImsEventDef.IMS_EVENT_RTT_SETTING, data.readInt());
            assertEquals(rttMode, data.readInt());
        } finally {
            data.recycle();
        }
    }

    @Test
    @SmallTest
    public void testSystemCallStartTimer() {
        setUpSystemInterface();
        long tid = 1L;
        int duration = 32000;
        Parcel data = Parcel.obtain();
        try {
            data.writeInt(MSimUtils.DEFAULT_SLOT_ID);
            data.writeInt(SystemConstants.SET_TIMER);
            data.writeInt(duration);
            data.writeLong(tid);
            data.setDataPosition(0);
            mSystemInterface.onMessage(data, null);
        } finally {
            data.recycle();
        }

        verify(mDefaultSystemCall).startTimer(eq(tid), eq(duration * 1L));
    }

    @Test
    @SmallTest
    public void testSystemCallStopTimer() {
        setUpSystemInterface();
        long tid = 1L;
        Parcel data = Parcel.obtain();
        try {
            data.writeInt(MSimUtils.DEFAULT_SLOT_ID);
            data.writeInt(SystemConstants.KILL_TIMER);
            data.writeLong(tid);
            data.setDataPosition(0);
            mSystemInterface.onMessage(data, null);
        } finally {
            data.recycle();
        }

        verify(mDefaultSystemCall).stopTimer(eq(tid));
    }

    @Test
    @SmallTest
    public void testSystemCallGetPreference() {
        setUpSystemInterface();
        String fileName = "ims";
        String key = "impi";
        Parcel data = Parcel.obtain();
        try {
            data.writeInt(MSimUtils.DEFAULT_SLOT_ID);
            data.writeInt(SystemConstants.GET_PREFERENCE);
            data.writeString(fileName);
            data.writeString(key);
            data.writeInt(SLOT0);
            data.setDataPosition(0);
            mSystemInterface.onMessage(data, null);
        } finally {
            data.recycle();
        }

        verify(mDefaultSystemCall).getPreference(eq(fileName), eq(key), eq(SLOT0));
    }

    @Test
    @SmallTest
    public void testSystemCallSetPreference() {
        setUpSystemInterface();
        String fileName = "ims";
        String key = "impi";
        String value = "1234@ims.com";
        Parcel data = Parcel.obtain();
        try {
            data.writeInt(MSimUtils.DEFAULT_SLOT_ID);
            data.writeInt(SystemConstants.SET_PREFERENCE);
            data.writeString(fileName);
            data.writeString(key);
            data.writeString(value);
            data.writeInt(SLOT0);
            data.setDataPosition(0);
            mSystemInterface.onMessage(data, null);
        } finally {
            data.recycle();
        }

        verify(mDefaultSystemCall).setPreference(eq(fileName), eq(key), eq(value), eq(SLOT0));
    }

    @Test
    @SmallTest
    public void testSystemCallGetWifiState() {
        setUpSystemInterface();
        byte[] result;
        Parcel data = Parcel.obtain();
        try {
            data.writeInt(MSimUtils.DEFAULT_SLOT_ID);
            data.writeInt(SystemConstants.GET_WIFI_STATE);
            when(mWifiInterface.isWifiEnabled()).thenReturn(true);
            data.setDataPosition(0);
            result = mSystemInterface.onMessage(data, null);

            assertNotEquals(JniImsProxy.RESULT_FAILURE, result);

            when(mWifiInterface.isWifiEnabled()).thenReturn(false);
            data.setDataPosition(0);
            result = mSystemInterface.onMessage(data, null);

            assertNotEquals(JniImsProxy.RESULT_FAILURE, result);
            verify(mWifiInterface, times(2)).isWifiEnabled();

            when(mDefaultSystemCall.getWifiInterface()).thenReturn(null);
            data.setDataPosition(0);
            result = mSystemInterface.onMessage(data, null);

            assertNotEquals(JniImsProxy.RESULT_FAILURE, result);
        } finally {
            data.recycle();
        }
    }

    @Test
    @SmallTest
    public void testSystemCallGetWifiConnectionState() {
        setUpSystemInterface();
        byte[] result;
        Parcel data = Parcel.obtain();
        try {
            data.writeInt(MSimUtils.DEFAULT_SLOT_ID);
            data.writeInt(SystemConstants.GET_WIFI_CONNECTION_STATE);
            when(mWifiInterface.isWifiConnected()).thenReturn(true);
            data.setDataPosition(0);
            result = mSystemInterface.onMessage(data, null);

            assertNotEquals(JniImsProxy.RESULT_FAILURE, result);

            when(mWifiInterface.isWifiConnected()).thenReturn(false);
            data.setDataPosition(0);
            result = mSystemInterface.onMessage(data, null);

            assertNotEquals(JniImsProxy.RESULT_FAILURE, result);
            verify(mWifiInterface, times(2)).isWifiConnected();

            when(mDefaultSystemCall.getWifiInterface()).thenReturn(null);
            data.setDataPosition(0);
            result = mSystemInterface.onMessage(data, null);

            assertNotEquals(JniImsProxy.RESULT_FAILURE, result);
        } finally {
            data.recycle();
        }
    }

    @Test
    @SmallTest
    public void testSystemCallGetBssId() {
        setUpSystemInterface();
        byte[] result;
        Parcel data = Parcel.obtain();
        try {
            data.writeInt(MSimUtils.DEFAULT_SLOT_ID);
            data.writeInt(SystemConstants.GET_WIFI_BSS_ID);
            data.setDataPosition(0);
            result = mSystemInterface.onMessage(data, null);

            assertNotEquals(JniImsProxy.RESULT_FAILURE, result);
            verify(mWifiInterface).getBssId();

            when(mDefaultSystemCall.getWifiInterface()).thenReturn(null);
            data.setDataPosition(0);
            result = mSystemInterface.onMessage(data, null);

            assertNotEquals(JniImsProxy.RESULT_FAILURE, result);
        } finally {
            data.recycle();
        }
    }

    @Test
    @SmallTest
    public void testSystemCallGetSsId() {
        setUpSystemInterface();
        byte[] result;
        Parcel data = Parcel.obtain();
        try {
            data.writeInt(MSimUtils.DEFAULT_SLOT_ID);
            data.writeInt(SystemConstants.GET_WIFI_SSID);
            data.setDataPosition(0);
            result = mSystemInterface.onMessage(data, null);

            assertNotEquals(JniImsProxy.RESULT_FAILURE, result);
            verify(mWifiInterface).getSsId();

            when(mDefaultSystemCall.getWifiInterface()).thenReturn(null);
            data.setDataPosition(0);
            result = mSystemInterface.onMessage(data, null);

            assertNotEquals(JniImsProxy.RESULT_FAILURE, result);
        } finally {
            data.recycle();
        }
    }

    @Test
    @SmallTest
    public void testSystemCallGetUuid() {
        setUpSystemInterface();
        int version = 3;
        String name = "device-name";
        Parcel data = Parcel.obtain();
        try {
            data.writeInt(MSimUtils.DEFAULT_SLOT_ID);
            data.writeInt(SystemConstants.GET_UUID);
            data.writeInt(version);
            data.writeString(name);
            data.setDataPosition(0);
            mSystemInterface.onMessage(data, null);
        } finally {
            data.recycle();
        }

        verify(mDefaultSystemCall).getUuid(eq(version), eq(name));
    }

    @Test
    @SmallTest
    public void testSystemCallGetBatteryLevel() {
        setUpSystemInterface();
        Parcel data = Parcel.obtain();
        try {
            data.writeInt(MSimUtils.DEFAULT_SLOT_ID);
            data.writeInt(SystemConstants.GET_BATTERY_LEVEL);
            data.setDataPosition(0);
            mSystemInterface.onMessage(data, null);
        } finally {
            data.recycle();
        }

        verify(mDefaultSystemCall).getBatteryLevel();
    }

    @Test
    @SmallTest
    public void testSystemCallGetDeviceName() {
        setUpSystemInterface();
        Parcel data = Parcel.obtain();
        try {
            data.writeInt(MSimUtils.DEFAULT_SLOT_ID);
            data.writeInt(SystemConstants.GET_DEVICE_NAME);
            data.setDataPosition(0);
            mSystemInterface.onMessage(data, null);
        } finally {
            data.recycle();
        }

        verify(mDefaultSystemCall).getDeviceName();
    }

    @Test
    @SmallTest
    public void testSystemCallGetExternalStoragePath() {
        setUpSystemInterface();
        Parcel data = Parcel.obtain();
        try {
            data.writeInt(MSimUtils.DEFAULT_SLOT_ID);
            data.writeInt(SystemConstants.GET_EXTERNAL_STORAGE_PATH);
            data.setDataPosition(0);
            mSystemInterface.onMessage(data, null);
        } finally {
            data.recycle();
        }

        verify(mDefaultSystemCall).getExternalStoragePath();
    }

    @Test
    @SmallTest
    public void testSystemCallGetPrivateProperty() {
        setUpSystemInterface();
        String key = "test-key";
        String value = "persistent";
        setUpSharedPreferences(null, value);

        int persistent = 1;
        byte[] result;
        Parcel data = Parcel.obtain();
        try {
            data.writeInt(MSimUtils.DEFAULT_SLOT_ID);
            data.writeInt(SystemConstants.GET_PRIVATE_PROPERTY);
            data.writeInt(persistent);
            data.writeString(key);
            data.writeInt(SLOT0);
            data.setDataPosition(0);
            result = mSystemInterface.onMessage(data, null);
        } finally {
            data.recycle();
        }

        Parcel resultData = Parcel.obtain();
        try {
            resultData.unmarshall(result, 0, result.length);
            resultData.setDataPosition(0);

            assertEquals(value, resultData.readString());
        } finally {
            resultData.recycle();
        }

        value = "ephemeral";
        when(mSp.getString(anyString(), anyString())).thenReturn(value);

        persistent = 0;
        data = Parcel.obtain();
        try {
            data.writeInt(MSimUtils.DEFAULT_SLOT_ID);
            data.writeInt(SystemConstants.GET_PRIVATE_PROPERTY);
            data.writeInt(persistent);
            data.writeString(key);
            data.writeInt(SLOT0);
            data.setDataPosition(0);
            result = mSystemInterface.onMessage(data, null);
        } finally {
            data.recycle();
        }

        resultData = Parcel.obtain();
        try {
            resultData.unmarshall(result, 0, result.length);
            resultData.setDataPosition(0);

            assertEquals(value, resultData.readString());
        } finally {
            resultData.recycle();
        }
    }

    @Test
    @SmallTest
    public void testSystemCallSetPrivateProperty() {
        setUpSystemInterface();
        setUpSharedPreferences(null, null);
        String key = "test-key1";
        String value = "persistent";
        int persistent = 1;
        Parcel data = Parcel.obtain();
        try {
            data.writeInt(MSimUtils.DEFAULT_SLOT_ID);
            data.writeInt(SystemConstants.SET_PRIVATE_PROPERTY);
            data.writeInt(persistent);
            data.writeString(key);
            data.writeString(value);
            data.writeInt(SLOT0);
            data.setDataPosition(0);
            mSystemInterface.onMessage(data, null);
        } finally {
            data.recycle();
        }

        verify(mSpEditor).putString(eq(key), eq(value));

        key = "test-key2";
        value = "ephemeral";
        persistent = 0;
        data = Parcel.obtain();
        try {
            data.writeInt(MSimUtils.DEFAULT_SLOT_ID);
            data.writeInt(SystemConstants.SET_PRIVATE_PROPERTY);
            data.writeInt(persistent);
            data.writeString(key);
            data.writeString(value);
            data.writeInt(SLOT0);
            data.setDataPosition(0);
            mSystemInterface.onMessage(data, null);
        } finally {
            data.recycle();
        }

        verify(mSpEditor).putString(eq(key), eq(value));
    }

    @Test
    @SmallTest
    public void testSystemCallSendEventForWakeLock() {
        setUpSystemInterface();
        int param1 = 0;
        int param2 = 2000;
        Parcel data = Parcel.obtain();
        try {
            data.writeInt(MSimUtils.DEFAULT_SLOT_ID);
            data.writeInt(SystemConstants.SEND_EVENT);
            data.writeInt(ImsEventDef.IMS_EVENT_WAKE_LOCK);
            data.writeInt(param1);
            data.writeInt(param2);
            data.setDataPosition(0);
            mSystemInterface.onMessage(data, null);
        } finally {
            data.recycle();
        }

        verify(mDefaultSystemCall).acquireWakeLock(eq(param2));
    }

    @Test
    @SmallTest
    public void testSystemCallSendEventForWifiService() {
        setUpSystemInterface();
        int param1 = ImsEventDef.IMS_WIFI_ON;
        int param2 = 0;
        byte[] result;
        Parcel data = Parcel.obtain();
        try {
            data.writeInt(MSimUtils.DEFAULT_SLOT_ID);
            data.writeInt(SystemConstants.SEND_EVENT);
            data.writeInt(ImsEventDef.IMS_EVENT_WIFI_SERVICE);
            data.writeInt(param1);
            data.writeInt(param2);
            data.setDataPosition(0);
            result = mSystemInterface.onMessage(data, null);

            assertNotEquals(JniImsProxy.RESULT_FAILURE, result);
            verify(mWifiInterface).requestWifiService(eq(true));

            when(mDefaultSystemCall.getWifiInterface()).thenReturn(null);
            data.setDataPosition(0);
            result = mSystemInterface.onMessage(data, null);

            assertNotEquals(JniImsProxy.RESULT_FAILURE, result);
        } finally {
            data.recycle();
        }
    }

    @Test
    @SmallTest
    public void testSystemCallSetTrafficPriority() {
        setUpSystemInterface();
        int regPriorityType = 1;
        byte[] result;
        Parcel data = Parcel.obtain();
        try {
            data.writeInt(MSimUtils.DEFAULT_SLOT_ID);
            data.writeInt(SystemConstants.SET_TRAFFIC_PRIORITY);
            data.writeInt(regPriorityType);
            data.writeInt(SLOT0);
            data.setDataPosition(0);
            mSystemInterface.onMessage(data, null);
        } finally {
            data.recycle();
        }

        verify(mDefaultSystemCall).setTrafficPriority(eq(regPriorityType), eq(SLOT0));
    }

    @Test
    @SmallTest
    public void testSendSystemEventWithException() {
        doThrow(new RuntimeException("JNI throws an exception."))
                .when(mJniIms).sendDataForSystem(anyLong(), any());

        Parcel data = Parcel.obtain();
        try {
            mSystemInterface.sendSystemEvent(data);
        } finally {
            data.recycle();
        }

        // Expected: exception will be handled internally.
        verify(mJniIms).sendDataForSystem(anyLong(), any());
    }

    @Test
    @SmallTest
    public void testSystemCallStopTimerWhenDefaultSystemCallInterfaceNull() {
        mSystemInterface.setSystemCallInterface(null);
        long tid = 1L;
        byte[] result;
        Parcel data = Parcel.obtain();
        try {
            data.writeInt(MSimUtils.DEFAULT_SLOT_ID);
            data.writeInt(SystemConstants.KILL_TIMER);
            data.writeLong(tid);
            data.setDataPosition(0);
            result = mSystemInterface.onMessage(data, null);
        } finally {
            data.recycle();
        }

        assertEquals(JniImsProxy.RESULT_FAILURE, result);
        verify(mDefaultSystemCall, never()).stopTimer(anyLong());
    }

    @Test
    @SmallTest
    public void testSystemCallStopTimerWhenExceptionThrown() {
        setUpSystemInterface();
        doThrow(new RuntimeException("stopTimer throws an exception."))
                .when(mDefaultSystemCall).stopTimer(anyLong());
        long tid = 1L;
        byte[] result;
        Parcel data = Parcel.obtain();
        try {
            data.writeInt(MSimUtils.DEFAULT_SLOT_ID);
            data.writeInt(SystemConstants.KILL_TIMER);
            data.writeLong(tid);
            data.setDataPosition(0);
            result = mSystemInterface.onMessage(data, null);
        } finally {
            data.recycle();
        }

        assertEquals(JniImsProxy.RESULT_FAILURE, result);
        verify(mDefaultSystemCall).stopTimer(anyLong());
    }

    @Test
    @SmallTest
    public void testSystemCallSendEvent() {
        setUpSystemInterface();
        setUpSystem();
        Parcel data = Parcel.obtain();
        try {
            data.writeInt(SLOT0);
            data.writeInt(SystemConstants.SEND_EVENT);
            data.writeInt(ImsEventDef.IMS_EVENT_NATIVE_BOOT_COMPLETED);
            data.writeInt(0);
            data.writeInt(0);
            data.setDataPosition(0);
            mSystemInterface.onMessage(data, null);
        } finally {
            data.recycle();
        }

        verify(mSystemCall).updateNativeServiceReady(eq(true));
    }

    @Test
    @SmallTest
    public void testSystemCallGetTtyMode() {
        setUpSystemInterface();
        setUpSystem();
        Parcel data = Parcel.obtain();
        try {
            data.writeInt(SLOT0);
            data.writeInt(SystemConstants.GET_TTY_MODE);
            data.setDataPosition(0);
            mSystemInterface.onMessage(data, null);
        } finally {
            data.recycle();
        }

        verify(mMmTelFeatureRegistry).getTtyMode();
    }

    @Test
    @SmallTest
    public void testSystemCallGetRttMode() {
        setUpSystemInterface();
        setUpSystem();
        Parcel data = Parcel.obtain();
        try {
            data.writeInt(SLOT0);
            data.writeInt(SystemConstants.GET_RTT_MODE);
            data.setDataPosition(0);
            mSystemInterface.onMessage(data, null);
        } finally {
            data.recycle();
        }

        verify(mMmTelFeatureRegistry).getRttMode();
    }

    @Test
    @SmallTest
    public void testSystemCallIsWfcEnabled() {
        setUpSystemInterface();
        setUpSystem();
        Parcel data = Parcel.obtain();
        try {
            data.writeInt(SLOT0);
            data.writeInt(SystemConstants.IS_WFC_ENABLED);
            data.setDataPosition(0);
            mSystemInterface.onMessage(data, null);
        } finally {
            data.recycle();
        }

        verify(mMmTelFeatureRegistry).isVoWiFiSettingEnabled();
    }

    @Test
    @SmallTest
    public void testSystemCallGetWfcMode() {
        setUpSystemInterface();
        setUpSystem();
        Parcel data = Parcel.obtain();
        try {
            data.writeInt(SLOT0);
            data.writeInt(SystemConstants.GET_WFC_PREFERENCES);
            data.setDataPosition(0);
            mSystemInterface.onMessage(data, null);
        } finally {
            data.recycle();
        }

        verify(mMmTelFeatureRegistry).getVoWiFiModeSetting();
    }

    @Test
    @SmallTest
    public void testSystemCallIsWfcProvisioned() {
        setUpSystemInterface();
        setUpSystem();
        byte[] result;
        Parcel data = Parcel.obtain();
        try {
            data.writeInt(SLOT0);
            data.writeInt(SystemConstants.IS_WFC_PROVISIONED);
            data.setDataPosition(0);
            result = mSystemInterface.onMessage(data, null);
        } finally {
            data.recycle();
        }

        assertNotEquals(JniImsProxy.RESULT_FAILURE, result);
    }

    @Test
    @SmallTest
    public void testSystemCallGetWfcAddressId() {
        ImsConfigImplBase originalImsConfig = ImsServiceRegistry.getInstance(SLOT0).getImsConfig();
        ImsConfigImplBase mockImsConfigImplBase = Mockito.mock(ImsConfigImplBase.class);
        ImsServiceRegistry.getInstance(SLOT0).setImsConfig(mockImsConfigImplBase);
        setUpSystemInterface();
        setUpSystem();
        String entitlementId = "123456";
        when(mockImsConfigImplBase.getConfigString(
                eq(ProvisioningManager.KEY_VOICE_OVER_WIFI_ENTITLEMENT_ID)))
                .thenReturn(entitlementId);
        byte[] result;
        Parcel data = Parcel.obtain();
        try {
            data.writeInt(SLOT0);
            data.writeInt(SystemConstants.GET_WFC_ADDRESS_ID);
            data.setDataPosition(0);
            result = mSystemInterface.onMessage(data, null);
        } finally {
            data.recycle();
        }

        Parcel resultData = Parcel.obtain();
        try {
            resultData.unmarshall(result, 0, result.length);
            resultData.setDataPosition(0);

            assertEquals(entitlementId, resultData.readString());
        } finally {
            resultData.recycle();
        }
        ImsServiceRegistry.getInstance(SLOT0).setImsConfig(originalImsConfig);
    }

    @Test
    @SmallTest
    public void testSystemCallRequestNetwork() {
        setUpSystemInterface();
        setUpSystem();
        int apnType = EApnType.IMS.getType();
        Parcel data = Parcel.obtain();
        try {
            data.writeInt(SLOT0);
            data.writeInt(SystemConstants.REQUEST_NETWORK);
            data.writeInt(apnType);
            data.setDataPosition(0);
            mSystemInterface.onMessage(data, null);
        } finally {
            data.recycle();
        }

        verify(mSystemCall).requestNetwork(eq(apnType));
    }

    @Test
    @SmallTest
    public void testSystemCallReleaseNetwork() {
        setUpSystemInterface();
        setUpSystem();
        int apnType = EApnType.IMS.getType();
        Parcel data = Parcel.obtain();
        try {
            data.writeInt(SLOT0);
            data.writeInt(SystemConstants.RELEASE_NETWORK);
            data.writeInt(apnType);
            data.setDataPosition(0);
            mSystemInterface.onMessage(data, null);
        } finally {
            data.recycle();
        }

        verify(mSystemCall).releaseNetwork(eq(apnType));
    }

    @Test
    @SmallTest
    public void testSystemCallGetAccessNetworkInfo() {
        setUpSystemInterface();
        setUpSystem();
        String[] ani = new String[] { "001", "01", "c8235", "a113", "FDD" };
        IDcUtils.AccessNetworkInfo accessNetworkInfo =
                new IDcUtils.AccessNetworkInfo(TelephonyManager.NETWORK_TYPE_LTE, ani);
        when(mSystemCall.getAccessNetworkInfo(anyInt())).thenReturn(accessNetworkInfo);
        byte[] result;
        Parcel data = Parcel.obtain();
        try {
            data.writeInt(SLOT0);
            data.writeInt(SystemConstants.GET_ACCESS_NETWORK_INFO);
            data.writeInt(accessNetworkInfo.mNetworkType);
            data.setDataPosition(0);
            result = mSystemInterface.onMessage(data, null);
        } finally {
            data.recycle();
        }

        Parcel resultData = Parcel.obtain();
        try {
            resultData.unmarshall(result, 0, result.length);
            resultData.setDataPosition(0);

            assertEquals(TelephonyManager.NETWORK_TYPE_LTE, resultData.readInt());
            assertEquals(ani.length, resultData.readInt());
            for (int i = 0; i < ani.length; ++i) {
                assertEquals(ani[i], resultData.readString());
            }
        } finally {
            resultData.recycle();
        }
    }

    @Test
    @SmallTest
    public void testSystemCallGetApnName() {
        setUpSystemInterface();
        setUpSystem();
        int apnType = EApnType.IMS.getType();
        Parcel data = Parcel.obtain();
        try {
            data.writeInt(SLOT0);
            data.writeInt(SystemConstants.GET_APN_NAME);
            data.writeInt(apnType);
            data.setDataPosition(0);
            mSystemInterface.onMessage(data, null);
        } finally {
            data.recycle();
        }

        verify(mSystemCall).getApnName(eq(apnType));
    }

    @Test
    @SmallTest
    public void testSystemCallGetDataConnectionState() {
        setUpSystemInterface();
        setUpSystem();
        int apnType = EApnType.IMS.getType();
        Parcel data = Parcel.obtain();
        try {
            data.writeInt(SLOT0);
            data.writeInt(SystemConstants.GET_DATA_CONNECTION_STATE);
            data.writeInt(apnType);
            data.setDataPosition(0);
            mSystemInterface.onMessage(data, null);
        } finally {
            data.recycle();
        }

        verify(mSystemCall).getDataConnectionState(eq(apnType));
    }

    @Test
    @SmallTest
    public void testSystemCallGetHostByNameOnCellular() {
        setUpSystemInterface();
        setUpSystem();
        int apnType = EApnType.IMS.getType();
        int ipVersion = EIpVersion.IPV6V4.getInt();
        String host = "ims.com";
        String[] ipAddrs = new String[] { "192.168.0.1" };
        when(mSystemCall.getHostByName(anyInt(), anyInt(), anyString())).thenReturn(ipAddrs);
        Parcel data = Parcel.obtain();
        try {
            data.writeInt(SLOT0);
            data.writeInt(SystemConstants.GET_HOST_BY_NAME);
            data.writeInt(apnType);
            data.writeInt(ipVersion);
            data.writeString(host);
            data.setDataPosition(0);
            mSystemInterface.onMessage(data, null);
        } finally {
            data.recycle();
        }

        verify(mSystemCall).getHostByName(eq(apnType), eq(ipVersion), eq(host));
    }

    @Test
    @SmallTest
    public void testSystemCallGetHostByNameOnWifi() {
        setUpSystemInterface();
        setUpSystem();
        int ipVersion = EIpVersion.IPV6V4.getInt();
        String host = "ims.com";
        Parcel data = Parcel.obtain();
        try {
            data.writeInt(SLOT0);
            data.writeInt(SystemConstants.GET_HOST_BY_NAME);
            data.writeInt(EApnType.WIFI.getType());
            data.writeInt(ipVersion);
            data.writeString(host);
            data.setDataPosition(0);
            mSystemInterface.onMessage(data, null);
        } finally {
            data.recycle();
        }

        verify(mWifiInterface).getHostByName(eq(ipVersion), eq(host));
    }

    @Test
    @SmallTest
    public void testSystemCallGetIfaceIdOnCellular() {
        setUpSystemInterface();
        setUpSystem();
        int apnType = EApnType.IMS.getType();
        Parcel data = Parcel.obtain();
        try {
            data.writeInt(SLOT0);
            data.writeInt(SystemConstants.GET_IFACE_ID);
            data.writeInt(apnType);
            data.setDataPosition(0);
            mSystemInterface.onMessage(data, null);
        } finally {
            data.recycle();
        }

        verify(mSystemCall).getIfaceId(eq(apnType));
    }

    @Test
    @SmallTest
    public void testSystemCallGetIfaceIdOnWifi() {
        setUpSystemInterface();
        setUpSystem();
        Parcel data = Parcel.obtain();
        try {
            data.writeInt(SLOT0);
            data.writeInt(SystemConstants.GET_IFACE_ID);
            data.writeInt(EApnType.WIFI.getType());
            data.setDataPosition(0);
            mSystemInterface.onMessage(data, null);
        } finally {
            data.recycle();
        }

        verify(mWifiInterface).getIfaceId();
    }

    @Test
    @SmallTest
    public void testSystemCallGetIfaceNameOnCellular() {
        setUpSystemInterface();
        setUpSystem();
        int apnType = EApnType.IMS.getType();
        Parcel data = Parcel.obtain();
        try {
            data.writeInt(SLOT0);
            data.writeInt(SystemConstants.GET_IFACE_NAME);
            data.writeInt(apnType);
            data.setDataPosition(0);
            mSystemInterface.onMessage(data, null);
        } finally {
            data.recycle();
        }

        verify(mSystemCall).getIfaceName(eq(apnType));
    }

    @Test
    @SmallTest
    public void testSystemCallGetIfaceNameOnWifi() {
        setUpSystemInterface();
        setUpSystem();
        Parcel data = Parcel.obtain();
        try {
            data.writeInt(SLOT0);
            data.writeInt(SystemConstants.GET_IFACE_NAME);
            data.writeInt(EApnType.WIFI.getType());
            data.setDataPosition(0);
            mSystemInterface.onMessage(data, null);
        } finally {
            data.recycle();
        }

        verify(mWifiInterface).getIfaceName();
    }

    @Test
    @SmallTest
    public void testSystemCallGetLastAccessNetworkInfo() {
        setUpSystemInterface();
        setUpSystem();
        int networkType = TelephonyManager.NETWORK_TYPE_LTE;
        String[] lani = new String[] { String.valueOf(networkType),
                "2023-06-15T08:07:49Z", "9", "001", "01", "c8235", "a113", "FDD" };
        when(mSystemCall.getLastAccessNetworkInfo(anyInt())).thenReturn(lani);
        Parcel data = Parcel.obtain();
        try {
            data.writeInt(SLOT0);
            data.writeInt(SystemConstants.GET_LAST_ACCESS_NETWORK_INFO);
            data.writeInt(networkType);
            data.setDataPosition(0);
            mSystemInterface.onMessage(data, null);
        } finally {
            data.recycle();
        }

        verify(mSystemCall).getLastAccessNetworkInfo(eq(networkType));
    }

    @Test
    @SmallTest
    public void testSystemCallGetLocalAddressOnCellular() {
        setUpSystemInterface();
        setUpSystem();
        int apnType = EApnType.IMS.getType();
        int ipVersion = EIpVersion.IPV6V4.getInt();
        Parcel data = Parcel.obtain();
        try {
            data.writeInt(SLOT0);
            data.writeInt(SystemConstants.GET_LOCAL_ADDRESS);
            data.writeInt(apnType);
            data.writeInt(ipVersion);
            data.setDataPosition(0);
            mSystemInterface.onMessage(data, null);
        } finally {
            data.recycle();
        }

        verify(mSystemCall).getLocalAddress(eq(apnType), eq(ipVersion));
    }

    @Test
    @SmallTest
    public void testSystemCallGetLocalAddressOnWifi() {
        setUpSystemInterface();
        setUpSystem();
        int ipVersion = EIpVersion.IPV6V4.getInt();
        Parcel data = Parcel.obtain();
        try {
            data.writeInt(SLOT0);
            data.writeInt(SystemConstants.GET_LOCAL_ADDRESS);
            data.writeInt(EApnType.WIFI.getType());
            data.writeInt(ipVersion);
            data.setDataPosition(0);
            mSystemInterface.onMessage(data, null);
        } finally {
            data.recycle();
        }

        verify(mWifiInterface).getLocalAddress(eq(ipVersion));
    }

    @Test
    @SmallTest
    public void testSystemCallGetIpcanCategoryOnCellular() {
        setUpSystemInterface();
        setUpSystem();
        int apnType = EApnType.IMS.getType();
        Parcel data = Parcel.obtain();
        try {
            data.writeInt(SLOT0);
            data.writeInt(SystemConstants.GET_IPCAN_CATEGORY);
            data.writeInt(apnType);
            data.setDataPosition(0);
            mSystemInterface.onMessage(data, null);
        } finally {
            data.recycle();
        }

        verify(mSystemCall).getIpcanCategory(eq(apnType));
    }

    @Test
    @SmallTest
    public void testSystemCallGetIpcanCategoryOnWifi() {
        setUpSystemInterface();
        setUpSystem();
        byte[] result;
        Parcel data = Parcel.obtain();
        try {
            data.writeInt(SLOT0);
            data.writeInt(SystemConstants.GET_IPCAN_CATEGORY);
            data.writeInt(EApnType.WIFI.getType());
            data.setDataPosition(0);
            result = mSystemInterface.onMessage(data, null);
        } finally {
            data.recycle();
        }

        Parcel resultData = Parcel.obtain();
        try {
            resultData.unmarshall(result, 0, result.length);
            resultData.setDataPosition(0);

            assertEquals(IApn.IPCAN_CATEGORY_WLAN, resultData.readInt());
        } finally {
            resultData.recycle();
        }
    }

    @Test
    @SmallTest
    public void testSystemCallGetPcscfAddresses() {
        setUpSystemInterface();
        setUpSystem();
        int apnType = EApnType.IMS.getType();
        int ipVersion = EIpVersion.IPV6V4.getInt();
        String[] pcscfAddresses = new String[] { "192.168.0.1" };
        when(mSystemCall.getPcscfAddresses(anyInt(), anyInt())).thenReturn(pcscfAddresses);
        Parcel data = Parcel.obtain();
        try {
            data.writeInt(SLOT0);
            data.writeInt(SystemConstants.GET_PCSCF_ADDRESSES);
            data.writeInt(apnType);
            data.writeInt(ipVersion);
            data.setDataPosition(0);
            mSystemInterface.onMessage(data, null);
        } finally {
            data.recycle();
        }

        verify(mSystemCall).getPcscfAddresses(eq(apnType), eq(ipVersion));
    }

    @Test
    @SmallTest
    public void testSystemCallGetRoamingState() {
        setUpSystemInterface();
        setUpSystem();
        Parcel data = Parcel.obtain();
        try {
            data.writeInt(SLOT0);
            data.writeInt(SystemConstants.GET_ROAMING_STATE);
            data.setDataPosition(0);
            mSystemInterface.onMessage(data, null);
        } finally {
            data.recycle();
        }

        verify(mSystemCall).isNetworkRoaming();
    }

    @Test
    @SmallTest
    public void testSystemCallGetServiceState() {
        setUpSystemInterface();
        setUpSystem();
        Parcel data = Parcel.obtain();
        try {
            data.writeInt(SLOT0);
            data.writeInt(SystemConstants.GET_SERVICE_STATE);
            data.setDataPosition(0);
            mSystemInterface.onMessage(data, null);
        } finally {
            data.recycle();
        }

        verify(mSystemCall).getDataServiceState();
    }

    @Test
    @SmallTest
    public void testSystemCallIsLteEmergencyOnly() {
        setUpSystemInterface();
        setUpSystem();
        Parcel data = Parcel.obtain();
        try {
            data.writeInt(SLOT0);
            data.writeInt(SystemConstants.IS_LTE_EMERGENCY_ONLY);
            data.setDataPosition(0);
            mSystemInterface.onMessage(data, null);
        } finally {
            data.recycle();
        }

        verify(mSystemCall).isLteEmergencyOnly();
    }

    @Test
    @SmallTest
    public void testSystemCallIsMobileDataEnabled() {
        setUpSystemInterface();
        setUpSystem();
        Parcel data = Parcel.obtain();
        try {
            data.writeInt(SLOT0);
            data.writeInt(SystemConstants.IS_MOBILE_DATA_ENABLED);
            data.setDataPosition(0);
            mSystemInterface.onMessage(data, null);
        } finally {
            data.recycle();
        }

        verify(mSystemCall).isMobileDataEnabled();
    }

    @Test
    @SmallTest
    public void testSystemCallGetMocnPlmnInfo() {
        setUpSystemInterface();
        setUpSystem();
        Parcel data = Parcel.obtain();
        try {
            data.writeInt(SLOT0);
            data.writeInt(SystemConstants.GET_MOCN_PLMN_INFO);
            data.setDataPosition(0);
            mSystemInterface.onMessage(data, null);
        } finally {
            data.recycle();
        }

        verify(mSystemCall).getMocnPlmnInfo();
    }

    @Test
    @SmallTest
    public void testSystemCallGetVoiceServiceState() {
        setUpSystemInterface();
        setUpSystem();
        Parcel data = Parcel.obtain();
        try {
            data.writeInt(SLOT0);
            data.writeInt(SystemConstants.GET_VOICE_SERVICE_STATE);
            data.setDataPosition(0);
            mSystemInterface.onMessage(data, null);
        } finally {
            data.recycle();
        }

        verify(mSystemCall).getVoiceServiceState();
    }

    @Test
    @SmallTest
    public void testSystemCallGetVoiceRoamingType() {
        setUpSystemInterface();
        setUpSystem();
        Parcel data = Parcel.obtain();
        try {
            data.writeInt(SLOT0);
            data.writeInt(SystemConstants.GET_VOICE_ROAMING_TYPE);
            data.setDataPosition(0);
            mSystemInterface.onMessage(data, null);
        } finally {
            data.recycle();
        }

        verify(mSystemCall).getVoiceRoamingType();
    }

    @Test
    @SmallTest
    public void testSystemCallGetDataRoamingType() {
        setUpSystemInterface();
        setUpSystem();
        Parcel data = Parcel.obtain();
        try {
            data.writeInt(SLOT0);
            data.writeInt(SystemConstants.GET_DATA_ROAMING_TYPE);
            data.setDataPosition(0);
            mSystemInterface.onMessage(data, null);
        } finally {
            data.recycle();
        }

        verify(mSystemCall).getDataRoamingType();
    }

    @Test
    @SmallTest
    public void testSystemCallGetMtuOnCellular() {
        setUpSystemInterface();
        setUpSystem();
        int apnType = EApnType.IMS.getType();
        Parcel data = Parcel.obtain();
        try {
            data.writeInt(SLOT0);
            data.writeInt(SystemConstants.GET_MTU);
            data.writeInt(apnType);
            data.setDataPosition(0);
            mSystemInterface.onMessage(data, null);
        } finally {
            data.recycle();
        }

        verify(mSystemCall).getMtu(eq(apnType));
    }

    @Test
    @SmallTest
    public void testSystemCallGetMtuOnWifi() {
        setUpSystemInterface();
        setUpSystem();
        Parcel data = Parcel.obtain();
        try {
            data.writeInt(SLOT0);
            data.writeInt(SystemConstants.GET_MTU);
            data.writeInt(EApnType.WIFI.getType());
            data.setDataPosition(0);
            mSystemInterface.onMessage(data, null);
        } finally {
            data.recycle();
        }

        verify(mWifiInterface).getMtu();
    }

    @Test
    @SmallTest
    public void testSystemCallIsEmergencyAttachSupported() {
        setUpSystemInterface();
        setUpSystem();
        Parcel data = Parcel.obtain();
        try {
            data.writeInt(SLOT0);
            data.writeInt(SystemConstants.IS_EMERGENCY_ATTACH_SUPPORTED);
            data.setDataPosition(0);
            mSystemInterface.onMessage(data, null);
        } finally {
            data.recycle();
        }

        verify(mSystemCall).isEmergencyAttachSupported();
    }

    @Test
    @SmallTest
    public void testSystemCallBindSocketOnCellular() {
        setUpSystemInterface();
        setUpSystem();
        FileDescriptor fd = new FileDescriptor();
        int apnType = EApnType.IMS.getType();
        Parcel data = Parcel.obtain();
        try {
            data.writeInt(SLOT0);
            data.writeInt(SystemConstants.BIND_SOCKET);
            data.writeInt(apnType);
            data.setDataPosition(0);
            mSystemInterface.onMessage(data, fd);
        } finally {
            data.recycle();
        }

        verify(mSystemCall).bindSocket(eq(apnType), eq(fd));
    }

    @Test
    @SmallTest
    public void testSystemCallBindSocketOnWifi() {
        setUpSystemInterface();
        setUpSystem();
        FileDescriptor fd = new FileDescriptor();
        Parcel data = Parcel.obtain();
        try {
            data.writeInt(SLOT0);
            data.writeInt(SystemConstants.BIND_SOCKET);
            data.writeInt(EApnType.WIFI.getType());
            data.setDataPosition(0);
            mSystemInterface.onMessage(data, fd);
        } finally {
            data.recycle();
        }

        verify(mWifiInterface).bindSocket(eq(fd));
    }

    @Test
    @SmallTest
    public void testSystemCallIsIpv6Preferred() {
        setUpSystemInterface();
        setUpSystem();
        int apnType = EApnType.IMS.getType();
        Parcel data = Parcel.obtain();
        try {
            data.writeInt(SLOT0);
            data.writeInt(SystemConstants.IS_IPV6_PREFERRED);
            data.writeInt(apnType);
            data.setDataPosition(0);
            mSystemInterface.onMessage(data, null);
        } finally {
            data.recycle();
        }

        verify(mSystemCall).isIpv6Preferred(eq(apnType));
    }

    @Test
    @SmallTest
    public void testSystemCallGetIsimState() {
        setUpSystemInterface();
        setUpSystem();
        Parcel data = Parcel.obtain();
        try {
            data.writeInt(SLOT0);
            data.writeInt(SystemConstants.GET_ISIM_STATE);
            data.setDataPosition(0);
            mSystemInterface.onMessage(data, null);
        } finally {
            data.recycle();
        }

        verify(mSystemCall).getIsimState();
    }

    @Test
    @SmallTest
    public void testSystemCallGetIsimRecord() {
        setUpSystemInterface();
        setUpSystem();
        int fileId = Sim.ISIM_FILE_ID_IMPU;
        Parcel data = Parcel.obtain();
        try {
            data.writeInt(SLOT0);
            data.writeInt(SystemConstants.GET_ISIM_RECORD);
            data.writeInt(fileId);
            data.setDataPosition(0);
            mSystemInterface.onMessage(data, null);
        } finally {
            data.recycle();
        }

        verify(mSystemCall).getIsimRecord(eq(fileId));
    }

    @Test
    @SmallTest
    public void testSystemCallRequestIsimAuthentication() {
        setUpSystemInterface();
        setUpSystem();
        String nonce = "ECu9VdWOHm/rohnOqCzsBqMQhEGy6E9CAACOw6M59Ouwjg==";
        long owner = 1L;
        Parcel data = Parcel.obtain();
        try {
            data.writeInt(SLOT0);
            data.writeInt(SystemConstants.REQUEST_ISIM_AUTH);
            data.writeString(nonce);
            data.writeLong(owner);
            data.setDataPosition(0);
            mSystemInterface.onMessage(data, null);
        } finally {
            data.recycle();
        }

        verify(mSystemCall).requestIsimAuthentication(eq(nonce), eq(owner));
    }

    @Test
    @SmallTest
    public void testSystemCallRequestUsimAuthentication() {
        setUpSystemInterface();
        setUpSystem();
        String nonce = "ECu9VdWOHm/rohnOqCzsBqMQhEGy6E9CAACOw6M59Ouwjg==";
        long owner = 1L;
        Parcel data = Parcel.obtain();
        try {
            data.writeInt(SLOT0);
            data.writeInt(SystemConstants.REQUEST_USIM_AUTH);
            data.writeString(nonce);
            data.writeLong(owner);
            data.setDataPosition(0);
            mSystemInterface.onMessage(data, null);
        } finally {
            data.recycle();
        }

        verify(mSystemCall).requestUsimAuthentication(eq(nonce), eq(owner));
    }

    @Test
    @SmallTest
    public void testSystemCallGetDeviceId() {
        setUpSystemInterface();
        setUpSystem();
        Parcel data = Parcel.obtain();
        try {
            data.writeInt(SLOT0);
            data.writeInt(SystemConstants.GET_DEVICE_ID);
            data.setDataPosition(0);
            mSystemInterface.onMessage(data, null);
        } finally {
            data.recycle();
        }

        verify(mSystemCall).getImei();
    }

    @Test
    @SmallTest
    public void testSystemCallGetDeviceSoftwareVersion() {
        setUpSystemInterface();
        setUpSystem();
        Parcel data = Parcel.obtain();
        try {
            data.writeInt(SLOT0);
            data.writeInt(SystemConstants.GET_DEVICE_SOFTWARE_VERSION);
            data.setDataPosition(0);
            mSystemInterface.onMessage(data, null);
        } finally {
            data.recycle();
        }

        verify(mSystemCall).getDeviceSoftwareVersion();
    }

    @Test
    @SmallTest
    public void testSystemCallGetPhoneNumber() {
        setUpSystemInterface();
        setUpSystem();
        Parcel data = Parcel.obtain();
        try {
            data.writeInt(SLOT0);
            data.writeInt(SystemConstants.GET_PHONE_NUMBER);
            data.setDataPosition(0);
            mSystemInterface.onMessage(data, null);
        } finally {
            data.recycle();
        }

        verify(mSystemCall).getPhoneNumber();
    }

    @Test
    @SmallTest
    public void testSystemCallGetSubscriberId() {
        setUpSystemInterface();
        setUpSystem();
        Parcel data = Parcel.obtain();
        try {
            data.writeInt(SLOT0);
            data.writeInt(SystemConstants.GET_SUBSCRIBER_ID);
            data.setDataPosition(0);
            mSystemInterface.onMessage(data, null);
        } finally {
            data.recycle();
        }

        verify(mSystemCall).getSubscriberId();
    }

    @Test
    @SmallTest
    public void testSystemCallGetSimMcc() {
        setUpSystemInterface();
        setUpSystem();
        Parcel data = Parcel.obtain();
        try {
            data.writeInt(SLOT0);
            data.writeInt(SystemConstants.GET_SIM_MCC);
            data.setDataPosition(0);
            mSystemInterface.onMessage(data, null);
        } finally {
            data.recycle();
        }

        verify(mSystemCall).getSimMcc();
    }

    @Test
    @SmallTest
    public void testSystemCallGetSimMnc() {
        setUpSystemInterface();
        setUpSystem();
        Parcel data = Parcel.obtain();
        try {
            data.writeInt(SLOT0);
            data.writeInt(SystemConstants.GET_SIM_MNC);
            data.setDataPosition(0);
            mSystemInterface.onMessage(data, null);
        } finally {
            data.recycle();
        }

        verify(mSystemCall).getSimMnc();
    }

    @Test
    @SmallTest
    public void testSystemCallGetSimCountryIso() {
        setUpSystemInterface();
        setUpSystem();
        Parcel data = Parcel.obtain();
        try {
            data.writeInt(SLOT0);
            data.writeInt(SystemConstants.GET_SIM_COUNTRY_ISO);
            data.setDataPosition(0);
            mSystemInterface.onMessage(data, null);
        } finally {
            data.recycle();
        }

        verify(mSystemCall).getSimCountryIso();
    }

    @Test
    @SmallTest
    public void testSystemCallGetNetworkCountryIso() {
        setUpSystemInterface();
        setUpSystem();
        Parcel data = Parcel.obtain();
        try {
            data.writeInt(SLOT0);
            data.writeInt(SystemConstants.GET_NETWORK_COUNTRY_ISO);
            data.setDataPosition(0);
            mSystemInterface.onMessage(data, null);
        } finally {
            data.recycle();
        }

        verify(mSystemCall).getNetworkCountryIso();
    }

    @Test
    @SmallTest
    public void testSystemCallGetNetworkType() {
        setUpSystemInterface();
        setUpSystem();
        Parcel data = Parcel.obtain();
        try {
            data.writeInt(SLOT0);
            data.writeInt(SystemConstants.GET_NETWORK_TYPE);
            data.setDataPosition(0);
            mSystemInterface.onMessage(data, null);
        } finally {
            data.recycle();
        }

        verify(mSystemCall).getNetworkType();
    }

    @Test
    @SmallTest
    public void testSystemCallGetVoiceNetworkType() {
        setUpSystemInterface();
        setUpSystem();
        Parcel data = Parcel.obtain();
        try {
            data.writeInt(SLOT0);
            data.writeInt(SystemConstants.GET_VOICE_NETWORK_TYPE);
            data.setDataPosition(0);
            mSystemInterface.onMessage(data, null);
        } finally {
            data.recycle();
        }

        verify(mSystemCall).getVoiceNetworkType();
    }

    @Test
    @SmallTest
    public void testSystemCallGetCsCallState() {
        setUpSystemInterface();
        setUpSystem();
        Parcel data = Parcel.obtain();
        try {
            data.writeInt(SLOT0);
            data.writeInt(SystemConstants.GET_CS_CALL_STATE);
            data.setDataPosition(0);
            mSystemInterface.onMessage(data, null);
        } finally {
            data.recycle();
        }

        verify(mSystemCall).getCsCallState();
    }

    @Test
    @SmallTest
    public void testSystemCallGetCsCallStateInOtherSlot() {
        setUpSystemInterface();
        setUpSystem();
        Parcel data = Parcel.obtain();
        try {
            data.writeInt(SLOT0);
            data.writeInt(SystemConstants.GET_CS_CALL_STATE_IN_OTHER_SLOT);
            data.setDataPosition(0);
            mSystemInterface.onMessage(data, null);
        } finally {
            data.recycle();
        }

        verify(mSystemCall).getCsCallStateInOtherSlot();
    }

    @Test
    @SmallTest
    public void testSystemCallIsEmergencyNumber() {
        setUpSystemInterface();
        setUpSystem();
        String phoneNumber = "911";
        Parcel data = Parcel.obtain();
        try {
            data.writeInt(SLOT0);
            data.writeInt(SystemConstants.IS_EMERGENCY_NUMBER);
            data.writeString(phoneNumber);
            data.setDataPosition(0);
            mSystemInterface.onMessage(data, null);
        } finally {
            data.recycle();
        }

        verify(mSystemCall).isEmergencyNumber(eq(phoneNumber));
    }

    @Test
    @SmallTest
    public void testSystemCallStartListeningForLocation() {
        setUpSystemInterface();
        setUpSystem();
        int locationUpdateSec = 2000;
        Parcel data = Parcel.obtain();
        try {
            data.writeInt(SLOT0);
            data.writeInt(SystemConstants.START_LISTENING_FOR_LOCATION);
            data.writeInt(locationUpdateSec);
            data.setDataPosition(0);
            mSystemInterface.onMessage(data, null);
        } finally {
            data.recycle();
        }

        verify(mSystemCall).startListeningForLocation(eq(locationUpdateSec));
    }

    @Test
    @SmallTest
    public void testSystemCallStopListeningForLocation() {
        setUpSystemInterface();
        setUpSystem();
        Parcel data = Parcel.obtain();
        try {
            data.writeInt(SLOT0);
            data.writeInt(SystemConstants.STOP_LISTENING_FOR_LOCATION);
            data.setDataPosition(0);
            mSystemInterface.onMessage(data, null);
        } finally {
            data.recycle();
        }

        verify(mSystemCall).stopListeningForLocation();
    }

    @Test
    @SmallTest
    public void testSystemCallGetLastKnownLocation() {
        setUpSystemInterface();
        setUpSystem();
        String[] locationInfo = new String[] { "37.3361542", "127.0848087", "18.759", "Ellipsoid",
                "68", "2023-06-15T08:08:41Z", "DBH", "kr", null, null, null, "162.5", "14.550725" };
        when(mSystemCall.getLastKnownLocation(anyInt())).thenReturn(locationInfo);
        Parcel data = Parcel.obtain();
        try {
            data.writeInt(SLOT0);
            data.writeInt(SystemConstants.GET_LAST_KNOWN_LOCATION);
            data.writeInt(LocationInterface.LOCATION_CATEGORY_ALL);
            data.setDataPosition(0);
            mSystemInterface.onMessage(data, null);
        } finally {
            data.recycle();
        }

        verify(mSystemCall).getLastKnownLocation(eq(LocationInterface.LOCATION_CATEGORY_ALL));
    }

    @Test
    @SmallTest
    public void testSystemCallStartInstantLocationUpdate() {
        setUpSystemInterface();
        setUpSystem();
        Parcel data = Parcel.obtain();
        try {
            data.writeInt(SLOT0);
            data.writeInt(SystemConstants.START_INSTANT_LOCATION_UPDATE);
            data.setDataPosition(0);
            mSystemInterface.onMessage(data, null);
        } finally {
            data.recycle();
        }

        verify(mSystemCall).startInstantLocationUpdate();
    }

    @Test
    @SmallTest
    public void testSystemCallStartImsTraffic() {
        setUpSystemInterface();
        setUpSystem();
        int id = 1;
        int trafficType = ImsRadioInterface.TRAFFIC_TYPE_VOICE;
        int accessNetworkType = ImsRadioInterface.ACCESS_NETWORK_TYPE_EUTRAN;
        int direction = ImsRadioInterface.DIRECTION_MO;
        Parcel data = Parcel.obtain();
        try {
            data.writeInt(SLOT0);
            data.writeInt(SystemConstants.START_IMS_TRAFFIC);
            data.writeInt(id);
            data.writeInt(trafficType);
            data.writeInt(accessNetworkType);
            data.writeInt(direction);
            data.setDataPosition(0);
            mSystemInterface.onMessage(data, null);
        } finally {
            data.recycle();
        }

        verify(mSystemCall).startImsTraffic(eq(id),
                eq(trafficType), eq(accessNetworkType), eq(direction));
    }

    @Test
    @SmallTest
    public void testSystemCallStopImsTraffic() {
        setUpSystemInterface();
        setUpSystem();
        int id = 1;
        Parcel data = Parcel.obtain();
        try {
            data.writeInt(SLOT0);
            data.writeInt(SystemConstants.STOP_IMS_TRAFFIC);
            data.writeInt(id);
            data.setDataPosition(0);
            mSystemInterface.onMessage(data, null);
        } finally {
            data.recycle();
        }

        verify(mSystemCall).stopImsTraffic(eq(id));
    }

    @Test
    @SmallTest
    public void testSystemCallTriggerEpsFallback() {
        setUpSystemInterface();
        setUpSystem();
        int reason = 1;
        Parcel data = Parcel.obtain();
        try {
            data.writeInt(SLOT0);
            data.writeInt(SystemConstants.TRIGGER_EPS_FALLBACK);
            data.writeInt(reason);
            data.setDataPosition(0);
            mSystemInterface.onMessage(data, null);
        } finally {
            data.recycle();
        }

        verify(mSystemCall).triggerEpsFallback(eq(reason));
    }

    @Test
    @SmallTest
    public void testSystemCallAddIpSecSaParameter() {
        setUpSystemInterface();
        setUpSystem();
        int id = 12;
        int spi = 1;
        byte[] ck = {0x62, 0x36, 0x62, 0x6c, 0x07, 0x77, 0x78};
        byte[] ik = {0x6d, 0x51, 0x27, 0x6a, 0x51, 0x31, 0x74};
        IpSecSaPolicy mSaPolicy = new IpSecSaPolicy(spi,
                IpSecSaPolicy.DIRECTION_OUT, IpSecSaPolicy.MODE_TRANSPORT,
                "192.168.0.1", "192.168.0.2");
        List<IpSecSaPolicy> mSaPolicys = List.of(mSaPolicy);
        IpSecSaParameter saParameter = new IpSecSaParameter(id,
                IpSecSaParameter.INTEGRITY_ALGORITHM_HMAC_SHA_1_96, ik,
                IpSecSaParameter.ENCRYPTION_ALGORITHM_AES_CBC, ck, mSaPolicys);
        Parcel data = Parcel.obtain();
        try {
            data.writeInt(SLOT0);
            data.writeInt(SystemConstants.ADD_IPSEC_SA_PARAMETER);
            saParameter.writeToParcel(data, 0);
            data.setDataPosition(0);
            mSystemInterface.onMessage(data, null);
        } finally {
            data.recycle();
        }

        verify(mSystemCall).addIpSecSaParameter(any(IpSecSaParameter.class));
    }

    @Test
    @SmallTest
    public void testSystemCallRemoveIpSecSaParameter() {
        setUpSystemInterface();
        setUpSystem();
        int id = 12;
        Parcel data = Parcel.obtain();
        try {
            data.writeInt(SLOT0);
            data.writeInt(SystemConstants.REMOVE_IPSEC_SA_PARAMETER);
            data.writeInt(id);
            data.setDataPosition(0);
            mSystemInterface.onMessage(data, null);
        } finally {
            data.recycle();
        }

        verify(mSystemCall).removeIpSecSaParameter(eq(id));
    }

    @Test
    @SmallTest
    public void testSystemCallApplyIpSecSa() {
        setUpSystemInterface();
        setUpSystem();
        int ipsecId = 1;
        int spi = 10;
        int intFd = 20;
        FileDescriptor fd = new FileDescriptor();
        Parcel data = Parcel.obtain();
        try {
            data.writeInt(SLOT0);
            data.writeInt(SystemConstants.APPLY_IPSEC_SA);
            data.writeInt(ipsecId);
            data.writeInt(spi);
            data.writeInt(intFd);
            data.setDataPosition(0);
            mSystemInterface.onMessage(data, fd);
        } finally {
            data.recycle();
        }

        verify(mSystemCall).applyIpSecSa(eq(ipsecId), eq(spi), eq(intFd), eq(fd));
    }

    @Test
    @SmallTest
    public void testSystemCallRemoveIpSecSa() {
        setUpSystemInterface();
        setUpSystem();
        int ipsecId = 1;
        int spi = 10;
        int intFd = 20;
        FileDescriptor fd = new FileDescriptor();
        Parcel data = Parcel.obtain();
        try {
            data.writeInt(SLOT0);
            data.writeInt(SystemConstants.REMOVE_IPSEC_SA);
            data.writeInt(ipsecId);
            data.writeInt(spi);
            data.writeInt(intFd);
            data.setDataPosition(0);
            mSystemInterface.onMessage(data, fd);
        } finally {
            data.recycle();
        }

        verify(mSystemCall).removeIpSecSa(eq(ipsecId), eq(spi), eq(intFd), eq(fd));
    }

    @Test
    @SmallTest
    public void testSystemCallIsCrossSimRedialingAvailable() {
        setUpSystemInterface();
        setUpSystem();
        Parcel data = Parcel.obtain();
        int slotId = 1;
        try {
            data.writeInt(SLOT0);
            data.writeInt(SystemConstants.IS_CROSS_SIM_REDIALING_AVAILABLE);
            data.writeInt(slotId);
            data.setDataPosition(0);
            mSystemInterface.onMessage(data, null);
        } finally {
            data.recycle();
        }

        verify(mDefaultSystemCall).isCrossSimRedialingAvailable(eq(slotId));
    }

    @Test
    @SmallTest
    public void testSystemCallGetCarrierConfig() {
        setUpSystemInterface();
        setUpSystem();
        CarrierConfig cc = mock(CarrierConfig.class);
        when(mSystemCall.getCarrierConfig()).thenReturn(cc);
        Parcel data = Parcel.obtain();
        try {
            data.writeInt(SLOT0);
            data.writeInt(SystemConstants.GET_CARRIER_CONFIG);
            data.setDataPosition(0);
            mSystemInterface.onMessage(data, null);
        } finally {
            data.recycle();
        }

        verify(mSystemCall).getCarrierConfig();
        verify(cc).writeToParcel(any(Parcel.class));
    }

    @Test
    @SmallTest
    public void testSystemCallGetCarrierConfigWhenCarrierConfigNull() {
        setUpSystemInterface();
        setUpSystem();
        when(mSystemCall.getCarrierConfig()).thenReturn(null);
        Parcel data = Parcel.obtain();
        try {
            data.writeInt(SLOT0);
            data.writeInt(SystemConstants.GET_CARRIER_CONFIG);
            data.setDataPosition(0);
            mSystemInterface.onMessage(data, null);
        } finally {
            data.recycle();
        }

        verify(mSystemCall).getCarrierConfig();
    }

    @Test
    @SmallTest
    public void testSystemCallIsImsVoiceCallSupported() {
        setUpSystemInterface();
        setUpSystem();
        Parcel data = Parcel.obtain();
        try {
            data.writeInt(SLOT0);
            data.writeInt(SystemConstants.IS_IMS_VOICE_CALL_SUPPORTED);
            data.setDataPosition(0);
            mSystemInterface.onMessage(data, null);
        } finally {
            data.recycle();
        }

        verify(mSystemCall).isImsVoiceCallSupported();
    }

    @Test
    @SmallTest
    public void testSystemCallIsImsVoiceCallSupportedWhenSystemCallNull() {
        setUpSystemInterface();
        setUpSystem();
        ISystem system = mSystemInterface.getSystem(SLOT0);
        system.setSystemCallInterface(null);
        Parcel data = Parcel.obtain();
        try {
            data.writeInt(SLOT0);
            data.writeInt(SystemConstants.IS_IMS_VOICE_CALL_SUPPORTED);
            data.setDataPosition(0);
            mSystemInterface.onMessage(data, null);
        } finally {
            data.recycle();
        }

        verify(mSystemCall, never()).isImsVoiceCallSupported();
    }

    private void setUpSystemInterface() {
        mSystemInterface.init();
        mSystemInterface.setSystemCallInterface(mDefaultSystemCall);
        when(mDefaultSystemCall.getWifiInterface()).thenReturn(mWifiInterface);
    }

    private ISystem setUpSystem() {
        mSystemInterface.start(SLOT0, mMessageExecutor, mMmTelFeatureRegistry);
        ISystem system = mSystemInterface.getSystem(SLOT0);
        system.setSystemCallInterface(mSystemCall);
        return system;
    }

    private ISystem setUpSystemWithLooper() {
        return setUpSystemWithLooper(-1,
                ImsServiceRegistry.getInstance(SLOT0).getMmTelFeatureRegistry());
    }

    private ISystem setUpSystemWithLooper(int event) {
        return setUpSystemWithLooper(event,
                ImsServiceRegistry.getInstance(SLOT0).getMmTelFeatureRegistry());
    }

    private ISystem setUpSystemWithLooper(int event, MmTelFeatureRegistry mmTelFeatureRegistry) {
        mSystemInterface.start(SLOT0, mMessageExecutor, mmTelFeatureRegistry);
        ISystem system = mSystemInterface.getSystem(SLOT0);
        system.setSystemCallInterface(mSystemCall);

        if (event > 0) {
            registerEvent(event);
        }
        return system;
    }

    private void setUpSharedPreferences(String key, String value) {
        Context context = mock(Context.class);
        AppContext.deinit();
        AppContext.init(context);
        if (key == null) {
            when(mSp.getString(anyString(), anyString())).thenReturn(value);
        } else {
            when(mSp.getString(eq(key), anyString())).thenReturn(value);
        }
        when(mSp.edit()).thenReturn(mSpEditor);
        when(context.getSharedPreferences(anyString(), anyInt())).thenReturn(mSp);
    }

    private MmTelFeatureRegistry.Listener getMmTelFeatureRegistryListener() {
        ArgumentCaptor<MmTelFeatureRegistry.Listener> captor =
                ArgumentCaptor.forClass(MmTelFeatureRegistry.Listener.class);
        verify(mMmTelFeatureRegistry).addListener(captor.capture());
        return captor.getValue();
    }

    private void registerEvent(int event) {
        Parcel p = Parcel.obtain();
        try {
            p.writeInt(SLOT0);
            p.writeInt(SystemConstants.SET_EVENT);
            p.writeInt(event);
            p.setDataPosition(0);
            mSystemInterface.onMessage(p, null);
        } finally {
            p.recycle();
        }
    }

    private void unregisterEvent(int event) {
        Parcel p = Parcel.obtain();
        try {
            p.writeInt(SLOT0);
            p.writeInt(SystemConstants.RESET_EVENT);
            p.writeInt(event);
            p.setDataPosition(0);
            mSystemInterface.onMessage(p, null);
        } finally {
            p.recycle();
        }
    }

    private Parcel getDataForSystem() {
        ArgumentCaptor<byte[]> captor = ArgumentCaptor.forClass(byte[].class);
        verify(mJniIms).sendDataForSystem(eq(NATIVE_OBJECT), captor.capture());
        byte[] value = captor.getValue();
        Parcel data = Parcel.obtain();
        data.unmarshall(value, 0, value.length);
        data.setDataPosition(0);
        return data;
    }
}
