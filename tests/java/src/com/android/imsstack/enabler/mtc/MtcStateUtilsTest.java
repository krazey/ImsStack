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

package com.android.imsstack.enabler.mtc;

import static org.junit.Assert.assertEquals;
import static org.junit.Assert.assertFalse;
import static org.junit.Assert.assertTrue;
import static org.mockito.Mockito.doReturn;

import com.android.imsstack.core.agents.AgentFactory;
import com.android.imsstack.core.agents.ConfigInterface;
import com.android.imsstack.core.config.CarrierConfig;

import org.junit.Test;
import org.junit.runner.RunWith;
import org.junit.runners.JUnit4;
import org.mockito.Mock;
import org.mockito.MockitoAnnotations;

@RunWith(JUnit4.class)
public class MtcStateUtilsTest {
    private int mSlotId0 = 0;

    @Mock private ConfigInterface mConfigInterface;
    @Mock private CarrierConfig mCarrierConfig;

    @Test
    public void testInit() {
        MockitoAnnotations.initMocks(this);
        AgentFactory.getInstance().setAgent(ConfigInterface.class, mConfigInterface, mSlotId0);
        doReturn(mCarrierConfig).when(mConfigInterface).getCarrierConfig();

        MtcStateUtils.initializeImsState(null, mSlotId0, MtcStateUtils.INIT_ALL);
        MtcStateUtils.initializeState(null, mSlotId0);

        assertFalse(MtcStateUtils.isVoLteProvisioned(null, mSlotId0));

        doReturn(true).when(mCarrierConfig).isVoLteProvisioningRequired();
        MtcStateUtils.initializeImsState(null, mSlotId0, MtcStateUtils.INIT_ALL);
        MtcStateUtils.updateVoLteProvisioned(null, mSlotId0, MtcStateUtils.STATE_ACTIVE);
        MtcStateUtils.updateVtProvisioned(null, mSlotId0, MtcStateUtils.STATE_ACTIVE);
        MtcStateUtils.initializeState(null, mSlotId0);

        assertFalse(MtcStateUtils.isVoLteProvisioned(null, mSlotId0));
        assertFalse(MtcStateUtils.isVtProvisioned(null, mSlotId0));

        MtcStateUtils.initializeImsState(null, mSlotId0, MtcStateUtils.INIT_ALL);
    }

    @Test
    public void testRegister() {
        MtcStateUtils.updateRegState(null, mSlotId0, IUMtcService.SERVICE_EMERGENCY);

        assertEquals(IUMtcService.SERVICE_EMERGENCY,
                MtcStateUtils.getRegisteredServiceType(null, mSlotId0));

        MtcStateUtils.updateRegState(null, mSlotId0, IUMtcService.SERVICE_VT);

        assertEquals(IUMtcService.SERVICE_VT,
                MtcStateUtils.getRegisteredServiceType(null, mSlotId0));
    }

    @Test
    public void testVoLteProvisioned() {
        MtcStateUtils.updateVoLteProvisioned(null, mSlotId0, MtcStateUtils.STATE_ACTIVE);

        assertTrue(MtcStateUtils.isVoLteProvisioned(null, mSlotId0));

        MtcStateUtils.updateVoLteProvisioned(null, mSlotId0, MtcStateUtils.STATE_INACTIVE);

        assertFalse(MtcStateUtils.isVoLteProvisioned(null, mSlotId0));
    }

    @Test
    public void testVtProvisioned() {
        MtcStateUtils.updateVtProvisioned(null, mSlotId0, MtcStateUtils.STATE_ACTIVE);

        assertTrue(MtcStateUtils.isVtProvisioned(null, mSlotId0));

        MtcStateUtils.updateVtProvisioned(null, mSlotId0, MtcStateUtils.STATE_INACTIVE);

        assertFalse(MtcStateUtils.isVtProvisioned(null, mSlotId0));
    }

    @Test
    public void testWfcProvisioned() {
        MtcStateUtils.updateWfcProvisioned(null, mSlotId0, MtcStateUtils.STATE_ACTIVE);

        assertTrue(MtcStateUtils.isWfcProvisioned(null, mSlotId0));

        MtcStateUtils.updateWfcProvisioned(null, mSlotId0, MtcStateUtils.STATE_INACTIVE);

        assertFalse(MtcStateUtils.isWfcProvisioned(null, mSlotId0));
    }
}
