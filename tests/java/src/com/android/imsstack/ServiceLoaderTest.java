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

package com.android.imsstack;

import static org.junit.Assert.assertFalse;
import static org.junit.Assert.assertTrue;
import static org.mockito.ArgumentMatchers.any;
import static org.mockito.ArgumentMatchers.anyInt;
import static org.mockito.ArgumentMatchers.anyString;
import static org.mockito.ArgumentMatchers.eq;
import static org.mockito.Mockito.doAnswer;
import static org.mockito.Mockito.doReturn;
import static org.mockito.Mockito.verify;
import static org.mockito.Mockito.verifyNoMoreInteractions;
import static org.mockito.Mockito.when;

import android.content.Context;
import android.content.SharedPreferences;
import android.os.Looper;
import android.provider.Settings;
import android.telephony.SubscriptionManager;
import android.telephony.TelephonyManager;
import android.test.mock.MockContentResolver;

import androidx.test.filters.SmallTest;

import com.android.imsstack.base.AppContext;
import com.android.imsstack.core.agents.AgentFactory;
import com.android.imsstack.core.agents.ConfigAgent;
import com.android.imsstack.core.agents.ConfigInterface;
import com.android.imsstack.core.agents.dcm.DcFactory;
import com.android.imsstack.core.carrier.SimCarrierId;
import com.android.imsstack.internal.ImsStackRegistry;
import com.android.imsstack.jni.JniIms;
import com.android.imsstack.jni.JniImsProxy;
import com.android.imsstack.system.ISystem;
import com.android.imsstack.system.SystemInterface;
import com.android.internal.util.test.FakeSettingsProvider;

import org.junit.After;
import org.junit.Before;
import org.junit.Test;
import org.junit.runner.RunWith;
import org.junit.runners.JUnit4;
import org.mockito.Mock;
import org.mockito.Mockito;
import org.mockito.MockitoAnnotations;

@RunWith(JUnit4.class)
public class ServiceLoaderTest {
    private static final int SLOT0 = 0;
    private static final int[] SUB_ID = { 1 };

    @Mock private SharedPreferences mSp;
    @Mock private JniIms mJniIms;
    @Mock private ISystem mSystem;
    @Mock private SystemInterface mSystemInterface;

    private ContextFixture mContextFixture;
    private FakeSettingsProvider mSettingsProvider;
    private ServiceLoader mServiceLoader;

    @Before
    public void setUp() throws Exception {
        MockitoAnnotations.initMocks(this);

        if (Looper.myLooper() == null) {
            Looper.prepare();
        }

        mContextFixture = new ContextFixture();
        Context context = mContextFixture.getTestDouble();
        TelephonyManager tm = context.getSystemService(TelephonyManager.class);
        when(tm.getActiveModemCount()).thenReturn(1);
        when(tm.getSupportedModemCount()).thenReturn(1);
        when(tm.getSimApplicationState()).thenReturn(TelephonyManager.SIM_STATE_ABSENT);
        when(tm.getHalVersion(TelephonyManager.HAL_SERVICE_IMS)).thenReturn(
                TelephonyManager.HAL_VERSION_UNSUPPORTED);
        SubscriptionManager sm = context.getSystemService(SubscriptionManager.class);
        when(sm.getSubscriptionIds(eq(SLOT0))).thenReturn(SUB_ID);
        mSettingsProvider = new FakeSettingsProvider();
        ((MockContentResolver) context.getContentResolver()).addProvider(
                Settings.AUTHORITY, mSettingsProvider);
        setUpSharedPreferences(context);

        AppContext.init(context);
        JniImsProxy.setJniIms(mJniIms);
        ImsStackRegistry.setImsServiceState(SLOT0, false);
        when(mSystemInterface.getSystem(eq(SLOT0))).thenReturn(mSystem);
        SystemInterface.setSystemInterface(mSystemInterface);

        mServiceLoader = new ServiceLoader();
    }

    @After
    public void tearDown() throws Exception {
        mSp = null;
        mSettingsProvider = null;
        mServiceLoader = null;
        mContextFixture = null;
        FakeSettingsProvider.clearSettingsProvider();
        ImsStackRegistry.setImsServiceState(SLOT0, false);
        SystemInterface.setSystemInterface(null);
        JniImsProxy.setJniIms(null);
        DcFactory.clear(SLOT0);
        AgentFactory.getInstance().clear();
        AppContext.deinit();
    }

    @Test
    @SmallTest
    public void testInitJni() {
        mServiceLoader.initJni();

        verify(mJniIms).init();
        verify(mJniIms).sendCommand(anyInt(), anyInt(), any());

        mServiceLoader.initJni();

        verifyNoMoreInteractions(mJniIms);
    }

    @Test
    @SmallTest
    public void testInit() {
        assertFalse(mServiceLoader.isInitialized());

        mServiceLoader.init();

        assertTrue(mServiceLoader.isInitialized());
        verify(mJniIms).sendCommand(anyInt(), anyInt(), any());
        verify(mSystemInterface).init();

        mServiceLoader.init();

        verifyNoMoreInteractions(mJniIms);
    }

    @Test
    @SmallTest
    public void testStart() {
        mServiceLoader.start(SLOT0);

        verify(mSystemInterface).start(eq(SLOT0));
        assertTrue(ImsStackRegistry.isImsServiceStarted(SLOT0));
    }

    @Test
    @SmallTest
    public void testStop() {
        mServiceLoader.stop(SLOT0);

        verify(mSystemInterface).stop(eq(SLOT0));

        assertFalse(ImsStackRegistry.isImsServiceStarted(SLOT0));
    }

    @Test
    @SmallTest
    public void testUpdateCarrierConfig() {
        ConfigAgent configAgent = Mockito.mock(ConfigAgent.class);
        AgentFactory.getInstance().setAgent(ConfigInterface.class, configAgent, SLOT0);
        ServiceLoader.updateCarrierConfig(SLOT0);

        verify(configAgent).updateCarrierConfig(eq(SUB_ID[0]), any(SimCarrierId.class));
        verify(mSystem).notifyConfigurationChanged(anyInt());

        AgentFactory.getInstance().setAgent(ConfigInterface.class, null, SLOT0);
        ServiceLoader.updateCarrierConfig(SLOT0);

        verifyNoMoreInteractions(configAgent);
        verifyNoMoreInteractions(mSystem);
    }

    private void setUpSharedPreferences(Context context) {
        doAnswer(invocation -> {
            return (String) invocation.getArgument(1);
        }).when(mSp).getString(anyString(), anyString());
        doReturn(mSp).when(context).getSharedPreferences(anyString(), anyInt());
    }
}
