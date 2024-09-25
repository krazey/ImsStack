/*
 * Copyright (C) 2024 The Android Open Source Project
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
package com.android.imsstack.its.servercontrol;

import com.android.imsstack.its.core.agents.WifiAgent;
import com.android.imsstack.util.Log;

import java.io.PrintWriter;
import java.net.InetSocketAddress;
import java.net.Socket;
import java.util.concurrent.CompletableFuture;
import java.util.concurrent.ExecutorService;
import java.util.concurrent.Executors;

/**
 * {@code ControlConnection} is responsible for establishing a socket connection to a remote server
 * over a Wi-Fi network. It allows sending control commands to the server and receiving a response
 * asynchronously. It ensures the connection is managed properly and handles cases where the
 * Wi-Fi network is unavailable or lost.
 */
public final class ControlConnection {
    private static final ExecutorService EXECUTOR_SERVICE = Executors.newCachedThreadPool();
    // TODO: The port number should be injected to the constructor so that multiple servers can be
    // supported.
    private static final int SERVER_PORT = 12345;

    private Socket mSocket;
    private CompletableFuture<Void> mConnectionFuture;

    /**
     * Constructs a {@code ControlConnection} and initiates the connection to the server using
     * Wi-Fi.
     *
     * @param serverIp The IP address of the server to connect to.
     * @throws RuntimeException If Wi-Fi is not enabled or any other error occurs during connection.
     */
    public ControlConnection(String serverIp) {
        mConnectionFuture = connectToServer(serverIp);
    }

    /**
     * Sends a control command to the server and receives a response asynchronously.
     *
     * @param command The control command to be sent to the server.
     * @throws RuntimeException If the socket is not connected or if an error occurs during
     *                          communication.
     */
    public CompletableFuture<Void> sendControlCommand(String command) {
        return mConnectionFuture.thenAcceptAsync(v -> {
            if (mSocket == null || !mSocket.isConnected()) {
                throw new RuntimeException("Socket is not connected. Cannot send command.");
            }

            try (PrintWriter out = new PrintWriter(mSocket.getOutputStream(), true)) {
                out.println(command);
                // TODO: Read the result and interrupts the test.
            } catch (Exception e) {
                throw new RuntimeException("Error during communication");
            }
        }, EXECUTOR_SERVICE);
    }

    /**
     * Disconnects the control connection by closing the socket.
     *
     * This method ensures the socket is properly closed, preventing potential resource leaks.
     */
    public void disconnect() {
        try {
            if (mSocket != null) {
                mSocket.close();
                Log.i(Log.TAG, "Socket closed.");
            }
        } catch (Exception e) {
            Log.e(Log.TAG, "Error during disconnect");
        }
    }

    /**
     * Establishes a connection to the server over Wi-Fi. This method is non-blocking and returns
     * a {@code CompletableFuture} which completes when the connection is established or fails.
     *
     * @param serverIp The IP address of the server to connect to.
     * @return A {@code CompletableFuture<Void>} which completes once the connection is established.
     */
    private CompletableFuture<Void> connectToServer(String serverIp) {
        CompletableFuture<Void> future = new CompletableFuture<>();

        if (!WifiAgent.getInstance().isWifiConnected()) {
            future.completeExceptionally(
                new RuntimeException("Wi-Fi is not connected."));
            return future;
        }

        try {
            mSocket = WifiAgent.getInstance().getNetwork().getSocketFactory().createSocket();
            mSocket.connect(new InetSocketAddress(serverIp, SERVER_PORT), 5000);
            future.complete(null);
        } catch (Exception e) {
            future.completeExceptionally(
                new RuntimeException("Error during connection", e));
        }

        return future;
    }
}
