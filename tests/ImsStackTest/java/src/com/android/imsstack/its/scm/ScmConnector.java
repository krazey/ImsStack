/*
 * Copyright (C) 2025 The Android Open Source Project
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
package com.android.imsstack.its.scm;

import static org.junit.Assert.assertNotNull;
import static org.junit.Assert.assertTrue;
import static org.junit.Assert.fail;

import android.os.Bundle;

import androidx.test.platform.app.InstrumentationRegistry;

import com.android.imsstack.its.core.agents.WifiAgent;
import com.android.imsstack.util.Log;

import org.junit.rules.ExternalResource;

import java.io.DataInputStream;
import java.io.DataOutputStream;
import java.io.IOException;
import java.net.Socket;

public class ScmConnector extends ExternalResource {
    private static final String KEY_SCM = "use_scm";
    private static final String VALUE_TRUE = "true";

    private static final int CONTROL_HOST_PORT = 9527;
    private static final String HOST_IP = "192.168.98.1";

    private final WifiAgent mWifiAgent = WifiAgent.getInstance();
    private Socket mControlSocket;
    private DataOutputStream mControlOut;
    private DataInputStream mControlIn;

    @Override
    protected void before() throws Throwable {
        if (!isScmArgumentSet()) {
            return;
        }

        openScmConnection();
    }

    @Override
    protected void after() {
        closeScmConnection();
    }

    private void openScmConnection() {
        mWifiAgent.waitForWifiConnected();
        assertTrue("Failed to connect to WiFi", mWifiAgent.isWifiConnected());

        try {
            Log.i(this, "CONTROL: Attempting to connect to host IP " + HOST_IP
                    + " on port " + CONTROL_HOST_PORT);
            mControlSocket = new Socket(HOST_IP, CONTROL_HOST_PORT);
            mControlOut = new DataOutputStream(mControlSocket.getOutputStream());
            mControlIn = new DataInputStream(mControlSocket.getInputStream());
            Log.i(this, "CONTROL: Successfully connected to control server.");

            ScmRequest setupRequest = ScmRequest.newBuilder()
                    .setAction(Action.START)
                    .setNumberOfTiss(1)
                    .build();
            sendRequest(setupRequest);
            Log.i(this, "CONTROL: Sent START request");

            ScmResponse setupResponse = receiveResponse();
            Log.i(this, "CONTROL: Received setup response: '" + setupResponse + "'");
            assertNotNull("Control server did not respond during setup", setupResponse);
            assertTrue("Control server returned an error: " + setupResponse.getError(),
                    setupResponse.getError().isEmpty());

            Log.i(this, "TISS instance started at: " + setupResponse.getTissStatus(0));

        } catch (IOException e) {
            Log.e(this, "CONTROL: Failed to establish control socket", e);
            fail("Could not connect to the control server at "
                    + HOST_IP + ":" + CONTROL_HOST_PORT
                    + e);
        }
    }

    private void closeScmConnection() {
        if (mControlSocket == null || mControlSocket.isClosed()) {
            return;
        }

        try {
            Log.i(this, "CONTROL: Closing control socket connection");

            ScmRequest stopRequest = ScmRequest.newBuilder()
                    .setAction(Action.STOP)
                    .build();
            sendRequest(stopRequest);
            Log.i(this, "CONTROL: Sent STOP request");

            ScmResponse teardownResponse = receiveResponse();
            Log.i(this, "CONTROL: Received teardown response: '" + teardownResponse + "'");

        } catch (IOException e) {
            Log.e(this, "CONTROL: Error during teardown communication", e);

        } finally {
            try {
                if (mControlOut != null) mControlOut.close();
                if (mControlIn != null) mControlIn.close();
                mControlSocket.close();
            } catch (IOException e) {
                Log.e(this, "CONTROL: Error while closing control socket resources", e);
            }
        }
    }

    private void sendRequest(ScmRequest request) throws IOException {
        byte[] requestBytes = request.toByteArray();
        mControlOut.writeInt(requestBytes.length);
        mControlOut.write(requestBytes);
        mControlOut.flush();
    }

    private ScmResponse receiveResponse() throws IOException {
        int responseLength = mControlIn.readInt();
        byte[] responseBytes = new byte[responseLength];
        mControlIn.readFully(responseBytes);
        return ScmResponse.parseFrom(responseBytes);
    }

    private boolean isScmArgumentSet() {
        Bundle args = InstrumentationRegistry.getArguments();
        if (args == null) {
            return false;
        }

        return VALUE_TRUE.equalsIgnoreCase(args.getString(KEY_SCM));
    }
}
