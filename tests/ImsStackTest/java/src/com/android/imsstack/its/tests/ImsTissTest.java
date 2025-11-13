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
package com.android.imsstack.its.tests;

import static com.android.imsstack.its.base.TestConstants.SLOT0;

import static org.junit.Assert.assertNotNull;
import static org.junit.Assert.assertTrue;
import static org.junit.Assert.fail;

import android.platform.test.annotations.Presubmit;
import android.testing.AndroidTestingRunner;
import android.testing.TestableLooper;
import android.util.Log;

import com.android.imsstack.its.base.TelephonyManagerProxyImpl;
import com.android.imsstack.its.imsservice.reg.ImsRegistrationWrapper;
import com.android.imsstack.its.scm.Action;
import com.android.imsstack.its.scm.ScmRequest;
import com.android.imsstack.its.scm.ScmResponse;
import com.android.imsstack.its.servercontrol.ScenarioGeneratorUtils;
import com.android.imsstack.its.tests.registration.tests.RegistrationTestBase;
import com.android.imsstack.its.tests.registration.util.TestRegistration;
import com.android.imsstack.its.util.SingleLatch;

import org.junit.After;
import org.junit.Before;
import org.junit.Test;
import org.junit.runner.RunWith;

import java.io.DataInputStream;
import java.io.DataOutputStream;
import java.io.IOException;
import java.net.Socket;

@Presubmit
@RunWith(AndroidTestingRunner.class)
@TestableLooper.RunWithLooper
public class ImsTissTest extends RegistrationTestBase {
    private static final String TAG = "ImsTiss";
    private static final int CONTROL_HOST_PORT = 9999;
    private static final String HOST_IP = "192.168.98.1";
    private Socket mControlSocket;
    private DataOutputStream mControlOut;
    private DataInputStream mControlIn;

    private ImsRegistrationWrapper mImsRegistration;

    @Before
    public void setUp() throws Exception {
        Log.i(TAG, "ImsTissTest setup.");

        mWifiAgent.waitForWifiConnected();
        assertTrue("Failed to connect to WiFi", mWifiAgent.isWifiConnected());
        Log.i(TAG, "Successfully connected to WiFi.");

        Log.i(TAG, "Setup Connection with tiss_scm and launch TISS.");
        try {
            Log.i(
                    TAG,
                    "CONTROL: Attempting to connect to Host IP "
                            + HOST_IP
                            + " on port "
                            + CONTROL_HOST_PORT);
            mControlSocket = new Socket(HOST_IP, CONTROL_HOST_PORT);

            mControlOut = new DataOutputStream(mControlSocket.getOutputStream());
            mControlIn = new DataInputStream(mControlSocket.getInputStream());

            Log.i(TAG, "CONTROL: Successfully connected to control server.");

            ScmRequest setupRequest =
                    ScmRequest.newBuilder().setAction(Action.START).setNumberOfTiss(1).build();

            byte[] requestBytes = setupRequest.toByteArray();
            mControlOut.writeInt(requestBytes.length); // Send 4-byte length prefix
            mControlOut.write(requestBytes); // Send proto bytes
            mControlOut.flush();
            Log.i(TAG, "CONTROL: Sent START request.");

            int responseLength = mControlIn.readInt();
            byte[] responseBytes = new byte[responseLength];
            mControlIn.readFully(responseBytes);
            ScmResponse setupResponse = ScmResponse.parseFrom(responseBytes);

            Log.i(TAG, "CONTROL: Received setup response: '" + setupResponse + "'");
            assertNotNull("Control server did not respond during setup", setupResponse);
            assertTrue(
                    "SCM server returned an error: " + setupResponse.getError(),
                    setupResponse.getError().isEmpty());
            Log.i(TAG, "TISS instance started at: " + setupResponse.getTissStatus(0));

        } catch (IOException e) {
            Log.e(TAG, "CONTROL: Failed to establish control socket in setUp()", e);
            fail(
                    "Could not connect to the control server at "
                            + HOST_IP
                            + ":"
                            + CONTROL_HOST_PORT
                            + e);
        }

        setUpBase(SLOT0);
        mImsRegistration = mImsServiceConnector.getRegistration();
        mRegistration = new TestRegistration(mImsRegistration);
        createControlConnection(mRegistration);
    }

    @After
    public void tearDown() throws Exception {
        Log.i(TAG, "ImsTissTest tearDown.");
        mServerControlConnection.disconnect();

        tearDownBase(SLOT0);

        if (mControlSocket != null && !mControlSocket.isClosed()) {
            try {
                Log.i(TAG, "CONTROL: Closing control socket connection.");

                ScmRequest stopRequest = ScmRequest.newBuilder().setAction(Action.STOP).build();
                byte[] requestBytes = stopRequest.toByteArray();
                mControlOut.writeInt(requestBytes.length);
                mControlOut.write(requestBytes);
                mControlOut.flush();
                Log.i(TAG, "CONTROL: Sent STOP request.");

                int responseLength = mControlIn.readInt();
                byte[] responseBytes = new byte[responseLength];
                mControlIn.readFully(responseBytes);
                ScmResponse teardownResponse = ScmResponse.parseFrom(responseBytes);
                Log.i(TAG, "CONTROL: Received teardown response: '" + teardownResponse + "'");

                mControlOut.close();
                mControlIn.close();
                mControlSocket.close();
            } catch (IOException e) {
                Log.e(TAG, "CONTROL: Error while closing control socket", e);
            }
        }
    }

    @Test
    public void testTriggerImsRegistration() {
        Log.i(TAG, "Send scenario to TISS");
        ScenarioGeneratorUtils generator = new ScenarioGeneratorUtils();
        generator.addMessages("<200-REGISTER | >SUBSCRIBE | <200-SUBSCRIBE");
        mServerControlConnection.sendControlCommand(generator.build().toString());

        Log.i(TAG, "Start Test");
        setTestValueInitializer(
                (slotId, simApplicationState) -> {
                    int subId = getSubId(slotId);
                    TelephonyManagerProxyImpl telephony = getTelephonyManagerProxy(subId);
                    telephony.setSimCarrierId(1); // TMO-US
                });
        startImsStack(SLOT0, mConfig);
        enableAllMmTelCapabilities();
        mEventLatch.sleep(SingleLatch.SHORT_SLEEP_MS);
        mConnectivityManagerProxy.notifyNetworkAvailable(APN_IMS);

        mImsRegistration.waitForRegistered();
        assertTrue(mImsRegistration.isRegistered());
    }

    @Test
    public void testTriggerImsRegistrationStartingFromNoService() throws Exception {
        Log.i(TAG, "Send scenario to TISS");
        ScenarioGeneratorUtils generator = new ScenarioGeneratorUtils();
        generator.addMessages("<200-REGISTER | >SUBSCRIBE | <200-SUBSCRIBE");
        mServerControlConnection.sendControlCommand(generator.build().toString());

        Log.i(TAG, "Start Test");
        startImsStackWithNoService(SLOT0, mConfig);
        mEventLatch.sleep(SingleLatch.SHORT_SLEEP_MS);
        mConnectivityManagerProxy.notifyNetworkAvailable(APN_IMS);
        // Waits for AoS ready.
        mEventLatch.sleep(SingleLatch.SHORT_SLEEP_MS);

        enableAllMmTelCapabilities();
        triggerInService(SLOT0);

        // Verify that IMS registration is successfully completed within a certain period of time.
        mImsRegistration.waitForRegistered();

        assertTrue(mImsRegistration.isRegistered());
    }
}
