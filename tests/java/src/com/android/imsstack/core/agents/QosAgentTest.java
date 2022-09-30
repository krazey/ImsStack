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

import android.util.Pair;

import com.android.imsstack.util.ImsLog;

import org.junit.After;
import org.junit.Before;
import org.junit.Test;
import org.junit.runner.RunWith;
import org.junit.runners.JUnit4;

import java.io.IOException;
import java.net.DatagramSocket;
import java.net.InetAddress;
import java.net.InetSocketAddress;

@RunWith(JUnit4.class)
public class QosAgentTest {

    public static final String TAG = "QosAgentTest";
    private int mSlotId;
    private QosAgent mQosAgent;

    @Before
    public void setUp() throws Exception {
        mSlotId = 0;
        ImsLog.e(mSlotId, "Start of the Unit Test for QosAgentTest");

        mQosAgent = new QosAgent(mSlotId);
    }

    @Test
    public void testCreateQosConnectionWithoutRemoteInfo() {
        final String strLocalAddr = "111.11.11.11";
        final int localPort = 1111;

        InetAddress localAddress = InetAddress.parseNumericAddress(strLocalAddr);
        final Pair<DatagramSocket, DatagramSocket> retSocket =
                mQosAgent.createQosConnection(strLocalAddr, localPort);

        assertEquals(new InetSocketAddress(
                localAddress, localPort), retSocket.first.getLocalSocketAddress());
        assertEquals(new InetSocketAddress(
                localAddress, localPort + 1), retSocket.second.getLocalSocketAddress());
    }

    @Test
    public void testCreateQosConnectionWithRemoteInfo() {
        final String strLocalAddr = "111.11.11.11";
        final int localPort = 1111;
        final String strRemoteAddr = "122.22.22.22";
        final int remotePort = 2222;

        InetAddress localAddress = InetAddress.parseNumericAddress(strLocalAddr);
        InetAddress remoteAddress = InetAddress.parseNumericAddress(strRemoteAddr);
        final Pair<DatagramSocket, DatagramSocket> retSocket =
                mQosAgent.createQosConnection(strLocalAddr, localPort, strRemoteAddr, remotePort);

        assertEquals(new InetSocketAddress(
                localAddress, localPort), retSocket.first.getLocalSocketAddress());
        assertEquals(new InetSocketAddress(
                remoteAddress, remotePort), retSocket.first.getRemoteSocketAddress());
        assertEquals(new InetSocketAddress(
                localAddress, localPort + 1), retSocket.second.getLocalSocketAddress());
        assertEquals(new InetSocketAddress(
                remoteAddress, remotePort + 1), retSocket.second.getRemoteSocketAddress());
    }

    @Test
    public void testUpdateQosConnection() {
        final String strLocalAddr = "111.11.11.11";
        final int localPort = 1111;
        final String strRemoteAddr = "122.22.22.22";
        final int remotePort = 2222;

        try {
            InetAddress localAddress = InetAddress.parseNumericAddress(strLocalAddr);
            InetAddress remoteAddress = InetAddress.parseNumericAddress(strRemoteAddr);
            DatagramSocket rtpSocket = new DatagramSocket(localPort, localAddress);
            DatagramSocket rtcpSocket = new DatagramSocket(remotePort, remoteAddress);

            final boolean mRetResult =
                    mQosAgent.updateQosConnection(rtpSocket, rtcpSocket, strRemoteAddr, remotePort);

            assertEquals(true, mRetResult);
        } catch (IOException e) {
            e.printStackTrace();
        }
    }

    @Test
    public void testdestroyQosConnection() {
        final String strLocalAddr = "111.11.11.11";
        final int tpLocalPort = 1111;

        try {
            InetAddress localAddress = InetAddress.parseNumericAddress(strLocalAddr);

            DatagramSocket rtpSocket = new DatagramSocket(tpLocalPort, localAddress);
            DatagramSocket rtcpSocket = new DatagramSocket(tpLocalPort + 1, localAddress);

            mQosAgent.destroyQosConnection(rtpSocket, rtcpSocket);

            assertEquals("", rtpSocket);
            assertEquals("", rtcpSocket);
        } catch (IOException e) {
            e.printStackTrace();
        }
    }

    @After
    public void tearDown() throws Exception {
        mSlotId = 0;
        ImsLog.e(mSlotId, "End of the Unit Test for mQosAgentTest");
    }
}
