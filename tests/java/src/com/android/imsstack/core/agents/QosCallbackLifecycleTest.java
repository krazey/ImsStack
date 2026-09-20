/*
 * Copyright (C) 2026 The Android Open Source Project
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
import static org.junit.Assert.assertFalse;
import static org.junit.Assert.assertTrue;
import static org.mockito.ArgumentMatchers.any;
import static org.mockito.ArgumentMatchers.anyInt;
import static org.mockito.Mockito.doAnswer;
import static org.mockito.Mockito.doNothing;
import static org.mockito.Mockito.doThrow;
import static org.mockito.Mockito.never;
import static org.mockito.Mockito.times;
import static org.mockito.Mockito.verify;
import static org.mockito.Mockito.when;

import android.net.ConnectivityManager;
import android.net.LinkAddress;
import android.net.LinkProperties;
import android.net.Network;
import android.net.QosCallback;
import android.net.QosCallbackException;
import android.net.QosSession;
import android.net.QosSocketInfo;
import android.telephony.data.EpsBearerQosSessionAttributes;
import android.telephony.data.NrQosSessionAttributes;
import android.util.Pair;

import com.android.imsstack.ContextFixture;
import com.android.imsstack.base.AppContext;
import com.android.imsstack.core.agents.QosAgent.ImsQosCallback;
import com.android.imsstack.core.agents.dcm.DcFactory;
import com.android.imsstack.core.agents.dcmif.IDcApn;

import org.junit.After;
import org.junit.Before;
import org.junit.Test;
import org.junit.runner.RunWith;
import org.junit.runners.JUnit4;
import org.mockito.ArgumentCaptor;
import org.mockito.Mock;
import org.mockito.MockitoAnnotations;

import java.net.DatagramSocket;
import java.net.InetAddress;
import java.net.InetSocketAddress;
import java.util.ArrayList;
import java.util.List;
import java.util.concurrent.Executor;

@RunWith(JUnit4.class)
public class QosCallbackLifecycleTest {
    private static final int SLOT = 0;
    private static final String REMOTE = "127.0.0.2";
    private static final int PORT = 1240;
    private final List<Pair<DatagramSocket, DatagramSocket>> mSockets = new ArrayList<>();
    private final ArgumentCaptor<QosCallback> mCallbacks =
            ArgumentCaptor.forClass(QosCallback.class);
    private QosAgent mAgent;
    private ConnectivityManager mConnectivityManager;
    private InetSocketAddress mRemote;
    @Mock private Network mNetwork;
    @Mock private IDcApn mDcApn;
    @Mock private ImsQosCallback mListener;

    @Before
    public void setUp() throws Exception {
        MockitoAnnotations.initMocks(this);
        ContextFixture fixture = new ContextFixture();
        AppContext.init(fixture.getTestDouble());
        mConnectivityManager = fixture.getTestDouble().getSystemService(ConnectivityManager.class);
        LinkProperties lp = new LinkProperties();
        lp.addLinkAddress(new LinkAddress("127.0.0.1/8"));
        when(mConnectivityManager.getLinkProperties(mNetwork)).thenReturn(lp);
        DcFactory.setDcAgent(IDcApn.class, mDcApn, SLOT);
        when(mDcApn.getNetworkByCapability(anyInt())).thenReturn(mNetwork);
        mRemote = new InetSocketAddress(InetAddress.getByName(REMOTE), PORT);
        mAgent = new QosAgent(SLOT);
        mAgent.setCallback(mListener);
    }

    @After
    public void tearDown() {
        for (Pair<DatagramSocket, DatagramSocket> sockets : mSockets) {
            mAgent.destroyQosConnection(sockets.first, sockets.second);
        }
        DcFactory.setDcAgent(IDcApn.class, null, SLOT);
        AppContext.deinit();
    }

    private Pair<DatagramSocket, DatagramSocket> sockets() throws Exception {
        InetAddress loopback = InetAddress.getByName("127.0.0.1");
        DatagramSocket rtp = new DatagramSocket(new InetSocketAddress(loopback, 0));
        DatagramSocket rtcp;
        try {
            rtcp = new DatagramSocket(new InetSocketAddress(loopback, 0));
        } catch (Exception e) {
            rtp.close();
            throw e;
        }
        Pair<DatagramSocket, DatagramSocket> pair = new Pair<>(rtp, rtcp);
        mSockets.add(pair);
        return pair;
    }

    private boolean register(Pair<DatagramSocket, DatagramSocket> pair) {
        return mAgent.updateQosConnection(pair.first, pair.second, REMOTE, PORT, false);
    }

    private QosCallback callback(int count) {
        verify(mConnectivityManager, times(count)).registerQosCallback(
                any(QosSocketInfo.class), any(Executor.class), mCallbacks.capture());
        return mCallbacks.getValue();
    }

    private void available(QosCallback callback, int id) {
        callback.onQosSessionAvailable(new QosSession(id, QosSession.TYPE_EPS_BEARER),
                new EpsBearerQosSessionAttributes(1, 41, 41, 39, 39, List.of()));
    }

    @Test
    public void registrationFailureCanBeRetriedWithoutReconnectingThePeer() throws Exception {
        Pair<DatagramSocket, DatagramSocket> pair = sockets();
        doThrow(new IllegalStateException("temporary registration failure"))
                .when(mConnectivityManager).registerQosCallback(any(), any(), any());
        assertFalse(register(pair));
        assertFalse(pair.first.isClosed());
        assertEquals(mRemote, pair.first.getRemoteSocketAddress());
        doNothing().when(mConnectivityManager).registerQosCallback(any(), any(), any());
        assertTrue(register(pair));
        available(callback(2), 1);
        verify(mListener).onNotifyQosConnectionAvailable(mRemote);
    }

    @Test
    public void repeatedRequestDoesNotRegisterAnotherCallback() throws Exception {
        Pair<DatagramSocket, DatagramSocket> pair = sockets();
        assertTrue(register(pair));
        assertTrue(register(pair));
        callback(1);
        verify(mListener, never()).onNotifyQosConnectionAvailable(any());
    }

    @Test
    public void socketsSharingRemotePortHaveIndependentLifetimes() throws Exception {
        Pair<DatagramSocket, DatagramSocket> first = sockets();
        Pair<DatagramSocket, DatagramSocket> second = sockets();
        assertTrue(register(first));
        QosCallback old = callback(1);
        assertTrue(register(second));
        QosCallback current = callback(2);
        mAgent.destroyQosConnection(first.first, first.second);
        verify(mConnectivityManager).unregisterQosCallback(old);
        verify(mConnectivityManager, never()).unregisterQosCallback(current);
        assertFalse(second.first.isClosed());
        available(old, 1);
        available(current, 2);
        verify(mListener, times(1)).onNotifyQosConnectionAvailable(mRemote);
    }

    @Test
    public void replacedCallbackCannotReportAgainstTheNewPeer() throws Exception {
        Pair<DatagramSocket, DatagramSocket> pair = sockets();
        assertTrue(register(pair));
        QosCallback old = callback(1);
        assertTrue(mAgent.updateQosConnection(pair.first, pair.second, "127.0.0.3", PORT, true));
        QosCallback current = callback(2);
        available(old, 1);
        old.onQosSessionLost(new QosSession(1, QosSession.TYPE_EPS_BEARER));
        old.onError(new QosCallbackException("obsolete"));
        verify(mListener, never()).onNotifyQosConnectionAvailable(any());
        verify(mListener, never()).onNotifyQosConnectionLost(any());
        verify(mListener, never()).onNotifyQosCallbackError(any(), any());
        available(current, 2);
        verify(mListener).onNotifyQosConnectionAvailable(
                new InetSocketAddress(InetAddress.getByName("127.0.0.3"), PORT));
    }

    @Test
    public void asyncErrorAllowsReregistrationWithoutReportingBearerLoss() throws Exception {
        Pair<DatagramSocket, DatagramSocket> pair = sockets();
        assertTrue(register(pair));
        QosCallback old = callback(1);
        old.onError(new QosCallbackException("unregistered"));
        verify(mListener).onNotifyQosCallbackError(pair.first, mRemote);
        verify(mListener, never()).onNotifyQosConnectionLost(any());
        assertTrue(register(pair));
        QosCallback current = callback(2);
        available(old, 1);
        available(current, 2);
        verify(mListener, times(1)).onNotifyQosConnectionAvailable(mRemote);
    }

    @Test
    public void immediateAsyncErrorDoesNotReportRegistrationSuccess() throws Exception {
        Pair<DatagramSocket, DatagramSocket> pair = sockets();
        doAnswer(invocation -> {
            QosCallback callback = invocation.getArgument(2);
            callback.onError(new QosCallbackException("immediate error"));
            return null;
        }).when(mConnectivityManager).registerQosCallback(any(), any(), any());
        assertFalse(register(pair));
    }

    @Test
    public void onlyLosingLastAcceptedSessionReportsLoss() throws Exception {
        Pair<DatagramSocket, DatagramSocket> pair = sockets();
        assertTrue(register(pair));
        QosCallback callback = callback(1);
        available(callback, 7);
        callback.onQosSessionAvailable(new QosSession(7, QosSession.TYPE_NR_BEARER),
                new NrQosSessionAttributes(1, 11, 41, 41, 39, 39, 20, List.of()));
        callback.onQosSessionLost(new QosSession(99, QosSession.TYPE_EPS_BEARER));
        callback.onQosSessionLost(new QosSession(7, QosSession.TYPE_EPS_BEARER));
        verify(mListener, never()).onNotifyQosConnectionLost(any());
        callback.onQosSessionLost(new QosSession(7, QosSession.TYPE_NR_BEARER));
        callback.onQosSessionLost(new QosSession(7, QosSession.TYPE_NR_BEARER));
        verify(mListener, times(1)).onNotifyQosConnectionAvailable(mRemote);
        verify(mListener, times(1)).onNotifyQosConnectionLost(mRemote);
    }

    @Test
    public void invalidAndClosedSocketsAreNotRegistered() throws Exception {
        Pair<DatagramSocket, DatagramSocket> pair = sockets();
        assertFalse(mAgent.updateQosConnection(pair.first, pair.second, null, PORT, false));
        assertFalse(mAgent.updateQosConnection(pair.first, pair.second, REMOTE, 65535, false));
        pair.first.close();
        assertFalse(register(pair));
        verify(mConnectivityManager, never()).registerQosCallback(any(), any(), any());
    }
}
