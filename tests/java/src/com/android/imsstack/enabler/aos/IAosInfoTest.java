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

import android.testing.AndroidTestingRunner;

import com.android.imsstack.core.agents.EmergencyStateInterface.EmergencyCallbackModeState;

import org.junit.Test;
import org.junit.runner.RunWith;

@RunWith(AndroidTestingRunner.class)
public class IAosInfoTest {

    @Test
    public void roamingPreferredVoiceNetwork_getValue_shouldReturnCorrectValues() {
        assertEquals(0, IAosInfo.RoamingPreferredVoiceNetwork.CELLULAR.getValue());
        assertEquals(1, IAosInfo.RoamingPreferredVoiceNetwork.WIFI.getValue());
    }

    @Test
    public void serviceSetting_getValue_shouldReturnCorrectValues() {
        assertEquals(0, IAosInfo.ServiceSetting.OFF.getValue());
        assertEquals(1, IAosInfo.ServiceSetting.ON.getValue());
        assertEquals(2, IAosInfo.ServiceSetting.PRESENTITY.getValue());
    }

    @Test
    public void locationInfo_getValue_shouldReturnCorrectValues() {
        assertEquals(1, IAosInfo.LocationInfo.FIXED.getValue());
        assertEquals(2, IAosInfo.LocationInfo.COUNTRY_CHANGED.getValue());
        assertEquals(3, IAosInfo.LocationInfo.CHANGED.getValue());
        assertEquals(4, IAosInfo.LocationInfo.AVAILABLE.getValue());
    }

    @Test
    public void phoneNumberState_getValue_shouldReturnCorrectValues() {
        assertEquals(0, IAosInfo.PhoneNumberState.SIM_LOADED.getValue());
        assertEquals(1, IAosInfo.PhoneNumberState.RETRY_SUCCESS.getValue());
        assertEquals(2, IAosInfo.PhoneNumberState.RETRY_FAILURE.getValue());
    }

    @Test
    public void crossSimStatus_getValue_shouldReturnCorrectValues() {
        assertEquals(0, IAosInfo.CrossSimStatus.DATA_DISCONNECTED.getValue());
        assertEquals(1, IAosInfo.CrossSimStatus.DATA_CONNECTED.getValue());
    }

    @Test
    public void emergencyCallbackModeType_getValue_shouldReturnCorrectValues() {
        assertEquals(1, IAosInfo.EmergencyCallbackModeType.CALL.getValue());
        assertEquals(2, IAosInfo.EmergencyCallbackModeType.SMS.getValue());
    }

    @Test
    public void emergencyCallbackModeState_getValue_shouldReturnCorrectValues() {
        assertEquals(0, EmergencyCallbackModeState.STOP.getValue());
        assertEquals(1, EmergencyCallbackModeState.START.getValue());
        assertEquals(2, EmergencyCallbackModeState.STOP_BY_EMERGENCY.getValue());
    }

    @Test
    public void emergencyCallbackModeState_toString_shouldReturnNameAndValue() {
        for (EmergencyCallbackModeState state : EmergencyCallbackModeState.values()) {

            String expected = state.name() + "(" + state.getValue() + ")";
            String actual = state.toString();

            assertEquals(
                    "toString() for " + state.name() + " should return NAME(VALUE) format",
                    expected,
                    actual
            );
        }
    }
}
