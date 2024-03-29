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

package com.android.imsstack.core.agents;

import static com.android.imsstack.base.TestAppContext.SLOT0;
import static com.android.imsstack.base.TestAppContext.SUB_ID_1;
import static com.android.imsstack.base.TestAppContext.SUB_ID_2;

import static org.mockito.ArgumentMatchers.any;
import static org.mockito.ArgumentMatchers.eq;
import static org.mockito.Mockito.times;
import static org.mockito.Mockito.verify;
import static org.mockito.Mockito.when;

import android.telephony.TelephonyCallback;
import android.telephony.TelephonyManager;
import android.testing.AndroidTestingRunner;
import android.testing.TestableLooper;

import androidx.test.filters.SmallTest;

import com.android.imsstack.base.TelephonyManagerProxy;
import com.android.imsstack.base.TestAppContext;
import com.android.imsstack.system.ISystem;
import com.android.imsstack.system.SystemInterface;

import org.junit.After;
import org.junit.Before;
import org.junit.Test;
import org.junit.runner.RunWith;
import org.mockito.ArgumentCaptor;
import org.mockito.Mock;
import org.mockito.MockitoAnnotations;

import java.util.Collections;
import java.util.Set;

@RunWith(AndroidTestingRunner.class)
@TestableLooper.RunWithLooper
public class ImsRadioAgentTest {
    @Mock private ImsTrafficInterface mMockImsTrafficInterface;
    @Mock private ISystem mMockISystem;
    @Mock private SystemInterface mMockSystemInterface;

    private TestableLooper mTestableLooper;
    private TestAppContext mTestAppContext;
    private TelephonyManagerProxy mTelephonyManagerProxy;
    private ImsRadioAgent mImsRadioAgent;

    @Before
    public void setUp() throws Exception {
        MockitoAnnotations.initMocks(this);

        mTestableLooper = TestableLooper.get(this);
        mTestAppContext = new TestAppContext();
        mTestAppContext.setUpWithLooper(mTestableLooper.getLooper());

        AgentFactory.getInstance().setAgent(ImsTrafficInterface.class, mMockImsTrafficInterface);
        SystemInterface.setSystemInterface(mMockSystemInterface);
        when(mMockSystemInterface.getSystem(SLOT0)).thenReturn(mMockISystem);

        mTelephonyManagerProxy = mTestAppContext.getSystemServiceProxy(TelephonyManagerProxy.class);
        when(mTelephonyManagerProxy.getHalVersion(TelephonyManager.HAL_SERVICE_IMS))
                .thenReturn(TelephonyManager.HAL_VERSION_UNSUPPORTED);

        mImsRadioAgent = new ImsRadioAgent(SLOT0);
        mImsRadioAgent.init(mTestAppContext.getContext());
    }

    @After
    public void tearDown() throws Exception {
        if (mImsRadioAgent != null) {
            mImsRadioAgent.cleanup();
            mImsRadioAgent = null;
        }

        mTestAppContext.tearDown();
        mTestAppContext = null;
    }

    @Test
    @SmallTest
    public void notifyImsTrafficAndNativeIfScSupportIsChangedToSupport() {
        Set<Integer> scSupportIds = Set.of(SUB_ID_1, SUB_ID_2);
        TelephonyCallback.SimultaneousCellularCallingSupportListener imsSccsListener =
                captureImsSccsListener();

        imsSccsListener.onSimultaneousCellularCallingSubscriptionsChanged(scSupportIds);

        verify(mMockImsTrafficInterface).setSimultaneousCallingSupported(eq(true), eq(SLOT0));
        verify(mMockISystem).notifySimultaneousCallingSupportChanged(
                eq(mImsRadioAgent.EVENT_SIMULTANEOUS_CALLING_SUPPORT_CHANGED), eq(true));
    }

    @Test
    @SmallTest
    public void notifyImsTrafficAndNativeIfScSupportIsChangedToNotSupport() {
        Set<Integer> scSupportIds = Set.of(SUB_ID_1, SUB_ID_2);
        TelephonyCallback.SimultaneousCellularCallingSupportListener imsSccsListener =
                captureImsSccsListener();
        imsSccsListener.onSimultaneousCellularCallingSubscriptionsChanged(scSupportIds);

        imsSccsListener.onSimultaneousCellularCallingSubscriptionsChanged(Collections.emptySet());

        verify(mMockImsTrafficInterface).setSimultaneousCallingSupported(eq(false), eq(SLOT0));
        verify(mMockISystem).notifySimultaneousCallingSupportChanged(
                eq(mImsRadioAgent.EVENT_SIMULTANEOUS_CALLING_SUPPORT_CHANGED), eq(false));
    }

    @Test
    @SmallTest
    public void ignoreIfScSupportIsChangedWithSameValue() {
        Set<Integer> scSupportIds = Set.of(SUB_ID_1, SUB_ID_2);
        TelephonyCallback.SimultaneousCellularCallingSupportListener imsSccsListener =
                captureImsSccsListener();

        imsSccsListener.onSimultaneousCellularCallingSubscriptionsChanged(scSupportIds);
        imsSccsListener.onSimultaneousCellularCallingSubscriptionsChanged(scSupportIds);

        verify(mMockImsTrafficInterface, times(1))
                .setSimultaneousCallingSupported(eq(true), eq(SLOT0));
        verify(mMockISystem, times(1)).notifySimultaneousCallingSupportChanged(
                eq(mImsRadioAgent.EVENT_SIMULTANEOUS_CALLING_SUPPORT_CHANGED), eq(true));
    }

    private TelephonyCallback.SimultaneousCellularCallingSupportListener captureImsSccsListener() {
        ArgumentCaptor<TelephonyCallback> captor = ArgumentCaptor.forClass(TelephonyCallback.class);
        verify(mTelephonyManagerProxy).registerTelephonyCallback(any(), captor.capture());
        return (TelephonyCallback.SimultaneousCellularCallingSupportListener) captor.getValue();
    }
}
