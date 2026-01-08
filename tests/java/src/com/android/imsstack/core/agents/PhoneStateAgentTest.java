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

import static com.android.imsstack.base.TestAppContext.SLOT0;
import static com.android.imsstack.base.TestAppContext.SLOT1;
import static com.android.imsstack.base.TestAppContext.SUB_ID_1;
import static com.android.imsstack.base.TestAppContext.SUB_ID_2;

import static org.junit.Assert.assertEquals;
import static org.junit.Assert.assertNotNull;
import static org.mockito.ArgumentMatchers.any;
import static org.mockito.ArgumentMatchers.anyInt;
import static org.mockito.ArgumentMatchers.eq;
import static org.mockito.Mockito.mock;
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
import android.telephony.TelephonyCallback;
import android.telephony.TelephonyManager;

import androidx.test.filters.SmallTest;

import com.android.imsstack.base.MSimUtils;
import com.android.imsstack.base.SystemServiceProxy.SubscriptionManagerProxy;
import com.android.imsstack.base.TelephonyManagerProxy;
import com.android.imsstack.base.TestAppContext;
import com.android.imsstack.core.agents.internal.PhoneStateNotifier;
import com.android.imsstack.system.ISystem;
import com.android.imsstack.system.ImsEventDef;
import com.android.imsstack.system.SystemInterface;
import com.android.imsstack.util.ImsLog;

import org.junit.After;
import org.junit.Before;
import org.junit.Test;
import org.junit.runner.RunWith;
import org.junit.runners.JUnit4;
import org.mockito.ArgumentCaptor;
import org.mockito.Mock;
import org.mockito.MockitoAnnotations;

import java.util.ArrayList;
import java.util.List;
import java.util.concurrent.Executor;

@RunWith(JUnit4.class)
public class PhoneStateAgentTest {
    @Mock private Context mContext;
    @Mock private ServiceState mServiceState;
    @Mock private SimInterface mSimInterface;
    @Mock private ImsPhoneStateListener mPsListener;
    @Mock private PhoneStateNotifier mPsNotifier;
    @Mock private ISystem mSystem;
    @Mock private SystemInterface mSystemInterface;

    private TestAppContext mTestAppContext;
    private TelephonyManagerProxy mTelephonyManagerProxy;
    private NetworkRegistrationInfo mNetworkRegistrationInfo;
    private TelephonyCallback.ServiceStateListener mServiceStateListener;
    private PhoneStateAgent mPsAgent;

    @Before
    public void setUp() throws Exception {
        MockitoAnnotations.initMocks(this);

        mTestAppContext = new TestAppContext(mContext);
        mTestAppContext.setUp();

        SystemInterface.setSystemInterface(mSystemInterface);
        when(mSystemInterface.getSystem(eq(SLOT0))).thenReturn(mSystem);
        mTelephonyManagerProxy = mTestAppContext.getSystemServiceProxy(TelephonyManagerProxy.class);
        ImsLog.setDebugOn(true);
        AgentFactory.getInstance()
                .setAgent(SimInterface.class, mSimInterface, SLOT0);
        mPsAgent = new PhoneStateAgent(SLOT0);
        mPsAgent.init(mContext);
    }

    @After
    public void tearDown() throws Exception {
        AgentFactory.getInstance().setAgent(SimInterface.class, null, SLOT0);
        ImsLog.setDebugOn(false);
        mTelephonyManagerProxy = null;
        mPsNotifier = null;
        mPsListener = null;
        mSimInterface = null;
        mPsAgent.cleanup();
        mPsAgent = null;
        mSystem = null;
        mSystemInterface = null;
        mTestAppContext.tearDown();
        mTestAppContext = null;
    }

    @Test
    @SmallTest
    public void testCleanup() {
        int eventCount = getEventCount(mPsAgent.getPhoneStateEvents().getEvents());
        mPsAgent.cleanup();

        verify(mTelephonyManagerProxy, times(eventCount))
                .registerTelephonyCallback(any(Executor.class), any(TelephonyCallback.class));
        verify(mSimInterface).addListener(any(Sim.Listener.class));
        verify(mTelephonyManagerProxy, times(eventCount))
                .unregisterTelephonyCallback(any(TelephonyCallback.class));
        verify(mSimInterface).removeListener(any(Sim.Listener.class));
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

        verify(mTelephonyManagerProxy, times(originalEventCount + newEventCount))
                .registerTelephonyCallback(any(Executor.class), any(TelephonyCallback.class));

        mPsAgent.removeNotifier(mPsNotifier);

        verify(mTelephonyManagerProxy, times(originalEventCount + newEventCount))
                .unregisterTelephonyCallback(any(TelephonyCallback.class));

        // Already registered events for service state and call state.
        when(mPsNotifier.getEvents()).thenReturn(originalEvents);
        mPsAgent.addNotifier(mPsNotifier);

        verify(mPsNotifier, never()).notifyServiceState(any(ServiceState.class));
        verify(mPsNotifier).notifyCallState(anyInt());
        verify(mPsNotifier, never()).notifyBarringInfo(any(BarringInfo.class));

        TelephonyManagerProxy tmp = mock(TelephonyManagerProxy.class);
        when(mTelephonyManagerProxy.createForSubscriptionId(eq(SUB_ID_1)))
                .thenReturn(tmp);
        PhoneStateNotifier psNotifier1 = mock(PhoneStateNotifier.class);
        int newEvents = originalEvents | ImsPhoneStateListener.LISTEN_CELL_INFO;
        when(psNotifier1.getEvents()).thenReturn(newEvents);
        mPsAgent.addNotifier(psNotifier1);

        ArgumentCaptor<TelephonyCallback> captor = ArgumentCaptor.forClass(TelephonyCallback.class);
        verify(tmp, times(getEventCount(newEvents)))
                .registerTelephonyCallback(any(Executor.class), captor.capture());
        for (TelephonyCallback callback : captor.getAllValues()) {
            invokeTelephonyCallback(callback);
        }

        PhoneStateNotifier psNotifier2 = mock(PhoneStateNotifier.class);
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

        verify(mTelephonyManagerProxy, times(originalEventCount + newEventCount))
                .registerTelephonyCallback(any(Executor.class), any(TelephonyCallback.class));
        verify(mTelephonyManagerProxy, times(originalEventCount))
                .unregisterTelephonyCallback(any(TelephonyCallback.class));
    }

    @Test
    @SmallTest
    public void testOnSimStateChanged() {
        int eventCount = getEventCount(mPsAgent.getPhoneStateEvents().getEvents());
        TelephonyManagerProxy tmp = mock(TelephonyManagerProxy.class);
        when(mTelephonyManagerProxy.createForSubscriptionId(eq(SUB_ID_1)))
                .thenReturn(tmp);
        ArgumentCaptor<Sim.Listener> captor = ArgumentCaptor.forClass(Sim.Listener.class);
        verify(mSimInterface).addListener(captor.capture());

        Sim.Listener simListener = captor.getValue();
        AgentFactory.getInstance().setAgent(SimInterface.class, null, SLOT0);
        // Ignored because SimInterface is null.
        simListener.onSimStateChanged();

        AgentFactory.getInstance()
                .setAgent(SimInterface.class, mSimInterface, SLOT0);
        when(mSimInterface.isSimLoadCompleted()).thenReturn(false);
        // Ignored because SIM state is not fully loaded.
        simListener.onSimStateChanged();

        // Same slot & same subscription
        when(mSimInterface.isSimLoadCompleted()).thenReturn(true);
        when(mSimInterface.getSubId()).thenReturn(SUB_ID_1);
        simListener.onSimStateChanged();

        TelephonyManagerProxy tmp1 = mock(TelephonyManagerProxy.class);
        when(mTelephonyManagerProxy.createForSubscriptionId(eq(SUB_ID_2)))
                .thenReturn(tmp1);
        when(mSimInterface.getSubId()).thenReturn(SUB_ID_2);
        // Same slot & different subscription
        simListener.onSimStateChanged();

        verify(tmp1, times(eventCount))
                .registerTelephonyCallback(any(Executor.class), any(TelephonyCallback.class));
        verify(tmp, times(eventCount))
                .unregisterTelephonyCallback(any(TelephonyCallback.class));
    }

    @Test
    @SmallTest
    public void testTelephonyCallbackCalled() {
        TelephonyManagerProxy tmp = mock(TelephonyManagerProxy.class);
        when(mTelephonyManagerProxy.createForSubscriptionId(eq(SUB_ID_1)))
                .thenReturn(tmp);

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
        verify(tmp, times(getEventCount(allEvents)))
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
        assertEquals(NetworkRegistrationInfo.REGISTRATION_STATE_HOME,
                mPsAgent.getCsNetworkRegistrationState());

        // Network is not registered.
        mNetworkRegistrationInfo = new NetworkRegistrationInfo.Builder().build();
        when(mServiceState.getNetworkRegistrationInfo(
                anyInt(), eq(AccessNetworkConstants.TRANSPORT_TYPE_WWAN)))
                .thenReturn(mNetworkRegistrationInfo);
        when(mServiceState.getRoaming()).thenReturn(true);
        mServiceStateListener.onServiceStateChanged(mServiceState);
        assertEquals(TelephonyManager.NETWORK_TYPE_UNKNOWN, mPsAgent.getCellularDataNetworkType());
        assertEquals(NetworkRegistrationInfo.REGISTRATION_STATE_NOT_REGISTERED_OR_SEARCHING,
                mPsAgent.getCsNetworkRegistrationState());

        // NetworkRegistrationInfo is null.
        when(mServiceState.getNetworkRegistrationInfo(anyInt(), anyInt())).thenReturn(null);
        mServiceStateListener.onServiceStateChanged(mServiceState);
        assertEquals(TelephonyManager.NETWORK_TYPE_UNKNOWN, mPsAgent.getCellularDataNetworkType());
        assertEquals(NetworkRegistrationInfo.REGISTRATION_STATE_NOT_REGISTERED_OR_SEARCHING,
                mPsAgent.getCsNetworkRegistrationState());
    }

    @Test
    @SmallTest
    public void testTelephonyCallbackServiceStateChanged() {
        int events = mPsAgent.getPhoneStateEvents().getEvents();
        ArgumentCaptor<TelephonyCallback> captor = ArgumentCaptor.forClass(TelephonyCallback.class);
        verify(mTelephonyManagerProxy, times(getEventCount(events)))
                .registerTelephonyCallback(any(Executor.class), captor.capture());

        for (TelephonyCallback callback : captor.getAllValues()) {
            invokeTelephonyCallback(callback);
        }

        assertEquals(TelephonyManager.NETWORK_TYPE_LTE, mPsAgent.getCellularDataNetworkType());
        assertEquals(NetworkRegistrationInfo.REGISTRATION_STATE_HOME,
                mPsAgent.getCsNetworkRegistrationState());

        // Network is not registered.
        ServiceState serviceState = mock(ServiceState.class);
        mNetworkRegistrationInfo = new NetworkRegistrationInfo.Builder().build();
        when(serviceState.getNetworkRegistrationInfo(
                anyInt(), eq(AccessNetworkConstants.TRANSPORT_TYPE_WWAN)))
                .thenReturn(mNetworkRegistrationInfo);
        when(mServiceState.getState()).thenReturn(ServiceState.STATE_OUT_OF_SERVICE);
        mServiceStateListener.onServiceStateChanged(serviceState);
        assertEquals(TelephonyManager.NETWORK_TYPE_UNKNOWN, mPsAgent.getCellularDataNetworkType());
        assertEquals(NetworkRegistrationInfo.REGISTRATION_STATE_NOT_REGISTERED_OR_SEARCHING,
                mPsAgent.getCsNetworkRegistrationState());

        // NetworkRegistrationInfo is null.
        when(serviceState.getNetworkRegistrationInfo(anyInt(), anyInt())).thenReturn(null);
        mServiceStateListener.onServiceStateChanged(serviceState);
        assertEquals(TelephonyManager.NETWORK_TYPE_UNKNOWN, mPsAgent.getCellularDataNetworkType());
        assertEquals(NetworkRegistrationInfo.REGISTRATION_STATE_NOT_REGISTERED_OR_SEARCHING,
                mPsAgent.getCsNetworkRegistrationState());
    }

    @Test
    @SmallTest
    public void testCsCallStateChanged() {
        int events = mPsAgent.getPhoneStateEvents().getEvents();
        ArgumentCaptor<TelephonyCallback> captor = ArgumentCaptor.forClass(TelephonyCallback.class);
        verify(mTelephonyManagerProxy, times(getEventCount(events)))
                .registerTelephonyCallback(any(Executor.class), captor.capture());

        TelephonyCallback.CallStateListener csListener = null;
        for (TelephonyCallback callback : captor.getAllValues()) {
            if (callback instanceof TelephonyCallback.CallStateListener) {
                csListener = (TelephonyCallback.CallStateListener) callback;
                break;
            }
        }

        assertNotNull(csListener);

        csListener.onCallStateChanged(TelephonyManager.CALL_STATE_OFFHOOK);

        assertEquals(TelephonyManager.CALL_STATE_OFFHOOK, mPsAgent.getCsCallState());
        verify(mSystem).notifyEvent(eq(ImsEventDef.IMS_EVENT_CSCALL_STATE),
                eq(TelephonyManager.CALL_STATE_OFFHOOK), eq(0));

        csListener.onCallStateChanged(TelephonyManager.CALL_STATE_IDLE);

        assertEquals(TelephonyManager.CALL_STATE_IDLE, mPsAgent.getCsCallState());
        verify(mSystem).notifyEvent(eq(ImsEventDef.IMS_EVENT_CSCALL_STATE),
                eq(TelephonyManager.CALL_STATE_IDLE), eq(0));
    }

    @Test
    @SmallTest
    public void testCsCallStateChangedWhenImsInCall() {
        int events = mPsAgent.getPhoneStateEvents().getEvents();
        ArgumentCaptor<TelephonyCallback> captor = ArgumentCaptor.forClass(TelephonyCallback.class);
        verify(mTelephonyManagerProxy, times(getEventCount(events)))
                .registerTelephonyCallback(any(Executor.class), captor.capture());

        TelephonyCallback.CallStateListener csListener = null;
        for (TelephonyCallback callback : captor.getAllValues()) {
            if (callback instanceof TelephonyCallback.CallStateListener) {
                csListener = (TelephonyCallback.CallStateListener) callback;
                break;
            }
        }

        assertNotNull(csListener);

        mPsAgent.setImsCallState(TelephonyManager.CALL_STATE_OFFHOOK);
        csListener.onCallStateChanged(TelephonyManager.CALL_STATE_OFFHOOK);

        assertEquals(TelephonyManager.CALL_STATE_OFFHOOK, mPsAgent.getImsCallState());
        assertEquals(TelephonyManager.CALL_STATE_IDLE, mPsAgent.getCsCallState());

        csListener.onCallStateChanged(TelephonyManager.CALL_STATE_IDLE);

        assertEquals(TelephonyManager.CALL_STATE_IDLE, mPsAgent.getImsCallState());
        assertEquals(TelephonyManager.CALL_STATE_IDLE, mPsAgent.getCsCallState());
        verify(mSystem, never())
                .notifyEvent(eq(ImsEventDef.IMS_EVENT_CSCALL_STATE), anyInt(), anyInt());
    }

    @Test
    @SmallTest
    public void testCallStateChangedWhenCallStateIsIdleWhileImsCallStateIsOffhook() {
        int events = mPsAgent.getPhoneStateEvents().getEvents();
        ArgumentCaptor<TelephonyCallback> captor = ArgumentCaptor.forClass(TelephonyCallback.class);
        verify(mTelephonyManagerProxy, times(getEventCount(events)))
                .registerTelephonyCallback(any(Executor.class), captor.capture());

        TelephonyCallback.CallStateListener csListener = null;
        for (TelephonyCallback callback : captor.getAllValues()) {
            if (callback instanceof TelephonyCallback.CallStateListener) {
                csListener = (TelephonyCallback.CallStateListener) callback;
                break;
            }
        }

        assertNotNull(csListener);

        // Set IMS call state to OFFHOOK. And then Telephony call state is changed to IDLE. This may
        // happen when ImsStack registers the CallStateListener when the call starts.
        mPsAgent.setImsCallState(TelephonyManager.CALL_STATE_OFFHOOK);
        csListener.onCallStateChanged(TelephonyManager.CALL_STATE_IDLE);

        assertEquals(TelephonyManager.CALL_STATE_IDLE, mPsAgent.getCsCallState());
        // IMS call state should not be changed and there should be no notification at this time
        // since the Telephony call state has not changed. IDLE ->IDLE.
        assertEquals(TelephonyManager.CALL_STATE_OFFHOOK, mPsAgent.getImsCallState());
        verify(mSystem, never()).notifyEvent(eq(ImsEventDef.IMS_EVENT_CSCALL_STATE),
                eq(TelephonyManager.CALL_STATE_IDLE), eq(0));

        csListener.onCallStateChanged(TelephonyManager.CALL_STATE_OFFHOOK);

        assertEquals(TelephonyManager.CALL_STATE_IDLE, mPsAgent.getCsCallState());
        assertEquals(TelephonyManager.CALL_STATE_OFFHOOK, mPsAgent.getImsCallState());
        verify(mSystem, never()).notifyEvent(eq(ImsEventDef.IMS_EVENT_CSCALL_STATE),
                eq(TelephonyManager.CALL_STATE_OFFHOOK), eq(0));
    }

    @Test
    @SmallTest
    public void testCsCallStateChangedWhenSim1LoadedAndSim2Absent() {
        when(mSimInterface.getSimCardState()).thenReturn(Sim.STATE_PRESENT);
        when(mSimInterface.getSimState()).thenReturn(Sim.STATE_LOADED);
        SubscriptionManagerProxy smp =
                mTestAppContext.getSystemServiceProxy(SubscriptionManagerProxy.class);
        when(smp.getSubscriptionId(eq(SLOT1))).thenReturn(MSimUtils.INVALID_SUB_ID);
        ISystem systemForSim2 = mock(ISystem.class);
        when(mSystemInterface.getSystem(eq(SLOT1))).thenReturn(systemForSim2);

        PhoneStateAgent psAgentForSim2 = null;

        try {
            psAgentForSim2 = new PhoneStateAgent(SLOT1);
            psAgentForSim2.init(mContext);

            int events = mPsAgent.getPhoneStateEvents().getEvents();
            int eventCount = getEventCount(events);
            events = psAgentForSim2.getPhoneStateEvents().getEvents();
            eventCount += getEventCount(events);
            ArgumentCaptor<TelephonyCallback> captor =
                    ArgumentCaptor.forClass(TelephonyCallback.class);
            verify(mTelephonyManagerProxy, times(eventCount))
                    .registerTelephonyCallback(any(Executor.class), captor.capture());

            List<TelephonyCallback.CallStateListener> csListeners = new ArrayList<>();
            for (TelephonyCallback callback : captor.getAllValues()) {
                if (callback instanceof TelephonyCallback.CallStateListener) {
                    csListeners.add((TelephonyCallback.CallStateListener) callback);
                }
            }

            assertEquals(2, csListeners.size());

            for (TelephonyCallback.CallStateListener listener : csListeners) {
                listener.onCallStateChanged(TelephonyManager.CALL_STATE_OFFHOOK);
            }

            assertEquals(TelephonyManager.CALL_STATE_OFFHOOK, mPsAgent.getCsCallState());
            verify(mSystem).notifyEvent(eq(ImsEventDef.IMS_EVENT_CSCALL_STATE),
                    eq(TelephonyManager.CALL_STATE_OFFHOOK), eq(0));
            // Call state change is ignored because this call state change is from SIM1
            // when SIM1 is in LOADED and SIM2 is absent.
            assertEquals(TelephonyManager.CALL_STATE_IDLE, psAgentForSim2.getCsCallState());

            for (TelephonyCallback.CallStateListener listener : csListeners) {
                listener.onCallStateChanged(TelephonyManager.CALL_STATE_IDLE);
            }

            assertEquals(TelephonyManager.CALL_STATE_IDLE, mPsAgent.getCsCallState());
            verify(mSystem).notifyEvent(eq(ImsEventDef.IMS_EVENT_CSCALL_STATE),
                    eq(TelephonyManager.CALL_STATE_IDLE), eq(0));
            assertEquals(TelephonyManager.CALL_STATE_IDLE, psAgentForSim2.getCsCallState());
            verify(systemForSim2, never())
                    .notifyEvent(eq(ImsEventDef.IMS_EVENT_CSCALL_STATE), anyInt(), anyInt());
        } finally {
            if (psAgentForSim2 != null) {
                psAgentForSim2.cleanup();
                psAgentForSim2 = null;
            }
        }
    }

    @Test
    @SmallTest
    public void testCsCallStateChangedWhenImsCallDelayed() {
        int events = mPsAgent.getPhoneStateEvents().getEvents();
        ArgumentCaptor<TelephonyCallback> captor = ArgumentCaptor.forClass(TelephonyCallback.class);
        verify(mTelephonyManagerProxy, times(getEventCount(events)))
                .registerTelephonyCallback(any(Executor.class), captor.capture());

        TelephonyCallback.CallStateListener csListener = null;
        for (TelephonyCallback callback : captor.getAllValues()) {
            if (callback instanceof TelephonyCallback.CallStateListener) {
                csListener = (TelephonyCallback.CallStateListener) callback;
                break;
            }
        }

        assertNotNull(csListener);

        csListener.onCallStateChanged(TelephonyManager.CALL_STATE_OFFHOOK);

        assertEquals(TelephonyManager.CALL_STATE_IDLE, mPsAgent.getImsCallState());
        assertEquals(TelephonyManager.CALL_STATE_OFFHOOK, mPsAgent.getCsCallState());

        mPsAgent.setImsCallState(TelephonyManager.CALL_STATE_OFFHOOK);

        assertEquals(TelephonyManager.CALL_STATE_OFFHOOK, mPsAgent.getImsCallState());
        assertEquals(TelephonyManager.CALL_STATE_IDLE, mPsAgent.getCsCallState());
        verify(mSystem, times(2))
                .notifyEvent(eq(ImsEventDef.IMS_EVENT_CSCALL_STATE), anyInt(), anyInt());
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
            SignalStrength ss = mock(SignalStrength.class);
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
