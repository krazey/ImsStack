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

import static org.junit.Assert.assertTrue;

import android.platform.test.annotations.Presubmit;
import android.testing.AndroidTestingRunner;
import android.testing.TestableLooper;
import android.util.Log;

import com.android.imsstack.its.base.TelephonyManagerProxyImpl;
import com.android.imsstack.its.imsservice.reg.ImsRegistrationWrapper;
import com.android.imsstack.its.servercontrol.ScenarioGeneratorUtils;
import com.android.imsstack.its.tests.registration.tests.RegistrationTestBase;
import com.android.imsstack.its.tests.registration.util.TestRegistration;
import com.android.imsstack.its.util.SingleLatch;

import org.junit.After;
import org.junit.Before;
import org.junit.Test;
import org.junit.runner.RunWith;

@Presubmit
@RunWith(AndroidTestingRunner.class)
@TestableLooper.RunWithLooper
public class ImsTissTest extends RegistrationTestBase {
    private static final String TAG = "ImsTiss";

    private ImsRegistrationWrapper mImsRegistration;

    @Before
    public void setUp() throws Exception {
        Log.i(TAG, "ImsTissTest setup.");

        setUpBase(SLOT0);
        mImsRegistration = mImsServiceConnector.getRegistration();
        mRegistration = new TestRegistration(mImsRegistration);
        createControlConnection(mRegistration);
    }

    @After
    public void tearDown() throws Exception {
        Log.i(TAG, "ImsTissTest tearDown.");

        tearDownBase(SLOT0);
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
