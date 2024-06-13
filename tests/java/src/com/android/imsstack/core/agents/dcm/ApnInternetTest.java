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

package com.android.imsstack.core.agents.dcm;

import static org.junit.Assert.assertEquals;
import static org.mockito.Mockito.times;
import static org.mockito.Mockito.verify;

import android.telephony.TelephonyManager;
import android.testing.AndroidTestingRunner;
import android.testing.TestableLooper;

import com.android.imsstack.ContextFixture;
import com.android.imsstack.base.AppContext;
import com.android.imsstack.core.agents.dcmif.EApnType;
import com.android.imsstack.core.agents.dcmif.EDataState;
import com.android.imsstack.core.agents.dcmif.IDcNetWatcher;
import com.android.imsstack.system.ISystem;

import org.junit.After;
import org.junit.AfterClass;
import org.junit.Before;
import org.junit.BeforeClass;
import org.junit.Test;
import org.junit.runner.RunWith;
import org.mockito.Mock;
import org.mockito.MockitoAnnotations;

import java.lang.reflect.Field;

@RunWith(AndroidTestingRunner.class)
@TestableLooper.RunWithLooper
public class ApnInternetTest {
    private static final int SLOT_0 = 0;
    static ContextFixture sContext;
    ApnInternet mApnInternet;

    @Mock private IDcNetWatcher mMockIDcNetWatcher;
    @Mock private ISystem mMockISystem;

    private TestableLooper mTestableLooper;

    @BeforeClass
    public static void setUpOnce() {
        sContext = new ContextFixture();
        AppContext.init(sContext.getTestDouble());
    }

    @Before
    public void setUp() throws Exception {
        MockitoAnnotations.initMocks(this);

        // create the instance to test
        mApnInternet = new ApnInternet(AppContext.getInstance(), SLOT_0);

        mTestableLooper = TestableLooper.get(ApnInternetTest.this);
        mTestableLooper.processAllMessages();

        replaceInstance(Apn.class, "mDcNetWatcher", mApnInternet, mMockIDcNetWatcher);
        replaceInstance(Apn.class, "mSystem", mApnInternet, mMockISystem);
    }

    @After
    public void tearDown() throws Exception {
        if (mApnInternet != null) {
            mApnInternet.cleanup();
            mApnInternet = null;
        }
        mTestableLooper = null;
    }

    @AfterClass
    public static void tearDownOnce() {
        AppContext.deinit();
        sContext = null;
    }

    @Test
    public void testHandleNetworkAvailable() throws Exception {
        // if data state is not DATA_CONNECTED, notify data connection state change
        mApnInternet.sendEmptyMessage(Apn.EVENT_NETWORK_AVAILABLE);
        mTestableLooper.processAllMessages();

        // if data state is already DATA_CONNECTED, do not notify data connection state change
        assertEquals(mApnInternet.getDataState(), TelephonyManager.DATA_CONNECTED);
        mApnInternet.sendEmptyMessage(Apn.EVENT_NETWORK_AVAILABLE);
        mTestableLooper.processAllMessages();

        verify(mMockIDcNetWatcher, times(1)).notifyDataConnectionState(
                EApnType.INTERNET, EDataState.DATA_STATE_CONNECTED);
        verify(mMockISystem, times(1)).notifyDataConnectionStateChanged(
                EApnType.INTERNET.getType(), EDataState.DATA_STATE_CONNECTED.getState());
    }

    @Test
    public void testHandleNetworkLost() throws Exception {
        // if data state is not DATA_DISCONNECTED, notify data connection state change
        mApnInternet.setDataState(TelephonyManager.DATA_CONNECTED);
        mApnInternet.sendEmptyMessage(Apn.EVENT_NETWORK_LOST);
        mTestableLooper.processAllMessages();

        // if data state is already DATA_DISCONNECTED, do not notify data connection state change
        assertEquals(mApnInternet.getDataState(), TelephonyManager.DATA_DISCONNECTED);
        mApnInternet.sendEmptyMessage(Apn.EVENT_NETWORK_LOST);
        mTestableLooper.processAllMessages();

        verify(mMockIDcNetWatcher, times(1)).notifyDataConnectionState(
                EApnType.INTERNET, EDataState.DATA_STATE_DISCONNECTED);
        verify(mMockISystem, times(1)).notifyDataConnectionStateChanged(
                EApnType.INTERNET.getType(), EDataState.DATA_STATE_DISCONNECTED.getState());
    }

    private synchronized void replaceInstance(final Class c, final String instanceName,
            final Object obj, final Object newValue) throws Exception {
        Field field = c.getDeclaredField(instanceName);
        field.setAccessible(true);
        field.set(obj, newValue);
    }
}
