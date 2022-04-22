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
import com.android.imsstack.util.ImsLog;
import java.net.DatagramSocket;
import java.net.InetAddress;
import java.net.InetSocketAddress;
import java.net.SocketException;
import java.net.UnknownHostException;

public class MediaSocket {

    public static DatagramSocket createDatagramSocket(@NonNull String  address, int port) {
        //TODO: QoSManager has to be used
        DatagramSocket socket = null;

        try {
            socket = new DatagramSocket(null);
            socket.setReuseAddress(true);
            InetSocketAddress sockAddr =
                new InetSocketAddress(InetAddress.getByName(address), port);
            socket.bind(sockAddr);
            ImsLog.d("createDatagramSocket with setReuseAddress(true)");
        } catch (SocketException e) {
            ImsLog.e("SocketException: " + e.toString());
        } catch (UnknownHostException e) {
            ImsLog.e("UnknownHostException: " + e.toString());
        }

        if (socket == null) {
            ImsLog.e("socket not found");
            return null;
        }
        return socket;
    }

    public static void closeDatagramSocket(DatagramSocket socket) {
        if (socket != null) {
            socket.close();
        }
    }
}