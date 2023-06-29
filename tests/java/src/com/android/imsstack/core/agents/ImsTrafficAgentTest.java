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

import static org.junit.Assert.assertTrue;
import static org.mockito.Mockito.times;
import static org.mockito.Mockito.verify;
import static org.mockito.Mockito.when;

import android.content.Context;
import android.telephony.TelephonyManager;
import android.test.suitebuilder.annotation.SmallTest;
import android.testing.TestableLooper;

import com.android.imsstack.ContextFixture;
import com.android.imsstack.util.AppContext;

import org.junit.After;
import org.junit.Before;
import org.junit.Test;
import org.junit.runner.RunWith;
import org.junit.runners.JUnit4;
import org.mockito.Mock;
import org.mockito.MockitoAnnotations;

@RunWith(JUnit4.class)
public class ImsTrafficAgentTest {
    private static final int SLOT0 = 0;
    private static final int SLOT1 = 1;

    @Mock ImsTrafficInterface.PriorityListener mPriorityListener;

    private TelephonyManager mTelephonyManager;
    private ContextFixture mContextFixture;
    private Context mContext;
    private TestableLooper mTestableLooper;
    private ImsTrafficAgent mImsTrafficAgent;

    @Before
    public void setUp() throws Exception {
        MockitoAnnotations.initMocks(this);

        mContextFixture = new ContextFixture();
        mContext = mContextFixture.getTestDouble();
        AppContext.init(mContext);
        mTelephonyManager = mContext.getSystemService(TelephonyManager.class);
        when(mTelephonyManager.getSupportedModemCount()).thenReturn(2);
        mTestableLooper = new TestableLooper(AppContext.getInstance().getMainLooper());

        mImsTrafficAgent = new ImsTrafficAgent();
        mImsTrafficAgent.init(mContext);
        mImsTrafficAgent.addListener(mPriorityListener);
    }

    @After
    public void tearDown() throws Exception {
        if (mImsTrafficAgent != null) {
            mImsTrafficAgent.removeListener(mPriorityListener);
            mImsTrafficAgent.cleanup();
            mImsTrafficAgent = null;
        }

        if (mTestableLooper != null) {
            mTestableLooper.destroy();
            mTestableLooper = null;
        }

        AppContext.deinit();
        mContext = null;
        mContextFixture = null;
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
