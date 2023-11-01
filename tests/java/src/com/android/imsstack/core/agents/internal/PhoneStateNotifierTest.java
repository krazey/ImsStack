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

package com.android.imsstack.core.agents.internal;

import static org.junit.Assert.assertEquals;
import static org.mockito.ArgumentMatchers.eq;
import static org.mockito.Mockito.verify;

import android.os.HandlerThread;
import android.telephony.BarringInfo;
import android.telephony.CellInfo;
import android.telephony.DisconnectCause;
import android.telephony.PreciseCallState;
import android.telephony.PreciseDataConnectionState;
import android.telephony.PreciseDisconnectCause;
import android.telephony.ServiceState;
import android.telephony.SignalStrength;
import android.telephony.TelephonyManager;
import android.testing.TestableLooper;

import androidx.test.filters.SmallTest;

import com.android.imsstack.core.agents.ImsPhoneStateListener;

import org.junit.After;
import org.junit.Before;
import org.junit.Test;
import org.junit.runner.RunWith;
import org.junit.runners.JUnit4;
import org.mockito.Mock;
import org.mockito.Mockito;
import org.mockito.MockitoAnnotations;

import java.util.ArrayList;
import java.util.List;

@RunWith(JUnit4.class)
public class PhoneStateNotifierTest {
    @Mock private ImsPhoneStateListener mPsListener;
    @Mock private ImsPhoneStateListener mPsListenerForNoHandler;
    @Mock private PhoneStateNotifier.EventObserver mPsnEventObserver;

    private TestableLooper mTestableLooper;
    private PhoneStateNotifier mPsNotifier;
    private PhoneStateNotifier mPsNotifierForNoHandler;

    @Before
    public void setUp() throws Exception {
        MockitoAnnotations.initMocks(this);

        HandlerThread handlerThread = new HandlerThread(
                PhoneStateNotifierTest.class.getSimpleName());
        handlerThread.start();
        mTestableLooper = new TestableLooper(handlerThread.getLooper());
        mPsNotifier = new PhoneStateNotifier(handlerThread.getLooper(), mPsnEventObserver);
        mPsNotifier.setListener(mPsListener);
        mPsNotifierForNoHandler = new PhoneStateNotifier(null, mPsnEventObserver);
        mPsNotifierForNoHandler.setListener(mPsListenerForNoHandler);
    }

    @After
    public void tearDown() throws Exception {
        if (mTestableLooper != null) {
            mTestableLooper.destroy();
            mTestableLooper = null;
        }

        mPsnEventObserver = null;
        mPsListenerForNoHandler = null;
        mPsListener = null;
        mPsNotifierForNoHandler = null;
        mPsNotifier = null;
    }

    @Test
    @SmallTest
    public void testSetEvents() {
        assertEquals(ImsPhoneStateListener.LISTEN_NONE, mPsNotifier.getEvents());

        int events = PhoneStateEvents.DEFAULT_EVENTS;
        mPsNotifier.setEvents(events);

        assertEquals(events, mPsNotifier.getEvents());
        verify(mPsnEventObserver)
                .onPhoneStateEventChanged(eq(mPsNotifier), eq(events), eq(events));

        events |= ImsPhoneStateListener.LISTEN_CELL_INFO;
        mPsNotifier.setEvents(events);

        assertEquals(events, mPsNotifier.getEvents());
        verify(mPsnEventObserver).onPhoneStateEventChanged(
                eq(mPsNotifier), eq(events), eq(ImsPhoneStateListener.LISTEN_CELL_INFO));
    }

    @Test
    @SmallTest
    public void testNotifyCallState() {
        mPsNotifier.setEvents(ImsPhoneStateListener.LISTEN_CALL_STATE);
        mPsNotifier.notifyCallState(TelephonyManager.CALL_STATE_OFFHOOK);
        processAllMessages();

        verify(mPsListener).onCallStateChanged(eq(TelephonyManager.CALL_STATE_OFFHOOK));

        // No interactions because the event is not registered.
        mPsNotifierForNoHandler.notifyCallState(TelephonyManager.CALL_STATE_OFFHOOK);

        mPsNotifierForNoHandler.setEvents(ImsPhoneStateListener.LISTEN_CALL_STATE);
        mPsNotifierForNoHandler.notifyCallState(TelephonyManager.CALL_STATE_OFFHOOK);

        verify(mPsListenerForNoHandler).onCallStateChanged(eq(TelephonyManager.CALL_STATE_OFFHOOK));
    }

    @Test
    @SmallTest
    public void testNotifyCellInfo() {
        List<CellInfo> cellInfos = new ArrayList<>();
        mPsNotifier.setEvents(ImsPhoneStateListener.LISTEN_CELL_INFO);
        mPsNotifier.notifyCellInfo(cellInfos);
        processAllMessages();

        verify(mPsListener).onCellInfoChanged(eq(cellInfos));

        // No interactions because the event is not registered.
        mPsNotifierForNoHandler.notifyCellInfo(cellInfos);

        mPsNotifierForNoHandler.setEvents(ImsPhoneStateListener.LISTEN_CELL_INFO);
        mPsNotifierForNoHandler.notifyCellInfo(cellInfos);

        verify(mPsListenerForNoHandler).onCellInfoChanged(eq(cellInfos));
    }

    @Test
    @SmallTest
    public void testNotifyPreciseCallState() {
        PreciseCallState callState = new PreciseCallState(
                PreciseCallState.PRECISE_CALL_STATE_IDLE,
                PreciseCallState.PRECISE_CALL_STATE_ACTIVE,
                PreciseCallState.PRECISE_CALL_STATE_IDLE,
                DisconnectCause.NOT_VALID,
                PreciseDisconnectCause.NOT_VALID);
        mPsNotifier.setEvents(ImsPhoneStateListener.LISTEN_PRECISE_CALL_STATE);
        mPsNotifier.notifyPreciseCallState(callState);
        processAllMessages();

        verify(mPsListener).onPreciseCallStateChanged(eq(callState));

        // No interactions because the event is not registered.
        mPsNotifierForNoHandler.notifyPreciseCallState(callState);

        mPsNotifierForNoHandler.setEvents(ImsPhoneStateListener.LISTEN_PRECISE_CALL_STATE);
        mPsNotifierForNoHandler.notifyPreciseCallState(callState);

        verify(mPsListenerForNoHandler).onPreciseCallStateChanged(eq(callState));
    }

    @Test
    @SmallTest
    public void testNotifyServiceState() {
        ServiceState serviceState = new ServiceState();
        mPsNotifier.setEvents(ImsPhoneStateListener.LISTEN_SERVICE_STATE);
        mPsNotifier.notifyServiceState(serviceState);
        processAllMessages();

        verify(mPsListener).onServiceStateChanged(eq(serviceState));

        // No interactions because the event is not registered.
        mPsNotifierForNoHandler.notifyServiceState(serviceState);

        mPsNotifierForNoHandler.setEvents(ImsPhoneStateListener.LISTEN_SERVICE_STATE);
        mPsNotifierForNoHandler.notifyServiceState(serviceState);

        verify(mPsListenerForNoHandler).onServiceStateChanged(eq(serviceState));
    }

    @Test
    @SmallTest
    public void testNotifySignalStrengths() {
        SignalStrength signalStrength = Mockito.mock(SignalStrength.class);
        mPsNotifier.setEvents(ImsPhoneStateListener.LISTEN_SIGNAL_STRENGTHS);
        mPsNotifier.notifySignalStrengths(signalStrength);
        processAllMessages();

        verify(mPsListener).onSignalStrengthsChanged(eq(signalStrength));

        // No interactions because the event is not registered.
        mPsNotifierForNoHandler.notifySignalStrengths(signalStrength);

        mPsNotifierForNoHandler.setEvents(ImsPhoneStateListener.LISTEN_SIGNAL_STRENGTHS);
        mPsNotifierForNoHandler.notifySignalStrengths(signalStrength);

        verify(mPsListenerForNoHandler).onSignalStrengthsChanged(eq(signalStrength));
    }

    @Test
    @SmallTest
    public void testNotifySrvccState() {
        mPsNotifier.setEvents(ImsPhoneStateListener.LISTEN_SRVCC_STATE);
        mPsNotifier.notifySrvccState(TelephonyManager.SRVCC_STATE_HANDOVER_STARTED);
        processAllMessages();

        verify(mPsListener).onSrvccStateChanged(eq(TelephonyManager.SRVCC_STATE_HANDOVER_STARTED));

        // No interactions because the event is not registered.
        mPsNotifierForNoHandler.notifySrvccState(TelephonyManager.SRVCC_STATE_HANDOVER_STARTED);

        mPsNotifierForNoHandler.setEvents(ImsPhoneStateListener.LISTEN_SRVCC_STATE);
        mPsNotifierForNoHandler.notifySrvccState(TelephonyManager.SRVCC_STATE_HANDOVER_STARTED);

        verify(mPsListenerForNoHandler)
                .onSrvccStateChanged(eq(TelephonyManager.SRVCC_STATE_HANDOVER_STARTED));
    }

    @Test
    @SmallTest
    public void testNotifyPreciseDataConnectionState() {
        PreciseDataConnectionState dataConnectionState =
                new PreciseDataConnectionState.Builder().build();
        mPsNotifier.setEvents(ImsPhoneStateListener.LISTEN_PRECISE_DATA_CONNECTION_STATE);
        mPsNotifier.notifyPreciseDataConnectionState(dataConnectionState);
        processAllMessages();

        verify(mPsListener).onPreciseDataConnectionStateChanged(eq(dataConnectionState));

        // No interactions because the event is not registered.
        mPsNotifierForNoHandler.notifyPreciseDataConnectionState(dataConnectionState);

        mPsNotifierForNoHandler.setEvents(
                ImsPhoneStateListener.LISTEN_PRECISE_DATA_CONNECTION_STATE);
        mPsNotifierForNoHandler.notifyPreciseDataConnectionState(dataConnectionState);

        verify(mPsListenerForNoHandler)
                .onPreciseDataConnectionStateChanged(eq(dataConnectionState));
    }

    @Test
    @SmallTest
    public void testNotifyBarringInfo() {
        BarringInfo barringInfo = new BarringInfo();
        mPsNotifier.setEvents(ImsPhoneStateListener.LISTEN_BARRING_INFO);
        mPsNotifier.notifyBarringInfo(barringInfo);
        processAllMessages();

        verify(mPsListener).onBarringInfoChanged(eq(barringInfo));

        // No interactions because the event is not registered.
        mPsNotifierForNoHandler.notifyBarringInfo(barringInfo);

        mPsNotifierForNoHandler.setEvents(ImsPhoneStateListener.LISTEN_BARRING_INFO);
        mPsNotifierForNoHandler.notifyBarringInfo(barringInfo);

        verify(mPsListenerForNoHandler).onBarringInfoChanged(eq(barringInfo));
    }

    private void processAllMessages() {
        while (!mTestableLooper.getLooper().getQueue().isIdle()) {
            mTestableLooper.processAllMessages();
        }
    }
}
