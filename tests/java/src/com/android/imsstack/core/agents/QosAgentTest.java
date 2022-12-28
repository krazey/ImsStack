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

package com.android.imsstack.core.agents;

import static org.junit.Assert.assertEquals;
import static org.junit.Assert.assertNotNull;
import static org.junit.Assert.assertTrue;
import static org.mockito.ArgumentMatchers.anyInt;
import static org.mockito.ArgumentMatchers.eq;
import static org.mockito.Mockito.times;
import static org.mockito.Mockito.verify;
import static org.mockito.Mockito.when;

import android.net.ConnectivityManager;
import android.net.LinkAddress;
import android.net.LinkProperties;
import android.net.Network;
import android.util.Pair;

import com.android.imsstack.ContextFixture;
import com.android.imsstack.core.agents.dcm.DcFactory;
import com.android.imsstack.core.agents.dcmif.EApnType;
import com.android.imsstack.core.agents.dcmif.IDc;
import com.android.imsstack.core.agents.dcmif.IDcApn;
import com.android.imsstack.util.AppContext;

import org.junit.After;
import org.junit.Before;
import org.junit.Test;
import org.junit.runner.RunWith;
import org.junit.runners.JUnit4;
import org.mockito.Mock;
import org.mockito.MockitoAnnotations;

import java.net.DatagramSocket;
import java.net.InetAddress;
import java.net.InetSocketAddress;
import java.util.HashMap;

@RunWith(JUnit4.class)
public class QosAgentTest {
    private static final int SLOT0 = 0;
    private ConnectivityManager mConnectivityManager;
    private ContextFixture mContext;
    private LinkProperties mLinkProperties;
    private QosAgent mQosAgent;

    @Mock private Network mMockNetwork;
    @Mock private IDcApn mMockDcApn;
    @Mock private DatagramSocket mMockRtpSocket;
    @Mock private DatagramSocket mMockRtcpSocket;

    private static final String LOCAL_RTP_ADDRESS = "127.0.0.1";
    private static final String LOCAL_LINK_ADDRESS = "127.0.0.1/24";
    private static final int LOCAL_RTP_PORT = 50010;
    private static final String REMOTE_RTP_ADDRESS = "127.0.0.10";
    private static final int REMOTE_RTP_PORT = 1240;

    @Before
    public void setUp() throws Exception {
        MockitoAnnotations.initMocks(this);

        mContext = new ContextFixture();
        mQosAgent = new QosAgent(SLOT0);
        AppContext.init(mContext.getTestDouble());

        mConnectivityManager = mContext.getTestDouble().getSystemService(ConnectivityManager.class);
        mLinkProperties = new LinkProperties();
        mLinkProperties.addLinkAddress(new LinkAddress(LOCAL_LINK_ADDRESS));
        when(mConnectivityManager.getLinkProperties(eq(mMockNetwork))).thenReturn(mLinkProperties);

        HashMap<Integer, IDc> dcObjects = new HashMap<>();
        dcObjects.put(DcFactory.APN, mMockDcApn);
        DcFactory.setObjects(SLOT0, dcObjects);
        when(mMockDcApn.getNetworkByCapability(anyInt())).thenReturn(mMockNetwork);
    }

    @After
    public void tearDown() throws Exception {
        AppContext.deinit();
        mContext = null;
        mQosAgent = null;
        mMockNetwork = null;
        mMockDcApn = null;
        DcFactory.setObjects(SLOT0, null);
    }

    @Test
    public void testCreateQosConnectionWithoutRemoteInfo() throws Exception {
        InetAddress localAddr = InetAddress.getByName(LOCAL_RTP_ADDRESS);

        Pair<DatagramSocket, DatagramSocket> retSocket =
                mQosAgent.createQosConnection(LOCAL_RTP_ADDRESS, LOCAL_RTP_PORT);

        verify(mConnectivityManager).getLinkProperties(eq(mMockNetwork));
        verify(mMockDcApn).getNetworkByCapability(eq(EApnType.IMS.getType()));

        assertNotNull(retSocket);
        assertEquals(new InetSocketAddress(
                localAddr, LOCAL_RTP_PORT), retSocket.first.getLocalSocketAddress());

        mQosAgent.destroyQosConnection(retSocket.first, retSocket.second);
    }

    @Test
    public void testCreateQosConnectionWithRemoteInfo() throws Exception {
        InetAddress localAddr = InetAddress.getByName(LOCAL_RTP_ADDRESS);

        Pair<DatagramSocket, DatagramSocket> retSocket =
                mQosAgent.createQosConnection(LOCAL_RTP_ADDRESS, LOCAL_RTP_PORT, REMOTE_RTP_ADDRESS,
                REMOTE_RTP_PORT);

        verify(mConnectivityManager).getLinkProperties(eq(mMockNetwork));
        verify(mMockDcApn).getNetworkByCapability(eq(EApnType.IMS.getType()));

        assertNotNull(retSocket);
        assertEquals(new InetSocketAddress(
                localAddr, LOCAL_RTP_PORT), retSocket.first.getLocalSocketAddress());
        assertEquals(new InetSocketAddress(
                localAddr, LOCAL_RTP_PORT + 1), retSocket.second.getLocalSocketAddress());

        mQosAgent.destroyQosConnection(retSocket.first, retSocket.second);
    }

    @Test
    public void testUpdateQosConnection() throws Exception {
        Pair<DatagramSocket, DatagramSocket> retSocket =
                mQosAgent.createQosConnection(LOCAL_RTP_ADDRESS, LOCAL_RTP_PORT);

        boolean retResult =
                mQosAgent.updateQosConnection(retSocket.first, retSocket.second, REMOTE_RTP_ADDRESS,
                REMOTE_RTP_PORT);

        assertTrue(retResult);
        verify(mConnectivityManager, times(2)).getLinkProperties(eq(mMockNetwork));
        verify(mMockDcApn, times(2)).getNetworkByCapability(eq(EApnType.IMS.getType()));

        mQosAgent.destroyQosConnection(retSocket.first, retSocket.second);
    }

    @Test
    public void testDestroyQosConnection() throws Exception {
        mQosAgent.destroyQosConnection(mMockRtpSocket, mMockRtcpSocket);

        verify(mMockRtpSocket, times(1)).close();
        verify(mMockRtcpSocket, times(1)).close();
    }
}
