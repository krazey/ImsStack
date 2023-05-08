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

import static org.junit.Assert.assertEquals;
import static org.junit.Assert.assertNotNull;
import static org.mockito.ArgumentMatchers.any;
import static org.mockito.ArgumentMatchers.anyInt;
import static org.mockito.ArgumentMatchers.eq;
import static org.mockito.Mockito.never;
import static org.mockito.Mockito.times;
import static org.mockito.Mockito.verify;
import static org.mockito.Mockito.when;

import android.content.Context;
import android.os.Looper;
import android.telephony.AccessNetworkConstants;
import android.telephony.BarringInfo;
import android.telephony.CellInfo;
import android.telephony.DisconnectCause;
import android.telephony.NetworkRegistrationInfo;
import android.telephony.PreciseCallState;
import android.telephony.PreciseDataConnectionState;
import android.telephony.PreciseDisconnectCause;
import android.telephony.ServiceState;
import android.telephony.SignalStrength;
import android.telephony.SubscriptionManager;
import android.telephony.TelephonyCallback;
import android.telephony.TelephonyManager;
import android.test.suitebuilder.annotation.SmallTest;

import com.android.imsstack.ContextFixture;
import com.android.imsstack.core.agents.internal.PhoneStateNotifier;
import com.android.imsstack.util.AppContext;
import com.android.imsstack.util.ImsLog;
import com.android.imsstack.util.MSimUtils;

import org.junit.After;
import org.junit.Before;
import org.junit.Test;
import org.junit.runner.RunWith;
import org.junit.runners.JUnit4;
import org.mockito.ArgumentCaptor;
import org.mockito.Mock;
import org.mockito.Mockito;
import org.mockito.MockitoAnnotations;

import java.util.ArrayList;
import java.util.List;
import java.util.concurrent.Executor;

@RunWith(JUnit4.class)
public class PhoneStateAgentTest {
    private static final int SLOT0 = 0;
    private static final int SLOT1 = 1;
    private static final int[] SUB_ID = { 1 };
    private static final int[] SUB_ID1 = { 2 };

    @Mock private ServiceState mServiceState;
    @Mock private ISubscription mSubscription;
    @Mock private ImsPhoneStateListener mPsListener;
    @Mock private PhoneStateNotifier mPsNotifier;

    private ContextFixture mContextFixture;
    private SubscriptionManager mSubscriptionManager;
    private TelephonyManager mTelephonyManager;
    private NetworkRegistrationInfo mNetworkRegistrationInfo;
    private TelephonyCallback.ServiceStateListener mServiceStateListener;
    private PhoneStateAgent mPsAgent;
    private MSimUtils.SubscriptionManagerProxy mSubscriptionManagerProxy =
            new MSimUtils.SubscriptionManagerProxy() {
                @Override
                public int getDefaultDataSubscriptionId() {
                    return SUB_ID[0];
                }

                @Override
                public int getSlotIndex(int subId) {
                    if (subId == SUB_ID[0]) {
                        return SLOT0;
                    } else if (subId == SUB_ID1[0]) {
                        return SLOT1;
                    }
                    return MSimUtils.INVALID_SLOT_ID;
                }
            };

    @Before
    public void setUp() throws Exception {
        MockitoAnnotations.initMocks(this);

        mContextFixture = new ContextFixture();
        Context context = mContextFixture.getTestDouble();
        mSubscriptionManager = context.getSystemService(SubscriptionManager.class);
        when(mSubscriptionManager.getSubscriptionIds(eq(SLOT0))).thenReturn(SUB_ID);
        mTelephonyManager = context.getSystemService(TelephonyManager.class);
        when(mTelephonyManager.createForSubscriptionId(eq(SUB_ID[0])))
                .thenReturn(mTelephonyManager);

        AppContext.init(context);
        ImsLog.setDebugOn(true);
        MSimUtils.setSubscriptionManagerProxy(mSubscriptionManagerProxy);
        AgentFactory.setDefaultAgent(AgentFactory.SUBSCRIPTION, mSubscription);
        mPsAgent = new PhoneStateAgent(SLOT0);
        mPsAgent.init(context);
    }

    @After
    public void tearDown() throws Exception {
        AgentFactory.setDefaultAgent(AgentFactory.SUBSCRIPTION, null);
        MSimUtils.setSubscriptionManagerProxy(null);
        ImsLog.setDebugOn(false);
        mPsNotifier = null;
        mPsListener = null;
        mSubscription = null;
        mTelephonyManager = null;
        mContextFixture = null;
        mPsAgent.cleanup();
        mPsAgent = null;
        AppContext.deinit();
    }

    @Test
    @SmallTest
    public void testCleanup() {
        int eventCount = getEventCount(mPsAgent.getPhoneStateEvents().getEvents());
        mPsAgent.cleanup();

        verify(mTelephonyManager, times(eventCount))
                .registerTelephonyCallback(any(Executor.class), any(TelephonyCallback.class));
        verify(mSubscription).addListener(any(SubscriptionListener.class));
        verify(mTelephonyManager, times(eventCount))
                .unregisterTelephonyCallback(any(TelephonyCallback.class));
        verify(mSubscription).removeListener(any(SubscriptionListener.class));
    }

    @Test
    @SmallTest
    public void testCreateNotifier() {
        IPhoneStateNotifier psn = mPsAgent.createNotifier(mPsListener);

        assertNotNull(psn);

        psn = mPsAgent.createNotifier(mPsListener, Looper.myLooper());

        assertNotNull(psn);
    }

    @Test
    @SmallTest
    public void testAddAndRemoveNotifier() {
        when(mPsNotifier.getEvents()).thenReturn(ImsPhoneStateListener.LISTEN_CELL_INFO);
        int originalEvents = mPsAgent.getPhoneStateEvents().getEvents();
        int originalEventCount = getEventCount(mPsAgent.getPhoneStateEvents().getEvents());
        mPsAgent.addNotifier(mPsNotifier);
        int newEventCount = getEventCount(mPsAgent.getPhoneStateEvents().getEvents());

        verify(mTelephonyManager, times(originalEventCount + newEventCount))
                .registerTelephonyCallback(any(Executor.class), any(TelephonyCallback.class));

        mPsAgent.removeNotifier(mPsNotifier);

        verify(mTelephonyManager, times(originalEventCount + newEventCount))
                .unregisterTelephonyCallback(any(TelephonyCallback.class));

        // Already registered events for service state and call state.
        when(mPsNotifier.getEvents()).thenReturn(originalEvents);
        mPsAgent.addNotifier(mPsNotifier);

        verify(mPsNotifier, never()).notifyServiceState(any(ServiceState.class));
        verify(mPsNotifier).notifyCallState(anyInt());
        verify(mPsNotifier, never()).notifyBarringInfo(any(BarringInfo.class));

        TelephonyManager tm = Mockito.mock(TelephonyManager.class);
        when(mTelephonyManager.createForSubscriptionId(eq(SUB_ID[0]))).thenReturn(tm);
        PhoneStateNotifier psNotifier1 = Mockito.mock(PhoneStateNotifier.class);
        int newEvents = originalEvents | ImsPhoneStateListener.LISTEN_CELL_INFO;
        when(psNotifier1.getEvents()).thenReturn(newEvents);
        mPsAgent.addNotifier(psNotifier1);

        ArgumentCaptor<TelephonyCallback> captor = ArgumentCaptor.forClass(TelephonyCallback.class);
        verify(tm, times(getEventCount(newEvents)))
                .registerTelephonyCallback(any(Executor.class), captor.capture());
        for (TelephonyCallback callback : captor.getAllValues()) {
            invokeTelephonyCallback(callback);
        }

        PhoneStateNotifier psNotifier2 = Mockito.mock(PhoneStateNotifier.class);
        when(psNotifier2.getEvents()).thenReturn(newEvents);
        mPsAgent.addNotifier(psNotifier2);

        verify(psNotifier2).notifyServiceState(any(ServiceState.class));
        verify(psNotifier2).notifyCallState(anyInt());
        verify(psNotifier2).notifyBarringInfo(any(BarringInfo.class));
    }

    @Test
    @SmallTest
    public void testOnPhoneStateEventChanged() {
        int originalEvents = mPsAgent.getPhoneStateEvents().getEvents();
        // No actions because it's not added before calling the below method.
        mPsAgent.onPhoneStateEventChanged(mPsNotifier, originalEvents, originalEvents);

        when(mPsNotifier.getEvents()).thenReturn(originalEvents);
        int originalEventCount = getEventCount(originalEvents);
        // There is no change for the registered events.
        mPsAgent.addNotifier(mPsNotifier);
        mPsAgent.onPhoneStateEventChanged(mPsNotifier,
                originalEvents | ImsPhoneStateListener.LISTEN_CELL_INFO,
                ImsPhoneStateListener.LISTEN_CELL_INFO);
        int newEventCount = getEventCount(mPsAgent.getPhoneStateEvents().getEvents());

        verify(mTelephonyManager, times(originalEventCount + newEventCount))
                .registerTelephonyCallback(any(Executor.class), any(TelephonyCallback.class));
        verify(mTelephonyManager, times(originalEventCount))
                .unregisterTelephonyCallback(any(TelephonyCallback.class));
    }

    @Test
    @SmallTest
    public void testSimLoadCompleted() {
        int eventCount = getEventCount(mPsAgent.getPhoneStateEvents().getEvents());
        TelephonyManager tm = Mockito.mock(TelephonyManager.class);
        when(mTelephonyManager.createForSubscriptionId(eq(SUB_ID[0]))).thenReturn(tm);
        ArgumentCaptor<SubscriptionListener> captor =
                ArgumentCaptor.forClass(SubscriptionListener.class);
        verify(mSubscription).addListener(captor.capture());

        SubscriptionListener listener = captor.getValue();
        // Same slot & same subscription
        listener.onSimLoadCompleted(SLOT0);

        TelephonyManager tm1 = Mockito.mock(TelephonyManager.class);
        when(mTelephonyManager.createForSubscriptionId(eq(SUB_ID1[0]))).thenReturn(tm1);
        when(mSubscriptionManager.getSubscriptionIds(eq(SLOT0))).thenReturn(SUB_ID1);
        when(mSubscription.getSubId(eq(SLOT0))).thenReturn(SUB_ID1[0]);

        // Same slot & different subscription
        listener.onSimLoadCompleted(SLOT0);

        verify(tm1, times(eventCount))
                .registerTelephonyCallback(any(Executor.class), any(TelephonyCallback.class));
        verify(tm, times(eventCount))
                .unregisterTelephonyCallback(any(TelephonyCallback.class));
    }

    @Test
    @SmallTest
    public void testDefaultSubscriptionChanged() {
        TelephonyManager defaultTelephonyManager = Mockito.mock(TelephonyManager.class);
        mContextFixture.setSystemService(Context.TELEPHONY_SERVICE, defaultTelephonyManager);

        int eventCount = getEventCount(mPsAgent.getPhoneStateEvents().getEvents());
        TelephonyManager tm = Mockito.mock(TelephonyManager.class);
        when(defaultTelephonyManager.createForSubscriptionId(eq(SUB_ID[0]))).thenReturn(tm);
        ArgumentCaptor<SubscriptionListener> captor =
                ArgumentCaptor.forClass(SubscriptionListener.class);
        verify(mSubscription).addListener(captor.capture());

        SubscriptionListener listener = captor.getValue();
        // Same subscription
        listener.onDefaultSubscriptionChanged(SUB_ID[0]);

        TelephonyManager tm1 = Mockito.mock(TelephonyManager.class);
        when(defaultTelephonyManager.createForSubscriptionId(eq(SUB_ID1[0]))).thenReturn(tm1);
        when(mSubscriptionManager.getSubscriptionIds(eq(SLOT0))).thenReturn(SUB_ID1);
        when(mSubscription.getSubId(eq(SLOT0))).thenReturn(SUB_ID1[0]);

        // Different subscription
        listener.onDefaultSubscriptionChanged(SUB_ID1[0]);

        verify(tm1, times(eventCount))
                .registerTelephonyCallback(any(Executor.class), any(TelephonyCallback.class));
        verify(tm, times(eventCount))
                .unregisterTelephonyCallback(any(TelephonyCallback.class));

        when(mSubscription.getSubId(eq(SLOT0))).thenReturn(MSimUtils.INVALID_SUB_ID);

        // Invalid subscription
        listener.onDefaultSubscriptionChanged(MSimUtils.INVALID_SUB_ID);

        verify(defaultTelephonyManager, times(eventCount))
                .registerTelephonyCallback(any(Executor.class), any(TelephonyCallback.class));
        verify(tm1, times(eventCount))
                .unregisterTelephonyCallback(any(TelephonyCallback.class));
    }

    @Test
    @SmallTest
    public void testDefaultDataSubscriptionChanged() {
        int eventCount = getEventCount(mPsAgent.getPhoneStateEvents().getEvents());
        TelephonyManager tm = Mockito.mock(TelephonyManager.class);
        when(mTelephonyManager.createForSubscriptionId(eq(SUB_ID[0]))).thenReturn(tm);
        ArgumentCaptor<SubscriptionListener> captor =
                ArgumentCaptor.forClass(SubscriptionListener.class);
        verify(mSubscription).addListener(captor.capture());

        SubscriptionListener listener = captor.getValue();
        // Same subscription
        listener.onDefaultDataSubscriptionChanged(SUB_ID[0]);

        TelephonyManager tm1 = Mockito.mock(TelephonyManager.class);
        when(mTelephonyManager.createForSubscriptionId(eq(SUB_ID1[0]))).thenReturn(tm1);
        when(mSubscriptionManager.getSubscriptionIds(eq(SLOT0))).thenReturn(SUB_ID1);
        when(mSubscription.getSubId(eq(SLOT0))).thenReturn(SUB_ID1[0]);

        // Different subscription
        listener.onDefaultDataSubscriptionChanged(SUB_ID1[0]);

        verify(tm1, times(eventCount))
                .registerTelephonyCallback(any(Executor.class), any(TelephonyCallback.class));
        verify(tm, times(eventCount))
                .unregisterTelephonyCallback(any(TelephonyCallback.class));
    }

    @Test
    @SmallTest
    public void testTelephonyCallbackCalled() {
        TelephonyManager tm = Mockito.mock(TelephonyManager.class);
        when(mTelephonyManager.createForSubscriptionId(eq(SUB_ID[0]))).thenReturn(tm);
        when(tm.getDataNetworkType()).thenReturn(TelephonyManager.NETWORK_TYPE_IWLAN);

        int allEvents = ImsPhoneStateListener.LISTEN_SERVICE_STATE
                | ImsPhoneStateListener.LISTEN_CALL_STATE
                | ImsPhoneStateListener.LISTEN_PRECISE_CALL_STATE
                | ImsPhoneStateListener.LISTEN_SRVCC_STATE
                | ImsPhoneStateListener.LISTEN_CELL_INFO
                | ImsPhoneStateListener.LISTEN_SIGNAL_STRENGTHS
                | ImsPhoneStateListener.LISTEN_PRECISE_DATA_CONNECTION_STATE
                | ImsPhoneStateListener.LISTEN_BARRING_INFO;
        when(mPsNotifier.getEvents()).thenReturn(allEvents);
        mPsAgent.addNotifier(mPsNotifier);

        ArgumentCaptor<TelephonyCallback> captor = ArgumentCaptor.forClass(TelephonyCallback.class);
        verify(tm, times(getEventCount(allEvents)))
                .registerTelephonyCallback(any(Executor.class), captor.capture());

        for (TelephonyCallback callback : captor.getAllValues()) {
            invokeTelephonyCallback(callback);
        }

        verify(mPsNotifier).notifyServiceState(any(ServiceState.class));
        verify(mPsNotifier).notifyCallState(anyInt());
        verify(mPsNotifier).notifyCellInfo(any());
        verify(mPsNotifier).notifyPreciseCallState(any(PreciseCallState.class));
        verify(mPsNotifier).notifySignalStrengths(any(SignalStrength.class));
        verify(mPsNotifier).notifySrvccState(anyInt());
        verify(mPsNotifier).notifyPreciseDataConnectionState(any(PreciseDataConnectionState.class));
        verify(mPsNotifier).notifyBarringInfo(any(BarringInfo.class));
        assertEquals(TelephonyManager.NETWORK_TYPE_LTE, mPsAgent.getCellularDataNetworkType());

        // Network is not registered.
        mNetworkRegistrationInfo = new NetworkRegistrationInfo.Builder().build();
        when(mServiceState.getNetworkRegistrationInfo(
                eq(NetworkRegistrationInfo.DOMAIN_PS),
                eq(AccessNetworkConstants.TRANSPORT_TYPE_WWAN)))
                .thenReturn(mNetworkRegistrationInfo);
        when(mServiceState.getRoaming()).thenReturn(true);
        mServiceStateListener.onServiceStateChanged(mServiceState);
        assertEquals(TelephonyManager.NETWORK_TYPE_UNKNOWN, mPsAgent.getCellularDataNetworkType());

        // NetworkRegistrationInfo is null.
        when(mServiceState.getNetworkRegistrationInfo(anyInt(), anyInt())).thenReturn(null);
        mServiceStateListener.onServiceStateChanged(mServiceState);
        assertEquals(TelephonyManager.NETWORK_TYPE_UNKNOWN, mPsAgent.getCellularDataNetworkType());
    }

    @Test
    @SmallTest
    public void testTelephonyCallbackServiceStateChanged() {
        when(mTelephonyManager.getDataNetworkType())
                .thenReturn(TelephonyManager.NETWORK_TYPE_IWLAN);

        int events = mPsAgent.getPhoneStateEvents().getEvents();
        ArgumentCaptor<TelephonyCallback> captor = ArgumentCaptor.forClass(TelephonyCallback.class);
        verify(mTelephonyManager, times(getEventCount(events)))
                .registerTelephonyCallback(any(Executor.class), captor.capture());

        for (TelephonyCallback callback : captor.getAllValues()) {
            invokeTelephonyCallback(callback);
        }

        assertEquals(TelephonyManager.NETWORK_TYPE_LTE, mPsAgent.getCellularDataNetworkType());

        // Network is not registered.
        ServiceState serviceState = Mockito.mock(ServiceState.class);
        mNetworkRegistrationInfo = new NetworkRegistrationInfo.Builder().build();
        when(serviceState.getNetworkRegistrationInfo(
                anyInt(), eq(AccessNetworkConstants.TRANSPORT_TYPE_WWAN)))
                .thenReturn(mNetworkRegistrationInfo);
        when(mServiceState.getState()).thenReturn(ServiceState.STATE_OUT_OF_SERVICE);
        mServiceStateListener.onServiceStateChanged(serviceState);
        assertEquals(TelephonyManager.NETWORK_TYPE_UNKNOWN, mPsAgent.getCellularDataNetworkType());

        // NetworkRegistrationInfo is null.
        when(serviceState.getNetworkRegistrationInfo(anyInt(), anyInt())).thenReturn(null);
        mServiceStateListener.onServiceStateChanged(serviceState);
        assertEquals(TelephonyManager.NETWORK_TYPE_UNKNOWN, mPsAgent.getCellularDataNetworkType());
    }

    private void invokeTelephonyCallback(TelephonyCallback callback) {
        if (callback instanceof TelephonyCallback.ServiceStateListener) {
            TelephonyCallback.ServiceStateListener listener =
                    (TelephonyCallback.ServiceStateListener) callback;
            mNetworkRegistrationInfo = new NetworkRegistrationInfo.Builder()
                    .setAccessNetworkTechnology(TelephonyManager.NETWORK_TYPE_LTE)
                    .setRegistrationState(NetworkRegistrationInfo.REGISTRATION_STATE_HOME)
                    .build();
            when(mServiceState.getNetworkRegistrationInfo(
                    anyInt(), eq(AccessNetworkConstants.TRANSPORT_TYPE_WWAN)))
                    .thenReturn(mNetworkRegistrationInfo);
            when(mServiceState.getState()).thenReturn(ServiceState.STATE_IN_SERVICE);
            when(mServiceState.getRoaming()).thenReturn(true);
            when(mServiceState.getOperatorNumeric()).thenReturn("00101");
            listener.onServiceStateChanged(mServiceState);
            mServiceStateListener = listener;
        } else if (callback instanceof TelephonyCallback.CallStateListener) {
            TelephonyCallback.CallStateListener listener =
                    (TelephonyCallback.CallStateListener) callback;
            listener.onCallStateChanged(TelephonyManager.CALL_STATE_OFFHOOK);
        } else if (callback instanceof TelephonyCallback.CellInfoListener) {
            List<CellInfo> cellInfos = new ArrayList<>();
            TelephonyCallback.CellInfoListener listener =
                    (TelephonyCallback.CellInfoListener) callback;
            listener.onCellInfoChanged(cellInfos);
        } else if (callback instanceof TelephonyCallback.PreciseCallStateListener) {
            PreciseCallState callState = new PreciseCallState(
                    PreciseCallState.PRECISE_CALL_STATE_IDLE,
                    PreciseCallState.PRECISE_CALL_STATE_ACTIVE,
                    PreciseCallState.PRECISE_CALL_STATE_IDLE,
                    DisconnectCause.NOT_VALID,
                    PreciseDisconnectCause.NOT_VALID);
            TelephonyCallback.PreciseCallStateListener listener =
                    (TelephonyCallback.PreciseCallStateListener) callback;
            listener.onPreciseCallStateChanged(callState);
        } else if (callback instanceof TelephonyCallback.SignalStrengthsListener) {
            SignalStrength ss = Mockito.mock(SignalStrength.class);
            TelephonyCallback.SignalStrengthsListener listener =
                    (TelephonyCallback.SignalStrengthsListener) callback;
            listener.onSignalStrengthsChanged(ss);
        } else if (callback instanceof TelephonyCallback.SrvccStateListener) {
            TelephonyCallback.SrvccStateListener listener =
                    (TelephonyCallback.SrvccStateListener) callback;
            listener.onSrvccStateChanged(TelephonyManager.SRVCC_STATE_HANDOVER_STARTED);
        } else if (callback instanceof TelephonyCallback.PreciseDataConnectionStateListener) {
            PreciseDataConnectionState dataConnectionState =
                    new PreciseDataConnectionState.Builder().build();
            TelephonyCallback.PreciseDataConnectionStateListener listener =
                    (TelephonyCallback.PreciseDataConnectionStateListener) callback;
            listener.onPreciseDataConnectionStateChanged(dataConnectionState);
        } else if (callback instanceof TelephonyCallback.BarringInfoListener) {
            BarringInfo barringInfo = new BarringInfo();
            TelephonyCallback.BarringInfoListener listener =
                    (TelephonyCallback.BarringInfoListener) callback;
            listener.onBarringInfoChanged(barringInfo);
        }
    }

    private static int getEventCount(int events) {
        int count = 0;
        for (int i = 0; i < 32; ++i) {
            int event = (events >> i);
            if ((event & 0x1) != 0) {
                count++;
            }
        }
        return count;
    }
}
