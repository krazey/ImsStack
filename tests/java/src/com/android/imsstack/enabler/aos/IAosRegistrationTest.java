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

package com.android.imsstack.enabler.aos;

import static org.junit.Assert.assertEquals;
import static org.junit.Assert.assertFalse;
import static org.junit.Assert.assertNotEquals;
import static org.junit.Assert.assertTrue;

import android.testing.AndroidTestingRunner;

import com.android.imsstack.enabler.aos.IAosRegistration.CapabilityPairs;
import com.android.imsstack.enabler.aos.IAosRegistration.Cause;
import com.android.imsstack.enabler.aos.IAosRegistrationListener.Capability;
import com.android.imsstack.enabler.aos.IAosRegistrationListener.NetworkType;

import org.junit.Test;
import org.junit.runner.RunWith;

@RunWith(AndroidTestingRunner.class)
public class IAosRegistrationTest {

    // --- IAosRegistration.Cause Enum Tests ---

    @Test
    public void cause_toString_shouldReturnSymbolicName() {
        for (Cause cause : Cause.values()) {
            assertEquals(
                    "toString() for " + cause.name() + " should return the exact name",
                    cause.name(),
                    cause.toString()
            );
        }
    }

    @Test
    public void cause_of_shouldReturnCorrectEnumForKnownValues() {
        assertEquals(Cause.UNKNOWN, Cause.of(0));
        assertEquals(Cause.DATA, Cause.of(1));
        assertEquals(Cause.RADIO_ALLOWED_NETWORK_TYPES_CHANGED, Cause.of(13));
        assertEquals(Cause.WIFI_OFF, Cause.of(26));
    }

    @Test
    public void cause_of_shouldReturnUnknownForInvalidValue() {
        assertEquals(Cause.UNKNOWN, Cause.of(999));
        assertEquals(Cause.UNKNOWN, Cause.of(-1));
    }

    // --- IAosRegistration.CapabilityPairs Class Tests ---

    @Test
    public void capabilityPairs_constructor_shouldBeEmpty() {
        CapabilityPairs pairs = new CapabilityPairs();
        assertTrue("Default constructor should create an empty map",
                pairs.getCapabilities().isEmpty());
    }

    @Test
    public void capabilityPairs_constructor_shouldAddInitialPair() {
        CapabilityPairs pairs = new CapabilityPairs(NetworkType.LTE, Capability.VOICE);
        assertFalse(pairs.getCapabilities().isEmpty());
        assertTrue("Constructor should add initial (LTE, VOICE) pair",
                pairs.hasCapability(NetworkType.LTE, Capability.VOICE));
    }

    @Test
    public void capabilityPairs_addCapability_shouldBitwiseOrExisting() {
        CapabilityPairs pairs = new CapabilityPairs();

        // 1. Add the first capability (VOICE = 1)
        pairs.addCapability(NetworkType.LTE, Capability.VOICE);
        assertTrue(pairs.hasCapability(NetworkType.LTE, Capability.VOICE));
        assertFalse(pairs.hasCapability(NetworkType.LTE, Capability.SMS));

        // 2. Add the second capability (SMS = 8)
        pairs.addCapability(NetworkType.LTE, Capability.SMS);

        // 3. Verify that both the existing (VOICE) and new (SMS) capabilities are present
        assertTrue("Bitwise OR failed, VOICE should still be present",
                pairs.hasCapability(NetworkType.LTE, Capability.VOICE));
        assertTrue("Bitwise OR failed, SMS should now be present",
                pairs.hasCapability(NetworkType.LTE, Capability.SMS));

        // 4. Verify that other networks are not affected
        assertFalse(pairs.hasCapability(NetworkType.IWLAN, Capability.VOICE));
    }

    @Test
    public void capabilityPairs_equals_shouldBeEqualForSameContent() {
        CapabilityPairs pairs1 = new CapabilityPairs();
        pairs1.addCapability(NetworkType.LTE, Capability.VOICE | Capability.VIDEO);
        pairs1.addCapability(NetworkType.IWLAN, Capability.SMS);

        CapabilityPairs pairs2 = new CapabilityPairs();
        pairs2.addCapability(NetworkType.LTE, Capability.VOICE);
        pairs2.addCapability(NetworkType.LTE, Capability.VIDEO);
        pairs2.addCapability(NetworkType.IWLAN, Capability.SMS);

        assertEquals("Pairs with the same content should be equal", pairs1, pairs2);
        assertEquals("Hashcode for equal objects must be the same",
                pairs1.hashCode(), pairs2.hashCode());
    }

    @Test
    public void capabilityPairs_equals_shouldNotBeEqualForDifferentContent() {
        // Create a base object for comparison: {LTE: VOICE}
        CapabilityPairs pairs1 = new CapabilityPairs();
        pairs1.addCapability(NetworkType.LTE, Capability.VOICE);

        // Case 1: Create an object with the same network but different capability: {LTE: VIDEO}
        CapabilityPairs pairs2 = new CapabilityPairs();
        pairs2.addCapability(NetworkType.LTE, Capability.VIDEO);

        // Case 2: Create an object with the same capability but different network: {IWLAN: VOICE}
        CapabilityPairs pairs3 = new CapabilityPairs();
        pairs3.addCapability(NetworkType.IWLAN, Capability.VOICE);

        // Verify that equals() checks the capability value
        assertNotEquals("Should not be equal if capabilities differ", pairs1, pairs2);

        // Verify that equals() checks the network key
        assertNotEquals("Should not be equal if networks differ", pairs1, pairs3);
    }

    @Test
    public void capabilityPairs_toString_shouldFormatCorrectlyWhenEmpty() {
        CapabilityPairs pairs = new CapabilityPairs();
        // Note the prefix/suffix spaces from Collectors.joining
        String expected = "{ Size=0,  }";
        assertEquals(expected, pairs.toString());
    }

    @Test
    public void capabilityPairs_toString_shouldFormatCorrectly() {
        CapabilityPairs pairs = new CapabilityPairs();

        // (VOICE | SMS) = 1 | 8 = 9
        int lteCaps = Capability.VOICE | Capability.SMS;
        // (VIDEO) = 2
        int iwlanCaps = Capability.VIDEO;

        pairs.addCapability(NetworkType.LTE, lteCaps);
        pairs.addCapability(NetworkType.IWLAN, iwlanCaps);

        // Create the expected string using the Capability.toString() helper
        // (Assuming IAosRegistrationListener.Capability.toString() implementation
        //  returns "[voice sms]" and "[video]")
        String lteString = IAosRegistrationListener.Capability.toString(lteCaps);
        String iwlanString = IAosRegistrationListener.Capability.toString(iwlanCaps);

        // LinkedHashMap guarantees insertion order.
        String expected = "{ Size=2, (Network=LTE, Capabilities=" + lteString + "), "
                + "(Network=IWLAN, Capabilities=" + iwlanString + ") }";

        assertEquals(expected, pairs.toString());
    }
}
