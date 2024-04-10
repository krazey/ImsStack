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

import static com.android.imsstack.base.TestAppContext.SLOT0;
import static com.android.imsstack.base.TestAppContext.SUB_ID_1;

import static org.junit.Assert.assertThrows;
import static org.mockito.ArgumentMatchers.any;
import static org.mockito.ArgumentMatchers.eq;
import static org.mockito.Mockito.verify;
import static org.mockito.Mockito.verifyNoMoreInteractions;

import android.telephony.SubscriptionManager;
import android.testing.AndroidTestingRunner;
import android.testing.TestableLooper;

import androidx.test.filters.SmallTest;

import com.android.imsstack.ContextFixture;
import com.android.imsstack.base.SystemServiceProxy.CarrierConfigManagerProxy;
import com.android.imsstack.base.TestAppContext;
import com.android.imsstack.core.carrier.SimCarrierId;

import org.junit.After;
import org.junit.Before;
import org.junit.Test;
import org.junit.runner.RunWith;
import org.mockito.Mock;
import org.mockito.MockitoAnnotations;

@RunWith(AndroidTestingRunner.class)
@TestableLooper.RunWithLooper
public class ConfigAgentTest {
    @Mock private ConfigInterface.Listener mListener;

    private ContextFixture mContextFixture;
    private TestableLooper mTestableLooper;
    private TestAppContext mTestAppContext;
    private ConfigAgent mConfigAgent;

    @Before
    public void setUp() throws Exception {
        MockitoAnnotations.initMocks(this);

        mContextFixture = new ContextFixture();
        mTestableLooper = TestableLooper.get(this);
        mTestAppContext = new TestAppContext(mContextFixture.getTestDouble());
        mTestAppContext.setUpWithLooper(mTestableLooper.getLooper());

        mConfigAgent = new ConfigAgent(SLOT0);
    }

    @After
    public void tearDown() throws Exception {
        mConfigAgent.cleanup();
        mConfigAgent = null;

        mTestAppContext.tearDown();
        mTestAppContext = null;
        mContextFixture = null;
        mTestableLooper = null;
    }

    @Test
    @SmallTest
    public void testNotifyCarrierConfigChanged() {
        mConfigAgent.addListener(mListener);
        mConfigAgent.notifyCarrierConfigChanged(SUB_ID_1);
        processAllMessages();

        verify(mListener).onCarrierConfigChanged(eq(SLOT0), eq(SUB_ID_1));

        mConfigAgent.removeListener(mListener);

        verifyNoMoreInteractions(mListener);
    }

    @Test
    @SmallTest
    public void testNotifyCarrierConfigChangedWhenInvalidSubscription() {
        mConfigAgent.addListener(mListener);
        mConfigAgent.notifyCarrierConfigChanged(SubscriptionManager.INVALID_SUBSCRIPTION_ID);
        processAllMessages();

        verify(mListener).onCarrierConfigChanged(
                eq(SLOT0), eq(SubscriptionManager.INVALID_SUBSCRIPTION_ID));

        mConfigAgent.removeListener(mListener);

        verifyNoMoreInteractions(mListener);
    }

    @Test
    @SmallTest
    public void testAddListenerWhenListenerNull() throws IllegalArgumentException {
        assertThrows(IllegalArgumentException.class, () -> {
            mConfigAgent.addListener(null);
        });

        mConfigAgent.addListener(mListener);
    }

    @Test
    @SmallTest
    public void testRemoveListenerWhenListenerNull() throws IllegalArgumentException {
        assertThrows(IllegalArgumentException.class, () -> {
            mConfigAgent.removeListener(null);
        });

        mConfigAgent.removeListener(mListener);
    }

    @Test
    @SmallTest
    public void testGetCarrierConfigWhenUpdateCarrierConfig() {
        mConfigAgent.init(mTestAppContext.getContext());

        SimCarrierId scid = new SimCarrierId.Builder().build();
        mConfigAgent.updateCarrierConfig(SUB_ID_1, scid);

        CarrierConfigManagerProxy ccmp =
                mTestAppContext.getSystemServiceProxy(CarrierConfigManagerProxy.class);
        verify(ccmp).getConfigForSubId(eq(SUB_ID_1), any());
    }

    private void processAllMessages() {
        while (!mTestableLooper.getLooper().getQueue().isIdle()) {
            mTestableLooper.processAllMessages();
        }
    }
}
