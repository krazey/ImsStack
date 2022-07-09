/**
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

package com.android.imsstack.enabler.media;

import android.annotation.NonNull;
import android.net.ConnectivityManager;
import android.net.LinkProperties;
import android.net.Network;

import com.android.imsstack.util.AppContext;
import com.android.imsstack.util.ImsLog;

import java.io.IOException;
import java.net.DatagramSocket;
import java.net.InetAddress;
import java.net.InetSocketAddress;
import java.net.SocketException;
import java.net.UnknownHostException;
import java.util.List;

public class MediaSocket {

    /**
     * Creates a datagram socket, bound to the specified address and port
     *
     * @param address address to bind
     * @param port    The port number
     */
    public static DatagramSocket createDatagramSocket(@NonNull String address, int port) {
        //TODO: QoSManager has to be used
        DatagramSocket socket = null;

        try {
            socket = new DatagramSocket(null);

            if (socket == null) {
                ImsLog.e("socket not found");
                return null;
            }

            socket.setReuseAddress(true);
            InetAddress inetAddress = createInetAddress(address);
            if (inetAddress == null) {
                ImsLog.e("inetAddress not found");
                closeDatagramSocket(socket);
                return null;
            }

            InetSocketAddress sockAddr = new InetSocketAddress(inetAddress, port);
            socket.bind(sockAddr);

            Network network = getNetworkForIpAddress(inetAddress);
            if (network == null) {
                ImsLog.e("Network not found");
                closeDatagramSocket(socket);
                return null;
            }

            network.bindSocket(socket);
        } catch (SocketException e) {
            ImsLog.e("SocketException: " + e.toString());
        } catch (UnknownHostException e) {
            ImsLog.e("UnknownHostException: " + e.toString());
        } catch (IOException e) {
            ImsLog.e("IOException: " + e.toString());
            closeDatagramSocket(socket);
        } catch (IllegalArgumentException e) {
            ImsLog.e("IllegalArgumentException: " + e.toString());
            closeDatagramSocket(socket);
        }

        ImsLog.v("DatagramSocket created");
        return socket;
    }

    /**
     * Closes the datagram socket
     *
     * @param socket DatagramSocket to close
     */
    public static void closeDatagramSocket(DatagramSocket socket) {
        if (socket != null) {
            socket.close();
            ImsLog.v("DatagramSocket closed");
        }
    }

    private static Network getNetworkForIpAddress(InetAddress addr) {
        ConnectivityManager cm =
                AppContext.getInstance().getSystemService(ConnectivityManager.class);

        if (cm == null) {
            return null;
        }

        Network[] networks = cm.getAllNetworks();

        if (networks == null) {
            return null;
        }

        for (Network network : networks) {
            LinkProperties lp = cm.getLinkProperties(network);

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

    private static InetAddress createInetAddress(String address) {
        try {
            return InetAddress.getByName(address);
        } catch (IOException e) {
            ImsLog.e("IOException: " + e.toString());
        }

        return null;
    }
}