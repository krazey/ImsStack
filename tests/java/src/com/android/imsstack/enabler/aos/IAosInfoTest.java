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

import org.junit.Test;
import org.junit.runner.RunWith;

@RunWith(AndroidTestingRunner.class)
public class IAosInfoTest {

    @Test
    public void emergencyCallbackModeState_getValue_shouldReturnCorrectValues() {
        assertEquals(0, IAosInfo.EmergencyCallbackModeState.STOP.getValue());
        assertEquals(1, IAosInfo.EmergencyCallbackModeState.START.getValue());
        assertEquals(2, IAosInfo.EmergencyCallbackModeState.STOP_BY_EMERGENCY.getValue());
    }

    @Test
    public void emergencyCallbackModeState_toString_shouldReturnNameAndValue() {
        for (IAosInfo.EmergencyCallbackModeState state :
                IAosInfo.EmergencyCallbackModeState.values()) {

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
