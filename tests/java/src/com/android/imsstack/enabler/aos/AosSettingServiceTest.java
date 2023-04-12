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

import static org.junit.Assert.assertFalse;
import static org.junit.Assert.assertTrue;
import static org.mockito.ArgumentMatchers.any;
import static org.mockito.Mockito.never;
import static org.mockito.Mockito.verify;
import static org.mockito.Mockito.when;

import android.content.BroadcastReceiver;
import android.content.Context;
import android.content.Intent;
import android.os.Handler;
import android.os.Looper;
import android.os.Message;
import android.telephony.SubscriptionManager;
import android.telephony.TelephonyCallback;
import android.telephony.TelephonyManager;

import com.android.imsstack.ContextFixture;
import com.android.imsstack.core.agents.AgentFactory;
import com.android.imsstack.core.agents.ISubscription;
import com.android.imsstack.core.agents.NativeStateInterface;
import com.android.imsstack.core.agents.SubscriptionListener;
import com.android.imsstack.enabler.aos.service.AosService;
import com.android.imsstack.util.AppContext;

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

@RunWith(JUnit4.class)
public class AosSettingServiceTest {
    private static final int MAX_SIM_SLOT = 1;
    private static final int SLOT_0 = 0;
    private static final int SLOT_1 = 1;
    private static final int SUB_ID_0 = 1;
    private static final int SUB_ID_1 = 2;
    private static final int[] SUB_IDS = { SUB_ID_0 };

    private Context mContext;
    private SubscriptionManager mSubscriptionManager;
    private TelephonyManager mTelephonyManager;
    private AosSettingService mAosSettingService;

    @Mock ISubscription mMockSubscription;
    @Mock NativeStateInterface mMockNativeStateInterface;

    @Before
    public void setup() throws Exception {
        if (Looper.myLooper() == null) {
            Looper.prepare();
        }

        MockitoAnnotations.initMocks(this);

        mContext = new ContextFixture().getTestDouble();
        AppContext.init(mContext);

        mSubscriptionManager = mContext.getSystemService(SubscriptionManager.class);
        when(mSubscriptionManager.getSubscriptionIds(SLOT_0)).thenReturn(SUB_IDS);

        mTelephonyManager = mContext.getSystemService(TelephonyManager.class);
        when(mTelephonyManager.createForSubscriptionId(SUB_ID_0)).thenReturn(mTelephonyManager);
        when(mTelephonyManager.getActiveModemCount()).thenReturn(MAX_SIM_SLOT);
        when(mTelephonyManager.getSupportedModemCount()).thenReturn(MAX_SIM_SLOT);

        AgentFactory.setDefaultAgent(AgentFactory.SUBSCRIPTION, mMockSubscription);
        AgentFactory.getInstance().setAgent(
                NativeStateInterface.class, mMockNativeStateInterface, SLOT_0);

        mAosSettingService = new FakeAosSettingService(SLOT_0);
        mAosSettingService.init();
    }

    @After
    public void tearDown() throws Exception {
        mAosSettingService.cleanup();
        AgentFactory.getInstance().setAgent(NativeStateInterface.class, null, SLOT_0);
        AgentFactory.setDefaultAgent(AgentFactory.SUBSCRIPTION, null);
        AppContext.deinit();
    }

    @Test
    public void init_initializeNecessaryClasses() {
        // mAosSettingService.init() is called in setup()

        verify(mMockNativeStateInterface).addListener(any(NativeStateInterface.Listener.class));
        verify(mContext).registerReceiver(mAosSettingService.mIntentReceiverListener,
                mAosSettingService.mIntentReceiverListener.getFilter(), Context.RECEIVER_EXPORTED);
        verify(mMockSubscription).addListener(mAosSettingService.mSubscriptionListener);
        verify(mTelephonyManager).registerTelephonyCallback(
                AppContext.getInstance().getMainExecutor(),
                mAosSettingService.mUserMobileDataStateListener);
    }

    @Test
    public void cleanup_cleanUpResources() {
        TelephonyCallback callback = mAosSettingService.mUserMobileDataStateListener;
        SubscriptionListener listener = mAosSettingService.mSubscriptionListener;
        BroadcastReceiver receiver = mAosSettingService.mIntentReceiverListener;
        Handler handler = mAosSettingService.mHandler;

        mAosSettingService.cleanup();

        verify(mTelephonyManager).unregisterTelephonyCallback(callback);
        verify(mMockSubscription).removeListener(listener);
        verify(mContext).unregisterReceiver(receiver);
        verify(mMockNativeStateInterface).removeListener(any(NativeStateInterface.Listener.class));
    }

    @Test
    public void nativeStateListener_onNativeServiceReady() {
        AosService mockAosService = Mockito.mock(AosService.class);
        AosFactory.getInstance().mAosServices.put(SLOT_0, mockAosService);
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
        AosFactory.getInstance().mAosServices.put(SLOT_0, mockAosService);

        Message msg = Message.obtain(mAosSettingService.mHandler,
                AosSettingService.EVENT_MOBILE_DATA_STATE_CHANGED, false);
        mAosSettingService.mHandler.handleMessage(msg);

        verify(mockAosService, never()).notifyMobileDataSetting(false);
    }

    @Test
    public void settingServiceHandler_mobileDataStateChanged() {
        AosService mockAosService = Mockito.mock(AosService.class);
        AosFactory.getInstance().mAosServices.put(SLOT_0, mockAosService);

        Message msg = Message.obtain(mAosSettingService.mHandler,
                AosSettingService.EVENT_MOBILE_DATA_STATE_CHANGED, true);
        mAosSettingService.mHandler.handleMessage(msg);

        verify(mockAosService).notifyMobileDataSetting(true);
    }

    @Test
    public void settingServiceHandler_shutDown() {
        AosService mockAosService = Mockito.mock(AosService.class);
        AosFactory.getInstance().mAosServices.put(SLOT_0, mockAosService);

        Message msg = Message.obtain(mAosSettingService.mHandler, AosSettingService.EVENT_SHUTDOWN);
        mAosSettingService.mHandler.handleMessage(msg);

        verify(mockAosService).notifyPowerOff();
    }

    @Test
    public void subscriptionListenerProxy_onSimLoadCompleted() {
        TelephonyCallback callback = mAosSettingService.mUserMobileDataStateListener;

        mAosSettingService.mSubscriptionListener.onSimLoadCompleted(SLOT_0);

        verify(mTelephonyManager, never()).unregisterTelephonyCallback(callback);
    }

    @Test
    public void subscriptionListenerProxy_onDefaultSubscriptionChanged() {
        TelephonyCallback oldCallback = mAosSettingService.mUserMobileDataStateListener;
        when(mMockSubscription.getSubId(SLOT_0)).thenReturn(SUB_ID_1);
        when(mTelephonyManager.createForSubscriptionId(SUB_ID_1)).thenReturn(mTelephonyManager);

        mAosSettingService.mSubscriptionListener.onDefaultSubscriptionChanged(SUB_ID_1);

        verify(mSubscriptionManager).getSubscriptionIds(SLOT_0);
        verify(mTelephonyManager).unregisterTelephonyCallback(oldCallback);
        verify(mTelephonyManager).createForSubscriptionId(SUB_ID_1);

        TelephonyCallback newCallBack = mAosSettingService.mUserMobileDataStateListener;
        verify(mTelephonyManager)
                .registerTelephonyCallback(AppContext.getInstance().getMainExecutor(), newCallBack);
    }

    @Test
    public void subscriptionListenerProxy_onDefaultDataSubscriptionChanged() {
        TelephonyCallback oldCallback = mAosSettingService.mUserMobileDataStateListener;
        when(mMockSubscription.getSubId(SLOT_0)).thenReturn(SUB_ID_1);
        when(mTelephonyManager.createForSubscriptionId(SUB_ID_1)).thenReturn(mTelephonyManager);

        mAosSettingService.mSubscriptionListener.onDefaultDataSubscriptionChanged(SUB_ID_1);

        verify(mSubscriptionManager).getSubscriptionIds(SLOT_0);
        verify(mTelephonyManager).unregisterTelephonyCallback(oldCallback);
        verify(mTelephonyManager).createForSubscriptionId(SUB_ID_1);

        TelephonyCallback newCallBack = mAosSettingService.mUserMobileDataStateListener;
        verify(mTelephonyManager)
                .registerTelephonyCallback(AppContext.getInstance().getMainExecutor(), newCallBack);
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
        mAosSettingService.mIntentReceiverListener.onReceive(mContext, intent);

        assertTrue(mAosSettingService.mHandler.hasMessages(AosSettingService.EVENT_REBOOT));
    }

    @Test
    public void intentReceiverListener_actionShutdown() {
        Intent intent = new Intent(Intent.ACTION_SHUTDOWN);
        mAosSettingService.mIntentReceiverListener.onReceive(mContext, intent);

        assertTrue(mAosSettingService.mHandler.hasMessages(AosSettingService.EVENT_SHUTDOWN));
    }

    @Test
    public void intentReceiverListener_actionNotSupported() {
        Intent intent = new Intent(Intent.ACTION_AIRPLANE_MODE_CHANGED);
        mAosSettingService.mIntentReceiverListener.onReceive(mContext, intent);

        assertFalse(mAosSettingService.mHandler.hasMessagesOrCallbacks());
    }

    @Test
    public void intentReceiverListener_nullIntent() {
        mAosSettingService.mIntentReceiverListener.onReceive(mContext, null);

        assertFalse(mAosSettingService.mHandler.hasMessagesOrCallbacks());
    }

    private synchronized void replaceInstance(final Class c, final String instanceName,
            final Object obj, final Object newValue) throws Exception {
        Field field = c.getDeclaredField(instanceName);
        field.setAccessible(true);
        field.set(obj, newValue);
    }

    private static class FakeAosSettingService extends AosSettingService {
        FakeAosSettingService(int slotId) {
            super(slotId);
        }

        @Override
        protected int getSlotId(int subId) {
            super.getSlotId(subId);
            if (subId == SUB_ID_0) {
                return SLOT_0;
            } else if (subId == SUB_ID_1) {
                return SLOT_1;
            }

            return SLOT_0;
        }
    }
}
