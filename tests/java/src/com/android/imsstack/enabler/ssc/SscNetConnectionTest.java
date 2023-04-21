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
import static org.mockito.ArgumentMatchers.any;
import static org.mockito.ArgumentMatchers.anyLong;
import static org.mockito.ArgumentMatchers.eq;
import static org.mockito.Mockito.never;
import static org.mockito.Mockito.verify;
import static org.mockito.Mockito.when;

import android.os.AsyncResult;
import android.os.Handler;
import android.os.Message;
import android.telephony.TelephonyManager;

import com.android.imsstack.core.agents.ConfigAgent;
import com.android.imsstack.core.agents.TimerInterface;
import com.android.imsstack.core.agents.dcm.DcFactory;
import com.android.imsstack.core.agents.dcmif.ApnStateListener;
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
    private static final EApnType APN_TYPE = EApnType.XCAP;
    private static final int INACTIVITY_TIME_SEC = 100;
    private static final long TIMER_ID = 1L;

    private SscNetConnection mSscNetConnection = null;
    private Handler mSscNetConnectionHandler = null;

    @Mock private TimerInterface mMockTimerInterface;
    @Mock private IApn mMockApn;
    @Mock private IDcApn mMockDcApn;
    @Mock private IDcNetWatcher mMockDcNetWatcher;
    @Mock private CarrierConfig mMockCarrierConfig;
    @Mock private ConfigAgent mMockConfigAgent;
    @Mock private Handler mMockSscTransactionHandler;
    @Mock private SscServiceState mMockSscServiceState;

    @Captor ArgumentCaptor<Message> mMessageCaptor;
    @Captor ArgumentCaptor<ApnStateListener> mApnStateListenerCaptor;

    @Before
    public void setup() {
        MockitoAnnotations.initMocks(this);

        when(mMockConfigAgent.getCarrierConfig()).thenReturn(mMockCarrierConfig);
        when(mMockCarrierConfig.getInt(
                CarrierConfig.Assets.KEY_UT_XCAP_APN_INACTIVITY_TIMER_SEC_INT))
                .thenReturn(INACTIVITY_TIME_SEC);
        SscConfig.setConfigAgent(SLOT_0, mMockConfigAgent);

        when(mMockDcApn.getApnControl(APN_TYPE.getType())).thenReturn(mMockApn);
        HashMap<Integer, IDc> dcs = new HashMap<Integer, IDc>(2);
        dcs.put(DcFactory.NETWORK_WATCHER, mMockDcNetWatcher);
        dcs.put(DcFactory.APN, mMockDcApn);
        DcFactory.setObjects(SLOT_0, dcs);

        mSscNetConnection = new FakeSscNetConnection(SLOT_0);
        mSscNetConnection.init(APN_TYPE);
        mSscNetConnection.setCallbackHandler(mMockSscTransactionHandler);
        mSscNetConnectionHandler = mSscNetConnection.mSscNetConnectionHandler;
    }

    @After
    public void tearDown() {
        mSscNetConnection.cleanup();
        SscConfig.clear(SLOT_0);
    }

    @Test
    public void init_registerEvents() {
        // mSscNetConnection.init(EApnType.XCAP) is called in setup() function
        verify(mMockDcNetWatcher).registerForDataStateChanged(mSscNetConnectionHandler,
                SscNetConnection.EVENT_PDN_DATA_STATE_CHANGED, null);
        verify(mMockDcNetWatcher).registerForPdnConnectionFailed(mSscNetConnectionHandler,
                SscNetConnection.EVENT_PDN_CONNECTION_FAILED, null);
        verify(mMockApn).addListener(any());

        assertEquals(INACTIVITY_TIME_SEC * 1000, mSscNetConnection.mConnectionInactivityTimer);
    }

    @Test
    public void cleanup_unregisterEvents() {
        mSscNetConnection.cleanup();

        verify(mMockDcApn).disconnect(eq(APN_TYPE.getType()));
        verify(mMockApn).removeListener(any());
        verify(mMockDcNetWatcher).unregisterForDataServiceStateChanged(mSscNetConnectionHandler);
        verify(mMockDcNetWatcher).unregisterForPdnConnectionFailed(mSscNetConnectionHandler);
    }

    @Test
    public void isConnected_apnTypeNull() {
        mSscNetConnection.init(null);

        boolean isConnected = mSscNetConnection.isConnected();
        assertEquals(false, isConnected);
        verify(mMockDcApn, never()).getDataState(APN_TYPE.getType());
    }

    @Test
    public void isConnected_getDcApnNull() {
        DcFactory.setObjects(SLOT_0, null);

        boolean isConnected = mSscNetConnection.isConnected();
        assertEquals(false, isConnected);
        verify(mMockDcApn, never()).getDataState(APN_TYPE.getType());
    }

    @Test
    public void isConnected_getDataStateConnected() {
        when(mMockDcApn.getDataState(APN_TYPE.getType()))
                .thenReturn(TelephonyManager.DATA_CONNECTED);

        boolean isConnected = mSscNetConnection.isConnected();
        verify(mMockDcApn).getDataState(APN_TYPE.getType());
        assertEquals(true, isConnected);
    }

    @Test
    public void isConnected_getDataStateDisconnected() {
        when(mMockDcApn.getDataState(APN_TYPE.getType()))
                .thenReturn(TelephonyManager.DATA_DISCONNECTED);

        boolean isConnected = mSscNetConnection.isConnected();
        verify(mMockDcApn).getDataState(APN_TYPE.getType());
        assertEquals(false, isConnected);
    }

    @Test
    public void connect_apnTypeNull() {
        mSscNetConnection.init(null);

        boolean result = mSscNetConnection.connect();
        assertEquals(false, result);
        verify(mMockDcApn, never()).connect(APN_TYPE.getType());
    }

    @Test
    public void connect_alreadyConnected() {
        when(mMockDcApn.getDataState(APN_TYPE.getType()))
                .thenReturn(TelephonyManager.DATA_CONNECTED);

        boolean result = mSscNetConnection.connect();
        verify(mMockDcApn).getDataState(APN_TYPE.getType());
        assertEquals(true, result);
    }

    @Test
    public void connect_getDcApnNull() {
        DcFactory.setObjects(SLOT_0, null);
        when(mMockDcApn.getDataState(APN_TYPE.getType()))
                .thenReturn(TelephonyManager.DATA_DISCONNECTED);

        boolean result = mSscNetConnection.connect();
        assertEquals(false, result);
    }

    @Test
    public void connect_requestConnectionFailure() {
        when(mMockDcApn.getDataState(APN_TYPE.getType()))
                .thenReturn(TelephonyManager.DATA_DISCONNECTED);
        when(mMockDcApn.connect(eq(APN_TYPE.getType()))).thenReturn(false);

        boolean result = mSscNetConnection.connect();
        assertEquals(false, result);
    }

    @Test
    public void connect_requestConnectionSuccess() {
        when(mMockDcApn.getDataState(APN_TYPE.getType()))
                .thenReturn(TelephonyManager.DATA_DISCONNECTED);
        when(mMockDcApn.connect(eq(APN_TYPE.getType()))).thenReturn(true);
        when(mMockTimerInterface.startTimer(
                eq(SscNetConnection.PDN_CONNECTION_TIMEOUT_TIMER),
                any(TimerInterface.Listener.class))).thenReturn(TIMER_ID);

        boolean result = mSscNetConnection.connect();

        verify(mMockTimerInterface).startTimer(
                eq(SscNetConnection.PDN_CONNECTION_TIMEOUT_TIMER),
                any(TimerInterface.Listener.class));

        assertEquals(Long.valueOf(TIMER_ID),
                mSscNetConnection.mTimerIdTable.get(SscNetConnection.EVENT_PDN_REQUEST_TIMEOUT));
        assertEquals(true, result);
    }

    @Test
    public void disconnect_apnTypeNull() {
        mSscNetConnection.init(null);

        mSscNetConnection.disconnect();

        verify(mMockDcApn, never()).disconnect(APN_TYPE.getType());
    }

    @Test
    public void disconnect_getDcApnNull() {
        DcFactory.setObjects(SLOT_0, null);

        mSscNetConnection.disconnect();

        verify(mMockDcApn, never()).disconnect(APN_TYPE.getType());
    }

    @Test
    public void disconnect_requestDisconnection() {
        long timerIdForRequestTimeout = 2L;
        long timerIdForConnectionExpired = 3L;
        mSscNetConnection.mTimerIdTable.put(SscNetConnection.EVENT_PDN_REQUEST_TIMEOUT,
                timerIdForRequestTimeout);
        mSscNetConnection.mTimerIdTable.put(SscNetConnection.EVENT_PDN_CONNECTION_EXPIRED,
                timerIdForConnectionExpired);

        mSscNetConnection.disconnect();

        verify(mMockTimerInterface).stopTimer(timerIdForRequestTimeout);
        verify(mMockTimerInterface).stopTimer(timerIdForConnectionExpired);
        verify(mMockDcApn).disconnect(eq(APN_TYPE.getType()));
        assertEquals(null,
                mSscNetConnection.mTimerIdTable.get(SscNetConnection.EVENT_PDN_REQUEST_TIMEOUT));
        assertEquals(null,
                mSscNetConnection.mTimerIdTable.get(SscNetConnection.EVENT_PDN_CONNECTION_EXPIRED));
    }

    @Test
    public void getNetwork_typeMobile() {
        when(mMockDcApn.getIpcanCategory(APN_TYPE.getType()))
                .thenReturn(IApn.IPCAN_CATEGORY_MOBILE);
        when(mMockDcNetWatcher.getNetworkType()).thenReturn(TelephonyManager.NETWORK_TYPE_LTE);

        int networkType = mSscNetConnection.getNetworkType();

        assertEquals(TelephonyManager.NETWORK_TYPE_LTE, networkType);
    }

    @Test
    public void getNetwork_typeIwlan() {
        when(mMockDcApn.getIpcanCategory(APN_TYPE.getType())).thenReturn(IApn.IPCAN_CATEGORY_WLAN);

        int networkType = mSscNetConnection.getNetworkType();

        assertEquals(TelephonyManager.NETWORK_TYPE_IWLAN, networkType);
        verify(mMockDcNetWatcher, never()).getNetworkType();
    }

    @Test
    public void refreshConnectionTimer_stopPreviousTimer() {
        long timerIdForConnectionExpired = TIMER_ID + 1; // different value from TIMER_ID
        mSscNetConnection.mTimerIdTable.put(SscNetConnection.EVENT_PDN_CONNECTION_EXPIRED,
                timerIdForConnectionExpired);

        when(mMockTimerInterface.startTimer(
                eq(INACTIVITY_TIME_SEC * 1000L), any(TimerInterface.Listener.class)))
                .thenReturn(TIMER_ID);

        mSscNetConnection.refreshConnectionTimer();

        verify(mMockTimerInterface).stopTimer(timerIdForConnectionExpired);
        verify(mMockTimerInterface).startTimer(
                eq(INACTIVITY_TIME_SEC * 1000L), any(TimerInterface.Listener.class));
        assertEquals(Long.valueOf(TIMER_ID),
                mSscNetConnection.mTimerIdTable.get(SscNetConnection.EVENT_PDN_CONNECTION_EXPIRED));
    }

    @Test
    public void onIpcanCategoryChanged() {
        verify(mMockApn).addListener(mApnStateListenerCaptor.capture());
        ApnStateListener apnStateListener = mApnStateListenerCaptor.getValue();
        assertNotNull(apnStateListener);

        apnStateListener.onIpcanCategoryChanged(APN_TYPE.getType(), IApn.IPCAN_CATEGORY_WLAN);

        verify(mMockSscTransactionHandler).sendMessageAtTime(mMessageCaptor.capture(), anyLong());
        Message msg = mMessageCaptor.getValue();
        assertNotNull(msg);
        assertEquals(SscNetConnection.EVENT_PDN_IPCAN_CHANGED, msg.what);
    }

    @Test
    public void handleMessage_connectionConnected() {
        long timerIdForRequestTimeout = TIMER_ID + 1;
        mSscNetConnection.mTimerIdTable.put(SscNetConnection.EVENT_PDN_REQUEST_TIMEOUT,
                timerIdForRequestTimeout);

        when(mMockTimerInterface.startTimer(
                eq(INACTIVITY_TIME_SEC * 1000L), any(TimerInterface.Listener.class)))
                .thenReturn(TIMER_ID);

        IDcNetWatcher.NotiObj notiObj = new IDcNetWatcher.NotiObj(EApnType.XCAP,
                EDataState.DATA_STATE_CONNECTED, -1);
        AsyncResult ar = new AsyncResult(null, notiObj, null);
        triggerEventHandler(SscNetConnection.EVENT_PDN_DATA_STATE_CHANGED, ar);

        verify(mMockTimerInterface).stopTimer(timerIdForRequestTimeout);
        verify(mMockTimerInterface).startTimer(
                eq(INACTIVITY_TIME_SEC * 1000L), any(TimerInterface.Listener.class));
        assertEquals(Long.valueOf(TIMER_ID),
                mSscNetConnection.mTimerIdTable.get(SscNetConnection.EVENT_PDN_CONNECTION_EXPIRED));
        verify(mMockSscTransactionHandler).sendMessageAtTime(mMessageCaptor.capture(), anyLong());
        Message msg = mMessageCaptor.getValue();
        assertNotNull(msg);
        assertEquals(SscNetConnection.EVENT_PDN_CONNECTED, msg.what);
    }

    @Test
    public void handleMessage_connectionDisconnected() {
        long timerIdForRequestTimeout = TIMER_ID + 1;
        mSscNetConnection.mTimerIdTable.put(SscNetConnection.EVENT_PDN_REQUEST_TIMEOUT,
                timerIdForRequestTimeout);

        when(mMockTimerInterface.startTimer(
                eq(SscNetConnection.DISCONNECTION_DELAY), any(TimerInterface.Listener.class)))
                .thenReturn(TIMER_ID);

        IDcNetWatcher.NotiObj notiObj = new IDcNetWatcher.NotiObj(EApnType.XCAP,
                EDataState.DATA_STATE_DISCONNECTED, -1);
        AsyncResult ar = new AsyncResult(null, notiObj, null);
        triggerEventHandler(SscNetConnection.EVENT_PDN_DATA_STATE_CHANGED, ar);

        verify(mMockTimerInterface).startTimer(
                eq(SscNetConnection.DISCONNECTION_DELAY), any(TimerInterface.Listener.class));
        assertEquals(Long.valueOf(TIMER_ID),
                mSscNetConnection.mTimerIdTable.get(SscNetConnection.EVENT_PDN_CONNECTION_EXPIRED));
        verify(mMockSscTransactionHandler).sendMessageAtTime(mMessageCaptor.capture(), anyLong());
        Message msg = mMessageCaptor.getValue();
        assertNotNull(msg);
        assertEquals(SscNetConnection.EVENT_PDN_DISCONNECTED, msg.what);
    }

    @Test
    public void handleMessage_connectionFailed() {
        int smCause = 33;
        long timerIdForRequestTimeout = 2L;
        mSscNetConnection.mTimerIdTable.put(SscNetConnection.EVENT_PDN_REQUEST_TIMEOUT,
                timerIdForRequestTimeout);
        SscServiceStateAgent.getInstance().setSscServiceState(SLOT_0, mMockSscServiceState);

        IDcNetWatcher.NotiObj notiObj = new IDcNetWatcher.NotiObj(EApnType.XCAP,
                EDataState.DATA_STATE_CONNECT_FAILED, smCause);
        AsyncResult ar = new AsyncResult(null, notiObj, null);
        triggerEventHandler(SscNetConnection.EVENT_PDN_CONNECTION_FAILED, ar);

        verify(mMockSscServiceState).setPdnConnectionFailed(smCause);
        verify(mMockSscTransactionHandler).sendMessageAtTime(mMessageCaptor.capture(), anyLong());
        Message msg = mMessageCaptor.getValue();
        assertNotNull(msg);
        assertEquals(SscNetConnection.EVENT_PDN_CONNECTION_FAILED, msg.what);

        verify(mMockTimerInterface).stopTimer(timerIdForRequestTimeout);
        verify(mMockDcApn).disconnect(eq(APN_TYPE.getType()));
        assertEquals(null,
                mSscNetConnection.mTimerIdTable.get(SscNetConnection.EVENT_PDN_REQUEST_TIMEOUT));
    }

    @Test
    public void handleMessage_connectionRequestTimeout() {
        long timerIdForRequestTimeout = 2L;
        mSscNetConnection.mTimerIdTable.put(SscNetConnection.EVENT_PDN_REQUEST_TIMEOUT,
                timerIdForRequestTimeout);

        triggerEventHandler(SscNetConnection.EVENT_PDN_REQUEST_TIMEOUT, null);

        verify(mMockSscTransactionHandler).sendMessageAtTime(mMessageCaptor.capture(), anyLong());
        Message msg = mMessageCaptor.getValue();
        assertNotNull(msg);
        assertEquals(SscNetConnection.EVENT_PDN_REQUEST_TIMEOUT, msg.what);

        verify(mMockTimerInterface).stopTimer(timerIdForRequestTimeout);
        verify(mMockDcApn).disconnect(eq(APN_TYPE.getType()));
        assertEquals(null,
                mSscNetConnection.mTimerIdTable.get(SscNetConnection.EVENT_PDN_REQUEST_TIMEOUT));
    }

    @Test
    public void handleMessage_connectionTimerExpired() {
        long timerIdForConnectionExpired = 3L;
        mSscNetConnection.mTimerIdTable.put(SscNetConnection.EVENT_PDN_CONNECTION_EXPIRED,
                timerIdForConnectionExpired);

        triggerEventHandler(SscNetConnection.EVENT_PDN_CONNECTION_EXPIRED, null);

        verify(mMockTimerInterface).stopTimer(timerIdForConnectionExpired);
        verify(mMockDcApn).disconnect(eq(APN_TYPE.getType()));
        assertEquals(null,
                mSscNetConnection.mTimerIdTable.get(SscNetConnection.EVENT_PDN_CONNECTION_EXPIRED));
    }

    @Test
    public void startTimer_invalidTimerId() {
        long timerIdForConnectionExpired = TIMER_ID + 1; // different value from TIMER_ID
        mSscNetConnection.mTimerIdTable.put(SscNetConnection.EVENT_PDN_CONNECTION_EXPIRED,
                timerIdForConnectionExpired);

        when(mMockTimerInterface.startTimer(
                eq(INACTIVITY_TIME_SEC * 1000L), any(TimerInterface.Listener.class)))
                .thenReturn(TIMER_ID);

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
        protected TimerInterface getTimerInterface() {
            return mMockTimerInterface;
        }
    }
}
