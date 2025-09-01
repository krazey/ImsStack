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

package com.android.imsstack.core;

import static com.android.imsstack.base.TestAppContext.SLOT0;
import static com.android.imsstack.base.TestAppContext.SUB_ID_1;

import static org.mockito.Mockito.verify;
import static org.mockito.Mockito.verifyNoInteractions;
import static org.mockito.Mockito.when;

import android.content.Intent;
import android.telephony.TelephonyManager;

import androidx.test.filters.SmallTest;

import com.android.imsstack.base.TestAppContext;
import com.android.imsstack.core.agents.AgentFactory;
import com.android.imsstack.core.agents.SimInterface;
import com.android.imsstack.core.agents.Usat;
import com.android.imsstack.core.agents.UsatInterface;

import org.junit.After;
import org.junit.Before;
import org.junit.Test;
import org.junit.runner.RunWith;
import org.junit.runners.JUnit4;
import org.mockito.Mock;
import org.mockito.MockitoAnnotations;

@RunWith(JUnit4.class)
public class StkSetupEventReceiverTest {
    private TestAppContext mTestAppContext;
    private StkSetupEventReceiver mStkSetupEventReceiver;

    @Mock private SimInterface mMockSimInterface;
    @Mock private UsatInterface mMockUsatInterface;

    @Before
    public void setUp() throws Exception {
        MockitoAnnotations.openMocks(this);
        mTestAppContext = new TestAppContext();
        mTestAppContext.setUp();

        AgentFactory.getInstance().setAgent(SimInterface.class, mMockSimInterface, SLOT0);
        when(mMockSimInterface.getUsatInterface()).thenReturn(mMockUsatInterface);

        mStkSetupEventReceiver = new StkSetupEventReceiver();
    }

    @After
    public void tearDown() throws Exception {
        mStkSetupEventReceiver = null;
        AgentFactory.getInstance().setAgent(SimInterface.class, null, SLOT0);
        mTestAppContext.tearDown();
        mTestAppContext = null;
    }

    @Test
    @SmallTest
    public void onReceive_withValidIntent_updatesEventList() {
        int[] setupEventList = {Usat.SETUP_EVENT_IMS_REGISTRATION};
        Intent intent = new Intent(TelephonyManager.ACTION_STK_SETUP_EVENT_LIST);
        intent.putExtra(TelephonyManager.EXTRA_SETUP_EVENT_LIST, setupEventList);
        intent.putExtra(TelephonyManager.EXTRA_SUBSCRIPTION_ID, SUB_ID_1);

        // receive broadcast intent
        mStkSetupEventReceiver.onReceive(mTestAppContext.getContext(), intent);

        verify(mMockUsatInterface).updateSetupEventList(setupEventList);
    }

    @Test
    @SmallTest
    public void onReceive_simInterfaceNotAvailable_doesNotUpdateEventList() {
        int[] setupEventList = {Usat.SETUP_EVENT_IMS_REGISTRATION};
        Intent intent = new Intent(TelephonyManager.ACTION_STK_SETUP_EVENT_LIST);
        intent.putExtra(TelephonyManager.EXTRA_SETUP_EVENT_LIST, setupEventList);
        intent.putExtra(TelephonyManager.EXTRA_SUBSCRIPTION_ID, SUB_ID_1);
        AgentFactory.getInstance().setAgent(SimInterface.class, null, SLOT0);

        // receive broadcast intent
        mStkSetupEventReceiver.onReceive(mTestAppContext.getContext(), intent);

        verifyNoInteractions(mMockUsatInterface);
    }
}
