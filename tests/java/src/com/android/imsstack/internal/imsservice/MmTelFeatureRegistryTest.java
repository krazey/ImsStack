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
package com.android.imsstack.internal.imsservice;

import static org.junit.Assert.assertEquals;
import static org.junit.Assert.assertFalse;
import static org.junit.Assert.assertNotNull;
import static org.junit.Assert.assertTrue;
import static org.mockito.ArgumentMatchers.anyInt;
import static org.mockito.ArgumentMatchers.eq;
import static org.mockito.Mockito.doAnswer;
import static org.mockito.Mockito.never;
import static org.mockito.Mockito.times;
import static org.mockito.Mockito.verify;
import static org.mockito.Mockito.verifyNoMoreInteractions;
import static org.mockito.Mockito.when;

import android.content.Context;
import android.database.ContentObserver;
import android.net.Uri;
import android.provider.Settings;
import android.telecom.TelecomManager;
import android.telephony.SubscriptionManager;
import android.telephony.ims.ImsManager;
import android.telephony.ims.ImsMmTelManager;
import android.telephony.ims.ProvisioningManager;
import android.test.mock.MockContentResolver;
import android.test.suitebuilder.annotation.SmallTest;
import android.testing.TestableLooper;

import com.android.imsstack.ContextFixture;
import com.android.imsstack.util.AppContext;
import com.android.internal.util.test.FakeSettingsProvider;

import org.junit.After;
import org.junit.Before;
import org.junit.Test;
import org.junit.runner.RunWith;
import org.junit.runners.JUnit4;
import org.mockito.Mock;
import org.mockito.MockitoAnnotations;

@RunWith(JUnit4.class)
public class MmTelFeatureRegistryTest {
    private static final int SLOT0 = 0;
    private static final int[] SUB_ID = { 1 };

    @Mock ImsMmTelManager mMmTelManager;
    @Mock ProvisioningManager mProvisioningManager;
    @Mock MmTelFeatureRegistry.Listener mListener;
    private Context mContext;
    private ContextFixture mContextFixture;
    private FakeSettingsProvider mSettingsProvider;
    private ImsManager mImsManager;
    private SubscriptionManager mSubscriptionManager;
    private TestableLooper mTestableLooper;
    private MmTelFeatureRegistry mMmTelFeatureRegistry;

    @Before
    public void setUp() throws Exception {
        MockitoAnnotations.initMocks(this);
        FakeSettingsProvider.clearSettingsProvider();

        mContextFixture = new ContextFixture();
        mContext = mContextFixture.getTestDouble();
        AppContext.init(mContext);
        mSubscriptionManager = mContext.getSystemService(SubscriptionManager.class);
        when(mSubscriptionManager.getSubscriptionIds(eq(SLOT0))).thenReturn(SUB_ID);
        mImsManager = mContext.getSystemService(ImsManager.class);
        when(mImsManager.getImsMmTelManager(anyInt())).thenReturn(mMmTelManager);
        when(mImsManager.getProvisioningManager(anyInt())).thenReturn(mProvisioningManager);
        mSettingsProvider = new FakeSettingsProvider();
        ((MockContentResolver) mContext.getContentResolver()).addProvider(
                Settings.AUTHORITY, mSettingsProvider);
        mTestableLooper = new TestableLooper(AppContext.getInstance().getMainLooper());

        mMmTelFeatureRegistry = new MmTelFeatureRegistry(SLOT0);
    }

    @After
    public void tearDown() throws Exception {
        if (mTestableLooper != null) {
            mTestableLooper.destroy();
            mTestableLooper = null;
        }
        mProvisioningManager = null;
        mMmTelManager = null;
        mMmTelFeatureRegistry = null;
        AppContext.deinit();
        mContext = null;
        mContextFixture = null;
        FakeSettingsProvider.clearSettingsProvider();
    }

    @Test
    @SmallTest
    public void testSetTerminalBasedCallWaitingStatus() {
        mMmTelFeatureRegistry.setTerminalBasedCallWaitingStatus(true);
        assertTrue(mMmTelFeatureRegistry.isTerminalBasedCallWaitingEnabled());

        mMmTelFeatureRegistry.setTerminalBasedCallWaitingStatus(false);
        assertFalse(mMmTelFeatureRegistry.isTerminalBasedCallWaitingEnabled());
    }

    @Test
    @SmallTest
    public void testSetSrvccState() {
        mMmTelFeatureRegistry.setSrvccState(MmTelFeatureRegistry.SRVCC_STATE_STARTED);
        assertEquals(MmTelFeatureRegistry.SRVCC_STATE_STARTED,
                mMmTelFeatureRegistry.getSrvccState());

        mMmTelFeatureRegistry.setSrvccState(MmTelFeatureRegistry.SRVCC_STATE_COMPLETED);
        assertEquals(MmTelFeatureRegistry.SRVCC_STATE_NONE,
                mMmTelFeatureRegistry.getSrvccState());

        mMmTelFeatureRegistry.setSrvccState(MmTelFeatureRegistry.SRVCC_STATE_CANCELED);
        assertEquals(MmTelFeatureRegistry.SRVCC_STATE_NONE,
                mMmTelFeatureRegistry.getSrvccState());

        mMmTelFeatureRegistry.setSrvccState(MmTelFeatureRegistry.SRVCC_STATE_FAILED);
        assertEquals(MmTelFeatureRegistry.SRVCC_STATE_NONE,
                mMmTelFeatureRegistry.getSrvccState());

        mMmTelFeatureRegistry.setSrvccState(MmTelFeatureRegistry.SRVCC_STATE_NONE);
        assertEquals(MmTelFeatureRegistry.SRVCC_STATE_NONE,
                mMmTelFeatureRegistry.getSrvccState());
    }

    @Test
    @SmallTest
    public void testSetTtyMode() {
        assertEquals(TelecomManager.TTY_MODE_OFF, mMmTelFeatureRegistry.getTtyMode());

        mMmTelFeatureRegistry.setTtyMode(TelecomManager.TTY_MODE_FULL);
        assertEquals(TelecomManager.TTY_MODE_FULL, mMmTelFeatureRegistry.getTtyMode());

        mMmTelFeatureRegistry.setTtyMode(TelecomManager.TTY_MODE_HCO);
        assertEquals(TelecomManager.TTY_MODE_HCO, mMmTelFeatureRegistry.getTtyMode());

        mMmTelFeatureRegistry.setTtyMode(TelecomManager.TTY_MODE_VCO);
        assertEquals(TelecomManager.TTY_MODE_VCO, mMmTelFeatureRegistry.getTtyMode());
    }

    @Test
    @SmallTest
    public void testAddListener() {
        verify(mListener, never()).onTerminalBasedCallWaitingStatusChanged();
        verify(mListener, never()).onSrvccStateChanged(anyInt());

        mMmTelFeatureRegistry.addListener(mListener);
        mMmTelFeatureRegistry.setTerminalBasedCallWaitingStatus(true);
        mMmTelFeatureRegistry.setSrvccState(MmTelFeatureRegistry.SRVCC_STATE_STARTED);
        mMmTelFeatureRegistry.setTerminalBasedCallWaitingStatus(false);
        mMmTelFeatureRegistry.setSrvccState(MmTelFeatureRegistry.SRVCC_STATE_COMPLETED);

        verify(mListener, times(2)).onTerminalBasedCallWaitingStatusChanged();
        verify(mListener, times(2)).onSrvccStateChanged(anyInt());

        mMmTelFeatureRegistry.removeListener(mListener);
        mMmTelFeatureRegistry.setTerminalBasedCallWaitingStatus(true);
        mMmTelFeatureRegistry.setSrvccState(MmTelFeatureRegistry.SRVCC_STATE_STARTED);
        mMmTelFeatureRegistry.setTerminalBasedCallWaitingStatus(false);
        mMmTelFeatureRegistry.setSrvccState(MmTelFeatureRegistry.SRVCC_STATE_COMPLETED);

        verifyNoMoreInteractions(mListener);
    }

    @Test
    @SmallTest
    public void testAddListenerWhenSrvccStateChanged() {
        verify(mListener, never()).onSrvccStateChanged(anyInt());

        mMmTelFeatureRegistry.addListener(mListener);
        mMmTelFeatureRegistry.setSrvccState(MmTelFeatureRegistry.SRVCC_STATE_STARTED);
        mMmTelFeatureRegistry.setSrvccState(MmTelFeatureRegistry.SRVCC_STATE_COMPLETED);
        mMmTelFeatureRegistry.setSrvccState(MmTelFeatureRegistry.SRVCC_STATE_STARTED);
        mMmTelFeatureRegistry.setSrvccState(MmTelFeatureRegistry.SRVCC_STATE_CANCELED);
        mMmTelFeatureRegistry.setSrvccState(MmTelFeatureRegistry.SRVCC_STATE_STARTED);
        mMmTelFeatureRegistry.setSrvccState(MmTelFeatureRegistry.SRVCC_STATE_FAILED);

        verify(mListener, times(3))
                .onSrvccStateChanged(eq(MmTelFeatureRegistry.SRVCC_STATE_STARTED));
        verify(mListener).onSrvccStateChanged(eq(MmTelFeatureRegistry.SRVCC_STATE_COMPLETED));
        verify(mListener).onSrvccStateChanged(eq(MmTelFeatureRegistry.SRVCC_STATE_CANCELED));
        verify(mListener).onSrvccStateChanged(eq(MmTelFeatureRegistry.SRVCC_STATE_FAILED));

        mMmTelFeatureRegistry.removeListener(mListener);
        mMmTelFeatureRegistry.setSrvccState(MmTelFeatureRegistry.SRVCC_STATE_STARTED);
        mMmTelFeatureRegistry.setSrvccState(MmTelFeatureRegistry.SRVCC_STATE_COMPLETED);

        verifyNoMoreInteractions(mListener);
    }

    @Test
    @SmallTest
    public void testAddListenerWhenUserSettingsChanged() {
        verify(mListener, never()).onAdvancedCallingSettingChanged();
        verify(mListener, never()).onVtSettingChanged();
        verify(mListener, never()).onVoWiFiSettingChanged();
        verify(mListener, never()).onRttModeChanged();

        mMmTelFeatureRegistry.addListener(mListener);
        when(mMmTelManager.isAdvancedCallingSettingEnabled()).thenReturn(true);
        when(mMmTelManager.isVtSettingEnabled()).thenReturn(true);
        when(mMmTelManager.isVoWiFiSettingEnabled()).thenReturn(true);
        when(mMmTelManager.getVoWiFiModeSetting()).thenReturn(
                ImsMmTelManager.WIFI_MODE_CELLULAR_PREFERRED);
        when(mMmTelManager.isVoWiFiRoamingSettingEnabled()).thenReturn(true);
        when(mMmTelManager.getVoWiFiRoamingModeSetting()).thenReturn(
                ImsMmTelManager.WIFI_MODE_CELLULAR_PREFERRED);
        Settings.Secure.putInt(mContext.getContentResolver(),
                MmTelFeatureRegistry.RTT_MODE_SETTING, 1);
        mMmTelFeatureRegistry.reloadAllUserSettings();
        processAllMessages();

        verify(mListener).onAdvancedCallingSettingChanged();
        verify(mListener).onVtSettingChanged();
        verify(mListener, times(4)).onVoWiFiSettingChanged();
        verify(mListener).onRttModeChanged();

        mMmTelFeatureRegistry.removeListener(mListener);
        when(mMmTelManager.isAdvancedCallingSettingEnabled()).thenReturn(false);
        when(mMmTelManager.isVtSettingEnabled()).thenReturn(false);
        when(mMmTelManager.isVoWiFiSettingEnabled()).thenReturn(false);
        when(mMmTelManager.getVoWiFiModeSetting()).thenReturn(
                ImsMmTelManager.WIFI_MODE_WIFI_PREFERRED);
        when(mMmTelManager.isVoWiFiRoamingSettingEnabled()).thenReturn(false);
        when(mMmTelManager.getVoWiFiRoamingModeSetting()).thenReturn(
                ImsMmTelManager.WIFI_MODE_WIFI_PREFERRED);
        Settings.Secure.putInt(mContext.getContentResolver(),
                MmTelFeatureRegistry.RTT_MODE_SETTING, 2);
        mMmTelFeatureRegistry.reloadAllUserSettings();
        processAllMessages();

        verifyNoMoreInteractions(mListener);
    }

    @Test
    @SmallTest
    public void testInitUserSettings() {
        when(mMmTelManager.isAdvancedCallingSettingEnabled()).thenReturn(true, false);
        when(mMmTelManager.isVtSettingEnabled()).thenReturn(true, false);
        when(mMmTelManager.isVoWiFiSettingEnabled()).thenReturn(true, false);
        when(mMmTelManager.getVoWiFiModeSetting()).thenReturn(
                ImsMmTelManager.WIFI_MODE_CELLULAR_PREFERRED,
                ImsMmTelManager.WIFI_MODE_WIFI_PREFERRED);
        when(mMmTelManager.isVoWiFiRoamingSettingEnabled()).thenReturn(true, false);
        when(mMmTelManager.getVoWiFiRoamingModeSetting()).thenReturn(
                ImsMmTelManager.WIFI_MODE_CELLULAR_PREFERRED,
                ImsMmTelManager.WIFI_MODE_WIFI_PREFERRED);
        Settings.Secure.putInt(mContext.getContentResolver(),
                MmTelFeatureRegistry.RTT_MODE_SETTING, 1);
        mMmTelFeatureRegistry.initUserSettings();

        assertTrue(mMmTelFeatureRegistry.isAdvancedCallingSettingEnabled());
        assertTrue(mMmTelFeatureRegistry.isVtSettingEnabled());
        assertTrue(mMmTelFeatureRegistry.isVoWiFiSettingEnabled());
        assertEquals(ImsMmTelManager.WIFI_MODE_CELLULAR_PREFERRED,
                mMmTelFeatureRegistry.getVoWiFiModeSetting());
        assertTrue(mMmTelFeatureRegistry.isVoWiFiRoamingSettingEnabled());
        assertEquals(ImsMmTelManager.WIFI_MODE_CELLULAR_PREFERRED,
                mMmTelFeatureRegistry.getVoWiFiRoamingModeSetting());
        assertEquals(1, mMmTelFeatureRegistry.getRttMode());

        Settings.Secure.putInt(mContext.getContentResolver(),
                MmTelFeatureRegistry.RTT_MODE_SETTING, 0);
        mMmTelFeatureRegistry.initUserSettings();

        assertFalse(mMmTelFeatureRegistry.isAdvancedCallingSettingEnabled());
        assertFalse(mMmTelFeatureRegistry.isVtSettingEnabled());
        assertFalse(mMmTelFeatureRegistry.isVoWiFiSettingEnabled());
        assertEquals(ImsMmTelManager.WIFI_MODE_WIFI_PREFERRED,
                mMmTelFeatureRegistry.getVoWiFiModeSetting());
        assertFalse(mMmTelFeatureRegistry.isVoWiFiRoamingSettingEnabled());
        assertEquals(ImsMmTelManager.WIFI_MODE_WIFI_PREFERRED,
                mMmTelFeatureRegistry.getVoWiFiRoamingModeSetting());
        assertEquals(0, mMmTelFeatureRegistry.getRttMode());
    }

    @Test
    @SmallTest
    public void testGetUserSettings() {
        MmTelFeatureRegistry.UserSettings settings = mMmTelFeatureRegistry.getUserSettings();

        assertNotNull(settings);
        verify(mImsManager).getImsMmTelManager(eq(SUB_ID[0]));

        final int[] testSubId = { 2 };
        when(mSubscriptionManager.getSubscriptionIds(SLOT0)).thenReturn(testSubId);
        settings = mMmTelFeatureRegistry.getUserSettings();

        assertNotNull(settings);
        verify(mImsManager).getImsMmTelManager(eq(testSubId[0]));

        final int[] testInvalidSubId = { SubscriptionManager.INVALID_SUBSCRIPTION_ID };
        when(mSubscriptionManager.getSubscriptionIds(SLOT0)).thenReturn(testInvalidSubId);
        settings = mMmTelFeatureRegistry.getUserSettings();

        assertNotNull(settings);
        verifyNoMoreInteractions(mImsManager);
    }

    @Test
    @SmallTest
    public void testUserSettingsChanged() {
        MmTelFeatureRegistry.UserSettings settings = mMmTelFeatureRegistry.getUserSettings();

        assertNotNull(settings);

        ContentObserver observer = settings.getContentObserver();

        assertNotNull(observer);
        assertFalse(mMmTelFeatureRegistry.isAdvancedCallingSettingEnabled());
        assertFalse(mMmTelFeatureRegistry.isVtSettingEnabled());
        assertFalse(mMmTelFeatureRegistry.isVoWiFiSettingEnabled());
        assertEquals(ImsMmTelManager.WIFI_MODE_WIFI_PREFERRED,
                mMmTelFeatureRegistry.getVoWiFiModeSetting());
        assertFalse(mMmTelFeatureRegistry.isVoWiFiRoamingSettingEnabled());
        assertEquals(ImsMmTelManager.WIFI_MODE_WIFI_PREFERRED,
                mMmTelFeatureRegistry.getVoWiFiRoamingModeSetting());
        assertEquals(0, mMmTelFeatureRegistry.getRttMode());

        when(mMmTelManager.isAdvancedCallingSettingEnabled()).thenReturn(true);
        when(mMmTelManager.isVtSettingEnabled()).thenReturn(true);
        when(mMmTelManager.isVoWiFiSettingEnabled()).thenReturn(true);
        when(mMmTelManager.getVoWiFiModeSetting())
                .thenReturn(ImsMmTelManager.WIFI_MODE_CELLULAR_PREFERRED);
        when(mMmTelManager.isVoWiFiRoamingSettingEnabled()).thenReturn(true);
        when(mMmTelManager.getVoWiFiRoamingModeSetting())
                .thenReturn(ImsMmTelManager.WIFI_MODE_CELLULAR_PREFERRED);
        Settings.Secure.putInt(mContext.getContentResolver(),
                MmTelFeatureRegistry.RTT_MODE_SETTING, 1);
        observer.onChange(false,
                getUriFor(SubscriptionManager.ADVANCED_CALLING_ENABLED_CONTENT_URI, SUB_ID[0]));
        observer.onChange(false, getUriFor(SubscriptionManager.VT_ENABLED_CONTENT_URI, SUB_ID[0]));
        observer.onChange(false, getUriFor(SubscriptionManager.WFC_ENABLED_CONTENT_URI, SUB_ID[0]));
        observer.onChange(false, getUriFor(SubscriptionManager.WFC_MODE_CONTENT_URI, SUB_ID[0]));
        observer.onChange(false,
                getUriFor(SubscriptionManager.WFC_ROAMING_ENABLED_CONTENT_URI, SUB_ID[0]));
        observer.onChange(false,
                getUriFor(SubscriptionManager.WFC_ROAMING_MODE_CONTENT_URI, SUB_ID[0]));
        observer.onChange(false, Settings.Secure.getUriFor(mMmTelFeatureRegistry.RTT_MODE_SETTING));
        // Unknown URI.
        observer.onChange(false, Uri.parse("content://unknown"));

        assertTrue(mMmTelFeatureRegistry.isAdvancedCallingSettingEnabled());
        assertTrue(mMmTelFeatureRegistry.isVtSettingEnabled());
        assertTrue(mMmTelFeatureRegistry.isVoWiFiSettingEnabled());
        assertEquals(ImsMmTelManager.WIFI_MODE_CELLULAR_PREFERRED,
                mMmTelFeatureRegistry.getVoWiFiModeSetting());
        assertTrue(mMmTelFeatureRegistry.isVoWiFiRoamingSettingEnabled());
        assertEquals(ImsMmTelManager.WIFI_MODE_CELLULAR_PREFERRED,
                mMmTelFeatureRegistry.getVoWiFiRoamingModeSetting());
        assertEquals(1, mMmTelFeatureRegistry.getRttMode());
    }

    @Test
    @SmallTest
    public void testCheckUserSettingsWhenExceptionOrNull() {
        doAnswer((invocation) -> {
            throw new RuntimeException("isAdvancedCallingSettingEnabled failed.");
        }).when(mMmTelManager).isAdvancedCallingSettingEnabled();
        doAnswer((invocation) -> {
            throw new RuntimeException("isVtSettingEnabled failed.");
        }).when(mMmTelManager).isVtSettingEnabled();
        doAnswer((invocation) -> {
            throw new RuntimeException("isVoWiFiSettingEnabled failed.");
        }).when(mMmTelManager).isVoWiFiSettingEnabled();
        doAnswer((invocation) -> {
            throw new RuntimeException("getVoWiFiModeSetting failed.");
        }).when(mMmTelManager).getVoWiFiModeSetting();
        doAnswer((invocation) -> {
            throw new RuntimeException("isVoWiFiRoamingSettingEnabled failed.");
        }).when(mMmTelManager).isVoWiFiRoamingSettingEnabled();
        doAnswer((invocation) -> {
            throw new RuntimeException("getVoWiFiRoamingModeSetting failed.");
        }).when(mMmTelManager).getVoWiFiRoamingModeSetting();
        MmTelFeatureRegistry.UserSettings settings = mMmTelFeatureRegistry.getUserSettings();

        assertNotNull(settings);
        assertFalse(settings.isAdvancedCallingSettingEnabled());
        assertFalse(settings.isVtSettingEnabled());
        assertFalse(settings.isVoWiFiSettingEnabled());
        assertEquals(ImsMmTelManager.WIFI_MODE_WIFI_PREFERRED, settings.getVoWiFiModeSetting());
        assertFalse(settings.isVoWiFiRoamingSettingEnabled());
        assertEquals(ImsMmTelManager.WIFI_MODE_WIFI_PREFERRED,
                settings.getVoWiFiRoamingModeSetting());

        when(mImsManager.getImsMmTelManager(anyInt())).thenReturn(null);

        assertFalse(settings.isAdvancedCallingSettingEnabled());
        assertFalse(settings.isVtSettingEnabled());
        assertFalse(settings.isVoWiFiSettingEnabled());
        assertEquals(ImsMmTelManager.WIFI_MODE_WIFI_PREFERRED, settings.getVoWiFiModeSetting());
        assertFalse(settings.isVoWiFiRoamingSettingEnabled());
        assertEquals(ImsMmTelManager.WIFI_MODE_WIFI_PREFERRED,
                settings.getVoWiFiRoamingModeSetting());
    }

    private void processAllMessages() {
        while (!mTestableLooper.getLooper().getQueue().isIdle()) {
            mTestableLooper.processAllMessages();
        }
    }

    private Uri getUriFor(Uri uri, int subId) {
        return Uri.withAppendedPath(uri, String.valueOf(subId));
    }
}
