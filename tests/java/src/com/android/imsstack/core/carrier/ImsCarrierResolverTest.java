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
package com.android.imsstack.core.carrier;

import static org.junit.Assert.assertEquals;
import static org.junit.Assert.assertFalse;
import static org.junit.Assert.assertNotNull;
import static org.junit.Assert.assertTrue;

import android.test.suitebuilder.annotation.SmallTest;

import org.junit.Test;
import org.junit.runner.RunWith;
import org.junit.runners.JUnit4;

@RunWith(JUnit4.class)
public class ImsCarrierResolverTest {
    @Test
    @SmallTest
    public void testGetCarrierFromCarrierIdWithTestSim() {
        int carrierId = 1911;
        SimCarrierId scid = new SimCarrierId.Builder()
                .setCarrierId(carrierId)
                .build();
        ImsCarrierResolver.Carrier carrier = ImsCarrierResolver.getCarrierFromCarrierId(scid);

        assertNotNull(carrier);
        assertEquals(carrierId, carrier.getCarrierId());
        assertEquals(SimCarrierId.UNKNOWN_ID, carrier.getSpecificCarrierId());
        assertEquals("TEST", carrier.getOperator());
        assertEquals("COM", carrier.getCountry());
        assertEquals("", carrier.getOperatorSub());
        assertTrue(carrier.isTestSim());
        assertTrue(carrier.isValid());
    }

    @Test
    @SmallTest
    public void testGetCarrierFromCarrierIdWithKrSim() {
        int carrierId = 1890;
        SimCarrierId scid = new SimCarrierId.Builder()
                .setCarrierId(carrierId)
                .build();
        ImsCarrierResolver.Carrier carrier = ImsCarrierResolver.getCarrierFromCarrierId(scid);

        assertCarrier(carrier, carrierId, "KT", "KR");

        carrierId = 1891;
        scid = new SimCarrierId.Builder()
                .setCarrierId(carrierId)
                .build();
        carrier = ImsCarrierResolver.getCarrierFromCarrierId(scid);

        assertCarrier(carrier, carrierId, "SKT", "KR");

        carrierId = 1892;
        scid = new SimCarrierId.Builder()
                .setCarrierId(carrierId)
                .build();
        carrier = ImsCarrierResolver.getCarrierFromCarrierId(scid);

        assertCarrier(carrier, carrierId, "LGU", "KR");
    }

    @Test
    @SmallTest
    public void testGetCarrierFromCarrierIdWithJpSim() {
        int carrierId = 850;
        SimCarrierId scid = new SimCarrierId.Builder()
                .setCarrierId(carrierId)
                .build();
        ImsCarrierResolver.Carrier carrier = ImsCarrierResolver.getCarrierFromCarrierId(scid);

        assertCarrier(carrier, carrierId, "DCM", "JP");

        carrierId = 1581;
        scid = new SimCarrierId.Builder()
                .setCarrierId(carrierId)
                .build();
        carrier = ImsCarrierResolver.getCarrierFromCarrierId(scid);

        assertCarrier(carrier, carrierId, "KDDI", "JP");

        carrierId = 1894;
        scid = new SimCarrierId.Builder()
                .setCarrierId(carrierId)
                .build();
        carrier = ImsCarrierResolver.getCarrierFromCarrierId(scid);

        assertCarrier(carrier, carrierId, "SBM", "JP");
    }

    @Test
    @SmallTest
    public void testGetCarrierFromCarrierIdWithUsSim() {
        int carrierId = 1;
        SimCarrierId scid = new SimCarrierId.Builder()
                .setCarrierId(carrierId)
                .build();
        ImsCarrierResolver.Carrier carrier = ImsCarrierResolver.getCarrierFromCarrierId(scid);

        assertCarrier(carrier, carrierId, "TMO", "US");

        carrierId = 1187;
        scid = new SimCarrierId.Builder()
                .setCarrierId(carrierId)
                .build();
        carrier = ImsCarrierResolver.getCarrierFromCarrierId(scid);

        assertCarrier(carrier, carrierId, "ATT", "US");

        carrierId = 1779;
        scid = new SimCarrierId.Builder()
                .setCarrierId(carrierId)
                .build();
        carrier = ImsCarrierResolver.getCarrierFromCarrierId(scid);

        assertCarrier(carrier, carrierId, "CRK", "US");

        carrierId = 1839;
        scid = new SimCarrierId.Builder()
                .setCarrierId(carrierId)
                .build();
        carrier = ImsCarrierResolver.getCarrierFromCarrierId(scid);

        assertCarrier(carrier, carrierId, "VZW", "US");

        carrierId = 1246;
        scid = new SimCarrierId.Builder()
                .setCarrierId(carrierId)
                .build();
        carrier = ImsCarrierResolver.getCarrierFromCarrierId(scid);

        assertCarrier(carrier, carrierId, "VZW", "US");

        carrierId = 1952;
        scid = new SimCarrierId.Builder()
                .setCarrierId(carrierId)
                .build();
        carrier = ImsCarrierResolver.getCarrierFromCarrierId(scid);

        assertCarrier(carrier, carrierId, "USC", "US");

        carrierId = 1989;
        int specificCarrierId = 10014;
        scid = new SimCarrierId.Builder()
                .setCarrierId(carrierId)
                .setSpecificCarrierId(specificCarrierId)
                .build();
        carrier = ImsCarrierResolver.getCarrierFromCarrierId(scid);

        assertCarrier(carrier, carrierId, specificCarrierId, "TMO", "US");

        specificCarrierId = 10015;
        scid = new SimCarrierId.Builder()
                .setCarrierId(carrierId)
                .setSpecificCarrierId(specificCarrierId)
                .build();
        carrier = ImsCarrierResolver.getCarrierFromCarrierId(scid);

        assertCarrier(carrier, carrierId, specificCarrierId, "SPR", "US");

        specificCarrierId = 10016;
        scid = new SimCarrierId.Builder()
                .setCarrierId(carrierId)
                .setSpecificCarrierId(specificCarrierId)
                .build();
        carrier = ImsCarrierResolver.getCarrierFromCarrierId(scid);

        assertCarrier(carrier, carrierId, specificCarrierId, "USC", "US");

        specificCarrierId = 10018;
        scid = new SimCarrierId.Builder()
                .setCarrierId(carrierId)
                .setSpecificCarrierId(specificCarrierId)
                .build();
        carrier = ImsCarrierResolver.getCarrierFromCarrierId(scid);

        assertFalse(carrier.isValid());

        carrierId = 2119;
        scid = new SimCarrierId.Builder()
                .setCarrierId(2119)
                .build();
        carrier = ImsCarrierResolver.getCarrierFromCarrierId(scid);

        assertCarrier(carrier, carrierId, "ATT", "US");

        carrierId = 2256;
        scid = new SimCarrierId.Builder()
                .setCarrierId(carrierId)
                .build();
        carrier = ImsCarrierResolver.getCarrierFromCarrierId(scid);

        assertCarrier(carrier, carrierId, "CCA", "US");
    }

    @Test
    @SmallTest
    public void testGetCarrierFromCarrierIdWithCaSim() {
        int carrierId = 576;
        SimCarrierId scid = new SimCarrierId.Builder()
                .setCarrierId(carrierId)
                .build();
        ImsCarrierResolver.Carrier carrier = ImsCarrierResolver.getCarrierFromCarrierId(scid);

        assertCarrier(carrier, carrierId, "BELL", "CA");

        carrierId = 578;
        scid = new SimCarrierId.Builder()
                .setCarrierId(carrierId)
                .build();
        carrier = ImsCarrierResolver.getCarrierFromCarrierId(scid);

        assertCarrier(carrier, carrierId, "BELL", "CA", "MTS");

        carrierId = 580;
        scid = new SimCarrierId.Builder()
                .setCarrierId(carrierId)
                .build();
        carrier = ImsCarrierResolver.getCarrierFromCarrierId(scid);

        assertCarrier(carrier, carrierId, "BELL", "CA", "SKC");

        carrierId = 1403;
        scid = new SimCarrierId.Builder()
                .setCarrierId(carrierId)
                .build();
        carrier = ImsCarrierResolver.getCarrierFromCarrierId(scid);

        assertCarrier(carrier, carrierId, "RGS", "CA");

        carrierId = 1404;
        scid = new SimCarrierId.Builder()
                .setCarrierId(carrierId)
                .build();
        carrier = ImsCarrierResolver.getCarrierFromCarrierId(scid);

        assertCarrier(carrier, carrierId, "TLS", "CA");

        carrierId = 1895;
        scid = new SimCarrierId.Builder()
                .setCarrierId(carrierId)
                .build();
        carrier = ImsCarrierResolver.getCarrierFromCarrierId(scid);

        assertCarrier(carrier, carrierId, "VTR", "CA", "FRD");

        carrierId = 2008;
        scid = new SimCarrierId.Builder()
                .setCarrierId(carrierId)
                .build();
        carrier = ImsCarrierResolver.getCarrierFromCarrierId(scid);

        assertCarrier(carrier, carrierId, "VTR", "CA");
    }

    @Test
    @SmallTest
    public void testGetCarrierFromCarrierIdWithUnknownSim() {
        SimCarrierId scid = new SimCarrierId.Builder()
                .setCarrierId(SimCarrierId.UNKNOWN_ID)
                .build();
        ImsCarrierResolver.Carrier carrier = ImsCarrierResolver.getCarrierFromCarrierId(scid);

        assertCarrier(carrier, SimCarrierId.UNKNOWN_ID, SimCarrierId.UNKNOWN_ID, "OPEN", "COM");

        scid = new SimCarrierId.Builder()
                .setCarrierId(60000)
                .setSpecificCarrierId(60001)
                .build();
        carrier = ImsCarrierResolver.getCarrierFromCarrierId(scid);

        assertCarrier(carrier, 60000, 60001, "60000", "60001");
    }

    private void assertCarrier(ImsCarrierResolver.Carrier carrier, int carrierId,
            String op, String co) {
        assertCarrier(carrier, carrierId, op, co, "");
    }

    private void assertCarrier(ImsCarrierResolver.Carrier carrier, int carrierId,
            int specificCarrierId, String op, String co) {
        assertCarrier(carrier, carrierId, specificCarrierId, op, co, "");
    }

    private void assertCarrier(ImsCarrierResolver.Carrier carrier, int carrierId,
            String op, String co, String opSub) {
        assertCarrier(carrier, carrierId, SimCarrierId.UNKNOWN_ID, op, co, opSub);
    }

    private void assertCarrier(ImsCarrierResolver.Carrier carrier, int carrierId,
            int specificCarrierId, String op, String co, String opSub) {
        assertNotNull(carrier);
        assertEquals(carrierId, carrier.getCarrierId());
        assertEquals(specificCarrierId, carrier.getSpecificCarrierId());
        assertEquals(op, carrier.getOperator());
        assertEquals(co, carrier.getCountry());
        assertEquals(opSub, carrier.getOperatorSub());
        assertFalse(carrier.isTestSim());
        assertTrue(carrier.isValid());
        assertNotNull(carrier.toString());
    }
}
