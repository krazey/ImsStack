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
import java.util.HashSet;
import java.util.IdentityHashMap;
import java.util.List;
import java.util.Map;
import java.util.Set;

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

        /** The callback was unregistered by the framework, not a bearer-loss indication. */
        default void onNotifyQosCallbackError(
                DatagramSocket socket, InetSocketAddress remoteAddress) {}
    }

    private class QosSocket extends QosCallback {
        final DatagramSocket mSocket;
        final Network mNetwork;
        final InetSocketAddress mRemoteAddress;
        // Guarded by mSockets. Include the type since EPS and NR IDs may overlap.
        final Set<Long> mAvailableSessions = new HashSet<>();

        QosSocket(Network network, DatagramSocket socket) {
            mSocket = socket;
            mNetwork = network;
            mRemoteAddress = (InetSocketAddress) socket.getRemoteSocketAddress();
        }

        private boolean isCurrent() {
            return mSockets.get(mSocket) == this;
        }

        private boolean isConnected() {
            return isCurrent() && !mSocket.isClosed()
                    && mRemoteAddress.equals(mSocket.getRemoteSocketAddress());
        }

        @Override
        public void onError(final QosCallbackException exception) {
            synchronized (mSockets) {
                if (!isCurrent()) {
                    return;
                }
                // onError automatically unregisters this callback in ConnectivityManager.
                mSockets.remove(mSocket);
                ImsLog.w(this, mSlotId, "QoS callback unregistered: " + exception);
                ImsQosCallback callback = mCallback;
                if (callback != null) {
                    callback.onNotifyQosCallbackError(mSocket, mRemoteAddress);
                }
            }
        }

        @Override
        public void onQosSessionAvailable(
                QosSession session, QosSessionAttributes sessionAttributes) {
            synchronized (mSockets) {
                if (!isConnected()) {
                    return;
                }

                int qosIdentifier = 0;
                if (session.getSessionType() == QosSession.TYPE_EPS_BEARER
                        && sessionAttributes instanceof EpsBearerQosSessionAttributes) {
                    qosIdentifier = ((EpsBearerQosSessionAttributes) sessionAttributes)
                            .getQosIdentifier();
                } else if (session.getSessionType() == QosSession.TYPE_NR_BEARER
                        && sessionAttributes instanceof NrQosSessionAttributes) {
                    qosIdentifier = ((NrQosSessionAttributes) sessionAttributes).getQosIdentifier();
                }

                if (qosIdentifier == 0) {
                    ImsLog.d(this, mSlotId, "Invalid QCI value");
                    return;
                }

                boolean wasAvailable = !mAvailableSessions.isEmpty();
                mAvailableSessions.add(session.getUniqueId());
                if (!wasAvailable) {
                    ImsLog.i(mSlotId, "QoS available: " + session);
                    notifyQosConnectionAvailable(mRemoteAddress);
                }
            }
        }

        @Override
        public void onQosSessionLost(final QosSession session) {
            synchronized (mSockets) {
                if (!isConnected() || !mAvailableSessions.remove(session.getUniqueId())) {
                    return;
                }
                if (mAvailableSessions.isEmpty()) {
                    ImsLog.i(mSlotId, "QoS lost: " + session);
                    notifyQosConnectionLost(mRemoteAddress);
                }
            }
        }

    }

    private final int mSlotId;
    private volatile ImsQosCallback mCallback;

    // A remote port is not unique across sockets, early dialogs or access networks.
    private final Map<DatagramSocket, QosSocket> mSockets = new IdentityHashMap<>();

    public QosAgent(int slotId) {
        mSlotId = slotId;
    }

    public void setCallback(ImsQosCallback callback) {
        this.mCallback = callback;
    }

    private void notifyQosConnectionAvailable(InetSocketAddress remoteAddress) {
        ImsQosCallback callback = mCallback;
        if (callback != null) {
            callback.onNotifyQosConnectionAvailable(remoteAddress);
        }
    }

    private void notifyQosConnectionLost(InetSocketAddress remoteAddress) {
        ImsQosCallback callback = mCallback;
        if (callback != null) {
            callback.onNotifyQosConnectionLost(remoteAddress);
        }
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

            synchronized (mSockets) {
                registerQosSocket(network, rtpSocket);
            }
        }

        return new Pair<>(rtpSocket, rtcpSocket);
    }

    /**
     * Establish a remote network connection, specifying both the remote address and port.
     * Subsequently, register a new Quality of Service (QoS) callback function.
     * Reuse an existing callback only for the same socket, network and remote endpoint.
     * Replace obsolete callbacks without closing the media sockets.
     *
     * @param rtpSocket rtp datagram socket
     * @param rtcpSocket rtcp datagram socket
     * @param remoteAddress Remote address
     * @param remotePort Remote port
     * @param isRemoteChanged caller hint; the socket and registration are always checked
     * @return true if a callback is registered, not an indication of bearer availability
     */
    public boolean updateQosConnection(
            DatagramSocket rtpSocket, DatagramSocket rtcpSocket,
            String remoteAddress, int remotePort, boolean isRemoteChanged) {

        if (rtpSocket == null || rtcpSocket == null || rtpSocket.isClosed()
                || rtcpSocket.isClosed() || remoteAddress == null || remoteAddress.isEmpty()
                || remotePort <= 0 || remotePort >= 65535) {
            ImsLog.e(this, mSlotId,
                    "updateQosConnection - Sockets are null or invalid remote address");
            return false;
        }

        InetAddress remoteAddr = createInetAddress(remoteAddress);

        if (remoteAddr == null) {
            ImsLog.e(this, mSlotId, "updateQosConnection - Remote address not found; rtpSocket: "
                    + rtpSocket + " remotePort: " + remotePort);
            return false;
        }

        Network network = getNetworkForIpAddress(rtpSocket.getLocalAddress());

        if (network == null) {
            ImsLog.e(this, mSlotId, "updateQosConnection - Network not found; rtpSocket: "
                    + rtpSocket + " remotePort: " + remotePort);
            return false;
        }

        synchronized (mSockets) {
            InetSocketAddress remote = new InetSocketAddress(remoteAddr, remotePort);
            InetSocketAddress remoteRtcp = new InetSocketAddress(remoteAddr, remotePort + 1);
            QosSocket current = mSockets.get(rtpSocket);
            if (current != null && current.mNetwork.equals(network)
                    && current.mRemoteAddress.equals(remote) && current.isConnected()
                    && remoteRtcp.equals(rtcpSocket.getRemoteSocketAddress())) {
                return true;
            }

            // Invalidate the old callback before reconnecting the shared media socket.
            removeQosConnection(rtpSocket);
            try {
                rtpSocket.connect(remoteAddr, remotePort);
                rtcpSocket.connect(remoteAddr, remotePort + 1);
            } catch (RuntimeException e) {
                ImsLog.e(this, mSlotId, "updateQosConnection: " + e);
                return false;
            }
            return registerQosSocket(network, rtpSocket);
        }
    }

    /** Request to close the RTP/RTCP sockets and unregister their callback. */
    public void destroyQosConnection(DatagramSocket rtpSocket, DatagramSocket rtcpSocket) {
        synchronized (mSockets) {
            removeQosConnection(rtpSocket);
            ImsUtils.closeQuietly(rtpSocket);
            ImsUtils.closeQuietly(rtcpSocket);
        }
    }

    // Caller holds mSockets. Never close a socket owned by another registration.
    private void removeQosConnection(DatagramSocket rtpSocket) {
        QosSocket callback = mSockets.remove(rtpSocket);
        if (callback != null) {
            unregisterQosCallback(callback);
        }
    }

    // Caller holds mSockets. Success means registered, not that QoS is available.
    private boolean registerQosSocket(Network network, DatagramSocket rtpSocket) {
        QosSocket callback = new QosSocket(network, rtpSocket);
        mSockets.put(rtpSocket, callback);
        if (!registerQosCallback(network, rtpSocket, callback)) {
            if (mSockets.get(rtpSocket) == callback) {
                mSockets.remove(rtpSocket);
            }
            return false;
        }
        return mSockets.get(rtpSocket) == callback;
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

    private boolean registerQosCallback(
            Network network, DatagramSocket socket, QosCallback callback) {
        ConnectivityManagerProxy cmp = getConnectivityManagerProxy();
        ImsLog.d(this, mSlotId, "registerQosCallback: " + callback);
        try {
            QosSocketInfo socketInfo = new QosSocketInfo(network, socket);
            cmp.registerQosCallback(
                    socketInfo, AppContext.getInstance().getMainExecutor(), callback);
            ImsLog.i(mSlotId, "QoS callback registered on network " + network);
            return true;
        } catch (IOException | RuntimeException e) {
            ImsLog.e(this, mSlotId, "registerQosCallback: " + e);
            unregisterQosCallback(callback);
            return false;
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
