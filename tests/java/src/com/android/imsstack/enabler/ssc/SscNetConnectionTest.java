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

package com.android.imsstack.enabler.ssc;

import static org.junit.Assert.assertEquals;
import static org.junit.Assert.assertNotNull;
import static org.mockito.ArgumentMatchers.anyInt;
import static org.mockito.ArgumentMatchers.anyLong;
import static org.mockito.ArgumentMatchers.eq;
import static org.mockito.Mockito.verify;
import static org.mockito.Mockito.verifyNoMoreInteractions;
import static org.mockito.Mockito.when;

import android.os.AsyncResult;
import android.os.Handler;
import android.os.Message;
import android.telephony.TelephonyManager;

import com.android.imsstack.core.agents.ConfigAgent;
import com.android.imsstack.core.agents.agentif.IAlarmTimer;
import com.android.imsstack.core.agents.dcm.DcFactory;
import com.android.imsstack.core.agents.dcmif.EApnType;
import com.android.imsstack.core.agents.dcmif.EDataState;
import com.android.imsstack.core.agents.dcmif.IApn;
import com.android.imsstack.core.agents.dcmif.IDc;
import com.android.imsstack.core.agents.dcmif.IDcApn;
import com.android.imsstack.core.agents.dcmif.IDcNetWatcher;
import com.android.imsstack.core.config.CarrierConfig;

import org.junit.After;
import org.junit.Before;
import org.junit.Test;
import org.junit.runner.RunWith;
import org.junit.runners.JUnit4;
import org.mockito.ArgumentCaptor;
import org.mockito.Captor;
import org.mockito.Mock;
import org.mockito.MockitoAnnotations;

import java.util.HashMap;

@RunWith(JUnit4.class)
public class SscNetConnectionTest {
    private static final int SLOT_0 = 0;

    private SscNetConnection mSscNetConnection = null;
    private Handler mSscNetConnectionHandler = null;
    private EApnType mApnType = EApnType.XCAP;
    private int mInactivityTimer = 100;
    private int mTimerId = 1;

    @Mock private IAlarmTimer mMockAlarmTimer;
    @Mock private IApn mMockApnXcap;
    @Mock private IDcApn mMockDcApn;
    @Mock private IDcNetWatcher mMockDcNetWatcher;
    @Mock private CarrierConfig mMockCarrierConfig;
    @Mock private ConfigAgent mMockConfigAgent;
    @Mock private Handler mMockSscTransactionHandler;
    @Mock private SscServiceState mMockSscServiceState;

    @Captor ArgumentCaptor<Message> mMessageCaptor;

    @Before
    public void setup() {
        MockitoAnnotations.initMocks(this);

        when(mMockConfigAgent.getCarrierConfig()).thenReturn(mMockCarrierConfig);
        when(mMockCarrierConfig.getInt(
                CarrierConfig.Assets.KEY_UT_XCAP_APN_INACTIVITY_TIMER_SEC_INT))
                .thenReturn(mInactivityTimer);
        SscConfig.setConfigAgent(SLOT_0, mMockConfigAgent);

        HashMap<Integer, IDc> dcs = new HashMap<Integer, IDc>(2);
        dcs.put(DcFactory.NETWORK_WATCHER, mMockDcNetWatcher);
        dcs.put(DcFactory.APN, mMockDcApn);
        DcFactory.setObjects(SLOT_0, dcs);

        mSscNetConnection = new FakeSscNetConnection(SLOT_0);
        mSscNetConnection.init(mApnType);
        mSscNetConnection.setCallbackHandler(mMockSscTransactionHandler);
        mSscNetConnectionHandler = mSscNetConnection.mSscNetConnectionHandler;
    }

    @After
    public void tearDown() {
        mSscNetConnection.cleanup();
    }

    @Test
    public void init_registerEvents() {
        // mSscNetConnection.init(EApnType.XCAP) is called in setup() function
        verify(mMockDcNetWatcher).registerForDataStateChanged(mSscNetConnectionHandler,
                SscNetConnection.EVENT_PDN_DATA_STATE_CHANGED, null);
        verify(mMockDcNetWatcher).registerForPdnConnectionFailed(mSscNetConnectionHandler,
                SscNetConnection.EVENT_PDN_CONNECTION_FAILED, null);

        assertEquals(mInactivityTimer * 1000, mSscNetConnection.mConnectionInactivityTimer);
    }

    @Test
    public void cleanup_unregisterEvents() {
        mSscNetConnection.cleanup();

        verify(mMockDcApn).disconnect(eq(mApnType.getType()));
        verify(mMockDcNetWatcher).unregisterForDataServiceStateChanged(mSscNetConnectionHandler);
        verify(mMockDcNetWatcher).unregisterForPdnConnectionFailed(mSscNetConnectionHandler);
    }

    @Test
    public void isConnected_apnTypeNull() {
        mSscNetConnection.init(null);

        boolean isConnected = mSscNetConnection.isConnected();
        assertEquals(false, isConnected);
        verifyNoMoreInteractions(mMockDcApn);
    }

    @Test
    public void isConnected_getDcApnNull() {
        DcFactory.setObjects(SLOT_0, null);

        boolean isConnected = mSscNetConnection.isConnected();
        assertEquals(false, isConnected);
        verifyNoMoreInteractions(mMockDcApn);
    }

    @Test
    public void isConnected_getDataStateConnected() {
        when(mMockDcApn.getDataState(mApnType.getType()))
                .thenReturn(TelephonyManager.DATA_CONNECTED);

        boolean isConnected = mSscNetConnection.isConnected();
        verify(mMockDcApn).getDataState(mApnType.getType());
        assertEquals(true, isConnected);
    }

    @Test
    public void isConnected_getDataStateDisconnected() {
        when(mMockDcApn.getDataState(mApnType.getType()))
                .thenReturn(TelephonyManager.DATA_DISCONNECTED);

        boolean isConnected = mSscNetConnection.isConnected();
        verify(mMockDcApn).getDataState(mApnType.getType());
        assertEquals(false, isConnected);
    }

    @Test
    public void connect_apnTypeNull() {
        mSscNetConnection.init(null);

        boolean result = mSscNetConnection.connect();
        assertEquals(false, result);
        verifyNoMoreInteractions(mMockDcApn);
    }

    @Test
    public void connect_alreadyConnected() {
        when(mMockDcApn.getDataState(mApnType.getType()))
                .thenReturn(TelephonyManager.DATA_CONNECTED);

        boolean result = mSscNetConnection.connect();
        verify(mMockDcApn).getDataState(mApnType.getType());
        assertEquals(true, result);
    }

    @Test
    public void connect_getDcApnNull() {
        DcFactory.setObjects(SLOT_0, null);
        when(mMockDcApn.getDataState(mApnType.getType()))
                .thenReturn(TelephonyManager.DATA_DISCONNECTED);

        boolean result = mSscNetConnection.connect();
        assertEquals(false, result);
    }

    @Test
    public void connect_requestConnectionFailure() {
        when(mMockDcApn.getDataState(mApnType.getType()))
                .thenReturn(TelephonyManager.DATA_DISCONNECTED);
        when(mMockDcApn.connect(eq(mApnType.getType()))).thenReturn(false);

        boolean result = mSscNetConnection.connect();
        assertEquals(false, result);
    }

    @Test
    public void connect_requestConnectionSuccess() {
        when(mMockDcApn.getDataState(mApnType.getType()))
                .thenReturn(TelephonyManager.DATA_DISCONNECTED);
        when(mMockDcApn.connect(eq(mApnType.getType()))).thenReturn(true);
        when(mMockAlarmTimer.getTimerId()).thenReturn(mTimerId);
        when(mMockAlarmTimer.startTimer(mTimerId, SscNetConnection.PDN_CONNECTION_TIMEOUT_TIMER))
                .thenReturn(true);

        boolean result = mSscNetConnection.connect();

        verify(mMockAlarmTimer).registerForTimerExpired(mTimerId, mSscNetConnectionHandler,
                SscNetConnection.EVENT_PDN_REQUEST_TIMEOUT, null);
        verify(mMockAlarmTimer).startTimer(mTimerId, SscNetConnection.PDN_CONNECTION_TIMEOUT_TIMER);

        assertEquals(Integer.valueOf(mTimerId),
                mSscNetConnection.mTimerIdTable.get(SscNetConnection.EVENT_PDN_REQUEST_TIMEOUT));
        assertEquals(true, result);
    }

    @Test
    public void isPdnAvailable() {
        // TODO: Should it disconnect other slot's connection?
    }

    @Test
    public void disconnect_apnTypeNull() {
        mSscNetConnection.init(null);

        mSscNetConnection.disconnect();

        verifyNoMoreInteractions(mMockDcApn);
    }

    @Test
    public void disconnect_getDcApnNull() {
        DcFactory.setObjects(SLOT_0, null);

        mSscNetConnection.disconnect();

        verifyNoMoreInteractions(mMockDcApn);
    }

    @Test
    public void disconnect_requestDisconnection() {
        int timerIdForRequestTimeout = 2;
        int timerIdForConnectionExpired = 3;
        mSscNetConnection.mTimerIdTable.put(SscNetConnection.EVENT_PDN_REQUEST_TIMEOUT,
                timerIdForRequestTimeout);
        mSscNetConnection.mTimerIdTable.put(SscNetConnection.EVENT_PDN_CONNECTION_EXPIRED,
                timerIdForConnectionExpired);

        mSscNetConnection.disconnect();

        verify(mMockAlarmTimer).stopTimer(timerIdForRequestTimeout);
        verify(mMockAlarmTimer).unregisterForTimerExpired(timerIdForRequestTimeout,
                mSscNetConnectionHandler);
        verify(mMockAlarmTimer).stopTimer(timerIdForConnectionExpired);
        verify(mMockAlarmTimer).unregisterForTimerExpired(timerIdForConnectionExpired,
                mSscNetConnectionHandler);
        verify(mMockDcApn).disconnect(eq(mApnType.getType()));
        assertEquals(null,
                mSscNetConnection.mTimerIdTable.get(SscNetConnection.EVENT_PDN_REQUEST_TIMEOUT));
        assertEquals(null,
                mSscNetConnection.mTimerIdTable.get(SscNetConnection.EVENT_PDN_CONNECTION_EXPIRED));
    }

    @Test
    public void refreshConnectionTimer_stopPreviousTimer() {
        int timerIdForConnectionExpired = mTimerId + 1; // different value from mTimerId
        mSscNetConnection.mTimerIdTable.put(SscNetConnection.EVENT_PDN_CONNECTION_EXPIRED,
                timerIdForConnectionExpired);

        when(mMockAlarmTimer.getTimerId()).thenReturn(mTimerId);
        when(mMockAlarmTimer.startTimer(mTimerId, mInactivityTimer * 1000)).thenReturn(true);

        mSscNetConnection.refreshConnectionTimer();

        verify(mMockAlarmTimer).stopTimer(timerIdForConnectionExpired);
        verify(mMockAlarmTimer).startTimer(mTimerId, mInactivityTimer * 1000);
        assertEquals(Integer.valueOf(mTimerId),
                mSscNetConnection.mTimerIdTable.get(SscNetConnection.EVENT_PDN_CONNECTION_EXPIRED));
    }

    @Test
    public void handleMessage_connectionConnected() {
        int timerIdForRequestTimeout = mTimerId + 1;
        mSscNetConnection.mTimerIdTable.put(SscNetConnection.EVENT_PDN_REQUEST_TIMEOUT,
                timerIdForRequestTimeout);

        when(mMockAlarmTimer.startTimer(mTimerId, mInactivityTimer * 1000)).thenReturn(true);
        when(mMockAlarmTimer.getTimerId()).thenReturn(mTimerId);

        IDcNetWatcher.NotiObj notiObj = new IDcNetWatcher.NotiObj(EApnType.XCAP,
                EDataState.DATA_STATE_CONNECTED);
        AsyncResult ar = new AsyncResult(null, notiObj, null);
        triggerEventHandler(SscNetConnection.EVENT_PDN_DATA_STATE_CHANGED, ar);

        verify(mMockAlarmTimer).stopTimer(timerIdForRequestTimeout);
        verify(mMockAlarmTimer).startTimer(mTimerId, mInactivityTimer * 1000);
        assertEquals(Integer.valueOf(mTimerId),
                mSscNetConnection.mTimerIdTable.get(SscNetConnection.EVENT_PDN_CONNECTION_EXPIRED));
        verify(mMockSscTransactionHandler).sendMessageAtTime(mMessageCaptor.capture(), anyLong());
        Message msg = mMessageCaptor.getValue();
        assertNotNull(msg);
        assertEquals(mSscNetConnection.EVENT_PDN_CONNECTED, msg.what);
    }

    @Test
    public void handleMessage_connectionDisconnected() {
        int timerIdForRequestTimeout = mTimerId + 1;
        mSscNetConnection.mTimerIdTable.put(SscNetConnection.EVENT_PDN_REQUEST_TIMEOUT,
                timerIdForRequestTimeout);

        when(mMockAlarmTimer.startTimer(mTimerId, SscNetConnection.DISCONNECTION_DELAY))
                .thenReturn(true);
        when(mMockAlarmTimer.getTimerId()).thenReturn(mTimerId);

        IDcNetWatcher.NotiObj notiObj = new IDcNetWatcher.NotiObj(EApnType.XCAP,
                EDataState.DATA_STATE_DISCONNECTED);
        AsyncResult ar = new AsyncResult(null, notiObj, null);
        triggerEventHandler(SscNetConnection.EVENT_PDN_DATA_STATE_CHANGED, ar);

        verify(mMockAlarmTimer).startTimer(mTimerId, SscNetConnection.DISCONNECTION_DELAY);
        assertEquals(Integer.valueOf(mTimerId),
                mSscNetConnection.mTimerIdTable.get(SscNetConnection.EVENT_PDN_CONNECTION_EXPIRED));
        verify(mMockSscTransactionHandler).sendMessageAtTime(mMessageCaptor.capture(), anyLong());
        Message msg = mMessageCaptor.getValue();
        assertNotNull(msg);
        assertEquals(mSscNetConnection.EVENT_PDN_DISCONNECTED, msg.what);
    }

    @Test
    public void handleMessage_connectionFailed() {
        int timerIdForRequestTimeout = 2;
        mSscNetConnection.mTimerIdTable.put(SscNetConnection.EVENT_PDN_REQUEST_TIMEOUT,
                timerIdForRequestTimeout);
        SscServiceStateAgent.getInstance().setSscServiceState(SLOT_0, mMockSscServiceState);
        when(mMockDcApn.getApnControl(anyInt())).thenReturn(mMockApnXcap);
        when(mMockApnXcap.isESMCausePermanentFailure()).thenReturn(true);

        AsyncResult ar = new AsyncResult(null, EApnType.XCAP, null);
        triggerEventHandler(SscNetConnection.EVENT_PDN_CONNECTION_FAILED, ar);

        verify(mMockSscServiceState).setPdnConnectionFailed(true);
        verify(mMockSscTransactionHandler).sendMessageAtTime(mMessageCaptor.capture(), anyLong());
        Message msg = mMessageCaptor.getValue();
        assertNotNull(msg);
        assertEquals(mSscNetConnection.EVENT_PDN_CONNECTION_FAILED, msg.what);

        verify(mMockAlarmTimer).stopTimer(timerIdForRequestTimeout);
        verify(mMockAlarmTimer).unregisterForTimerExpired(timerIdForRequestTimeout,
                mSscNetConnectionHandler);
        verify(mMockDcApn).disconnect(eq(mApnType.getType()));
        assertEquals(null,
                mSscNetConnection.mTimerIdTable.get(SscNetConnection.EVENT_PDN_REQUEST_TIMEOUT));
    }

    @Test
    public void handleMessage_connectionRequestTimeout() {
        int timerIdForRequestTimeout = 2;
        mSscNetConnection.mTimerIdTable.put(SscNetConnection.EVENT_PDN_REQUEST_TIMEOUT,
                timerIdForRequestTimeout);

        triggerEventHandler(SscNetConnection.EVENT_PDN_REQUEST_TIMEOUT, null);

        verify(mMockSscTransactionHandler).sendMessageAtTime(mMessageCaptor.capture(), anyLong());
        Message msg = mMessageCaptor.getValue();
        assertNotNull(msg);
        assertEquals(mSscNetConnection.EVENT_PDN_REQUEST_TIMEOUT, msg.what);

        verify(mMockAlarmTimer).stopTimer(timerIdForRequestTimeout);
        verify(mMockAlarmTimer).unregisterForTimerExpired(timerIdForRequestTimeout,
                mSscNetConnectionHandler);
        verify(mMockDcApn).disconnect(eq(mApnType.getType()));
        assertEquals(null,
                mSscNetConnection.mTimerIdTable.get(SscNetConnection.EVENT_PDN_REQUEST_TIMEOUT));
    }

    @Test
    public void handleMessage_connectionTimerExpired() {
        int timerIdForConnectionExpired = 3;
        mSscNetConnection.mTimerIdTable.put(SscNetConnection.EVENT_PDN_CONNECTION_EXPIRED,
                timerIdForConnectionExpired);

        triggerEventHandler(SscNetConnection.EVENT_PDN_CONNECTION_EXPIRED, null);

        verify(mMockAlarmTimer).stopTimer(timerIdForConnectionExpired);
        verify(mMockAlarmTimer).unregisterForTimerExpired(timerIdForConnectionExpired,
                mSscNetConnectionHandler);
        verify(mMockDcApn).disconnect(eq(mApnType.getType()));
        assertEquals(null,
                mSscNetConnection.mTimerIdTable.get(SscNetConnection.EVENT_PDN_CONNECTION_EXPIRED));
    }

    @Test
    public void startTimer_invalidTimerId() {
        int timerIdForConnectionExpired = mTimerId + 1; // different value from mTimerId
        mSscNetConnection.mTimerIdTable.put(SscNetConnection.EVENT_PDN_CONNECTION_EXPIRED,
                timerIdForConnectionExpired);

        when(mMockAlarmTimer.getTimerId()).thenReturn(mTimerId);
        when(mMockAlarmTimer.startTimer(mTimerId, mInactivityTimer * 1000)).thenReturn(true);

        mSscNetConnection.refreshConnectionTimer();
    }

    private void triggerEventHandler(int event, AsyncResult ar) {
        mSscNetConnectionHandler.handleMessage(
                Message.obtain(mSscNetConnectionHandler, event, ar));
    }

    private class FakeSscNetConnection extends SscNetConnection {
        private FakeSscNetConnection(int slotId) {
            super(slotId);
        }

        @Override
        protected IAlarmTimer getAlarmTimer() {
            return mMockAlarmTimer;
        }
    }
}
