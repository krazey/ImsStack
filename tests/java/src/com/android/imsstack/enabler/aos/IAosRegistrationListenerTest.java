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
import static org.junit.Assert.assertNotNull;
import static org.junit.Assert.assertTrue;

import android.telephony.ims.ImsReasonInfo;
import android.testing.AndroidTestingRunner;
import android.util.Pair;

import com.android.imsstack.enabler.aos.IAosRegistrationListener.Capability;
import com.android.imsstack.enabler.aos.IAosRegistrationListener.CapabilityReason;
import com.android.imsstack.enabler.aos.IAosRegistrationListener.FeatureTagMask;
import com.android.imsstack.enabler.aos.IAosRegistrationListener.NetworkType;
import com.android.imsstack.enabler.aos.IAosRegistrationListener.ReasonCode;
import com.android.imsstack.enabler.aos.IAosRegistrationListener.ReasonCodeMap;
import com.android.imsstack.enabler.aos.IAosRegistrationListener.RegistrationState;
import com.android.imsstack.enabler.aos.IAosRegistrationListener.RegistrationType;

import org.junit.Test;
import org.junit.runner.RunWith;

@RunWith(AndroidTestingRunner.class)
public class IAosRegistrationListenerTest {

    // --- RegistrationState Tests ---

    @Test
    public void registrationState_getValue_shouldReturnCorrectValues() {
        assertEquals(0, RegistrationState.DEREGISTERED.getValue());
        assertEquals(1, RegistrationState.REGISTERING.getValue());
        assertEquals(2, RegistrationState.REGISTERED.getValue());
    }

    @Test
    public void registrationState_toString_shouldReturnLowercaseName() {
        assertEquals("deregistered", RegistrationState.DEREGISTERED.toString());
        assertEquals("registering", RegistrationState.REGISTERING.toString());
        assertEquals("registered", RegistrationState.REGISTERED.toString());
    }

    // --- RegistrationType Tests ---

    @Test
    public void registrationType_toString_shouldReturnCorrectNames() {
        assertEquals("NORMAL", RegistrationType.toString(RegistrationType.NORMAL));
        assertEquals("EMERGENCY", RegistrationType.toString(RegistrationType.EMERGENCY));
        assertEquals("FAKE", RegistrationType.toString(RegistrationType.FAKE));
    }

    @Test
    public void registrationType_toString_shouldReturnNoneForDefault() {
        assertEquals("NONE", RegistrationType.toString(99));
        assertEquals("NONE", RegistrationType.toString(-1));
    }

    // --- NetworkType Tests ---

    @Test
    public void networkType_getValue_shouldReturnCorrectValues() {
        assertEquals(-1, NetworkType.NONE.getValue());
        assertEquals(0, NetworkType.LTE.getValue());
        assertEquals(1, NetworkType.IWLAN.getValue());
        assertEquals(2, NetworkType.CROSS_SIM.getValue());
        assertEquals(3, NetworkType.NR.getValue());
        assertEquals(4, NetworkType.UTRAN.getValue());
    }

    @Test
    public void networkType_toString_shouldReturnSymbolicName() {
        for (NetworkType type : NetworkType.values()) {
            assertEquals("toString() for " + type.name(),
                    type.name(),
                    type.toString()
            );
        }
    }

    @Test
    public void networkType_of_shouldReturnCorrectEnum() {
        assertEquals(NetworkType.NONE, NetworkType.of(-1));
        assertEquals(NetworkType.LTE, NetworkType.of(0));
        assertEquals(NetworkType.UTRAN, NetworkType.of(4));
    }

    @Test
    public void networkType_of_shouldReturnNoneForInvalidValue() {
        assertEquals(NetworkType.NONE, NetworkType.of(99));
        assertEquals(NetworkType.NONE, NetworkType.of(-10));
    }

    // --- Capability Tests ---

    @Test
    public void capability_toString_shouldReturnEmptyBracketsForNone() {
        assertEquals("[]", Capability.toString(Capability.NONE));
    }

    @Test
    public void capability_toString_shouldReturnSingleName() {
        assertEquals("[voice ]", Capability.toString(Capability.VOICE));
        assertEquals("[sms ]", Capability.toString(Capability.SMS));
    }

    @Test
    public void capability_toString_shouldReturnMultipleNames() {
        int caps = Capability.VOICE | Capability.UT | Capability.PRESENCE_UCE;
        String expected = "[voice ut presence_uce ]";
        assertEquals(expected, Capability.toString(caps));
    }

    // --- FeatureTagMask Tests ---

    @Test
    public void featureTagMask_toString_shouldReturnEmptyBracketsForNone() {
        assertEquals("[]", FeatureTagMask.toString(FeatureTagMask.NONE));
    }

    @Test
    public void featureTagMask_toString_shouldReturnSingleName() {
        assertEquals("[mmtel ]", FeatureTagMask.toString(FeatureTagMask.MMTEL));
        assertEquals("[smsip ]", FeatureTagMask.toString(FeatureTagMask.SMSIP));
    }

    @Test
    public void featureTagMask_toString_shouldReturnMultipleNames() {
        int features = FeatureTagMask.MMTEL | FeatureTagMask.VIDEO | FeatureTagMask.PRESENCE;
        String expected = "[mmtel video presence ]";
        assertEquals(expected, FeatureTagMask.toString(features));
    }

    // --- ReasonCode Tests ---

    @Test
    public void reasonCode_getValue_shouldReturnCorrectCalculatedValues() {
        // BASE (0) + 0
        assertEquals(0, ReasonCode.UNSPECIFIED.getValue());
        // BASE (0) + 10
        assertEquals(10, ReasonCode.USIM_AUTHENTICATION_FAILURES.getValue());

        // BASE_MODEM (2000) + 0
        assertEquals(2000, ReasonCode.PLMN_BLOCK.getValue());

        // BASE_RESP_4XX (14000) + 403
        assertEquals(14403, ReasonCode.REG_RESP_403.getValue());

        // BASE_RESP_WFC_OTHER (27000) + 3
        assertEquals(27003, ReasonCode.WFC_SUB_NOTIFY_TERMINATED.getValue());
    }

    @Test
    public void reasonCode_toString_shouldReturnSymbolicName() {
        for (ReasonCode reason : ReasonCode.values()) {
            assertEquals("toString() for " + reason.name(),
                    reason.name(),
                    reason.toString()
            );
        }
    }

    @Test
    public void reasonCode_of_shouldReturnCorrectEnum() {
        assertEquals(ReasonCode.UNSPECIFIED, ReasonCode.of(0));
        assertEquals(ReasonCode.INTERNAL_ERROR, ReasonCode.of(11));
        assertEquals(ReasonCode.PLMN_BLOCK, ReasonCode.of(2000));
        assertEquals(ReasonCode.REG_RESP_403, ReasonCode.of(14403));
    }

    @Test
    public void reasonCode_of_shouldReturnUnspecifiedForInvalidValue() {
        assertEquals(ReasonCode.UNSPECIFIED, ReasonCode.of(99999));
        assertEquals(ReasonCode.UNSPECIFIED, ReasonCode.of(-1));
    }

    // --- ReasonCodeMap Tests ---

    @Test
    public void reasonCodeMap_getReasonMap_shouldNotBeEmpty() {
        assertNotNull(ReasonCodeMap.getReasonMap());
        // Simple check that some keys exist in the map
        assertTrue(ReasonCodeMap.getReasonMap().containsKey(ReasonCode.REG_RESP_403));
        assertTrue(ReasonCodeMap.getReasonMap().containsKey(
                ReasonCode.DATA_EPDG_TUNNEL_IKEV2_AUTH_FAILURE));
    }

    @Test
    public void reasonCodeMap_getImsReasonPair_shouldHandleDataDisconnected() {
        int arbitraryDataFailCause = 12345;

        Pair<Integer, Integer> expected = Pair.create(
                ImsReasonInfo.CODE_LOCAL_NETWORK_NO_SERVICE,
                arbitraryDataFailCause
        );

        assertEquals(expected, ReasonCodeMap.getImsReasonPair(
                ReasonCode.DATA_DISCONNECTED,
                arbitraryDataFailCause
        ));
    }

    @Test
    public void reasonCodeMap_getImsReasonPair_shouldReturnMappedPair() {
        // A key that exists in the map (and is not DATA_DISCONNECTED)
        Pair<Integer, Integer> expected = Pair.create(
                ImsReasonInfo.CODE_REGISTRATION_ERROR,
                ImsReasonInfo.CODE_SIP_FORBIDDEN
        );
        assertEquals(expected, ReasonCodeMap.getImsReasonPair(ReasonCode.REG_RESP_403, 0));
    }

    @Test
    public void reasonCodeMap_getImsReasonPair_shouldReturnDefaultPairForUnknown() {
        // A key that does not exist in the map
        Pair<Integer, Integer> expectedDefault = Pair.create(
                ImsReasonInfo.CODE_REGISTRATION_ERROR,
                ImsReasonInfo.CODE_UNSPECIFIED
        );

        // UNSPECIFIED is not defined in the map, so it should return the default pair.
        assertEquals(expectedDefault, ReasonCodeMap.getImsReasonPair(ReasonCode.UNSPECIFIED, 0));
    }

    // --- CapabilityReason Tests ---

    @Test
    public void capabilityReason_toString_shouldReturnCorrectNames() {
        assertEquals("ERROR_GENERIC", CapabilityReason.toString(CapabilityReason.ERROR_GENERIC));
        assertEquals("SUCCESS", CapabilityReason.toString(CapabilityReason.SUCCESS));
    }

    @Test
    public void capabilityReason_toString_shouldReturnUnknownForDefault() {
        int unknownReason = 123;
        String expected = "UNKNOWN(" + unknownReason + ")";
        assertEquals(expected, CapabilityReason.toString(unknownReason));
    }
}
