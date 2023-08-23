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
package com.android.imsstack.core.agents;

import static org.junit.Assert.assertFalse;
import static org.junit.Assert.assertTrue;
import static org.mockito.ArgumentMatchers.anyInt;
import static org.mockito.ArgumentMatchers.eq;
import static org.mockito.Mockito.never;
import static org.mockito.Mockito.verify;
import static org.mockito.Mockito.verifyNoMoreInteractions;
import static org.mockito.Mockito.when;

import android.content.Context;
import android.os.Looper;
import android.telephony.TelephonyManager;
import android.test.suitebuilder.annotation.SmallTest;
import android.testing.TestableLooper;

import com.android.imsstack.ContextFixture;
import com.android.imsstack.core.agents.dcm.DcFactory;
import com.android.imsstack.core.agents.dcmif.IDcNetWatcher;
import com.android.imsstack.core.config.ServiceCaps;
import com.android.imsstack.system.ISystem;
import com.android.imsstack.system.ImsEventDef;
import com.android.imsstack.system.SystemInterface;
import com.android.imsstack.util.AppContext;

import org.junit.After;
import org.junit.Before;
import org.junit.Test;
import org.junit.runner.RunWith;
import org.junit.runners.JUnit4;
import org.mockito.Mock;
import org.mockito.Mockito;
import org.mockito.MockitoAnnotations;

@RunWith(JUnit4.class)
public class NativeStateAgentTest {
    private static final int SLOT0 = 0;

    @Mock BatteryStateInterface mBatteryState;
    @Mock CellInfoInterface mCellInfoInterface;
    @Mock IDcNetWatcher mDcNetWatcher;
    @Mock ISystem mSystem;
    @Mock SystemInterface mSystemInterface;
    @Mock NativeStateInterface.Listener mNativeStateListener;

    private ContextFixture mContextFixture;
    private TestableLooper mTestableLooper;
    private NativeStateAgent mNativeStateAgent;

    @Before
    public void setUp() throws Exception {
        MockitoAnnotations.initMocks(this);

        mContextFixture = new ContextFixture();
        Context context = mContextFixture.getTestDouble();
        AppContext.init(context);
        TelephonyManager tm = context.getSystemService(TelephonyManager.class);
        when(tm.getSupportedModemCount()).thenReturn(1);
        ServiceCaps.setServiceCapabilities(SLOT0, false, false, true);
        SystemInterface.setSystemInterface(mSystemInterface);
        when(mSystemInterface.getSystem(eq(SLOT0))).thenReturn(mSystem);
        AgentFactory.getInstance().setAgent(BatteryStateInterface.class, mBatteryState);
        AgentFactory.getInstance().setAgent(CellInfoInterface.class, mCellInfoInterface, SLOT0);
        DcFactory.setDcAgent(IDcNetWatcher.class, mDcNetWatcher, SLOT0);
        mTestableLooper = new TestableLooper(Looper.getMainLooper());

        mNativeStateAgent = new NativeStateAgent(SLOT0, Looper.getMainLooper());
        mNativeStateAgent.init(context);
        mNativeStateAgent.addListener(mNativeStateListener);
    }

    @After
    public void tearDown() throws Exception {
        if (mNativeStateAgent != null) {
            mNativeStateAgent.removeListener(mNativeStateListener);
            mNativeStateAgent.cleanup();
            mNativeStateAgent = null;
        }

        if (mTestableLooper != null) {
            mTestableLooper.destroy();
            mTestableLooper = null;
        }

        DcFactory.setDcAgent(IDcNetWatcher.class, null, SLOT0);
        AgentFactory.getInstance().setAgent(CellInfoInterface.class, null, SLOT0);
        AgentFactory.getInstance().setAgent(BatteryStateInterface.class, null);
        SystemInterface.setSystemInterface(null);
        mCellInfoInterface = null;
        mBatteryState = null;
        mSystem = null;
        mSystemInterface = null;
        mNativeStateListener = null;
        mContextFixture = null;
        AppContext.deinit();
        ServiceCaps.clear();
    }

    @Test
    @SmallTest
    public void testInit() {
        assertFalse(mNativeStateAgent.isServiceReady());
    }

    @Test
    @SmallTest
    public void testCleanup() {
        mNativeStateAgent.cleanup();

        verify(mCellInfoInterface).stopTrackingCellInfo();
    }

    @Test
    @SmallTest
    public void testAddListenerWhenServiceReady() {
        mNativeStateAgent.updateServiceReady(true);
        processAllMessages();

        assertTrue(mNativeStateAgent.isServiceReady());

        NativeStateInterface.Listener listener = Mockito.mock(NativeStateInterface.Listener.class);
        mNativeStateAgent.addListener(listener);
        processAllMessages();

        verify(mNativeStateListener).onNativeServiceReady();
        verify(listener).onNativeServiceReady();
    }

    @Test
    @SmallTest
    public void testUpdateServiceReady() {
        assertFalse(mNativeStateAgent.isServiceReady());

        mNativeStateAgent.updateServiceReady(true);
        processAllMessages();

        assertTrue(mNativeStateAgent.isServiceReady());
        verify(mBatteryState).notifyLowBatteryState(eq(SLOT0));
        verify(mSystem).notifyEvent(
                eq(ImsEventDef.IMS_EVENT_IMS_VOICE_OVER_PS_STATE), anyInt(), anyInt());
        verify(mSystem).notifyEvent(
                eq(ImsEventDef.IMS_EVENT_VOLTE_SETTING), anyInt(), anyInt());
        verify(mSystem).notifyEvent(
                eq(ImsEventDef.IMS_EVENT_WFC_SETTING_CHANGED), anyInt(), anyInt());
        verify(mSystem).notifyEvent(
                eq(ImsEventDef.IMS_EVENT_RTT_SETTING), anyInt(), anyInt());
        verify(mCellInfoInterface).startTrackingCellInfo();
        verify(mNativeStateListener).onNativeServiceReady();

        mNativeStateAgent.removeListener(mNativeStateListener);
        mNativeStateAgent.updateServiceReady(false);
        processAllMessages();

        verifyNoMoreInteractions(mNativeStateListener);
    }

    @Test
    @SmallTest
    public void testUpdateServiceReadyWhenServiceNotReady() {
        assertFalse(mNativeStateAgent.isServiceReady());

        mNativeStateAgent.updateServiceReady(false);
        processAllMessages();

        assertFalse(mNativeStateAgent.isServiceReady());
        verify(mBatteryState, never()).notifyLowBatteryState(anyInt());
        verify(mSystem, never()).notifyEvent(anyInt(), anyInt(), anyInt());
        verify(mNativeStateListener).onNativeServiceReady();
    }

    private void processAllMessages() {
        while (!mTestableLooper.getLooper().getQueue().isIdle()) {
            mTestableLooper.processAllMessages();
        }
    }
}
