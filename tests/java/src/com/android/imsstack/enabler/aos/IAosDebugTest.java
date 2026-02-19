/*
 * Copyright (C) 2026 The Android Open Source Project
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

import static com.google.common.truth.Truth.assertThat;

import static org.junit.Assert.assertThrows;

import org.junit.Before;
import org.junit.Test;
import org.junit.runner.RunWith;
import org.junit.runners.JUnit4;

import java.util.Map;

@RunWith(JUnit4.class)
public class IAosDebugTest {

    private IAosDebug.DebugData mDebugData;

    @Before
    public void setUp() {
        mDebugData = new IAosDebug.DebugData();
    }

    @Test
    public void debugKey_values_areMappedCorrectly() {
        assertThat(IAosDebug.DebugKey.SUB_ID.getValue()).isEqualTo(0);
        assertThat(IAosDebug.DebugKey.REGISTER.getValue()).isEqualTo(1);
    }

    @Test
    public void constructor_initializesData_withDefaultValues() {
        Map<IAosDebug.DebugKey, String> data = mDebugData.getDebugData();

        for (IAosDebug.DebugKey key : IAosDebug.DebugKey.values()) {
            if (key == IAosDebug.DebugKey.DATA_CONNECTION_STATE
                    || key == IAosDebug.DebugKey.WIFI_CONNECTION_STATE) {
                // Connection state keys should be initialized to DISCONNECTED.
                assertThat(data.get(key)).isEqualTo(IAosDebug.DebugData.STR_DISCONNECTED);
            } else {
                // All other keys should be initialized to "_" (STR_EMPTY).
                assertThat(data.get(key)).isEqualTo(IAosDebug.DebugData.STR_EMPTY);
            }
        }
    }

    @Test
    public void put_savesValue_and_get_retrievesIt() {
        mDebugData.put(IAosDebug.DebugKey.APN_NAME, "test_apn");

        // Verify that the value saved by put() is correctly retrieved by get().
        assertThat(mDebugData.get(IAosDebug.DebugKey.APN_NAME)).isEqualTo("test_apn");
    }

    @Test
    public void putInt_convertsIntToString_and_savesIt() {
        mDebugData.putInt(IAosDebug.DebugKey.SUB_ID, 123);

        // Verify that putInt() converts the integer to a String before saving.
        assertThat(mDebugData.get(IAosDebug.DebugKey.SUB_ID)).isEqualTo("123");
    }

    @Test
    public void get_returnsUnderscore_whenKeyHasNoValue() {
        // Verify that get() returns the default value STR_EMPTY when no value has been set.
        assertThat(mDebugData.get(IAosDebug.DebugKey.FEATURES))
                .isEqualTo(IAosDebug.DebugData.STR_EMPTY);
    }

    @Test
    public void clear_retainsValues_forPreservedKeys() {
        mDebugData.put(IAosDebug.DebugKey.SUB_ID, "123");
        mDebugData.put(IAosDebug.DebugKey.APN_NAME, "test_apn");

        mDebugData.clear();

        // Verify that SUB_ID retains its value as it is in PRESERVED_KEYS.
        assertThat(mDebugData.get(IAosDebug.DebugKey.SUB_ID)).isEqualTo("123");
        // Verify that APN_NAME is reset to the default value.
        assertThat(mDebugData.get(IAosDebug.DebugKey.APN_NAME))
                .isEqualTo(IAosDebug.DebugData.STR_EMPTY);
    }

    @Test
    public void clear_resetsConnectionState_toDisconnected() {
        mDebugData.put(IAosDebug.DebugKey.DATA_CONNECTION_STATE,
                IAosDebug.DebugData.STR_CONNECTED);
        mDebugData.put(IAosDebug.DebugKey.WIFI_CONNECTION_STATE,
                IAosDebug.DebugData.STR_CONNECTED);

        mDebugData.clear();

        // Verify that clear() resets the connection states to DISCONNECTED.
        assertThat(mDebugData.get(IAosDebug.DebugKey.DATA_CONNECTION_STATE))
                .isEqualTo(IAosDebug.DebugData.STR_DISCONNECTED);
        assertThat(mDebugData.get(IAosDebug.DebugKey.WIFI_CONNECTION_STATE))
                .isEqualTo(IAosDebug.DebugData.STR_DISCONNECTED);
    }

    @Test
    public void getDebugData_returnsUnmodifiableMap() {
        Map<IAosDebug.DebugKey, String> data = mDebugData.getDebugData();

        // Verify modification throws UnsupportedOperationException.
        assertThrows(UnsupportedOperationException.class,
                () -> data.put(IAosDebug.DebugKey.APN_NAME, "fail"));
    }
}
