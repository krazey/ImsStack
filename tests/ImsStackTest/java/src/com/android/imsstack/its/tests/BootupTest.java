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
package com.android.imsstack.its.tests;

import static com.android.imsstack.its.base.TestConstants.SLOT0;

import static org.junit.Assert.assertTrue;

import android.os.PersistableBundle;
import android.testing.AndroidTestingRunner;
import android.testing.TestableLooper;

import com.android.imsstack.its.base.SystemProxyResolver;
import com.android.imsstack.its.base.TelephonyManagerProxyImpl;
import com.android.imsstack.its.imsservice.reg.ImsRegistrationWrapper;
import com.android.imsstack.its.util.SingleLatch;

import org.junit.After;
import org.junit.Before;
import org.junit.Test;
import org.junit.runner.RunWith;

@RunWith(AndroidTestingRunner.class)
@TestableLooper.RunWithLooper
public class BootupTest extends ImsStackTestBase {
    private final SingleLatch mEventLatch = new SingleLatch(BootupTest.class.getSimpleName());
    private PersistableBundle mConfig = null;
    private ImsRegistrationWrapper mImsRegistration;

    @Before
    public void setUp() throws Exception {
        TelephonyManagerProxyImpl telephony =
                SystemProxyResolver.getTelephonyManagerProxy(getSubId(SLOT0));
        // TODO: Need to be removed when ImsService can handle the startImsTraffic.
        telephony.setHalVersion(-2, -2);

        setUpBase(SLOT0);

        mImsRegistration = mImsServiceConnector.getRegistration();
    }

    @After
    public void tearDown() throws Exception {
        tearDownBase(SLOT0);
    }

    @Test
    public void testTriggerImsRegistration() throws Exception {
        setTestValueInitializer((slotId, simApplicationState) -> {
            int subId = getSubId(slotId);
            TelephonyManagerProxyImpl telephony = getTelephonyManagerProxy(subId);
            telephony.setSimCarrierId(1); // TMO-US
        });
        startImsStack(SLOT0, mConfig);
        enableAllMmTelCapabilities();
        mEventLatch.sleep(SingleLatch.SHORT_SLEEP_MS);
        mConnectivityManagerProxy.notifyNetworkAvailable(APN_IMS);

        // Verify that IMS registration is successfully completed within a certain period of time.
        mImsRegistration.waitForRegistered();

        assertTrue(mImsRegistration.isRegistered());
    }

    @Test
    public void testTriggerImsRegistrationStartingFromNoService() throws Exception {
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
