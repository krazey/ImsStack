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
import static org.junit.Assert.assertFalse;
import static org.junit.Assert.assertNull;
import static org.mockito.ArgumentMatchers.any;
import static org.mockito.ArgumentMatchers.eq;
import static org.mockito.Mockito.verify;
import static org.mockito.Mockito.verifyNoMoreInteractions;
import static org.mockito.Mockito.when;

import android.test.suitebuilder.annotation.SmallTest;

import com.android.imsstack.core.agents.dcm.DcFactory;
import com.android.imsstack.core.agents.dcmif.IDc;
import com.android.imsstack.core.agents.dcmif.IDcNetWatcher;
import com.android.imsstack.system.ISystem;
import com.android.imsstack.system.SystemCallInterface;
import com.android.imsstack.system.SystemInterface;

import org.junit.After;
import org.junit.Before;
import org.junit.Test;
import org.junit.runner.RunWith;
import org.junit.runners.JUnit4;
import org.mockito.Mock;
import org.mockito.MockitoAnnotations;

import java.util.HashMap;

@RunWith(JUnit4.class)
public class SystemCallAgentTest {
    private static final int SLOT0 = 0;
    private static final int ISIM_EF_IMPI = 0x6F02;

    @Mock ISystem mSystem;
    @Mock SystemInterface mSystemInterface;
    @Mock ConfigInterface mConfigInterface;
    @Mock LocationInterface mLocationInterface;
    @Mock IpSecInterface mIpSecInterface;
    @Mock IDcNetWatcher mDcNetWatcher;
    @Mock SimAgent mSimAgent;
    @Mock NativeStateAgent mNativeStateAgent;

    private SystemCallAgent mSystemCallAgent;

    @Before
    public void setUp() throws Exception {
        MockitoAnnotations.initMocks(this);

        SystemInterface.setSystemInterface(mSystemInterface);
        when(mSystemInterface.getSystem(eq(SLOT0))).thenReturn(mSystem);
        AgentFactory.getInstance().setAgent(ConfigInterface.class, mConfigInterface, SLOT0);
        AgentFactory.getInstance().setAgent(LocationInterface.class, mLocationInterface, SLOT0);
        AgentFactory.getInstance().setAgent(IpSecInterface.class, mIpSecInterface, SLOT0);
        AgentFactory.getInstance().setAgent(SimInterface.class, mSimAgent, SLOT0);
        AgentFactory.getInstance().setAgent(NativeStateInterface.class, mNativeStateAgent, SLOT0);
        replaceDcNetWatcher(mDcNetWatcher);

        mSystemCallAgent = new SystemCallAgent(SLOT0);
    }

    @After
    public void tearDown() throws Exception {
        if (mSystemCallAgent != null) {
            mSystemCallAgent.destroy();
            mSystemCallAgent = null;
        }

        DcFactory.setObjects(SLOT0, null);
        AgentFactory.getInstance().setAgent(ConfigInterface.class, null, SLOT0);
        AgentFactory.getInstance().setAgent(LocationInterface.class, null, SLOT0);
        AgentFactory.getInstance().setAgent(IpSecInterface.class, null, SLOT0);
        AgentFactory.getInstance().setAgent(SimInterface.class, null, SLOT0);
        AgentFactory.getInstance().setAgent(NativeStateInterface.class, null, SLOT0);
        SystemInterface.setSystemInterface(null);
        mNativeStateAgent = null;
        mSimAgent = null;
        mDcNetWatcher = null;
        mIpSecInterface = null;
        mConfigInterface = null;
        mSystem = null;
        mSystemInterface = null;
    }

    @Test
    @SmallTest
    public void testDestroy() {
        verify(mSystem).setSystemCallInterface(any(SystemCallInterface.class));

        mSystemCallAgent.destroy();
        mSystemCallAgent = null;

        verify(mSystem).setSystemCallInterface(eq(null));
    }

    @Test
    @SmallTest
    public void testGetCarrierConfig() {
        mSystemCallAgent.getCarrierConfig();

        verify(mConfigInterface).getCarrierConfig();

        AgentFactory.getInstance().setAgent(ConfigInterface.class, null, SLOT0);

        assertNull(mSystemCallAgent.getCarrierConfig());
        verifyNoMoreInteractions(mConfigInterface);
    }

    @Test
    @SmallTest
    public void testAddIpSecSaParameter() {
        mSystemCallAgent.addIpSecSaParameter(null);

        verify(mIpSecInterface).addIpSecSaParameter(eq(null));

        AgentFactory.getInstance().setAgent(IpSecInterface.class, null, SLOT0);
        int result = mSystemCallAgent.addIpSecSaParameter(null);

        assertEquals(SystemCallInterface.RESULT_ERROR, result);
        verifyNoMoreInteractions(mIpSecInterface);
    }

    @Test
    @SmallTest
    public void testRemoveIpSecSaParameter() {
        mSystemCallAgent.removeIpSecSaParameter(1);

        verify(mIpSecInterface).removeIpSecSaParameter(eq(1));

        AgentFactory.getInstance().setAgent(IpSecInterface.class, null, SLOT0);

        verifyNoMoreInteractions(mIpSecInterface);
    }

    @Test
    @SmallTest
    public void testApplyIpSecSa() {
        mSystemCallAgent.applyIpSecSa(1, 2, 3, null);

        verify(mIpSecInterface).applyIpSecSa(eq(1), eq(2), eq(3), eq(null));

        AgentFactory.getInstance().setAgent(IpSecInterface.class, null, SLOT0);
        int result = mSystemCallAgent.applyIpSecSa(1, 2, 3, null);

        assertEquals(SystemCallInterface.RESULT_ERROR, result);
        verifyNoMoreInteractions(mIpSecInterface);
    }

    @Test
    @SmallTest
    public void testRemoveIpSecSa() {
        mSystemCallAgent.removeIpSecSa(1, 2, 3, null);

        verify(mIpSecInterface).removeIpSecSa(eq(1), eq(2), eq(3), eq(null));

        AgentFactory.getInstance().setAgent(IpSecInterface.class, null, SLOT0);

        verifyNoMoreInteractions(mIpSecInterface);
    }

    @Test
    @SmallTest
    public void testGetIsimState() {
        mSystemCallAgent.getIsimState();

        verify(mSimAgent).getIsimStateString();

        AgentFactory.getInstance().setAgent(SimInterface.class, null, SLOT0);

        assertEquals("UNKNOWN", mSystemCallAgent.getIsimState());
        verifyNoMoreInteractions(mSimAgent);
    }

    @Test
    @SmallTest
    public void testReadIsimFileAttributes() {
        int result = mSystemCallAgent.readIsimFileAttributes(ISIM_EF_IMPI);

        assertEquals(SystemCallInterface.RESULT_OK, result);
        verify(mSimAgent).readIsimFileAttributes(eq(ISIM_EF_IMPI));

        AgentFactory.getInstance().setAgent(SimInterface.class, null, SLOT0);
        result = mSystemCallAgent.readIsimFileAttributes(ISIM_EF_IMPI);

        assertEquals(SystemCallInterface.RESULT_FAIL, result);
        verifyNoMoreInteractions(mSimAgent);
    }

    @Test
    @SmallTest
    public void testReadIsimRecord() {
        int result = mSystemCallAgent.readIsimRecord(ISIM_EF_IMPI, 0);

        assertEquals(SystemCallInterface.RESULT_OK, result);
        verify(mSimAgent).readIsimRecord(eq(ISIM_EF_IMPI), eq(0));

        AgentFactory.getInstance().setAgent(SimInterface.class, null, SLOT0);
        result = mSystemCallAgent.readIsimRecord(ISIM_EF_IMPI, 0);

        assertEquals(SystemCallInterface.RESULT_FAIL, result);
        verifyNoMoreInteractions(mSimAgent);
    }

    @Test
    @SmallTest
    public void testRequestIsimAuthentication() {
        String nonce = "EMQHeGstjjt2pkTck3aM95AQtArxaBmBAADaolOr+yoMYw==";
        long owner = 1L;
        int result = mSystemCallAgent.requestIsimAuthentication(nonce, owner);

        assertEquals(SystemCallInterface.RESULT_OK, result);
        verify(mSimAgent).requestSimAuthentication(eq(Sim.APP_TYPE_ISIM), eq(nonce), eq(owner));

        AgentFactory.getInstance().setAgent(SimInterface.class, null, SLOT0);
        result = mSystemCallAgent.requestIsimAuthentication(nonce, owner);

        assertEquals(SystemCallInterface.RESULT_FAIL, result);
        verifyNoMoreInteractions(mSimAgent);
    }

    @Test
    @SmallTest
    public void testRequestUsimAuthentication() {
        String nonce = "EMQHeGstjjt2pkTck3aM95AQtArxaBmBAADaolOr+yoMYw==";
        long owner = 1L;
        int result = mSystemCallAgent.requestUsimAuthentication(nonce, owner);

        assertEquals(SystemCallInterface.RESULT_OK, result);
        verify(mSimAgent).requestSimAuthentication(eq(Sim.APP_TYPE_USIM), eq(nonce), eq(owner));

        AgentFactory.getInstance().setAgent(SimInterface.class, null, SLOT0);
        result = mSystemCallAgent.requestUsimAuthentication(nonce, owner);

        assertEquals(SystemCallInterface.RESULT_FAIL, result);
        verifyNoMoreInteractions(mSimAgent);
    }

    @Test
    @SmallTest
    public void testIsImsVoiceCallSupported() {
        mSystemCallAgent.isImsVoiceCallSupported();

        verify(mDcNetWatcher).isVops();

        replaceDcNetWatcher(null);
        boolean result = mSystemCallAgent.isImsVoiceCallSupported();

        assertFalse(result);
        verifyNoMoreInteractions(mDcNetWatcher);
    }

    @Test
    @SmallTest
    public void testUpdateNativeServiceReady() {
        int result = mSystemCallAgent.updateNativeServiceReady(true);

        assertEquals(SystemCallInterface.RESULT_OK, result);
        verify(mNativeStateAgent).updateServiceReady(eq(true));

        AgentFactory.getInstance().setAgent(NativeStateInterface.class, null, SLOT0);
        result = mSystemCallAgent.updateNativeServiceReady(true);

        assertEquals(SystemCallInterface.RESULT_FAIL, result);
        verifyNoMoreInteractions(mNativeStateAgent);
    }

    @Test
    @SmallTest
    public void testGetLastKnownLocation() {
        mSystemCallAgent.getLastKnownLocation(LocationInterface.LOCATION_CATEGORY_ALL);

        verify(mLocationInterface).getLastKnownLocation(
                eq(LocationInterface.LOCATION_CATEGORY_ALL));

        AgentFactory.getInstance().setAgent(LocationInterface.class, null, SLOT0);
        String[] result = mSystemCallAgent.getLastKnownLocation(
                LocationInterface.LOCATION_CATEGORY_ALL);

        assertNull(result);
        verifyNoMoreInteractions(mLocationInterface);
    }

    @Test
    @SmallTest
    public void testStartListeningForLocation() {
        mSystemCallAgent.startListeningForLocation(10);

        verify(mLocationInterface).startListeningForLocation(eq(10));

        AgentFactory.getInstance().setAgent(LocationInterface.class, null, SLOT0);
        mSystemCallAgent.startListeningForLocation(10);

        verifyNoMoreInteractions(mLocationInterface);
    }

    @Test
    @SmallTest
    public void testStopListeningForLocation() {
        mSystemCallAgent.stopListeningForLocation();

        verify(mLocationInterface).stopListeningForLocation();

        AgentFactory.getInstance().setAgent(LocationInterface.class, null, SLOT0);
        mSystemCallAgent.stopListeningForLocation();

        verifyNoMoreInteractions(mLocationInterface);
    }

    @Test
    @SmallTest
    public void testStartInstantLocationUpdate() {
        mSystemCallAgent.startInstantLocationUpdate();

        verify(mLocationInterface).startInstantLocationUpdate();

        AgentFactory.getInstance().setAgent(LocationInterface.class, null, SLOT0);
        mSystemCallAgent.startInstantLocationUpdate();

        verifyNoMoreInteractions(mLocationInterface);
    }

    private void replaceDcNetWatcher(IDcNetWatcher dcNetWatcher) {
        HashMap<Integer, IDc> dcObjects = DcFactory.getObjects(SLOT0);
        if (dcObjects == null) {
            dcObjects = new HashMap<>();
        }
        dcObjects.put(DcFactory.NETWORK_WATCHER, dcNetWatcher);
        DcFactory.setObjects(SLOT0, dcObjects);
    }
}
