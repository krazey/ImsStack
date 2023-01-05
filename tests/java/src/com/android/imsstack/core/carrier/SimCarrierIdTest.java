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
package com.android.imsstack.core.carrier;

import static org.junit.Assert.assertEquals;
import static org.junit.Assert.assertFalse;
import static org.junit.Assert.assertTrue;

import android.test.suitebuilder.annotation.SmallTest;

import org.junit.Test;
import org.junit.runner.RunWith;
import org.junit.runners.JUnit4;

@RunWith(JUnit4.class)
public class SimCarrierIdTest {
    private static final int SIM_CARRIER_ID = 100;
    private static final int SIM_SPECIFIC_CARRIER_ID = 101;
    private static final String SIM_MCC = "001";
    private static final String SIM_MNC = "01";
    private static final String SIM_IMSI = "2040439303";
    private static final String SIM_GID1 = "A00020";
    private static final String SIM_SPN = "Test-Carrier";
    private static final String SIM_ICCID = "89011201";

    @Test
    @SmallTest
    public void constructor_simAbsent() throws Exception {
        SimCarrierId cid = new SimCarrierId.Builder().build();

        assertEquals(SimCarrierId.UNKNOWN_ID, cid.getCarrierId());
        assertEquals(SimCarrierId.UNKNOWN_ID, cid.getSpecificCarrierId());
        assertEquals("", cid.getMcc());
        assertEquals("", cid.getMnc());
        assertEquals("", cid.getImsi());
        assertEquals("", cid.getGid1());
        assertEquals("", cid.getSpn());
        assertEquals("", cid.getIccId());
        assertFalse(cid.isSimLoaded());
        assertFalse(cid.isSimLocked());
        assertTrue(cid.isSimAbsent());
    }

    @Test
    @SmallTest
    public void constructor_simLocked() throws Exception {
        SimCarrierId cid = new SimCarrierId.Builder()
                .setIccId(SIM_ICCID)
                .setSimState(SimCarrierId.SIM_LOCKED)
                .build();

        assertEquals(SimCarrierId.UNKNOWN_ID, cid.getCarrierId());
        assertEquals(SimCarrierId.UNKNOWN_ID, cid.getSpecificCarrierId());
        assertEquals("", cid.getMcc());
        assertEquals("", cid.getMnc());
        assertEquals("", cid.getImsi());
        assertEquals("", cid.getGid1());
        assertEquals("", cid.getSpn());
        assertEquals(SIM_ICCID, cid.getIccId());
        assertFalse(cid.isSimLoaded());
        assertTrue(cid.isSimLocked());
        assertFalse(cid.isSimAbsent());
    }

    @Test
    @SmallTest
    public void constructor_simLoaded() throws Exception {
        SimCarrierId cid = new SimCarrierId.Builder()
                .setCarrierId(SIM_CARRIER_ID)
                .setSpecificCarrierId(SIM_SPECIFIC_CARRIER_ID)
                .setMcc(SIM_MCC)
                .setMnc(SIM_MNC)
                .setImsi(SIM_IMSI)
                .setGid1(SIM_GID1)
                .setSpn(SIM_SPN)
                .setIccId(SIM_ICCID)
                .setSimState(SimCarrierId.SIM_LOADED)
                .build();

        assertEquals(SIM_CARRIER_ID, cid.getCarrierId());
        assertEquals(SIM_SPECIFIC_CARRIER_ID, cid.getSpecificCarrierId());
        assertEquals(SIM_MCC, cid.getMcc());
        assertEquals(SIM_MNC, cid.getMnc());
        assertEquals(SIM_IMSI, cid.getImsi());
        assertEquals(SIM_GID1, cid.getGid1());
        assertEquals(SIM_SPN, cid.getSpn());
        assertEquals(SIM_ICCID, cid.getIccId());
        assertTrue(cid.isSimLoaded());
        assertFalse(cid.isSimLocked());
        assertFalse(cid.isSimAbsent());
    }

    @Test
    @SmallTest
    public void equals() throws Exception {
        SimCarrierId cid1 = new SimCarrierId.Builder().build();
        SimCarrierId cid2 = new SimCarrierId.Builder().build();

        assertTrue(cid1.equals(cid2));

        cid2 = new SimCarrierId.Builder()
                .setCarrierId(SIM_CARRIER_ID)
                .setSpecificCarrierId(SIM_SPECIFIC_CARRIER_ID)
                .setMcc(SIM_MCC)
                .setMnc(SIM_MNC)
                .setImsi(SIM_IMSI)
                .setGid1(SIM_GID1)
                .setSpn(SIM_SPN)
                .setIccId(SIM_ICCID)
                .setSimState(SimCarrierId.SIM_LOADED)
                .build();

        assertFalse(cid1.equals(cid2));

        SimCarrierId cid3 = new SimCarrierId.Builder()
                .setCarrierId(SIM_CARRIER_ID)
                .setSpecificCarrierId(SIM_SPECIFIC_CARRIER_ID)
                .setMcc(SIM_MCC)
                .setMnc(SIM_MNC)
                .setImsi(SIM_IMSI)
                .setGid1(SIM_GID1)
                .setSpn(SIM_SPN)
                .setIccId(SIM_ICCID)
                .setSimState(SimCarrierId.SIM_LOADED)
                .build();

        assertTrue(cid2.equals(cid3));
    }
}
