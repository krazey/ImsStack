/*
 * Copyright (C) 2023 The Android Open Source Project
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
import static com.android.imsstack.base.TestAppContext.SLOT1;

import static org.junit.Assert.assertTrue;
import static org.mockito.Mockito.times;
import static org.mockito.Mockito.verify;

import android.testing.AndroidTestingRunner;
import android.testing.TestableLooper;

import androidx.test.filters.SmallTest;

import com.android.imsstack.base.DeviceConfig;
import com.android.imsstack.base.TestAppContext;

import org.junit.After;
import org.junit.Before;
import org.junit.Test;
import org.junit.runner.RunWith;
import org.mockito.Mock;
import org.mockito.MockitoAnnotations;

@RunWith(AndroidTestingRunner.class)
@TestableLooper.RunWithLooper
public class ImsTrafficAgentTest {
    @Mock private ImsTrafficInterface.PriorityListener mPriorityListener;

    private TestableLooper mTestableLooper;
    private TestAppContext mTestAppContext;
    private ImsTrafficAgent mImsTrafficAgent;

    @Before
    public void setUp() throws Exception {
        MockitoAnnotations.initMocks(this);

        mTestableLooper = TestableLooper.get(this);
        mTestAppContext = new TestAppContext();
        mTestAppContext.setUpWithLooper(mTestableLooper.getLooper());
        DeviceConfig.setSimCount(2, 2);

        mImsTrafficAgent = new ImsTrafficAgent();
        mImsTrafficAgent.init(mTestAppContext.getContext());
        mImsTrafficAgent.addListener(mPriorityListener);
    }

    @After
    public void tearDown() throws Exception {
        if (mImsTrafficAgent != null) {
            mImsTrafficAgent.removeListener(mPriorityListener);
            mImsTrafficAgent.cleanup();
            mImsTrafficAgent = null;
        }

        DeviceConfig.setSimCount(1, 1);
        mTestAppContext.tearDown();
        mTestAppContext = null;
    }

    @Test
    @SmallTest
    public void testTrafficIsAllowedIfSimultaneousCallingIsSupported() {
        mImsTrafficAgent.setSimultaneousCallingSupported(true, SLOT0);
        assertTrue(mImsTrafficAgent.isAllowed(ImsRadioInterface.TRAFFIC_TYPE_REGISTRATION, SLOT0));
    }

    @Test
    @SmallTest
    public void testIsAllowed() {
        assertTrue(mImsTrafficAgent.isAllowed(ImsRadioInterface.TRAFFIC_TYPE_UT_XCAP, SLOT0));
    }

    @Test
    @SmallTest
    public void testSetTrafficPriority() {
        mImsTrafficAgent.setTrafficPriority(ImsTrafficAgent.TRAFFIC_PRIORITY_REGISTRATION, SLOT0);
        processAllMessages();
        verify(mPriorityListener, times(1)).onTrafficPriorityChanged();
    }

    @Test
    @SmallTest
    public void testSetWlan() {
        mImsTrafficAgent.setWlan(true, SLOT0);
        mImsTrafficAgent.setTrafficPriority(ImsTrafficAgent.TRAFFIC_PRIORITY_REGISTRATION, SLOT0);
        processAllMessages();
        verify(mPriorityListener, times(1)).onTrafficPriorityChanged();
        assertTrue(mImsTrafficAgent.isAllowed(ImsRadioInterface.TRAFFIC_TYPE_UT_XCAP, SLOT1));
    }

    @Test
    @SmallTest
    public void testAddListener() {
        mImsTrafficAgent.setTrafficPriority(ImsTrafficAgent.TRAFFIC_PRIORITY_REGISTRATION, SLOT0);
        processAllMessages();
        verify(mPriorityListener, times(1)).onTrafficPriorityChanged();
    }

    @Test
    @SmallTest
    public void testRemoveListener() {
        mImsTrafficAgent.removeListener(mPriorityListener);
        mImsTrafficAgent.setTrafficPriority(ImsTrafficAgent.TRAFFIC_PRIORITY_REGISTRATION, SLOT0);
        processAllMessages();
        verify(mPriorityListener, times(0)).onTrafficPriorityChanged();
    }

    private void processAllMessages() {
        while (!mTestableLooper.getLooper().getQueue().isIdle()) {
            mTestableLooper.processAllMessages();
        }
    }
}
