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

package com.android.imsstack.enabler.ssc;

import static org.junit.Assert.assertEquals;
import static org.mockito.Mockito.when;

import com.android.imsstack.core.agents.AgentFactory;
import com.android.imsstack.core.agents.WifiInterface;
import com.android.imsstack.core.agents.dcmif.EApnType;

import org.junit.After;
import org.junit.Before;
import org.junit.Test;
import org.junit.runner.RunWith;
import org.junit.runners.JUnit4;
import org.mockito.Mock;
import org.mockito.MockitoAnnotations;

@RunWith(JUnit4.class)
public class SscNetConnectionWifiTest {
    private static final int SLOT_0 = 0;
    private static final EApnType APN_TYPE = EApnType.WIFI;

    private SscNetConnectionWifi mSscNetConnectionWifi = null;

    @Mock private WifiInterface mMockWifiInterface;


    @Before
    public void setup() {
        MockitoAnnotations.initMocks(this);

        AgentFactory.getInstance().setAgent(WifiInterface.class, mMockWifiInterface);

        mSscNetConnectionWifi = new SscNetConnectionWifi(SLOT_0);
        mSscNetConnectionWifi.init(APN_TYPE);
    }

    @After
    public void tearDown() {
        mSscNetConnectionWifi.cleanup();
        AgentFactory.getInstance().setAgent(WifiInterface.class, null);
    }

    @Test
    public void isConnected_wifiIsNull() {
        AgentFactory.getInstance().setAgent(WifiInterface.class, null);

        boolean isConnected = mSscNetConnectionWifi.isConnected();
        assertEquals(false, isConnected);
    }

    @Test
    public void isConnected_wifiIsNotConnected() {
        when(mMockWifiInterface.isWifiConnected()).thenReturn(false);

        boolean isConnected = mSscNetConnectionWifi.isConnected();
        assertEquals(false, isConnected);
    }

    @Test
    public void isConnected_wifiIsConnected() {
        when(mMockWifiInterface.isWifiConnected()).thenReturn(true);

        boolean isConnected = mSscNetConnectionWifi.isConnected();
        assertEquals(true, isConnected);
    }
}
