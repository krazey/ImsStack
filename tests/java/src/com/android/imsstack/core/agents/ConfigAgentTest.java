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

import static org.junit.Assert.assertThrows;
import static org.mockito.ArgumentMatchers.eq;
import static org.mockito.Mockito.verify;
import static org.mockito.Mockito.verifyNoMoreInteractions;

import android.telephony.SubscriptionManager;
import android.test.suitebuilder.annotation.SmallTest;
import android.testing.TestableLooper;

import com.android.imsstack.ContextFixture;
import com.android.imsstack.util.AppContext;

import org.junit.After;
import org.junit.AfterClass;
import org.junit.Before;
import org.junit.BeforeClass;
import org.junit.Test;
import org.junit.runner.RunWith;
import org.junit.runners.JUnit4;
import org.mockito.Mock;
import org.mockito.MockitoAnnotations;

@RunWith(JUnit4.class)
public class ConfigAgentTest {
    private static final int SLOT0 = 0;
    private static final int[] SUB_ID = { 1 };

    static ContextFixture sContext;

    @Mock ConfigInterface.Listener mListener;

    private TestableLooper mTestableLooper;
    private ConfigAgent mConfigAgent;

    public ConfigAgentTest() {
        mConfigAgent = new ConfigAgent(SLOT0);
    }

    @BeforeClass
    public static void setUpOnce() {
        sContext = new ContextFixture();
        AppContext.init(sContext.getTestDouble());
    }

    @Before
    public void setUp() throws Exception {
        MockitoAnnotations.initMocks(this);

        mTestableLooper = new TestableLooper(AppContext.getInstance().getMainLooper());
    }

    @After
    public void tearDown() throws Exception {
        mConfigAgent.cleanup();

        if (mTestableLooper != null) {
            mTestableLooper.destroy();
            mTestableLooper = null;
        }
    }

    @AfterClass
    public static void tearDownOnce() {
        AppContext.deinit();
        sContext = null;
    }

    @Test
    @SmallTest
    public void notifyCarrierConfigChanged() {
        mConfigAgent.addListener(mListener);
        mConfigAgent.notifyCarrierConfigChanged(SUB_ID[0]);
        processAllMessages();

        verify(mListener).onCarrierConfigChanged(eq(SLOT0), eq(SUB_ID[0]));

        mConfigAgent.removeListener(mListener);

        verifyNoMoreInteractions(mListener);
    }

    @Test
    @SmallTest
    public void notifyCarrierConfigChangedWhenInvalidSubscription() {
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
    public void addListenerWhenListenerNull() throws IllegalArgumentException {
        assertThrows(IllegalArgumentException.class, () -> {
            mConfigAgent.addListener(null);
        });

        mConfigAgent.addListener(mListener);
    }

    @Test
    @SmallTest
    public void removeListenerWhenListenerNull() throws IllegalArgumentException {
        assertThrows(IllegalArgumentException.class, () -> {
            mConfigAgent.removeListener(null);
        });

        mConfigAgent.removeListener(mListener);
    }

    private void processAllMessages() {
        while (!mTestableLooper.getLooper().getQueue().isIdle()) {
            mTestableLooper.processAllMessages();
        }
    }
}
