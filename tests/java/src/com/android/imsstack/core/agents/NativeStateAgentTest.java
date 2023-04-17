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
import static org.mockito.ArgumentMatchers.any;
import static org.mockito.ArgumentMatchers.anyInt;
import static org.mockito.ArgumentMatchers.eq;
import static org.mockito.Mockito.never;
import static org.mockito.Mockito.verify;
import static org.mockito.Mockito.verifyNoMoreInteractions;
import static org.mockito.Mockito.when;

import android.content.Context;
import android.telephony.TelephonyManager;
import android.test.suitebuilder.annotation.SmallTest;
import android.testing.TestableLooper;

import com.android.imsstack.ContextFixture;
import com.android.imsstack.core.agents.dcm.DcFactory;
import com.android.imsstack.core.agents.dcmif.IDc;
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

import java.util.HashMap;

@RunWith(JUnit4.class)
public class NativeStateAgentTest {
    private static final int SLOT0 = 0;

    @Mock IBatteryState mBatteryState;
    @Mock ICellInfo mCellInfo;
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
        TelephonyManager tm = context.getSystemService(TelephonyManager.class);
        when(tm.getSupportedModemCount()).thenReturn(1);
        AppContext.init(context);
        ServiceCaps.setServiceCapabilities(SLOT0, false, false, true);
        SystemInterface.setSystemInterface(mSystemInterface);
        when(mSystemInterface.getSystem(eq(SLOT0))).thenReturn(mSystem);
        AgentFactory.setDefaultAgent(AgentFactory.BATTERY_STATE, mBatteryState);
        AgentFactory.setAgentForMIms(mCellInfo, AgentFactory.CELL_INFO, SLOT0);
        replaceDcNetWatcher(mDcNetWatcher);
        mTestableLooper = new TestableLooper(AppContext.getInstance().getMainLooper());

        mNativeStateAgent = new NativeStateAgent(SLOT0);
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

        DcFactory.setObjects(SLOT0, null);
        AgentFactory.setAgentForMIms(null, AgentFactory.CELL_INFO, SLOT0);
        AgentFactory.setDefaultAgent(AgentFactory.BATTERY_STATE, null);
        SystemInterface.setSystemInterface(null);
        mCellInfo = null;
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

        verify(mCellInfo).stopTrackingCellInfo(any(Context.class));
        verify(mCellInfo).cleanup();
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
        verify(mCellInfo).init(any(Context.class));
        verify(mCellInfo).startTrackingCellInfo(any(Context.class));
        verify(mCellInfo).setLastCellInfoStorage(eq(true));
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
        verify(mCellInfo, never()).init(any());
        verify(mNativeStateListener).onNativeServiceReady();
    }

    private void replaceDcNetWatcher(IDcNetWatcher dcNetWatcher) {
        HashMap<Integer, IDc> dcObjects = DcFactory.getObjects(SLOT0);
        if (dcObjects == null) {
            dcObjects = new HashMap<>();
        }
        dcObjects.put(DcFactory.NETWORK_WATCHER, dcNetWatcher);
        DcFactory.setObjects(SLOT0, dcObjects);
    }

    private void processAllMessages() {
        while (!mTestableLooper.getLooper().getQueue().isIdle()) {
            mTestableLooper.processAllMessages();
        }
    }
}
