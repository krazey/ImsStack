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
package com.android.imsstack.enabler.aos;

import static com.android.imsstack.enabler.aos.AosDebugTest.FakeAosDebug.LatchType;

import static org.junit.Assert.assertEquals;
import static org.junit.Assert.assertFalse;
import static org.junit.Assert.assertNotEquals;
import static org.junit.Assert.assertNotNull;
import static org.junit.Assert.assertNull;
import static org.junit.Assert.assertTrue;
import static org.mockito.ArgumentMatchers.any;
import static org.mockito.ArgumentMatchers.anyInt;
import static org.mockito.ArgumentMatchers.anyString;
import static org.mockito.Mockito.atLeastOnce;
import static org.mockito.Mockito.doReturn;
import static org.mockito.Mockito.mock;
import static org.mockito.Mockito.never;
import static org.mockito.Mockito.spy;
import static org.mockito.Mockito.times;
import static org.mockito.Mockito.verify;
import static org.mockito.Mockito.when;

import android.app.Activity;
import android.app.NotificationChannel;
import android.app.NotificationManager;
import android.content.Intent;
import android.content.pm.PackageManager;
import android.net.LinkAddress;
import android.net.LinkProperties;
import android.net.Network;
import android.net.NetworkCapabilities;
import android.net.wifi.WifiInfo;
import android.os.Looper;
import android.os.Message;
import android.telephony.AccessNetworkConstants;
import android.telephony.CellSignalStrength;
import android.telephony.CellSignalStrengthGsm;
import android.telephony.CellSignalStrengthLte;
import android.telephony.CellSignalStrengthNr;
import android.telephony.CellSignalStrengthWcdma;
import android.telephony.PreciseDataConnectionState;
import android.telephony.ServiceState;
import android.telephony.SignalStrength;
import android.telephony.TelephonyManager;
import android.telephony.data.ApnSetting;
import android.util.Singleton;

import com.android.imsstack.ImsStackTest;
import com.android.imsstack.base.MSimUtils;
import com.android.imsstack.base.TestAppContext;
import com.android.imsstack.core.agents.AgentFactory;
import com.android.imsstack.core.agents.IPhoneStateNotifier;
import com.android.imsstack.core.agents.ImsPhoneStateListener;
import com.android.imsstack.core.agents.NativeStateInterface;
import com.android.imsstack.core.agents.PhoneStateInterface;
import com.android.imsstack.core.agents.Sim;
import com.android.imsstack.core.agents.SimInterface;
import com.android.imsstack.core.agents.TelephonyInterface;
import com.android.imsstack.core.carrier.SimCarrierId;
import com.android.imsstack.enabler.aos.service.AosService;

import org.junit.After;
import org.junit.Before;
import org.junit.Ignore;
import org.junit.Test;
import org.junit.runner.RunWith;
import org.junit.runners.JUnit4;
import org.mockito.Mock;
import org.mockito.MockitoAnnotations;

import java.util.ArrayList;
import java.util.List;
import java.util.concurrent.CountDownLatch;

@RunWith(JUnit4.class)
public class AosDebugTest extends ImsStackTest {
    private static final String TEST_VALUE = "TEST_VALUE";

    private TestAppContext mTestAppContext;
    private FakeAosDebug mFakeAosDebug;

    @Mock Activity mMockActivity;
    @Mock AosDebug.ConnectivityCallback mMockConnectivityCallback;
    @Mock AosDebug.DebugImsPhoneStateListener mMockImsPhoneStateListener;
    @Mock AosDebug.SignalStrengthsListener mMockSignalStrengthsListener;
    @Mock AosService mMockAosService;
    @Mock IPhoneStateNotifier mMockIPhoneStateNotifier;
    @Mock NativeStateInterface mMockNativeStateInterface;
    @Mock NotificationManager mMockNotificationManager;
    @Mock PhoneStateInterface mMockPhoneStateInterface;
    @Mock ServiceState mMockServiceState;
    @Mock SimInterface mMockSimInterface;
    @Mock TelephonyInterface mMockTelephonyInterface;
    @Mock ApnSetting mMockApnSetting;

    @Before
    public void setup() throws Exception {
        super.setUp(getClass().getSimpleName());
        MockitoAnnotations.initMocks(this);

        mTestAppContext = new TestAppContext(mContext);
        mTestAppContext.setUp();

        // AgentFactory
        AgentFactory.getInstance().setAgent(
                SimInterface.class, mMockSimInterface, TestAppContext.SLOT0);
        when(mMockSimInterface.getSubId()).thenReturn(TestAppContext.SUB_ID_1);

        AgentFactory.getInstance().setAgent(
                PhoneStateInterface.class, mMockPhoneStateInterface, TestAppContext.SLOT0);
        when(mMockPhoneStateInterface.createNotifier(
                any(ImsPhoneStateListener.class), any(Looper.class)))
                .thenReturn(mMockIPhoneStateNotifier);

        AgentFactory.getInstance().setAgent(
                NativeStateInterface.class, mMockNativeStateInterface, TestAppContext.SLOT0);

        AgentFactory.getInstance().setAgent(
                TelephonyInterface.class, mMockTelephonyInterface, TestAppContext.SLOT0);

        // AosFactory
        AosFactory.getInstance().replaceService(TestAppContext.SLOT0, mMockAosService);

        // FakeAosDebug
        mFakeAosDebug = spy(new FakeAosDebug(TestAppContext.SLOT0));
        mFakeAosDebug.mNotificationManager = mMockNotificationManager;
        mFakeAosDebug.mSignalStrengthsListener = mMockSignalStrengthsListener;
        mFakeAosDebug.mConnectivityCallback = mMockConnectivityCallback;
        mFakeAosDebug.mImsPhoneStateListener = mMockImsPhoneStateListener;

        mFakeAosDebug.init();
    }

    @After
    public void tearDown() throws Exception {
        if (mFakeAosDebug != null) {
            mFakeAosDebug.cleanup();
        }
        super.tearDown();

        AosFactory.getInstance().replaceService(TestAppContext.SLOT0, null);
        AgentFactory.getInstance().setAgent(TelephonyInterface.class, null, TestAppContext.SLOT0);
        AgentFactory.getInstance().setAgent(NativeStateInterface.class, null, TestAppContext.SLOT0);
        AgentFactory.getInstance().setAgent(PhoneStateInterface.class, null, TestAppContext.SLOT0);
        AgentFactory.getInstance().setAgent(SimInterface.class, null, TestAppContext.SLOT0);

        mTestAppContext.tearDown();
        mTestAppContext = null;
    }

    @Test
    public void testInit() {
        // GIVEN
        // WHEN init() has already been called in setup().

        // THEN
        assertNotNull(mFakeAosDebug.mContext);
        assertNotNull(mFakeAosDebug.mDebugData);
        assertNotNull(mFakeAosDebug.mHandler);
        assertNotNull(mFakeAosDebug.mNativeStateListener);
        assertNotNull(mFakeAosDebug.mSimListener);
        assertNotNull(mFakeAosDebug.mAosRegistration);
        assertNotNull(mFakeAosDebug.mRegistrationListener);
        assertNotNull(mFakeAosDebug.mDebugBroadcastReceiver);
        assertNotEquals(MSimUtils.INVALID_SUB_ID, mFakeAosDebug.mSubId);
        assertNotNull(mFakeAosDebug.mSignalStrengthsListener);
        assertNotNull(mFakeAosDebug.mConnectivityCallback);
        assertNotNull(mFakeAosDebug.mImsPhoneStateListener);

        verify(mMockNativeStateInterface).addListener(any(NativeStateInterface.Listener.class));
        verify(mMockSimInterface).addListener(any(Sim.Listener.class));
        verify(mMockAosService).addListener(any(FakeAosDebug.RegistrationListener.class));
        verify(mMockSignalStrengthsListener).register();
        verify(mMockConnectivityCallback).register();
        verify(mMockImsPhoneStateListener).setListener();
    }

    @Test
    public void testCleanup() {
        // GIVEN
        assertNotNull(mFakeAosDebug.mImsPhoneStateListener);
        assertNotNull(mFakeAosDebug.mConnectivityCallback);
        assertNotNull(mFakeAosDebug.mSignalStrengthsListener);
        assertNotNull(mFakeAosDebug.mDebugBroadcastReceiver);
        assertNotNull(mFakeAosDebug.mRegistrationListener);
        assertNotNull(mFakeAosDebug.mAosRegistration);
        assertNotNull(mFakeAosDebug.mSimListener);
        assertNotNull(mFakeAosDebug.mNativeStateListener);
        assertNotNull(mFakeAosDebug.mHandler);
        assertNotNull(mFakeAosDebug.mDebugData);
        assertNotNull(mFakeAosDebug.mNotificationManager);
        assertNotNull(mFakeAosDebug.mContext);

        // WHEN
        mFakeAosDebug.cleanup();

        // THEN
        assertNull(mFakeAosDebug.mImsPhoneStateListener);
        assertNull(mFakeAosDebug.mConnectivityCallback);
        assertNull(mFakeAosDebug.mSignalStrengthsListener);
        assertNull(mFakeAosDebug.mDebugBroadcastReceiver);
        assertNull(mFakeAosDebug.mRegistrationListener);
        assertNull(mFakeAosDebug.mAosRegistration);
        assertNull(mFakeAosDebug.mSimListener);
        assertNull(mFakeAosDebug.mNativeStateListener);
        assertNull(mFakeAosDebug.mHandler);
        assertNull(mFakeAosDebug.mDebugData);
        assertNull(mFakeAosDebug.mNotificationManager);
        assertNull(mFakeAosDebug.mContext);
    }

    @Test
    public void testShowOrDismissNotificationDisabled() {
        // GIVEN
        mFakeAosDebug.mIsDebugScreenEnabled = false;

        // WHEN
        mFakeAosDebug.showOrDismissNotification(mMockActivity);

        // THEN
        verify(mMockNotificationManager).cancel(anyInt());
        verify(mMockNotificationManager).deleteNotificationChannel(anyString());

        verify(mMockNotificationManager, never()).createNotificationChannel(any());
        verify(mMockNotificationManager, never()).notify(anyInt(), any());

        assertEquals(1, mFakeAosDebug.getLatch(LatchType.ON_REQUEST_PERMISSION).getCount());
    }

    @Test
    public void testShowOrDismissNotificationDisabledNotificationManagerIsNull() {
        // GIVEN
        mFakeAosDebug.mIsDebugScreenEnabled = false;
        mFakeAosDebug.mNotificationManager = null;

        // WHEN
        mFakeAosDebug.showOrDismissNotification(mMockActivity);

        // THEN
        verify(mMockNotificationManager, never()).cancel(anyInt());
        verify(mMockNotificationManager, never()).deleteNotificationChannel(anyString());

        verify(mMockNotificationManager, never()).createNotificationChannel(any());
        verify(mMockNotificationManager, never()).notify(anyInt(), any());

        assertEquals(1, mFakeAosDebug.getLatch(LatchType.ON_REQUEST_PERMISSION).getCount());
    }

    @Test
    @Ignore
    public void testShowOrDismissNotificationPermissionGranted() throws Exception {
        // GIVEN
        mFakeAosDebug.mIsDebugScreenEnabled = true;
        mFakeAosDebug.mIsGranted = true;
        // Restore the ActivityManager to create a real PendingIntent.
        restoreInstance(Singleton.class, "mInstance", mIActivityManagerSingleton);

        // WHEN
        mFakeAosDebug.showOrDismissNotification(mMockActivity);

        // THEN
        verify(mMockNotificationManager, never()).cancel(anyInt());
        verify(mMockNotificationManager, never()).deleteNotificationChannel(anyString());

        verify(mMockNotificationManager, atLeastOnce())
                .createNotificationChannel(any(NotificationChannel.class));
        verify(mMockNotificationManager, atLeastOnce()).notify(anyInt(), any());

        assertEquals(1, mFakeAosDebug.getLatch(LatchType.ON_REQUEST_PERMISSION).getCount());
    }

    @Test
    public void testShowOrDismissNotificationPermissionGrantedButNotificationManagerIsNull() {
        // GIVEN
        mFakeAosDebug.mIsDebugScreenEnabled = true;
        mFakeAosDebug.mIsGranted = true;
        mFakeAosDebug.mNotificationManager = null;

        // WHEN
        mFakeAosDebug.showOrDismissNotification(mMockActivity);

        // THEN
        verify(mMockNotificationManager, never()).cancel(anyInt());
        verify(mMockNotificationManager, never()).deleteNotificationChannel(anyString());

        verify(mMockNotificationManager, never())
                .createNotificationChannel(any(NotificationChannel.class));
        verify(mMockNotificationManager, never()).notify(anyInt(), any());

        assertEquals(1, mFakeAosDebug.getLatch(LatchType.ON_REQUEST_PERMISSION).getCount());
    }

    @Test
    public void testShowOrDismissNotificationRequestPermission() {
        // GIVEN
        mFakeAosDebug.mIsDebugScreenEnabled = true;
        mFakeAosDebug.mIsGranted = false;

        // WHEN
        mFakeAosDebug.showOrDismissNotification(mMockActivity);

        // THEN
        verify(mMockNotificationManager, never()).cancel(anyInt());
        verify(mMockNotificationManager, never()).deleteNotificationChannel(anyString());
        verify(mMockNotificationManager, never())
                .createNotificationChannel(any(NotificationChannel.class));
        verify(mMockNotificationManager, never()).notify(anyInt(), any());

        assertEquals(0, mFakeAosDebug.getLatch(LatchType.ON_REQUEST_PERMISSION).getCount());
    }

    @Test
    @Ignore
    public void testNotifyPermissionsResultPermissionGranted() throws Exception {
        // GIVEN
        int requestCode = FakeAosDebug.REQUEST_CODE_DEBUG;
        int[] grantResults = new int[]{ PackageManager.PERMISSION_GRANTED };
        // Restore the ActivityManager to create a real PendingIntent.
        restoreInstance(Singleton.class, "mInstance", mIActivityManagerSingleton);

        // WHEN
        mFakeAosDebug.notifyPermissionsResult(requestCode, new String[]{}, grantResults,
                mMockActivity);

        // THEN
        verify(mMockNotificationManager, atLeastOnce())
                .createNotificationChannel(any(NotificationChannel.class));
        verify(mMockNotificationManager, atLeastOnce()).notify(anyInt(), any());

        assertEquals(1, mFakeAosDebug.getLatch(LatchType.ON_REQUEST_PERMISSION).getCount());
    }

    @Test
    public void testNotifyPermissionsResultPermissionDenied() {
        // GIVEN
        int requestCode = FakeAosDebug.REQUEST_CODE_DEBUG;
        int[] grantResults = new int[]{ PackageManager.PERMISSION_DENIED };

        // WHEN
        mFakeAosDebug.notifyPermissionsResult(requestCode, new String[]{}, grantResults,
                mMockActivity);

        // THEN
        verify(mMockNotificationManager, never())
                .createNotificationChannel(any(NotificationChannel.class));
        verify(mMockNotificationManager, never()).notify(anyInt(), any());

        assertEquals(0, mFakeAosDebug.getLatch(LatchType.ON_HANDLE_PERMISSION_DENIED).getCount());
    }

    @Test
    public void testNotifyPermissionsResultRequestCodeInvalid() {
        // GIVEN
        int requestCode = -1;
        int[] grantResults = new int[]{ PackageManager.PERMISSION_GRANTED };

        // WHEN
        mFakeAosDebug.notifyPermissionsResult(requestCode, new String[]{}, grantResults,
                mMockActivity);

        // THEN
        verify(mMockNotificationManager, never())
                .createNotificationChannel(any(NotificationChannel.class));
        verify(mMockNotificationManager, never()).notify(anyInt(), any());

        assertEquals(1, mFakeAosDebug.getLatch(LatchType.ON_REQUEST_PERMISSION).getCount());
    }

    @Test
    public void testNotifyPermissionsResultGrantResultsInvalid() {
        // GIVEN
        int requestCode = FakeAosDebug.REQUEST_CODE_DEBUG;
        int[] grantResults = new int[]{};

        // WHEN
        mFakeAosDebug.notifyPermissionsResult(requestCode, new String[]{}, grantResults,
                mMockActivity);

        // THEN
        verify(mMockNotificationManager, never())
                .createNotificationChannel(any(NotificationChannel.class));
        verify(mMockNotificationManager, never()).notify(anyInt(), any());

        assertEquals(1, mFakeAosDebug.getLatch(LatchType.ON_REQUEST_PERMISSION).getCount());
    }

    @Test
    public void testGetDebugMessageRegistered() {
        // GIVEN
        mFakeAosDebug.mDebugData.put(IAosDebug.DebugKey.REGISTER,
                IAosDebug.DebugData.STR_IMS_REGISTERED);
        mFakeAosDebug.mDebugData.put(IAosDebug.DebugKey.FEATURES,
                "TEST_VALUE_FEATURES");

        // WHEN
        String text = mFakeAosDebug.getDebugMessage();

        // THEN
        assertTrue(text.contains(IAosDebug.DebugData.STR_IMS_REGISTERED));
        assertTrue(text.contains("TEST_VALUE_FEATURES"));
    }

    @Test
    public void testGetDebugMessageDeregistered() {
        // GIVEN
        mFakeAosDebug.mDebugData.put(IAosDebug.DebugKey.REGISTER,
                IAosDebug.DebugData.STR_IMS_DEREGISTERED);
        mFakeAosDebug.mDebugData.put(IAosDebug.DebugKey.DEREGISTER_REASON,
                "TEST_VALUE_DEREGISTER_REASON");

        // WHEN
        String text = mFakeAosDebug.getDebugMessage();

        // THEN
        assertTrue(text.contains(IAosDebug.DebugData.STR_IMS_DEREGISTERED));
        assertTrue(text.contains("TEST_VALUE_DEREGISTER_REASON"));
    }

    @Test
    public void testGetDebugMessageDataConnected() {
        // GIVEN
        mFakeAosDebug.mDebugData.put(IAosDebug.DebugKey.DATA_CONNECTION_STATE, "CONNECTED");
        mFakeAosDebug.mDebugData.put(IAosDebug.DebugKey.CELLULAR_DATA_RAT, "LTE");

        mFakeAosDebug.mDebugData.put(IAosDebug.DebugKey.NETWORK_TYPE, TEST_VALUE + 1);
        mFakeAosDebug.mDebugData.put(IAosDebug.DebugKey.IP_ADDRESSES, TEST_VALUE + 2);
        mFakeAosDebug.mDebugData.put(IAosDebug.DebugKey.INTERFACE_NAME, TEST_VALUE + 3);
        mFakeAosDebug.mDebugData.put(IAosDebug.DebugKey.MTU, TEST_VALUE + 4);
        mFakeAosDebug.mDebugData.put(IAosDebug.DebugKey.APN_NAME, TEST_VALUE + 5);
        mFakeAosDebug.mDebugData.put(IAosDebug.DebugKey.APN_ENTRY_NAME, TEST_VALUE + 6);
        mFakeAosDebug.mDebugData.put(IAosDebug.DebugKey.APN_TYPES, TEST_VALUE + 7);
        mFakeAosDebug.mDebugData.put(IAosDebug.DebugKey.PCSCF_ADDRESSES, TEST_VALUE + 8);
        mFakeAosDebug.mDebugData.put(IAosDebug.DebugKey.SERVICE_STATE, TEST_VALUE + 9);
        mFakeAosDebug.mDebugData.put(IAosDebug.DebugKey.DATA_REG_STATE,  TEST_VALUE + 10);
        mFakeAosDebug.mDebugData.put(IAosDebug.DebugKey.LTE_ATTACH_TYPE, TEST_VALUE + 11);
        mFakeAosDebug.mDebugData.put(IAosDebug.DebugKey.ROAMING_STATE, TEST_VALUE + 12);
        mFakeAosDebug.mDebugData.put(IAosDebug.DebugKey.VOICE_ROAMING_TYPE, TEST_VALUE + 13);
        mFakeAosDebug.mDebugData.put(IAosDebug.DebugKey.DATA_ROAMING_TYPE, TEST_VALUE + 14);
        mFakeAosDebug.mDebugData.put(IAosDebug.DebugKey.VOICE_RAT, TEST_VALUE + 15);
        mFakeAosDebug.mDebugData.put(IAosDebug.DebugKey.NETWORK_OPERATOR, TEST_VALUE + 16);
        mFakeAosDebug.mDebugData.put(IAosDebug.DebugKey.NETWORK_OPERATOR_NUMERIC,
                TEST_VALUE + 17);
        mFakeAosDebug.mDebugData.put(IAosDebug.DebugKey.NETWORK_SUPPORT_VOPS,
                TEST_VALUE + 18);
        mFakeAosDebug.mDebugData.put(IAosDebug.DebugKey.NETWORK_SUPPORT_EMCBS,
                TEST_VALUE + 19);

        // WHEN
        String text = mFakeAosDebug.getDebugMessage();

        // THEN
        assertTrue(text.contains("CONNECTED"));
        assertTrue(text.contains("LTE"));

        for (int i = 1; i < 20; i++) {
            assertTrue(text.contains(TEST_VALUE + i));
        }
    }

    @Test
    public void testGetDebugMessageSignalStrength() {
        //GIVEN
        mFakeAosDebug.mDebugData.put(IAosDebug.DebugKey.DATA_CONNECTION_STATE, "CONNECTED");

        mFakeAosDebug.mDebugData.put(IAosDebug.DebugKey.UTRAN_LEVEL, TEST_VALUE + 1);
        mFakeAosDebug.mDebugData.put(IAosDebug.DebugKey.UTRAN_DBM, TEST_VALUE + 2);
        mFakeAosDebug.mDebugData.put(IAosDebug.DebugKey.EUTRAN_RSRP, TEST_VALUE + 3);
        mFakeAosDebug.mDebugData.put(IAosDebug.DebugKey.EUTRAN_RSRQ, TEST_VALUE + 4);
        mFakeAosDebug.mDebugData.put(IAosDebug.DebugKey.NGRAN_SSRSRP, TEST_VALUE + 5);
        mFakeAosDebug.mDebugData.put(IAosDebug.DebugKey.NGRAN_SSRSRQ, TEST_VALUE + 6);

        // WHEN
        String text = mFakeAosDebug.getDebugMessage();

        // THEN
        assertTrue(text.contains("CONNECTED"));

        for (int i = 1; i < 7; i++) {
            assertTrue(text.contains(TEST_VALUE + i));
        }
    }

    @Test
    public void testGetDebugMessageWifiConnected() {
        // GIVEN
        mFakeAosDebug.mDebugData.put(IAosDebug.DebugKey.WIFI_CONNECTION_STATE,
                IAosDebug.DebugData.STR_CONNECTED);
        mFakeAosDebug.mDebugData.put(IAosDebug.DebugKey.WIFI_ADDRESSES, TEST_VALUE + 1);
        mFakeAosDebug.mDebugData.put(IAosDebug.DebugKey.WIFI_INTERFACE_NAME, TEST_VALUE + 2);
        mFakeAosDebug.mDebugData.put(IAosDebug.DebugKey.WIFI_RSSI, TEST_VALUE + 3);
        mFakeAosDebug.mDebugData.put(IAosDebug.DebugKey.WIFI_BSSID, TEST_VALUE + 4);
        mFakeAosDebug.mDebugData.put(IAosDebug.DebugKey.WIFI_SSID, TEST_VALUE + 5);
        mFakeAosDebug.mDebugData.put(IAosDebug.DebugKey.WIFI_MAC_ADDRESS, TEST_VALUE + 6);

        // WHEN
        String text = mFakeAosDebug.getDebugMessage();

        // THEN
        assertTrue(text.contains(IAosDebug.DebugData.STR_CONNECTED));

        for (int i = 1; i < 7; i++) {
            assertTrue(text.contains(TEST_VALUE + i));
        }
    }

    @Test
    public void testGetDebugMessageWifiDisconnected() {
        // GIVEN
        mFakeAosDebug.mDebugData.put(IAosDebug.DebugKey.WIFI_CONNECTION_STATE,
                IAosDebug.DebugData.STR_DISCONNECTED);
        mFakeAosDebug.mDebugData.put(IAosDebug.DebugKey.WIFI_ADDRESSES, TEST_VALUE + 1);
        mFakeAosDebug.mDebugData.put(IAosDebug.DebugKey.WIFI_INTERFACE_NAME, TEST_VALUE + 2);
        mFakeAosDebug.mDebugData.put(IAosDebug.DebugKey.WIFI_RSSI, TEST_VALUE + 3);
        mFakeAosDebug.mDebugData.put(IAosDebug.DebugKey.WIFI_BSSID, TEST_VALUE + 4);
        mFakeAosDebug.mDebugData.put(IAosDebug.DebugKey.WIFI_SSID, TEST_VALUE + 5);
        mFakeAosDebug.mDebugData.put(IAosDebug.DebugKey.WIFI_MAC_ADDRESS, TEST_VALUE + 6);

        // WHEN
        String text = mFakeAosDebug.getDebugMessage();

        // THEN
        assertTrue(text.contains(IAosDebug.DebugData.STR_DISCONNECTED));

        for (int i = 1; i < 7; i++) {
            assertFalse(text.contains(TEST_VALUE + i));
        }
    }

    @Test
    @Ignore
    public void testNativeStateListenerOnNativeServiceReady() throws Exception {
        // GIVEN
        mFakeAosDebug.mIsDebugScreenEnabled = true;
        mFakeAosDebug.mIsGranted = false;
        // Restore the ActivityManager to create a real PendingIntent.
        restoreInstance(Singleton.class, "mInstance", mIActivityManagerSingleton);

        // WHEN
        mFakeAosDebug.mNativeStateListener.onNativeServiceReady();

        // THEN
        verify(mMockNotificationManager, never()).cancel(anyInt());
        verify(mMockNotificationManager, never()).deleteNotificationChannel(anyString());
        verify(mMockNotificationManager, times(1))
                .createNotificationChannel(any(NotificationChannel.class));
        assertEquals(0, mFakeAosDebug.getLatch(LatchType.ON_SEND_NOTIFICATION).getCount());
        assertEquals(1, mFakeAosDebug.getLatch(LatchType.ON_REQUEST_PERMISSION).getCount());
    }

    @Test
    public void testHandleMessageHandleAirplaneModeChangedStateIsTrue() {
        // GIVEN
        for (IAosDebug.DebugKey key : IAosDebug.DebugKey.values()) {
            if (!IAosDebug.PRESERVED_KEYS.contains(key)) {
                mFakeAosDebug.mDebugData.put(key, TEST_VALUE + key.getValue());
            }
        }

        boolean isOn = true;

        // WHEN
        Message msg = Message.obtain();
        msg.what = com.android.imsstack.enabler.aos.AosDebug.DEBUG_AIRPLANE_MODE_CHANGED;
        msg.obj = new Intent(Intent.ACTION_AIRPLANE_MODE_CHANGED).putExtra("state", isOn);
        mFakeAosDebug.mHandler.handleMessage(msg);

        // THEN Expects everything to be "STR_EMPTY" except "KEY_XXX_CONNECTION_STATE".
        for (IAosDebug.DebugKey key : IAosDebug.DebugKey.values()) {
            if (!IAosDebug.PRESERVED_KEYS.contains(key)) {
                if (key == IAosDebug.DebugKey.DATA_CONNECTION_STATE
                        || key == IAosDebug.DebugKey.WIFI_CONNECTION_STATE) {
                    assertEquals(IAosDebug.DebugData.STR_DISCONNECTED,
                            mFakeAosDebug.mDebugData.get(key));
                } else {
                    assertEquals(IAosDebug.DebugData.STR_EMPTY, mFakeAosDebug.mDebugData.get(key));
                }
            }
        }
    }

    @Test
    public void testHandleMessageHandleAirplaneModeChangedStateIsFalse() {
        // GIVEN
        for (IAosDebug.DebugKey key : IAosDebug.DebugKey.values()) {
            if (!IAosDebug.PRESERVED_KEYS.contains(key)) {
                mFakeAosDebug.mDebugData.put(key, TEST_VALUE + key.getValue());
            }
        }

        boolean isOn = false;

        // WHEN
        Message msg = Message.obtain();
        msg.what = com.android.imsstack.enabler.aos.AosDebug.DEBUG_AIRPLANE_MODE_CHANGED;
        msg.obj = new Intent(Intent.ACTION_AIRPLANE_MODE_CHANGED).putExtra("state", isOn);
        mFakeAosDebug.mHandler.handleMessage(msg);

        // THEN Expect the values of ClearableKeys to not change.
        for (IAosDebug.DebugKey key : IAosDebug.DebugKey.values()) {
            if (!IAosDebug.PRESERVED_KEYS.contains(key)) {
                assertEquals(TEST_VALUE + key.getValue(), mFakeAosDebug.mDebugData.get(key));
            }
        }
    }

    @Test
    public void testHandleMessageHandleSubscriptionChanged() {
        // GIVEN
        Message msg = Message.obtain();
        msg.what = com.android.imsstack.enabler.aos.AosDebug.DEBUG_SUBSCRIPTION_CHANGED;

        // WHEN
        mFakeAosDebug.mHandler.handleMessage(msg);

        // THEN
        assertEquals(0, mFakeAosDebug.getLatch(LatchType.ON_UPDATE_SUBSCRIPTION).getCount());
        assertEquals(0, mFakeAosDebug.getLatch(LatchType.ON_UPDATE_CARRIER_INFO).getCount());
        assertEquals(0, mFakeAosDebug.getLatch(LatchType.ON_SELF_CHECK_DEBUG_NOTIFICATION)
                .getCount());
    }

    @Test
    public void testUpdateSubscriptionSuccess() {
        // GIVEN
        when(mMockSimInterface.isSimLoadCompleted()).thenReturn(true);
        when(mMockSimInterface.getSubId()).thenReturn(TestAppContext.SUB_ID_2);

        // WHEN
        mFakeAosDebug.updateSubscription();

        // THEN
        assertEquals(String.valueOf(TestAppContext.SUB_ID_2),
                mFakeAosDebug.mDebugData.get(IAosDebug.DebugKey.SUB_ID));

        verify(mMockSignalStrengthsListener, times(1)).unregister();
        // It must be called once in init() and once again in this function.
        verify(mMockSignalStrengthsListener, times(2)).register();
    }

    @Test
    public void testUpdateSubscriptionSubIdIsSame() {
        // GIVEN
        when(mMockSimInterface.isSimLoadCompleted()).thenReturn(true);

        // WHEN
        mFakeAosDebug.updateSubscription();

        // THEN
        assertEquals(String.valueOf(TestAppContext.SUB_ID_1),
                mFakeAosDebug.mDebugData.get(IAosDebug.DebugKey.SUB_ID));

        verify(mMockSignalStrengthsListener, never()).unregister();
        // It must be called once in init().
        verify(mMockSignalStrengthsListener, times(1)).register();
    }

    @Test
    public void testUpdateSubscriptionSimLoadIsNotCompleted() {
        // GIVEN
        when(mMockSimInterface.isSimLoadCompleted()).thenReturn(false);

        // WHEN
        mFakeAosDebug.updateSubscription();

        // THEN
        assertEquals(String.valueOf(TestAppContext.SUB_ID_1),
                mFakeAosDebug.mDebugData.get(IAosDebug.DebugKey.SUB_ID));

        verify(mMockSimInterface, never()).getSubId();
        verify(mMockSignalStrengthsListener, never()).unregister();
        // It must be called once in init().
        verify(mMockSignalStrengthsListener, times(1)).register();
    }

    @Test
    public void testUpdateSubscriptionSimInterfaceIsNull() {
        // GIVEN
        AgentFactory.getInstance().setAgent(SimInterface.class, null, TestAppContext.SLOT0);

        // WHEN
        mFakeAosDebug.updateSubscription();

        // THEN
        assertEquals(String.valueOf(TestAppContext.SUB_ID_1),
                mFakeAosDebug.mDebugData.get(IAosDebug.DebugKey.SUB_ID));

        verify(mMockSimInterface, never()).isSimLoadCompleted();
        verify(mMockSimInterface, never()).getSubId();
        verify(mMockSignalStrengthsListener, never()).unregister();
        // It must be called once in init.
        verify(mMockSignalStrengthsListener, times(1)).register();
    }

    @Test
    public void testUpdateCarrierInfoSuccess() {
        // GIVEN
        mFakeAosDebug.mSimCarrierId = new SimCarrierId.Builder().build();
        mFakeAosDebug.mOperator = IAosDebug.DebugData.STR_EMPTY;
        mFakeAosDebug.mCountry = IAosDebug.DebugData.STR_EMPTY;

        // WHEN
        mFakeAosDebug.updateCarrierInfo();

        // THEN
        assertNotEquals(IAosDebug.DebugData.STR_EMPTY, mFakeAosDebug.mOperator);
        assertNotEquals(IAosDebug.DebugData.STR_EMPTY, mFakeAosDebug.mCountry);
    }

    @Test
    public void testUpdateCarrierInfoCarrierIdIsNull() {
        // GIVEN
        mFakeAosDebug.mSimCarrierId = null;
        mFakeAosDebug.mOperator = IAosDebug.DebugData.STR_EMPTY;
        mFakeAosDebug.mCountry = IAosDebug.DebugData.STR_EMPTY;

        // WHEN
        mFakeAosDebug.updateCarrierInfo();

        // THEN
        assertEquals(IAosDebug.DebugData.STR_EMPTY, mFakeAosDebug.mOperator);
        assertEquals(IAosDebug.DebugData.STR_EMPTY, mFakeAosDebug.mCountry);
    }

    @Test
    public void testHandleMessageHandleSignalStrengthsChanged() {
        // GIVEN
        SignalStrength mockSignalStrength = mock(SignalStrength.class);
        List<CellSignalStrength> spyCellSignalStrengths = spy(new ArrayList<>());
        CellSignalStrength mockCellSignalStrength = mock(CellSignalStrength.class);
        spyCellSignalStrengths.add(mockCellSignalStrength);

        doReturn(spyCellSignalStrengths).when(mockSignalStrength).getCellSignalStrengths();
        doReturn(mockCellSignalStrength).when(spyCellSignalStrengths).get(0);

        Message msg = Message.obtain();
        msg.what = com.android.imsstack.enabler.aos.AosDebug.DEBUG_SIGNALSTRENGTHS_CHANGED;
        msg.obj = mockSignalStrength;

        // WHEN
        mFakeAosDebug.mHandler.handleMessage(msg);

        // THEN
        assertEquals(0,
                mFakeAosDebug.getLatch(LatchType.ON_UPDATE_SIGNAL_STRENGTH_DATA).getCount());
    }

    @Test
    public void testUpdateSignalStrengthDataUtran() {
        // GIVEN
        mFakeAosDebug.mDebugData.put(IAosDebug.DebugKey.UTRAN_LEVEL, "TEST_VALUE");
        mFakeAosDebug.mDebugData.put(IAosDebug.DebugKey.UTRAN_DBM, "TEST_VALUE");
        mFakeAosDebug.mDebugData.put(IAosDebug.DebugKey.EUTRAN_RSRP, "TEST_VALUE");
        mFakeAosDebug.mDebugData.put(IAosDebug.DebugKey.EUTRAN_RSRQ, "TEST_VALUE");
        mFakeAosDebug.mDebugData.put(IAosDebug.DebugKey.NGRAN_SSRSRP, "TEST_VALUE");
        mFakeAosDebug.mDebugData.put(IAosDebug.DebugKey.NGRAN_SSRSRQ, "TEST_VALUE");

        CellSignalStrength css = new CellSignalStrengthWcdma();
        int network = AccessNetworkConstants.AccessNetworkType.UTRAN;

        // WHEN
        mFakeAosDebug.updateSignalStrengthData(css, network);

        // THEN
        assertNotEquals("TEST_VALUE",
                mFakeAosDebug.mDebugData.get(IAosDebug.DebugKey.UTRAN_LEVEL));
        assertNotEquals("TEST_VALUE",
                mFakeAosDebug.mDebugData.get(IAosDebug.DebugKey.UTRAN_DBM));

        assertEquals("TEST_VALUE",
                mFakeAosDebug.mDebugData.get(IAosDebug.DebugKey.EUTRAN_RSRP));
        assertEquals("TEST_VALUE",
                mFakeAosDebug.mDebugData.get(IAosDebug.DebugKey.EUTRAN_RSRQ));

        assertEquals("TEST_VALUE",
                mFakeAosDebug.mDebugData.get(IAosDebug.DebugKey.NGRAN_SSRSRP));
        assertEquals("TEST_VALUE",
                mFakeAosDebug.mDebugData.get(IAosDebug.DebugKey.NGRAN_SSRSRQ));
    }

    @Test
    public void testUpdateSignalStrengthDataEutran() {
        // GIVEN
        mFakeAosDebug.mDebugData.put(IAosDebug.DebugKey.UTRAN_LEVEL, "TEST_VALUE");
        mFakeAosDebug.mDebugData.put(IAosDebug.DebugKey.UTRAN_DBM, "TEST_VALUE");
        mFakeAosDebug.mDebugData.put(IAosDebug.DebugKey.EUTRAN_RSRP, "TEST_VALUE");
        mFakeAosDebug.mDebugData.put(IAosDebug.DebugKey.EUTRAN_RSRQ, "TEST_VALUE");
        mFakeAosDebug.mDebugData.put(IAosDebug.DebugKey.NGRAN_SSRSRP, "TEST_VALUE");
        mFakeAosDebug.mDebugData.put(IAosDebug.DebugKey.NGRAN_SSRSRQ, "TEST_VALUE");

        CellSignalStrength css = new CellSignalStrengthLte();
        int network = AccessNetworkConstants.AccessNetworkType.EUTRAN;

        // WHEN
        mFakeAosDebug.updateSignalStrengthData(css, network);

        // THEN
        assertEquals("TEST_VALUE",
                mFakeAosDebug.mDebugData.get(IAosDebug.DebugKey.UTRAN_LEVEL));
        assertEquals("TEST_VALUE",
                mFakeAosDebug.mDebugData.get(IAosDebug.DebugKey.UTRAN_DBM));

        assertNotEquals("TEST_VALUE",
                mFakeAosDebug.mDebugData.get(IAosDebug.DebugKey.EUTRAN_RSRP));
        assertNotEquals("TEST_VALUE",
                mFakeAosDebug.mDebugData.get(IAosDebug.DebugKey.EUTRAN_RSRQ));

        assertEquals("TEST_VALUE",
                mFakeAosDebug.mDebugData.get(IAosDebug.DebugKey.NGRAN_SSRSRP));
        assertEquals("TEST_VALUE",
                mFakeAosDebug.mDebugData.get(IAosDebug.DebugKey.NGRAN_SSRSRQ));
    }

    @Test
    public void testUpdateSignalStrengthDataNgran() {
        // GIVEN
        mFakeAosDebug.mDebugData.put(IAosDebug.DebugKey.UTRAN_LEVEL, "TEST_VALUE");
        mFakeAosDebug.mDebugData.put(IAosDebug.DebugKey.UTRAN_DBM, "TEST_VALUE");
        mFakeAosDebug.mDebugData.put(IAosDebug.DebugKey.EUTRAN_RSRP, "TEST_VALUE");
        mFakeAosDebug.mDebugData.put(IAosDebug.DebugKey.EUTRAN_RSRQ, "TEST_VALUE");
        mFakeAosDebug.mDebugData.put(IAosDebug.DebugKey.NGRAN_SSRSRP, "TEST_VALUE");
        mFakeAosDebug.mDebugData.put(IAosDebug.DebugKey.NGRAN_SSRSRQ, "TEST_VALUE");

        CellSignalStrength css = new CellSignalStrengthNr();
        int network = AccessNetworkConstants.AccessNetworkType.NGRAN;

        // WHEN
        mFakeAosDebug.updateSignalStrengthData(css, network);

        // THEN
        assertEquals("TEST_VALUE",
                mFakeAosDebug.mDebugData.get(IAosDebug.DebugKey.UTRAN_LEVEL));
        assertEquals("TEST_VALUE",
                mFakeAosDebug.mDebugData.get(IAosDebug.DebugKey.UTRAN_DBM));

        assertEquals("TEST_VALUE",
                mFakeAosDebug.mDebugData.get(IAosDebug.DebugKey.EUTRAN_RSRP));
        assertEquals("TEST_VALUE",
                mFakeAosDebug.mDebugData.get(IAosDebug.DebugKey.EUTRAN_RSRQ));

        assertNotEquals("TEST_VALUE",
                mFakeAosDebug.mDebugData.get(IAosDebug.DebugKey.NGRAN_SSRSRP));
        assertNotEquals("TEST_VALUE",
                mFakeAosDebug.mDebugData.get(IAosDebug.DebugKey.NGRAN_SSRSRQ));
    }

    @Test
    public void testHandleWifiOnAvailable() {
        // GIVEN: The handler will see the network as connected.
        when(mMockConnectivityCallback.isConnected()).thenReturn(true);
        when(mMockConnectivityCallback.getNetworkCapabilities())
                .thenReturn(mock(NetworkCapabilities.class));
        when(mMockConnectivityCallback.getLinkProperties()).thenReturn(mock(LinkProperties.class));

        // A message simulating the onAvailable callback.
        Message msg = Message.obtain(mFakeAosDebug.mHandler,
                AosDebug.DEBUG_WIFI_CONNECTIVITY_CHANGED, mock(Network.class));

        // WHEN
        mFakeAosDebug.mHandler.handleWifiConnectivityChanged(msg);

        // THEN
        assertEquals(IAosDebug.DebugData.STR_CONNECTED,
                mFakeAosDebug.mDebugData.get(IAosDebug.DebugKey.WIFI_CONNECTION_STATE));
    }

    @Test
    public void testHandleWifiOnLost() {
        // GIVEN: The handler will see the network as disconnected.
        when(mMockConnectivityCallback.isConnected()).thenReturn(false);

        // A message simulating the onLost callback (obj is null).
        Message msg = Message.obtain(mFakeAosDebug.mHandler,
                AosDebug.DEBUG_WIFI_CONNECTIVITY_CHANGED, null);

        // WHEN
        mFakeAosDebug.mHandler.handleWifiConnectivityChanged(msg);

        // THEN
        assertEquals(IAosDebug.DebugData.STR_DISCONNECTED,
                mFakeAosDebug.mDebugData.get(IAosDebug.DebugKey.WIFI_CONNECTION_STATE));
    }

    @Test
    public void testHandleWifiOnLinkPropertiesChanged() {
        // GIVEN
        when(mMockConnectivityCallback.isConnected()).thenReturn(true);
        when(mMockConnectivityCallback.getNetworkCapabilities())
                .thenReturn(mock(NetworkCapabilities.class));

        LinkProperties linkProperties = new LinkProperties();
        linkProperties.setInterfaceName("test_wlan0");
        when(mMockConnectivityCallback.getLinkProperties()).thenReturn(linkProperties);

        Message msg = Message.obtain(mFakeAosDebug.mHandler,
                AosDebug.DEBUG_WIFI_CONNECTIVITY_CHANGED, mock(Network.class));

        // WHEN
        mFakeAosDebug.mHandler.handleWifiConnectivityChanged(msg);

        // THEN
        assertEquals("test_wlan0",
                mFakeAosDebug.mDebugData.get(IAosDebug.DebugKey.WIFI_INTERFACE_NAME));
    }

    @Test
    public void testHandleWifiOnCapabilitiesChanged() {
        // GIVEN
        when(mMockConnectivityCallback.isConnected()).thenReturn(true);
        when(mMockConnectivityCallback.getLinkProperties()).thenReturn(mock(LinkProperties.class));

        NetworkCapabilities capabilities = mock(NetworkCapabilities.class);
        when(mMockConnectivityCallback.getNetworkCapabilities()).thenReturn(capabilities);
        WifiInfo mockWifiInfo = mock(WifiInfo.class);
        when(mockWifiInfo.getSSID()).thenReturn("TestWifi-SSID");
        when(mockWifiInfo.getBSSID()).thenReturn("AA:BB:CC:DD:EE:FF");
        when(mockWifiInfo.getMacAddress()).thenReturn("FF:EE:DD:CC:BB:AA");
        doReturn(mockWifiInfo).when(mFakeAosDebug).getWifiInfo(capabilities);

        Message msg = Message.obtain(mFakeAosDebug.mHandler,
                AosDebug.DEBUG_WIFI_CONNECTIVITY_CHANGED, mock(Network.class));

        // WHEN
        mFakeAosDebug.mHandler.handleWifiConnectivityChanged(msg);

        // THEN
        assertEquals("TestWifi-SSID", mFakeAosDebug.mDebugData.get(IAosDebug.DebugKey.WIFI_SSID));
    }

    @Test
    public void testHandleMessageHandleWifiConnectivityChangedConnected() {
        // GIVEN
        when(mMockConnectivityCallback.isConnected()).thenReturn(true);

        Message msg = Message.obtain();
        msg.what = com.android.imsstack.enabler.aos.AosDebug.DEBUG_WIFI_CONNECTIVITY_CHANGED;
        msg.obj = new NetworkCapabilities();

        // WHEN
        mFakeAosDebug.mHandler.handleMessage(msg);

        // THEN
        assertEquals(IAosDebug.DebugData.STR_CONNECTED,
                mFakeAosDebug.mDebugData.get(IAosDebug.DebugKey.WIFI_CONNECTION_STATE));
    }

    @Test
    public void testHandleMessageHandleWifiConnectivityChangedDisConnected() {
        // GIVEN
        when(mMockConnectivityCallback.isConnected()).thenReturn(false);

        Message msg = Message.obtain();
        msg.what = com.android.imsstack.enabler.aos.AosDebug.DEBUG_WIFI_CONNECTIVITY_CHANGED;
        msg.obj = new NetworkCapabilities();

        // WHEN
        mFakeAosDebug.mHandler.handleMessage(msg);

        // THEN
        assertEquals(IAosDebug.DebugData.STR_DISCONNECTED,
                mFakeAosDebug.mDebugData.get(IAosDebug.DebugKey.WIFI_CONNECTION_STATE));
    }

    @Test
    public void testHandleMessageHandleWifiConnectivityChangedMessageIsNull() {
        // GIVEN
        when(mMockConnectivityCallback.isConnected()).thenReturn(false);

        Message msg = Message.obtain();
        msg.what = com.android.imsstack.enabler.aos.AosDebug.DEBUG_WIFI_CONNECTIVITY_CHANGED;
        msg.obj = null;

        // WHEN
        mFakeAosDebug.mHandler.handleMessage(msg);

        // THEN
        assertEquals(IAosDebug.DebugData.STR_DISCONNECTED,
                mFakeAosDebug.mDebugData.get(IAosDebug.DebugKey.WIFI_CONNECTION_STATE));
    }

    @Test
    public void testHandleMessageHandleWifiConnectivityChangedUpdateNetworkCapabilities() {
        // GIVEN
        when(mMockConnectivityCallback.isConnected()).thenReturn(true);

        NetworkCapabilities networkCapabilities = new NetworkCapabilities();
        when(mMockConnectivityCallback.getNetworkCapabilities()).thenReturn(networkCapabilities);

        WifiInfo mockWifiInfo = mock(WifiInfo.class);
        when(mockWifiInfo.getRssi()).thenReturn(100);
        when(mockWifiInfo.getBSSID()).thenReturn("test_bssid");
        when(mockWifiInfo.getSSID()).thenReturn("test_ssid");
        when(mockWifiInfo.getMacAddress()).thenReturn("test_address");
        mFakeAosDebug.mWifiInfo = mockWifiInfo;

        Message msg = Message.obtain();
        msg.what = com.android.imsstack.enabler.aos.AosDebug.DEBUG_WIFI_CONNECTIVITY_CHANGED;
        msg.obj = networkCapabilities;

        // WHEN
        mFakeAosDebug.mHandler.handleMessage(msg);

        // THEN
        assertTrue(mFakeAosDebug.mDebugData.get(IAosDebug.DebugKey.WIFI_RSSI)
                .contains("100"));
        assertTrue(mFakeAosDebug.mDebugData.get(IAosDebug.DebugKey.WIFI_BSSID)
                .contains("test_bssid"));
        assertTrue(mFakeAosDebug.mDebugData.get(IAosDebug.DebugKey.WIFI_SSID)
                .contains("test_ssid"));
        assertTrue(mFakeAosDebug.mDebugData.get(IAosDebug.DebugKey.WIFI_MAC_ADDRESS)
                .contains("test_address"));
    }

    @Test
    public void testHandleMessageHandleWifiConnectivityChangedNetworkCapabilitiesIsNull() {
        // GIVEN
        when(mMockConnectivityCallback.isConnected()).thenReturn(true);

        when(mMockConnectivityCallback.getNetworkCapabilities()).thenReturn(null);

        WifiInfo mockWifiInfo = mock(WifiInfo.class);
        when(mockWifiInfo.getRssi()).thenReturn(100);
        when(mockWifiInfo.getBSSID()).thenReturn("test_bssid");
        when(mockWifiInfo.getSSID()).thenReturn("test_ssid");
        when(mockWifiInfo.getMacAddress()).thenReturn("test_address");
        mFakeAosDebug.mWifiInfo = mockWifiInfo;

        Message msg = Message.obtain();
        msg.what = com.android.imsstack.enabler.aos.AosDebug.DEBUG_WIFI_CONNECTIVITY_CHANGED;
        msg.obj = null;

        // WHEN
        mFakeAosDebug.mHandler.handleMessage(msg);

        // THEN
        assertFalse(mFakeAosDebug.mDebugData.get(IAosDebug.DebugKey.WIFI_RSSI)
                .contains("100"));
        assertFalse(mFakeAosDebug.mDebugData.get(IAosDebug.DebugKey.WIFI_BSSID)
                .contains("test_bssid"));
        assertFalse(mFakeAosDebug.mDebugData.get(IAosDebug.DebugKey.WIFI_SSID)
                .contains("test_ssid"));
        assertFalse(mFakeAosDebug.mDebugData.get(IAosDebug.DebugKey.WIFI_MAC_ADDRESS)
                .contains("test_address"));
    }

    @Test
    public void testHandleMessageHandleServiceStateChanged() {
        // GIVEN

        // WHEN
        Message msg = Message.obtain();
        msg.what = com.android.imsstack.enabler.aos.AosDebug.DEBUG_SERVICE_STATE_CHANGED;
        msg.obj = mMockServiceState;
        mFakeAosDebug.mHandler.handleMessage(msg);

        // THEN
        assertEquals(0, mFakeAosDebug.getLatch(LatchType.ON_UPDATE_SERVICE_STATE).getCount());
        assertEquals(0, mFakeAosDebug.getLatch(LatchType.ON_UPDATE_VOICE_RAT).getCount());
        assertEquals(0, mFakeAosDebug.getLatch(LatchType.ON_UPDATE_DATA_REG_STATE).getCount());
        assertEquals(0, mFakeAosDebug.getLatch(LatchType.ON_UPDATE_CELLULAR_DATA_RAT).getCount());
        assertEquals(0, mFakeAosDebug.getLatch(LatchType.ON_UPDATE_NETWORK_OPERATOR).getCount());
        assertEquals(0, mFakeAosDebug.getLatch(LatchType.ON_UPDATE_OPERATOR_NUMERIC).getCount());
        assertEquals(0, mFakeAosDebug.getLatch(LatchType.ON_UPDATE_ROAMING_STATE).getCount());
        assertEquals(0, mFakeAosDebug.getLatch(LatchType.ON_UPDATE_VOICE_ROAMING_TYPE).getCount());
        assertEquals(0, mFakeAosDebug.getLatch(LatchType.ON_UPDATE_DATA_ROAMING_TYPE).getCount());
        assertEquals(0, mFakeAosDebug.getLatch(LatchType.ON_UPDATE_NETWORK_FEATURE).getCount());
    }

    @Test
    public void testUpdateServiceStateInService() {
        // GIVEN
        int serviceState = ServiceState.STATE_IN_SERVICE;

        // WHEN
        mFakeAosDebug.updateServiceState(serviceState);

        // THEN
        assertEquals("In Service",
                mFakeAosDebug.mDebugData.get(IAosDebug.DebugKey.SERVICE_STATE));
    }

    @Test
    public void testUpdateServiceStateOutOfService() {
        // GIVEN
        int serviceState = ServiceState.STATE_OUT_OF_SERVICE;

        // WHEN
        mFakeAosDebug.updateServiceState(serviceState);

        // THEN
        assertEquals("Out of Service",
                mFakeAosDebug.mDebugData.get(IAosDebug.DebugKey.SERVICE_STATE));
    }

    @Test
    public void testUpdateServiceStateEmergencyOnly() {
        // GIVEN
        int serviceState = ServiceState.STATE_EMERGENCY_ONLY;

        // WHEN
        mFakeAosDebug.updateServiceState(serviceState);

        // THEN
        assertEquals("Emergency call only",
                mFakeAosDebug.mDebugData.get(IAosDebug.DebugKey.SERVICE_STATE));
    }

    @Test
    public void testUpdateServiceStatePowerOff() {
        // GIVEN
        int serviceState = ServiceState.STATE_POWER_OFF;

        // WHEN
        mFakeAosDebug.updateServiceState(serviceState);

        // THEN
        assertEquals("Radio off",
                mFakeAosDebug.mDebugData.get(IAosDebug.DebugKey.SERVICE_STATE));
    }

    @Test
    public void testUpdateServiceStateInvalidState() {
        // GIVEN
        int serviceState = -1;

        // WHEN
        mFakeAosDebug.updateServiceState(serviceState);

        // THEN
        assertEquals(IAosDebug.DebugData.STR_EMPTY,
                mFakeAosDebug.mDebugData.get(IAosDebug.DebugKey.SERVICE_STATE));
    }

    @Test
    public void testUpdateVoiceRatNetworkRegistrationInfoIsNull() {
        // GIVEN
        mFakeAosDebug.mDebugData.put(IAosDebug.DebugKey.VOICE_RAT, "TEST_NETWORK");
        assertEquals("TEST_NETWORK",
                mFakeAosDebug.mDebugData.get(IAosDebug.DebugKey.VOICE_RAT));

        when(mMockServiceState.getNetworkRegistrationInfo(anyInt(), anyInt())).thenReturn(null);

        // WHEN
        mFakeAosDebug.updateVoiceRat(mMockServiceState);

        // THEN
        assertEquals(IAosDebug.DebugData.STR_EMPTY,
                mFakeAosDebug.mDebugData.get(IAosDebug.DebugKey.VOICE_RAT));
    }

    @Test
    public void testUpdateDataRegStateNetworkRegistrationInfoIsNull() {
        // GIVEN
        mFakeAosDebug.mDebugData.put(IAosDebug.DebugKey.DATA_REG_STATE, "In Service");

        when(mMockServiceState.getNetworkRegistrationInfo(anyInt(), anyInt())).thenReturn(null);

        // WHEN
        mFakeAosDebug.updateDataRegState(mMockServiceState);

        // THEN
        assertEquals(IAosDebug.DebugData.STR_EMPTY,
                mFakeAosDebug.mDebugData.get(IAosDebug.DebugKey.DATA_REG_STATE));
    }

    @Test
    public void testUpdateDataRegStateRadioOff() {
        // GIVEN
        mFakeAosDebug.mDebugData.put(IAosDebug.DebugKey.DATA_REG_STATE, "In Service");

        when(mMockServiceState.getNetworkRegistrationInfo(anyInt(), anyInt())).thenReturn(null);
        when(mMockServiceState.getState()).thenReturn(ServiceState.STATE_POWER_OFF);

        // WHEN
        mFakeAosDebug.updateDataRegState(mMockServiceState);

        // THEN
        assertTrue(mFakeAosDebug.mDebugData.get(IAosDebug.DebugKey.DATA_REG_STATE)
                .contains("Radio off"));
    }

    @Test
    public void testUpdateCellularDataRatWithTelephonyInterface() {
        // GIVEN
        when(mMockTelephonyInterface.getNetworkType())
                .thenReturn(TelephonyManager.NETWORK_TYPE_LTE_CA);

        // WHEN
        mFakeAosDebug.updateCellularDataRat();

        // THEN
        assertEquals(FakeAosDebug.getNetworkTypeToString(TelephonyManager.NETWORK_TYPE_LTE_CA),
                mFakeAosDebug.mDebugData.get(IAosDebug.DebugKey.CELLULAR_DATA_RAT));
    }

    @Test
    public void testUpdateCellularDataRatWithoutTelephonyInterface() {
        // GIVEN
        AgentFactory.getInstance().setAgent(TelephonyInterface.class, null, TestAppContext.SLOT0);

        // WHEN
        mFakeAosDebug.updateCellularDataRat();

        // THEN
        assertEquals(FakeAosDebug.getNetworkTypeToString(TelephonyManager.NETWORK_TYPE_UNKNOWN),
                mFakeAosDebug.mDebugData.get(IAosDebug.DebugKey.CELLULAR_DATA_RAT));
    }

    @Test
    public void testUpdateNetworkOperator() {
        // GIVEN
        when(mMockServiceState.getOperatorAlphaLong()).thenReturn("TEST_OPERATOR_ALPHA_LONG");

        // WHEN
        mFakeAosDebug.updateNetworkOperator(mMockServiceState);

        // THEN
        assertEquals("TEST_OPERATOR_ALPHA_LONG",
                mFakeAosDebug.mDebugData.get(IAosDebug.DebugKey.NETWORK_OPERATOR));
    }

    @Test
    public void testUpdateOperatorNumeric() {
        // GIVEN
        when(mMockServiceState.getOperatorNumeric()).thenReturn("TEST_OPERATOR_NUMERIC");

        // WHEN
        mFakeAosDebug.updateOperatorNumeric(mMockServiceState);

        // THEN
        assertEquals("TEST_OPERATOR_NUMERIC",
                mFakeAosDebug.mDebugData.get(IAosDebug.DebugKey.NETWORK_OPERATOR_NUMERIC));
    }

    @Test
    public void testUpdateRoamingStateIsRoaming() {
        // GIVEN
        when(mMockServiceState.getRoaming()).thenReturn(true);

        // WHEN
        mFakeAosDebug.updateRoamingState(mMockServiceState);

        // THEN
        assertEquals("Roaming",
                mFakeAosDebug.mDebugData.get(IAosDebug.DebugKey.ROAMING_STATE));
    }

    @Test
    public void testUpdateRoamingStateIsNotRoaming() {
        // GIVEN
        when(mMockServiceState.getRoaming()).thenReturn(false);

        // WHEN
        mFakeAosDebug.updateRoamingState(mMockServiceState);

        // THEN
        assertEquals("Not Roaming",
                mFakeAosDebug.mDebugData.get(IAosDebug.DebugKey.ROAMING_STATE));
    }

    @Test
    public void testUpdateVoiceRoamingTypeNetworkRegistrationInfoIsNull() {
        // GIVEN
        when(mMockServiceState.getNetworkRegistrationInfo(anyInt(), anyInt())).thenReturn(null);

        // WHEN
        mFakeAosDebug.updateVoiceRoamingType(mMockServiceState);

        // THEN
        assertEquals(FakeAosDebug.getRoamingTypeToString(ServiceState.ROAMING_TYPE_NOT_ROAMING),
                mFakeAosDebug.mDebugData.get(IAosDebug.DebugKey.VOICE_ROAMING_TYPE));
    }

    @Test
    public void testUpdateDataRoamingTypeNetworkRegistrationInfoIsNull() {
        // GIVEN
        when(mMockServiceState.getNetworkRegistrationInfo(anyInt(), anyInt())).thenReturn(null);

        // WHEN
        mFakeAosDebug.updateDataRoamingType(mMockServiceState);

        // THEN
        assertEquals(FakeAosDebug.getRoamingTypeToString(ServiceState.ROAMING_TYPE_NOT_ROAMING),
                mFakeAosDebug.mDebugData.get(IAosDebug.DebugKey.DATA_ROAMING_TYPE));
    }

    @Test
    public void testUpdateNetworkFeatureNetworkRegistrationInfoIsNull() {
        // GIVEN
        mFakeAosDebug.mDebugData.put(IAosDebug.DebugKey.NETWORK_SUPPORT_VOPS, "SUPPORT");
        mFakeAosDebug.mDebugData.put(IAosDebug.DebugKey.NETWORK_SUPPORT_EMCBS, "SUPPORT");
        mFakeAosDebug.mDebugData.put(IAosDebug.DebugKey.LTE_ATTACH_TYPE, "COMBINED");

        when(mMockServiceState.getNetworkRegistrationInfo(anyInt(), anyInt())).thenReturn(null);

        // WHEN
        mFakeAosDebug.updateNetworkFeature(mMockServiceState);

        // THEN
        assertEquals(IAosDebug.DebugData.STR_EMPTY,
                mFakeAosDebug.mDebugData.get(IAosDebug.DebugKey.NETWORK_SUPPORT_VOPS));
        assertEquals(IAosDebug.DebugData.STR_EMPTY,
                mFakeAosDebug.mDebugData.get(IAosDebug.DebugKey.NETWORK_SUPPORT_EMCBS));
        assertEquals(IAosDebug.DebugData.STR_EMPTY,
                mFakeAosDebug.mDebugData.get(IAosDebug.DebugKey.LTE_ATTACH_TYPE));
    }

    @Test
    public void testUpdatePreciseDataConnectionStateWithApnName() {
        // GIVEN
        when(mMockApnSetting.getApnName()).thenReturn("TEST_APN_NAME");

        // WHEN
        Message msg = Message.obtain();
        msg.what = com.android.imsstack.enabler.aos.AosDebug.DEBUG_PRECISE_DATA_CONNECTION_CHANGED;
        msg.obj = new PreciseDataConnectionState.Builder()
                .setApnSetting(mMockApnSetting)
                .build();
        mFakeAosDebug.mHandler.handleMessage(msg);

        // THEN
        assertEquals(mFakeAosDebug.mDebugData.get(IAosDebug.DebugKey.APN_NAME),
                "TEST_APN_NAME");
    }

    @Test
    public void testUpdatePreciseDataConnectionStateWithApnEntryName() {
        // GIVEN
        when(mMockApnSetting.getEntryName()).thenReturn("TEST_APN_ENTRY_NAME");

        // WHEN
        Message msg = Message.obtain();
        msg.what = com.android.imsstack.enabler.aos.AosDebug.DEBUG_PRECISE_DATA_CONNECTION_CHANGED;
        msg.obj = new PreciseDataConnectionState.Builder()
                .setApnSetting(mMockApnSetting)
                .build();
        mFakeAosDebug.mHandler.handleMessage(msg);

        // THEN
        assertEquals(mFakeAosDebug.mDebugData.get(IAosDebug.DebugKey.APN_ENTRY_NAME),
                "TEST_APN_ENTRY_NAME");
    }

    @Test
    public void testUpdatePreciseDataConnectionStateWithApnTypes() {
        // GIVEN
        int testApnTypeBitmask = ApnSetting.TYPE_IMS | ApnSetting.TYPE_EMERGENCY;
        when(mMockApnSetting.getApnTypeBitmask()).thenReturn(testApnTypeBitmask);

        // WHEN
        Message msg = Message.obtain();
        msg.what = com.android.imsstack.enabler.aos.AosDebug.DEBUG_PRECISE_DATA_CONNECTION_CHANGED;
        msg.obj = new PreciseDataConnectionState.Builder()
                .setApnSetting(mMockApnSetting)
                .build();
        mFakeAosDebug.mHandler.handleMessage(msg);

        // THEN
        String apnTypes = mFakeAosDebug.mDebugData.get(IAosDebug.DebugKey.APN_TYPES);
        assertTrue(apnTypes.contains("IMS"));
        assertTrue(apnTypes.contains("EMERGENCY"));
    }

    @Test
    public void testUpdatePreciseDataConnectionStateWithoutApnSetting() {
        // GIVEN
        // WHEN
        Message msg = Message.obtain();
        msg.what = com.android.imsstack.enabler.aos.AosDebug.DEBUG_PRECISE_DATA_CONNECTION_CHANGED;
        msg.obj = new PreciseDataConnectionState.Builder().build();
        mFakeAosDebug.mHandler.handleMessage(msg);

        // THEN
        assertEquals(mFakeAosDebug.mDebugData.get(IAosDebug.DebugKey.APN_NAME),
                IAosDebug.DebugData.STR_EMPTY);
        assertEquals(mFakeAosDebug.mDebugData.get(IAosDebug.DebugKey.APN_TYPES),
                IAosDebug.DebugData.STR_EMPTY);
        assertEquals(mFakeAosDebug.mDebugData.get(IAosDebug.DebugKey.APN_ENTRY_NAME),
                IAosDebug.DebugData.STR_EMPTY);
    }

    @Test
    public void testUpdatePreciseDataConnectionStateWithLinkProperties() {
        // GIVEN
        LinkProperties lp = new LinkProperties();
        lp.addLinkAddress(new LinkAddress("1.1.1.1/8"));
        lp.addLinkAddress(new LinkAddress("2001::1/63"));
        lp.setInterfaceName("TEST_INTERFACE_NAME");
        lp.setMtu(1500);

        // WHEN
        Message msg = Message.obtain();
        msg.what = com.android.imsstack.enabler.aos.AosDebug.DEBUG_PRECISE_DATA_CONNECTION_CHANGED;
        msg.obj = new PreciseDataConnectionState.Builder()
                .setLinkProperties(lp)
                .build();
        mFakeAosDebug.mHandler.handleMessage(msg);

        // THEN
        assertEquals(mFakeAosDebug.mDebugData.get(IAosDebug.DebugKey.IP_ADDRESSES),
                lp.getAddresses().toString());
        assertEquals(mFakeAosDebug.mDebugData.get(IAosDebug.DebugKey.INTERFACE_NAME),
                lp.getInterfaceName());
        assertEquals(mFakeAosDebug.mDebugData.get(IAosDebug.DebugKey.MTU),
                String.valueOf(lp.getMtu()));
    }

    @Test
    public void testUpdatePreciseDataConnectionStateWithoutLinkProperties() {
        // GIVEN
        // WHEN
        Message msg = Message.obtain();
        msg.what = com.android.imsstack.enabler.aos.AosDebug.DEBUG_PRECISE_DATA_CONNECTION_CHANGED;
        msg.obj = new PreciseDataConnectionState.Builder().build();
        mFakeAosDebug.mHandler.handleMessage(msg);

        // THEN
        assertEquals(mFakeAosDebug.mDebugData.get(IAosDebug.DebugKey.IP_ADDRESSES),
                IAosDebug.DebugData.STR_EMPTY);
        assertEquals(mFakeAosDebug.mDebugData.get(IAosDebug.DebugKey.INTERFACE_NAME),
                IAosDebug.DebugData.STR_EMPTY);
        assertEquals(mFakeAosDebug.mDebugData.get(IAosDebug.DebugKey.MTU),
                IAosDebug.DebugData.STR_EMPTY);
    }

    @Test
    public void testHandleNotifyRegisteredLte() {
        // GIVEN
        int networkType = IAosRegistrationListener.NetworkType.LTE.getValue();
        int featureTagBits = IAosRegistrationListener.FeatureTagMask.MMTEL;

        // WHEN
        Message msg = Message.obtain();
        msg.what = com.android.imsstack.enabler.aos.AosDebug.DEBUG_NOTIFY_REGISTERED;
        msg.arg1 = networkType;
        msg.arg2 = featureTagBits;
        mFakeAosDebug.mHandler.handleMessage(msg);

        // THEN
        assertEquals(IAosDebug.DebugData.STR_IMS_REGISTERED,
                mFakeAosDebug.mDebugData.get(IAosDebug.DebugKey.REGISTER));
        assertEquals(FakeAosDebug.sTestCurrentTime,
                mFakeAosDebug.mDebugData.get(IAosDebug.DebugKey.REGISTER_TIME));
        assertEquals(IAosRegistrationListener.NetworkType.of(networkType).toString(),
                mFakeAosDebug.mDebugData.get(IAosDebug.DebugKey.REGISTERED_NETWORK_TYPE));
        assertEquals(IAosRegistrationListener.FeatureTagMask.toString(featureTagBits),
                mFakeAosDebug.mDebugData.get(IAosDebug.DebugKey.FEATURES));
    }

    @Test
    public void testHandleNotifyRegisteredIwlan() {
        // GIVEN
        int networkType = IAosRegistrationListener.NetworkType.IWLAN.getValue();
        int featureTagBits = IAosRegistrationListener.FeatureTagMask.MMTEL;

        // WHEN
        Message msg = Message.obtain();
        msg.what = com.android.imsstack.enabler.aos.AosDebug.DEBUG_NOTIFY_REGISTERED;
        msg.arg1 = networkType;
        msg.arg2 = featureTagBits;
        mFakeAosDebug.mHandler.handleMessage(msg);

        // THEN
        assertEquals(IAosDebug.DebugData.STR_IMS_REGISTERED,
                mFakeAosDebug.mDebugData.get(IAosDebug.DebugKey.REGISTER));
        assertEquals(FakeAosDebug.sTestCurrentTime,
                mFakeAosDebug.mDebugData.get(IAosDebug.DebugKey.REGISTER_TIME));
        assertEquals(IAosRegistrationListener.NetworkType.of(networkType).toString(),
                mFakeAosDebug.mDebugData.get(IAosDebug.DebugKey.REGISTERED_NETWORK_TYPE));
        assertEquals(IAosRegistrationListener.FeatureTagMask.toString(featureTagBits),
                mFakeAosDebug.mDebugData.get(IAosDebug.DebugKey.FEATURES));
    }

    @Test
    public void testHandleNotifyRegisteredCrossSim() {
        // GIVEN
        int networkType = IAosRegistrationListener.NetworkType.CROSS_SIM.getValue();
        int featureTagBits = IAosRegistrationListener.FeatureTagMask.MMTEL;

        // WHEN
        Message msg = Message.obtain();
        msg.what = com.android.imsstack.enabler.aos.AosDebug.DEBUG_NOTIFY_REGISTERED;
        msg.arg1 = networkType;
        msg.arg2 = featureTagBits;
        mFakeAosDebug.mHandler.handleMessage(msg);

        // THEN
        assertEquals(IAosDebug.DebugData.STR_IMS_REGISTERED,
                mFakeAosDebug.mDebugData.get(IAosDebug.DebugKey.REGISTER));
        assertEquals(FakeAosDebug.sTestCurrentTime,
                mFakeAosDebug.mDebugData.get(IAosDebug.DebugKey.REGISTER_TIME));
        assertEquals(IAosRegistrationListener.NetworkType.of(networkType).toString(),
                mFakeAosDebug.mDebugData.get(IAosDebug.DebugKey.REGISTERED_NETWORK_TYPE));
        assertEquals(IAosRegistrationListener.FeatureTagMask.toString(featureTagBits),
                mFakeAosDebug.mDebugData.get(IAosDebug.DebugKey.FEATURES));
    }

    @Test
    public void testHandleNotifyRegisteredNr() {
        // GIVEN
        int networkType = IAosRegistrationListener.NetworkType.NR.getValue();
        int featureTagBits = IAosRegistrationListener.FeatureTagMask.MMTEL;

        // WHEN
        Message msg = Message.obtain();
        msg.what = com.android.imsstack.enabler.aos.AosDebug.DEBUG_NOTIFY_REGISTERED;
        msg.arg1 = networkType;
        msg.arg2 = featureTagBits;
        mFakeAosDebug.mHandler.handleMessage(msg);

        // THEN
        assertEquals(IAosDebug.DebugData.STR_IMS_REGISTERED,
                mFakeAosDebug.mDebugData.get(IAosDebug.DebugKey.REGISTER));
        assertEquals(FakeAosDebug.sTestCurrentTime,
                mFakeAosDebug.mDebugData.get(IAosDebug.DebugKey.REGISTER_TIME));
        assertEquals(IAosRegistrationListener.NetworkType.of(networkType).toString(),
                mFakeAosDebug.mDebugData.get(IAosDebug.DebugKey.REGISTERED_NETWORK_TYPE));
        assertEquals(IAosRegistrationListener.FeatureTagMask.toString(featureTagBits),
                mFakeAosDebug.mDebugData.get(IAosDebug.DebugKey.FEATURES));
    }

    @Test
    public void testHandleNotifyRegisteredCrossUtran() {
        // GIVEN
        int networkType = IAosRegistrationListener.NetworkType.UTRAN.getValue();
        int featureTagBits = IAosRegistrationListener.FeatureTagMask.MMTEL;

        // WHEN
        Message msg = Message.obtain();
        msg.what = com.android.imsstack.enabler.aos.AosDebug.DEBUG_NOTIFY_REGISTERED;
        msg.arg1 = networkType;
        msg.arg2 = featureTagBits;
        mFakeAosDebug.mHandler.handleMessage(msg);

        // THEN
        assertEquals(IAosDebug.DebugData.STR_IMS_REGISTERED,
                mFakeAosDebug.mDebugData.get(IAosDebug.DebugKey.REGISTER));
        assertEquals(FakeAosDebug.sTestCurrentTime,
                mFakeAosDebug.mDebugData.get(IAosDebug.DebugKey.REGISTER_TIME));
        assertEquals(IAosRegistrationListener.NetworkType.of(networkType).toString(),
                mFakeAosDebug.mDebugData.get(IAosDebug.DebugKey.REGISTERED_NETWORK_TYPE));
        assertEquals(IAosRegistrationListener.FeatureTagMask.toString(featureTagBits),
                mFakeAosDebug.mDebugData.get(IAosDebug.DebugKey.FEATURES));
    }

    @Test
    public void testHandleNotifyRegisteredCrossNone() {
        // GIVEN
        int networkType = IAosRegistrationListener.NetworkType.NONE.getValue();
        int featureTagBits = IAosRegistrationListener.FeatureTagMask.MMTEL;

        // WHEN
        Message msg = Message.obtain();
        msg.what = com.android.imsstack.enabler.aos.AosDebug.DEBUG_NOTIFY_REGISTERED;
        msg.arg1 = networkType;
        msg.arg2 = featureTagBits;
        mFakeAosDebug.mHandler.handleMessage(msg);

        // THEN
        assertEquals(IAosDebug.DebugData.STR_IMS_REGISTERED,
                mFakeAosDebug.mDebugData.get(IAosDebug.DebugKey.REGISTER));
        assertEquals(FakeAosDebug.sTestCurrentTime,
                mFakeAosDebug.mDebugData.get(IAosDebug.DebugKey.REGISTER_TIME));
        assertEquals(IAosRegistrationListener.NetworkType.of(networkType).toString(),
                mFakeAosDebug.mDebugData.get(IAosDebug.DebugKey.REGISTERED_NETWORK_TYPE));
        assertEquals(IAosRegistrationListener.FeatureTagMask.toString(featureTagBits),
                mFakeAosDebug.mDebugData.get(IAosDebug.DebugKey.FEATURES));
    }

    @Test
    public void testHandleNotifyRegisteredMultipleFeatureTags() {
        // GIVEN
        int networkType = IAosRegistrationListener.NetworkType.LTE.getValue();
        int featureTagBits = IAosRegistrationListener.FeatureTagMask.MMTEL
                | IAosRegistrationListener.FeatureTagMask.VIDEO
                | IAosRegistrationListener.FeatureTagMask.TEXT
                | IAosRegistrationListener.FeatureTagMask.USSI
                | IAosRegistrationListener.FeatureTagMask.VERSTAT
                | IAosRegistrationListener.FeatureTagMask.SMSIP
                | IAosRegistrationListener.FeatureTagMask.STANDALONE_MSG
                | IAosRegistrationListener.FeatureTagMask.CHAT_IM
                | IAosRegistrationListener.FeatureTagMask.CHAT_SESSION
                | IAosRegistrationListener.FeatureTagMask.FILE_TRANSFER
                | IAosRegistrationListener.FeatureTagMask.FILE_TRANSFER_VIA_SMS
                | IAosRegistrationListener.FeatureTagMask.CALL_COMPOSER_ENRICHED_CALLING
                | IAosRegistrationListener.FeatureTagMask.CALL_COMPOSER_VIA_TELEPHONY
                | IAosRegistrationListener.FeatureTagMask.POST_CALL
                | IAosRegistrationListener.FeatureTagMask.SHARED_MAP
                | IAosRegistrationListener.FeatureTagMask.SHARED_SKETCH
                | IAosRegistrationListener.FeatureTagMask.GEO_PUSH
                | IAosRegistrationListener.FeatureTagMask.GEO_PUSH_VIA_SMS
                | IAosRegistrationListener.FeatureTagMask.CHATBOT_COMMUNICATION_USING_SESSION
                | IAosRegistrationListener.FeatureTagMask
                        .CHATBOT_COMMUNICATION_USING_STANDALONE_MSG
                | IAosRegistrationListener.FeatureTagMask.CHATBOT_VERSION_SUPPORTED
                | IAosRegistrationListener.FeatureTagMask.CHATBOT_VERSION_V2_SUPPORTED
                | IAosRegistrationListener.FeatureTagMask.CHATBOT_ROLE
                | IAosRegistrationListener.FeatureTagMask.PRESENCE;


        // WHEN
        Message msg = Message.obtain();
        msg.what = com.android.imsstack.enabler.aos.AosDebug.DEBUG_NOTIFY_REGISTERED;
        msg.arg1 = networkType;
        msg.arg2 = featureTagBits;
        mFakeAosDebug.mHandler.handleMessage(msg);

        // THEN
        assertTrue(mFakeAosDebug.mDebugData.get(IAosDebug.DebugKey.FEATURES).contains(
                IAosRegistrationListener.FeatureTagMask.toString(featureTagBits)));
    }

    @Test
    public void testHandleNotifyDeregisteredUnspecified() {
        // GIVEN
        int networkType = IAosRegistrationListener.NetworkType.LTE.getValue();
        int reason = IAosRegistrationListener.ReasonCode.UNSPECIFIED.getValue();

        // WHEN
        Message msg = Message.obtain();
        msg.what = com.android.imsstack.enabler.aos.AosDebug.DEBUG_NOTIFY_DEREGISTERED;
        msg.arg1 = networkType;
        msg.arg2 = reason;
        mFakeAosDebug.mHandler.handleMessage(msg);

        // THEN
        assertEquals(IAosDebug.DebugData.STR_IMS_DEREGISTERED,
                mFakeAosDebug.mDebugData.get(IAosDebug.DebugKey.REGISTER));
        assertEquals(FakeAosDebug.sTestCurrentTime,
                mFakeAosDebug.mDebugData.get(IAosDebug.DebugKey.DEREGISTER_TIME));
        assertEquals(IAosRegistrationListener.FeatureTagMask
                        .toString(IAosRegistrationListener.FeatureTagMask.NONE),
                mFakeAosDebug.mDebugData.get(IAosDebug.DebugKey.FEATURES));
        assertEquals(IAosRegistrationListener.ReasonCode.of(reason).toString(),
                mFakeAosDebug.mDebugData.get(IAosDebug.DebugKey.DEREGISTER_REASON));
    }

    @Test
    public void testHandleNotifyDeregisteredPlmnBlock() {
        // GIVEN
        int networkType = IAosRegistrationListener.NetworkType.LTE.getValue();
        int reason = IAosRegistrationListener.ReasonCode.PLMN_BLOCK.getValue();

        // WHEN
        Message msg = Message.obtain();
        msg.what = com.android.imsstack.enabler.aos.AosDebug.DEBUG_NOTIFY_DEREGISTERED;
        msg.arg1 = networkType;
        msg.arg2 = reason;
        mFakeAosDebug.mHandler.handleMessage(msg);

        // THEN
        assertEquals(IAosDebug.DebugData.STR_IMS_DEREGISTERED,
                mFakeAosDebug.mDebugData.get(IAosDebug.DebugKey.REGISTER));
        assertEquals(FakeAosDebug.sTestCurrentTime,
                mFakeAosDebug.mDebugData.get(IAosDebug.DebugKey.DEREGISTER_TIME));
        assertEquals(IAosRegistrationListener.FeatureTagMask
                        .toString(IAosRegistrationListener.FeatureTagMask.NONE),
                mFakeAosDebug.mDebugData.get(IAosDebug.DebugKey.FEATURES));
        assertEquals(IAosRegistrationListener.ReasonCode.of(reason).toString(),
                mFakeAosDebug.mDebugData.get(IAosDebug.DebugKey.DEREGISTER_REASON));
    }

    @Test
    public void testHandleNotifyDeregisteredPlmnBlockWithTimeout() {
        // GIVEN
        int networkType = IAosRegistrationListener.NetworkType.LTE.getValue();
        int reason = IAosRegistrationListener.ReasonCode.PLMN_BLOCK_WITH_TIMEOUT.getValue();

        // WHEN
        Message msg = Message.obtain();
        msg.what = com.android.imsstack.enabler.aos.AosDebug.DEBUG_NOTIFY_DEREGISTERED;
        msg.arg1 = networkType;
        msg.arg2 = reason;
        mFakeAosDebug.mHandler.handleMessage(msg);

        // THEN
        assertEquals(IAosDebug.DebugData.STR_IMS_DEREGISTERED,
                mFakeAosDebug.mDebugData.get(IAosDebug.DebugKey.REGISTER));
        assertEquals(FakeAosDebug.sTestCurrentTime,
                mFakeAosDebug.mDebugData.get(IAosDebug.DebugKey.DEREGISTER_TIME));
        assertEquals(IAosRegistrationListener.FeatureTagMask
                        .toString(IAosRegistrationListener.FeatureTagMask.NONE),
                mFakeAosDebug.mDebugData.get(IAosDebug.DebugKey.FEATURES));
        assertEquals(IAosRegistrationListener.ReasonCode.of(reason).toString(),
                mFakeAosDebug.mDebugData.get(IAosDebug.DebugKey.DEREGISTER_REASON));
    }

    @Test
    public void testHandleNotifyDeregisteredRegistrationError() {
        // GIVEN
        int networkType = IAosRegistrationListener.NetworkType.LTE.getValue();
        int reason = IAosRegistrationListener.ReasonCode.REGISTRATION_ERROR.getValue();

        // WHEN
        Message msg = Message.obtain();
        msg.what = com.android.imsstack.enabler.aos.AosDebug.DEBUG_NOTIFY_DEREGISTERED;
        msg.arg1 = networkType;
        msg.arg2 = reason;
        mFakeAosDebug.mHandler.handleMessage(msg);

        // THEN
        assertEquals(IAosDebug.DebugData.STR_IMS_DEREGISTERED,
                mFakeAosDebug.mDebugData.get(IAosDebug.DebugKey.REGISTER));
        assertEquals(FakeAosDebug.sTestCurrentTime,
                mFakeAosDebug.mDebugData.get(IAosDebug.DebugKey.DEREGISTER_TIME));
        assertEquals(IAosRegistrationListener.FeatureTagMask
                        .toString(IAosRegistrationListener.FeatureTagMask.NONE),
                mFakeAosDebug.mDebugData.get(IAosDebug.DebugKey.FEATURES));
        assertEquals(IAosRegistrationListener.ReasonCode.of(reason).toString(),
                mFakeAosDebug.mDebugData.get(IAosDebug.DebugKey.DEREGISTER_REASON));
    }

    @Test
    public void testHandleNotifyDeregisteredRegistrationErrorWfcReg403() {
        // GIVEN
        int networkType = IAosRegistrationListener.NetworkType.LTE.getValue();
        int reason = IAosRegistrationListener.ReasonCode.WFC_REG_RESP_403.getValue();

        // WHEN
        Message msg = Message.obtain();
        msg.what = com.android.imsstack.enabler.aos.AosDebug.DEBUG_NOTIFY_DEREGISTERED;
        msg.arg1 = networkType;
        msg.arg2 = reason;
        mFakeAosDebug.mHandler.handleMessage(msg);

        // THEN
        assertEquals(IAosDebug.DebugData.STR_IMS_DEREGISTERED,
                mFakeAosDebug.mDebugData.get(IAosDebug.DebugKey.REGISTER));
        assertEquals(FakeAosDebug.sTestCurrentTime,
                mFakeAosDebug.mDebugData.get(IAosDebug.DebugKey.DEREGISTER_TIME));
        assertEquals(IAosRegistrationListener.FeatureTagMask
                        .toString(IAosRegistrationListener.FeatureTagMask.NONE),
                mFakeAosDebug.mDebugData.get(IAosDebug.DebugKey.FEATURES));
        assertEquals(IAosRegistrationListener.ReasonCode.of(reason).toString(),
                mFakeAosDebug.mDebugData.get(IAosDebug.DebugKey.DEREGISTER_REASON));
    }

    @Test
    public void testHandleNotifyDeregisteredRegistrationWfcReg500() {
        // GIVEN
        int networkType = IAosRegistrationListener.NetworkType.LTE.getValue();
        int reason = IAosRegistrationListener.ReasonCode.WFC_REG_RESP_500.getValue();

        // WHEN
        Message msg = Message.obtain();
        msg.what = com.android.imsstack.enabler.aos.AosDebug.DEBUG_NOTIFY_DEREGISTERED;
        msg.arg1 = networkType;
        msg.arg2 = reason;
        mFakeAosDebug.mHandler.handleMessage(msg);

        // THEN
        assertEquals(IAosDebug.DebugData.STR_IMS_DEREGISTERED,
                mFakeAosDebug.mDebugData.get(IAosDebug.DebugKey.REGISTER));
        assertEquals(FakeAosDebug.sTestCurrentTime,
                mFakeAosDebug.mDebugData.get(IAosDebug.DebugKey.DEREGISTER_TIME));
        assertEquals(IAosRegistrationListener.FeatureTagMask
                        .toString(IAosRegistrationListener.FeatureTagMask.NONE),
                mFakeAosDebug.mDebugData.get(IAosDebug.DebugKey.FEATURES));
        assertEquals(IAosRegistrationListener.ReasonCode.of(reason).toString(),
                mFakeAosDebug.mDebugData.get(IAosDebug.DebugKey.DEREGISTER_REASON));
    }

    @Test
    public void testHandleNotifyDeregisteredRegistrationErrorWfcNotSupportedCountry() {
        // GIVEN
        int networkType = IAosRegistrationListener.NetworkType.LTE.getValue();
        int reason = IAosRegistrationListener.ReasonCode
                .WFC_REG_RESP_403_NOT_SUPPORTED_COUNTRY.getValue();

        // WHEN
        Message msg = Message.obtain();
        msg.what = com.android.imsstack.enabler.aos.AosDebug.DEBUG_NOTIFY_DEREGISTERED;
        msg.arg1 = networkType;
        msg.arg2 = reason;
        mFakeAosDebug.mHandler.handleMessage(msg);

        // THEN
        assertEquals(IAosDebug.DebugData.STR_IMS_DEREGISTERED,
                mFakeAosDebug.mDebugData.get(IAosDebug.DebugKey.REGISTER));
        assertEquals(FakeAosDebug.sTestCurrentTime,
                mFakeAosDebug.mDebugData.get(IAosDebug.DebugKey.DEREGISTER_TIME));
        assertEquals(IAosRegistrationListener.FeatureTagMask
                        .toString(IAosRegistrationListener.FeatureTagMask.NONE),
                mFakeAosDebug.mDebugData.get(IAosDebug.DebugKey.FEATURES));
        assertEquals(IAosRegistrationListener.ReasonCode.of(reason).toString(),
                mFakeAosDebug.mDebugData.get(IAosDebug.DebugKey.DEREGISTER_REASON));
    }

    @Test
    public void testHandleNotifyDeregisteredRegistrationErrorWfcSub403() {
        // GIVEN
        int networkType = IAosRegistrationListener.NetworkType.LTE.getValue();
        int reason = IAosRegistrationListener.ReasonCode.WFC_SUB_RESP_403.getValue();

        // WHEN
        Message msg = Message.obtain();
        msg.what = com.android.imsstack.enabler.aos.AosDebug.DEBUG_NOTIFY_DEREGISTERED;
        msg.arg1 = networkType;
        msg.arg2 = reason;
        mFakeAosDebug.mHandler.handleMessage(msg);

        // THEN
        assertEquals(IAosDebug.DebugData.STR_IMS_DEREGISTERED,
                mFakeAosDebug.mDebugData.get(IAosDebug.DebugKey.REGISTER));
        assertEquals(FakeAosDebug.sTestCurrentTime,
                mFakeAosDebug.mDebugData.get(IAosDebug.DebugKey.DEREGISTER_TIME));
        assertEquals(IAosRegistrationListener.FeatureTagMask
                        .toString(IAosRegistrationListener.FeatureTagMask.NONE),
                mFakeAosDebug.mDebugData.get(IAosDebug.DebugKey.FEATURES));
        assertEquals(IAosRegistrationListener.ReasonCode.of(reason).toString(),
                mFakeAosDebug.mDebugData.get(IAosDebug.DebugKey.DEREGISTER_REASON));
    }

    @Test
    public void testHandleNotifyDeregisteredRegistrationErrorWfcNotifyTerminated() {
        // GIVEN
        int networkType = IAosRegistrationListener.NetworkType.LTE.getValue();
        int reason = IAosRegistrationListener.ReasonCode.WFC_SUB_NOTIFY_TERMINATED.getValue();

        // WHEN
        Message msg = Message.obtain();
        msg.what = com.android.imsstack.enabler.aos.AosDebug.DEBUG_NOTIFY_DEREGISTERED;
        msg.arg1 = networkType;
        msg.arg2 = reason;
        mFakeAosDebug.mHandler.handleMessage(msg);

        // THEN
        assertEquals(IAosDebug.DebugData.STR_IMS_DEREGISTERED,
                mFakeAosDebug.mDebugData.get(IAosDebug.DebugKey.REGISTER));
        assertEquals(FakeAosDebug.sTestCurrentTime,
                mFakeAosDebug.mDebugData.get(IAosDebug.DebugKey.DEREGISTER_TIME));
        assertEquals(IAosRegistrationListener.FeatureTagMask
                        .toString(IAosRegistrationListener.FeatureTagMask.NONE),
                mFakeAosDebug.mDebugData.get(IAosDebug.DebugKey.FEATURES));
        assertEquals(IAosRegistrationListener.ReasonCode.of(reason).toString(),
                mFakeAosDebug.mDebugData.get(IAosDebug.DebugKey.DEREGISTER_REASON));
    }

    @Test
    public void testHandleNotifyDeregisteredRegistrationErrorWfcOtherFailures() {
        // GIVEN
        int networkType = IAosRegistrationListener.NetworkType.LTE.getValue();
        int reason = IAosRegistrationListener.ReasonCode.WFC_REG_RESP_OTHER_FAILURES.getValue();

        // WHEN
        Message msg = Message.obtain();
        msg.what = com.android.imsstack.enabler.aos.AosDebug.DEBUG_NOTIFY_DEREGISTERED;
        msg.arg1 = networkType;
        msg.arg2 = reason;
        mFakeAosDebug.mHandler.handleMessage(msg);

        // THEN
        assertEquals(IAosDebug.DebugData.STR_IMS_DEREGISTERED,
                mFakeAosDebug.mDebugData.get(IAosDebug.DebugKey.REGISTER));
        assertEquals(FakeAosDebug.sTestCurrentTime,
                mFakeAosDebug.mDebugData.get(IAosDebug.DebugKey.DEREGISTER_TIME));
        assertEquals(IAosRegistrationListener.FeatureTagMask
                        .toString(IAosRegistrationListener.FeatureTagMask.NONE),
                mFakeAosDebug.mDebugData.get(IAosDebug.DebugKey.FEATURES));
        assertEquals(IAosRegistrationListener.ReasonCode.of(reason).toString(),
                mFakeAosDebug.mDebugData.get(IAosDebug.DebugKey.DEREGISTER_REASON));
    }

    @Test
    public void testHandleNotifyDeregisteredLocalPowerOff() {
        // GIVEN
        int networkType = IAosRegistrationListener.NetworkType.LTE.getValue();
        int reason = IAosRegistrationListener.ReasonCode.POWER_OFF.getValue();

        // WHEN
        Message msg = Message.obtain();
        msg.what = com.android.imsstack.enabler.aos.AosDebug.DEBUG_NOTIFY_DEREGISTERED;
        msg.arg1 = networkType;
        msg.arg2 = reason;
        mFakeAosDebug.mHandler.handleMessage(msg);

        // THEN
        assertEquals(IAosDebug.DebugData.STR_IMS_DEREGISTERED,
                mFakeAosDebug.mDebugData.get(IAosDebug.DebugKey.REGISTER));
        assertEquals(FakeAosDebug.sTestCurrentTime,
                mFakeAosDebug.mDebugData.get(IAosDebug.DebugKey.DEREGISTER_TIME));
        assertEquals(IAosRegistrationListener.FeatureTagMask
                        .toString(IAosRegistrationListener.FeatureTagMask.NONE),
                mFakeAosDebug.mDebugData.get(IAosDebug.DebugKey.FEATURES));
        assertEquals(IAosRegistrationListener.ReasonCode.of(reason).toString(),
                mFakeAosDebug.mDebugData.get(IAosDebug.DebugKey.DEREGISTER_REASON));
    }

    @Test
    public void testHandleNotifyDeregisteredLocalLowBattery() {
        // GIVEN
        int networkType = IAosRegistrationListener.NetworkType.LTE.getValue();
        int reason = IAosRegistrationListener.ReasonCode.LOW_BATTERY.getValue();

        // WHEN
        Message msg = Message.obtain();
        msg.what = com.android.imsstack.enabler.aos.AosDebug.DEBUG_NOTIFY_DEREGISTERED;
        msg.arg1 = networkType;
        msg.arg2 = reason;
        mFakeAosDebug.mHandler.handleMessage(msg);

        // THEN
        assertEquals(IAosDebug.DebugData.STR_IMS_DEREGISTERED,
                mFakeAosDebug.mDebugData.get(IAosDebug.DebugKey.REGISTER));
        assertEquals(FakeAosDebug.sTestCurrentTime,
                mFakeAosDebug.mDebugData.get(IAosDebug.DebugKey.DEREGISTER_TIME));
        assertEquals(IAosRegistrationListener.FeatureTagMask
                        .toString(IAosRegistrationListener.FeatureTagMask.NONE),
                mFakeAosDebug.mDebugData.get(IAosDebug.DebugKey.FEATURES));
        assertEquals(IAosRegistrationListener.ReasonCode.of(reason).toString(),
                mFakeAosDebug.mDebugData.get(IAosDebug.DebugKey.DEREGISTER_REASON));
    }

    @Test
    public void testHandleNotifyDeregisteredLocalNetworkNoService() {
        // GIVEN
        int networkType = IAosRegistrationListener.NetworkType.LTE.getValue();
        int reason = IAosRegistrationListener.ReasonCode.NETWORK_NO_SERVICE.getValue();

        // WHEN
        Message msg = Message.obtain();
        msg.what = com.android.imsstack.enabler.aos.AosDebug.DEBUG_NOTIFY_DEREGISTERED;
        msg.arg1 = networkType;
        msg.arg2 = reason;
        mFakeAosDebug.mHandler.handleMessage(msg);

        // THEN
        assertEquals(IAosDebug.DebugData.STR_IMS_DEREGISTERED,
                mFakeAosDebug.mDebugData.get(IAosDebug.DebugKey.REGISTER));
        assertEquals(FakeAosDebug.sTestCurrentTime,
                mFakeAosDebug.mDebugData.get(IAosDebug.DebugKey.DEREGISTER_TIME));
        assertEquals(IAosRegistrationListener.FeatureTagMask
                        .toString(IAosRegistrationListener.FeatureTagMask.NONE),
                mFakeAosDebug.mDebugData.get(IAosDebug.DebugKey.FEATURES));
        assertEquals(IAosRegistrationListener.ReasonCode.of(reason).toString(),
                mFakeAosDebug.mDebugData.get(IAosDebug.DebugKey.DEREGISTER_REASON));
    }

    @Test
    public void testHandleNotifyDeregisteredLocalNetworkNoLteCoverage() {
        // GIVEN
        int networkType = IAosRegistrationListener.NetworkType.LTE.getValue();
        int reason = IAosRegistrationListener.ReasonCode.NETWORK_NO_LTE_COVERAGE.getValue();

        // WHEN
        Message msg = Message.obtain();
        msg.what = com.android.imsstack.enabler.aos.AosDebug.DEBUG_NOTIFY_DEREGISTERED;
        msg.arg1 = networkType;
        msg.arg2 = reason;
        mFakeAosDebug.mHandler.handleMessage(msg);

        // THEN
        assertEquals(IAosDebug.DebugData.STR_IMS_DEREGISTERED,
                mFakeAosDebug.mDebugData.get(IAosDebug.DebugKey.REGISTER));
        assertEquals(FakeAosDebug.sTestCurrentTime,
                mFakeAosDebug.mDebugData.get(IAosDebug.DebugKey.DEREGISTER_TIME));
        assertEquals(IAosRegistrationListener.FeatureTagMask
                        .toString(IAosRegistrationListener.FeatureTagMask.NONE),
                mFakeAosDebug.mDebugData.get(IAosDebug.DebugKey.FEATURES));
        assertEquals(IAosRegistrationListener.ReasonCode.of(reason).toString(),
                mFakeAosDebug.mDebugData.get(IAosDebug.DebugKey.DEREGISTER_REASON));
    }

    @Test
    public void testHandleNotifyDeregisteredLocalNetworkRoaming() {
        // GIVEN
        int networkType = IAosRegistrationListener.NetworkType.LTE.getValue();
        int reason = IAosRegistrationListener.ReasonCode.NETWORK_ROAMING.getValue();

        // WHEN
        Message msg = Message.obtain();
        msg.what = com.android.imsstack.enabler.aos.AosDebug.DEBUG_NOTIFY_DEREGISTERED;
        msg.arg1 = networkType;
        msg.arg2 = reason;
        mFakeAosDebug.mHandler.handleMessage(msg);

        // THEN
        assertEquals(IAosDebug.DebugData.STR_IMS_DEREGISTERED,
                mFakeAosDebug.mDebugData.get(IAosDebug.DebugKey.REGISTER));
        assertEquals(FakeAosDebug.sTestCurrentTime,
                mFakeAosDebug.mDebugData.get(IAosDebug.DebugKey.DEREGISTER_TIME));
        assertEquals(IAosRegistrationListener.FeatureTagMask
                        .toString(IAosRegistrationListener.FeatureTagMask.NONE),
                mFakeAosDebug.mDebugData.get(IAosDebug.DebugKey.FEATURES));
        assertEquals(IAosRegistrationListener.ReasonCode.of(reason).toString(),
                mFakeAosDebug.mDebugData.get(IAosDebug.DebugKey.DEREGISTER_REASON));
    }

    @Test
    public void testHandleNotifyDeregisteredLocalNetworkIpChanged() {
        // GIVEN
        int networkType = IAosRegistrationListener.NetworkType.LTE.getValue();
        int reason = IAosRegistrationListener.ReasonCode.NETWORK_IP_CHANGED.getValue();

        // WHEN
        Message msg = Message.obtain();
        msg.what = com.android.imsstack.enabler.aos.AosDebug.DEBUG_NOTIFY_DEREGISTERED;
        msg.arg1 = networkType;
        msg.arg2 = reason;
        mFakeAosDebug.mHandler.handleMessage(msg);

        // THEN
        assertEquals(IAosDebug.DebugData.STR_IMS_DEREGISTERED,
                mFakeAosDebug.mDebugData.get(IAosDebug.DebugKey.REGISTER));
        assertEquals(FakeAosDebug.sTestCurrentTime,
                mFakeAosDebug.mDebugData.get(IAosDebug.DebugKey.DEREGISTER_TIME));
        assertEquals(IAosRegistrationListener.FeatureTagMask
                        .toString(IAosRegistrationListener.FeatureTagMask.NONE),
                mFakeAosDebug.mDebugData.get(IAosDebug.DebugKey.FEATURES));
        assertEquals(IAosRegistrationListener.ReasonCode.of(reason).toString(),
                mFakeAosDebug.mDebugData.get(IAosDebug.DebugKey.DEREGISTER_REASON));
    }

    @Test
    public void testHandleNotifyDeregisteredLocalServiceUnavailable() {
        // GIVEN
        int networkType = IAosRegistrationListener.NetworkType.LTE.getValue();
        int reason = IAosRegistrationListener.ReasonCode.SERVICE_UNAVAILABLE.getValue();

        // WHEN
        Message msg = Message.obtain();
        msg.what = com.android.imsstack.enabler.aos.AosDebug.DEBUG_NOTIFY_DEREGISTERED;
        msg.arg1 = networkType;
        msg.arg2 = reason;
        mFakeAosDebug.mHandler.handleMessage(msg);

        // THEN
        assertEquals(IAosDebug.DebugData.STR_IMS_DEREGISTERED,
                mFakeAosDebug.mDebugData.get(IAosDebug.DebugKey.REGISTER));
        assertEquals(FakeAosDebug.sTestCurrentTime,
                mFakeAosDebug.mDebugData.get(IAosDebug.DebugKey.DEREGISTER_TIME));
        assertEquals(IAosRegistrationListener.FeatureTagMask
                        .toString(IAosRegistrationListener.FeatureTagMask.NONE),
                mFakeAosDebug.mDebugData.get(IAosDebug.DebugKey.FEATURES));
        assertEquals(IAosRegistrationListener.ReasonCode.of(reason).toString(),
                mFakeAosDebug.mDebugData.get(IAosDebug.DebugKey.DEREGISTER_REASON));
    }

    @Test
    public void testHandleNotifyDeregisteredLocalNotRegistered() {
        // GIVEN
        int networkType = IAosRegistrationListener.NetworkType.LTE.getValue();
        int reason = IAosRegistrationListener.ReasonCode.NOT_REGISTERED.getValue();

        // WHEN
        Message msg = Message.obtain();
        msg.what = com.android.imsstack.enabler.aos.AosDebug.DEBUG_NOTIFY_DEREGISTERED;
        msg.arg1 = networkType;
        msg.arg2 = reason;
        mFakeAosDebug.mHandler.handleMessage(msg);

        // THEN
        assertEquals(IAosDebug.DebugData.STR_IMS_DEREGISTERED,
                mFakeAosDebug.mDebugData.get(IAosDebug.DebugKey.REGISTER));
        assertEquals(FakeAosDebug.sTestCurrentTime,
                mFakeAosDebug.mDebugData.get(IAosDebug.DebugKey.DEREGISTER_TIME));
        assertEquals(IAosRegistrationListener.FeatureTagMask
                        .toString(IAosRegistrationListener.FeatureTagMask.NONE),
                mFakeAosDebug.mDebugData.get(IAosDebug.DebugKey.FEATURES));
        assertEquals(IAosRegistrationListener.ReasonCode.of(reason).toString(),
                mFakeAosDebug.mDebugData.get(IAosDebug.DebugKey.DEREGISTER_REASON));
    }

    @Test
    public void testHandleNotifyDeregisteredUnknown() {
        // GIVEN
        int networkType = IAosRegistrationListener.NetworkType.LTE.getValue();
        int reason = -1;

        // WHEN
        Message msg = Message.obtain();
        msg.what = com.android.imsstack.enabler.aos.AosDebug.DEBUG_NOTIFY_DEREGISTERED;
        msg.arg1 = networkType;
        msg.arg2 = reason;
        mFakeAosDebug.mHandler.handleMessage(msg);

        // THEN
        assertEquals(IAosDebug.DebugData.STR_IMS_DEREGISTERED,
                mFakeAosDebug.mDebugData.get(IAosDebug.DebugKey.REGISTER));
        assertEquals(FakeAosDebug.sTestCurrentTime,
                mFakeAosDebug.mDebugData.get(IAosDebug.DebugKey.DEREGISTER_TIME));
        assertEquals(IAosRegistrationListener.FeatureTagMask
                        .toString(IAosRegistrationListener.FeatureTagMask.NONE),
                mFakeAosDebug.mDebugData.get(IAosDebug.DebugKey.FEATURES));
        assertEquals(IAosRegistrationListener.ReasonCode.of(reason).toString(),
                mFakeAosDebug.mDebugData.get(IAosDebug.DebugKey.DEREGISTER_REASON));
    }

    @Test
    public void testHandleNotifyCapabilitiesUpdated() {
        // GIVEN
        IAosRegistration.CapabilityPairs pairs = new IAosRegistration.CapabilityPairs(
                IAosRegistrationListener.NetworkType.LTE,
                IAosRegistrationListener.Capability.VOICE
                        | IAosRegistrationListener.Capability.VIDEO
                        | IAosRegistrationListener.Capability.UT
                        | IAosRegistrationListener.Capability.SMS
                        | IAosRegistrationListener.Capability.CALL_COMPOSER
                        | IAosRegistrationListener.Capability.OPTIONS_UCE
                        | IAosRegistrationListener.Capability.PRESENCE_UCE);

        // WHEN
        Message msg = Message.obtain();
        msg.what = com.android.imsstack.enabler.aos.AosDebug.DEBUG_NOTIFY_CAPABILITIES_UPDATED;
        msg.obj = pairs;
        mFakeAosDebug.mHandler.handleMessage(msg);

        // THEN
        assertTrue(mFakeAosDebug.mDebugData.get(IAosDebug.DebugKey.CAPABILITIES).contains(
                IAosRegistrationListener.Capability.toString(
                        IAosRegistrationListener.Capability.VOICE
                                | IAosRegistrationListener.Capability.VIDEO
                                | IAosRegistrationListener.Capability.UT
                                | IAosRegistrationListener.Capability.SMS
                                | IAosRegistrationListener.Capability.CALL_COMPOSER
                                | IAosRegistrationListener.Capability.OPTIONS_UCE
                                | IAosRegistrationListener.Capability.PRESENCE_UCE)));
    }

    @Test
    public void testHandleNotifyCapabilitiesUpdatedPairsIsNull() {
        // GIVEN

        // WHEN
        Message msg = Message.obtain();
        msg.what = com.android.imsstack.enabler.aos.AosDebug.DEBUG_NOTIFY_CAPABILITIES_UPDATED;
        msg.obj = null;
        mFakeAosDebug.mHandler.handleMessage(msg);

        // THEN
        assertEquals(IAosDebug.DebugData.STR_EMPTY,
                mFakeAosDebug.mDebugData.get(IAosDebug.DebugKey.CAPABILITIES));
    }

    @Test
    public void testGetNetworkTypeToStringGprs() {
        // GIVEN
        int type = TelephonyManager.NETWORK_TYPE_GPRS;

        // WHEN
        String result = FakeAosDebug.getNetworkTypeToString(type);

        // THEN
        assertTrue(result.contains("GPRS"));
    }

    @Test
    public void testGetNetworkTypeToStringEdge() {
        // GIVEN
        int type = TelephonyManager.NETWORK_TYPE_EDGE;

        // WHEN
        String result = FakeAosDebug.getNetworkTypeToString(type);

        // THEN
        assertTrue(result.contains("EDGE"));
    }

    @Test
    public void testGetNetworkTypeToStringUmts() {
        // GIVEN
        int type = TelephonyManager.NETWORK_TYPE_UMTS;

        // WHEN
        String result = FakeAosDebug.getNetworkTypeToString(type);

        // THEN
        assertTrue(result.contains("UMTS"));
    }

    @Test
    public void testGetNetworkTypeToStringHsdpa() {
        // GIVEN
        int type = TelephonyManager.NETWORK_TYPE_HSDPA;

        // WHEN
        String result = FakeAosDebug.getNetworkTypeToString(type);

        // THEN
        assertTrue(result.contains("HSDPA"));
    }

    @Test
    public void testGetNetworkTypeToStringHsupa() {
        // GIVEN
        int type = TelephonyManager.NETWORK_TYPE_HSUPA;

        // WHEN
        String result = FakeAosDebug.getNetworkTypeToString(type);

        // THEN
        assertTrue(result.contains("HSUPA"));
    }

    @Test
    public void testGetNetworkTypeToStringHspa() {
        // GIVEN
        int type = TelephonyManager.NETWORK_TYPE_HSPA;

        // WHEN
        String result = FakeAosDebug.getNetworkTypeToString(type);

        // THEN
        assertTrue(result.contains("HSPA"));
    }

    @Test
    public void testGetNetworkTypeToStringLte() {
        // GIVEN
        int type = TelephonyManager.NETWORK_TYPE_LTE;

        // WHEN
        String result = FakeAosDebug.getNetworkTypeToString(type);

        // THEN
        assertTrue(result.contains("LTE"));
    }

    @Test
    public void testGetNetworkTypeToStringHspap() {
        // GIVEN
        int type = TelephonyManager.NETWORK_TYPE_HSPAP;

        // WHEN
        String result = FakeAosDebug.getNetworkTypeToString(type);

        // THEN
        assertTrue(result.contains("HSPA+"));
    }

    @Test
    public void testGetNetworkTypeToStringGsm() {
        // GIVEN
        int type = TelephonyManager.NETWORK_TYPE_GSM;

        // WHEN
        String result = FakeAosDebug.getNetworkTypeToString(type);

        // THEN
        assertTrue(result.contains("GSM"));
    }

    @Test
    public void testGetNetworkTypeToStringScdma() {
        // GIVEN
        int type = TelephonyManager.NETWORK_TYPE_TD_SCDMA;

        // WHEN
        String result = FakeAosDebug.getNetworkTypeToString(type);

        // THEN
        assertTrue(result.contains("SCDMA"));
    }

    @Test
    public void testGetNetworkTypeToStringIwlan() {
        // GIVEN
        int type = TelephonyManager.NETWORK_TYPE_IWLAN;

        // WHEN
        String result = FakeAosDebug.getNetworkTypeToString(type);

        // THEN
        assertTrue(result.contains("IWLAN"));
    }

    @Test
    public void testGetNetworkTypeToStringLteCa() {
        // GIVEN
        int type = TelephonyManager.NETWORK_TYPE_LTE_CA;

        // WHEN
        String result = FakeAosDebug.getNetworkTypeToString(type);

        // THEN
        assertTrue(result.contains("LTE_CA"));
    }

    @Test
    public void testGetNetworkTypeToStringNr() {
        // GIVEN
        int type = TelephonyManager.NETWORK_TYPE_NR;

        // WHEN
        String result = FakeAosDebug.getNetworkTypeToString(type);

        // THEN
        assertTrue(result.contains("NR"));
    }

    @Test
    public void testGetNetworkTypeToStringUnknown() {
        // GIVEN
        int type = TelephonyManager.NETWORK_TYPE_UNKNOWN;

        // WHEN
        String result = FakeAosDebug.getNetworkTypeToString(type);

        // THEN
        assertTrue(result.contains(IAosDebug.DebugData.STR_EMPTY));
    }

    @Test
    public void testGetDataStateToStringDisconnected() {
        // GIVEN
        int state = TelephonyManager.DATA_DISCONNECTED;

        // WHEN
        String result = FakeAosDebug.getDataStateToString(state);

        // THEN
        assertTrue(result.contains("DISCONNECTED"));
    }

    @Test
    public void testGetDataStateToStringConnecting() {
        // GIVEN
        int state = TelephonyManager.DATA_CONNECTING;

        // WHEN
        String result = FakeAosDebug.getDataStateToString(state);

        // THEN
        assertTrue(result.contains("CONNECTING"));
    }

    @Test
    public void testGetDataStateToStringConnected() {
        // GIVEN
        int state = TelephonyManager.DATA_CONNECTED;

        // WHEN
        String result = FakeAosDebug.getDataStateToString(state);

        // THEN
        assertTrue(result.contains("CONNECTED"));
    }

    @Test
    public void testGetDataStateToStringSuspended() {
        // GIVEN
        int state = TelephonyManager.DATA_SUSPENDED;

        // WHEN
        String result = FakeAosDebug.getDataStateToString(state);

        // THEN
        assertTrue(result.contains("SUSPENDED"));
    }

    @Test
    public void testGetDataStateToStringDisconnecting() {
        // GIVEN
        int state = TelephonyManager.DATA_DISCONNECTING;

        // WHEN
        String result = FakeAosDebug.getDataStateToString(state);

        // THEN
        assertTrue(result.contains("DISCONNECTING"));
    }

    @Test
    public void testGetDataStateToStringHandoverInProgress() {
        // GIVEN
        int state = TelephonyManager.DATA_HANDOVER_IN_PROGRESS;

        // WHEN
        String result = FakeAosDebug.getDataStateToString(state);

        // THEN
        assertTrue(result.contains("HANDOVER"));
    }

    @Test
    public void testGetDataStateToStringUnknown() {
        // GIVEN
        int state = TelephonyManager.DATA_UNKNOWN;

        // WHEN
        String result = FakeAosDebug.getDataStateToString(state);

        // THEN
        assertTrue(result.contains(IAosDebug.DebugData.STR_EMPTY));
    }

    @Test
    public void testGetRoamingTypeToStringNotRoaming() {
        // GIVEN
        int type = ServiceState.ROAMING_TYPE_NOT_ROAMING;

        // WHEN
        String result = FakeAosDebug.getRoamingTypeToString(type);

        // THEN
        assertTrue(result.contains("Not Roaming"));
    }


    @Test
    public void testGetRoamingTypeToStringNotRoamingDomestic() {
        // GIVEN
        int type = ServiceState.ROAMING_TYPE_DOMESTIC;

        // WHEN
        String result = FakeAosDebug.getRoamingTypeToString(type);

        // THEN
        assertTrue(result.contains("Domestic"));
    }

    @Test
    public void testGetRoamingTypeToStringNotRoamingInternational() {
        // GIVEN
        int type = ServiceState.ROAMING_TYPE_INTERNATIONAL;

        // WHEN
        String result = FakeAosDebug.getRoamingTypeToString(type);

        // THEN
        assertTrue(result.contains("International"));
    }

    @Test
    public void testGetRoamingTypeToStringNotRoamingUnknown() {
        // GIVEN
        int type = ServiceState.ROAMING_TYPE_UNKNOWN;

        // WHEN
        String result = FakeAosDebug.getRoamingTypeToString(type);

        // THEN
        assertTrue(result.contains(IAosDebug.DebugData.STR_EMPTY));
    }

    @Test
    public void testGetAccessNetworkTypeNgran() {
        // GIVEN
        CellSignalStrength css = new CellSignalStrengthNr();

        // WHEN
        int type = mFakeAosDebug.getAccessNetworkType(css);

        // THEN
        assertEquals(AccessNetworkConstants.AccessNetworkType.NGRAN, type);
    }

    @Test
    public void testGetAccessNetworkTypeLte() {
        // GIVEN
        CellSignalStrength css = new CellSignalStrengthLte();

        // WHEN
        int type = mFakeAosDebug.getAccessNetworkType(css);

        // THEN
        assertEquals(AccessNetworkConstants.AccessNetworkType.EUTRAN, type);
    }

    @Test
    public void testHandleMessageGetAccessNetworkTypeWcdma() {
        // GIVEN
        CellSignalStrength css = new CellSignalStrengthWcdma();

        // WHEN
        int type = mFakeAosDebug.getAccessNetworkType(css);

        // THEN
        assertEquals(AccessNetworkConstants.AccessNetworkType.UTRAN, type);
    }

    @Test
    public void testGetAccessNetworkTypeGsm() {
        // GIVEN
        CellSignalStrength css = new CellSignalStrengthGsm();

        // WHEN
        int type = mFakeAosDebug.getAccessNetworkType(css);

        // THEN
        assertEquals(AccessNetworkConstants.AccessNetworkType.GERAN, type);
    }

    @Test
    public void testGetAccessNetworkTypeUnknown() {
        // GIVEN
        CellSignalStrength css = null;

        // WHEN
        int type = mFakeAosDebug.getAccessNetworkType(css);

        // THEN
        assertEquals(AccessNetworkConstants.AccessNetworkType.UNKNOWN, type);
    }

    public class FakeAosDebug extends AosDebug {

        boolean mIsDebugScreenEnabled = false;
        boolean mIsGranted = false;
        static String sTestCurrentTime = "9999-12-25 12:12:12";
        SimCarrierId mSimCarrierId;
        WifiInfo mWifiInfo;
        CountDownLatch[] mCountDownLatches;

        public enum LatchType {
            ON_SEND_NOTIFICATION,
            ON_REQUEST_PERMISSION,
            ON_HANDLE_PERMISSION_DENIED,

            ON_UPDATE_SERVICE_STATE,
            ON_UPDATE_VOICE_RAT,
            ON_UPDATE_DATA_REG_STATE,
            ON_UPDATE_CELLULAR_DATA_RAT,
            ON_UPDATE_NETWORK_OPERATOR,
            ON_UPDATE_OPERATOR_NUMERIC,
            ON_UPDATE_ROAMING_STATE,
            ON_UPDATE_VOICE_ROAMING_TYPE,
            ON_UPDATE_DATA_ROAMING_TYPE,
            ON_UPDATE_NETWORK_FEATURE,

            ON_UPDATE_SUBSCRIPTION,
            ON_UPDATE_CARRIER_INFO,
            ON_SELF_CHECK_DEBUG_NOTIFICATION,

            ON_UPDATE_SIGNAL_STRENGTH_DATA
        }

        public FakeAosDebug(int slotId) {
            super(slotId);

            int latchCount = LatchType.values().length;
            mCountDownLatches = new CountDownLatch[latchCount];
            for (int i = 0; i < latchCount; i++) {
                mCountDownLatches[i] = new CountDownLatch(1);
            }
        }

        public CountDownLatch getLatch(LatchType latchType) {
            int index = latchType.ordinal();
            if (index < mCountDownLatches.length) {
                return mCountDownLatches[index];
            }
            return null;
        }

        @Override
        protected boolean isDebugScreenEnabled() {
            return mIsDebugScreenEnabled;
        }

        @Override
        protected boolean checkPermission() {
            return mIsGranted;
        }

        @Override
        protected void sendNotification() {
            super.sendNotification();
            getLatch(LatchType.ON_SEND_NOTIFICATION).countDown();
        }

        @Override
        protected void requestPermission(Activity activity) {
            getLatch(LatchType.ON_REQUEST_PERMISSION).countDown();
        }

        @Override
        protected void handlePermissionDenied(Activity activity) {
            getLatch(LatchType.ON_HANDLE_PERMISSION_DENIED).countDown();
        }

        @Override
        protected void updateServiceState(int state) {
            super.updateServiceState(state);
            getLatch(LatchType.ON_UPDATE_SERVICE_STATE).countDown();
        }

        @Override
        protected void updateVoiceRat(ServiceState ss) {
            super.updateVoiceRat(ss);
            getLatch(LatchType.ON_UPDATE_VOICE_RAT).countDown();
        }

        @Override
        protected void updateDataRegState(ServiceState ss) {
            super.updateDataRegState(ss);
            getLatch(LatchType.ON_UPDATE_DATA_REG_STATE).countDown();
        }

        @Override
        protected void updateCellularDataRat() {
            super.updateCellularDataRat();
            getLatch(LatchType.ON_UPDATE_CELLULAR_DATA_RAT).countDown();
        }

        @Override
        protected void updateNetworkOperator(ServiceState ss) {
            super.updateNetworkOperator(ss);
            getLatch(LatchType.ON_UPDATE_NETWORK_OPERATOR).countDown();
        }

        @Override
        protected void updateOperatorNumeric(ServiceState ss) {
            super.updateOperatorNumeric(ss);
            getLatch(LatchType.ON_UPDATE_OPERATOR_NUMERIC).countDown();
        }

        @Override
        protected void updateRoamingState(ServiceState ss) {
            super.updateRoamingState(ss);
            getLatch(LatchType.ON_UPDATE_ROAMING_STATE).countDown();
        }

        @Override
        protected void updateVoiceRoamingType(ServiceState ss) {
            super.updateVoiceRoamingType(ss);
            getLatch(LatchType.ON_UPDATE_VOICE_ROAMING_TYPE).countDown();
        }

        @Override
        protected void updateDataRoamingType(ServiceState ss) {
            super.updateDataRoamingType(ss);
            getLatch(LatchType.ON_UPDATE_DATA_ROAMING_TYPE).countDown();
        }
        @Override
        protected void updateNetworkFeature(ServiceState ss) {
            super.updateNetworkFeature(ss);
            getLatch(LatchType.ON_UPDATE_NETWORK_FEATURE).countDown();
        }

        @Override
        protected void updateSubscription() {
            super.updateSubscription();
            getLatch(LatchType.ON_UPDATE_SUBSCRIPTION).countDown();
        }

        @Override
        protected void updateCarrierInfo() {
            super.updateCarrierInfo();
            getLatch(LatchType.ON_UPDATE_CARRIER_INFO).countDown();
        }

        @Override
        protected void selfCheckDebugNotification() {
            super.selfCheckDebugNotification();
            getLatch(LatchType.ON_SELF_CHECK_DEBUG_NOTIFICATION).countDown();
        }

        @Override
        protected void updateSignalStrengthData(CellSignalStrength cs, int network) {
            super.updateSignalStrengthData(cs, network);
            getLatch(LatchType.ON_UPDATE_SIGNAL_STRENGTH_DATA).countDown();
        }

        @Override
        protected NotificationManager getNotificationManager() {
            return mNotificationManager;
        }

        @Override
        protected SimCarrierId getCarrierId() {
            return mSimCarrierId;
        }

        @Override
        protected WifiInfo getWifiInfo(NetworkCapabilities capabilities) {
            return mWifiInfo;
        }

        @Override
        protected String getCurrentTime() {
            return sTestCurrentTime;
        }

        @Override
        protected SignalStrengthsListener createSignalStrengthsListener() {
            return mSignalStrengthsListener;
        }

        @Override
        protected ConnectivityCallback createConnectivityCallback() {
            return mConnectivityCallback;
        }

        @Override
        protected DebugImsPhoneStateListener createDebugImsPhoneStateListener() {
            return mImsPhoneStateListener;
        }
    }
}
