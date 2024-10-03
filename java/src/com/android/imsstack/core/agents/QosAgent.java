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

import android.annotation.NonNull;
import android.net.LinkProperties;
import android.net.Network;
import android.net.QosCallback;
import android.net.QosCallbackException;
import android.net.QosSession;
import android.net.QosSessionAttributes;
import android.net.QosSocketInfo;
import android.telephony.data.EpsBearerQosSessionAttributes;
import android.telephony.data.NrQosSessionAttributes;
import android.util.Pair;
import android.util.SparseArray;

import com.android.imsstack.base.AppContext;
import com.android.imsstack.base.SystemServiceProxy.ConnectivityManagerProxy;
import com.android.imsstack.core.agents.dcm.DcFactory;
import com.android.imsstack.core.agents.dcmif.EApnType;
import com.android.imsstack.core.agents.dcmif.IDcApn;
import com.android.imsstack.util.ImsLog;
import com.android.imsstack.util.ImsUtils;

import java.io.IOException;
import java.net.DatagramSocket;
import java.net.InetAddress;
import java.net.InetSocketAddress;
import java.util.ArrayList;
import java.util.List;

/**
 * Class responsible for registering and receiving qoscallback
 */
public class QosAgent {
    /**
    * Interface for ImsMediaImpl
    */
    public interface ImsQosCallback {
        /**
        * Notify that QoS is available
        */
        void onNotifyQosConnectionAvailable(InetSocketAddress remoteAddress);
        /**
        * Notify that QoS is lost
        */
        void onNotifyQosConnectionLost(InetSocketAddress remoteAddress);
    }

    private class QosSocket extends QosCallback {

        /**
        * LTE EPS Session.
        */
        public static final int TYPE_EPS_BEARER = 1;
        /**
        * NR Session.
        */
        public static final int TYPE_NR_BEARER = 2;

        public DatagramSocket mSocket;

        QosSocket(DatagramSocket socket) {
            mSocket = socket;
        }

        public void close() {
            if (mSocket != null) {
                mSocket.close();
                mSocket = null;
            }
        }

        @Override
        public void onError(final QosCallbackException exception) {
            ImsLog.d(this, mSlotId, "onError: " + exception.toString());
        }

        @Override
        public void onQosSessionAvailable(
                QosSession session, QosSessionAttributes sessionAttributes) {

            int qosIdentifier = 0;

            ImsLog.d(this, mSlotId, "onQosSessionAvailable - QosSession: "
                    + session + ", QosSessionAttributes: " + sessionAttributes);

            if (session.getSessionType() == TYPE_EPS_BEARER) {
                EpsBearerQosSessionAttributes attributes =
                        (EpsBearerQosSessionAttributes) sessionAttributes;

                qosIdentifier = attributes.getQosIdentifier();

                ImsLog.d(this, mSlotId, "EpsBearerQosSessionAttributes - qci: " + qosIdentifier
                        + ", MAXUplink: " + attributes.getMaxUplinkBitRateKbps()
                        + ", MAXDownlink: " + attributes.getMaxDownlinkBitRateKbps()
                        + ", gbrUplink: " + attributes.getGuaranteedUplinkBitRateKbps()
                        + ", gbrDownlink: " + attributes.getGuaranteedDownlinkBitRateKbps());
            } else if (session.getSessionType() == TYPE_NR_BEARER) {
                NrQosSessionAttributes attributes = (NrQosSessionAttributes) sessionAttributes;

                qosIdentifier = attributes.getQosIdentifier();
                ImsLog.d(this, mSlotId, "NrQosSessionAttributes - qci: " + qosIdentifier
                        + ", MAXUplink: " + attributes.getMaxUplinkBitRateKbps()
                        + ", MAXDownlink: " + attributes.getMaxDownlinkBitRateKbps()
                        + ", gbrUplink: " + attributes.getGuaranteedUplinkBitRateKbps()
                        + ", gbrDownlink: " + attributes.getGuaranteedDownlinkBitRateKbps());
            }

            if (qosIdentifier == 0) {
                ImsLog.d(this, mSlotId, "Invalid QCI value");
            } else {
                InetSocketAddress remoteAddress =
                        (InetSocketAddress) mSocket.getRemoteSocketAddress();

                notifyQosConnectionAvailable(remoteAddress);
            }
        }

        @Override
        public void onQosSessionLost(final QosSession session) {
            ImsLog.d(this, mSlotId, "onQosSessionLost - QosSession: " + session);

            InetSocketAddress remoteAddress = (InetSocketAddress) mSocket.getRemoteSocketAddress();

            notifyQosConnectionLost(remoteAddress);
        }
    }

    private final int mSlotId;
    private ImsQosCallback mCallback;

    private final SparseArray<QosSocket> mSockets = new SparseArray<>(10);

    public QosAgent(int slotId) {
        mSlotId = slotId;
    }

    public void setCallback(ImsQosCallback callback) {
        this.mCallback = callback;
    }

    private void notifyQosConnectionAvailable(InetSocketAddress remoteAddress) {
        mCallback.onNotifyQosConnectionAvailable(remoteAddress);
    }

    private void notifyQosConnectionLost(InetSocketAddress remoteAddress) {
        mCallback.onNotifyQosConnectionLost(remoteAddress);
    }

    /**
    * Create datagramsockets for rtpsocket/rtcpsocket without remote address and port
    */
    public Pair<DatagramSocket, DatagramSocket> createQosConnection(
            String localAddress, int localPort) {
        InetAddress localAddr = createInetAddress(localAddress);
        Network network = getNetworkForIpAddress(localAddr);
        ImsLog.d(this, mSlotId, "createQosConnection without remote address");

        if (network == null) {
            return null;
        }

        DatagramSocket rtpSocket = createDatagramSocket(network, localAddr, localPort);
        DatagramSocket rtcpSocket = createDatagramSocket(network, localAddr, (localPort + 1));

        if (rtpSocket == null || rtcpSocket == null) {
            return null;
        }

        return new Pair<>(rtpSocket, rtcpSocket);
    }

    /**
    * Create datagramsockets for rtpsocket/rtcpsocket with remote address and port
    * Register QosCallback
    */
    public Pair<DatagramSocket, DatagramSocket> createQosConnection(
            String localAddress, int localPort, String remoteAddress, int remotePort) {
        InetAddress localAddr = createInetAddress(localAddress);
        Network network = getNetworkForIpAddress(localAddr);

        if (network == null) {
            ImsLog.e(this, mSlotId, "Network not found");
            return null;
        }

        ImsLog.d(this, mSlotId, "createQosConnection with remote address");

        InetAddress remoteAddr = createInetAddress(remoteAddress);
        DatagramSocket rtpSocket = createDatagramSocket(network, localAddr, localPort);
        DatagramSocket rtcpSocket = createDatagramSocket(network, localAddr, (localPort + 1));

        if (rtpSocket == null || rtcpSocket == null) {
            return null;
        }

        if (!remoteAddress.isEmpty() && (remotePort > 0)) {
            rtpSocket.connect(remoteAddr, remotePort);
            rtcpSocket.connect(remoteAddr, (remotePort + 1));

            QosSocket qosSocket = new QosSocket(rtpSocket);
            mSockets.put(remotePort, qosSocket);
            registerQosCallback(network, rtpSocket, qosSocket);
        }

        return new Pair<>(rtpSocket, rtcpSocket);
    }

    /**
    * Update rtpsocket/rtcpsocket with remote address and port
    * Register QosCallback
    */
    public boolean updateQosConnection(
            DatagramSocket rtpSocket, DatagramSocket rtcpSocket,
            String remoteAddress, int remotePort) {
        InetAddress remoteAddr = createInetAddress(remoteAddress);

        if (remoteAddr == null) {
            ImsLog.e(this, mSlotId, "Remote address not found; rtpSocket: " + rtpSocket
                    + " remotePort: " + remotePort);
            return false;
        }

        Network network = getNetworkForIpAddress(rtpSocket.getLocalAddress());

        if (network == null) {
            ImsLog.e(this, mSlotId, "Network not found; rtpSocket: " + rtpSocket
                    + " remotePort: " + remotePort);
            return false;
        }

        ImsLog.d(this, mSlotId, "updateQosConnection - rtpSocket: " + rtpSocket
                + " remotePort: " + remotePort);

        if (!remoteAddress.isEmpty() && (remotePort > 0)) {
            rtpSocket.connect(remoteAddr, remotePort);
            rtcpSocket.connect(remoteAddr, (remotePort + 1));

            QosSocket qosSocket = new QosSocket(rtpSocket);
            mSockets.put(remotePort, qosSocket);
            registerQosCallback(network, rtpSocket, qosSocket);
        }

        return true;
    }

    /**
    * Request to close rtpsocket/rtcpsocket
    */
    public void destroyQosConnection(DatagramSocket rtpSocket, DatagramSocket rtcpSocket) {
        if (rtcpSocket != null) {
            ImsUtils.closeQuietly(rtcpSocket);
        }

        if (rtpSocket != null) {
            int remotePort = rtpSocket.getPort();
            QosSocket socket = mSockets.get(remotePort);

            if (socket != null) {
                unregisterQosCallback(socket);
                mSockets.remove(remotePort);
                ImsLog.d(this, mSlotId, "QosSocket closed");
                socket.close();
            } else {
                ImsLog.d(this, mSlotId, "Socket not found");
            }

            ImsUtils.closeQuietly(rtpSocket);
        }
    }

    private DatagramSocket createDatagramSocket(Network network, InetAddress ipAddr, int port) {

        DatagramSocket socket = null;

        ImsLog.d(this, mSlotId, "createDatagramSocket - ipAddr=" + ipAddr + ", port=" + port);

        try {
            socket = new DatagramSocket(null);

            if (socket != null) {
                socket.setReuseAddress(true);
                socket.bind(new InetSocketAddress(ipAddr, port));
                network.bindSocket(socket);
            }
        } catch (IOException e) {
            ImsLog.e(this, mSlotId, "createDatagramSocket: " + e.toString());

            if (socket != null) {
                socket.close();
                socket = null;
            }
        }

        return socket;
    }

    private Network getNetworkForIpAddress(InetAddress addr) {
        ConnectivityManagerProxy cmp = getConnectivityManagerProxy();
        List<Network> networks = getAllNetworks(mSlotId);

        if (networks.isEmpty()) {
            ImsLog.w(this, mSlotId, "No networks");
            return null;
        }

        for (Network network : networks) {
            LinkProperties lp = cmp.getLinkProperties(network);

            if (lp == null) {
                continue;
            }

            List<InetAddress> linkAddrs = lp.getAddresses();
            for (InetAddress linkAddr : linkAddrs) {
                if (addr.equals(linkAddr)) {
                    return network;
                }
            }
        }
        return null;
    }

    private void registerQosCallback(Network network, DatagramSocket socket, QosCallback callback) {
        ConnectivityManagerProxy cmp = getConnectivityManagerProxy();
        ImsLog.d(this, mSlotId, "registerQosCallback: " + callback);
        try {
            QosSocketInfo socketInfo = new QosSocketInfo(network, socket);
            cmp.registerQosCallback(
                    socketInfo, AppContext.getInstance().getMainExecutor(), callback);
        } catch (Throwable t) {
            ImsLog.e(this, mSlotId, "registerQosCallback: " + t.toString());
        }
    }

    private void unregisterQosCallback(QosCallback callback) {
        ConnectivityManagerProxy cmp = getConnectivityManagerProxy();
        ImsLog.d(this, mSlotId, "unregisterQosCallback: " + callback);
        try {
            cmp.unregisterQosCallback(callback);
        } catch (Throwable t) {
            ImsLog.e(this, mSlotId, "unregisterQosCallback: " + t.toString());
        }
    }

    private static InetAddress createInetAddress(String address) {
        try {
            return InetAddress.getByName(address);
        } catch (IOException e) {
            ImsLog.e(null, "getByName: " + e);
        }

        return null;
    }

    private static @NonNull List<Network> getAllNetworks(int slotId) {
        List<Network> allNetworks = new ArrayList<>();
        IDcApn dcApn = DcFactory.getDcAgent(IDcApn.class, slotId);

        if (dcApn != null) {
            Network network = dcApn.getNetworkByCapability(EApnType.IMS.getType());
            if (network != null) {
                allNetworks.add(network);
            }

            network = dcApn.getNetworkByCapability(EApnType.EMERGENCY.getType());
            if (network != null) {
                allNetworks.add(network);
            }
        }

        WifiInterface wifi = AgentFactory.getInstance().getAgent(WifiInterface.class);
        if (wifi != null && wifi.isWifiConnected()) {
            allNetworks.add(wifi.getNetwork());
        }

        return allNetworks;
    }

    private static ConnectivityManagerProxy getConnectivityManagerProxy() {
        return AppContext.getInstance().getSystemServiceProxy(ConnectivityManagerProxy.class);
    }
}
