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
import static com.android.imsstack.base.TestAppContext.SLOT1;

import static org.junit.Assert.assertFalse;
import static org.junit.Assert.assertTrue;
import static org.mockito.Mockito.when;

import androidx.test.filters.SmallTest;

import com.android.imsstack.base.DeviceConfig;

import org.junit.After;
import org.junit.Before;
import org.junit.Test;
import org.junit.runner.RunWith;
import org.junit.runners.JUnit4;
import org.mockito.Mock;
import org.mockito.MockitoAnnotations;

@RunWith(JUnit4.class)
public class AgentUtilsTest {
    @Mock private SimInterface mSim1;
    @Mock private SimInterface mSim2;

    @Before
    public void setUp() throws Exception {
        MockitoAnnotations.initMocks(this);
        DeviceConfig.setSimCount(2, 2);

        AgentFactory.getInstance().setAgent(SimInterface.class, mSim1, SLOT0);
        AgentFactory.getInstance().setAgent(SimInterface.class, mSim2, SLOT1);

        when(mSim1.getSimCardState()).thenReturn(Sim.STATE_PRESENT);
        when(mSim2.getSimCardState()).thenReturn(Sim.STATE_PRESENT);
    }

    @After
    public void tearDown() throws Exception {
        AgentFactory.getInstance().setAgent(SimInterface.class, null, SLOT0);
        AgentFactory.getInstance().setAgent(SimInterface.class, null, SLOT1);

        DeviceConfig.setSimCount(1, 1);
        mSim1 = null;
        mSim2 = null;
    }

    @Test
    @SmallTest
    public void testIsAllSimAbsent() {
        when(mSim1.getSimState()).thenReturn(Sim.STATE_ABSENT);
        when(mSim2.getSimState()).thenReturn(Sim.STATE_ABSENT);

        assertTrue(AgentUtils.isAllSimAbsent());

        when(mSim1.getSimState()).thenReturn(Sim.STATE_ABSENT);
        when(mSim2.getSimState()).thenReturn(Sim.STATE_LOADED);

        assertFalse(AgentUtils.isAllSimAbsent());

        when(mSim1.getSimCardState()).thenReturn(Sim.STATE_ABSENT);
        when(mSim2.getSimCardState()).thenReturn(Sim.STATE_ABSENT);
        when(mSim1.getSimState()).thenReturn(Sim.STATE_UNKNOWN);
        when(mSim2.getSimState()).thenReturn(Sim.STATE_UNKNOWN);

        assertTrue(AgentUtils.isAllSimAbsent());

        // Initial state.
        when(mSim1.getSimCardState()).thenReturn(Sim.STATE_UNKNOWN);
        when(mSim2.getSimCardState()).thenReturn(Sim.STATE_UNKNOWN);
        when(mSim1.getSimState()).thenReturn(Sim.STATE_UNKNOWN);
        when(mSim2.getSimState()).thenReturn(Sim.STATE_UNKNOWN);

        assertTrue(AgentUtils.isAllSimAbsent());

        AgentFactory.getInstance().setAgent(SimInterface.class, null, SLOT0);
        AgentFactory.getInstance().setAgent(SimInterface.class, null, SLOT1);

        assertTrue(AgentUtils.isAllSimAbsent());
    }

    @Test
    @SmallTest
    public void testIsAllSimAbsentOrLocked() {
        when(mSim1.getSimState()).thenReturn(Sim.STATE_ABSENT);
        when(mSim2.getSimState()).thenReturn(Sim.STATE_ABSENT);

        assertTrue(AgentUtils.isAllSimAbsentOrLocked());

        when(mSim1.getSimState()).thenReturn(Sim.STATE_ABSENT);
        when(mSim2.getSimState()).thenReturn(Sim.STATE_LOCKED);

        assertTrue(AgentUtils.isAllSimAbsentOrLocked());

        when(mSim1.getSimState()).thenReturn(Sim.STATE_LOCKED);
        when(mSim2.getSimState()).thenReturn(Sim.STATE_LOCKED);

        assertTrue(AgentUtils.isAllSimAbsentOrLocked());

        when(mSim1.getSimState()).thenReturn(Sim.STATE_ABSENT);
        when(mSim2.getSimState()).thenReturn(Sim.STATE_LOADED);

        assertFalse(AgentUtils.isAllSimAbsentOrLocked());

        when(mSim1.getSimState()).thenReturn(Sim.STATE_LOCKED);
        when(mSim2.getSimState()).thenReturn(Sim.STATE_LOADED);

        assertFalse(AgentUtils.isAllSimAbsentOrLocked());

        when(mSim1.getSimCardState()).thenReturn(Sim.STATE_ABSENT);
        when(mSim2.getSimCardState()).thenReturn(Sim.STATE_ABSENT);
        when(mSim1.getSimState()).thenReturn(Sim.STATE_UNKNOWN);
        when(mSim2.getSimState()).thenReturn(Sim.STATE_UNKNOWN);

        assertTrue(AgentUtils.isAllSimAbsentOrLocked());

        // Initial state.
        when(mSim1.getSimCardState()).thenReturn(Sim.STATE_UNKNOWN);
        when(mSim2.getSimCardState()).thenReturn(Sim.STATE_UNKNOWN);
        when(mSim1.getSimState()).thenReturn(Sim.STATE_UNKNOWN);
        when(mSim2.getSimState()).thenReturn(Sim.STATE_UNKNOWN);

        assertTrue(AgentUtils.isAllSimAbsentOrLocked());

        AgentFactory.getInstance().setAgent(SimInterface.class, null, SLOT0);
        AgentFactory.getInstance().setAgent(SimInterface.class, null, SLOT1);

        assertTrue(AgentUtils.isAllSimAbsentOrLocked());
    }
}
