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

import static org.junit.Assert.assertEquals;
import static org.junit.Assert.assertNull;
import static org.mockito.ArgumentMatchers.eq;
import static org.mockito.Mockito.verify;

import android.test.suitebuilder.annotation.SmallTest;

import com.android.imsstack.system.SystemInterface;

import org.junit.After;
import org.junit.Before;
import org.junit.Test;
import org.junit.runner.RunWith;
import org.junit.runners.JUnit4;
import org.mockito.Mock;
import org.mockito.Mockito;
import org.mockito.MockitoAnnotations;

@RunWith(JUnit4.class)
public class DefaultSystemCallAgentTest {
    @Mock SystemInterface mSystemInterface;

    private DefaultSystemCallAgent mDefaultSystemCallAgent;

    @Before
    public void setUp() throws Exception {
        MockitoAnnotations.initMocks(this);

        SystemInterface.setSystemInterface(mSystemInterface);

        mDefaultSystemCallAgent = new DefaultSystemCallAgent();
    }

    @After
    public void tearDown() throws Exception {
        if (mDefaultSystemCallAgent != null) {
            mDefaultSystemCallAgent.destroy();
            mDefaultSystemCallAgent = null;
        }
        SystemInterface.setSystemInterface(null);
        mSystemInterface = null;
    }

    @Test
    @SmallTest
    public void testCreateAndDestroy() {
        verify(mSystemInterface).setSystemCallInterface(eq(mDefaultSystemCallAgent));

        mDefaultSystemCallAgent.destroy();
        mDefaultSystemCallAgent = null;

        verify(mSystemInterface).setSystemCallInterface(eq(null));
    }

    @Test
    @SmallTest
    public void testGetWifiInterface() {
        WifiInterface wifiInterface = Mockito.mock(WifiInterface.class);
        AgentFactory.getInstance().setAgent(WifiInterface.class, wifiInterface);
        WifiInterface wifi = mDefaultSystemCallAgent.getWifiInterface();

        assertEquals(wifiInterface, wifi);

        AgentFactory.getInstance().setAgent(WifiInterface.class, null);
        wifi = mDefaultSystemCallAgent.getWifiInterface();

        assertNull(wifi);
    }
}
