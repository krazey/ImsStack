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

package com.android.imsstack.enabler.aos;

import static com.android.imsstack.base.TestAppContext.SLOT0;
import static com.android.imsstack.base.TestAppContext.SUB_ID_1;

import static org.junit.Assert.assertFalse;
import static org.junit.Assert.assertTrue;
import static org.mockito.ArgumentMatchers.any;
import static org.mockito.ArgumentMatchers.eq;
import static org.mockito.Mockito.never;
import static org.mockito.Mockito.times;
import static org.mockito.Mockito.verify;
import static org.mockito.Mockito.when;

import android.content.BroadcastReceiver;
import android.content.Intent;
import android.content.IntentFilter;
import android.os.Handler;
import android.os.Looper;
import android.os.Message;
import android.telephony.TelephonyCallback;

import com.android.imsstack.ContextFixture;
import com.android.imsstack.base.TelephonyManagerProxy;
import com.android.imsstack.base.TestAppContext;
import com.android.imsstack.core.agents.AgentFactory;
import com.android.imsstack.core.agents.NativeStateInterface;
import com.android.imsstack.core.agents.Sim;
import com.android.imsstack.core.agents.SimInterface;
import com.android.imsstack.enabler.aos.service.AosService;

import org.junit.After;
import org.junit.Before;
import org.junit.Test;
import org.junit.runner.RunWith;
import org.junit.runners.JUnit4;
import org.mockito.ArgumentCaptor;
import org.mockito.Mock;
import org.mockito.Mockito;
import org.mockito.MockitoAnnotations;

import java.lang.reflect.Field;
import java.util.concurrent.Executor;

@RunWith(JUnit4.class)
public class AosSettingServiceTest {
    private TestAppContext mTestAppContext;
    private TelephonyManagerProxy mTelephonyManagerProxy;
    private AosSettingService mAosSettingService;

    @Mock SimInterface mMockSimInterface;
    @Mock NativeStateInterface mMockNativeStateInterface;

    @Before
    public void setup() throws Exception {
        if (Looper.myLooper() == null) {
            Looper.prepare();
        }

        MockitoAnnotations.initMocks(this);

        mTestAppContext = new TestAppContext(new ContextFixture().getTestDouble());
        mTestAppContext.setUp();

        mTelephonyManagerProxy = mTestAppContext.getSystemServiceProxy(TelephonyManagerProxy.class);
        AgentFactory.getInstance().setAgent(
                SimInterface.class, mMockSimInterface, SLOT0);
        AgentFactory.getInstance().setAgent(
                NativeStateInterface.class, mMockNativeStateInterface, SLOT0);
        when(mMockSimInterface.getSubId()).thenReturn(SUB_ID_1);

        mAosSettingService = new AosSettingService(SLOT0);
        mAosSettingService.init();
    }

    @After
    public void tearDown() throws Exception {
        mAosSettingService.cleanup();
        AgentFactory.getInstance().setAgent(NativeStateInterface.class, null, SLOT0);
        AgentFactory.getInstance().setAgent(SimInterface.class, null, SLOT0);
        AosFactory.getInstance().mAosServices.remove(SLOT0);
        mTestAppContext.tearDown();
        mTestAppContext = null;
    }

    @Test
    public void init_initializeNecessaryClasses() {
        // mAosSettingService.init() is called in setup()

        verify(mMockNativeStateInterface).addListener(any(NativeStateInterface.Listener.class));
        verify(mTestAppContext.getBroadcastReceiverProxy()).registerReceiver(
                eq(mAosSettingService.mIntentReceiverListener), any(IntentFilter.class));
        verify(mMockSimInterface).addListener(any(Sim.Listener.class));
        verify(mTelephonyManagerProxy).registerTelephonyCallback(
                any(Executor.class), eq(mAosSettingService.mUserMobileDataStateListener));
    }

    @Test
    public void cleanup_cleanUpResources() {
        TelephonyCallback callback = mAosSettingService.mUserMobileDataStateListener;
        BroadcastReceiver receiver = mAosSettingService.mIntentReceiverListener;
        Handler handler = mAosSettingService.mHandler;

        mAosSettingService.cleanup();

        verify(mTelephonyManagerProxy).unregisterTelephonyCallback(callback);
        verify(mMockSimInterface).removeListener(any(Sim.Listener.class));
        verify(mTestAppContext.getBroadcastReceiverProxy()).unregisterReceiver(receiver);
        verify(mMockNativeStateInterface).removeListener(any(NativeStateInterface.Listener.class));
    }

    @Test
    public void nativeStateListener_onNativeServiceReady() {
        AosService mockAosService = Mockito.mock(AosService.class);
        AosFactory.getInstance().mAosServices.put(TestAppContext.SLOT0, mockAosService);
        ArgumentCaptor<NativeStateInterface.Listener> listenerCaptor =
                ArgumentCaptor.forClass(NativeStateInterface.Listener.class);
        verify(mMockNativeStateInterface).addListener(listenerCaptor.capture());

        NativeStateInterface.Listener listener = listenerCaptor.getValue();
        listener.onNativeServiceReady();

        verify(mockAosService).notifyMobileDataSetting(false);
    }

    @Test
    public void settingServiceHandler_mobileDataStateNotChanged() {
        AosService mockAosService = Mockito.mock(AosService.class);
        AosFactory.getInstance().mAosServices.put(TestAppContext.SLOT0, mockAosService);

        Message msg = Message.obtain(mAosSettingService.mHandler,
                AosSettingService.EVENT_MOBILE_DATA_STATE_CHANGED, false);
        mAosSettingService.mHandler.handleMessage(msg);

        verify(mockAosService, never()).notifyMobileDataSetting(false);
    }

    @Test
    public void settingServiceHandler_mobileDataStateChanged() {
        AosService mockAosService = Mockito.mock(AosService.class);
        AosFactory.getInstance().mAosServices.put(TestAppContext.SLOT0, mockAosService);

        Message msg = Message.obtain(mAosSettingService.mHandler,
                AosSettingService.EVENT_MOBILE_DATA_STATE_CHANGED, true);
        mAosSettingService.mHandler.handleMessage(msg);

        verify(mockAosService).notifyMobileDataSetting(true);
    }

    @Test
    public void settingServiceHandler_shutDown() {
        AosService mockAosService = Mockito.mock(AosService.class);
        AosFactory.getInstance().mAosServices.put(TestAppContext.SLOT0, mockAosService);

        Message msg = Message.obtain(mAosSettingService.mHandler, AosSettingService.EVENT_SHUTDOWN);
        mAosSettingService.mHandler.handleMessage(msg);

        verify(mockAosService).notifyPowerOff();
    }

    @Test
    public void simListener_onSimStateChanged() {
        ArgumentCaptor<Sim.Listener> captor = ArgumentCaptor.forClass(Sim.Listener.class);
        verify(mMockSimInterface).addListener(captor.capture());
        Sim.Listener simListener = captor.getValue();

        AgentFactory.getInstance().setAgent(SimInterface.class, null, TestAppContext.SLOT0);
        // Ignored because SimInterface is null.
        simListener.onSimStateChanged();

        AgentFactory.getInstance().setAgent(
                SimInterface.class, mMockSimInterface, TestAppContext.SLOT0);
        when(mMockSimInterface.isSimLoadCompleted()).thenReturn(false);
        // Ignored because SIM state is not fully loaded.
        simListener.onSimStateChanged();

        // Same subscription
        when(mMockSimInterface.isSimLoadCompleted()).thenReturn(true);
        simListener.onSimStateChanged();

        when(mMockSimInterface.getSubId()).thenReturn(TestAppContext.SUB_ID_2);
        // Different subscription
        simListener.onSimStateChanged();

        verify(mTelephonyManagerProxy).unregisterTelephonyCallback(any(TelephonyCallback.class));
        verify(mTelephonyManagerProxy, times(2))
                .registerTelephonyCallback(any(Executor.class), any(TelephonyCallback.class));
    }

    @Test
    public void userMobileDataStateListener_onUserMobileDataStateChanged() {
        mAosSettingService.mUserMobileDataStateListener.onUserMobileDataStateChanged(true);

        assertTrue(mAosSettingService.mHandler
                .hasMessages(AosSettingService.EVENT_MOBILE_DATA_STATE_CHANGED));
    }

    @Test
    public void intentReceiverListener_actionReboot() {
        Intent intent = new Intent(Intent.ACTION_REBOOT);
        mAosSettingService.mIntentReceiverListener.onReceive(mTestAppContext.getContext(), intent);

        assertTrue(mAosSettingService.mHandler.hasMessages(AosSettingService.EVENT_REBOOT));
    }

    @Test
    public void intentReceiverListener_actionShutdown() {
        Intent intent = new Intent(Intent.ACTION_SHUTDOWN);
        mAosSettingService.mIntentReceiverListener.onReceive(mTestAppContext.getContext(), intent);

        assertTrue(mAosSettingService.mHandler.hasMessages(AosSettingService.EVENT_SHUTDOWN));
    }

    @Test
    public void intentReceiverListener_actionNotSupported() {
        Intent intent = new Intent(Intent.ACTION_AIRPLANE_MODE_CHANGED);
        mAosSettingService.mIntentReceiverListener.onReceive(mTestAppContext.getContext(), intent);

        assertFalse(mAosSettingService.mHandler.hasMessagesOrCallbacks());
    }

    @Test
    public void intentReceiverListener_nullIntent() {
        mAosSettingService.mIntentReceiverListener.onReceive(mTestAppContext.getContext(), null);

        assertFalse(mAosSettingService.mHandler.hasMessagesOrCallbacks());
    }

    private synchronized void replaceInstance(final Class c, final String instanceName,
            final Object obj, final Object newValue) throws Exception {
        Field field = c.getDeclaredField(instanceName);
        field.setAccessible(true);
        field.set(obj, newValue);
    }
}
